#include <string>
#include <vector>
#include <sstream>
#include "cgi_utils.hpp"
//DO: split function to split a string by a delimiter
//RETURN: a vector of strings
std::vector<std::string> split(const std::string& str, char delimiter)
{
    std::vector<std::string> result;
    std::stringstream ss(str);
    std::string item;

    while (std::getline(ss, item, delimiter)) 
    {
        if (item.size() > 0)
        result.push_back(item);
    }

    return result;
}

