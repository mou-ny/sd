#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int countWords(char *str) {
    int count = 0;
    char *t = strtok(str, " ,;:");
    while (t != NULL) {
        count++;
        t = strtok(NULL, " ,;:");
    }
    return count;
}

int main() {
    int dfs_serveur, dfs_client, read_size;
    struct sockaddr_in adresse_serveur, adresse_client;
    socklen_t long_adr_client;
    char buffer[1024];
    int totalWords = 0;

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
        exit(1);
    }

    listen(dfs_serveur, 5);
    printf("Server waiting for connection...\n");

    while (1) {
        long_adr_client = sizeof(adresse_client);
        dfs_client = accept(dfs_serveur, (struct sockaddr *)&adresse_client, &long_adr_client);

        if (dfs_client < 0) {
            perror("accept error");
            continue;
        }

        printf("\nClient connected!\n");

        while ((read_size = recv(dfs_client, buffer, sizeof(buffer), 0)) > 0) {
            buffer[read_size] = '\0';&
            printf("\nReceived: \"%s\"\n", buffer);

            char temp[1024];
            strcpy(temp, buffer);

            int words = countWords(temp);
            totalWords += words;

            printf("Words in this message: %d\n", words);
            printf("Total words received: %d\n", totalWords);

            char reply[256];
            sprintf(reply, "Words in this message: %d | Total words you sent : %d", words, totalWords);

            send(dfs_client, reply, strlen(reply), 0);
        }

        printf("Client disconnected.\n");
        close(dfs_client);
    }

    close(dfs_serveur);
    return 0;
}
