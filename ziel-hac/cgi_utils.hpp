#ifndef CGI_UTILS_HPP
#define CGI_UTILS_HPP

#include <iostream>
#include <cstdlib>
#include <sstream>
#include <string>
#include <vector>

std::vector<std::string> split(const std::string& str, const std::string& delimiters);
std::vector<std::string> split_header_and_body(const std::string& str);



#endif