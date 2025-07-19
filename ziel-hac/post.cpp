#include "cgi.hpp"

int parsechunked(request &req, server ser) //<-- i need to parse chuncked body, for both cgi and non cgi, put to bodies one in the request to usit in cgi and the in the upload file in the conf.file 
{
	std::vector<std::string> chunks = split(req.getBody(), "\r\n");
	std::string upload_file = ser.getUploadFile() + "/uploads.txt";
	int fd = open(upload_file.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0644);
	if (fd < 0)
	{
		std::cerr << "Failed to open upload file." << std::endl;
		fd = open("/tmp/upload.txt", O_RDWR | O_CREAT | O_TRUNC, 0644);
		if (fd < 0)
			return (std::cerr << "Failed to open upload file." << std::endl, 0);
	}
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
	return 1;
}

int posthandler(request *req, response *res)
{
    if (req->getContenttype() == "multipart/form-data")
    {
        if (!req->getTransferEncoding().empty())
            if(!parsechunked(*req, *res))
                return 0; // Handle error appropriately
        else if (req->getContentLength())
        {
            parsecontentlength(*req, *res);
        }
        else
        {
            std::cerr << "bad request" << std::endl;
        }
    }
}