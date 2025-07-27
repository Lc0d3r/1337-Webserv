#include "Config.hpp"
#include "Router.hpp"


int Config::getKeepAliveTimeout(std::string host, int port) const {
        ServerConfig server = matchServer(*this, host, port);
        if (server.keep_alive_timeout > 0) {    
            return server.keep_alive_timeout;
        }
        return 10; // default keep-alive timeout
}
