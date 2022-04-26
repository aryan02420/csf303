#include <unistd.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details.
#include <pthread.h>
#include <semaphore.h>
#define MAX 5000

int sock_fd, active_conn = 0, input_done = 0;
char server_data[MAX];
sem_t mutex1, mutex2, mutex3;

struct client_Data
{
	int conn_fd;
    struct sockaddr_in client;	
};

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

void verifyServerListen(int tmp)
{
	if (tmp == 0) 
    {
        printf("The Server is listening.\n");
    }

    else
    {    
        exit(0);
    }
}

void verifyClientAccept(int tmp)
{
	if (tmp < 0) 
    {
        printf("Server failed to accept.\n");
        exit(0);
    }

    else
    {
        // printf("Server accepted the client.\n");
    }
}

char *strrev(char *str){
    char c, *front, *back;

    if(!str || !*str)
        return str;
    for(front=str,back=str+strlen(str)-1;front < back;front++,back--){
        c=*front;*front=*back;*back=c;
    }
    return str;
}

void *serverThread(void *var)
{
    char buf[MAX], ip[MAX];
    struct client_Data *data = (struct client_Data*) var;
    

    bzero(ip, MAX);
    inet_ntop(AF_INET, &(data->client.sin_addr), ip, INET_ADDRSTRLEN);
    printf("Accepted connection from client with address: %s:%d\n", ip, data->client.sin_port);


    while(1)
    {        
        // Read the data sent from the client
        bzero(buf, MAX);
        read(data->conn_fd, buf, MAX);

        sem_wait(&mutex2);
        input_done++;
        sem_post(&mutex2);

        if(strcmp(buf, "exit") == 0)
        {
            sem_wait(&mutex1);
            printf("Client with address %s:%d closed the connection.\n", ip, data->client.sin_port);
            active_conn--;
            sem_post(&mutex1);

            // Close the connection
            close(data->conn_fd);

            pthread_exit(0);
        }        


        strrev(buf);
        printf("Client with IP: %s, PORT: %d sent Data: %s\n", ip, data->client.sin_port, buf);
        
        
        while(input_done != active_conn){};

        
        sem_wait(&mutex3);
        if(input_done == active_conn)
        {
            // Read a line from as input and send it to the client
            sleep(1);
            printf("Server Input: ");
            bzero(server_data, MAX);
            scanf("%[^\n]%*c", server_data);
            input_done = 0;
        }
        sem_post(&mutex3);

        write(data->conn_fd, server_data, MAX);

    }

}

int main(int argc, char* argv[])
{

    //input is port number
    int port = atoi(argv[1]);


    int tmp, conn_fd = 0, len;;
    struct sockaddr_in addr, client;
    char buf[MAX], ip[MAX], msg[MAX] = "success", msg1[MAX] = "refused.";
    len = sizeof(client);
    
    sem_init(&mutex1, 0, 1);
    sem_init(&mutex2, 0, 1);
    sem_init(&mutex3, 0, 1);

    // Create the socket file descriptor
    sock_fd = socket(AF_INET, SOCK_STREAM, 0);


    // Verify if the creation of the socket 
    // file descriptor is successful
	verifySocketCreation(sock_fd);


	// Specify the server address by assigning IP and PORT
    bzero(&addr, sizeof(addr));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);


	// Bind the network socket to the newly created port
    // and verify if the binding was successful
	tmp = bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr));
    verifySocketBinding(tmp);


    // Listen at the port for any connections
    tmp = listen(sock_fd, 5);
    verifyServerListen(tmp);


    while(1)
    {      

        pthread_t thread_id;
        struct client_Data data[10];        

        if(active_conn < 4)
        {
            // Accept the data packet from the client
            conn_fd = accept(sock_fd, (struct sockaddr*)&client, &len);

            
            // Verifying if the data packet from client was accepted
            verifyClientAccept(conn_fd);


            write(conn_fd, msg, MAX);


            data[active_conn].conn_fd = conn_fd;
            data[active_conn].client = client;


            // Create the thread to handle the client
            if(pthread_create(&thread_id, NULL, serverThread, &data[active_conn]))
            {
                printf("Thread creation error!");
                exit(-1);
            }

            else
            {
                active_conn++;
            }
        }

        else
        {
            conn_fd = accept(sock_fd, (struct sockaddr*)&client, &len);
            write(conn_fd, msg1, MAX);
        }

    }

    sem_destroy(&mutex1);
    sem_destroy(&mutex2);

    return 0;

}