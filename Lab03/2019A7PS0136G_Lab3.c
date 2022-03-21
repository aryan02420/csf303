#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
 
#define PORT 4444
 
int main (int argc, char const *argv[]) {

  int server_fd, new_sock;
  struct sockaddr_in address;
  int addrlen = sizeof(address);
  char *hello = "Hello World!\n";

  //write your code here

  // ### SETUP ###

  // CREATE SOCKET USING IPv4 AND TCP
  server_fd = socket(AF_INET, SOCK_STREAM, 0); 
  if (server_fd == -1) {
    perror("ERROR: Could not create socket\n");
    exit(EXIT_FAILURE);
  }
  printf("Socket created\n");

  // SET SOCKET OPTIONS
  // REUSES SOCKET IF IT EXISTS
  // PREVENTS ERROR WHEN SOCKET IS IN USE
  int socket_opt = 1;
  int socket_opt_success = setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &socket_opt, sizeof(socket_opt));
  if (socket_opt_success == -1) {
    perror("ERROR: Could not takeover socket\n");
    exit(EXIT_FAILURE);
  }

  // ASSIGN IP AND PORT
  // localhost:4444
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  // BIND SOCKET
  int bind_success = bind(server_fd, (struct sockaddr*) &address, sizeof(address));
  if (bind_success == -1) {
    perror("ERROR: Could not bind socket\n");
    exit(EXIT_FAILURE);
  }
  printf("Socket bound\n");

  // LISTEN ON SOCKET
  int listen_success = listen(server_fd, 3);
  if (listen_success == -1) {
    perror("ERROR: could not listen on socket\n");
    exit(EXIT_FAILURE);
  }
  printf("Listening on http://localhost:%d\n", PORT);

  // ### SERVE ###

  // SERVE CONTINUOUSLY
  while (1) {

    // ACCEPT CONNECTION
    new_sock = accept(server_fd, (struct sockaddr*) &address, (socklen_t*) &addrlen);
    if (new_sock == -1) {
      perror("ERROR: could not accept connection\n");
      exit(EXIT_FAILURE);
    }
    printf("Connection accepted\n");

    // FIXES: `curl: (56) Recv failure: Connection reset by peer`
    // buffer STORES THE INCOMING REQUEST
    char buffer[1024] = {0};
    read(new_sock, buffer, 1024);
    // printf(buffer);

    // SEND RESPONSE
    send(new_sock, hello, strlen(hello), 0);
    printf("Hello message sent to browser\n");

    // CLOSE THE CONNECTION
    // OTHERWISE CLIENT KEEPS WAITING
    close(new_sock);

  }

  return 0;
    
}