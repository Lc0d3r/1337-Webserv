#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include "cgi_utils.hpp"
//DO: split function to split a string by a delimiter
//RETURN: a vector of strings
std::vector<std::string> split(const std::string& str, const std::string& delimiters) {
    std::vector<std::string> parts;
    size_t start = 0;
    size_t pos = 0;
    while ((pos = str.find(delimiters, start)) != std::string::npos) {
        if (pos > start) {
            std::string part = str.substr(start, pos - start);
            if (!part.empty()) {
                parts.push_back(part);
            }
        }
        start = pos + delimiters.length();
    }
    if (start < str.length()) {
        std::string part = str.substr(start);
        if (!part.empty()) {
            parts.push_back(part);
        }
    }
    return parts;
}


std::vector<std::string> split_header_and_body(const std::string& str) {
    std::vector<std::string> parts;
    size_t pos = str.find("\r\n\r\n");
    if (pos != std::string::npos) {
        parts.push_back(str.substr(0, pos));    
        parts.push_back(str.substr(pos + 4));
    } else {
        parts.push_back(str);
    }
    return parts;
}