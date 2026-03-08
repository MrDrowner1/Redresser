#pragma once
#include <format>
#include <string>
#include "settings.h"

template<typename... Args>
void debug_output(std::format_string<Args...> fmt, Args&&... args){
    try {
        std::string message = std::format(fmt, std::forward<Args>(args)...);
        if(r_settings.debug)
            RE::ConsoleLog::GetSingleton()->Print(message.c_str());
    } catch (const std::format_error& e) {
        std::string error_msg = std::format("Format error: {}", e.what());
        RE::ConsoleLog::GetSingleton()->Print(error_msg.c_str());
    }
}