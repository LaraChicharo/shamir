SRCFOLDER=src/
LDFLAGS=-lmcrypt
EXECUTABLE_NAME=cipher

main: main.o cipher.o
	cc -o $(EXECUTABLE_NAME) cipher.o main.o $(LDFLAGS)
main.o:
	cc -c $(SRCFOLDER)main.c
cipher.o: $(SRCFOLDER)cipher.h
	cc -c $(SRCFOLDER)cipher.c

clean:
	rm *.o $(EXECUTABLE_NAME)
