#include <iostream>
#include <string>
#include "cgi.hpp"
#include "cgi_utils.hpp"
#include <sstream>
#include <fstream>
#include <fcntl.h>
#include <unistd.h>

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

// int handle_multiple_form_data(request &req)
// {
// 	std::string boundary = req.getBoundary();
// 	// std::cout << "Boundary: " << boundary << std::endl;
// 	std::vector<std::string> parts = split(req.getBody(), "--" + boundary);
// 	for(size_t i = 0; i < parts.size(); ++i)
// 	{
// 		size_t n = 0;
// 		std::vector<std::string> headers_and_body = split_header_and_body(parts[i]);
// 		if ((n = headers_and_body[0].find("filename=\"")) != std::string::npos)
// 		{
// 			std::string filename = headers_and_body[0].substr(n + 9, headers_and_body[0].size() - (n + 10));
// 			std::cout << "Filename: " << filename << std::endl;
// 		}
// 		else
// 		{
// 			n = headers_and_body[0].find("name=\"");
// 			std::string filename = headers_and_body[0].substr(n + 6, headers_and_body[0].size() - (n + 7));
// 			int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
// 			if (fd < 0)
// 			{
// 				std::cerr << "Failed to open file: " << filename << std::endl;
// 				return 0; // Handle error appropriately
// 			}
// 			if (write(fd, headers_and_body[1].c_str(), headers_and_body[1].length()) < 0)
// 			{
// 				std::cerr << "Failed to write to file: " << filename << std::endl;
// 				close(fd);
// 				return 0; // Handle error appropriately
// 			}
// 		}
// 	}
// 	return 1; // Indicate success
// }

// int parsechunked(request &req, server ser) //<-- i need to parse chuncked body, for both cgi and non cgi, put to bodies one in the request to usit in cgi and the in the upload file in the conf.file 
// {
// 	std::vector<std::string> chunks = split(req.getBody(), "\r\n");
// 	req.body.clear(); // Clear the body to store parsed content
// 	std::string upload_file = ser.getUploadFile() + "/uploads.txt";
// 	int fd = open(upload_file.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
// 	if (fd < 0)
// 	{
// 		std::cerr << "Failed to open upload file." << std::endl;
// 		fd = open("/tmp/upload", O_RDWR | O_CREAT | O_TRUNC, 0644);
// 		if (fd < 0)
// 			return (std::cerr << "Failed to open upload file." << std::endl, 0);
// 	}
// 	size_t chunk_size = 0;
// 	size_t i = 0;
// 	std::istringstream(chunks[i]) >> std::hex >> chunk_size;
// 	while(chunk_size > 0)
// 	{
// 		i++;
// 		for (size_t j = 0; j < chunk_size; ++j) {
// 			if (i + 1 < chunks.size()) {
// 				req.body += chunks[i][j];
// 			}
// 		}
// 		write(fd, chunks[i].c_str(), chunk_size);
// 		i++;
// 		std::istringstream(chunks[i]) >> std::hex >> chunk_size;
// 	}
// 	return 1;
// }

int main() 
{
	server serv;
	request req;
	fillRequest(req);
	std::cout << posthandler(&req, &serv) << std::endl;
	(void)serv; // To avoid unused variable warning
	return 0;
}