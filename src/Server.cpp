#include "Server.h"
#include "ConnectionListener.h"
#include "htmldata.hpp"

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
        sf.offset_and_size(m_data_offset, m_data_size);

        return init_resp(request->create_response())
            .append_header_date_field()
            .append_header(restinio::http_field::content_type, "application / octet - stream")
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

/**
 * @brief Handle a file upload request
 * 
 * This is a very dirty implementation. There is not much point in optimizing it here as this should be handled 
 * on a lower level by restinio. 
 * 
 * @param file_content the whole request as a string
 * @param file_path the path where the file will be saved
 * @return int 
 */
int poor_man_file_writer(const std::string &file_content, std::string &file_path) {
    std::string file_terminator;
    std::stringstream sstream{file_content, std::ios::binary | std::ios::in};
	std::getline(sstream, file_terminator,'\r');
    std::getline(sstream, file_path, '\r');
    std::smatch match;
    static const std::regex rgx("filename=\\\"(\\w+.\\w+)\\\"");
    if (std::regex_search(file_path, match, rgx) && match.size() > 1) {
        file_path = match.str(1);
    } else {
        return -1;
    };
    std::string line_buffer;
    std::ofstream output;
    std::getline(sstream, line_buffer, '\r');
    std::getline(sstream, line_buffer, '\r');
    const int offset_start = 1 + sstream.tellg();
    const auto it = std::search(file_content.begin() + offset_start, file_content.end(), file_terminator.c_str(), file_terminator.data() + file_terminator.length());
    const int offset_end = std::distance(file_content.begin(), it);
    output.open(file_path, std::ios::binary | std::ios::out);   
	output << file_content.substr(offset_start, offset_end - offset_start - 2);
    output.close();
    return 1;
}

Server::router *Server::make_router(const std::string &served_path, const std::string &randomized_path) {
    auto *r = new router();
    std::string path;
    if (randomized_path.size() > 0)
        path.append(randomized_path);
    else
        path = std::experimental::filesystem::path(served_path).filename().u8string();
    r->http_get(
        "/",
        [](auto req, auto) {
            init_resp(req->create_response())
                .append_header(restinio::http_field::content_type, "text/plain; charset=utf-8")
                .set_body("This is qr-filetransfer_cpp")
                .done();

            return restinio::request_accepted();
        });
    if (allow_upload_) {
        fmt::printf("%s:%d : Upload allowed on %s \n", __FILE__, __LINE__, path);
        static std::string static_html = fmt::format(html, randomized_path);
        r->http_get(
            "/" + path,
            [&](auto req, auto) {
                init_resp(req->create_response())
                    .append_header(restinio::http_field::content_type, "text/html; charset=utf-8")
                    .set_body(static_html)
                    .done();

                return restinio::request_accepted();
            });
        r->http_post(
            "/upload/" + path,
            [&](restinio::request_handle_t req, restinio::router::route_params_t params) {
                const std::string ctype = req->header().get_field_or("Content-Type", "");
                if (ctype.find("multipart/form-data;") != 0)
                    return restinio::request_rejected();
                std::string saved_path;
                if (poor_man_file_writer(req->body(), saved_path) < 0) {
                    return restinio::request_rejected();
                };
                init_resp(req->create_response())
                    .append_header(restinio::http_field::content_type, "text/plain; charset=utf-8")
                    .set_body("File transferred")
                    .done();
                std::cout << "File saved in: " << saved_path;
                if (!keep_alive_)
                    InitShutdown();
                return restinio::request_accepted();
            });
    } else {
        fmt::printf("%s:%d : %s -> %s", __FILE__, __LINE__, served_path, path);
        r->http_get(
            "/" + path,
            [&](auto req, auto) {
                auto res = sendfile(served_path, req);
                if (!keep_alive_)
                    InitShutdown();
                return res;
            });
    }
    return r;
}

Server::Server(const std::string &addr, unsigned short port, const std::string &served_path, const std::string &randomized_path, bool keep_alive, bool allow_upload) {
    keep_alive_ = keep_alive;
    allow_upload_ = allow_upload;
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
        if (wait)
            runner_->wait();
    }
}

Server::~Server() {
    Stop();
}