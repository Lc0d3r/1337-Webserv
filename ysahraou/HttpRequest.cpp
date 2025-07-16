#include "HttpRequest.hpp"

void parse_req(std::string request_data)
{
    std::istringstream req_stream(request_data);
    std::string line;

    // get and parse the first line
    std::string first_line;
    std::getline(req_stream, first_line);
    std::cout << "first_line: " << first_line << std::endl;
    while (std::getline(req_stream, line)) {
        std::cout << "Line: " << line << std::endl;
    }
}