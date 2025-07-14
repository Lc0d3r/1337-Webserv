CPP = c++
CPPFLAGS = -Wall -Wextra -Werror -std=c++98

SRCS = main.cpp Cgi.cpp cgi_utils.cpp
TARGET = test

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CPP) $(CPPFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)

re: clean all