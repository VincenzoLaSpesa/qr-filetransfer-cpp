#include "Server.h"
#include "Connections.h"
#include "htmldata.hpp"
#include "CustomLogger.h"

#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <fstream>
#include <fmt/printf.h>
#include <restinio/helpers/file_upload.hpp>

std::shared_ptr<QrFileTransfer::ConnectionTimeoutController> QrFileTransfer::ConnectionTimeoutManagerFactory::timeout_controller_ = nullptr;

std::uint64_t filesize(const std::string &filename) {
    std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
    if (!in.good())
        return 0;
    return static_cast<std::uint64_t>(in.tellg());
}

template <typename RESP>
RESP init_resp(RESP resp) {
    resp.append_header("qr-filetransfer_cpp", "qr-filetransfer_cpp powered by RESTinio");
    resp.append_header_date_field()
        .append_header("Content-Type", "text/plain; charset=utf-8");

    return resp;
}

restinio::request_handling_status_t sendfile(const std::string &file_path, std::shared_ptr<restinio::request_t> request, float minimum_speed_kBs) {
    try {
        const auto file_size = filesize(file_path);
        if (file_size == 0)
            return restinio::request_rejected();
        const float timeout_seconds = (((float)file_size) / (1024 * minimum_speed_kBs));  // the time needed for downloading with an average speed of minimum_speed_kBs
        restinio::file_offset_t m_data_offset{0};
        restinio::file_size_t m_data_size{file_size};
        restinio::sendfile_t sf = restinio::sendfile(file_path);
        sf.offset_and_size(m_data_offset, m_data_size);
        sf.timelimit(std::chrono::milliseconds((unsigned long)(timeout_seconds * 1000)));

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

using namespace QrFileTransfer;

void store_file_to_disk(const std::string &file_path, restinio::string_view_t file_name, restinio::string_view_t raw_content) {
    std::ofstream dest_file;
    dest_file.exceptions(std::ofstream::failbit);
    dest_file.open(fmt::format("{}/{}", file_path, file_name), std::ios_base::out | std::ios_base::trunc | std::ios_base::binary);
    dest_file.write(raw_content.data(), raw_content.size());
    dest_file.close();
}

size_t file_writer(const std::string &file_content, const std::string &file_path) {
    std::string line_buffer;
    std::ofstream output;
    output.open(file_path, std::ios::binary | std::ios::out);
    output << file_content;
    output.close();
    return file_content.length();
}

bool QrFileTransfer::Server::File_save(const std::string &file_folder, const restinio::request_handle_t &req) {
    using namespace restinio::file_upload;

    const auto enumeration_result = enumerate_parts_with_files(
        *req,
        [&file_folder](const part_description_t &part) {
            if ("file" == part.name || "files" == part.name) {
                if (part.filename) {                    
					store_file_to_disk(file_folder, *part.filename, part.body);
                    return handling_result_t::stop_enumeration;
                }
            }
            return handling_result_t::terminate_enumeration;
        });
    if (!enumeration_result || 1u != *enumeration_result)
        return false;
    return true;
}


//! Contains any non trivial handler used by the router ( for avoiding huge lambdas )
struct Handlers {
    static restinio::request_handling_status_t http_post_upload(const restinio::request_handle_t &req, const restinio::router::route_params_t &params, Server *caller, bool keep_alive) {
		const std::string ctype = req->header().get_field_or("Content-Type", "");
        if (ctype.find("multipart/form-data;") != 0)
            return restinio::request_rejected();
        std::string saved_path="./"; //@TODO allowing the client to set the filename could be a bad idea
        if (Server::File_save(saved_path, req))
            return restinio::request_accepted();
        return restinio::request_rejected();
        
/*        if (caller->poor_man_file_writer(req->body(), saved_path, req->connection_id()) < 0) {
            return restinio::request_rejected();
        };
        init_resp(req->create_response())
            .append_header(restinio::http_field::content_type, "text/plain; charset=utf-8")
            .set_body("File transferred")
            .done();
        caller->GetLogger().info(fmt::sprintf("File saved in %s", saved_path));
        if (!keep_alive)
            caller->InitShutdown();
        return restinio::request_accepted();*/
    }

    static restinio::request_handling_status_t http_get_basic(const restinio::request_handle_t &req, const std::string &body, const std::string& content_type){
        init_resp(req->create_response())
            .append_header(restinio::http_field::content_type, content_type)
            .set_body(body)
            .done();

        return restinio::request_accepted();	
	};

    static restinio::request_handling_status_t http_get_file(const restinio::request_handle_t &req, Server *caller, bool keep_alive, const std::string &served_path) {
            auto res = sendfile(served_path, req, 100);
            caller->GetLogger().info(fmt::sprintf("%s served", served_path));
            if (!keep_alive)
                caller->InitShutdown();
            return res;
    };
};

Server::router *Server::make_router(const std::string &served_path, const std::string &randomized_path) {
    auto *r = new router();
    std::string path;
    if (randomized_path.size() > 0)
        path.append(randomized_path);
    else
        path = std::filesystem::path(served_path).filename().u8string();
    
	r->http_get("/", [](auto req, auto) { return Handlers::http_get_basic(req, "This is qr-filetransfer_cpp", "text/plain; charset=utf-8"); });
    if (allow_upload_) {
        fmt::printf("%s:%d : Upload allowed on %s \n", __FILE__, __LINE__, path);
        static std::string static_html = fmt::format(html, randomized_path);
        r->http_get("/" + path, [&](auto req, auto) { return Handlers::http_get_basic(req, static_html, "text/html; charset=utf-8"); });
        r->http_post("/upload/" + path,[&](auto req, auto params) { return Handlers::http_post_upload(req, params, this, keep_alive_); });
    } else {
        fmt::printf("%s:%d : %s -> %s", __FILE__, __LINE__, served_path, path);
        r->http_get("/" + path, [&](auto req, auto) {return Handlers::http_get_file(req, this, keep_alive_, served_path);});
    }
    return r;
}

Server::Server(const std::string &addr, unsigned short port, const std::string &served_path, const std::string &randomized_path, bool keep_alive, bool allow_upload, bool verbose) {
    keep_alive_ = keep_alive;
    allow_upload_ = allow_upload;
    connection_timeout_controller_.reset(new ConnectionTimeoutController());
    ConnectionTimeoutManagerFactory::SetTimeoutController(connection_timeout_controller_);
    std::unique_ptr<Server::router> r{make_router(served_path, randomized_path)};
    std::shared_ptr<ConnectionListener> connection_listener{new ConnectionListener(connection_timeout_controller_)};
    auto settings = restinio::server_settings_t<server_traits>{}
                        .port(port)
                        .address(addr)
                        .connection_state_listener(connection_listener)
                        .request_handler(std::move(r))
                        .concurrent_accepts_count(10)
                        .logger(verbose ? LogLevel::Trace : LogLevel::Info, logger_.GetRawLogger());
    restinio_server_.reset(new http_server{restinio::own_io_context(), std::move(settings)});
    runner_.reset(new restinio::on_pool_runner_t<http_server>{
        std::thread::hardware_concurrency(),
        *restinio_server_});
    runner_->start();
    connection_listener->set_server(this);
    started_ = WaitForStartup();
}

bool Server::WaitForStartup(int timeout_seconds) {
    using namespace std::chrono_literals;
    int waited = 0;
    while (waited < timeout_seconds) {
        if (restinio_server_ != nullptr && runner_ != nullptr && runner_->started())
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