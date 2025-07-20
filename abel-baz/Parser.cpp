#include "Parser.hpp"
// #include <stdexcept>

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
// Parses the entire configuration file and returns a Config object

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
    
    while (peek().type != BRACE_CLOSE && peek().type != END_OF_FILE)
    {
        Token key = get();
        
        if (key.type != KEYWORD)
            throw std::runtime_error("Expected directive inside server block");
        
        if (key.text == "listen") {
            parseListen(server);
        } else if (key.text == "server_name") {
            parseServerName(server);
        } else if (key.text == "location") {
            parseLocation(server);
        } else if (key.text == "error_page") {
            parseErrorPage(server);
        } else if (key.text == "max_body_size") {
            parseMaxBodySize(server);
        } else {
            throw std::runtime_error("Unknown server directive: " + key.text);
        }
    }
    
    if (get().type != BRACE_CLOSE)
        throw std::runtime_error("Expected '}' at end of server block");
    
    config.servers.push_back(server);
}
