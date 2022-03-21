.PHONY: build client server

build:
	gcc ./server/hserver.c -o ./server/server.out
	gcc ./client/hclient.c -o ./client/client.out

server:
	cd server && ./server.out 4444

client: 
	cd client && ./client.out 127.0.0.1 4444
