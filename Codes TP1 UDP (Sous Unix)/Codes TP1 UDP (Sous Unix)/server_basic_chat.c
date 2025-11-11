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
    socklen_t long_adr_client;
    struct sockaddr_in adresse_serveur;
    struct sockaddr_in adresse_client;
    char buf[MAX_SIZE];
    char message[MAX_SIZE];

    dfs_serveur = socket(AF_INET, SOCK_DGRAM, 0);
    if (dfs_serveur == -1) {
        perror("socket");
        exit(1);
    }

    adresse_serveur.sin_family = AF_INET;
    inet_pton(AF_INET, "127.0.0.1", &adresse_serveur.sin_addr.s_addr);
    adresse_serveur.sin_port = htons(1234);

    if (bind(dfs_serveur, (struct sockaddr *)&adresse_serveur, 
             sizeof(adresse_serveur)) == -1) {
        perror("bind error");
        exit(1);
    }

    printf("Server started. Waiting for messages...\n");

    while (1) {
        // Receive from client
        memset(buf, 0, MAX_SIZE);
        long_adr_client = sizeof(adresse_client);
        
        if (recvfrom(dfs_serveur, buf, MAX_SIZE, 0, 
                     (struct sockaddr *)&adresse_client, &long_adr_client) == -1) {
            perror("recvfrom");
            continue;
        }
        
        printf("Client: %s\n", buf);
        
        if (strcmp(buf, "exit") == 0) {
            printf("Client left the chat. Waiting for new client...\n");
            continue;  // Changed from break to continue
        }

        // Send to client
        printf("You: ");
        fgets(message, MAX_SIZE, stdin);
        message[strcspn(message, "\n")] = 0;
        
        sendto(dfs_serveur, message, strlen(message), 0,
               (struct sockaddr *)&adresse_client, long_adr_client);
        
        if (strcmp(message, "exit") == 0) {
            printf("Exiting chat...\n");
            break;  // Server exits only when YOU type exit
        }
    }

    close(dfs_serveur);
    return 0;
}