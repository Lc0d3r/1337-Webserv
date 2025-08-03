#pragma once

#include "sockets.hpp"

#define LOG_FILE "webserv.log"
#define DiSPLAY_LOG 1

void print_log(const std::string& message, int display);
int hexCharToInt(char c);
std::string decodePath(std::string path);