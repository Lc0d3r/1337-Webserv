#ifndef CGI_HPP
#define CGI_HPP

#include <iostream>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <string>
#include <map>
#include "cgi_utils.hpp"
#include <sstream>
#include <fcntl.h>
#include <unistd.h>
#include "HttpResponse.hpp"
#include "HttpRequest.hpp"
#include "../abel-baz/Router.hpp"


// Dummy server class
// class server {
// public:
// 	std::string getServerName() const { return "localhost"; }
// 	std::string getDocumentRoot() const { return "/var/www/html"; }
// 	std::string getExtention() const { return ".php"; }
// 	std::string getScriptPath() const { return "/usr/bin/cgi-php"; }
// 	std::string getPort() const { return "8080"; }
// 	std::string getUploadFile() const { return "/var/www/uploads"; } // Simulated upload directory
// };

// // Dummy request class
// struct request {
// 	    std::string method;          // e.g., "GET"
//     std::string path;            // e.g., "/index.html"  
//     std::string http_version;    // e.g., "HTTP/1.1"
//     std::map<std::string, std::string> headers;
//     std::string body; // the body
	
// 	std::string getPath() const { return "/cgi-bin/script.php"; }
// 	std::string getExtension() const { return ".php"; }
// 	std::string getMethod() const { return "GET"; }
// 	std::string getQueryString() const { return "id=42&name=test"; }
// 	std::string getContentType() const { return "text/html"; }
// 	std::string getCookie() const { return "sessionid=abc123"; }
// 	std::string getContentLength() const { return "0"; } // Simulate no body
// 	std::string getExtantion() const { return ".php"; } // Typo in class, must match your real implementation
// 	std::string getScriptFilename() const { return "/var/www/html/script.php"; }
// 	std::string getBody() const { return "4\r\nrestnigga\r\n5\r\nlesssnigga\r\na\r\n1234567890nigga\r\n0\r\n\r\n"; }
// 	// std::string getBody() const { 
// 	// 	return "--WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: form-data; name=\"username\"\r\n\r\n1337\r\n--WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: form-data; name=\"younesszfat\"\r\n\r\njohn_pork\r\n--WebKitFormBoundary7MA4YWxkTrZu0gW\r\nContent-Disposition: form-data; name=\"ziadwa3r\"\r\n\r\nniggaballs\r\n--WebKitFormBoundary7MA4YWxkTrZu0gW--\r\n"; 
// 	// }
// 	std::string getBoundary() const { return "WebKitFormBoundary7MA4YWxkTrZu0gW"; } // Simulated boundary
// 	std::string getTransferEncoding() const { return "chuncked"; } // Simulated transfer encoding
// 	std::string getContenttype() const { return "multipart/form-data"; } // Simulated content type
// };


class Cgi
{
    private:
		std::map<std::string, std::string>	_tmpEnv;
		char								**_envc;
		std::vector<std::string>			_envVector;
		int									output_fd[2];
		int									input_fd[2];
		int									valid_checker;

    public:
		std::string							getScriptFilename(HttpRequest &req) const;
		void								setEnv(RoutingResult &serv, HttpRequest &req);
		bool								_check_extra_path(HttpRequest &rep);
		int									_checker(RoutingResult &serv, HttpRequest &req, HttpResponse &res);
		int									_checkExtention(const std::string &path, const std::vector<std::string> &ext);
		int									_checkInterpreter(const std::string &ext, const std::string &interpreter);
		int									_checkInterpreterScrpt(HttpRequest &req);
		int									_checkPathExtension(const std::string &ext, const std::string &interpreter);
		int									_executeScript(RoutingResult &serv, HttpRequest &req, HttpResponse &res);
		int									_mergeEnv();
		void								_printEnv();
		Cgi(RoutingResult &serv, HttpRequest &req, HttpResponse &res);
		~Cgi();
};


#endif