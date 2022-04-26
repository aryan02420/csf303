#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <limits.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include <pthread.h>
#include <semaphore.h>

#define MAX_BUFFER_SIZE 4096
#define MAX_CLIENTS 2

// types

#pragma pack(1)
typedef struct message_s {
  // 10 = message
  // 20 = instruction
  // 40 = EXIT
  unsigned char code;
  char name[32];
  unsigned short int len;
  char data[USHRT_MAX];
} message_t;

typedef struct client_s {
  struct sockaddr_in address;
  int sockfd;
  char ip[16];
  int port;
  char name[32];
  int uid;
  char *pub_key;
} client_t;

// global variables

int socket_fd, connection_fd;
struct sockaddr_in socket_addr;

static _Atomic unsigned int num_clients = 0;
static int uuid = 1000;
static client_t *clients[MAX_CLIENTS];

pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Helper Functions

int write_with_log(int fd, char data[MAX_BUFFER_SIZE]) {
  printf("\rS: %s\n", data);
  return write(fd, data, MAX_BUFFER_SIZE);
}

int read_with_log(int fd, char data[MAX_BUFFER_SIZE]) {
  int len = read(fd, data, MAX_BUFFER_SIZE);
  printf("\rC: %s\n", data);
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

void display_message(message_t *msg) {
  if (msg->code == 10)
    printf("\r%s: %s\n", msg->name, msg->data);
}

void send_message(message_t *msg, int uid) {
  pthread_mutex_lock(&clients_mutex);
  for(int i=0; i<MAX_CLIENTS; i++)
    if(clients[i] && clients[i]->uid != uid)
      write_no_log(clients[i]->sockfd, (char*)msg);
  pthread_mutex_unlock(&clients_mutex);
}

void addClient(client_t *client) {
  char message[MAX_BUFFER_SIZE];
  pthread_mutex_lock(&clients_mutex);
  for(int i=0; i < MAX_CLIENTS; i++)
    if(!clients[i]) {
      clients[i] = client;
      break;
    }
  pthread_mutex_unlock(&clients_mutex);
  num_clients++;
  sprintf(message, "%d/%d connected clients", num_clients, MAX_CLIENTS);
  log_message(message);
}

void removeClient(int uid) {
  char message[MAX_BUFFER_SIZE];
  pthread_mutex_lock(&clients_mutex);
  for(int i=0; i < MAX_CLIENTS; i++)
    if(clients[i] && clients[i]->uid == uid) {
      clients[i] = NULL;
      break;
    }
  pthread_mutex_unlock(&clients_mutex);
  num_clients--;
  sprintf(message, "%d/%d connected clients", num_clients, MAX_CLIENTS);
  log_message(message);
}

void *handleClient(void *arg) {
  
  client_t *client = (client_t *)arg;
  addClient(client);

  int connection_fd = client->sockfd;
  char name[32];
  strcpy(name, client->name);
  int uid = client->uid;

  char write_buffer[MAX_BUFFER_SIZE];
  char read_buffer[MAX_BUFFER_SIZE];
  char message[MAX_BUFFER_SIZE];

  sprintf(message, "%s Connected", name);
  log_message(message);

  sprintf(write_buffer, "%d", uid);
  write_no_log(connection_fd, write_buffer);

  read_no_log(connection_fd, read_buffer);
  client->pub_key = read_buffer;

  // SERVE THE CLIENT
  while(1) {
    int read_len = read_no_log(connection_fd, read_buffer);
    if (read_len > 0) {
      // send message
      display_message((message_t*)read_buffer);
      message_t msg;
      msg.code = 10;
      strcpy(msg.name, name);
      msg.len = strlen(read_buffer);
      strcpy(msg.data, read_buffer);
      send_message(&msg, uid);
    }
    if (strcmp(read_buffer, "EXIT") == 0) {
      break;
    }
  }

  // CLOSE CONNECTION
  close(connection_fd);
  sprintf(write_buffer, "%s Disconnected", name);
  log_message(write_buffer);
  removeClient(uid);
  pthread_detach(pthread_self());
  return NULL;
}



int main(int argc, char *argv[]) {

  char message[MAX_BUFFER_SIZE];

  // CHECK IF PORT NUM IS PROVIDED AS CLI ARG
  if (argc <= 1) {
    sprintf(message, "USEGE: %s 4444", argv[0]);
    log_error("please provide a port number as CLI arg");
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
  int listen_success = listen(socket_fd, 5);
  if (listen_success == -1) {
    log_error("Could not listen on socket");
    exit(EXIT_FAILURE);
  }
  sprintf(message, "Listening on localhost:%d", server_port);
  log_message(message);

  // ### SERVE ###

  while(1) {

    char message[MAX_BUFFER_SIZE];

    // ACCEPT CONNECTION
    struct sockaddr_in connection_addr;
    int addr_len = sizeof(connection_addr);
    connection_fd = accept(socket_fd, (struct sockaddr*) &connection_addr, (socklen_t*) &addr_len);
    char *connection_ip = inet_ntoa(connection_addr.sin_addr);
    int connection_port = ntohs(connection_addr.sin_port);
    if (connection_fd == -1) {
      log_error("Could not accept connection");
      exit(EXIT_FAILURE);
    }

    // CHECK IF MAX CLIENTS CONNECTED
    if (num_clients == MAX_CLIENTS - 1) {
      log_message("Max clients connected.");
    } else if (num_clients >= MAX_CLIENTS) {
      write_no_log(connection_fd, "Max client limit reached");
      write_no_log(connection_fd, "EXIT");
      close(connection_fd);
      log_error("Connection rejected.");
      continue;
    }

    // CREATE A NEW PROCESS TO HANDLE EACH CLIENT
    pthread_t thread_id;
    client_t client;
    client.address = connection_addr;
    client.sockfd = connection_fd;
    strcpy(client.ip, connection_ip);
    client.port = connection_port;
    client.uid = ++uuid;
    char name[32];
    sprintf(name, "%s:%d [%d]", connection_ip, connection_port, uuid);
    strcpy(client.name, name);
    
    int thread_status = pthread_create(&thread_id, NULL, handleClient, &client);

    sleep(0.25);

  }

}
