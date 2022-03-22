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
  printf("\rS: %s\n", data);
  write(fd, data, MAX_BUFFER_SIZE);
}

void read_with_log(int fd, char data[MAX_BUFFER_SIZE]) {
  read(fd, data, MAX_BUFFER_SIZE);
  printf("\rC: %s\n", data);
}

void read_stdin(char data[MAX_BUFFER_SIZE]) {
  printf("\r>> ");
	fgets(data, MAX_BUFFER_SIZE, stdin);
	data[strlen(data) - 1] = 0;
}

void log_error(char message[]) {
  printf("ERROR: %s\n", message);
}

void log_message(char message[]) {
  printf("\r%s\n", message);
}

void reverse_str(char *str) {
  int i, len, temp;  
  len = strlen(str); 
  for (i = 0; i < len/2; i++) {
    temp = str[i];
    str[i] = str[len - i - 1];
    str[len - i - 1] = temp;
  }
}


int main(int argc, char *argv[]) {

  int socket_fd, connection_fd;
  struct sockaddr_in socket_addr, connection_addr;
  char write_buffer[MAX_BUFFER_SIZE];
  char read_buffer[MAX_BUFFER_SIZE];
  char temp_str[MAX_BUFFER_SIZE];

  // CHECK IF PORT NUM IS PROVIDED AS CLI ARG
  if (argc <= 1) {
    log_error("please provide a port number as CLI arg\neg. $ ./server.out 4444");
    exit(EXIT_FAILURE);
  }

  // PARSE PORT NUM
  int server_port = atoi(argv[1]);

  // ### SETUP ###

  // CREATE SOCKET USING IPv4 AND TCP
  socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
  if (socket_fd == -1) {
    log_error("Could not create socket");
    exit(EXIT_FAILURE);
  }
  log_message("Socket created");

  // SET SOCKET OPTIONS
  // REUSES SOCKET IF IT EXISTS
  // PREVENTS ERROR WHEN SOCKET IS IN USE
  int socket_opt = 1;
  int socket_opt_success = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &socket_opt, sizeof(socket_opt));
  if (socket_opt_success == -1) {
    log_error("Could not takeover socket");
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
    log_error("Could not bind socket");
    exit(EXIT_FAILURE);
  }
  log_message("Socket bound");

  // LISTEN ON SOCKET
  int listen_success = listen(socket_fd, 3);
  if (listen_success == -1) {
    log_error("Could not listen on socket");
    exit(EXIT_FAILURE);
  }
  sprintf(temp_str, "Listening on localhost:%d", server_port);
  log_message(temp_str);

  // ### SERVE ###

  int num_clients = 0;

  while(1) {

    // ACCEPT CONNECTION
    int addr_len = sizeof(socket_addr);
    connection_fd = accept(socket_fd, (struct sockaddr*) &connection_addr, (socklen_t*) &addr_len);
    char *connection_ip = inet_ntoa(connection_addr.sin_addr);
    int connection_port = ntohs(connection_addr.sin_port);

    if (connection_fd == -1) {
      log_error("Could not accept connection");
      exit(EXIT_FAILURE);
    }
    sprintf(temp_str, "Connection accepted from %s:%d", connection_ip, connection_port);
    log_message(temp_str);

    // CREATE A NEW PROCESS TO HANDLE EACH CLIENT
    int child_pid = fork();
    if (child_pid < 0) {
      log_error("Could not create a child process");
      exit(EXIT_FAILURE);
    }

    num_clients++;

    // SERVER ACCEPTS UTMOST 4 CLIENTS
    if (num_clients > 4) {
      log_error("cannot connect more than 4 clients");
      close(connection_fd);
      continue;
    }

    if(child_pid == 0){
      close(socket_fd);
      
      // SERVE THE CLIENT
      while(1) {
        read_with_log(connection_fd, read_buffer);
        reverse_str(read_buffer);
        log_message(read_buffer);
        sprintf(temp_str, "FROM %s:%d", connection_ip, connection_port);
        log_message(temp_str);
        read_stdin(read_buffer);
        write_with_log(connection_fd, read_buffer);
        // break;
      }
  
      // CLOSE CONNECTION
      close(connection_fd);
      num_clients--;
		}

  }

}
