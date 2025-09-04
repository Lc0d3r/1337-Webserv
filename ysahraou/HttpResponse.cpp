#include "HttpResponse.hpp"
#include "sockets.hpp"
#include "utils.hpp"

bool get_error_page(HttpResponse &response, int error_code, const HttpRequest &request, std::string error_message) {
    std::map<int, std::string> error_pages = request.error_pages;
    if (error_pages.count(error_code)) {
        std::string error_page_path = error_pages[error_code];
        std::ifstream error_page_file(error_page_path.c_str());
        if (error_page_file.is_open()) {
            error_page_file.seekg(0, std::ios::end);
            response.body.resize(error_page_file.tellg());
            error_page_file.seekg(0, std::ios::beg);
            error_page_file.read(response.body.data(), response.body.size());
            error_page_file.close();
            response.statusCode = error_code;
            response.statusMessage = error_message;
            response.addHeader("Content-Type", "text/html");
            response.addHeader("Content-Length", intToString(response.body.size()));
            if (request.is_keep_alive && error_code != 400 && error_code != 505) {
                response.addHeader("Connection", "keep-alive");
            } else {
                response.addHeader("Connection", "close");
            }
            print_log("Error page loaded for error code: " + intToString(error_code) + " from path: " + error_page_path, DiSPLAY_LOG);
            return true;
        }
        else 
            std::cerr << "Error: Could not open error page file: " << error_page_path << std::endl;
    }
    std::cerr << "Error: No error page defined for error code: " << error_code << std::endl;
    return false;
}

std::string HttpResponse::toString() const {
    std::string responseString = httpVersion + " " + intToString(statusCode) + " " + statusMessage + "\r\n";
    
    // Ensure all headers, including Content-Type, are added
    for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        responseString += it->first + ": " + it->second + "\r\n";
    }
    
    responseString += "\r\n"; // End of headers
    
    return responseString;
};

std::string HttpResponse::setSessionId()
{
    std::srand(std::time(0));
    size_t sessionid = std::rand();
    return intToString(sessionid);
}

void HttpResponse::setTextBody(const std::string& content) {
    body = std::vector<char>(content.begin(), content.end());
}

void HttpResponse::addHeader(const std::string& key, const std::string& value) {
        headers[key] = value;
}

void appandtostring(std::string& str, const std::string& content) {
    str += content;
}

void splithostport(const std::string& host, std::string& hostname, int& port) {
    size_t colonPos = host.find(':');
    hostname = host.substr(0, colonPos);
    port = std::atoi(host.substr(colonPos + 1).c_str());
}

// generate autoindex HTML for a directory
bool generateAutoIndex(const std::string& dirPath, const std::string& requestPath, std::string& body) {
    DIR* dir = opendir(dirPath.c_str());
    if (!dir) {
        std::cerr << "Failed to open directory: " << dirPath << std::endl;
        return false;
    }
    struct dirent* entry;
    body = "<!DOCTYPE html>\n"
           "<html>\n"
           "<head>\n"
           "<meta charset=\"UTF-8\">\n"
           "<title>Index of " + requestPath + "</title>\n"
           "<style>\n"
           "body { background: #f8f9fa; font-family: Arial, sans-serif; }\n"
           ".container { max-width: 600px; margin: 60px auto; background: #fff; border-radius: 10px; box-shadow: 0 2px 8px rgba(0,0,0,0.1); padding: 30px; text-align: center; }\n"
           "h1 { color: #007bff; margin-bottom: 30px; }\n"
           "ul { list-style: none; padding: 0; }\n"
           "li { margin: 12px 0; }\n"
           "a { text-decoration: none; color: #333; font-size: 18px; transition: color 0.2s; }\n"
           "a:hover { color: #007bff; }\n"
           "</style>\n"
           "</head>\n"
           "<body>\n"
           "<div class=\"container\">\n"
           "<h1>Index of " + requestPath + "</h1>\n"
           "<ul>\n";
    while ((entry = readdir(dir)) != NULL) {
        std::string entryName = entry->d_name;
        if (entryName == "." || entryName == "..") continue;
        std::string href = requestPath;
        if (href.empty() || href[href.size() - 1] != '/')
            href += "/";
        href += entryName;
        body += "<li><a href=\"" + href + "\">" + entryName + "</a></li>\n";
    }
    body += "</ul>\n</div>\n</body>\n</html>\n";
    closedir(dir);
    return true;
}

std::string check_file_format(const std::string& file_path) {
    size_t pos = file_path.find_last_of('.');
    if (pos == std::string::npos) {
        return ""; // No extension found
    }
    return file_path.substr(pos + 1);
}

std::streamsize Check_file_size(const std::string& file_path) {
    std::ifstream file(file_path.c_str(), std::ios::binary );
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return 0;
    }
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);
    if (size <= 0) {
        std::cerr << "File is empty or error reading size: " << file_path << std::endl;
        return 0;
    }
    return size;
}

std::string streamsizeToString(std::streamsize size) {
    std::ostringstream oss;
    oss << size;
    return oss.str();
}

bool supported_file_format(const std::string& file_path) {
    std::string extension = check_file_format(file_path);
    return (extension == "txt" || extension == "pdf" || extension == "jpg" || extension == "png" || 
            extension == "html" || extension == "css" || extension == "js" || extension == "ico" ||
            extension == "json" || extension == "gpg" || extension == "mp4" || extension == "gif" || extension == "jpeg");
}

bool read_file(const std::string& file_path, HttpResponse& response, ConnectionInfo& connections) {
    response.addHeader("Content-Length", streamsizeToString(Check_file_size(file_path)));
    std::ifstream file(file_path.c_str(), std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return false;
    }
    print_log("Reading file: " + file_path, DiSPLAY_LOG);

    response.body.resize(MAX_TO_SEND); // Allocate space
    file.read(response.body.data(), MAX_TO_SEND);
    std::streamsize bytesRead = file.gcount();  // Will be 20 here

    if (bytesRead > 0) {
        response.body.resize(bytesRead);  // Trim unused bytes
        // OK to use this chunk
    } else {
        std::cerr << "Nothing was read.\n";
    }
    // if (!file.read(response.body.data(), MAX_TO_SEND)) {
    //     std::cerr << "Error reading file: " << file_path << std::endl;
    //     return false;
    // }
    std::streamsize bytes_read = file.gcount();
    if (bytes_read > 0 && !file.eof() && connections.pos < file.tellg()) {
        print_log("Set info to resume sending: " + file_path, DiSPLAY_LOG);
        connections.pos += bytes_read;
        connections.file_path = file_path;
        connections.is_old = true;
    }

    if (!supported_file_format(file_path)) {
        print_log("Unsupported file format: " + file_path + " adding Content-Disposition header", DiSPLAY_LOG);
        response.addHeader("Content-disposition", "attachment; filename=\"" + file_path.substr(file_path.find_last_of('/') + 1) + "\"");
    }
    file.close();
    return true;
}

bool resumeSending(ConnectionInfo &connections, std::vector<char> &buffer, int client_fd) {
    print_log("Resuming file sending: " + connections.file_path, DiSPLAY_LOG);
    if (connections.pos < Check_file_size(connections.file_path)) {
        std::ifstream file(connections.file_path.c_str(), std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: " << connections.file_path << std::endl;
            return false;
        }
        file.seekg(connections.pos);
        // Implement binary file sending logic here
        file.read(buffer.data(), buffer.size());
        std::streamsize bytes_read = file.gcount();

        if (bytes_read > 0) {
            // Send these bytes (e.g. using write/send)
            int bytes_written = write(client_fd, buffer.data(), bytes_read);
            if (bytes_written > 0)
                connections.pos += bytes_written;

            print_log("Sent " + intToString(bytes_written) + " bytes, pos now: " + intToString(connections.pos), DiSPLAY_LOG);
        }

        if (file.eof() || connections.pos >= Check_file_size(connections.file_path)) {
            print_log("Finished sending the file.", DiSPLAY_LOG);
            connections.is_old = false;
            connections.pos = 0;
            connections.file_path.clear();
            return false;
        }
    }
    return true;
}

void handleGETRequest(HttpResponse& response, const HttpRequest& request, const Config& config, ConnectionInfo& connections) {

    int port;
    std::string hostname;
    splithostport(request.headers.at("Host"), hostname, port);
    errorType error = NO_ERROR;
    RoutingResult result = routingResult(config, hostname, connections.portToConnect, request.path_without_query, 
        request.method, error, connections.hostToConnect);
    print_log("Routing result for GET request: " + result.file_path, DiSPLAY_LOG);
    print_log("servername: " + hostname + ", port: " + intToString(connections.portToConnect) + ", ip: " + connections.hostToConnect, DiSPLAY_LOG);
    if (result.is_redirect) {
        response.statusCode = 301; // Moved Permanently
        response.statusMessage = "Moved Permanently";
        print_log("Redirecting to: " + result.redirect_url, DiSPLAY_LOG);
        response.addHeader("Location", result.redirect_url);
        response.addHeader("Content-Length", "0");
        if (request.is_keep_alive) {
            response.addHeader("Connection", "keep-alive");
        } else {
            response.addHeader("Connection", "close");
        }
        return;
    }
    if (result.use_autoindex) {
        print_log("Using autoindex for path: " + result.file_path, DiSPLAY_LOG);
        response.statusCode = 200; // OK
        response.statusMessage = "OK";
        std::string body;
        if (generateAutoIndex(result.file_path, request.path_without_query , body)) {
            response.setTextBody(body);
            response.addHeader("Content-Type", "text/html");
            response.addHeader("Content-Length", intToString(body.length()));
            if (request.is_keep_alive) {
                response.addHeader("Connection", "keep-alive");
            } else {
                response.addHeader("Connection", "close");
            }
            if (!request.getSessionId().empty()) {
                response.addHeader("set-Cookie", "session_id=" + request.getSessionId());
                print_log("Session ID get in request: " + request.getSessionId(), DiSPLAY_LOG);
            }
            else {
                response.addHeader("set-Cookie", "session_id=" + response.setSessionId());
                print_log("Session ID set in response: " + response.setSessionId(), DiSPLAY_LOG);
            }
        } else {
            if (!get_error_page(response, 403, request, "Forbidden")) {
                print_log("Failed to generate autoindex for: " + result.file_path, DiSPLAY_LOG);
                response.statusCode = 403; // Forbidden
                response.statusMessage = "Forbidden";
                response.addHeader("Content-Type", "text/html");
                response.setTextBody("<h1>403 Forbidden</h1>");
                response.addHeader("Content-Length", intToString(response.body.size()));
                if (request.is_keep_alive) {
                    response.addHeader("Connection", "keep-alive");
                } else {
                    response.addHeader("Connection", "close");
                }
            }
        }
        return;
    } 
    // Not using autoindex an absolute path
    else {
        print_log("Not using autoindex for: " + result.file_path, DiSPLAY_LOG);
        response.statusCode = 200; // OK
        response.statusMessage = "OK";
        std::string body;

        // Check the file format and read the file accordingly
        if (read_file(result.file_path, response, connections)) {
            std::string ext = check_file_format(result.file_path);
            if (ext == "html" || ext == "txt" || ext == "css") {
                response.addHeader("Content-Type", "text/" + ext);
            } else if (ext == "js" || ext == "json") {
                response.addHeader("Content-Type", "application/" + ext);
            } else if (ext == "gpg" || ext == "png" || ext == "jpeg" || ext == "jpg" || ext == "gif") {
                response.addHeader("Content-Type", "image/" + ext);
            } else if (ext == "mp4") {
                response.addHeader("Content-Type", "video/mp4");
            }
        }
        else {
                if (!get_error_page(response, 404, request, "Not Found")) {
                    print_log("File not found: " + result.file_path, DiSPLAY_LOG);
                    response.statusCode = 404; // Not Found
                    response.statusMessage = "Not Found";
                    response.addHeader("Content-Type", "text/html");
                    response.setTextBody("<h1>404 Not Found</h1>");
                    response.addHeader("Content-Length", intToString(response.body.size()));
                }
            }
        if (request.is_keep_alive) {
            response.addHeader("Connection", "keep-alive");
        } else {
        response.addHeader("Connection", "close");
    }
    }
}

bool handleDeleteRequest(HttpResponse& response, const HttpRequest& request, RoutingResult& routing_result) {
    std::string file_path = routing_result.file_path;
    if (remove(file_path.c_str()) == 0) {
        response.statusCode = 200; // OK
        response.statusMessage = "OK";
        response.setTextBody("<h1>File deleted successfully</h1>");
        response.addHeader("Content-Type", "text/html");
        response.addHeader("Content-Length", intToString(response.body.size()));
    } else {
        if (!get_error_page(response, 500, request, "Internal Server Error")) {
            print_log("Failed to delete file: " + file_path, DiSPLAY_LOG);
            response.statusCode = 500; // Internal Server Error
            response.statusMessage = "Internal Server Error";
            response.setTextBody("<h1>Failed to delete file</h1>");
            response.addHeader("Content-Length", intToString(response.body.size()));
            response.addHeader("Content-Type", "text/html");
        }
        return false;
    }
    if (request.is_keep_alive)
        response.addHeader("Connection", "keep-alive");
    else
        response.addHeader("Connection", "close");
    return true;
}

bool response(int client_fd, HttpRequest &request, Config &config, ConnectionInfo &connections)
{
    print_log( "Preparing response for request: " + request.method + " " + request.path_without_query , DiSPLAY_LOG);
    HttpResponse response(200, "OK");
    errorType error = NO_ERROR;
    int port;
    std::string hostname;
    if (request.headers.count("Host") == 0) {
        print_log( "Host header not found in request, closing connection." , DiSPLAY_LOG);
        if (!get_error_page(response, 400, request, "Bad Request")) {
            response.statusCode = 400; // Bad Request
            response.statusMessage = "Bad Request";
            response.addHeader("Content-Type", "text/html");
            response.setTextBody("<h1>400 Bad Request</h1>");
            response.addHeader("Connection", "close");
            response.addHeader("Content-Length", intToString(response.body.size()));
        }
        write(client_fd, response.toString().c_str(), response.toString().size());
        write(client_fd, response.body.data(), response.body.size());
        return false;
    }
    splithostport(request.headers.at("Host"), hostname, port);
    RoutingResult routing_result = routingResult(config, hostname, connections.portToConnect, request.path_without_query, request.method, error, connections.hostToConnect);
    // check error flag 
    if (error == NO_ERROR)
    {
        if (!routing_result.getExtension().empty())
        {
            Cgi handlecgi(routing_result, request, response);
            if (handlecgi.getvalidChecker() == 1)
            if (!handlecgi._executeScript(routing_result, request, response))
                print_log( "Failed to execute CGI script." , DiSPLAY_LOG);
        }
        else if (request.method == "GET") {
            handleGETRequest(response, request, config, connections);
        } else if (request.method == "POST") {
            if (error == NO_ERROR) {
                posthandler(&request, &routing_result, response);
            }
        }
        else if (request.method == "DELETE") {
            if (!handleDeleteRequest(response, request, routing_result)) {
                print_log( "Failed to handle DELETE request." , DiSPLAY_LOG);
                return false;
            }
        }
    }
    else if (error == SERVER_NOT_FOUND || error == LOCATION_NOT_FOUND || error == FILE_NOT_FOUND) {
        print_log( "server or page not found for host: " + hostname + ":" + intToString(port) , DiSPLAY_LOG);
        if (!get_error_page(response, 404, request, "Not Found")) {
            response.statusCode = 404; // Not Found
            response.statusMessage = "Not Found";
            response.addHeader("Content-Type", "text/html");
            response.setTextBody("<h1>404 Not Found</h1>");
            response.addHeader("Content-Length", intToString(response.body.size()));
            if (request.is_keep_alive)
                response.addHeader("Connection", "keep-alive");
            else
                response.addHeader("Connection", "close");
        }
    } else if (error == METHOD_NOT_ALLOWED) {
        print_log( "Method not allowed for path: " + request.path_without_query , DiSPLAY_LOG);
        if (!get_error_page(response, 405, request, "Method Not Allowed")) {
            response.statusCode = 405; // Method Not Allowed
            response.statusMessage = "Method Not Allowed";
            response.addHeader("Content-Type", "text/html");
            response.setTextBody("<h1>405 Method Not Allowed</h1>");
            response.addHeader("Content-Length", intToString(response.body.size()));
            if (request.is_keep_alive)
                response.addHeader("Connection", "keep-alive");
            else
                response.addHeader("Connection", "close");
        }
    } else if (error == ACCESS_DENIED) {
        print_log( "Access denied for path: " + request.path_without_query , DiSPLAY_LOG);
        if (!get_error_page(response, 403, request, "Forbidden")) {
            response.statusCode = 403; // Forbidden
            response.statusMessage = "Forbidden";
            response.addHeader("Content-Type", "text/html");
            response.setTextBody("<h1>403 Forbidden</h1>");
            response.addHeader("Content-Length", intToString(response.body.size()));
            if (request.is_keep_alive)
                response.addHeader("Connection", "keep-alive");
            else
                response.addHeader("Connection", "close");
        }
    }
    else {
        print_log( "Unknown error occurred, setting 500 Internal Server Error." , DiSPLAY_LOG);
        if (!get_error_page(response, 500, request, "Internal Server Error")) {
            response.statusCode = 500; // Internal Server Error
            response.statusMessage = "Internal Server Error";
            response.addHeader("Content-Type", "text/html");
            response.setTextBody("<h1>500 Internal Server Error</h1>");
            response.addHeader("Content-Length", intToString(response.body.size()));
            if (request.is_keep_alive)
                response.addHeader("Connection", "keep-alive");
            else
                response.addHeader("Connection", "close");
        }
    }
    print_log( "Response prepared with status code: " + intToString(response.statusCode) + " and message: " + response.statusMessage , DiSPLAY_LOG);
    if (response.statusCode == 200) {
        if (!request.getSessionId().empty()) {
            response.addHeader("set-Cookie", "session_id=" + request.getSessionId());
            print_log("Session ID get in request: " + request.getSessionId(), DiSPLAY_LOG);
        }
        else {
            response.addHeader("set-Cookie", "session_id=" + response.setSessionId());
            print_log("Session ID set in response: " + response.setSessionId(), DiSPLAY_LOG);
        }
    }
    // sending the response headers
    write(client_fd , response.toString().c_str() , strlen(response.toString().c_str()));
    // sending the response body
    write(client_fd, response.body.data(), response.body.size());
    print_log( "Response sent successfully." , DiSPLAY_LOG);
    return true;
}
