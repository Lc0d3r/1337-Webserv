#include "Parser.hpp"
#include "../ysahraou/utils.hpp"

Token Parser::peek() {
    if (_index < _tokens.size())
        return _tokens[_index];
    return Token(END_OF_FILE, "");
}

Token Parser::get() {
    if (_index < _tokens.size())
        return _tokens[_index++];
    return Token(END_OF_FILE, "");
}

//PARSING SECTION

Config Parser::parse() {
    Config config;
    
    while (peek().type != END_OF_FILE) {
        parseServer(config);
    }

    if (config.servers.empty())
        throw std::runtime_error("No server blocks found in configuration");
    
    return config;
}

void Parser::parseServer(Config& config) {
    Token t = get();
    if (t.type != KEYWORD || t.text != "server")
        throw std::runtime_error("Expected 'server' keyword");
    
    if (get().type != BRACE_OPEN)
        throw std::runtime_error("Expected '{' after server");
    
    ServerConfig server;
    bool max_body_size_set = false;
    bool keep_alive_timeout_set = false;
    bool server_name_set = false;
    
    while (peek().type != BRACE_CLOSE && peek().type != END_OF_FILE)
    {
        Token key = get();
        
        if (key.type != KEYWORD)
            throw std::runtime_error("Expected directive inside server block");
        
        if (key.text == "listen") {
            parseListen(server);
        } else if (key.text == "server_name") {
            if (server_name_set)
                throw std::runtime_error("Duplicate 'server_name' directive in server block");
            server_name_set = true;
            parseServerName(server);
        } else if (key.text == "location") {
            parseLocation(server);
        } else if (key.text == "error_page") {
            parseErrorPage(server);
        } else if (key.text == "max_body_size") {
            if (max_body_size_set)
                throw std::runtime_error("Duplicate 'max_body_size' directive in server block");
            max_body_size_set = true;
            parseMaxBodySize(server);
        } else if (key.text == "keep_alive_timeout") {
            if (keep_alive_timeout_set)
                throw std::runtime_error("Duplicate 'keep_alive_timeout' directive in server block");
            keep_alive_timeout_set = true;
            parseKeepAlive(server);
        } else {
            throw std::runtime_error("Unknown server directive: " + key.text);
        }
    }
    
    if (get().type != BRACE_CLOSE)
        throw std::runtime_error("Expected '}' at end of server block");
    if (server.listens.empty())
    {
        HostPort defaultHostPort;
        server.listens.push_back(defaultHostPort);
        print_log("No listen directive found, using default listen host and port: " + defaultHostPort.listen_host + ":" + intToString(defaultHostPort.listen_port), DiSPLAY_LOG);
    }
    config.servers.push_back(server);
}
