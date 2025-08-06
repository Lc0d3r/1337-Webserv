#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cctype>

#include "Config.hpp"

enum TokenType
{
    KEYWORD,
    VALUE,
    BRACE_OPEN,
    BRACE_CLOSE,
    SEMICOLON,
    EQUAL,
    END_OF_FILE
};


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

