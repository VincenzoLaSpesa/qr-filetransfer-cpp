#include "Connections.h"
#include "Server.h"

void QrFileTransfer::ConnectionTimeoutManagerChecker::schedule(restinio::tcp_connection_ctx_weak_handle_t weak_handle) {
	operation_timer_.expires_after(check_period_);
    operation_timer_.async_wait(
        [weak_handle = std::move(weak_handle), this](const auto &ec) {
            if (!ec) {
                if (auto h = weak_handle.lock()) {
                    assert(timeout_controller_);
					if (!timeout_controller_ || timeout_controller_->GetRemainingSeconds(h->connection_id()) < 0)
                        h->check_timeout(h);
                }
            }
        });
}
