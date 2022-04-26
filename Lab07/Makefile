.PHONY: build client server

build:
	gcc server.c -o server.out -lpthread 
	gcc client.c -o client.out -lpthread 

server:
	./server.out 4444

client: 
	./client.out 127.0.0.1 4444
