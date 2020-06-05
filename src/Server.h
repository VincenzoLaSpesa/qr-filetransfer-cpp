#pragma once
#include <restinio/all.hpp>
#include <string>
#include "CustomLogger.h"

namespace QrFileTransfer {
class ConnectionTimeoutManager;
class ConnectionListener;

///! Keeps a list of connection to protect from timeout
class ConnectionTimeoutController {
   public:
    ConnectionTimeoutController() {
        printf("Summoned");
    }
    int ProtectConnection(const uint64_t c_id, float extra_seconds) {
        std::lock_guard<std::mutex> lock(m_);
        delayed_connections_.insert({c_id, timeLimit(extra_seconds)});
        return (int)delayed_connections_.size();
    };
    int UnprotectConnection(const uint64_t c_id) {
        std::lock_guard<std::mutex> lock(m_);
        delayed_connections_.erase(c_id);
        return (int)delayed_connections_.size();
    };

    bool IsProtected(const uint64_t c_id) {
        std::lock_guard<std::mutex> lock(m_);
        return delayed_connections_.find(c_id) != delayed_connections_.end();
    }

    float GetRemainingSeconds(const uint64_t c_id) {
        std::lock_guard<std::mutex> lock(m_);
        const auto limit = delayed_connections_.find(c_id);
        if (limit != delayed_connections_.end()) {
            const auto remaining = limit->second - std::chrono::system_clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(remaining).count() / 1000.0f;
        }
        return -HUGE_VALF;
    }

   private:
    std::chrono::time_point<std::chrono::system_clock> timeLimit(float seconds) {
        std::chrono::time_point<std::chrono::system_clock> t = std::chrono::system_clock::now();
        return t + std::chrono::milliseconds((long long)(seconds * 1000));
    }
    std::mutex m_;
    std::map<uint64_t, std::chrono::time_point<std::chrono::system_clock>> delayed_connections_;
};

/**
 * @brief the Server, the only interface to restinio used in the application
 * Holds togheter a restinio::router, a http_server and all the accessory objects.
 */
class Server {
   public:
    Server(const std::string &addr, unsigned short port, const std::string &served_path, const std::string &virtual_path = "", bool keep_alive = false, bool allow_upload = false, bool verbose = false);
    bool WaitForStartup(int timeout_seconds = 5);
    void Wait();
    void Stop(bool wait = true);
    void InitShutdown() { stopping_ = true; }
    const bool IsShuttingDown() { return stopping_; }
    CustomLogger &GetLogger() { return logger_; };
    ~Server();

	/**
	 * @brief Handle a file upload request
	 * 
	 * This is a very dirty implementation. There is not much point in optimizing it here as this should be handled 
	 * on a lower level by restinio. 
	 * 
	 * @param file_content the whole request as a string
	 * @param file_path the path where the file will be saved
	 * @param minimum_speed_kBs the speed used for calculating the timeout
	 * @return int 
	 */
	[[deprecated("To be handled by restinio")]]    
	int poor_man_file_writer(const std::string &file_content, std::string &file_path, restinio::connection_id_t c_id, float minimum_speed_kBs = 10);
   private:
    using router = restinio::router::express_router_t<restinio::router::std_regex_engine_t>;

    struct server_traits : public restinio::traits_t<ConnectionTimeoutManager,
                                                     restinio::single_threaded_ostream_logger_t,
                                                     router> {
        using connection_state_listener_t = ConnectionListener;
        using logger_t = CustomLogger;
    };

    using http_server = restinio::http_server_t<server_traits>;

    router *make_router(const std::string &served_path, const std::string &virtual_path = "");
    

    bool file_save(const std::string &file_folder, const restinio::request_t &req);

    std::unique_ptr<http_server> restinio_server_;
    std::unique_ptr<restinio::on_pool_runner_t<http_server>> runner_;
    std::shared_ptr<ConnectionTimeoutController> connection_timeout_controller_;
    CustomLogger logger_;
    bool started_ = false;
    bool keep_alive_ = false;
    bool stopping_ = false;
    bool allow_upload_ = false;
};
}  // namespace QrFileTransfer
