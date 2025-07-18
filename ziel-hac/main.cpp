#include <iostream>
#include <string>
#include "cgi.hpp"
#include "cgi_utils.hpp"
#include <sstream>
#include <fcntl.h>
#include <unistd.h>


int parsechunked(request &req, server ser) //<-- i need to parse chuncked body, for both cgi and non cgi, put to bodies one in the request to usit in cgi and the in the upload file in the conf.file 
{
	std::vector<std::string> chunks = split(req.getBody(), "\r\n");
	std::string upload_file = ser.getUploadFile() + "/uploads.txt";
	std::cout << "Upload file: " << upload_file << std::endl;
	int fd = open(upload_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd < 0)
	{
		std::cerr << "Failed to open upload file." << std::endl;
		fd = open("/tmp/upload.txt", O_RDONLY | O_CREAT | O_TRUNC, 0644);
		if (fd < 0)
		{
			std::cerr << "Failed to open upload file." << std::endl;
			return -1; // Error opening file
		}
	}
	std::cout << "fd: " << fd << std::endl;
	long chunk_size = 0;
	int i = 0;
	std::istringstream(chunks[i]) >> std::hex >> chunk_size;
	while(chunk_size > 0)
	{
		i++;
		write(fd, chunks[i].c_str(), chunk_size);
		i++;
		std::istringstream(chunks[i]) >> std::hex >> chunk_size;
	}
	return close(fd);
}

int main() {
	server serv;
	request req;
	int i = parsechunked(req, serv);
	std::cout << "Parsed " << i << " bytes." << std::endl;
	return 0;
}