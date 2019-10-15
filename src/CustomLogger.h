#pragma once
#include <restinio/all.hpp>

namespace QrFileTransfer {

enum class LogLevel { None,
                      Error,
                      Warning,
                      Info,
                      Trace
};

/**
 * @brief A template-free wrapper of restinio::ostream_logger_t<std::mutex>.
 * Allows multiple logger to write to the same logging channel with different level of verbosity
 */
class CustomLogger {
   public:
    CustomLogger() { setup(); };
    CustomLogger(const LogLevel &ll) { setup(ll); };
    CustomLogger(const LogLevel &ll, const std::shared_ptr<restinio::ostream_logger_t<std::mutex>> logger) { setup(ll, logger); };

    LogLevel GetLogLevel() { return log_level_; }
    void SetLogLevel(const LogLevel &ll) {
        log_level_ = ll;
    }

    std::shared_ptr<restinio::ostream_logger_t<std::mutex>> GetRawLogger() { return logger_; }

    void trace(const std::function<std::string(void)> msg_builder) {
        if (log_level_ >= LogLevel::Trace)
            logger_->trace(msg_builder);
    }

    void info(const std::function<std::string(void)> msg_builder) {
        if (log_level_ >= LogLevel::Info)
            logger_->info(msg_builder);
    }

    void warn(const std::function<std::string(void)> msg_builder) {
        if (log_level_ >= LogLevel::Warning)
            logger_->warn(msg_builder);
    }

    void error(const std::function<std::string(void)> msg_builder) {
        if (log_level_ >= LogLevel::Error)
            logger_->error(msg_builder);
    }

    void trace(const std::string &msg) {
        logger_->trace([&] { return msg; });
    }

    void info(std::string msg) {
        logger_->info([&] { return msg; });
    }

    void warn(const std::string &msg) {
        logger_->warn([&] { return msg; });
    }

    void error(const std::string &msg) {
        logger_->error([&] { return msg; });
    }

   private:
    void setup(const LogLevel &ll = LogLevel::Warning, std::shared_ptr<restinio::ostream_logger_t<std::mutex>> l = nullptr) {
        log_level_ = ll;
        if (l)
            logger_ = l;
        else
            logger_ = std::make_shared<restinio::ostream_logger_t<std::mutex>>();
    }
    std::shared_ptr<restinio::ostream_logger_t<std::mutex>> logger_;
    LogLevel log_level_;
};
}  // namespace QrFileTransfer