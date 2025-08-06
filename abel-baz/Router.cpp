#include "Router.hpp"
#include "sys/stat.h"
#include "unistd.h"
#include "../ysahraou/utils.hpp"

std::string RoutingResult::getUploadFile() const
{
    return (location->upload_dir);
}

std::string RoutingResult::getScriptFilename() const
{
    return (file_path);
}

std::string RoutingResult::getDocumentRoot() const
{
    if (server && !server->locations.empty())
        return server->locations[0].root;
    return "";
}

std::string RoutingResult::getServerName() const
{
    if (server && !server->server_name.empty())
        return server->server_name[0];
    return "localhost";
}
std::vector<std::string> RoutingResult::getExtension() const
{
    if (!location->cgi_extension.empty()) {
        return location->cgi_extension;
    }
    return std::vector<std::string>();
}




// DO: Match a server block based on host and port
// RETURN: the first server block that matches the port, or the first server block matches the host if no port match is found
const ServerConfig& matchServer(const Config& config, const std::string& host,
        int port, errorType& error, const std::string& server_ip) {
    const static ServerConfig emptyServer;
    const ServerConfig* fallback = NULL;

    for (size_t i = 0; i < config.servers.size(); ++i)
    {
        const ServerConfig& server = config.servers[i];

        for (size_t j = 0; j < server.listens.size(); ++j)
        {
            if (server.listens[j].listen_port == port
                && server.listens[j].listen_host == server_ip)
            {

                if (!fallback)
                {
                    error = NO_ERROR;
                    fallback = &server;
                }

                for (size_t k = 0; k < server.server_name.size(); ++k)
                {
                    if (server.server_name[k] == host)
                    {
                        error = NO_ERROR;
                        return server;
                    }
                }
            }
        }
    }
    if (fallback)
        return *fallback;
    else
    {
        error = SERVER_NOT_FOUND;
        return emptyServer;
    }
}

// DO: This function matches the longest location path for a given URI in a server block.
// RETURN: the location block that matches the URI
const LocationConfig& matchLocation(const ServerConfig& server, const std::string& uri, errorType& error) {
    
    static const LocationConfig emptyLocation;
    const LocationConfig *match = NULL;
    size_t longest = 0;

    for (size_t i = 0; i < server.locations.size(); ++i)
    {
        const LocationConfig& loc = server.locations[i];
        const std::string& path = loc.path;

        if (uri.compare(0, path.size(), path) == 0)
        {
            if (path == "/" || uri.size() == path.size() || uri[path.size()] == '/')
            {
                if (path.size() > longest)
                {
                    match = &loc;
                    longest = path.size();
                }
            }
        }
    }

    if (!match)
    {
        error = LOCATION_NOT_FOUND;
        return emptyLocation;
    }

    return *match;
}


// DO: This function gives you the physical file path on disk based on the config and URI.
// RETURN: root + (uri - location.path)
std::string finalPath(const LocationConfig& location, const std::string& uri) {
    const std::string& root = location.root;
    const std::string& locPath = location.path;
    std::string remain = uri.substr(locPath.length());

    if (!remain.empty() && root[root.size() - 1] == '/' && remain[0] == '/')
        remain = remain.substr(1);
    else if (!remain.empty() && root[root.size() - 1] != '/' && remain[0] != '/')
        remain = "/" + remain;
    return root + remain;
}



// Server looks for: /www/docs/index.html
// If it doesn’t exist, but autoindex is on → generate a listing
// If it doesn’t exist and autoindex is off → return 404

// Check if a path is a directory
bool isDirectory(const std::string& path) {
    // data type for file status
    struct stat s;
    // if path exists and s filled ir return 0
    // S_ISDIR checks if the file is a directory through .st_mode member
        // and return true if it is a directory
    //st_mode Field in struct stat that encodes type and permissions
    return (stat(path.c_str(), &s) == 0 && S_ISDIR(s.st_mode));
}

// Check if a file exists
bool fileExists(const std::string& path) {
    struct stat s;
    return (stat(path.c_str(), &s) == 0);
}

// DO: This function routes a request based on the configuration, host, port, and URI.
// RETURN: a RoutingResult containing the matched server, location, file path, and redirection
// 📌 Summary :
    // we have many cases like:
    // 1. if the location has a redirection, we return the redirection URL
    // 2. if the location is a directory and has an index file, we return the index file path after checks
    // 3. if the location is a directory and has autoindex enabled, we return the directory path and set use_autoindex to true
    // 4. if the location is a file, we check if it exists and is accessible, then return the file path
RoutingResult routingResult(const Config& config, const std::string& host,
    int port, const std::string& uri, const std::string& method, errorType& error, const std::string& server_ip) {
        std::cout << "before matching server" << std::endl;
        const ServerConfig& server = matchServer(config, host, port, error, server_ip);
        std::cout << "after matching server" << std::endl;
        const LocationConfig& location = matchLocation(server, uri, error);
        if (error != NO_ERROR)
        {
            return RoutingResult(); // Return an empty RoutingResult on error
        }
        
        RoutingResult result;
        result.server = &server;
        result.location = &location;
        result.server_count = config.servers.size();
    result.use_autoindex = false;
    
    if (!isMethodAllowed(location, method))
    {
        error = METHOD_NOT_ALLOWED;
        print_log("Method not allowed for path: " + uri, DiSPLAY_LOG);
    }
    if (!location.redirection.empty())
    {
        result.is_redirect = true;
        result.redirect_url = location.redirection;
        result.use_autoindex = false;
    }
    else
    {
        result.file_path = finalPath(location, uri);
        result.is_redirect = false;
        // std::cout << "is dir ==> " << isDirectory(result.file_path) << std::endl;
        if (isDirectory(result.file_path))
        {
            // TODO : remove this and always check if the file exists
            std::string index_path;
            if (!location.index.empty())
                index_path = result.file_path + "/" + location.index;
            else
                index_path = result.file_path + "/index.html";

            if (fileExists(index_path))
            {
                if (access(index_path.c_str(), R_OK) != 0)
                    error = ACCESS_DENIED;
                result.use_autoindex = false;
                result.file_path = index_path;
                return result; // We're done
            }


            // Either index was empty or the index file was missing
            // it will never reach here cuz index always exists
            if (location.autoindex)
            {
                result.use_autoindex = true;
            }
            else
            {
                error = NO_INDEX_FILE;
            }
        }
        // if the file does not exist here that means that's ur prblm you provided the wrong path
        else
        {
            result.use_autoindex = false;
            result.is_directory = false; // It's a file, not a directory
            // std::cout << "result.file_path ===> " << result.file_path << std::endl;
            if (!fileExists(result.file_path)){
                error = FILE_NOT_FOUND;
            }
            if (access(result.file_path.c_str(), R_OK) != 0)
            {
                if (error != FILE_NOT_FOUND)
                    error = ACCESS_DENIED;
            }
        }
    }


    return result;
}

bool isMethodAllowed(const LocationConfig& location, const std::string& method) {
    for (size_t i = 0; i < location.methods.size(); ++i){
        if (location.methods[i] == method)
            return true;
    }
    return false;
}