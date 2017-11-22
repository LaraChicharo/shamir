LDFLAGS=-lmcrypt
EXECUTABLE_NAME=cipher

main: main.o cipher.o
	cc -o $(EXECUTABLE_NAME) cipher.o main.o $(LDFLAGS)
main.o:
	cc -c main.c
cipher.o: cipher.h
	cc -c cipher.c

clean:
	rm *.o $(EXECUTABLE_NAME)
