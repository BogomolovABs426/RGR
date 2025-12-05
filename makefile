
CC = g++
CFLAGS = -std=c++17 -Wall -Wextra -Werror -fPIC
LDFLAGS = -lssl -lcrypto -ldl

all: crypto_system libdes.so librc2.so librot13.so

crypto_system: main.o utils.o
	$(CC) $(CFLAGS) -o crypto_system main.o utils.o $(LDFLAGS)

libdes.so: des.o utils.o
	$(CC) $(CFLAGS) -shared -o libdes.so des.o utils.o $(LDFLAGS)

librc2.so: rc2.o utils.o
	$(CC) $(CFLAGS) -shared -o librc2.so rc2.o utils.o $(LDFLAGS)

librot13.so: rot13.o
	$(CC) $(CFLAGS) -shared -o librot13.so rot13.o

main.o: main.cpp utils.h
	$(CC) $(CFLAGS) -c main.cpp

utils.o: utils.cpp utils.h
	$(CC) $(CFLAGS) -c utils.cpp

des.o: des.cpp des.h utils.h
	$(CC) $(CFLAGS) -c des.cpp

rc2.o: rc2.cpp rc2.h utils.h
	$(CC) $(CFLAGS) -c rc2.cpp

rot13.o: rot13.cpp rot13.h
	$(CC) $(CFLAGS) -c rot13.cpp

clean:
	rm -f *.o crypto_system *.so *.txt

install-deps:
	sudo apt-get update
	sudo apt-get install -y libssl-dev

run: all
	./crypto_system

test: all
	@echo "Testing encryption..."
	@echo "Hello, World!" > test.txt
	@echo "Testing DES..."
	@./crypto_system <<< "123\n1\n1\n1\ntest.txt\ntest_des.enc\nmykey123\n2\n"
	@echo "Testing RC2..."
	@./crypto_system <<< "123\n1\n2\n1\ntest.txt\ntest_rc2.enc\nmyrc2key\n128\n2\n"
	@echo "Testing ROT13..."
	@./crypto_system <<< "123\n1\n3\n1\ntest.txt\ntest_rot13.enc\n2\n"
	@echo "Tests completed."

.PHONY: all clean install-deps run test
