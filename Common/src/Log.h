#pragma once

#include <spdlog/spdlog.h>
#include <memory>


// logging class making use of spdlog for debug info/warnings/errors
class Log
{
public:
	static void Init();

	static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; }

private:
	static std::shared_ptr<spdlog::logger> s_Logger;
};

// logging macros for easy use in the codebase
#define LOG_ERROR(...)     ::Log::GetLogger()->error(__VA_ARGS__)
#define LOG_WARN(...)      ::Log::GetLogger()->warn(__VA_ARGS__)
#define LOG_INFO(...)      ::Log::GetLogger()->info(__VA_ARGS__)


// only trace and check errors in debug configuration
#ifdef _DEBUG
	#define LOG_TRACE(...)     ::Log::GetLogger()->trace(__VA_ARGS__)

	#define CHECK_ERROR(condition, ...)	if (!(condition)) ::Log::GetLogger()->error(__VA_ARGS__)
#else
	#define LOG_TRACE(...)     

	#define CHECK_ERROR(condition, ...)	condition
#endif
