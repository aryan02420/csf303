#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define PORT 4444
#define MAX_BUFFER_SIZE 1024

// Helper Functions

void write_with_log(int fd, char data[MAX_BUFFER_SIZE]) {
	printf("C: %s", data);
	write(fd, data, MAX_BUFFER_SIZE);
}

void read_with_log(int fd, char data[MAX_BUFFER_SIZE]) {
	read(fd, data, MAX_BUFFER_SIZE);
	printf("S: %s\n", data);
}

int main() {
  int socket_fd;
  struct sockaddr_in socket_addr;
  char write_buffer[MAX_BUFFER_SIZE], read_buffer[MAX_BUFFER_SIZE];

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
  socket_addr.sin_addr.s_addr = INADDR_ANY;
  socket_addr.sin_port = htons(PORT);

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
	fgets(&write_buffer, MAX_BUFFER_SIZE, stdin);
	write_with_log(socket_fd, write_buffer);

	// CHECK IF SERVER RESPONDS WITH ERROR CODE
	read_with_log(socket_fd, read_buffer);
	if(read_buffer[0] == '5') {
		strcpy(write_buffer, ".\n");
		write_with_log(socket_fd, write_buffer);
		read_with_log(socket_fd, read_buffer);
		close(socket_fd);
		exit(0);
	}	

	// SEND MULTIPLE LINES TO SERVER
	while (1) {
		fgets(&write_buffer, MAX_BUFFER_SIZE, stdin);
		write_with_log(socket_fd, write_buffer);
		if (strcmp(write_buffer, ".") == 0 || strcmp(write_buffer, ".\n") == 0) break;
	}

	// GET FINAL MESSAGE AND CLOSE
	read_with_log(socket_fd, read_buffer);
	close(socket_fd);

}