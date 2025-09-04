#include "cgi.hpp"
#include "../ysahraou/HttpResponse.hpp"
#include <sys/wait.h>
#include "../ysahraou/utils.hpp"

int Cgi::getvalidChecker() const
{
	return valid_checker;
}


Cgi::Cgi(RoutingResult &serv, HttpRequest &req, HttpResponse &res):_envc(NULL), valid_checker(1) 
{
	if (!_checker(serv, req, res))
	{
		setEnv(serv, req);
		_mergeEnv();
	}
	else
	{
		valid_checker = 0;
		return;
	}

}

int	Cgi::_executeScript(RoutingResult &serv, HttpRequest &req, HttpResponse &res)
{
	int timeout = 1;
	int status;
	int result; 
	std::string body;
	pipe(output_fd);
	pipe(input_fd);
	pid_t pid = fork();
	if (pid < 0)
	{
		if (!get_error_page(res, 500, req, "Internal Server Error")) {
			res.setTextBody("<h1>500 Internal Server Error</h1>");
			res.statusCode = 500;
			res.statusMessage = "Internal Server Error";
			res.addHeader("Content-Length", intToString(res.body.size()));
			res.addHeader("Content-Type", "text/html");
			if (req.is_keep_alive) {
				res.addHeader("Connection", "keep-alive");
			} else {
				res.addHeader("Connection", "close");
			}
		}
		return (0);
	}
	if(pid == 0)
	{
		if (req.method == "POST")
		{
			if (req.body.empty())
				exit(2);
			else
			{
				if (dup2(input_fd[0], STDIN_FILENO) < 0)
					exit(EXIT_FAILURE);
			}
		}
		if (dup2(output_fd[1], STDOUT_FILENO) < 0)
			exit(EXIT_FAILURE);
		close(input_fd[1]);
		close(output_fd[0]);
		close(output_fd[1]);
		close(input_fd[0]);
		char *argv[3];
		std::string scriptFilename = getScriptFilename(req);
		argv[0] = const_cast<char *>(scriptFilename.c_str());
		argv[1] = const_cast<char *>(serv.file_path.c_str());
		argv[2] = NULL;
		if (execve(argv[0], argv, _envc) < 0)
		{
			std::cout << "cscsc------------------------------------------------------------------------------" << std::endl;
			exit(EXIT_FAILURE);
		}
	}
	else
	{
		result = waitpid(pid, &status, WNOHANG);
		if (result == 0)
		usleep(500000);
		result = waitpid(pid, &status, WNOHANG);
		if (result == 0)
		{
			kill(pid, SIGKILL);
			if(!get_error_page(res, 504, req, "Gateway Timeout")) {
				std::cout << "CGI script timed out after " << timeout << " seconds." << std::endl;
				res.statusCode = 504; // Gateway Timeout
				res.statusMessage = "Gateway Timeout";
				res.setTextBody("<h1>504 Gateway Timeout</h1>");
				res.addHeader("Content-Length", intToString(res.body.size()));
				res.addHeader("Content-Type", "text/html");
				if (req.is_keep_alive) {
					res.addHeader("Connection", "keep-alive");
				} else {
					res.addHeader("Connection", "close");
				}

				print_log("CGI script timed out after " + intToString(timeout) + " seconds.", DiSPLAY_LOG);
			}
			return 0; // Timeout occurred
		}
		if (WIFEXITED(status))
		{
			int exitStatus = WEXITSTATUS(status);
			if (exitStatus == 1)
			{
					if (!get_error_page(res, 500, req, "Internal Server Error")) {
					res.statusCode = 500;
					res.statusMessage = "Internal Server Error";
					res.setTextBody("<h1>500 Internal Server Error</h1>");
					res.addHeader("Content-Length", intToString(res.body.size()));
					res.addHeader("Content-Type", "text/html");
					if (req.is_keep_alive)
						res.addHeader("Connection", "keep-alive");
					else 
						res.addHeader("Connection", "close");
					return 0;
				}
			}
			else if (exitStatus == 2)
			{
				if (!get_error_page(res, 404, req, "Not Found")) {
					res.setTextBody("<h1>404 Not Found</h1>");
					res.statusCode = 404;
					res.statusMessage = "Not Found";
					res.addHeader("Content-Length", intToString(res.body.size()));
					res.addHeader("Content-Type", "text/html");
					if (req.is_keep_alive) 
						res.addHeader("Connection", "keep-alive");
					else 
						res.addHeader("Connection", "close");
					return 0;
				}
			}
		}
		if (req.method == "POST" && !req.body.empty())
			write(input_fd[1], req.body.c_str(), req.body.length());
		close(input_fd[0]);
		close(output_fd[1]);
		close(input_fd[1]);
		char buffer[1024];
		int bytesRead;
		while ((bytesRead = read(output_fd[0], buffer, sizeof(buffer) - 1)) > 0)
		{
			buffer[bytesRead] = '\0'; 
			body += std::string(buffer, bytesRead);
		}
		res.setTextBody(body);
		res.headers["Content-Length"] = intToString(body.length());
		close(output_fd[0]);

	}
	return 1; // Add return statement
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
	if (!_envc)
	{
		std::cerr << "Memory allocation failed for environment variables." << std::endl;
		return 0;
	}
	_envc[_envVector.size()] = NULL;
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



bool Cgi::_check_extra_path(HttpRequest &req)
{
	std::string path = req.path;
	if (path.find(req.getExtension()) != std::string::npos)
	{
		int pos = path.find(req.getExtension());
		if (path[pos + req.getExtension().length()] == '/')
		{
			return true;
		}
		else 
			return false;
	}
	return false;
}

std::string getExtraPath(const std::string &path, HttpRequest &req)
{
	int pos = path.find(req.getExtension());
	pos = pos + req.getExtension().length();
	return path.substr(pos);
}

std::string Cgi::getScriptFilename(HttpRequest &req) const
{
	if (req.getExtension() == ".js")
		return ("/usr/bin/nodejs");
	return ("/usr/bin/python3");
}

void Cgi::setEnv(RoutingResult &serv, HttpRequest &req)
{
	if (_check_extra_path(req))
		_tmpEnv["PATH_INFO"] = getExtraPath(req.path, req);
	_tmpEnv["AUTH_TYPE"] = "Basic";
	_tmpEnv["SERVER_NAME"] = serv.getServerName();
	// _tmpEnv["SERVER_PORT"] = req.getPort(); // <-- TO DO! 
	_tmpEnv["SERVER_PROTOCOL"] = "HTTP/1.1";
	_tmpEnv["GATEWAY_INTERFACE"] = "CGI/1.1";
	_tmpEnv["SERVER_SOFTWARE"] = "WebServ/1.0";
	_tmpEnv["SCRIPT_NAME"] = req.path;
	_tmpEnv["REQUEST_METHOD"] = req.method;
	_tmpEnv["QUERY_STRING"] = req.getQueryString();
	_tmpEnv["DOCUMENT_ROOT"] = serv.getDocumentRoot();
	_tmpEnv["SCRIPT_FILENAME"] = serv.file_path;
	_tmpEnv["REDIRECT_STATUS"] = "200";
	_tmpEnv["CONTENT_TYPE"] = req.getContentType();
	_tmpEnv["CONTENT_LENGTH"] = req.getContentLength();
	_tmpEnv["HTTP_COOKIE"] = req.getCookie();
	_tmpEnv["INTERPRETER_SCRIPT"] = getScriptFilename(req);
	_tmpEnv["ACCOUNTS"] = intToString(cookies_map.size());
}
//DO:1.checks if the extension is in the conf file, 2.checks if the extension in the path is valid 3.checks if the interpreter is valid
//RETURN: 0 if all above is true, 1 if any of the above is false
int Cgi::_checker(RoutingResult &serv, HttpRequest &req, HttpResponse &res)
{
	if (!_checkExtention(req.path_without_query, serv.getExtension()) || !_checkPathExtension(req.getExtension(), getScriptFilename(req)))
	{
		if (get_error_page(res, 403, req, "Forbidden")) {
			res.statusCode = 403;
			res.statusMessage = "Forbidden";
			res.setTextBody("<h1>403 Forbidden</h1>");
			res.addHeader("Content-Length", intToString(res.body.size()));
			res.addHeader("Content-Type", "text/html");
			if (req.is_keep_alive) {
				res.addHeader("Connection", "keep-alive");
			} else {
				res.addHeader("Connection", "close");
			}
		}
		return 1; // Extension not allowed
	}
	else if (!_checkInterpreterScrpt(req))
	{
		res.statusCode = 502;
		res.statusMessage = "Bad Gateway";
		res.setTextBody("<h1>502 Bad Gateway</h1>");
		res.addHeader("Content-Length", intToString(res.body.size()));
		res.addHeader("Content-Type", "text/html");
		if (req.is_keep_alive) {
			res.addHeader("Connection", "keep-alive");
		} else {
			res.addHeader("Connection", "close");
		}
		return 1; // Interpreter not found or not executable
	}
	return 0;
}

int		Cgi::_checkExtention(const std::string &path, const std::vector<std::string> &ext)
{

	for (size_t i = 0; i < ext.size(); ++i)
	{
		if (path.find(ext[i]) != std::string::npos)
		{
			int pos = path.find(ext[i]);
			if (path[pos + ext[i].length()] == '\0' || path[pos + ext[i].length()] == '/')
				return 1;
			else
				return 0;
		}
	}
	return 0;
}

int Cgi::_checkPathExtension(const std::string &ext, const std::string &interpreter)
{
	std::vector<std::string> extVector = split(interpreter, "/");
	for (size_t i = 0; i < extVector.size(); ++i)
		if (i == extVector.size() - 1 && ((extVector[i] == "nodejs" && ext == ".js") || (extVector[i] == "python3" && ext == ".py")))
			return 1;
	return 0;
}

int Cgi::_checkInterpreterScrpt(HttpRequest &req)
{
	struct stat _stat;
	stat(getScriptFilename(req).c_str(), &_stat);
	if (S_ISDIR(_stat.st_mode)) //<-- checks is the script path is a directory
    	return 0;
	else
	{
		if (access(getScriptFilename(req).c_str(), F_OK | X_OK) != 0) // 
			return 0; //<-- checks if the script path exists and is executable
	}
	return 1; //<-- if the script path is not a directory and exists and is executable, return 1

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
	if (_envc != NULL && _envc[0] != NULL)
	{
		for (size_t i = 0; i < _envVector.size(); ++i)
			delete[] _envc[i];
		delete[] _envc;
	}
	else if (_envc != NULL)
		delete[] _envc;

}

