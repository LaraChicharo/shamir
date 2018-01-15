SRCFOLDER=src/
LDFLAGS=-lmcrypt
EXECUTABLE_NAME=cipher

TEST_FOLDER=test/
TEST_EXECNAME=tests

main: main.o cipher.o
	cc -o $(EXECUTABLE_NAME) cipher.o main.o $(LDFLAGS)
main.o:
	cc -c $(SRCFOLDER)main.c
cipher.o: $(SRCFOLDER)cipher.h
	cc -c $(SRCFOLDER)cipher.c

clean:
	rm *.o $(EXECUTABLE_NAME)

test: test_cipher.o cipher.o
	cc -o $(TEST_EXECNAME) cipher.o test_cipher.o $(LDFLAGS)
test_cipher.o: $(SRCFOLDER)$(TEST_FOLDER)minunit.h $(SRCFOLDER)cipher.c
	cc -c $(SRCFOLDER)$(TEST_FOLDER)test_cipher.c

testclean:
	rm *.o $(TEST_EXECNAME)
