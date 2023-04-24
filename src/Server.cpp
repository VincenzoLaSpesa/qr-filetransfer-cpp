#include "Server.h"
#include "htmldata.hpp"
#include "CustomLogger.h"

#include <iostream>
#include <filesystem>
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>
#include <fmt/printf.h>


namespace picohash
{
	#include "./picohash/picohash.h"
}

size_t filesize(const std::string &filename)
{
	std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
	if (! in.good())
		return 0;
	return static_cast<size_t>(in.tellg());
}

std::string replace_all(std::string str, const std::string &from, const std::string &to)
{
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
	return str;
}

void sanitize_string(std::string &str)
{
	replace_all(str, "/", "_");
	replace_all(str, "\\", "_");
	replace_all(str, "*", "_");
	replace_all(str, "?", "_");
	replace_all(str, "..", "_");
}

//! Contains any non trivial handler used by the routes ( for avoiding huge lambdas )
struct Handlers
{
	static bool http_post_upload(const httplib::Request &req, httplib::Response &res, const httplib::ContentReader &content_reader, QrFileTransfer::Server *caller)
	{
		using namespace httplib;
		picohash::picohash_ctx_t ctx;

		picohash::picohash_init_md5(&ctx);

		if (req.is_multipart_form_data())
		{
			// NOTE: `content_reader` is blocking until every form data field is read
			MultipartFormDataItems files;
			std::ofstream datafile;
			auto handler_result = content_reader(
			    [&](const MultipartFormData &file) {
				    auto str = file.filename;
				    sanitize_string(str);
				    if (! datafile.is_open())
					    datafile.open(str, std::ios_base::binary);
				    if (! datafile.is_open() || ! datafile.good())
					    return false;
				    return true;
			    },
			    [&](const char *data, size_t data_length) {
				    if (! datafile.is_open() || ! datafile.good())
					    return false;
				    picohash::picohash_update(&ctx, data, data_length);
				    datafile.write(data, data_length);
				    return true;
			    });

			if (datafile.good())
			{
				float mb = roundf(datafile.tellp() / (1024 * 1024 / 1000))/1000;
				std::stringstream ss{};
				uint8_t digest[16];
				picohash::picohash_final(&ctx, digest);
				ss << std::setfill('0') << std::setw(2) << std::hex;
				for (int i = 0; i < 16; i++)
					ss << std::setw(2) << (int) digest[i];
				const std::string message = fmt::format("Done,\n the file should be {0} bytes long, or {1} Mb\n md5 of file is {2}", datafile.tellp(), mb, ss.str());
				const std::string static_html = fmt::format(html_stub, message);
				res.set_content(static_html, "text/html; charset=utf-8");
				caller->GetLogger().Trace(message);
				datafile.close();
			}
			else
			{
				const std::string static_html = fmt::format(html_stub, "the upload failed");
				res.set_content(static_html, "text/html; charset=utf-8");
				res.status = 500;
				caller->GetLogger().Warn("An upload failed");
			}

			return handler_result;
		}
		else
		{
			res.status = 500;
			return false;
		}
		return true;
	};

	static void http_get_file(const httplib::Request &req, httplib::Response &res, QrFileTransfer::Server *caller, bool keep_alive, const std::string &served_path)
	{
		const size_t fsize = filesize(served_path);
		if (fsize == 0)
		{
			res.status = 404;
			return;
		}
		caller->PendingTaskAdd();

		std::shared_ptr<std::ifstream> data = std::make_shared<std::ifstream>(std::ifstream{});
		data->open(served_path, std::ifstream::binary);
		res.set_content_provider(
		    fsize, "application/octet-stream",
		    [=](size_t offset, size_t space_in_data_sink, httplib::DataSink &sink) {
			    std::vector<char> buffer(caller->kChunkSize, 0);

			    if (! data->is_open() && data->good())
			    {
				    return false;
			    }
			    if (! data->eof())
			    {
				    data->read(buffer.data(), std::min(space_in_data_sink, buffer.size()));
				    const std::streamsize dataSize = data->gcount();
				    sink.write(buffer.data(), dataSize);
			    }
			    else
			    {
				    sink.done();
				    data->close();
			    }
			    return true;
		    },
		    [caller](bool success) { 
				caller->PendingTaskRemove(); 
				caller->GetLogger().Trace("File downloaded");
			});

		if (! keep_alive)
			caller->InitShutdown();
	};
};

void QrFileTransfer::Server::setup_routes(const std::string &served_path, const std::string &randomized_path)
{
	using namespace httplib;
	std::string path;
	std::string css_path;
	if (use_bootstrap_)
		css_path = "<link href='https://cdn.jsdelivr.net/npm/bootstrap@5.2.0-beta1/dist/css/bootstrap.min.css' rel='stylesheet' "
		           "integrity='sha384-0evHe/X+R7YkIZDRvuzKMRqM+OrBnVFBL6DOitfPri4tjfHxaWutUpFmBp4vmVor' crossorigin='anonymous'>";
	else
		css_path = "<!-- bootstrap was disabled in the config -->";

	if (randomized_path.size() > 0)
		path.append(randomized_path);
	else
		path = std::filesystem::path(served_path).filename().u8string();

	server_.Get("/", [](const Request & /*req*/, Response &res) { res.set_content("This is qr-filetransfer_cpp", "text/plain; charset=utf-8"); });

	if (allow_upload_)
	{
		logger_.Info(fmt::sprintf("%s:%d : Upload allowed on %s \n", __FILE__, __LINE__, path));
		static std::string static_html = fmt::format(html, randomized_path, css_path);

		server_.Get("/" + path, [](const Request & /*req*/, Response &res) { res.set_content(static_html, "text/html; charset=utf-8"); });

		server_.Post("/upload/" + path, [&](const Request &req, Response &res, const ContentReader &content_reader) {
			Handlers::http_post_upload(req, res, content_reader, this);
			if (! keep_alive_)
				InitShutdown();
		});
	}
	else
	{
		logger_.Info(fmt::sprintf("%s:%d : %s -> %s", __FILE__, __LINE__, served_path, path));
		server_.Get("/" + path, [&](const Request &req, Response &res) { return Handlers::http_get_file(req, res, this, keep_alive_, served_path); });
	}
}

bool QrFileTransfer::Server::runner_main()
{
	const auto good = server_.listen(server_address_.c_str(), server_port_);
	if (!good)
		logger_.Error("The server didn't start");
	return good;
}

QrFileTransfer::Server::Server(const std::string &addr, unsigned short port, const std::string &served_path, const std::string &randomized_path, bool keep_alive, bool allow_upload,
                               bool verbose, bool use_bootstrap)
{
	keep_alive_ = keep_alive;
	allow_upload_ = allow_upload;
	server_address_ = addr;
	server_port_ = port;
	setup_routes(served_path, randomized_path);
	logger_.EnableLogToFile("serverlog.log");
}

bool QrFileTransfer::Server::WaitForStartup(int timeout_seconds)
{
	using namespace std::chrono_literals;
	int waited = 0;
	while (waited < timeout_seconds)
	{
		if (server_.is_valid() && server_.is_running())
			return true;
		std::this_thread::sleep_for(1s);
		waited++;
	}
	return false;
}

void QrFileTransfer::Server::Wait()
{
	if (started_)
		runner_.join();
}

bool QrFileTransfer::Server::Start(bool waitForStartup, bool WaitForExit)
{
	if (WaitForExit)
	{
		return runner_main();
	}
	else
	{
		runner_ = std::thread(&Server::runner_main, this);
		if (waitForStartup)
			started_ = WaitForStartup();
		return server_.is_running();
	}
}

void QrFileTransfer::Server::Stop(bool wait)
{
	if (started_)
	{
		server_.stop();
		started_ = false;
	}
	if (wait && runner_.joinable())
		runner_.join();
}

/**
	If you just stop the server inside a request handler the client will not get any data back. 
	It's better to stop accepting new connections and give some seconds to teh server to answer the pending ones
*/
void QrFileTransfer::Server::InitShutdown()
{
	if (! stopping_)
	{
		stopping_ = true;
		killer_ = std::thread([&]() {
			server_.set_pre_routing_handler([&](const auto &req, auto &res) {
				logger_.Info("Connection refused, the server is shutting down");
				return httplib::Server::HandlerResponse::Unhandled;
			});
			using namespace std::chrono_literals;
			int delay = 5;
			while (pending_ > 0)
			{				
				logger_.Trace(fmt::sprintf("Waiting for %d pending task(s)", pending_));
				std::this_thread::sleep_for(2s);
				delay -= 2;
			}
			if (delay > 0)
			{
				logger_.Trace("The server is not accepting new connections and it will shut down in 5 seconds.");
				std::this_thread::sleep_for(std::chrono::seconds(delay));
			}
			Stop(false);
		});
		logger_.Info("The server is shutting down.");
	}
}

QrFileTransfer::Server::~Server()
{
	Stop(true);
	if (killer_.joinable())
		killer_.join();
}