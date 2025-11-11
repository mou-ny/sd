#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

int countWords(char *str) {
    int count = 0;
    char *t = strtok(str, " ,;:");
    while (t != NULL) {
        count++;
        t = strtok(NULL, " ,;:");
    }
    return count;
}

void handle_sigchld(int sig) {
    (void)sig; // unused
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    int dfs_serveur, dfs_client;
    struct sockaddr_in adresse_serveur, adresse_client;
    socklen_t long_adr_client;
    char buffer[1024];
    int totalWords = 0;

    signal(SIGCHLD, handle_sigchld); 

    dfs_serveur = socket(AF_INET, SOCK_STREAM, 0);
    if (dfs_serveur == -1) {
        perror("socket error");
        exit(1);
    }

    adresse_serveur.sin_family = AF_INET;
    adresse_serveur.sin_addr.s_addr = inet_addr("127.0.0.1");
    adresse_serveur.sin_port = htons(1234);

    if (bind(dfs_serveur, (struct sockaddr *)&adresse_serveur, sizeof(adresse_serveur)) == -1) {
        perror("bind error");
        close(dfs_serveur);
        exit(1);
    }

    if (listen(dfs_serveur, 5) == -1) {
        perror("listen error");
        close(dfs_serveur);
        exit(1);
    }

    printf("Server waiting for connection...\n");

    while (1) {
        long_adr_client = sizeof(adresse_client);
        dfs_client = accept(dfs_serveur, (struct sockaddr *)&adresse_client, &long_adr_client);

        if (dfs_client < 0) {
            perror("accept error");
            continue;
        }

        printf("\nClient connected from %s:%d\n",
               inet_ntoa(adresse_client.sin_addr),
               ntohs(adresse_client.sin_port));

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork error");
            close(dfs_client);
            continue;
        }

        if (pid == 0) {
            close(dfs_serveur);
            int read_size;

            while ((read_size = recv(dfs_client, buffer, sizeof(buffer) - 1, 0)) > 0) {
                buffer[read_size] = '\0';
                printf("\n[Client %d] Sent: \"%s\"\n", getpid(), buffer);

                char temp[1024];
                strcpy(temp, buffer);

                int words = countWords(temp);
                totalWords += words;

                printf("[Client %d] Words in message: %d | Total words received: %d\n",
                       getpid(), words, totalWords);

                char reply[256];
                sprintf(reply, "Words in this message: %d | Total words you sent: %d",
                        words, totalWords);

                send(dfs_client, reply, strlen(reply), 0);
            }

            printf("[Client %d] Disconnected.\n", getpid());
            close(dfs_client);
            exit(0);
        } else {
            close(dfs_client);
        }
    }

    close(dfs_serveur);
    return 0;
}
