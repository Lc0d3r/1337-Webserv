#include <string>
#include <map>
#include "cgi.hpp"

std::map<std::string, struct cookies> cookies_map;

struct cookies {
    std::string theme;
};
