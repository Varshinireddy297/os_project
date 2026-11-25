#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080

int main()
{
    int sock;
    struct sockaddr_in server;

    char message[1024];
    char server_reply[1024];

    sock = socket(AF_INET,
                  SOCK_STREAM,
                  0);

    if (sock == -1)
    {
        printf("Could not create socket\n");
        return 1;
    }

    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_family = AF_INET;
    server.sin_port = htons(PORT);

    if (connect(sock,
                (struct sockaddr *)&server,
                sizeof(server)) < 0)
    {
        printf("Connection failed\n");
        return 1;
    }

    printf("Connected to server\n");

    while (1)
    {
        printf("\nCommands:\n");
        printf("DEPOSIT amount\n");
        printf("WITHDRAW amount\n");
        printf("BALANCE\n");
        printf("EXIT\n");

        printf("\nEnter Command: ");

        fgets(message,
              sizeof(message),
              stdin);

        send(sock,
             message,
             strlen(message),
             0);

        if (strncmp(message,
                    "EXIT",
                    4) == 0)
        {
            break;
        }

        memset(server_reply,
               0,
               sizeof(server_reply));

        recv(sock,
             server_reply,
             sizeof(server_reply),
             0);

        printf("Server: %s\n",
               server_reply);
    }

    close(sock);

    return 0;
}