#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <pthread.h>
#include <semaphore.h>

#define MAX_BUFFER_SIZE 4096

// Global variables
volatile sig_atomic_t request_exit = 0;
int socket_fd;

// Helper Functions

int write_with_log(int fd, char data[MAX_BUFFER_SIZE]) {
  printf("\rC: %s\n", data);
  return write(fd, data, MAX_BUFFER_SIZE);
}

int read_with_log(int fd, char data[MAX_BUFFER_SIZE]) {
  int len = read(fd, data, MAX_BUFFER_SIZE);
  printf("\rS: %s\n", data);
  return len;
}

int write_no_log(int fd, char data[MAX_BUFFER_SIZE]) {
  return write(fd, data, MAX_BUFFER_SIZE);
}

int read_no_log(int fd, char data[MAX_BUFFER_SIZE]) {
  return read(fd, data, MAX_BUFFER_SIZE);
}

void read_stdin(char data[MAX_BUFFER_SIZE]) {
  printf("\r>> ");
  fgets(data, MAX_BUFFER_SIZE, stdin);
  data[strlen(data) - 1] = 0;
}

void log_error(char message[]) {
  printf("\rERROR: %s\n", message);
}

void log_message(char message[]) {
  printf("\r%s\n", message);
}

void display_message(char *data, char *name) {
  printf("\r%s: %s\n>> ", name, data);
}

void sigint_or_exit(int sig) {
  request_exit = 1;
}

void send_msg_handler() {
  char write_buffer[MAX_BUFFER_SIZE];

  while(1) {
    // READ FROM STDIN AND SEND TO SERVER
    read_stdin(write_buffer);

    // EMPTY STRING
    if(strlen(write_buffer) == 0) {
      continue;
    }

    // EXIT COMMAND
    if(strcmp(write_buffer, "EXIT") == 0) {
      break;
    }

    write_no_log(socket_fd, write_buffer);
  }

  sigint_or_exit(1);
}

void recv_msg_handler() {
  char read_buffer[MAX_BUFFER_SIZE];

  while (1) {
		int read_len = read_no_log(socket_fd, read_buffer);

    // EMPTY MESSAGE
    if (read_len <= 0)
      continue;

    // EXIT COMMAND
    if(strcmp(read_buffer, "EXIT") == 0)
      break;

		display_message(read_buffer, "");
  }
  
  sigint_or_exit(1);
}

int main(int argc, char *argv[]) {

	signal(SIGINT, sigint_or_exit);

  struct sockaddr_in socket_addr;
  char message[MAX_BUFFER_SIZE];

  // CHECK IF IP AND PORT NUM IS PROVIDED AS CLI ARG
  if (argc <= 2) {
    log_error("Please provide an IP and port number as CLI arg");
    sprintf(message, "USAGE: %s <IP> <PORT>", argv[0]);
    log_error(message);
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

  // SEND PUBLIC KEY
  write_no_log(socket_fd ,"123");

  pthread_t send_msg_thread;
  if(pthread_create(&send_msg_thread, NULL, (void *) send_msg_handler, NULL) != 0){
    log_error("Could not create thread");
    exit(EXIT_FAILURE);
  }

  pthread_t recv_msg_thread;
  if(pthread_create(&recv_msg_thread, NULL, (void *) recv_msg_handler, NULL) != 0){
    log_error("Could not create thread");
    exit(EXIT_FAILURE);
  }

  while(request_exit == 0) {
    sleep(1);
  }

  // CLOSE CONNECTION
  log_message("Disconnecting...");
  write_no_log(socket_fd ,"EXIT");
	close(socket_fd);

}
