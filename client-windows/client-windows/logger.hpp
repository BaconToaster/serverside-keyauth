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
        std::cout << "[INFO] ";
        std::printf(format, args...);
        std::cout << "\n";
    }

    template<typename ... t_args>
    const void warning(const char* format, t_args ... args) const {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::cout << std::put_time(&tm, "[%H:%M:%S] ");
        std::cout << "[WARNING] ";
        std::printf(format, args...);
        std::cout << "\n";
    }

    template<typename ... t_args>
    const void error(const char* format, t_args ... args) const {
        auto t = std::time(nullptr);
        auto tm = *std::localtime(&t);
        std::cout << std::put_time(&tm, "[%H:%M:%S] ");
        std::cout << "[ERROR] ";
        std::printf(format, args...);
        std::cout << "\n";
    }
};

static const Logger* const logger = new Logger();