#include "Parser.hpp"
#include <unistd.h>

void Parser::parseListen(ServerConfig& server) {
    Token val = get();
    if (val.type != VALUE)
        throw std::runtime_error("Expected value for listen directive");

    size_t colon = val.text.find(':');
    if (colon == std::string::npos)
        throw std::runtime_error("Invalid listen format. Use IP:PORT");

    HostPort hp;
    hp.listen_host = val.text.substr(0, colon);
    hp.listen_port = std::atoi(val.text.substr(colon + 1).c_str());

    if (hp.listen_port < 1 || hp.listen_port > 65535)  // max port number
        throw std::runtime_error("Port must be between 1 and 65535");

    if (hp.listen_host.empty() || hp.listen_port == 0)
        throw std::runtime_error("Listen host and port cannot be empty");

    server.listens.push_back(hp);
    
    if (get().type != SEMICOLON)
        throw std::runtime_error("Expected ';' after listen");
}

void Parser::parseServerName(ServerConfig& server) {
    Token val = get();
    if (val.type != VALUE)
        throw std::runtime_error("Expected value for server_name directive");

    if (val.text.empty())
        throw std::runtime_error("Server name cannot be empty");

    server.server_name.push_back(val.text);
    
    if (get().type != SEMICOLON)
        throw std::runtime_error("Expected ';' after server_name");
}

void Parser::parseLocation(ServerConfig& server) {
    
    LocationConfig loc;
    Token path = get();

    if (path.type != VALUE)
        throw std::runtime_error("Expected value for location path");

    loc.path = path.text;
    
    if (get().type != BRACE_OPEN)
    throw std::runtime_error("Expected '{' after location path");

    bool root_seen = false;
    bool index_seen = false;
    bool autoindex_seen = false;
    bool methods_seen = false;
    bool upload_seen = false;
    bool redirect_seen = false;
    
    while (peek().type != BRACE_CLOSE && peek().type != END_OF_FILE) {
        Token lkey = get();
        
        if (lkey.text == "root") {
            if (root_seen)
                throw std::runtime_error("Duplicate root directive in location block");
            root_seen = true;
            parseLocationRoot(loc);
        } else if (lkey.text == "index") {
            if (index_seen)
                throw std::runtime_error("Duplicate index directive in location block");
            index_seen = true;
            parseLocationIndex(loc);
        } else if (lkey.text == "autoindex") {
            if (autoindex_seen)
                throw std::runtime_error("Duplicate autoindex directive in location block");
            autoindex_seen = true;
            parseLocationAutoindex(loc);
        } else if (lkey.text == "methods") {
            if (methods_seen)
                throw std::runtime_error("Duplicate methods directive in location block");
            methods_seen = true;
            parseLocationMethods(loc);
        } else if (lkey.text == "upload_dir") {
            if (upload_seen)
                throw std::runtime_error("Duplicate upload_dir directive in location block");
            upload_seen = true;
            parseLocationUpload(loc);
        } else if (lkey.text == "redirection") {
            if (redirect_seen)
                throw std::runtime_error("Duplicate redirection directive in location block");
            redirect_seen = true;
            parseLocationRedirect(loc);
        } else if (lkey.text == "cgi_extension") {
            parseLocationCGI(loc);
        } else {
            throw std::runtime_error("Unknown location directive: " + lkey.text);
        }
}

    if (get().type != BRACE_CLOSE)
        throw std::runtime_error("Expected '}' at end of location block");

    server.locations.push_back(loc);
}

void Parser::parseErrorPage(ServerConfig& server) {
    Token codeToken = get();
    if (codeToken.type != VALUE)
        throw std::runtime_error("Expected status code for error_page");

    int code = std::atoi(codeToken.text.c_str());
    if (code < 300 || code > 599)
        throw std::runtime_error("Invalid status code for error_page: " + codeToken.text);

    // Step 2: Get the file path
    Token fileToken = get();
    if (fileToken.type != VALUE)
        throw std::runtime_error("Expected path for error_page");

    if (fileToken.text.empty())
        throw std::runtime_error("Error page path cannot be empty");
    if (fileToken.text[0] != '/')
    {
        char buffer[1024];
        if (!getcwd(buffer, sizeof(buffer)))
            throw std::runtime_error("Failed to get current working directory for error_page path");
        fileToken.text = std::string(buffer) + "/" + fileToken.text; // absolute path
    }

    // Step 3: Store in map
    server.error_pages[code] = fileToken.text;

    if (get().type != SEMICOLON)
        throw std::runtime_error("Expected ';' after error_page");
}

void Parser::parseMaxBodySize(ServerConfig& server) {
    Token val = get();
    if (val.type != VALUE)
        throw std::runtime_error("Expected value for client_max_body_size");

    if (val.text.empty())
        throw std::runtime_error("client_max_body_size cannot be empty");

    for (size_t i = 0; i < val.text.length(); ++i) {
        if (!std::isdigit(val.text[i]))
            throw std::runtime_error("client_max_body_size must be a positive number");
    }

    long long size = std::atoll(val.text.c_str());
    if (size < 0 || size > 1073741824) // negative or more than 1GB
        throw std::runtime_error("client_max_body_size must not be negative or more than 1GB");

    server.max_body_size = static_cast<size_t>(size);

    if (get().type != SEMICOLON)
        throw std::runtime_error("Expected ';' after client_max_body_size");
}

void Parser::parseKeepAlive(ServerConfig& server) {
    Token val = get();
    if (val.type != VALUE)
        throw std::runtime_error("Expected value for keep_alive_timeout");

    if (val.text.empty())
        throw std::runtime_error("keep_alive_timeout cannot be empty");

    for (size_t i = 0; i < val.text.length(); ++i) {
        if (!std::isdigit(val.text[i]))
            throw std::runtime_error("keep_alive_timeout must be a positive number");
    }

    int timeout = std::atoi(val.text.c_str());
    if (timeout < 0 || timeout > 3600) // more than 1 hour
        throw std::runtime_error("keep_alive_timeout must not be negative or more than 1 hour");

    server.keep_alive_timeout = timeout;

    if (get().type != SEMICOLON)
        throw std::runtime_error("Expected ';' after keep_alive_timeout");
}
