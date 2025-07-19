#include <iostream>
#include "Tokenizer.hpp"
#include "Parser.hpp"
#include "Router.hpp"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: ./webserv <config_file>" << std::endl;
        return 1;
    }

    try {
        Tokenizer tokenizer(argv[1]);
        Parser parser(tokenizer.tokenize());
        Config config = parser.parse();

        RoutingResult result = routingResult(config, "localhost", 8080, "/docs/index.html", "DELETE");

        std::cout << "Server count: " << result.server_count << std::endl;
        if (result.is_redirect)
            std::cout << "Redirect to: " << result.redirect_url << std::endl;
        else if (result.use_autoindex)
            std::cout << "Autoindex enabled for: " << result.file_path << std::endl;
        else
            std::cout << "Serve file: " << result.file_path << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
