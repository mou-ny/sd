#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main() {
    int dfs_client;
    struct sockaddr_in adresse_serveur;
    char buffer[1024];
    char serverReply[256];

    dfs_client = socket(AF_INET, SOCK_STREAM, 0);

    adresse_serveur.sin_family = AF_INET;
    adresse_serveur.sin_addr.s_addr = inet_addr("127.0.0.1");
    adresse_serveur.sin_port = htons(1234);

    if (connect(dfs_client, (struct sockaddr *)&adresse_serveur, sizeof(adresse_serveur)) == -1) {
        perror("Connection error");
        exit(1);
    }

    printf("Connected to server.\n");
    printf("Type or exit :\n");

    while (1) {
        printf("> ");
        fgets(buffer, sizeof(buffer), stdin);
        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "exit") == 0)
            break;

        send(dfs_client, buffer, strlen(buffer), 0);

        int recvSize = recv(dfs_client, serverReply, sizeof(serverReply), 0);
        serverReply[recvSize] = '\0';
        printf("Server response: %s\n", serverReply);
    }

    close(dfs_client);
    return 0;
}
