#pragma once
#include <chrono>
#include <string>
#include <functional>
#include <mutex>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <cstring>
#include <ctime>

namespace {
int localtime_crossplatform( struct tm* const tmDest,  time_t const* const sourceTime)
{ //@TODO use some modern c++ stuff, not this horrible workaround
#ifdef _WIN32
	return localtime_s(tmDest, sourceTime);
#else
	//mock the windows one from posix
	localtime_r(sourceTime, tmDest);
	if(tmDest)
		return 0;
	return 22; 
#endif
}
}

enum class LogLevel
{
	None,
	Error,
	Warning,
	Info,
	Trace
};

class Logger
{
  public:
	Logger()
	{
		setup();
	};
  
	Logger(const LogLevel &ll, const char *filename = nullptr)
	{
		setup(ll);
		if (filename)
			EnableLogToFile(filename);
	};

	LogLevel GetLogLevel()
	{
		return _log_level;
	}

  void SetLogLevel(const LogLevel &ll)
	{
		_log_level = ll;
	}
	void Trace(const std::function<std::string(void)> msg_builder)
	{
		if (_log_level >= LogLevel::Trace)
			logText("TRACE", msg_builder(), true);
	}

	void Info(const std::function<std::string(void)> msg_builder)
	{
		if (_log_level >= LogLevel::Info)
			logText("INFO", msg_builder(), true);
	}

	void Warn(const std::function<std::string(void)> msg_builder)
	{
		if (_log_level >= LogLevel::Warning)
			logText("WARNING", msg_builder(), true);
	}

	void Error(const std::function<std::string(void)> msg_builder)
	{
		if (_log_level >= LogLevel::Error)
			logText("ERROR", msg_builder(), true);
	}

	void Trace(const std::string &msg)
	{
		logText("TRACE", msg, true);
	}

	void Info(std::string msg)
	{
		logText("INFO", msg, true);
	}

	void Warn(const std::string &msg)
	{
		logText("WARNING", msg, true);
	}

	void Error(const std::string &msg)
	{
		logText("ERROR", msg, true);
	}

	void Flush()
	{
		std::unique_lock<std::mutex> lock(_mutex);
		if (_fileStream.is_open() && _fileStream.good())
			_fileStream.flush();
	};

	~Logger()
	{
		if (_fileStream.is_open() && _fileStream.good())
			_fileStream.close();
	}

	bool EnableLogToFile(const char *filename)
	{
		if (_fileStream.is_open() && _fileStream.good())
			_fileStream.close();

		_fileStream.open(filename, std::ofstream::out | std::ofstream::app);
		if (_fileStream.bad())
			return false;
		_fileStream << std::setprecision(3);
		return true;
	}

  private:

  void logText(const char *header, const char *message, bool printTimestamp = false)
	{
		std::stringstream ss;
		ss << std::setprecision(3);
		if (printTimestamp)
			if (_relativeTimestamp)
			{
				const auto now = std::chrono::system_clock::now();
				const auto delta = now - _basetimestamp;
				const double elapsed_time_s = 0.001 * std::chrono::duration<double, std::milli>(delta).count();
				ss << elapsed_time_s << '\t';
			}
			else
			{
				const auto t = std::time(nullptr);
				tm localtime;
				localtime_crossplatform(&localtime, &t);
				ss << std::put_time(&localtime, "%FT%TZ") << '\t';
			}
		if (header && strlen(header) > 0)
			ss << header << '\t';
		if (message && strlen(message) > 0)
			ss << message << '\t';
		ss << std::endl;

		
		{
			std::unique_lock<std::mutex> lock(_mutex);

			std::cout << ss.str();
			if (_fileStream.is_open() && _fileStream.good())
				_fileStream << ss.str();
		}
	}
	void logText(const char *header, const std::string message, bool printTimestamp = false)
	{
		logText(header, message.c_str(), printTimestamp);
	}

	void setup(const LogLevel &ll = LogLevel::Warning, bool printRelativeTimestamp = false)
	{
		_log_level = ll;
		_basetimestamp = std::chrono::system_clock::now();
	}
	LogLevel _log_level;
	std::ofstream _fileStream;
	std::chrono::system_clock::time_point _basetimestamp;
	std::mutex _mutex;
	bool _relativeTimestamp = false;
};