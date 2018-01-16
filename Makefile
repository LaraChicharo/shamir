SRCFOLDER=src/
LDFLAGS=-lmcrypt -lgmp -lm -lmhash
EXECUTABLE_NAME=main

TEST_FOLDER=test/
TEST_EXECNAME=tests

main: main.o cipher.o shamir.o hasher.o
	cc -o $(EXECUTABLE_NAME) cipher.o shamir.o hasher.o main.o $(LDFLAGS)
main.o:
	cc -c $(SRCFOLDER)main.c
cipher.o: $(SRCFOLDER)cipher.h
	cc -c $(SRCFOLDER)cipher.c

shamir.o: $(SRCFOLDER)shamir.h
	cc -c $(SRCFOLDER)shamir.c

hasher.o: $(SRCFOLDER)hasher.h
	cc -c $(SRCFOLDER)hasher.c


clean:
	rm *.o $(EXECUTABLE_NAME)


test: test_cipher.o cipher.o 
	cc -o $(TEST_EXECNAME) cipher.o test_cipher.o $(LDFLAGS)
test_cipher.o: $(SRCFOLDER)$(TEST_FOLDER)minunit.h $(SRCFOLDER)cipher.c
	cc -c $(SRCFOLDER)$(TEST_FOLDER)test_cipher.c

testclean:
	rm *.o $(TEST_EXECNAME)
