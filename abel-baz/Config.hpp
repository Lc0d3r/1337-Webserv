#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <map>

struct LocationConfig
{
    std::string path;
    std::string root;
    std::string index;
    std::vector<std::string> methods;
    bool autoindex;
    std::string upload_dir;
    std::string redirection;
    std::vector<std::string> cgi_extension;
    bool empty;

    LocationConfig() : autoindex(false), empty(true) {}
};

struct HostPort
{
    std::string listen_host;
    int listen_port;

    HostPort() : listen_host("127.0.0.1"), listen_port(8080) {}
};

struct ServerConfig
{
    std::vector<HostPort> listens;
    std::vector<std::string> server_name;
    std::map<int, std::string> error_pages;
    std::vector<LocationConfig> locations;
    size_t max_body_size;
    int keep_alive_timeout;

    ServerConfig() : max_body_size(1000000), keep_alive_timeout(10) {}
};

// Holds the full parsed config file
struct Config
{
    std::vector<ServerConfig> servers;

    int getKeepAliveTimeout(std::string host, int port, const std::string& server_ip) const;
    size_t getMaxBodySize(std::string host, int port, const std::string& server_ip) const;
    std::map<int, std::string> getErrorPages(std::string host, int port, const std::string& server_ip) const;
};
