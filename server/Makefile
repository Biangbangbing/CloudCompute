all:httpServer

CFLAGS = -O2 -pthread -lpthread

LDFLAGS= -L./usr/lib/x86_64-linux-gnu
LDFLAGS = -ljsoncpp

LIB=-L./usr/lib/x86_64-linux-gnu -ljsoncpp
SRC = main.cpp

httpServer: $(SRC)
	g++ $(CFLAGS) $(SRC) $(LIB) -o httpserver

clean:
	-rm -f httpserver
