CFLAGS  = -std=c++11 -Wall -Werror -pedantic -fsanitize=address,undefined
SRC     = main.cc
CC      = g++

all: $(SRC)
	$(CC) $(CFLAGS) $^
