#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MAX_BUFFER_SIZE 4096

// Helper Functions

void write_with_log(int fd, char data[MAX_BUFFER_SIZE]) {
  printf("S: %s\n", data);
  write(fd, data, MAX_BUFFER_SIZE);
}

void read_with_log(int fd, char data[MAX_BUFFER_SIZE]) {
  read(fd, data, MAX_BUFFER_SIZE);
  printf("C: %s\n", data);
}

int alllower(char data[MAX_BUFFER_SIZE]) {
  int len = strlen(data);
  int index = 0;
  while (index < len) {
    if (islower(data[index]) == 0) return 0;
    index++;
  }
  return 1;
}


int main(int argc, char *argv[]) {

  int socket_fd, connection_fd;
  struct sockaddr_in socket_addr;
  char write_buffer[MAX_BUFFER_SIZE];
  char read_buffer[MAX_BUFFER_SIZE];
  char equations[MAX_BUFFER_SIZE];

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

  while(1) {

    // ACCEPT CONNECTION
    int addr_len = sizeof(socket_addr);
    connection_fd = accept(socket_fd, (struct sockaddr*) &socket_addr, (socklen_t*) &addr_len);
    if (connection_fd == -1) {
      perror("ERROR: could not accept connection\n");
      exit(EXIT_FAILURE);
    }
    printf("Connection accepted\n");

    // READ CAMPUS ID FROM CLIENT
    read_with_log(connection_fd, read_buffer);
    int campusId = atoi(read_buffer);
    int y = (campusId % ((campusId % 599) + (campusId % 599)) / 3) + 98;
    
    // READ FILE
    FILE* fp = fopen("math.txt", "r");
    if (fp != NULL) {
      fgets(equations, MAX_BUFFER_SIZE, fp);
    }

    // GET CORRECT TOKEN
    int index = y;
    char* token = strtok(equations, ";");
    while( token != NULL && index >= 0 ) {
        token = strtok(NULL, ";");
        index--;
    }

    // SEND EQUATION
    write_with_log(connection_fd, token);

    // ASK FOR NAME AND SEND TO CLIENT
    do {
      printf("Your name (in lowercase): ");
      fgets(write_buffer, MAX_BUFFER_SIZE, stdin);
	    write_buffer[strlen(write_buffer) - 1] = 0;
    } while (alllower(write_buffer) == 0);
    write_with_log(connection_fd, write_buffer);

  }
  
  // CLOSE CONNECTION
	close(socket_fd);

}
