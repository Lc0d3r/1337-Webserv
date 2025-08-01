#pragma once

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
#include "../ysahraou/HttpRequest.hpp"
#include "../abel-baz/Router.hpp"

// Forward declaration
struct HttpResponse;





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
		int									getvalidChecker() const;
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
