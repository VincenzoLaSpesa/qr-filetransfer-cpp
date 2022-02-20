#pragma once

enum class LogLevel
{
	None,
	Error,
	Warning,
	Info,
	Trace
};

/**
 * @brief A template-free wrapper of restinio::ostream_logger_t<std::mutex>.
 * Allows multiple logger to write to the same logging channel with different level of verbosity
 */
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
		return log_level_;
	}
	void SetLogLevel(const LogLevel &ll)
	{
		log_level_ = ll;
	}

	void Trace(const std::function<std::string(void)> msg_builder)
	{
		if (log_level_ >= LogLevel::Trace)
			logText("TRACE", msg_builder(), true);
	}

	void Info(const std::function<std::string(void)> msg_builder)
	{
		if (log_level_ >= LogLevel::Info)
			logText("INFO", msg_builder(), true);
	}

	void Warn(const std::function<std::string(void)> msg_builder)
	{
		if (log_level_ >= LogLevel::Warning)
			logText("WARNING", msg_builder(), true);
	}

	void Error(const std::function<std::string(void)> msg_builder)
	{
		if (log_level_ >= LogLevel::Error)
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
			ss << (float) (GetTickCount64() - _basetimestamp) / 1000.0f << " ";
		if (header && strlen(header) > 0)
			ss << header << '\t';
		if (message && strlen(message) > 0)
			ss << message << '\t';
		ss << std::endl;

		std::unique_lock<std::mutex> lock(_mutex);

		std::cout << ss.str();
		if (_fileStream.is_open() && _fileStream.good())
			_fileStream << ss.str();
	}
	void logText(const char *header, const std::string message, bool printTimestamp = false)
	{
		logText(header, message.c_str(), printTimestamp);
	}

	void setup(const LogLevel &ll = LogLevel::Warning)
	{
		log_level_ = ll;
		_basetimestamp = GetTickCount64();
	}
	LogLevel log_level_;
	std::ofstream _fileStream;
	uint64_t _basetimestamp;
	std::mutex _mutex;	
};