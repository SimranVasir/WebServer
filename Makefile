CC=gcc
CFLAGS=-Wall -Werror -g -Wextra -Wno-unused-parameter
LDFLAGS=

all: cshttp
cshttp: cshttp.o service.o util.o
	gcc cshttp.o service.o util.o -o cshttp
test_util: test_util.o util.o

cshttp.o: 
	$(CC) $(CFLAGS) -c cshttp.c service.h
service.o: 
	$(CC) $(CFLAGS) -c service.c service.h util.h
util.o: 
	$(CC) -c util.c util.h
test_util.o: 
	$(CC) $(CFLAGS) -c test_util.c util.h

clean:
	-rm -rf cshttp.o service.o util.o cshttp
