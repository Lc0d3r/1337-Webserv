#include "HttpResponse.hpp"
#include "sockets.hpp"
#include <dirent.h>
#include <fstream>

std::string HttpResponse::toString() const {
        std::string responseString = httpVersion + " " + intToString(statusCode) + " " + statusMessage + "\r\n";
        std::map<std::string, std::string> tempHeaders = headers;

        // loop through headers and append them to the response string in c++ 98 style
        for (int i = 0; i < (int)tempHeaders.size(); ++i) {
            responseString += tempHeaders.begin()->first + ": " + tempHeaders.begin()->second + "\r\n";
            tempHeaders.erase(tempHeaders.begin());
        }

        responseString += "\r\n";

        return responseString;
};

void HttpResponse::setTextBody(const std::string& content) {
    text_body = content;
    is_binary = false; // Set the flag to indicate this is a text response
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
    std::cout << "file_path ===>> " << file_path.substr(pos + 1) << std::endl;
    return file_path.substr(pos + 1);
}

bool read_file(const std::string& file_path, HttpResponse& response, const std::string& format) {
    if (format == "html" || format == "txt") {
        std::ifstream file(file_path.c_str());
        if (!file.is_open()) {
            std::cerr << "Failed to open text file: " << file_path << std::endl;
            return false;
        }
        std::cout << "Reading text file: " << file_path << std::endl;
        std::stringstream buffer;
        buffer << file.rdbuf();
        response.setTextBody(buffer.str());
        response.is_binary = false; // Mark the response as text
        return true;
    }
    else if (format == "pdf") {
        std::ifstream file(file_path.c_str(), std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open PDF file: " << file_path << std::endl;
            return false;
        }

        // Go to the end to get the size
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        if (size <= 0) {
            std::cerr << "PDF file is empty or error reading size: " << file_path << std::endl;
            return false;
        }

        response.binary_body.resize(size); // Allocate space
        if (!file.read(response.binary_body.data(), size)) {
            std::cerr << "Error reading PDF file: " << file_path << std::endl;
            return false;
        }

        response.is_binary = true; // Mark the response as binary
        std::cout << "PDF file read successfully: " << file_path << " (" << size << " bytes)\n";
        return true;
    } 
    else if (format == "jpg" || format == "jpeg" || format == "png") {
        std::ifstream file(file_path.c_str(), std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open image file: " << file_path << std::endl;
            return false;
        }

        // Go to the end to get the size
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        if (size <= 0) {
            std::cerr << "Image file is empty or error reading size: " << file_path << std::endl;
            return false;
        }

        response.binary_body.resize(size); // Allocate space
        if (!file.read(response.binary_body.data(), size)) {
            std::cerr << "Error reading image file: " << file_path << std::endl;
            return false;
        }

        response.is_binary = true; // Mark the response as binary
        std::cout << "Image file read successfully: " << file_path << " (" << size << " bytes)\n";
        return true;
    }
    else if (format == "css") {
        std::ifstream file(file_path.c_str());
        if (!file.is_open()) {
            std::cerr << "Failed to open CSS file: " << file_path << std::endl;
            return false;
        }
        std::cout << "Reading CSS file: " << file_path << std::endl;
        std::stringstream buffer;
        buffer << file.rdbuf();
        response.setTextBody(buffer.str());
        response.is_binary = false; // Mark the response as text
        return true;
    }
    else if (format == "js") {
        std::ifstream file(file_path.c_str());
        if (!file.is_open()) {
            std::cerr << "Failed to open JS file: " << file_path << std::endl;
            return false;
        }
        std::cout << "Reading JS file: " << file_path << std::endl;
        std::stringstream buffer;
        buffer << file.rdbuf();
        response.setTextBody(buffer.str());
        response.is_binary = false; // Mark the response as text
        return true;
    }
    else if (format == "json") {
        std::ifstream file(file_path.c_str());
        if (!file.is_open()) {
            std::cerr << "Failed to open JSON file: " << file_path << std::endl;
            return false;
        }
        std::cout << "Reading JSON file: " << file_path << std::endl;
        std::stringstream buffer;
        buffer << file.rdbuf();
        response.setTextBody(buffer.str());
        response.is_binary = false; // Mark the response as text
        return true;
    }
    else if (format == "mp4") {
        std::ifstream file(file_path.c_str(), std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open MP4 file: " << file_path << std::endl;
            return false;
        }

        // Go to the end to get the size
        file.seekg(0, std::ios::end);
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);

        if (size <= 0) {
            std::cerr << "MP4 file is empty or error reading size: " << file_path << std::endl;
            return false;
        }

        response.binary_body.resize(size); // Allocate space
        if (!file.read(response.binary_body.data(), size)) {
            std::cerr << "Error reading MP4 file: " << file_path << std::endl;
            return false;
        }

        response.is_binary = true; // Mark the response as binary
        std::cout << "MP4 file read successfully: " << file_path << " (" << size << " bytes)\n";
        return true;
    }
    else {
        std::cerr << "Unsupported file format: " << format << std::endl;
        return false;
    }
}


void handleGETRequest(HttpResponse& response, const HttpRequest& request, const Config& config) {
    (void)response;
    int port;
    std::string hostname;
    splithostport(request.headers.at("Host"), hostname, port);
    errorType error = NO_ERROR;
    RoutingResult result = routingResult(config, hostname, port, request.path, request.method, error);
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
        if (generateAutoIndex(result.file_path, request.path, body)) {
            response.setTextBody(body);
            response.addHeader("Content-Type", "text/html");
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
    else {
        std::cout << "Not using autoindex for: " << result.file_path << std::endl;
        response.statusCode = 200; // OK
        response.statusMessage = "OK";
        std::string body;

        // Check the file format and read the file accordingly
        if (check_file_format(result.file_path) == "html" && read_file(result.file_path, response, "html")) {
            response.addHeader("Content-Type", "text/html");
            if (request.is_keep_alive) {
                response.addHeader("Connection", "keep-alive");
            } else {
                response.addHeader("Connection", "close");
            }
        } else if (check_file_format(result.file_path) == "pdf" && read_file(result.file_path, response, "pdf")) {
            response.addHeader("Content-Type", "application/pdf");
            response.addHeader("Content-Length", intToString(response.binary_body.size()));
            if (request.is_keep_alive) {
                response.addHeader("Connection", "keep-alive");
            } else {
                response.addHeader("Connection", "close");
            }
        } else if (check_file_format(result.file_path) == "txt" && read_file(result.file_path, response, "txt")) {
            response.addHeader("Content-Type", "text/plain");
            response.addHeader("Content-Length", intToString(body.length()));
            if (request.is_keep_alive) {
                response.addHeader("Connection", "keep-alive");
            } else {
                response.addHeader("Connection", "close");
            }
        } else if ((check_file_format(result.file_path) == "jpg" || check_file_format(result.file_path) == "jpeg" || check_file_format(result.file_path) == "png") && read_file(result.file_path, response, check_file_format(result.file_path))) {
            response.addHeader("Content-Type", "image/" + check_file_format(result.file_path));
            response.addHeader("Content-Length", intToString(response.binary_body.size()));
            if (request.is_keep_alive) {
                response.addHeader("Connection", "keep-alive");
            } else {
                response.addHeader("Connection", "close");
            }
        } else if (check_file_format(result.file_path) == "css" && read_file(result.file_path, response, "css")) {
            response.addHeader("Content-Type", "text/css");
            if (request.is_keep_alive) {
                response.addHeader("Connection", "keep-alive");
            } else {
                response.addHeader("Connection", "close");
            }
        }
        else if (check_file_format(result.file_path) == "js" && read_file(result.file_path, response, "js")) {
            response.addHeader("Content-Type", "application/javascript");
            if (request.is_keep_alive) {
                response.addHeader("Connection", "keep-alive");
            } else {
                response.addHeader("Connection", "close");
            }
        } else if (check_file_format(result.file_path) == "json" && read_file(result.file_path, response, "json")) {
            response.addHeader("Content-Type", "application/json");
            if (request.is_keep_alive) {
                response.addHeader("Connection", "keep-alive");
            } else {
                response.addHeader("Connection", "close");
            }
        } else if (check_file_format(result.file_path) == "mp4" && read_file(result.file_path, response, "mp4")) {
            response.addHeader("Content-Type", "video/mp4");
            response.addHeader("Content-Length", intToString(response.binary_body.size()));
            if (request.is_keep_alive) {
                response.addHeader("Connection", "keep-alive");
            } else {
                response.addHeader("Connection", "close");
            }
        }
        else {
            response.statusCode = 404; // Not Found
            response.statusMessage = "Not Found";
            response.addHeader("Content-Type", "text/html");
            response.addHeader("Content-Length", "23");
            if (request.is_keep_alive) {
                response.addHeader("Connection", "keep-alive");
            } else {
                response.addHeader("Connection", "close");
            }
            response.setTextBody("<h1>404 Not Found</h1>");
        }
    }
    std::cout << "file path: " << result.file_path << std::endl;
}

void response(int client_fd, HttpRequest &request, Config &config)
{
    (void)request;
    (void)config;
    printf("===========\[sending...]===========\n");
    HttpResponse response(200, "OK");
    handleGETRequest(response, request, config);

    // sending the response
    std::cout << "strlen(response) = " << strlen(response.toString().c_str()) << std::endl;
    write(client_fd , response.toString().c_str() , strlen(response.toString().c_str()));
    if (response.is_binary) {
        write(client_fd, response.binary_body.data(), response.binary_body.size());
    }
    else {
        write(client_fd, response.text_body.c_str(), response.text_body.length());
    }
    std::cout << "===========\[response sent]===========\n";
}
