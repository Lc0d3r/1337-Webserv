#include "cgi.hpp"

Cgi::Cgi(RoutingResult *serv, HttpRequest *req)
{
	
	if (!_checker(serv, req))
	{
		setEnv(serv, req);
		_mergeEnv();
		if (!_executeScript())
		{
			std::cerr << "Failed to execute CGI script." << std::endl;
			return; // Handle error appropriately
		}
	}
	else
	{
		std::cerr << "CGI extension is not valid." << std::endl;// <-- I need to trow an exception here
		return;
	}

}

int	Cgi::_executeScript(RoutingResult *serv, HttpRequest *req, HttpResponse &res)
{
	pipe(output_fd);
	pipe(input_fd);
	pid_t pid = fork();
	if (pid < 0)
	{
		res.setBody("Internal Server Error");
		res.statusCode = 500;
		res.statusMessage = "Internal Server Error";
		return (0);
	}
	if(pid == 0)
	{
		if (req->method == "POST")
		{
			if (req->body.empty())
			{
				res.setBody("<h1>404 Not Found</h1>");
				res.statusCode = 404;
				res.statusMessage = "Not Found";
				return (0);
			}
			else
			{
				if (dup2(input_fd[0], STDIN_FILENO) < 0)
				{
					res.setBody("Internal Server Error");
					res.statusCode = 500;
					res.statusMessage = "Internal Server Error";
					return (0);
				}
			}
		}
		if (dup2(output_fd[1], STDOUT_FILENO) < 0)
		{
			res.setBody("Internal Server Error");
			res.statusCode = 500;
			res.statusMessage = "Internal Server Error";
			return (0);
		}
		if (req->method == "POST" && !req->body.empty())
			write(input_fd[1], req->body.c_str(), req->body.length());
		close(input_fd[1]);
		close(output_fd[0]);
		close(output_fd[1]);
		close(input_fd[0]);
		char *argv[3];
		argv[0] = const_cast<char *>(serv->getScriptPath().c_str());
		argv[1] = const_cast<char *>(req->path.c_str());
		argv[2] = NULL;
		if (execve(argv[0], argv, _envc) < 0)
		{
			res.setBody("Internal Server Error");
			res.statusCode = 500;
			res.statusMessage = "Internal Server Error";
			return (0);
		}
	}
	else
	{
		close(input_fd[0]);
		close(output_fd[1]);
		char buffer[1024];
		int bytesRead;
		while ((bytesRead = read(output_fd[0], buffer, sizeof(buffer) - 1)) > 0)
		{
			buffer[bytesRead] = '\0'; 
			write(STDOUT_FILENO, buffer, bytesRead);
			res.body += std::string(buffer, bytesRead);
		}
		res.headers["Content-Length"] = std::to_string(res.body.length());
	}
}

int	Cgi::_mergeEnv()
{
	std::map<std::string, std::string>::iterator it;
	for (it = _tmpEnv.begin(); it != _tmpEnv.end(); ++it)
	{
		std::string envVar = it->first + "=" + it->second;
		_envVector.push_back(envVar);
	}
	_envc = new char*[_envVector.size() + 1];
	_envc[_envVector.size()] = NULL; // Null-terminate the array of strings
	if (!_envc)
	{
		std::cerr << "Memory allocation failed for environment variables." << std::endl;
		return 0; // Memory allocation failure
	}
	for (size_t i = 0; i < _envVector.size(); ++i)
	{
		_envc[i] = new char[_envVector[i].length() + 1];
		if (!_envc[i])
		{
			std::cerr << "Memory allocation failed for environment variable: " << _envVector[i] << std::endl;
			return 0;
		}
		strcpy(_envc[i], _envVector[i].c_str());
	}
	return 1; 
}



bool Cgi::_check_extra_path(HttpRequest *req)
{
	std::string path = req->path;
	if (path.find(req->getExtension()) != std::string::npos)
	{
		int pos = path.find(req->getExtension());
		if (path[pos + req->getExtension().length()] == '/')
		{
			return true;
		}
		else 
			return false;
	}
	return false;
}

std::string getExtraPath(const std::string &path, RoutingResult *serv)
{
	int pos = path.find(serv->getExtension());
	pos = pos + serv->getExtension().length();
	return path.substr(pos);
}

std::string Cgi::getScriptFilename(RoutingResult *serv)
{
	if (req->getExtension() == ".php")
		return ("/usr/bin/php-cgi");
	return ("/usr/bin/python3");
}

void Cgi::setEnv(RoutingResult *serv, HttpRequest *req)
{
	std::cout << "Setting environment variables for CGI..." << std::endl;
	if (_check_extra_path(req))
		_tmpEnv["PATH_INFO"] = getExtraPath(req.path, serv);
	_tmpEnv["AUTH_TYPE"] = "Basic";
	_tmpEnv["SERVER_NAME"] = serv->getServerName();
	_tmpEnv["SERVER_PORT"] = req->getPort(); // <-- TO DO! 
	_tmpEnv["SERVER_PROTOCOL"] = "HTTP/1.1";
	_tmpEnv["GATEWAY_INTERFACE"] = "CGI/1.1";
	_tmpEnv["SERVER_SOFTWARE"] = "WebServ/1.0";
	_tmpEnv["SCRIPT_NAME"] = req->path;
	_tmpEnv["REQUEST_METHOD"] = req->method;
	_tmpEnv["QUERY_STRING"] = req->getQueryString();
	_tmpEnv["DOCUMENT_ROOT"] = serv->getDocumentRoot();
	_tmpEnv["SCRIPT_FILENAME"] = req->getScriptFilename();
	_tmpEnv["REDIRECT_STATUS"] = "200";
	_tmpEnv["CONTENT_TYPE"] = req->getContentType();
	_tmpEnv["CONTENT_LENGTH"] = req->getContentLength();
	_tmpEnv["HTTP_COOKIE"] = req->getCookie();
	_tmpEnv["SCRIPT_FILENAME"] = getScriptFilename();
}

int Cgi::_checker(RoutingResult *serv, HttpRequest *req)
{
	if (!_checkExtention(req->path, serv->getExtention()) && !_checkInterpreter(req->getExtantion(), serv->getScriptPath()))
	{
		std::cout << "Script path is not valid or not executable." << std::endl;
		return 1;
	}
	return 0;
}

int Cgi::_checkInterpreterScrpt(RoutingResult *serv)
{
	struct stat _stat;
	stat(getScriptFilename(serv).c_str(), &_stat);
	if (S_ISDIR(_stat.st_mode)) //<-- checks is the script path is a directory
    	return 0;
	else
	{
		if (access(getScriptFilename(serv).c_str(), F_OK | X_OK) != 0)
			return 0; //<-- checks if the script path exists and is executable
	}
	return 1; //<-- if the script path is not a directory and exists and is executable, return 1

}

int Cgi::_checkInterpreter(const std::string &ext, const std::string &interpreter) // checks if the interpreter matches the extension
{
	std::vector<std::string> extVector = split(interpreter, "/");
	for (size_t i = 0; i < extVector.size(); ++i)
		if (i == extVector.size() - 1 && ((extVector[i] == "cgi-php" && ext == ".php") || (extVector[i] == "python3" && ext == ".py")))
			return 1;
	return 0;
}

int		Cgi::_checkExtention(const std::string &path, const std::string &ext)// checks if the path ends with the given extension
{

	if (path.find(ext) != std::string::npos)
	{
		int pos = path.find(ext);
		if (path[pos + ext.length()] == '\0' || path[pos + ext.length()] == '/')
			return 1;
		else
			return 0;
	}
	return 0;
}

void Cgi::_printEnv()
{
	std::cout << "Environment Variables:" << std::endl;
	for (size_t i = 0; _envc[i] != NULL; ++i)
	{
		std::cout << _envc[i] << std::endl;
	}
}

Cgi::~Cgi()
{
}

