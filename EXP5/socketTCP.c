#include <stdio.h>
#include <string.h>
#include<stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORTNO 5001

int main()
{
    int ls; // Listening Scoket
    int s;  // Connecting Socket
    char buffer[256];
    char *ptr = buffer;
    int len = 0;  // No. of Bytes recieved
    int maxLen = sizeof(buffer);
    int n = 0;
    int waitSize = 16; //  Maximum no. of connection that can wait in the queue

    struct sockaddr_in serverAddr;
    struct sockaddr_in clientAddr;

    socklen_t clientAddrLen;

    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET; 
    serverAddr.sin_port = htons(PORTNO);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if((ls = socket(PF_INET,SOCK_STREAM,0)) < 0)
    {
        perror("Error : Listening Socket Creation Failed");
        exit(1);
    }

    if((bind(ls, (struct sockaddr*)&serverAddr, sizeof(serverAddr))) < 0)
    {
        perror("Error : Bind Listening Socket with Server-Address Failed !");
        exit(1);
    }

    listen(ls, waitSize);

    while(1)
    {
        printf("Waiting for Client...\n");
        clientAddrLen = sizeof(clientAddr);
        if((s = accept(ls, (struct sockaddr *)&clientAddr, &clientAddrLen)) < 0)
        {
            perror("Error : There was an error in accepting the connection !");
            exit(1);
        }
        printf("Client Connected !!\n");
        printf("Client IP Address : %s\n" , inet_ntoa(clientAddr.sin_addr));

        len = 0;
        ptr = buffer;

        while((n = recv(s, ptr, maxLen - len, 0)) > 0)
        {
            len += n;
            ptr += n;
        }

        if(n < 0)
        {
            perror("Error : Cannot Recieve Data");
            exit(1);
        }

        *ptr = '\0';
        int number = atoi(buffer);
        printf("Integer : %d\n", number);

        int digit, result = 0;

        while(number > 0)
        {
            digit = number % 10;
            result += digit;
            number /= 10;
        }

        char resultantArray[100];
        sprintf(resultantArray, "Sum : %d\n", result);


        if((send(s, resultantArray, strlen(resultantArray), 0)) < 0)
        {
            perror("Error : Cannot send Data/Message");
        }

        close(s);
    }

    close(ls);
    return 0;
}

