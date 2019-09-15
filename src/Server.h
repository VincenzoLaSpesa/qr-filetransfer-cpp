#pragma once
#include <restinio/all.hpp>
#include <string>

class ConnectionListener;

class Server {
   public:
    Server(const std::string &addr, unsigned short port, const std::string &served_path, const std::string &virtual_path = "", bool keep_alive = false, bool allow_upload=false);
    bool WaitForStartup(int timeout_seconds = 5);
    void Wait();
    void Stop(bool wait = true);
    void InitShutdown() { stopping_ = true; }
    const bool IsShuttingDown() { return stopping_; }
    ~Server();

   private:
    using router = restinio::router::express_router_t<restinio::router::std_regex_engine_t>;

    struct server_traits : public restinio::traits_t<
                               restinio::asio_timer_manager_t,
                               restinio::single_threaded_ostream_logger_t,
                               router> {
        using connection_state_listener_t = ConnectionListener;
    };

    using http_server = restinio::http_server_t<server_traits>;

    router *make_router(const std::string &served_path, const std::string &virtual_path = "");

    std::unique_ptr<http_server> server_;
    std::unique_ptr<restinio::on_pool_runner_t<http_server>> runner_;
    bool started_ = false;
    bool keep_alive_ = false;
    bool stopping_ = false;
    bool allow_upload_ = false;
};
