NAME = webserv

CXX = c++
CXXFLAGS = -std=c++98 -Iysahraou -Wall -Wextra -Werror
RM = rm -rf

SRC = main.cpp ysahraou/sockets.cpp ysahraou/HttpRequest.cpp \
		abel-baz/Config.cpp abel-baz/ParseLocation.cpp abel-baz/Parser.cpp \
		abel-baz/Parser_utils.cpp abel-baz/Router.cpp abel-baz/Tokenizer.cpp \
		ysahraou/HttpResponse.cpp ziel-hac/cgi_utils.cpp \
		ziel-hac/post.cpp  ysahraou/utils.cpp ziel-hac/Cgi.cpp

OBJ = $(SRC:.cpp=.o)

BOLD      = \e[1m
CGREEN    = \e[32m

all: $(NAME)

%.o: %.cpp
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(NAME): $(OBJ)
	@echo "$(BOLD)$(CGREEN)building the project...\e[0m"
	@$(CXX) $(CXXFLAGS) $(OBJ) -o $(NAME)

clean:
	@echo "$(BOLD)$(CGREEN)cleaning ...\033[0m"
	@$(RM) $(OBJ)

fclean: clean 
	@$(RM) $(NAME)

re: fclean all 

.PHONY: all clean fclean re

.SECONDARY: