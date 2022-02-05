#pragma once
#include "defs.hpp"

class Logger {
private:

public:
    template<typename ... t_args>
    const void info(const char* format, t_args ... args) const {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::cout << std::put_time(&tm, "[%H:%M:%S] ");
        std::cout << "\033[1;34m[i] ";
        std::printf(format, args...);
        std::cout << "\033[0m\n";
    }

    template<typename ... t_args>
    const void warning(const char* format, t_args ... args) const {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::cout << std::put_time(&tm, "[%H:%M:%S] ");
        std::cout << "\033[1;33m[!] ";
        std::printf(format, args...);
        std::cout << "\033[0m\n";
    }

    template<typename ... t_args>
    const void error(const char* format, t_args ... args) const {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::cout << std::put_time(&tm, "[%H:%M:%S] ");
        std::cout << "\033[1;31m[!] ";
        std::printf(format, args...);
        std::cout << "\033[0m\n";
    }
};

static const Logger* const logger = new Logger();