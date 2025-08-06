#include "Config.hpp"
#include "Router.hpp"


int Config::getKeepAliveTimeout(std::string host, int port, const std::string& server_ip) const {
    errorType error = NO_ERROR;
    ServerConfig server = matchServer(*this, host, port, error, server_ip);
    if (error != NO_ERROR) {
        std::cerr << "Error occurred: " << error << std::endl;
        return 0;
    }
    if (server.keep_alive_timeout > 0) {
        return server.keep_alive_timeout;
    }
    return 10;
}

size_t Config::getMaxBodySize(std::string host, int port, const std::string& server_ip) const {
    errorType error = NO_ERROR;
    ServerConfig server = matchServer(*this, host, port, error, server_ip);
    if (error != NO_ERROR) {
        std::cerr << "Error occurred: " << error << std::endl;
        return 0;
    }
    if (server.max_body_size > 0) {
        return server.max_body_size;
    }
    return 1000000;
}

std::map<int, std::string> Config::getErrorPages(std::string host, int port, const std::string& server_ip) const {
    errorType error = NO_ERROR;
    ServerConfig server = matchServer(*this, host, port, error, server_ip);
    if (error != NO_ERROR) {
        std::cerr << "Error occurred: " << error << std::endl;
        return std::map<int, std::string>();
    }
    return server.error_pages;
}
