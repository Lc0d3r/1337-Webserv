#include <iostream>
#include <string>
#include "cgi.hpp"
#include "cgi_utils.hpp"
#include <sstream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>
#include "HttpRequest.hpp"

void fillRequest(request& req) {
    req.method = req.getMethod();
    req.path = req.getPath();
    req.http_version = "HTTP/1.1"; // If you want to use a getter, replace it
    req.body = req.getBody();

    // Fill headers
    req.headers["Host"] = "localhost";
    req.headers["Content-Type"] = req.getContentType(); // or getContenttype() if intended
    req.headers["Cookie"] = req.getCookie();
    req.headers["Content-Length"] = req.getContentLength();
    req.headers["Transfer-Encoding"] = req.getTransferEncoding();

    // Optional: Add query string and others if needed
    req.headers["Query-String"] = req.getQueryString(); // Not standard but useful internally
    req.headers["Script-Filename"] = req.getScriptFilename();
    req.headers["Extension"] = req.getExtension(); // or getExtantion(), fix based on typo
}

int main() 
{
    
	return 0;
}