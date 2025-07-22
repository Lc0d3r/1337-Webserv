#pragma once

#include "sockets.hpp"


// e.g, GET /index.html?key=value&key=value HTTP/1.1

struct HttpRequest {
    std::string method;          // e.g., "GET"
    std::string path;            // e.g., "/index.html"  
    std::string http_version;    // e.g., "HTTP/1.1"
    std::map<std::string, std::string> headers;
    std::string body; // the body

    // std::string getPath() const { return "/cgi-bin/script.php"; }
	// std::string getExtension() const { return ".php"; }
	// std::string getMethod() const { return "GET"; }
	// std::string getQueryString() const { return "id=42&name=test"; }
	// std::string getContentType() const { return "text/html"; }
	// std::string getCookie() const { return "sessionid=abc123"; }
	// std::string getContentLength() const { return "10"; } // Simulate no body
	// std::string getExtantion() const { return ".php"; } // Typo in class, must match your real implementation
	// std::string getScriptFilename() const { return "/var/www/html/script.php"; }
	// std::string getBody() const { return "4\r\nrestnigga\r\n5\r\nlesssnigga\r\na\r\n1234567890nigga\r\n0\r\n\r\n"; } 
	// std::string getTransferEncoding() const { return "chunked"; } // Simulated transfer encoding
	// std::string getContenttype() const { return "multipart/form-data"; } // Simulated content type
};


int parse_req(std::string request_data, int socket_fd, HttpRequest &request);
void readHeaders(std::string &request_data, int new_socket);
void readBody(HttpRequest &request, std::string &str_body, int new_socket);