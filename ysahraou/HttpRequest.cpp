#include "HttpRequest.hpp"

const std::string& HttpRequest::getTransferEncoding() const
{
    if (headers.count("Transfer-Encoding")) {
        return headers.at("Transfer-Encoding");
    }
    return std::string();
}

const std::string& HttpRequest::getBoundary() const
{
    if (headers.count("boundary")) {
        return headers.at("boundary");
    }
    return std::string();
}

const std::string& HttpRequest::getContentLength() const
{
    if (headers.count("Content-Length")) {
        return headers.at("Content-Length");
    }
    return std::string();
}

const std::string& HttpRequest::getContentType() const
{
    if (headers.count("Content-Type")) {
        return headers.at("Content-Type");
    }
    return std::string(); 
}

const std::string& HttpRequest::getQueryString() const
{
    size_t pos = path.find('?');
    if (pos != std::string::npos) {
        return path.substr(pos + 1);
    }
    return std::string();
}

const std::string& HttpRequest::getExtension() const
{
    std::vector<std::string> parts = split(path, "/");
    for (std::vector<std::string>::iterator it = parts.begin(); it != parts.end(); ++it) {
        std::string part = *it;
        size_t pos = part.find_last_of('.');
        if (pos != std::string::npos && pos < part.length() - 1) {
            return part.substr(pos);
        }
    }
    return std::string();

}

std::string trim(const std::string& str) {
    size_t start = 0;
    while (start < str.length() && std::isspace(str[start]))
        ++start;

    size_t end = str.length();
    while (end > start && std::isspace(str[end - 1]))
        --end;

    return str.substr(start, end - start);
}

int parse_req(std::string request_data, int socket_fd, HttpRequest &request)
{
    std::istringstream req_stream(request_data);
    std::string line;

    // get and parse the first line
    std::string first_line;
    if (!first_line.empty() && first_line.c_str()[first_line.size() - 1] == '\r')
        first_line.erase(first_line.size() - 1);

    std::getline(req_stream, first_line);
    // std::cout << "first_line: " << first_line << std::endl;
    std::string method;
    std::string path;
    std::string http_version;

    std::istringstream sss(first_line);
    sss >> method >> path >> http_version;
    if (method != "GET" && method != "DELETE" && method != "POST") {
        //hand the  response "bad request 400"
        const char* response_400 =
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 22\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<h1>Bad Request</h1>";
        write(socket_fd , response_400 , strlen(response_400));
        std::cout << "method ["<< method << "] is not allowed" << std::endl;
        std::cout << "400 ==> Bad Request response has send" << std::endl;
        return 1;
    }
    if (http_version != "HTTP/1.1") {
        // send 505 response
        const char* response_505 =
            "HTTP/1.1 505 HTTP Version Not Supported\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 39\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<h1>HTTP Version Not Supported</h1>";
        write(socket_fd , response_505 , strlen(response_505));
        std::cout << "505 HTTP Version Not Supported res has send" << std::endl;
        return 1;
    }
    request.method = method;
    request.path = path;
    request.http_version = http_version;
    // print the values 
    std::cout << "method = " << request.method << std::endl;
    std::cout << "path = " << request.path << std::endl;
    std::cout << "http version = " << request.http_version << std::endl;
    // parse the other headers
    std::map<std::string, std::string> headers;
    while (std::getline(req_stream, line)) {
        if (!line.empty() && line.c_str()[line.size() - 1] == '\r') {
            line.erase(line.size() - 1);
        }
        // print lines
        // std::cout << "Line: " << line << std::endl;
        size_t pos = line.find(':');

        if (pos != std::string::npos) {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            // Trim whitespace if needed
            key = trim(key);
            value = trim(value);

            headers[key] = value;
        }
    }
    request.headers = headers;

    // for (auto e : headers)
    //     std::cout << "first => " << e.first << " second => " << e.second << std::endl;
    std::map<std::string, std::string> headers_copy = request.headers;
    for (int i = 0; i < (int)headers_copy.size(); ++i) {
        std::cout << "header " << i << ": " << headers_copy.begin()->first << " : " << headers_copy.begin()->second << std::endl;
        headers_copy.erase(headers_copy.begin());
    }

    // check if the request is a keep-alive request
    if (headers.count("Connection") && headers["Connection"] == "keep-alive")
    {
        request.is_keep_alive = true;
        std::cout << "Connection is keep-alive" << std::endl;
    }
    else
    {
        request.is_keep_alive = false;
        std::cout << "Connection is not keep-alive" << std::endl;
    }
    return 0;
}


void readHeaders(std::string &request_data, int new_socket) {
    char buffer[2] = {0};
    while (request_data.find("\r\n\r\n") == std::string::npos) {
        int bytes = read(new_socket, buffer, 1);
        if (bytes <= 0) {
            // client disconnected or error
            break;
        }
        request_data.append(buffer, bytes);
    }
}

void readBody(HttpRequest &request, std::string &str_body, int new_socket) {
    int content_length = 0;
    if (request.headers.count("Content-Length"))
    {
        content_length = std::atoi(request.headers["Content-Length"].c_str());
        std::cout << "true content lenght is there -> " << request.headers.at("Content-Length") << "\n";
        char buffer[2] = {0};
        while (str_body.size() < static_cast<size_t>(content_length))
        {
            int bytes = read(new_socket, buffer, 1);
            if (bytes <= 0) {
                // client disconnected or error
                break;
            }
            str_body.append(buffer, bytes);
        }
        request.body = str_body;
    }
    else 
    {
        std::cout << "false content lenght is not there\n";
        char buffer[2] = {0};
        while (str_body.find("\r\n\r\n") == std::string::npos && request.method == "POST")
        {
            int bytes = read(new_socket, buffer, 1);
            if (bytes <= 0) {
                // client disconnected or error
                break;
            }
            str_body.append(buffer, bytes);
        }
        request.body = str_body;
    }
    std::cout << "done reading the body" << std::endl;
}
