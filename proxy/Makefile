CC = g++
SRC = main.cpp proxy.cpp client.cpp connection.cpp requestheader.cpp cache.cpp cacheentry.cpp responseheader.cpp
OBJECT = proxy
CFLAGS = -Wall -g
OBJECTS = proxy requestheader connection client

all : $(SRC)
	$(CC) $(SRC) $(CFLAGS) -o $(OBJECT)
connection : connection.cpp
	$(CC) connection.cpp -o connection $(CFLAGS)
client : client.cpp
	$(CC) client.cpp -o client $(CFLAGS)
requestheader : requestheader.cpp
	$(CC) requestheader.cpp -o requestheader $(CFLAGS)
entry : cacheentry.cpp
	$(CC) cacheentry.cpp -o entry $(CFLAGS)
cache : cache.cpp cacheentry.cpp
	$(CC) cache.cpp cacheentry.cpp -o cache $(CFLAGS)
clean :
	rm -f $(OBJECTS)