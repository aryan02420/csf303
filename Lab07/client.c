#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 4096

// Helper Functions

void write_with_log(int fd, char data[MAX_BUFFER_SIZE]) {
	printf("C: %s\n", data);
	write(fd, data, MAX_BUFFER_SIZE);
}

void read_with_log(int fd, char data[MAX_BUFFER_SIZE]) {
	read(fd, data, MAX_BUFFER_SIZE);
	printf("S: %s\n", data);
}

void read_stdin(char data[MAX_BUFFER_SIZE]) {
  printf(">> ");
	fgets(data, MAX_BUFFER_SIZE, stdin);
	data[strlen(data) - 1] = 0;
}

void log_error(char message[]) {
    printf("ERROR: ");
    printf(message);
    printf("\n");
}
void log_message(char message[]) {
    printf(message);
    printf("\n");
}

int main(int argc, char *argv[]) {

  int socket_fd;
  struct sockaddr_in socket_addr;
  char read_buffer[MAX_BUFFER_SIZE];
  char write_buffer[MAX_BUFFER_SIZE];

  // CHECK IF IP AND PORT NUM IS PROVIDED AS CLI ARG
  if (argc <= 2) {
    log_error("Please provide an IP and port number as CLI arg\neg. $ ./client.out 127.0.0.1 4444");
    exit(EXIT_FAILURE);
  }

  // PARSE IP AND PORT NUM
	char* server_ip = argv[1];
  int server_port = atoi(argv[2]);

  // ### SETUP ###

  // CREATE SOCKET USING IPv4 AND TCP
  socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
  if (socket_fd == -1) {
    log_error("Could not create socket");
    exit(EXIT_FAILURE);
  }
  log_message("Socket created");

  // ASSIGN IP AND PORT
  // localhost:4444
  socket_addr.sin_family = AF_INET;
  // socket_addr.sin_addr.s_addr = INADDR_ANY;
	inet_pton(AF_INET, server_ip, &socket_addr.sin_addr);
  socket_addr.sin_port = htons(server_port);

	// CONNECT TO THE SERVER
	int connect_success = connect(socket_fd, (struct sockaddr*) &socket_addr, sizeof(socket_addr));
	if (connect_success == -1) {
    log_error("Could not connect to server");
    exit(EXIT_FAILURE);
	}
	log_message("Connected to server");

	// ### COMMUNICATION ###

  while(1) {

    // READ FROM STDIN AND SEND TO SERVER
    read_stdin(write_buffer);

    // if exit
    // break;

    // else
    write_with_log(socket_fd, write_buffer);

    read_with_log(socket_fd, read_buffer);
  
  }

  // CLOSE CONNECTION
	close(socket_fd);

}
