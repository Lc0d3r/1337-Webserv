#include "Config.hpp"
#include "Router.hpp"


int Config::getKeepAliveTimeout(std::string host, int port) const {
    errorType error = NO_ERROR;
    ServerConfig server = matchServer(*this, host, port, error);
    if (error != NO_ERROR) {
        std::cerr << "Error occurred: " << error << std::endl;
        return 0; // Meaningful value indicating error
    }
    if (server.keep_alive_timeout > 0) {
        return server.keep_alive_timeout;
    }
    return 10; // default keep-alive timeout
}

size_t Config::getMaxBodySize(std::string host, int port) const {
    errorType error = NO_ERROR;
    ServerConfig server = matchServer(*this, host, port, error);
    if (error != NO_ERROR) {
        std::cerr << "Error occurred: " << error << std::endl;
        return 0; // Meaningful value indicating error
    }
    if (server.max_body_size > 0) {
        return server.max_body_size;
    }
    return 1000000; // default max body size is 1 MB
}

std::map<int, std::string> Config::getErrorPages(int code, std::string host, int port) const {
    errorType error = NO_ERROR;
    ServerConfig server = matchServer(*this, host, port, error);
    if (error != NO_ERROR) {
        std::cerr << "Error occurred: " << error << std::endl;
        return std::map<int, std::string>(); // Return empty map on error
    }
    if (server.error_pages.count(code)) {
        return server.error_pages;
    }
    return std::map<int, std::string>(); // Return empty map if no error page is found
}
