#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "utils.hpp"

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
    return intToString(body.size());
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
        print_log( "Method not allowed: " + method + " sendin 400 Bad Request response." , DiSPLAY_LOG);
        HttpResponse response;
        if (!get_error_page(response, 400, request, "Bad Request")) {
            response.httpVersion = "HTTP/1.1";
            response.statusCode = 400;
            response.statusMessage = "Bad Request";
            response.addHeader("Content-Type", "text/html");
            response.setTextBody("<h1>400 Bad Request</h1>");
            response.addHeader("Content-Length", intToString(response.body.size()));
        }
        write(socket_fd, response.toString().c_str(), response.toString().size());
        write(socket_fd, response.body.data(), response.body.size());
        print_log( "Response sent for 400 bad request." , DiSPLAY_LOG);
        return 1;
    }
    if (http_version != "HTTP/1.1") {
        // send 505 response
        print_log( "HTTP version not supported: [" + http_version + "] sending 505 HTTP Version Not Supported response.", DiSPLAY_LOG);
        HttpResponse response;
        if (!get_error_page(response, 505, request, "HTTP Version Not Supported")) {
            response.httpVersion = "HTTP/1.1";
            response.statusCode = 505;
            response.statusMessage = "HTTP Version Not Supported";
            response.addHeader("Content-Type", "text/html");
            response.setTextBody("<h1>505 HTTP Version Not Supported</h1>");
            response.addHeader("Content-Length", intToString(response.body.size()));
        }
        write(socket_fd, response.toString().c_str(), response.toString().size());
        write(socket_fd, response.body.data(), response.body.size());
        print_log( "Response sent for 505 HTTP Version Not Supported." , DiSPLAY_LOG);
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
        setTheme(request);
    // check if the request is a keep-alive request
    if (headers.count("Connection")) {
        if (headers["Connection"] == "keep-alive")
            request.is_keep_alive = true;
        else
            request.is_keep_alive = false;
    }
    else {
        request.is_keep_alive = true;
    }

    // print headers
    // std::map<std::string, std::string> temp = request.headers;
    // for (std::map<std::string, std::string>::iterator it = temp.begin(); it != temp.end(); ++it) {
    //     std::cout << "Header: " << it->first << " : " << it->second << std::endl;
    // }   
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

bool isHex(const std::string& hexStr, int& value) {
    std::istringstream iss(hexStr);
    iss >> std::hex >> value;

    // Check if the stream failed or not fully consumed
    return !iss.fail() && iss.eof();
}

bool readChunkedBody(HttpRequest &request, std::string &str_body, int new_socket) {
    char buffer[2] = {0};
    std::string chunk_size_str;
    int chunk_size = 0;

    print_log( "Reading chunked body..." , DiSPLAY_LOG);
    while (true) {
        // Read chunk size
        chunk_size_str.clear();
        while (true) {
            int bytes = read(new_socket, buffer, 1);
            if (bytes <= 0) {
                // client disconnected or error
                return false;
            }
            if (buffer[0] == '\r') {
                // End of chunk size line
                break;
            }
            chunk_size_str += buffer[0];
        }
        // skip the newline character
        read(new_socket, buffer, 1); // Read '\n'
        if (chunk_size_str.empty()) {
            // No more chunks
            break;
        }
        // Convert hex to int
        if (!isHex(chunk_size_str, chunk_size))
        {
            print_log("Invalid chunk size: " + chunk_size_str, DiSPLAY_LOG);
            return false;
        }
        if (chunk_size == 0) {
            // Last chunk
            break;
        }
        // Read the chunk data
        str_body.clear();
        while ((int)str_body.size() < chunk_size) {
            int bytes = read(new_socket, buffer, 1);
            if (bytes <= 0) {
                // client disconnected or error
                return false;
            }
            str_body.append(buffer, bytes);
        }

        request.byte_readed += str_body.size();
        request.body += str_body;

        // Read the trailing CRLF after the chunk
        read(new_socket, buffer, 2); // Read \r\n
    }

    if (request.byte_readed > 0)
        print_log( "Chunked body read successfully, total bytes read: " + intToString(request.byte_readed), DiSPLAY_LOG);
    else 
        print_log( "No data read from chunked body.", DiSPLAY_LOG);
    return true;
}

bool readBody(HttpRequest &request, std::string &str_body, int new_socket) {
    int content_length = 0;
    print_log( "Reading the body..." , DiSPLAY_LOG);
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
            print_log( "Request is in progress, bytes readed: " + intToString(request.byte_readed) + ", content length: " + intToString(content_length ), DiSPLAY_LOG);
        } else {
            request.done = true;
            request.in_progress = false;
            print_log( "Request done, bytes readed: " + intToString(request.byte_readed) + ", content length: " + intToString(content_length) , DiSPLAY_LOG);
        }
        return true; // body read successfully
    }
    else if (request.headers.count("Transfer-Encoding") && request.headers["Transfer-Encoding"] == "chunked")
    {
        if (readChunkedBody(request, str_body, new_socket)) {
            request.in_progress = false;
            request.done = true;
            request.byte_readed = str_body.size();
            request.body = str_body;
        }
        return true; // chunked body read successfully
    }
    else {
        if (request.method == "POST")
            return false; // no body to read
        return true; // no body to read, but request is valid
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