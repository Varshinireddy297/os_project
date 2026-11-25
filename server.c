#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080

int balance = 1000;
pthread_mutex_t lock;

void *handle_client(void *socket_desc)
{
    int client_sock = *(int *)socket_desc;
    free(socket_desc);

    char buffer[1024];
    char response[1024];

    while (1)
    {
        memset(buffer, 0, sizeof(buffer));

        int read_size = recv(client_sock, buffer, sizeof(buffer), 0);

        if (read_size <= 0)
        {
            printf("Client disconnected\n");
            break;
        }

        if (strncmp(buffer, "DEPOSIT", 7) == 0)
        {
            int amount;
            sscanf(buffer, "DEPOSIT %d", &amount);

            pthread_mutex_lock(&lock);
            balance += amount;
            sprintf(response,
                    "Deposited %d. New Balance: %d",
                    amount, balance);
            pthread_mutex_unlock(&lock);
        }
        else if (strncmp(buffer, "WITHDRAW", 8) == 0)
        {
            int amount;
            sscanf(buffer, "WITHDRAW %d", &amount);

            pthread_mutex_lock(&lock);

            if (balance >= amount)
            {
                balance -= amount;
                sprintf(response,
                        "Withdrawn %d. New Balance: %d",
                        amount, balance);
            }
            else
            {
                sprintf(response,
                        "Insufficient Balance");
            }

            pthread_mutex_unlock(&lock);
        }
        else if (strncmp(buffer, "BALANCE", 7) == 0)
        {
            pthread_mutex_lock(&lock);
            sprintf(response,
                    "Current Balance: %d",
                    balance);
            pthread_mutex_unlock(&lock);
        }
        else if (strncmp(buffer, "EXIT", 4) == 0)
        {
            break;
        }
        else
        {
            strcpy(response,
                   "Invalid Command");
        }

        send(client_sock,
             response,
             strlen(response),
             0);
    }

    close(client_sock);
    pthread_exit(NULL);
}

int main()
{
    int server_fd, client_sock;
    struct sockaddr_in server, client;
    socklen_t client_len;

    pthread_mutex_init(&lock, NULL);

    server_fd = socket(AF_INET,
                       SOCK_STREAM,
                       0);

    if (server_fd == -1)
    {
        printf("Socket creation failed\n");
        return 1;
    }

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(PORT);

    bind(server_fd,
         (struct sockaddr *)&server,
         sizeof(server));

    listen(server_fd, 5);

    printf("Server listening on port %d...\n",
           PORT);

    while (1)
    {
        client_len = sizeof(client);

        client_sock =
            accept(server_fd,
                   (struct sockaddr *)&client,
                   &client_len);

        if (client_sock < 0)
        {
            printf("Accept failed\n");
            continue;
        }

        printf("Client connected\n");

        pthread_t thread_id;

        int *new_sock = malloc(sizeof(int));
        *new_sock = client_sock;

        if (pthread_create(&thread_id,
                           NULL,
                           handle_client,
                           (void *)new_sock) < 0)
        {
            printf("Thread creation failed\n");
            return 1;
        }

        pthread_detach(thread_id);
    }

    pthread_mutex_destroy(&lock);
    close(server_fd);

    return 0;
}