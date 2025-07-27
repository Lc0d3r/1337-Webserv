#include "cgi.hpp"
#include <sstream>
#include <fcntl.h>
#include <unistd.h>

int parsechunked(HttpRequest &req, RoutingResult &ser) //<-- i need to parse chuncked body, for both cgi and non cgi, put to bodies one in the request to usit in cgi and the in the upload file in the conf.file 
{
	std::vector<std::string> chunks = split(req.body, "\r\n");
	size_t chunk_size = 0;
	size_t i = 0;
	std::string upload_file = ser.getUploadFile() + "/uploads.txt";
	int fd = open(upload_file.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
	if(fd < 0)
	{
		std::cerr << "Failed to open upload file." << std::endl;
		fd = open("/tmp/uploads.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
		if (fd < 0)
			return (std::cerr << "Failed to open upload file." << std::endl, 0);
	}
	req.body.clear();
	std::istringstream(chunks[i]) >> std::hex >> chunk_size;
	while(chunk_size > 0)
	{
		i++;
		write(fd, chunks[i].c_str(), chunk_size);
		for (size_t j = 0; j < chunk_size; ++j) {
			if (i + 1 < chunks.size()) {
				req.body += chunks[i][j];
			}
		}
		i++;
		std::istringstream(chunks[i]) >> std::hex >> chunk_size;
	}
	return 1;
}

int handle_multiple_form_data(HttpRequest &req, RoutingResult &ser)
{
	std::string boundary = req.getBoundary();
	std::cout << "Boundary: " << boundary << std::endl;
	std::cout << req.body << std::endl;
	std::vector<std::string> parts = split(req.body, "--" + boundary);
	// for (std::vector<std::string>::iterator it = parts.begin(); it != parts.end(); ++it)
	// {
	// 	std::cout << "Part: " << *it << "end" << std::endl;
	// }
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
				upload_dir = "/tmp/" + filename;
				fd = open(upload_dir.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
				if (fd < 0)
				{
					std::cerr << "Failed to open file: " << filename << std::endl;
					return 0; // Handle error appropriately
				}
			}
			if (write(fd, headers_and_body[1].c_str(), headers_and_body[1].length()) < 0)
			{
				std::cerr << "Failed to write to file: " << filename << std::endl;
				close(fd);
				return 0; // Handle error appropriately
			}
		}
		else
		{
			n = headers_and_body[0].find("name=\"");
			std::string filename = headers_and_body[0].substr(n + 7, (headers_and_body[0].find("\"", n + 7)) - (n + 7));
			std::cout << "Filename: " << filename << std::endl;
			int fd = open(filename.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
			if (fd < 0)
			{
				std::cerr << "Failed to open file: " << filename << std::endl;
				return 0; // Handle error appropriately
			}
			if (write(fd, headers_and_body[1].c_str(), headers_and_body[1].length()) < 0)
			{
				std::cerr << "Failed to write to file: " << filename << std::endl;
				close(fd);
				return 0; // Handle error appropriately
			}
		}
	}
	return 1; // Indicate success
}

int posthandler(HttpRequest *req, RoutingResult *ser, HttpResponse &res)
{
	if (req->getContentType() == "multipart/form-data")
    {
		if (!req->getTransferEncoding().empty())
		{
			if(!parsechunked(*req, *ser))
            {
				res.setTextBody("<h1>400 Bad Request</h1>");
				res.statusCode = 400;
				res.statusMessage = "Bad Request";
				return 0;
			}
		}
        else if (!req->getContentLength().empty())
		{
            if (!handle_multiple_form_data(*req, *ser))
			{
				res.setTextBody("<h1>400 Bad Request</h1>");
				res.statusCode = 400;
				res.statusMessage = "Bad Request";
				return 0;
			}

		}
        else
		{
			res.setTextBody("<h1>400 Bad Request</h1>");
			res.statusCode = 400;
			res.statusMessage = "Bad Request";
			return 0;
		}
    }
	return 1;
}