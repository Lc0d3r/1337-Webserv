#include "Parser.hpp"

void Parser::parseLocationRoot(LocationConfig& loc) {
    Token val = get();
    if (val.type != VALUE)
        throw std::runtime_error("Expected value for root directive");
    if (val.text.empty())
        throw std::runtime_error("Root path cannot be empty");
    if (val.text[0] != '/')
        throw std::runtime_error("Root path must start with '/'");
    
    loc.root = val.text;
    if (get().type != SEMICOLON)
        throw std::runtime_error("Expected ';' after root");
}

void Parser::parseLocationIndex(LocationConfig& loc) {
    Token val = get();
    if (val.type != VALUE)
        throw std::runtime_error("Expected value for index directive");
    if (val.text.empty())
        throw std::runtime_error("Index cannot be empty");
    if (val.text.find('/') != std::string::npos)
        throw std::runtime_error("Index cannot contain '/' character");
    if (val.text.find('.') == std::string::npos)
        throw std::runtime_error("Index must have an extension (e.g. index.html)");
    
    loc.index = val.text;

    if (get().type != SEMICOLON)
        throw std::runtime_error("Expected ';' after index");
}

void Parser::parseLocationAutoindex(LocationConfig& loc) {
    Token val = get();
    if (val.type != VALUE)
        throw std::runtime_error("Expected value for autoindex directive");
    if (val.text == "on") {
        loc.autoindex = true;
    } else if (val.text == "off") {
        loc.autoindex = false;
    } else {
        throw std::runtime_error("autoindex must be 'on' or 'off'");
    }
    
    if (get().type != SEMICOLON)
        throw std::runtime_error("Expected ';' after autoindex");
}

void Parser::parseLocationMethods(LocationConfig& loc) {

    std::set<std::string> methodDup_set;

    while (peek().type == VALUE) {
        Token method = get();
        if (method.text != "GET" && method.text != "POST" && method.text != "DELETE")
            throw std::runtime_error("Invalid method: " + method.text);
            
        if (!methodDup_set.insert(method.text).second)
            throw std::runtime_error("Duplicate method: " + method.text);
        loc.methods.push_back(method.text);
    }
    if (loc.methods.empty())
        throw std::runtime_error("At least one method must be specified");

    if (get().type != SEMICOLON)
        throw std::runtime_error("Expected ';' after methods");
}

void Parser::parseLocationUpload(LocationConfig& loc) {
    Token val = get();
    if (val.type != VALUE)
        throw std::runtime_error("Expected value for upload_dir");
    if (val.text.empty())
        throw std::runtime_error("upload_dir cannot be empty");
    if (val.text[0] != '/')
        throw std::runtime_error("upload_dir path must start with '/'");

    loc.upload_dir = val.text;

    if (get().type != SEMICOLON)
        throw std::runtime_error("Expected ';' after upload_dir");
}

void Parser::parseLocationRedirect(LocationConfig& loc) {
    if (get().type != EQUAL)
        throw std::runtime_error("Expected '=' after redirection");

    Token val = get();
    if (val.type != VALUE)
        throw std::runtime_error("Expected URL after '='");
    if (val.text.empty())
        throw std::runtime_error("Redirection URL cannot be empty");
    loc.redirection = val.text;

    if (get().type != SEMICOLON)
        throw std::runtime_error("Expected ';' after redirection");
}

void Parser::parseLocationCGI(LocationConfig& loc) {
    Token val = get();
    if (val.type != VALUE)
        throw std::runtime_error("Expected value for cgi_extension");
    if (val.text.empty())
        throw std::runtime_error("cgi_extension cannot be empty");
    if (val.text[0] != '.')
        throw std::runtime_error("cgi_extension must start with '.'");

    loc.cgi_extension = val.text;

    if (get().type != SEMICOLON)
        throw std::runtime_error("Expected ';' after cgi_extension");
}
