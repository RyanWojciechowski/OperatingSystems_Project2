CC = gcc
CFLAGS = -Wall -Wextra -O2
LIBS = -lpthread
 
all: main
 
main: main.c
	$(CC) $(CFLAGS) -o main main.c $(LIBS)
 
clean:
	rm -f main