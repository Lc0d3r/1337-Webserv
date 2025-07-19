#include "Parser.hpp"

struct RoutingResult
{
    const ServerConfig* server;
    size_t server_count;
    const LocationConfig* location;
    std::string file_path;
    bool is_redirect;
    std::string redirect_url;
    bool is_directory; // true if the final path is a directory
    bool use_autoindex; // true if autoindex is enabled for the location
};


const ServerConfig& matchServer(const Config& config, const std::string& host, int port);
const LocationConfig& matchLocation(const ServerConfig& server, const std::string& uri);
std::string finalPath(const LocationConfig& location, const std::string& uri);
RoutingResult routingResult(const Config& config, const std::string& host,
                        int port, const std::string& uri, const std::string& method);
bool isMethodAllowed(const LocationConfig& location, const std::string& method);
