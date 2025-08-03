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

std::string Config::getErrorPage(int code, std::string host, int port) const {
    errorType error = NO_ERROR;
    ServerConfig server = matchServer(*this, host, port, error);
    if (error != NO_ERROR) {
        std::cerr << "Error occurred: " << error << std::endl;
        return ""; // Meaningful value indicating error
    }
    
    if (server.error_pages.find(code) != server.error_pages.end()) {
        return server.error_pages.at(code);
    }
    return ""; // No specific error page found for this code

}
