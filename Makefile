LDFLAGS=-lmcrypt
EXECUTABLE_NAME=cipher

cipher: cipher.o
	cc -o $(EXECUTABLE_NAME) cipher.o $(LDFLAGS)
cipher.o: cipher.h
	cc -c cipher.c

clean:
	rm *.o $(EXECUTABLE_NAME)
