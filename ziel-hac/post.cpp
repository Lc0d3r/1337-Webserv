#include "cgi.hpp"

int posthandler(request *req, response *res)
{
    if (req->getContenttype() == "multipart/form-data")
    {
        if (req->getTransferEncoding())
        {
            
        }
        else if (req->getContentLength())
        {
            // Process multipart/form-data
        }
        else
        {
            std::cerr << "bad request" << std::endl;
        }
    }
}