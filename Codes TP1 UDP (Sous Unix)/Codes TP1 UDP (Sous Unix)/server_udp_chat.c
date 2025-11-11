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
struct sockaddr_in adresse_client;
socklen_t long_adr_client;
int client_connected = 0;

void *receive_thread(void *arg) {
    char buf[MAX_SIZE];
    
    while (1) {
        memset(buf, 0, MAX_SIZE);
        long_adr_client = sizeof(adresse_client);
        
        if (recvfrom(dfs_serveur, buf, MAX_SIZE, 0, 
                     (struct sockaddr *)&adresse_client, &long_adr_client) > 0) {
            client_connected = 1;
            
            if (strcmp(buf, "exit") == 0) {
                printf("\nClient has left the chat.\n");
                printf("Type 'exit' to quit or continue messaging...\n");
                printf("You: ");
                fflush(stdout);
            } else {
                printf("\nClient: %s\n", buf);
                printf("You: ");
                fflush(stdout);
            }
        }
    }
    return NULL;
}

int main() {
    struct sockaddr_in adresse_serveur;
    pthread_t recv_thread;
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
    
    printf("Server started. Waiting for client...\n");
    
    // Start receive thread
    pthread_create(&recv_thread, NULL, receive_thread, NULL);
    
    while (1) {
        if (client_connected) {
            printf("You: ");
        }
        
        fgets(message, MAX_SIZE, stdin);
        message[strcspn(message, "\n")] = 0; // Remove newline
        
        if (client_connected) {
            sendto(dfs_serveur, message, strlen(message), 0,
                   (struct sockaddr *)&adresse_client, long_adr_client);
            
            if (strcmp(message, "exit") == 0) {
                printf("Exiting chat...\n");
                break;
            }
        }
    }
    
    close(dfs_serveur);
    return 0;
}