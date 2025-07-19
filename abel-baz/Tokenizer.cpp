#include "Tokenizer.hpp"

bool is_keyword(const std::string& word)
{
    static const std::string keywords[] = {
        "server", "listen", "location", "root", "methods",
        "index", "server_name", "autoindex", "error_page",
        "upload_dir", "cgi_extension", "redirection", "max_body_size"
    };

    for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); ++i)
        if (keywords[i] == word)
            return true;

    return false;
}

Tokenizer::Tokenizer(const std::string& filePath)
{
    std::ifstream file(filePath.c_str());
    if (!file)
        throw std::runtime_error("Failed to open the file");

    std::ostringstream stream; //strin version of std::cout
    std::string line;
    
    while (std::getline(file, line))
        stream << line << '\n'; //<< overloaded in stream class
                                //std::cout << "Hello" << '\n';
    _content = stream.str(); //Give me all the text I’ve added so far, as one full string.
}

std::vector<Token> Tokenizer::tokenize()
{
    std::vector<Token> tokens;
    std::string word;
    char c;

    for (size_t i = 0; i < _content.length(); ++i)
    {
        c = _content[i];

        if (std::isspace(c))
        {
            if (!word.empty())
            {
                if (is_keyword(word))
                    tokens.push_back(Token(KEYWORD, word));
                else
                    tokens.push_back(Token(VALUE, word));
                word.clear();
            }
            continue;
        }

        if (c == '{')
        {
            if (!word.empty()) {
                if (is_keyword(word))
                    tokens.push_back(Token(KEYWORD, word));
                else
                    tokens.push_back(Token(VALUE, word));
                word.clear();
            }
            tokens.push_back(Token(BRACE_OPEN, "{"));
        }
        else if (c == '}')
        {
            if (!word.empty()) {
                if (is_keyword(word))
                    tokens.push_back(Token(KEYWORD, word));
                else
                    tokens.push_back(Token(VALUE, word));
                word.clear();
            }
            tokens.push_back(Token(BRACE_CLOSE, "}"));
        }
        else if (c == ';')
        {
            if (!word.empty()) {
                if (is_keyword(word))
                    tokens.push_back(Token(KEYWORD, word));
                else
                    tokens.push_back(Token(VALUE, word));
                word.clear();
            }
            tokens.push_back(Token(SEMICOLON, ";"));
        }
        else if (c == '=')
        {
            if (!word.empty()) {
                if (is_keyword(word))
                    tokens.push_back(Token(KEYWORD, word));
                else
                    tokens.push_back(Token(VALUE, word));
                word.clear();
            }
            tokens.push_back(Token(EQUAL, "="));
        }
        else
        {
            word += c;
        }
    }

    if (!word.empty())
    {
        if (is_keyword(word))
            tokens.push_back(Token(KEYWORD, word));
        else
            tokens.push_back(Token(VALUE, word));
    }

    tokens.push_back(Token(END_OF_FILE, ""));
    return tokens;
}
