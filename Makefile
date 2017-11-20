LDFLAGS=-lmcrypt
EXECUTABLE_NAME=cipher

cipher: cipher.o
	cc -o $(EXECUTABLE_NAME) cipher.o $(LDFLAGS)
cipher.o:
	cc -c cipher.c

clean:
	rm *.o $(EXECUTABLE_NAME)
