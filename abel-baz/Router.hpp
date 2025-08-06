#pragma once

#include "Parser.hpp"

struct RoutingResult
{
    const ServerConfig* server;
    size_t server_count;
    const LocationConfig* location;
    std::string file_path;
    bool is_redirect;
    std::string redirect_url;
    bool is_directory;
    bool use_autoindex;

    // getters
    std::vector<std::string>    getExtension() const;
    std::string                 getServerName() const;
    std::string                 getDocumentRoot() const;
    std::string                 getScriptFilename() const;
    std::string                    getUploadFile() const;
};


enum errorType {
    NO_ERROR = 0,
    SERVER_NOT_FOUND,
    LOCATION_NOT_FOUND,
    METHOD_NOT_ALLOWED,
    FILE_NOT_FOUND,
    ACCESS_DENIED,
    NO_INDEX_FILE,
    AUTOINDEX_DISABLED
};

const ServerConfig& matchServer(const Config& config, const std::string& host, int port, errorType& error,
        const std::string& server_ip);
const LocationConfig& matchLocation(const ServerConfig& server, const std::string& uri, errorType& error);
std::string finalPath(const LocationConfig& location, const std::string& uri);
RoutingResult routingResult(const Config& config, const std::string& host,
                        int port, const std::string& uri, const std::string& method, errorType& error, const std::string& server_ip);
bool isMethodAllowed(const LocationConfig& location, const std::string& method);
