#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAX_SIZE 128

int dfs_serveur;

void *receive_thread(void *arg) {
    char buf[MAX_SIZE];
    
    while (1) {
        memset(buf, 0, MAX_SIZE);
        
        if (recvfrom(dfs_serveur, buf, MAX_SIZE, 0, NULL, NULL) > 0) {
            if (strcmp(buf, "exit") == 0) {
                printf("\nServer has left the chat.\n");
                printf("Type 'exit' to quit or continue messaging...\n");
                printf("You: ");
                fflush(stdout);
            } else {
                printf("\nServer: %s\n", buf);
                printf("You: ");
                fflush(stdout);
            }
        }
    }
    return NULL;
}

int main() {
    struct sockaddr_in adresse_serveur;
    struct sockaddr_in adresse_client;
    pthread_t recv_thread;
    char message[MAX_SIZE];
    
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
    
    // Start receive thread
    pthread_create(&recv_thread, NULL, receive_thread, NULL);
    
    while (1) {
        printf("You: ");
        fgets(message, MAX_SIZE, stdin);
        message[strcspn(message, "\n")] = 0; // Remove newline
        
        sendto(dfs_serveur, message, strlen(message), 0,
               (struct sockaddr *)&adresse_serveur, sizeof(adresse_serveur));
        
        if (strcmp(message, "exit") == 0) {
            printf("Exiting chat...\n");
            break;
        }
    }
    
    close(dfs_serveur);
    return 0;
}