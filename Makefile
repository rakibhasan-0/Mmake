CC = gcc 
FLAGS = -g -std=gnu11 -Werror -Wall -Wextra -Wpedantic -Wmissing-declarations -Wmissing-prototypes -Wold-style-definition



all: mmake.o parser.o
	$(CC) $(FLAGS) - o mmake.o parser.o

mmake.o: mmake.c 
	$(CC) $(FLAGS) -c mmake.c -o mmake.o

parser.o: parser.c parser.h
	$(CC) $(FLAGS) -c parser.c -o parser.o
clean:
	rm -rf *.o