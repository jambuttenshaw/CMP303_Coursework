#pragma once

#include <spdlog/spdlog.h>
#include <memory>


class Log
{
public:
	static void Init();

	static std::shared_ptr<spdlog::logger>& GetLogger() { return s_Logger; }

private:
	static std::shared_ptr<spdlog::logger> s_Logger;
};

#ifdef _DEBUG
	#define LOG_ERROR(...)     ::Log::GetLogger()->error(__VA_ARGS__)
	#define LOG_WARN(...)      ::Log::GetLogger()->warn(__VA_ARGS__)
	#define LOG_INFO(...)      ::Log::GetLogger()->info(__VA_ARGS__)
	#define LOG_TRACE(...)     ::Log::GetLogger()->trace(__VA_ARGS__)
	
	#define CHECK_ERROR(condition, ...)	if (!(condition)) ::Log::GetLogger()->error(__VA_ARGS__)
#else
	#define LOG_ERROR(...)     
	#define LOG_WARN(...)      
	#define LOG_INFO(...)      
	#define LOG_TRACE(...)     
	
	#define CHECK_ERROR(condition, ...)	condition
#endif
