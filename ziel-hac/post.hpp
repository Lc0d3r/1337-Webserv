#ifndef POST_HPP
#define POST_HPP

#include "../ysahraou/HttpRequest.hpp"
#include "../abel-baz/Router.hpp"
#include <sstream>
#include <fcntl.h>
#include <unistd.h>

// Forward declaration
struct HttpResponse;


int posthandler(HttpRequest *req, RoutingResult *ser, HttpResponse &res);
int handle_multiple_form_data(HttpRequest &req, RoutingResult &ser);
int parsechunked(HttpRequest &req, RoutingResult &ser);

#endif