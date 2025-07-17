CXX = c++
CXXFLAGS = #-std=c++98 -Wall -Wextra -Iysahraou

SRCS = main.cpp ysahraou/sockets.cpp ysahraou/HttpRequest.cpp
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
