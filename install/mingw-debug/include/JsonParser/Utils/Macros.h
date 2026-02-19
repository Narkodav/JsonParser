#pragma once
#include <stdexcept>

namespace Json::Detail {

    static void verify(bool condition, const char* message) {
#ifndef NODEBUG
        if(!condition) throw std::runtime_error(message);
#else
        (void)condition; (void)message;
#endif
    }
}

#define JSON_VERIFY(condition, message) Json::Detail::verify(condition, message)