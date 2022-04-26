#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <arpa/inet.h>
#define MAX 5000

void verifySocketCreation(int tmp)
{
	if (tmp != -1) 
    {
        // printf("Socket created successfully.\n");
    }

    else
    {
        printf("Socket creation failed.\n");
		exit(0);
    }
}

void verifySocketBinding(int tmp)
{
	if (tmp == 0) 
    {
        // printf("Socket binding successful.\n");
    }

    else
    {
        printf("Socket binding failed.\n");
        exit(0);
    }
}

void verifyClientServerConnection(int tmp)
{
	if (tmp == 0) 
	{
		// printf("Connected to the server.\n");
	}

	else
	{
		printf("Connection to the server has failed.\n");
		exit(0);
	}
		
}

char *strrev(char *str)
{
    char c, *front, *back;

    if(!str || !*str)
	{
		return str;
	}

    for(front = str, back = str + strlen(str) - 1; front < back; front++, back--)
	{
        c = *front;
		*front = *back;
		*back = c;
    }

    return str;
}

int main(int argc, char* argv[])
{
    //take the server IP, Port as input
    in_addr_t servIP = inet_addr(argv[1]);
    int servPort = atoi(argv[2]);

    int client_fd, sock_fd, tmp, i = 0;
    struct sockaddr_in addr, servAddr;
    char buf[MAX], name[MAX], ans[MAX], id[MAX], data[MAX]; 


	// Create the socket file descriptor
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);


    // Verify if the creation of the socket 
    // file descriptor is successful
	verifySocketCreation(sock_fd);


	// Specify the server address by assigning IP and PORT
    bzero(&servAddr, sizeof(servAddr));
    servAddr.sin_addr.s_addr = servIP;
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(servPort);


	// Connect the client and server socket
	tmp = connect(sock_fd, (struct sockaddr*)&servAddr, sizeof(servAddr));
	
	
	// Verify the connection
	verifyClientServerConnection(tmp);


	// Read the cconfirmation sent from the server
	bzero(buf, MAX);
	read(sock_fd, buf, MAX);


	if(strcmp(buf, "success") == 0)
	{
		while(1)
		{
			// Read a line from as input and send it to the server
			bzero(data, MAX);
			printf("Client Input: ");
			scanf("%[^\n]%*c", data);
			write(sock_fd, data, MAX);


			if(strcmp(data, "exit") == 0)
			{
				write(sock_fd, data, MAX);
				break;
			}


			// Read the data sent from the server
			bzero(buf, MAX);
			read(sock_fd, buf, MAX);
			strrev(buf);
			printf("Server sent Data: %s\n", buf);
			
		}		
	}

	else
	{
		printf("Connection %s (Client limit exceeded)\n", buf);
	}


	// Close the connection
	close(sock_fd);

	return 0;

}