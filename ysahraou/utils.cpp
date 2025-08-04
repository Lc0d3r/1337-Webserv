#include "utils.hpp"

int hexCharToInt(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    if (c >= 'A' && c <= 'F') return c - 'A' + 10;
    return -1;  // Invalid hex character
}


std::string decodePath(std::string path)
{
    std::string result_path;
    int high;
    int low;
//  /AllFileFormat/test%20%20.png
    int i = 0;
    while (i < (int)path.size())
    {
        if (path[i] == '%') {
            if (path[i+1] and path[i+2])
            {
                high = hexCharToInt(path[i+1]);
                low = hexCharToInt(path[i+2]);
                if (high != -1 && low != -1) {
                    result_path += ((high << 4) + low);
                }
                else {
                    std::cerr << "Invalid hex encoding in path: " << path << std::endl;
                    return ""; // Return empty string on error
                }
                i += 3; // Skip the next two characters
            }
            else {
                std::cerr << "Incomplete hex encoding in path: " << path << std::endl;
                return ""; // Return empty string on error
            }
        }
        else {
            result_path += path[i]; // Keep other characters as is
            i++;
        }
    }
    return result_path;
}

void print_log(const std::string& message, int display) {
    if (display) {
        log_time();
        std::cout << message << std::endl;
    }
}