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
#define REQUEST "GET %s HTTP/1.1\r\n" \
                "Host: %s\r\n" \
                "\r\n"
                // "Accept: */*\r\n" \
                // "Accept-Encoding: *\r\n" \
                // "Connection: close\r\n" \
                // "Content-type: application/x-www-form-urlencoded\r\n" \
                // "Content-length: %d\r\n" \
                // "%s\r\n"

void log_error(char message[]) {
  printf("\rERROR: %s\n", message);
}

void log_message(char message[]) {
  printf("\r%s\n", message);
}

int main(int argc, char *argv[]) {


    ///////////////////////
    // PARSE ARGS        //
    ///////////////////////

    char tmp_str[1024];
    char tmp_str2[1024];
    char *tmp_pstr;
    char *tmp_pstr2;
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
        strcpy(tmp_str, url); // START OF DOMAIN
    } else {
        strncpy(protocol, url, tmp_pstr - url);
        strcpy(tmp_str, tmp_pstr + 3); // START OF DOMAIN
    }
    
    // FIND END OF DOMAIN
    tmp_pstr = strchr(tmp_str, '/');
    
    // EXTRACT DOMAIN
    strncpy(domain, tmp_str, tmp_pstr - tmp_str);

    // PORT
    if (strcmp(protocol, "http") == 0) {
        port = 80;
    } else if (strcmp(protocol, "https") == 0) {
        port = 443;
    } else {
        sprintf(tmp_str2, "Unrecognized protocol: %s", protocol);
        log_error(tmp_str2);
        exit(EXIT_FAILURE);
    }
    
    // EXTRACT FILE NAME
    strcpy(file_name, basename(tmp_str));

    // EXTRACT PATH
    strcpy(path, tmp_pstr);
    if (strcmp(path, "") == 0) strcpy(path, "/");


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
        sprintf(tmp_str, "Could not resolve host: %s", domain);
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
    sprintf(write_buffer, REQUEST, path, domain);
    if (debug) {
        log_message("<<<<<<<<<<");
        log_message(write_buffer);
        log_message("<<<<<<<<<<");
    }
    int send_success = write(socket_fd, write_buffer, strlen(write_buffer));
    if(send_success == -1) {
        log_error("Could not send request");
        exit(EXIT_FAILURE); 
    }
    log_message("Request sent");

    // OPEN FILE
    fp = fopen(file_name, "r");
    if (fp != NULL) {
        sprintf(tmp_str, "%s already exists and will be overwritten", file_name);
        log_message(tmp_str);
    }
    fp = fopen(file_name, "wb");

    // GET RESPONSE
    int num_bytes;
    num_bytes = read(socket_fd, read_buffer, MAX_BUFFER_SIZE);

    // GET HTTP HEADERS
    char *body_begin = strstr(read_buffer, "\r\n\r\n");
    body_begin += 4;
    char headers[8192];
    strncpy(headers, read_buffer, body_begin - read_buffer);

    if (debug) {
        log_message("##########");
        log_message(headers);
        log_message("##########");
    }

    // GET CONTENT LENGTH
    tmp_pstr = strstr(headers, "Content-Length: ");
    tmp_pstr += 16;
    tmp_pstr2 = strstr(tmp_pstr, "\r\n");
    int content_length = 0;
    char content_length_str[64];
    strncpy(content_length_str, tmp_pstr, tmp_pstr2 - tmp_str);
    content_length = atoi(content_length_str);

    if (debug) {
        printf("download size: %d bytes\n", content_length);
    }

    int curr_bytes = num_bytes - (body_begin - read_buffer);
    printf("%d bytes downloaded\n", curr_bytes);
    fwrite(body_begin, 1, curr_bytes, fp);

    while (curr_bytes < content_length) {

        // GET MORE DATA
        num_bytes = read(socket_fd, read_buffer, MAX_BUFFER_SIZE);

        // CHECK IF RECEIVED CORRECTLY
        if (num_bytes == -1) {
            log_error("Could not fetch file");
            exit(EXIT_FAILURE);
        }

        curr_bytes += num_bytes;
        fwrite(read_buffer, 1, num_bytes, fp);
        printf("%d bytes downloaded\n", curr_bytes);

        // // DEBUGGING
        // if (debug) {
        //     log_message(">>>>>>>>>>");
        //     log_message(read_buffer);
        //     log_message(">>>>>>>>>>");
        //     // fwrite(content, 1, strlen(content), fp);
        //     // fwrite(read_buffer, 1, strlen(read_buffer), fp);
        // } else {
        //     // WRITE TO FILE
        //     // fwrite(content, 1, strlen(content), fp);
        // }
    }

    sprintf(tmp_str, "File received: %s", file_name);
    log_message(tmp_str);

    // CLOSE CONNECTION
    close(socket_fd);
    fclose(fp);
    return 0;
}