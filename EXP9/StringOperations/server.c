#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <ctype.h>

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
        return -1;

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

    return -1;
}

static void handle_client(int clientfd)
{
    char buffer[BUFFER_SIZE];
    char operation[50];
    char message[BUFFER_SIZE + 32];
    char result[BUFFER_SIZE];

    while (1)
    {
        int status = recv_line(clientfd, buffer, sizeof(buffer));

        if (status <= 0)
            break;

        if (strcmp(buffer, "exit") == 0)
            break;

        printf("Client %d sent: %s\n", getpid(), buffer);

        status = recv_line(clientfd, operation, sizeof(operation));

        if (status <= 0)
            break;

        if (strcmp(operation, "1") == 0)
        {
            int written = snprintf(
                message,
                sizeof(message),
                "Length of string = %zu",
                strlen(buffer)
            );

            if (written < 0 || (size_t)written >= sizeof(message))
                break;
        }
        else if (strcmp(operation, "2") == 0)
        {
            size_t length = strlen(buffer);

            for (size_t i = 0; i < length; i++)
                result[i] = buffer[length - i - 1];

            result[length] = '\0';

            int written = snprintf(
                message,
                sizeof(message),
                "Reversed string = %s",
                result
            );

            if (written < 0 || (size_t)written >= sizeof(message))
                break;
        }
        else if (strcmp(operation, "3") == 0)
        {
            size_t length = strlen(buffer);

            for (size_t i = 0; i < length; i++)
            {
                /*
                 * Cast to unsigned char before passing to toupper.
                 */
                result[i] = (char)toupper((unsigned char)buffer[i]);
            }

            result[length] = '\0';

            int written = snprintf(
                message,
                sizeof(message),
                "Uppercase string = %s",
                result
            );

            if (written < 0 || (size_t)written >= sizeof(message))
                break;
        }
        else if (strcmp(operation, "4") == 0)
        {
            size_t length = strlen(buffer);
            int palindrome = 1;

            for (size_t left = 0; left < length / 2; left++)
            {
                size_t right = length - left - 1;

                if (buffer[left] != buffer[right])
                {
                    palindrome = 0;
                    break;
                }
            }

            if (palindrome)
            {
                strcpy(message, "The string is a palindrome.");
            }
            else
            {
                strcpy(message, "The string is not a palindrome.");
            }
        }
        else
        {
            strcpy(message, "Invalid operation.");
        }

        if (send_line(clientfd, message) < 0)
            break;
    }

    close(clientfd);
}

int main(void)
{
    int serverfd;
    struct sockaddr_in serverAddr;

    signal(SIGCHLD, SIG_IGN);
    signal(SIGPIPE, SIG_IGN);

    serverfd = socket(AF_INET, SOCK_STREAM, 0);

    if (serverfd < 0)
    {
        perror("socket");
        return EXIT_FAILURE;
    }

    int reuse = 1;

    if (setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR,
                   &reuse, sizeof(reuse)) < 0)
    {
        perror("setsockopt");
        close(serverfd);
        return EXIT_FAILURE;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(serverfd, (struct sockaddr *)&serverAddr,
             sizeof(serverAddr)) < 0)
    {
        perror("bind");
        close(serverfd);
        return EXIT_FAILURE;
    }

    if (listen(serverfd, 5) < 0)
    {
        perror("listen");
        close(serverfd);
        return EXIT_FAILURE;
    }

    printf("String Operation Concurrent TCP Server running...\n");
    printf("Waiting for clients...\n");

    while (1)
    {
        struct sockaddr_in clientAddr;
        socklen_t addrLen = sizeof(clientAddr);

        int clientfd = accept(
            serverfd,
            (struct sockaddr *)&clientAddr,
            &addrLen
        );

        if (clientfd < 0)
        {
            if (errno == EINTR)
                continue;

            perror("accept");
            continue;
        }

        printf("New client connected.\n");

        pid_t pid = fork();

        if (pid < 0)
        {
            perror("fork");
            close(clientfd);
            continue;
        }

        if (pid == 0)
        {
            close(serverfd);
            handle_client(clientfd);
            _exit(EXIT_SUCCESS);
        }

        close(clientfd);
    }

    close(serverfd);
    return EXIT_SUCCESS;
}
