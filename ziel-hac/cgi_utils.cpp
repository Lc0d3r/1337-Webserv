#include <string>
#include <vector>
#include <sstream>
#include "cgi_utils.hpp"
//DO: split function to split a string by a delimiter
//RETURN: a vector of strings
std::vector<std::string> split(const std::string& str, const std::string& delimiters)
{
    std::vector<std::string> result;
    std::string token;
    
    for (size_t i = 0; i < str.length(); ++i)
    {
        if (delimiters.find(str[i]) != std::string::npos)
        {
            if (!token.empty())
            {
                result.push_back(token);
                token.clear();
            }
        }
        else
        {
            token += str[i];
        }
    }

    if (!token.empty())
        result.push_back(token);

    return result;
}

