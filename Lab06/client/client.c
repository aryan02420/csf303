#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 1024
#define MAX_BYTES 11

// Helper Functions

void write_with_log(int fd, char data[MAX_BUFFER_SIZE]) {
	data[strlen(data) - 1] = 0;
	printf("C: %s\n", data);
	write(fd, data, MAX_BUFFER_SIZE);
}

void read_with_log(int fd, char data[MAX_BUFFER_SIZE]) {
	read(fd, data, MAX_BUFFER_SIZE);
	printf("S: %s\n", data);
}

int main(int argc, char *argv[]) {

  int socket_fd;
  struct sockaddr_in socket_addr;
  char read_buffer[MAX_BUFFER_SIZE];
  char filename_buffer[MAX_BUFFER_SIZE], filecontent_buffer[MAX_BYTES];

  // CHECK IF IP AND PORT NUM IS PROVIDED AS CLI ARG
  if (argc <= 2) {
    perror("ERROR: please provide an IP and port number as CLI arg\neg. $ ./client.out 127.0.0.1 4444\n");
    exit(EXIT_FAILURE);
  }

  // PARSE IP AND PORT NUM
	char* server_ip = argv[1];
  int server_port = atoi(argv[2]);

  // ### SETUP ###

  // CREATE SOCKET USING IPv4 AND TCP
  socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
  if (socket_fd == -1) {
    perror("ERROR: Could not create socket\n");
    exit(EXIT_FAILURE);
  }
  printf("Socket created\n");

  // ASSIGN IP AND PORT
  // localhost:4444
  socket_addr.sin_family = AF_INET;
  // socket_addr.sin_addr.s_addr = INADDR_ANY;
	inet_pton(AF_INET, server_ip, &socket_addr.sin_addr);
  socket_addr.sin_port = htons(server_port);

	// CONNECT TO THE SERVER
	int connect_success = connect(socket_fd, (struct sockaddr*) &socket_addr, sizeof(socket_addr));
	if (connect_success == -1) {
    perror("ERROR: Could not connect to server\n");
    exit(EXIT_FAILURE);
	}
	printf("Connected to server\n");

	// ### COMMUNICATION ###

	// GET MESSAGE FROM SERVER
	read_with_log(socket_fd, read_buffer);

	// READ INPUT FROM STDIN AND SEND TO SERVER
	fgets(filename_buffer, MAX_BUFFER_SIZE, stdin);
	write_with_log(socket_fd, filename_buffer);

	// GET FILE FROM SERVER
	read(socket_fd, filecontent_buffer, MAX_BYTES);
  printf("File received!\n");
  printf("##########\n");
  printf("%s\n", filecontent_buffer);
  printf("##########\n");

  // SAVE FILE
	FILE* fp = fopen(filename_buffer, "w");
	fputs(filecontent_buffer, fp);
	fclose(fp);
  printf("Created %s\n", filename_buffer);

  // CLOSE CONNECTION
	close(socket_fd);

}
