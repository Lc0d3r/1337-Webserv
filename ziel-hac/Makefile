CPP = c++
CPPFLAGS = -Wall -Wextra -Werror -std=c++98 -g3

SRCS = main.cpp cgi_utils.cpp post.cpp
TARGET = test

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CPP) $(CPPFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

re: clean all