#include "cgi.hpp"

Cgi::Cgi(server *serv, request *req)
{
	
	if (!_checker(serv, req))//<-- TO DO!
	{
		setEnv(serv, req);
		_mergeEnv();//<-- TO DO!
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

response	Cgi::_executeScript(server *serv, request *req)
{
	response res;
	pipe(output_fd);
	pipe(input_fd);
	pid_t pid = fork();
	if (pid < 0)
	{
		std::cerr << "Fork failed." << std::endl;
		return NULL; // Fork failed
	}
	if(pid == 0)
	{
		if (req->getMethod() == "POST")
		{
			if (req->getBody().empty())
			{
				std::cerr << "No body to send for POST request." << std::endl;
				exit(1); // Exit child process on error
			}
			else
			{
				if (dup2(input_fd[0], STDIN_FILENO) < 0)
				{
					std::cerr << "Failed to redirect stdin." << std::endl;
					exit(1); // Exit child process on error
				}
			}
		}
		if (dup2(output_fd[1], STDOUT_FILENO) < 0)
		{
			std::cerr << "Failed to redirect stdout." << std::endl;
			exit(1); // Exit child process on error
		}
		if (req->getMethod() == "POST" && !req->getBody().empty())
			write(input_fd[1], req->getBody().c_str(), req->getBody().length());
		close(input_fd[1]);
		close(output_fd[0]);
		close(output_fd[1]);
		close(input_fd[0]);
		char *argv[3];
		argv[0] = const_cast<char *>(serv->getScriptPath().c_str());
		argv[1] = const_cast<char *>(req->getPath().c_str());
		argv[2] = NULL;
		if (execve(argv[0], argv, _envc) < 0)
		{
			std::cerr << "Failed to execute script: " << serv->getScriptPath() << std::endl;
			exit(1); // Exit child process on error
		}
	}
	else if (pid > 0)
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
	}
	else
	{
		std::cerr << "Unexpected fork return value." << std::endl;
		return 0; // Unexpected fork return value
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
			return 0; // Memory allocation failure
		}
		strcpy(_envc[i], _envVector[i].c_str());
	}
	return 1; // Successfully merged environment variables
}

Cgi::Cgi()
{
}


bool Cgi::_check_extra_path(request *rep)
{
	std::string path = rep->getPath();//<-- TO DO!
	if (path.find(rep->getExtension()) != std::string::npos)//<-- TO DO!
	{
		int pos = path.find(rep->getExtension());//<-- TO DO!
		if (path[pos + rep->getExtension().length()] == '/')//<-- TO DO!
		{
			return true;
		}
		else 
			return false;
	}
	return false;
}

std::string getExtraPath(const std::string &path, server *serv)
{
	int pos = path.find(serv->getExtention());//<-- TO DO!
	pos = pos + serv->getExtention().length();//<-- TO DO!
	return path.substr(pos);
}

void Cgi::setEnv(server *serv, request *req)
{
	std::cout << "Setting environment variables for CGI..." << std::endl;
	if (_check_extra_path(req))
		_tmpEnv["PATH_INFO"] = getExtraPath(req->getPath(), serv);
	_tmpEnv["AUTH_TYPE"] = "Basic";
	_tmpEnv["SERVER_NAME"] = serv->getServerName();//<-- DONE!
	_tmpEnv["SERVER_PORT"] = serv->getPort();//<-- DONE!
	_tmpEnv["SERVER_PROTOCOL"] = "HTTP/1.1";
	_tmpEnv["GATEWAY_INTERFACE"] = "CGI/1.1";
	_tmpEnv["SERVER_SOFTWARE"] = "WebServ/1.0";
	_tmpEnv["SCRIPT_NAME"] = req->getPath();//<-- TO DO!
	_tmpEnv["REQUEST_METHOD"] = req->getMethod();//<-- TO DO!
	_tmpEnv["QUERY_STRING"] = req->getQueryString();//<-- TO DO!
	_tmpEnv["DOCUMENT_ROOT"] = serv->getDocumentRoot();//<-- DONE!
	_tmpEnv["SCRIPT_FILENAME"] = req->getScriptFilename();// <-- ALMOST DONE!
	_tmpEnv["REDIRECT_STATUS"] = "200";
	_tmpEnv["CONTENT_TYPE"] = req->getContentType();//<-- TO DO!
	_tmpEnv["CONTENT_LENGTH"] = req->getContentLength();//<-- TO DO!
	_tmpEnv["HTTP_COOKIE"] = req->getCookie();//<-- TO DO!
}

int Cgi::_checker(server *serv,request *req)
{
	// if (!_checkInterpreterScrpt(serv))
	// {
	// 	std::cout << "Script path is not valid or not executable." << std::endl;
	// 	return 1;
	// }
	if (!_checkExtention(req->getPath(), serv->getExtention()) && !_checkInterpreter(req->getExtantion(), serv->getScriptPath()))
	{
		std::cout << "Script path is not valid or not executable.2" << std::endl;
		return 1;
	}
	return 0;
}

int Cgi::_checkInterpreterScrpt(server *serv)
{
	struct stat _stat;
	stat(serv->getScriptPath().c_str(), &_stat);
	if (S_ISDIR(_stat.st_mode)) //<-- checks is the script path is a directory
    	return 0;
	else
	{
		if (access(serv->getScriptPath().c_str(), F_OK | X_OK) != 0)
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

/*
TO DO!:
- i need to merge the variables in the map _tmpEnv to the char **_envc using a vector

*/
