#pragma once
#include <string>
#include <httplib.h>
#include "CustomLogger.h"

namespace QrFileTransfer
{
	class ConnectionTimeoutManager;
	class ConnectionListener;

	/**
    * @brief the Server, holds an httplib::server inside a thread
    */
	class Server
	{
	  public:
		Server(const std::string &addr, unsigned short port, const std::string &served_path, const std::string &virtual_path = "", bool keep_alive = false,
		       bool allow_upload = false, bool verbose = false, bool use_bootstrap = false);
		bool WaitForStartup(int timeout_seconds = 5);
		void Wait();
		bool Start(bool waitForStartup = true, bool WaitForExit = false);
		void Stop(bool wait = true);
		void InitShutdown();
		int PendingTaskAdd()
		{
			return ++pending_;
		}
		int PendingTaskRemove()
		{
			return --pending_;
		}
		const bool IsShuttingDown()
		{
			return stopping_;
		}
		Logger &GetLogger()
		{
			return logger_;
		};
		~Server();

		const unsigned short kChunkSize = 1024;

	  private:
		void setup_routes(const std::string &served_path, const std::string &randomized_path);
		bool runner_main();
		httplib::Server server_;
		std::thread runner_;
		std::thread killer_;

		Logger logger_;
		bool started_ = false;
		bool keep_alive_ = false;
		bool stopping_ = false;
		bool allow_upload_ = false;
		bool use_bootstrap_ = false;
		std::atomic<int> pending_ = 0;
		std::string server_address_;
		unsigned short server_port_ = 8080;
	};
} // namespace QrFileTransfer
