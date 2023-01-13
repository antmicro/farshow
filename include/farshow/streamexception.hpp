#pragma once

#include <cstring> //strerror
#include <iostream>
#include <memory>

namespace farshow
{

/**
 * Exception to handle errors from farshow
 */
class StreamException : public std::exception
{
public:
    /**
     * Constructor
     *
     * @param msg Exception description
     * @param error_code Linux error code (0 if not related)
     */
    StreamException(std::string msg, int error_code = 0) : error_code(error_code) { setMessage(msg); }

    /**
     * Returns the explanatory string.
     *
     * @return Message, which explains the error
     */
    const char *what() const throw() override { return msg.c_str(); }

private:
    /**
     * Combines description and error code
     * @param text Exception description
     */
    void setMessage(std::string text)
    {
        msg = (error_code != 0) ? (std::to_string(error_code) + " – " + strerror(error_code)) : "";
        msg = (text != "") ? (text + ": " + msg) : msg;
    }

    int error_code;  ///< linux error code (0 if not related)
    std::string msg; ///< description
};

}; // namespace farshow
