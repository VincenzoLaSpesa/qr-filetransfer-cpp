#include "Server.h"
#include "ConnectionListener.h"
#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <fstream>
#include <fmt/printf.h>


std::uint64_t filesize(const std::string &filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    return static_cast<std::uint64_t>(in.tellg());
}

template <typename RESP>
RESP init_resp(RESP resp) {
    resp.append_header("qr-filetransfer_cpp", "qr-filetransfer_cpp powered by RESTinio");
    resp.append_header_date_field()
        .append_header("Content-Type", "text/plain; charset=utf-8");

    return resp;
}

restinio::request_handling_status_t sendfile(const std::string &file_path, std::shared_ptr<restinio::request_t> request) {
	try {
        restinio::file_offset_t m_data_offset{0};
        restinio::file_size_t m_data_size{filesize(file_path)};
		
		auto sf = restinio::sendfile(file_path);
        sf.offset_and_size(m_data_offset,m_data_size);

        return init_resp(request->create_response())
            .append_header_date_field()
            .append_header(restinio::http_field::content_type,"application / octet - stream")
            .set_body(std::move(sf))
            .done();
    } catch (const std::exception &) {
        return request->create_response(
             restinio::status_not_found())
            .connection_close()
            .append_header_date_field()
            .done();
    }
	return restinio::request_rejected();
}

// https://github.com/Stiffstream/restinio/blob/master/dev/sample/express_router_tutorial/main.cpp
Server::router *Server::make_router(const std::string &served_path, const std::string &randomized_path) {
    auto *r = new router();
    std::string path;
    if (randomized_path.size() > 0)
        path.append(randomized_path);
    else
        path = std::experimental::filesystem::path(served_path).filename().u8string();
	fmt::printf("%s:%d : %s -> %s", __FILE__, __LINE__, served_path, path);
    r->http_get(
        "/"+path,
        [&](auto req, auto) {
            auto res= sendfile(served_path, req);
            if (!this->keep_alive_)
                this->InitShutdown();
			return res;
        });
	
	r->http_get(
        "/",
        [](auto req, auto) {
            init_resp(req->create_response())
                .append_header(restinio::http_field::content_type, "text/plain; charset=utf-8")
                .set_body("This is qr-filetransfer_cpp")
                .done();

            return restinio::request_accepted();
        });

    /*r->http_get(
        "/json",
        [](auto req, auto) {
            init_resp(req->create_response())
                .append_header(restinio::http_field::content_type, "text/json; charset=utf-8")
                .set_body(R"-({"message" : "Hello world!"})-")
                .done();

            return restinio::request_accepted();
        });

    r->http_get(
        "/html",
        [](auto req, auto) {
            init_resp(req->create_response())
                .append_header(restinio::http_field::content_type, "text/html; charset=utf-8")
                .set_body(
                    "<html>\r\n"
                    "  <head><title>Hello from RESTinio!</title></head>\r\n"
                    "  <body>\r\n"
                    "    <center><h1>Hello world</h1></center>\r\n"
                    "  </body>\r\n"
                    "</html>\r\n")
                .done();

            return restinio::request_accepted();
        });

    r->http_get("/single/:param", [](auto req, auto params) {
        return init_resp(req->create_response())
            .set_body(
                fmt::format(
                    "GET request with single parameter: '{}'",
                    params["param"]))
            .done();
    });*/

    return r;
}

Server::Server(const std::string &addr, unsigned short port, const std::string &served_path, const std::string &randomized_path, bool keep_alive) {
    keep_alive_ = keep_alive;
	std::unique_ptr<Server::router> r{make_router(served_path, randomized_path)};
    std::shared_ptr<ConnectionListener> connection_listener{new ConnectionListener()};
	server_.reset(new http_server{
        restinio::own_io_context(),
        restinio::server_settings_t<server_traits>{}
            .port(port)
            .address(addr)
            .connection_state_listener(connection_listener)
            .request_handler(std::move(r))});

    runner_.reset(new restinio::on_pool_runner_t<http_server>{
        std::thread::hardware_concurrency(),
        *server_});
    runner_->start();
    connection_listener->set_server(this);
	started_ = WaitForStartup();
}

bool Server::WaitForStartup(int timeout_seconds) {
    using namespace std::chrono_literals;
    int waited = 0;
    while (waited < timeout_seconds) {
        if (server_ != nullptr && runner_ != nullptr && runner_->started())
            return true;
        std::this_thread::sleep_for(1s);
        waited++;
    }
    return false;
}

void Server::Wait() {
    if (started_)
        runner_->wait();
}

void Server::Stop(bool wait) {
    if (started_) {
        runner_->stop();
        if(wait)
			runner_->wait();
    }
}

Server::~Server() {
    Stop();
}