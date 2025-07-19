NAME = webserv

CXX = c++
CXXFLAGS = -std=c++98 -Wall -Wextra -Werror
RM = rm -rf

SRC = main.cpp Tokenizer.cpp Parser.cpp Parser_utils.cpp  ParseLocation.cpp Router.cpp \

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