#pragma once
#include "Server.h"
#include <map>

namespace QrFileTransfer {

/**
 * @brief Connects a ConnectionTimeoutController to a restinio server
 * Implements Traits::connection_state_listener_t, can be used to pass a state listener to restinio.
 * It is used to allow a ConnectionTimeoutController to interact with restinio
 */
class ConnectionListener {
   public:
    ConnectionListener(std::shared_ptr<ConnectionTimeoutController> c) {
        assert(c);
        timeout_controller_ = c;
    }
    void state_changed(const restinio::connection_state::notice_t &notice) noexcept {
        std::lock_guard<std::mutex> l{m_};
        using namespace restinio::connection_state;
        const auto cause = notice.cause();
        if (timeout_controller_ && restinio::holds_alternative<closed_t>(cause))
            timeout_controller_->UnprotectConnection(notice.connection_id());
        if (server_ && server_->IsShuttingDown()) {
            server_->GetLogger().info("Closing the server");
            if (restinio::holds_alternative<closed_t>(cause))
                server_->Stop(false);
        }
    }

    void set_server(QrFileTransfer::Server *s) {
        server_ = s;
    }

   private:
    std::mutex m_;
    Server *server_;
    std::shared_ptr<ConnectionTimeoutController> timeout_controller_;
};

class ConnectionTimeoutManagerChecker {
   public:
    ConnectionTimeoutManagerChecker(
        restinio::asio_ns::io_context &io_context,
        std::chrono::steady_clock::duration check_period,
        std::shared_ptr<ConnectionTimeoutController> c) noexcept
        : operation_timer_{io_context}, check_period_{check_period} {
        assert(c);
        timeout_controller_ = c;
    }

    void schedule(restinio::tcp_connection_ctx_weak_handle_t weak_handle);

    void cancel() noexcept {
        restinio::utils::suppress_exceptions_quietly(
            [this] { operation_timer_.cancel(); });
    }

    void setConnectionTimeoutController(std::shared_ptr<ConnectionTimeoutController> c) { timeout_controller_ = c; }

   private:
    restinio::asio_ns::steady_timer operation_timer_;
    const std::chrono::steady_clock::duration check_period_;
    std::shared_ptr<ConnectionTimeoutController> timeout_controller_;
};

class ConnectionTimeoutManagerFactory;

//! This class is just a container
class ConnectionTimeoutManager : public std::enable_shared_from_this<ConnectionTimeoutManager> {
   public:
    //! Perform the actual check
    using timer_guard_t = QrFileTransfer::ConnectionTimeoutManagerChecker;
    //! Create a ConnectionTimeoutManager from an restinio::asio_ns::io_context
    using factory_t = QrFileTransfer::ConnectionTimeoutManagerFactory;

    ConnectionTimeoutManager(restinio::asio_ns::io_context &io_context, std::chrono::steady_clock::duration check_period, std::shared_ptr<ConnectionTimeoutController> c)
        : io_context_{io_context},
          check_period_{check_period} {
        assert(c);
        timeout_controller_ = c;
    }
    void start() const noexcept {}
    void stop() const noexcept {}
    //! Create guard for connection.
    timer_guard_t create_timer_guard() const {
        return timer_guard_t{io_context_, check_period_, timeout_controller_};
    }

   private:
    //! An instance of io_context to work with.
    restinio::asio_ns::io_context &io_context_;
    //! Check period for timer events.
    std::chrono::steady_clock::duration check_period_;
    std::shared_ptr<ConnectionTimeoutController> timeout_controller_;
};

class ConnectionTimeoutManagerFactory {
   public:
    const std::chrono::steady_clock::duration m_check_period{std::chrono::seconds{1}};
    static std::shared_ptr<ConnectionTimeoutController> timeout_controller_;

    auto create(restinio::asio_ns::io_context &io_context) const {
        assert(timeout_controller_);
        return std::make_shared<QrFileTransfer::ConnectionTimeoutManager>(io_context, m_check_period, timeout_controller_);
    }

    static void SetTimeoutController(std::shared_ptr<ConnectionTimeoutController> c) {
        timeout_controller_ = c;        
	}
};
}  // namespace QrFileTransfer