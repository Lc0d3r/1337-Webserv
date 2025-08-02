#include "HttpResponse.hpp"
#include "sockets.hpp"
#include "../ziel-hac/cgi.hpp"
#include "../ziel-hac/post.hpp"
#include <dirent.h>
#include <fstream>


std::string HttpResponse::toString() const {
        std::string responseString = httpVersion + " " + intToString(statusCode) + " " + statusMessage + "\r\n";

        // Ensure all headers, including Content-Type, are added
        for (std::map<std::string, std::string>::const_iterator it = headers.begin(); it != headers.end(); ++it) {
            responseString += it->first + ": " + it->second + "\r\n";
        }

        responseString += "\r\n"; // End of headers

        return responseString;
};

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
    std::cout << "File size: " << size << " bytes" << std::endl;
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
    std::cout << "content Length: [" << response.headers["Content-Length"] << "]" << std::endl;
    std::ifstream file(file_path.c_str(), std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return false;
    }
    std::cout << "Reading file: " << file_path << std::endl;

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
        std::cout << "Set info to resume sending: " << file_path << std::endl;
        connections.pos += bytes_read;
        connections.file_path = file_path;
        connections.is_old = true;
    }
        
    std::cout << "file readed successfully: " << file_path << " (" << bytes_read << " bytes)\n";
    if (!supported_file_format(file_path)) {
        std::cout << "Adding Content-Disposition header for file download: " << file_path << std::endl;
        response.addHeader("Content-disposition", "attachment; filename=\"" + file_path.substr(file_path.find_last_of('/') + 1) + "\"");
    }
    file.close();
    return true;
}

bool resumeSending(ConnectionInfo &connections, std::vector<char> &buffer, int client_fd) {
    std::cout << "Resuming file sending: " << connections.file_path << std::endl;
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

            std::cout << "Sent " << bytes_read << " bytes, pos now: " << connections.pos << std::endl;
        }

        if (file.eof() || connections.pos >= Check_file_size(connections.file_path)) {
            std::cout << "Finished sending the file." << std::endl;
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
    RoutingResult result = routingResult(config, hostname, port, request.path_without_query, request.method, error);
    if (result.is_redirect) {
        response.statusCode = 301; // Moved Permanently
        response.statusMessage = "Moved Permanently";
        response.addHeader("Location", result.redirect_url);
        if (request.is_keep_alive) {
            response.addHeader("Connection", "keep-alive");
        } else {
            response.addHeader("Connection", "close");
        }
        return;
    }
    if (result.use_autoindex) {
        std::cout << "Autoindex enabled for: " << result.file_path << std::endl;
        response.statusCode = 200; // OK
        response.statusMessage = "OK";
        std::string body;
        std::cout << "result.file_path = " << result.file_path << std::endl;
        if (generateAutoIndex(result.file_path, request.path_without_query , body)) {
            response.setTextBody(body);
            response.addHeader("Content-Type", "text/html");
            response.addHeader("Content-Length", intToString(body.length()));
            if (request.is_keep_alive) {
                response.addHeader("Connection", "keep-alive");
            } else {
                response.addHeader("Connection", "close");
            }
        } else {
            response.statusCode = 500; // Internal Server Error
            response.statusMessage = "Internal Server Error";
            if (request.is_keep_alive) {
                response.addHeader("Connection", "keep-alive");
            } else {
                response.addHeader("Connection", "close");
            }
            response.setTextBody("<h1>500 Internal Server Error</h1>");
        }
        return;
    } 
    // Not using autoindex an absolute path
    else { 
        std::cout << "Not using autoindex for: " << result.file_path << std::endl;
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
                response.statusCode = 404; // Not Found
                response.statusMessage = "Not Found";
                response.addHeader("Content-Type", "text/html");
                response.setTextBody("<h1>404 Not Found</h1>");
                response.addHeader("Content-Length", intToString(response.body.size()));
            }
        if (request.is_keep_alive) {
            response.addHeader("Connection", "keep-alive");
        } else {
        response.addHeader("Connection", "close");
    }
    }
}

bool response(int client_fd, HttpRequest &request, Config &config, ConnectionInfo &connections)
{
    log_time();
    std::cout << "Preparing response for request: " << request.method << " " << request.path_without_query << std::endl;
    HttpResponse response(200, "OK");
    errorType error = NO_ERROR;
    int port;
    std::string hostname;
    if (request.headers.count("Host") == 0) {
        log_time();
        std::cerr << "Host header not found in request, closing connection." << std::endl;
        response.statusCode = 400; // Bad Request
        response.statusMessage = "Bad Request";
        response.addHeader("Content-Type", "text/html");
        response.setTextBody("<h1>400 Bad Request</h1>");
        response.addHeader("Content-Length", intToString(response.body.size()));
        write(client_fd, response.toString().c_str(), response.toString().size());
        write(client_fd, response.body.data(), response.body.size());
        return false;
    }
    splithostport(request.headers.at("Host"), hostname, port);
    RoutingResult routing_result = routingResult(config, hostname, port, request.path_without_query, request.method, error);
    // check error flag 
    if (error == NO_ERROR)
    {
        if (!routing_result.getExtension().empty())
        {
            Cgi handlecgi(routing_result, request, response);
            if (handlecgi.getvalidChecker() == 1)
            if (!handlecgi._executeScript(routing_result, request, response))
            std::cout << "Failed to execute CGI script." << std::endl;
        }
        else if (request.method == "GET") {
            handleGETRequest(response, request, config, connections);
        } else if (request.method == "POST") {
            if (error == NO_ERROR) {
                posthandler(&request, &routing_result, response);
            }
        }
    }
    log_time();
    std::cout << "Response prepared with status code: " << response.statusCode << " and message: " << response.statusMessage << std::endl;

    // sending the response headers
    write(client_fd , response.toString().c_str() , strlen(response.toString().c_str()));
    // sending the response body
    write(client_fd, response.body.data(), response.body.size());
    log_time();
    std::cout << "Response sent successfully." << std::endl;
    return true;
}
