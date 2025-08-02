#pragma once

#include "sockets.hpp"

void print_log(const std::string& message);
int hexCharToInt(char c);
std::string decodePath(std::string path);