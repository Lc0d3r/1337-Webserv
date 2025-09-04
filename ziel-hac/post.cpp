#include "post.hpp"
#include "../ysahraou/HttpResponse.hpp"
#include "cgi_utils.hpp"
#include <../ysahraou/utils.hpp>

int parsechunked(HttpRequest &req, RoutingResult &ser) //<-- i need to parse chuncked body, for both cgi and non cgi, put to bodies one in the request to usit in cgi and the in the upload file in the conf.file 
{
	std::string upload_file = ser.getUploadFile() + "/uploads.txt";
	int fd = open(upload_file.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
	if(fd < 0)
		return 0;
	write(fd, req.body.c_str(), req.body.length());
	return 1;
}

int handle_multiple_form_data(HttpRequest &req, RoutingResult &ser)
{
	std::string boundary = req.getBoundary();
	std::vector<std::string> parts = split(req.body, "--" + boundary);
	parts.erase(parts.end() - 1);
	for(size_t i = 0; i < parts.size(); ++i)
	{
		size_t n = 0;
		std::vector<std::string> headers_and_body = split_header_and_body(parts[i]);
		if ((n = headers_and_body[0].find("filename=\"")) != std::string::npos)
		{
			std::string filename = headers_and_body[0].substr(n + 10,(headers_and_body[0].find("\"", n + 10)) - (n + 10));
			std::string upload_dir = ser.getUploadFile() + "/" + filename;
			int fd = open(upload_dir.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
			if (fd < 0)
			{
				print_log("500 error opening file", DiSPLAY_LOG);
				return 0; // Handle error appropriately
			}
			if (write(fd, headers_and_body[1].c_str(), headers_and_body[1].length()) < 0)
			{
				print_log("Failed to write to file: " + filename, DiSPLAY_LOG);
				close(fd);
				return 0; // Handle error appropriately
			}
		}
		else
		{
			n = headers_and_body[0].find("name=\"");
			std::string filename = headers_and_body[0].substr(n + 7, (headers_and_body[0].find("\"", n + 7)) - (n + 7));
			int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
			if (fd < 0)
				return 0;
			if (write(fd, headers_and_body[1].c_str(), headers_and_body[1].length()) < 0)
			{
				print_log("Failed to write to file: " + filename, DiSPLAY_LOG);
				close(fd);
				return 0; // Handle error appropriately
			}
		}
	}
	return 1; // Indicate success
}

int posthandler(HttpRequest *req, RoutingResult *ser, HttpResponse &res)
{
	if (!req->getTransferEncoding().empty() && req->getTransferEncoding() == "chunked")
		{
			if(!parsechunked(*req, *ser))
            {
				if (!get_error_page(res, 500, *req, "Internal Server Error")) {
					res.setTextBody("<h1>500 Internal Server Error</h1>");
					res.statusCode = 500;
					res.statusMessage = "Internal Server Error";
					res.addHeader("Content-Length", intToString(res.body.size()));
					res.addHeader("Content-Type", "text/html");
					if (req->is_keep_alive) {
						res.addHeader("Connection", "keep-alive");
					} else {
						res.addHeader("Connection", "close");
					}
				}
				return 0;
			}
			if (!get_error_page(res, 200, *req, "uploaded successfully")) {
				res.setTextBody("<h1>uploaded successfully</h1>");
				res.statusCode = 200;
				res.statusMessage = "OK";
				res.addHeader("Content-Length", intToString(res.body.size()));
				res.addHeader("Content-Type", "text/html");
				if (req->is_keep_alive)
					res.addHeader("Connection", "keep-alive");
				else 
					res.addHeader("Connection", "close");
				if (!req->getSessionId().empty()) {
					res.addHeader("set-Cookie", "session_id=" + req->getSessionId());
				}else
					res.addHeader("set-Cookie", "session_id=" + res.setSessionId());
			}
		}
	else if (req->getContentType() == "multipart/form-data")
    {

        if (!req->getContentLength().empty())
		{
            if (!handle_multiple_form_data(*req, *ser))
			{
				if (!get_error_page(res, 500, *req, "Internal Server Error")) {
					res.setTextBody("<h1>500 Internal Server Error</h1>");
					res.statusCode = 500;
					res.statusMessage = "Internal Server Error";
					res.addHeader("Content-Length", intToString(res.body.size()));
					res.addHeader("Content-Type", "text/html");
					if (req->is_keep_alive) {
						res.addHeader("Connection", "keep-alive");
					} else {
						res.addHeader("Connection", "close");
					}
				}
				return 0;
			}
			res.setTextBody("<h1>uploaded successfully</h1>");
			res.statusCode = 200;
			res.statusMessage = "OK";
			res.addHeader("Content-Length", intToString(res.body.size()));
			res.addHeader("Content-Type", "text/html");
			if (req->is_keep_alive)
				res.addHeader("Connection", "keep-alive");
			else 
				res.addHeader("Connection", "close");
			if (!req->getSessionId().empty()) {
				res.addHeader("set-Cookie", "session_id=" + req->getSessionId());
			}else
				res.addHeader("set-Cookie", "session_id=" + res.setSessionId());
		}
        else
		{
			if (!get_error_page(res, 500, *req, "Internal Server Error")) {
				res.setTextBody("<h1>411 Length Required</h1>");
				res.statusCode = 411;
				res.statusMessage = "Length Required";
				res.addHeader("Content-Length", intToString(res.body.size()));
				res.addHeader("Content-Type", "text/html");
				if (req->is_keep_alive) {
					res.addHeader("Connection", "keep-alive");
				} else {
					res.addHeader("Connection", "close");
				}
				return 0;
			}
		}
    }
	return 1;
}