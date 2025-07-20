CXX = c++
CXXFLAGS = #-std=c++98 -Wall -Wextra -Iysahraou

SRCS = main.cpp ysahraou/sockets.cpp ysahraou/HttpRequest.cpp \
abel-baz/Config.cpp abel-baz/ParseLocation.cpp abel-baz/Parser.cpp \
abel-baz/Parser_utils.cpp abel-baz/Router.cpp abel-baz/Tokenizer.cpp
OBJS = $(SRCS:.cpp=.o)

TARGET = server

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
