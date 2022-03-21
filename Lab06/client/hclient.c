#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#define MAX 80
#define SA struct sockaddr

int main()
{
	int sockfd, connfd, port;
	struct sockaddr_in servaddr, cli;
	char buff[MAX], str[INET_ADDRSTRLEN], filename[FILENAME_MAX], cwd[MAX];;
	FILE *f;

	// Socket create and verification
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
	printf(">Enter IP address: ");
	scanf("%s", str);
	
	// Assign IP, PORT
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, str, &(servaddr.sin_addr));
	servaddr.sin_port = htons(port);

	// Connect the client socket to server socket
	if(connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) 
	{
		printf("[-]Connection with the server failed\n");
		exit(0);
	}
	else
		printf("[+]Connected to the server\n");

	printf(">Enter filename to access from server: ");
	scanf("%s", filename);

	// Send filename to server
	write(sockfd, filename, sizeof(filename));

	// Read data from server
	bzero(buff, sizeof(buff));
	read(sockfd, buff, sizeof(buff));
	if(buff[0] != '\0')
	{
		// Create new file with server data
		printf("Recieved data: %s\n", buff);
		f = fopen(filename, "w");
		fputs(buff, f);
		fclose(f);
		printf("New file %s created.\n", filename);
	}
	else
	{
		// Write client path to server	
		getcwd(cwd, sizeof(cwd));
		strcat(cwd, "/");
		strcat(cwd, filename);
		write(sockfd, cwd, sizeof(cwd));
	}

	// close the socket
	close(sockfd);
}
