#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MAX_SIZE 128

int main() {
    int dfs_serveur;
    struct sockaddr_in adresse_serveur;
    struct sockaddr_in adresse_client;
    char message[MAX_SIZE];
    char buf[MAX_SIZE];

    dfs_serveur = socket(AF_INET, SOCK_DGRAM, 0);
    if (dfs_serveur == -1) {
        perror("socket error");
        exit(1);
    }

    adresse_client.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &adresse_client.sin_addr.s_addr);
    adresse_client.sin_port = htons(1254);

    if (bind(dfs_serveur, (struct sockaddr *)&adresse_client, 
             sizeof(adresse_client)) == -1) {
        perror("bind error");
        exit(1);
    }

    adresse_serveur.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &adresse_serveur.sin_addr.s_addr);
    adresse_serveur.sin_port = htons(1234);

    printf("Client started. Type 'exit' to quit.\n");

    while (1) {
        // Send to server
        printf("You: ");
        fgets(message, MAX_SIZE, stdin);
        message[strcspn(message, "\n")] = 0;
        
        sendto(dfs_serveur, message, strlen(message), 0,
               (struct sockaddr *)&adresse_serveur, sizeof(adresse_serveur));
        
        if (strcmp(message, "exit") == 0) {
            printf("Exiting chat...\n");
            break;
        }

        // Receive from server
        memset(buf, 0, MAX_SIZE);
        if (recvfrom(dfs_serveur, buf, MAX_SIZE, 0, NULL, NULL) > 0) {
            printf("Server: %s\n", buf);
            
            if (strcmp(buf, "exit") == 0) {
                printf("Server left the chat.\n");
                break;
            }
        }
    }

    close(dfs_serveur);
    return 0;
}