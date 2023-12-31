CC = g++
# CFLAGS = -std=c++14 -Wall -Wextra -static -O3
 CFLAGS = -std=c++14 -Wall -Wextra -g
EXECUTABLE = bwtsearch

# List of source files
SRCS = main.cpp utility.cpp bwtsearch.cpp

# List of object files to be generated
OBJS = $(SRCS:.cpp=.o)

# Rule for compiling .cpp files into object files
%.o: %.cpp
	$(CC) $(CFLAGS) -c $< -o $@

# Rule for linking object files into the executable
$(EXECUTABLE): $(OBJS)
#	$(CC) $(CFLAGS) -static $(OBJS) -o $@
	$(CC) $(CFLAGS) $(OBJS) -o $@

# Rule to clean generated files
clean:
	rm -f $(EXECUTABLE) $(OBJS)
	rm -f index.idx