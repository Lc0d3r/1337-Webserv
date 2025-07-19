#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>

#include "Config.hpp"
// TokenType: describes what kind of token we are dealing with
enum TokenType
{
    KEYWORD,        // Known keyword like "server", "listen", etc.
    VALUE,          // Any string that is a value (like "127.0.0.1:8080")
    BRACE_OPEN,     // {
    BRACE_CLOSE,    // }
    SEMICOLON,      // ;
    EQUAL,          // =
    END_OF_FILE     // Reached end of config
};

// Token: represents a single token with a type and actual text
struct Token
{
    TokenType type;
    std::string text;

    Token(TokenType tokenType, const std::string& tokenText)
        : type(tokenType), text(tokenText) {}
};

class Tokenizer
{
public:
    Tokenizer(const std::string& filePath);
    std::vector<Token> tokenize();

private:
    std::string _content;
};

