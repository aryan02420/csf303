#include <arpa/inet.h>
#include <errno.h>
#include <libgen.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/* man basename(3) */
#define _GNU_SOURCE
#define MAX_BUFFER_SIZE 8192
#define REQUEST "GET /%s HTTP/1.0\r\n" \
                "Host: %s\r\n" \
                "Accept: */*\r\n" \
                "Accept-Encoding: *\r\n" \
                "Connection: close\r\n" \
                "Content-type: application/x-www-form-urlencoded\r\n" \
                "Content-length: %d\r\n" \
                "\r\n" \
                "%s\r\n"

void log_error(char message[]) {
  printf("\rERROR: %s\n", message);
}

void log_message(char message[]) {
  printf("\r%s\n", message);
}

int main(int argc, char *argv[]) {

    char tmp_str[1024];
    char *tmp_pstr;
    int debug = 0;

    if (argc <= 1) {
        log_error("please provide an URL as CLI arg");
        sprintf(tmp_str, "USAGE: %s <url>", argv[0]);
        log_message(tmp_str);
        exit(EXIT_FAILURE);
    }

    if (strcmp(argv[argc - 1], "--debug") == 0) debug = 1;

    char url[1024];
    char protocol[1024];
    char domain[1024];
    char path[1024];
    char file_name[1024];
    int port;

    // COPY URL
    strcpy(url, argv[1]);
    strcat(url, "/");

    // EXTRACT PROTOCOL
    tmp_pstr = strstr(url, "://");
    if (tmp_pstr == NULL) {
        strcpy(protocol, "http");
        strcpy(tmp_str, url);
    } else {
        strncpy(protocol, url, tmp_pstr - url);
        strcpy(tmp_str, tmp_pstr + 3);
    }
    
    // EXTRACT PATH
    tmp_pstr = strchr(tmp_str, '/');
    strcpy(path, tmp_pstr);
    
    // EXTRACT DOMAIN
    strncpy(domain, tmp_str, tmp_pstr - tmp_str);

    // PORT
    if (strcmp(protocol, "http") == 0) {
        port = 80;
    } else if (strcmp(protocol, "https") == 0) {
        port = 443;
    } else {
        log_error("Unrecognized protocol");
    }
    
    // EXTRACT FILE NAME
    strcpy(file_name, basename(tmp_str));


    // DEBUGGING
    if (debug) {
        log_message("=== DEBUG INFO ===");
        printf("argv1=%s\nurl=%s\nprotocol=%s\ndomain=%s\nport=%d\npath=%s\nfile=%s\n", argv[1], url, protocol, domain, port, path, file_name);
        log_message("==================");
    }
    // exit(EXIT_SUCCESS);


    ///////////////////////
    // CONNECT TO SERVER //
    ///////////////////////


    int socket_fd;
    struct sockaddr_in server_addr;
    struct hostent *host;
    char read_buffer[MAX_BUFFER_SIZE];
    char write_buffer[MAX_BUFFER_SIZE];
    FILE *fp;

    // DNS LOOKUP
    host = gethostbyname(domain);
    if (host == NULL) {
        sprintf(tmp_str, "Could not resolve host %s", domain);
        log_error(tmp_str);
        exit(EXIT_FAILURE);
    }

    // CREATE SOCKET USING IPv4 AND TCP
    socket_fd = socket(AF_INET, SOCK_STREAM, 0); 
    if (socket_fd == -1) {
        log_error("Could not create socket");
        exit(EXIT_FAILURE);
    }
    log_message("Socket created");

    // ASSIGN IP AND PORT
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    server_addr.sin_port = htons(80);

    // CONNECT TO THE SERVER
    int connect_success = connect(socket_fd, (struct sockaddr*) &server_addr, sizeof(server_addr));
    if (connect_success == -1) {
        log_error("Could not connect to server");
        exit(EXIT_FAILURE);
    }
    log_message("Connected to server");

    // SEND REQUEST
    sprintf(write_buffer, REQUEST, path, domain, 0, "");
    int send_success = write(socket_fd, write_buffer, strlen(write_buffer));
    if(send_success == -1) {
        log_error("Could not send request");
        exit(EXIT_FAILURE); 
    }
    log_message("Request sent");

    // GET RESPONSE
    int num_bytes;
    fp = fopen(file_name, "wb");
    num_bytes = read(socket_fd, read_buffer, MAX_BUFFER_SIZE);
    read_buffer[num_bytes] = '\0';

    // REMOVE HTTP HEADERS
    char *content = strstr(read_buffer, "\r\n\r\n");
    if (content != NULL) content += 4; 
    else content = read_buffer;
    
    while (num_bytes > 0) {

        // DEBUGGING
        if (debug) {
            log_message(">>>>>>>>>>");
            log_message(read_buffer);
            log_message(">>>>>>>>>>");
            fwrite(read_buffer, 1, strlen(read_buffer), fp);
        } else {
            // WRITE TO FILE
            fwrite(content, strlen(content), 1, fp);
        }


        // GET MORE DATA
        num_bytes = read(socket_fd, read_buffer, MAX_BUFFER_SIZE);
        read_buffer[num_bytes] = '\0';
        content = read_buffer;
    }

    // CHECK IF RECEIVED CORRECTLY
    if (num_bytes == -1) {
        log_error("Could not fetch file");
        exit(EXIT_FAILURE);
    }
    log_message("File received");

    // CLOSE CONNECTION
    close(socket_fd);
    fclose(fp);
    return 0;
}