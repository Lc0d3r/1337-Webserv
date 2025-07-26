CXX = c++
CXXFLAGS = -std=c++98 -Wall -Wextra -Iysahraou

SRCS = main.cpp ysahraou/sockets.cpp ysahraou/HttpRequest.cpp \
abel-baz/Config.cpp abel-baz/ParseLocation.cpp abel-baz/Parser.cpp \
abel-baz/Parser_utils.cpp abel-baz/Router.cpp abel-baz/Tokenizer.cpp \
ysahraou/HttpResponse.cpp ziel-hac/cgi_utils.cpp \
ziel-hac/post.cpp 


OBJS = $(SRCS:.cpp=.o)

TARGET = server

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(TARGET)

re: fclean all

.PHONY: all clean fclean re
