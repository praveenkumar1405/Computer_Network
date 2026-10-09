#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 5000
#define BUFFER_SIZE 1024

static int send_all(int sockfd, const char *data, size_t length)
{
    size_t total = 0;

    while (total < length)
    {
        ssize_t sent = send(sockfd, data + total, length - total, 0);

        if (sent < 0)
        {
            if (errno == EINTR)
                continue;

            return -1;
        }

        if (sent == 0)
            return -1;

        total += (size_t)sent;
    }

    return 0;
}

static int send_line(int sockfd, const char *text)
{
    char line[BUFFER_SIZE + 2];
    int length = snprintf(line, sizeof(line), "%s\n", text);

    if (length < 0 || (size_t)length >= sizeof(line))
    {
        fprintf(stderr, "Input is too long.\n");
        return -1;
    }

    return send_all(sockfd, line, (size_t)length);
}

static int recv_line(int sockfd, char *buffer, size_t size)
{
    size_t i = 0;

    while (i + 1 < size)
    {
        char c;
        ssize_t received = recv(sockfd, &c, 1, 0);

        if (received < 0)
        {
            if (errno == EINTR)
                continue;

            return -1;
        }

        if (received == 0)
            return 0;

        if (c == '\n')
        {
            buffer[i] = '\0';
            return 1;
        }

        buffer[i++] = c;
    }

    buffer[size - 1] = '\0';

    /* Discard the rest of an overlong line. */
    while (1)
    {
        char c;
        ssize_t received = recv(sockfd, &c, 1, 0);

        if (received <= 0 || c == '\n')
            break;
    }

    fprintf(stderr, "Received line was too long.\n");
    return -1;
}

int main(void)
{
    int sockfd;
    char message[BUFFER_SIZE];
    char operation[50];
    char buffer[BUFFER_SIZE];

    struct sockaddr_in serverAddr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) <= 0)
    {
        fprintf(stderr, "Invalid server address.\n");
        close(sockfd);
        return EXIT_FAILURE;
    }

    if (connect(sockfd, (struct sockaddr *)&serverAddr,
                sizeof(serverAddr)) < 0)
    {
        perror("connect");
        close(sockfd);
        return EXIT_FAILURE;
    }

    printf("Connected to String Operation Server.\n");

    while (1)
    {
        printf("\nEnter string: ");
        fflush(stdout);

        if (fgets(message, sizeof(message), stdin) == NULL)
            break;

        message[strcspn(message, "\n")] = '\0';

        if (send_line(sockfd, message) < 0)
        {
            perror("send");
            break;
        }

        if (strcmp(message, "exit") == 0)
            break;

        printf("\nString Operations:\n");
        printf("1. Find Length\n");
        printf("2. Reverse String\n");
        printf("3. Convert to Uppercase\n");
        printf("4. Check Palindrome\n");

        printf("Enter operation: ");
        fflush(stdout);

        if (fgets(operation, sizeof(operation), stdin) == NULL)
            break;

        operation[strcspn(operation, "\n")] = '\0';

        if (send_line(sockfd, operation) < 0)
        {
            perror("send");
            break;
        }

        int result = recv_line(sockfd, buffer, sizeof(buffer));

        if (result <= 0)
        {
            printf("Server disconnected.\n");
            break;
        }

        printf("\nServer Result: %s\n", buffer);
    }

    close(sockfd);
    return EXIT_SUCCESS;
}
