STD_FLAG=-I../include -L../ -lftsocket -o

all:
	g++ -g -fsanitize=address client.cpp $(STD_FLAG) client
	g++ -g -fsanitize=address server.cpp $(STD_FLAG) server

clean:
	rm -rf client server *.dSYM

re: clean all

.PHONY: all clean re
