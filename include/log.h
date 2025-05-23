#pragma once

#include <colorful.hpp>
#include <sstream>

void log(ConsoleColor color, const std::string &message);

template <class... Args>
std::string sprint(Args... args)
{
    std::ostringstream oss;
    (oss << ... << args);
    return oss.str();
}

#ifdef DEBUG
#define LOG_DEBUG(...) log(ConsoleColor::Blue, sprint(__FILE__, ":", __LINE__, " : ", __VA_ARGS__))
#else
#define LOG_DEBUG(...)
#endif
#define LOG_INFO(...) log(ConsoleColor::White, sprint(__VA_ARGS__))
#define LOG_SUCCESS(...) log(ConsoleColor::Green, sprint(__VA_ARGS__))
#define LOG_WARN(...) log(ConsoleColor::Yellow, sprint(__VA_ARGS__))
#define LOG_ERROR(...) log(ConsoleColor::Red, sprint(__VA_ARGS__))