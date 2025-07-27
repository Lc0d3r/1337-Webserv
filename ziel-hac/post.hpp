#ifndef CGI_HPP
#define CGI_HPP

#include "../ysahraou/HttpRequest.hpp"
#include "../ysahraou/HttpResponse.hpp"
#include "../abel-baz/Router.hpp"


int posthandler(HttpRequest *req, RoutingResult *ser, HttpResponse &res);
int handle_multiple_form_data(HttpRequest &req, RoutingResult &ser);
int parsechunked(HttpRequest &req, RoutingResult &ser);

#endif