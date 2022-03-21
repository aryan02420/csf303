#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#define MAX 80
#define SA struct sockaddr

int main()
{
	int sockfd, connfd, len, port, size;
	struct sockaddr_in servaddr, cli;
	struct stat st;
	char buff[MAX], str[INET_ADDRSTRLEN], filename[FILENAME_MAX], newfile[MAX];
	FILE *fp;

	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd == -1) 
	{
		printf("[-]Socket creation failed\n");
		exit(0);
	}
	else
		printf("[+]Socket successfully created\n");
	bzero(&servaddr, sizeof(servaddr));

	printf(">Enter port number: ");
	scanf("%d", &port);
	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	servaddr.sin_port = htons(port);

	// Binding newly created socket to given IP and verification
	if((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) 
	{
		printf("[-]Socket bind failed\n");
		exit(0);
	}
	else
	{
		printf("[+]Socket successfully binded\n");
		inet_ntop(AF_INET, &(servaddr.sin_addr), str, INET_ADDRSTRLEN);
		printf("Socket bound to IP: %s and Port: %d\n", str, port);
	}

	// Now server is ready to listen and verification
	if((listen(sockfd, 10)) != 0) 
	{
		printf("[-]Listen failed\n");
		exit(0);
	}
	else
		printf("[+]Server listening\n");
	len = sizeof(cli);

	// Accept the data packet from client and verification
	connfd = accept(sockfd, (SA*)&cli, &len);
	if(connfd < 0) 
	{
		printf("[-]Server accept failed\n");
		exit(0);
	}
	else
		printf("[-]Server accepts the client\n");

	// Read filename from client
	read(connfd, filename, sizeof(filename));
	printf("Client requesting data from filename: %s\n", filename);

	// Check if file exists
	fp = fopen(filename, "r");
	bzero(buff, MAX);
  	if(fp == NULL) 
	{
		printf("[-]File not found\n");
		write(connfd, buff, sizeof(buff));

		// Read path of client 
		read(connfd, newfile, sizeof(newfile));

		// Send empty file to client
		fp = fopen(newfile, "w");
		printf("Empty file %s sent to client\n", filename);
  	}
	else 
	{
		printf("[+]File found, sending file data\n");

		// Check size of file data
		stat(filename, &st);
		size = st.st_size;
		if(size < 10)
			fgets(buff, size, fp);
		else
			fgets(buff, 11, fp);

		// Write data to client
		write(connfd, buff, sizeof(buff));
		printf("Data sent.\n");
	}

	// close the socket
	close(sockfd);
}
