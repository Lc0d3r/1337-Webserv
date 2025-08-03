#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

HttpRequest::HttpRequest() : in_progress(false), done(false), byte_readed(0), content_length(0) {}

void setTheme(HttpRequest &request)
{
    std::string theme = request.getCookie();
    size_t pos = theme.find("theme=");
    if (pos != std::string::npos) 
    {
        size_t endPos = theme.find(';', pos);
        if (endPos != std::string::npos)
            cookies_map[request.getSessionId()] = theme.substr(pos + 6, endPos - pos - 6);
        else
            cookies_map[request.getSessionId()] = theme.substr(pos + 6, theme.size() - pos - 6);
    }
    else 
        cookies_map[request.getSessionId()] = "default"; // default theme
}

std::string HttpRequest::getSessionId() const
{
    if (headers.count("Cookie"))
    {
        size_t pos = headers.at("Cookie").find("session_id=");
        if (pos != std::string::npos)
        {
            pos += std::string("session_id=").length();
            std::string session_id = headers.at("Cookie").substr(pos, headers.at("Cookie").find(';', pos) - pos);
            return session_id;
        }
    }
    return std::string();
}

std::string HttpRequest::getCookie() const
{
    if (headers.count("Cookie")) {
        return headers.at("Cookie");
    }
    return std::string();
}

std::string HttpRequest::getTransferEncoding() const
{
    if (headers.count("Transfer-Encoding")) {
        std::cout << "Transfer-Encodingnigger: " << headers.at("Transfer-Encoding") << std::endl;
        return headers.at("Transfer-Encoding");
    }
    return std::string();
}

std::string HttpRequest::getBoundary() const
{
    if (headers.count("Content-Type")) {
        std::vector<std::string> parts = split(headers.at("Content-Type"), "; ");
        std::vector<std::string> boundary = split(parts[1], "="); 
        return boundary[1];
    }
    return std::string();
}

std::string HttpRequest::getContentLength() const
{
    if (headers.count("Content-Length")) {
        return headers.at("Content-Length");
    }
    return std::string();
}

std::string HttpRequest::getContentType() const
{
    if (headers.count("Content-Type")) {
        std::vector<std::string> parts = split(headers.at("Content-Type"), ";");

        return parts[0];
    }
    return std::string(); 
}

std::string HttpRequest::getQueryString() const
{
    size_t pos = path.find('?');
    if (pos != std::string::npos) {
        return path.substr(pos + 1);
    }
    return std::string();
}

std::string HttpRequest::getExtension() const
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
        log_time();
        std::cout << "Method not allowed: " << method << " sendin 400 Bad Request response." << std::endl;
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
        log_time();
        std::cout << "HTTP version not supported: " << http_version << " sending 505 HTTP Version Not Supported response." << std::endl;
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
    if (headers.count("Cookie") > 0)
    {
        setTheme(request);
        if (cookies_map.count("1") > 0)
            std::cout << "Theme set to: " << cookies_map["1"] << std::endl;
        if (cookies_map.count("2") > 0)
            std::cout << "Theme set to: " << cookies_map["2"] << std::endl;
        if (cookies_map.count("3") > 0)
            std::cout << "Theme set to: " << cookies_map["3"] << std::endl;
    }
    std::cout << "accounts: " << cookies_map.size() << std::endl;

    // check if the request is a keep-alive request
    if (headers.count("Connection") && headers["Connection"] == "keep-alive")
        request.is_keep_alive = true;
    else
        request.is_keep_alive = false;
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

bool readChunkedBody(HttpRequest &request, std::string &str_body, int new_socket) {
    char buffer[2] = {0};
    while (str_body.find("\r\n\r\n") == std::string::npos && request.method == "POST")
    {
        int bytes = read(new_socket, buffer, 1);
        if (bytes <= 0) {
            break;
        }
        str_body.append(buffer, bytes);
    }
    return true;
}

void readBody(HttpRequest &request, std::string &str_body, int new_socket) {
    int content_length = 0;
    log_time();
    std::cout << "Reading the body..." << std::endl;
    if (request.headers.count("Content-Length"))
    {
        content_length = std::atoi(request.headers["Content-Length"].c_str());
        request.content_length = content_length;
        char buffer[2] = {0};
        while (str_body.size() < CHUNK_SIZE)
        {
            int bytes = read(new_socket, buffer, 1);
            if (bytes <= 0) {
                // client disconnected or error
                break;
            }
            str_body.append(buffer, bytes);
        }
        request.byte_readed += str_body.size();
        request.body += str_body;
        if (request.byte_readed < content_length) {
            request.in_progress = true;
            log_time();
            std::cout << "Request is in progress, bytes readed: " << request.byte_readed << ", content length: " << content_length << std::endl;
        } else {
            request.done = true;
            request.in_progress = false;
            log_time();
            std::cout << "Request done, bytes readed: " << request.byte_readed << ", content length: " << content_length << std::endl;
        }
    }
    else 
    {
        if (readChunkedBody(request, str_body, new_socket)) {
            request.in_progress = false;
            request.done = true;
            request.byte_readed = str_body.size();
            request.body = str_body;
        }
    }
}

void removeQueryString(HttpRequest &request) {
    size_t pos = request.path.find('?');
    if (pos != std::string::npos) {
        request.path_without_query = request.path.substr(0, pos);
    } else {
        request.path_without_query = request.path;
    }
}