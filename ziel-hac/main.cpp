#include <iostream>
#include <string>
#include "cgi.hpp"



int main() {
	server serv;
	request req;

	Cgi cgi(&serv, &req);
	

	std::cout << "\n--- Environment Variables in _envc ---\n";
	cgi._printEnv();

	return 0;
}