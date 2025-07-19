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

// Dummy server class
class server {
public:
	std::string getServerName() const { return "localhost"; }
	std::string getDocumentRoot() const { return "/var/www/html"; }
	std::string getExtention() const { return ".php"; }
	std::string getScriptPath() const { return "/usr/bin/cgi-php"; }
	std::string getPort() const { return "8080"; }
	std::string getUploadFile() const { return "/var/www/uploads"; } // Simulated upload directory
};

// Dummy request class
class request {
public:
	std::string getPath() const { return "/cgi-bin/script.php"; }
	std::string getExtension() const { return ".php"; }
	std::string getMethod() const { return "GET"; }
	std::string getQueryString() const { return "id=42&name=test"; }
	std::string getContentType() const { return "text/html"; }
	std::string getCookie() const { return "sessionid=abc123"; }
	std::string getContentLength() const { return "10"; } // Simulate no body
	std::string getExtantion() const { return ".php"; } // Typo in class, must match your real implementation
	std::string getScriptFilename() const { return "/var/www/html/script.php"; }
	std::string getBody() const { return "4\r\nrestnigga\r\n5\r\nlesssnigga\r\na\r\n1234567890nigga\r\n0\r\n\r\n"; } 
	std::string getTransferEncoding() const { return "chunked"; } // Simulated transfer encoding
	std::string getContenttype() const { return "multipart/form-data"; } // Simulated content type
};


class Cgi
{
    private:
		std::map<std::string, std::string>	_tmpEnv;
		char								**_envc;
		std::vector<std::string>			_envVector;
		int									output_fd[2];
		int									input_fd[2];

    public:
		void								setEnv(server *serv, request *req);
		bool								_check_extra_path(request *rep);
		int									_checker(server *serv,request *req);
		int									_checkExtention(const std::string &path, const std::string &ext);
		int									_checkInterpreter(const std::string &ext, const std::string &interpreter);
		int									_checkInterpreterScrpt(server *serv);
		int									_executeScript(server *serv, request *req);
		int									_mergeEnv();
		void								_printEnv();
		Cgi();
		Cgi(server *serv, request *req);
		~Cgi();
};

#endif

/*
"CONTENT_LENGTH" x |
"CONTENT_TYPE" x | "GATEWAY_INTERFACE" x |
"PATH_INFO" x | "PATH_TRANSLATED" x |
"QUERY_STRING" x | "REMOTE_ADDR" x |
"REMOTE_HOST" x | "REMOTE_IDENT" x |
"REMOTE_USER" x | "REQUEST_METHOD" x |
"SCRIPT_NAME" x | "SERVER_NAME" x |
"SERVER_PORT" x | "SERVER_PROTOCOL" x |
"SERVER_SOFTWARE" x
*/