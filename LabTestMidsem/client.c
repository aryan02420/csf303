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
  char write_buffer[MAX_BUFFER_SIZE];
  char name[MAX_BUFFER_SIZE];
  char campus_id[MAX_BUFFER_SIZE];
  char equation[MAX_BUFFER_SIZE];

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

	// READ CAMPUS IDs FROM STDIN AND SEND TO SERVER
  printf("campus id (3112xxxxxxx): ");
	fgets(campus_id, MAX_BUFFER_SIZE, stdin);
	write_with_log(socket_fd, campus_id);

  // GET EQUATION FROM SERVER
  read_with_log(socket_fd, equation);

  // GET NAME FROM SERVER
  read_with_log(socket_fd, name);

  printf("%s,%s,%s\n", name, campus_id, equation);

  // CLOSE CONNECTION
	close(socket_fd);

}
