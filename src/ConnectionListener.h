#pragma once
#include <restinio/all.hpp>
#include "Server.h"


class ConnectionListener {
   public:
    void state_changed(const restinio::connection_state::notice_t &notice) noexcept {
        std::lock_guard<std::mutex> l{m_};
        if (server_ && server_->IsShuttingDown()) {
            server_->GetLogger().info("Closing the server");
            using namespace restinio::connection_state;
            const auto cause = notice.cause();
            if (restinio::holds_alternative<closed_t>(cause))
                server_->Stop(false);
        }
    }

    void set_server(Server *s) {
        server_ = s;
	}

   private:
    std::mutex m_;
    Server *server_;
};