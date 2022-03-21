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
  printf("S: %s\n", data);
  write(fd, data, MAX_BUFFER_SIZE);
}

void read_with_log(int fd, char data[MAX_BUFFER_SIZE]) {
  read(fd, data, MAX_BUFFER_SIZE);
  printf("C: %s\n", data);
}


int main(int argc, char *argv[]) {

  int socket_fd, connection_fd;
  struct sockaddr_in socket_addr;
  char write_buffer[MAX_BUFFER_SIZE];
  char filename_buffer[MAX_BUFFER_SIZE], filecontent_buffer[MAX_BYTES];

  // CHECK IF PORT NUM IS PROVIDED AS CLI ARG
  if (argc <= 1) {
    perror("ERROR: please provide a port number as CLI arg\neg. $ ./server.out 4444\n");
    exit(EXIT_FAILURE);
  }

  // PARSE PORT NUM
  int server_port = atoi(argv[1]);

  // ### SETUP ###

  // CREATE SOCKET USING IPv4 AND TCP
  socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
  if (socket_fd == -1) {
    perror("ERROR: Could not create socket\n");
    exit(EXIT_FAILURE);
  }
  printf("Socket created\n");

  // SET SOCKET OPTIONS
  // REUSES SOCKET IF IT EXISTS
  // PREVENTS ERROR WHEN SOCKET IS IN USE
  int socket_opt = 1;
  int socket_opt_success = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &socket_opt, sizeof(socket_opt));
  if (socket_opt_success == -1) {
    perror("ERROR: Could not takeover socket\n");
    exit(EXIT_FAILURE);
  }

  // ASSIGN IP AND PORT
  // localhost:4444
  socket_addr.sin_family = AF_INET;
  socket_addr.sin_addr.s_addr = INADDR_ANY;
  socket_addr.sin_port = htons(server_port);

  // BIND SOCKET
  int bind_success = bind(socket_fd, (struct sockaddr*) &socket_addr, sizeof(socket_addr));
  if (bind_success == -1) {
    perror("ERROR: Could not bind socket\n");
    exit(EXIT_FAILURE);
  }
  printf("Socket bound\n");

  // LISTEN ON SOCKET
  int listen_success = listen(socket_fd, 3);
  if (listen_success == -1) {
    perror("ERROR: could not listen on socket\n");
    exit(EXIT_FAILURE);
  }
  printf("Listening on http://localhost:%d\n", server_port);

  // ### SERVE ###

  // ACCEPT CONNECTION
  int addr_len = sizeof(socket_addr);
  connection_fd = accept(socket_fd, (struct sockaddr*) &socket_addr, (socklen_t*) &addr_len);
  if (connection_fd == -1) {
    perror("ERROR: could not accept connection\n");
    exit(EXIT_FAILURE);
  }
  printf("Connection accepted\n");

  // ASK CLIENT FOR FILE NAME
  strcpy(write_buffer, "filename?");
  write_with_log(connection_fd, write_buffer);

  // READ FILE NAME FROM CLIENT
  read_with_log(connection_fd, filename_buffer);
  
  // SEND FILE
  FILE* fp = fopen(filename_buffer, "r");
  bzero(filecontent_buffer, MAX_BYTES);
  if (fp != NULL) {
    fgets(filecontent_buffer, MAX_BYTES, fp);
  }
  write(connection_fd, filecontent_buffer, MAX_BYTES);

  printf("File Sent!\n");
  
  // CLOSE CONNECTION
	close(socket_fd);

}
