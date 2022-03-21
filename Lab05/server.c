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


void write_with_log(int fd, char data[MAX_BUFFER_SIZE]) {
  printf("S: %s\n", data);
  write(fd, data, MAX_BUFFER_SIZE);
}

void read_with_log(int fd, char data[MAX_BUFFER_SIZE]) {
  read(fd, data, MAX_BUFFER_SIZE);
  printf("C: %s", data);
}


int main() {

  int socket_fd, connection_fd;
  struct sockaddr_in socket_addr;
  char write_buffer[MAX_BUFFER_SIZE], read_buffer[MAX_BUFFER_SIZE], tmp_buffer[MAX_BUFFER_SIZE];

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
  socket_addr.sin_port = htons(PORT);

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
  printf("Listening on http://localhost:%d\n", PORT);

  // ### SERVE ###

  while (1) {

    // ACCEPT CONNECTION
    int addr_len = sizeof(socket_addr);
    connection_fd = accept(socket_fd, (struct sockaddr*) &socket_addr, (socklen_t*) &addr_len);
    if (connection_fd == -1) {
      perror("ERROR: could not accept connection\n");
      exit(EXIT_FAILURE);
    }
    printf("Connection accepted\n");

    // ASK CLIENT FOR NAME
    strcpy(write_buffer, "Hello, what is your name?");
    write_with_log(connection_fd, write_buffer);

    // READ NAME FROM CLIENT
    read_with_log(connection_fd, read_buffer);
    strcpy(tmp_buffer, read_buffer);

    // CHECK IF NAME IS OF CORRECT FORMAT
    if(read_buffer[0] <= 'z' && read_buffer[0] >= 'a') {
      strcpy(write_buffer, "500 ERROR");
    } else {
      strcpy(write_buffer, "200 OK");
    }
    write_with_log(connection_fd, write_buffer);

    // READ MULTIPLE LINES FROM CLIENT
    while (1) {
      read_with_log(connection_fd, read_buffer);
      if (strcmp(read_buffer, ".") == 0 || strcmp(read_buffer, ".\n") == 0)
        break;
    }

    // SEND FINAL MESSAGE
    strcpy(write_buffer, "Thank you, ");
    strcat(write_buffer, tmp_buffer);
    write_with_log(connection_fd, write_buffer);
    
  }

  // CLOSE CONNECTION
	close(socket_fd);

}