#pragma once

#include <vector>
#include <string>
#include <cstdlib>
#include <set>
#include "Tokenizer.hpp"
#include "Config.hpp"

class Parser
{
    public:
        Parser(const std::vector<Token>& tokens) 
            : _tokens(tokens), _index(0) {};
        Config parse();

    private:
        std::vector<Token> _tokens;
        size_t _index;

        Token get();
        Token peek();

        // Helper functions
        void parseServer(Config& config);
        void parseListen(ServerConfig& server);
        void parseServerName(ServerConfig& server);
        void parseLocation(ServerConfig& server);
        void parseLocationRoot(LocationConfig& loc);
        void parseLocationIndex(LocationConfig& loc);
        void parseLocationAutoindex(LocationConfig& loc);
        void parseLocationMethods(LocationConfig& loc);
        void parseLocationUpload(LocationConfig& loc);
        void parseLocationRedirect(LocationConfig& loc);
        void parseLocationCGI(LocationConfig& loc);
        void parseErrorPage(ServerConfig& server);
        void parseMaxBodySize(ServerConfig& server);
        void parseKeepAlive(ServerConfig& server);

};

