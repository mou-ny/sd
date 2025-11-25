// ================= THREAD-BASED SERVER =======================
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#define PORT 1234

// Global shared counter
int totalWords = 0;

// Mutex for thread-safe access
pthread_mutex_t lock;

// Count words in a string
int countWords(char *str) {
    int count = 0;
    char *token = strtok(str, " ,;:");
    while (token != NULL) {
        count++;
        token = strtok(NULL, " ,;:");
    }
    return count;
}

// Data passed to a client thread
typedef struct {
    int clientSocket;
    struct sockaddr_in clientAddr;
} ThreadData;

void *clientHandler(void *arg) {
    ThreadData *data = (ThreadData *)arg;
    int client = data->clientSocket;
    struct sockaddr_in addr = data->clientAddr;
    free(data);   // free allocated memory (no leak)

    char buffer[1024];
    int readSize;

    printf("Client connected: %s:%d\n",
           inet_ntoa(addr.sin_addr),
           ntohs(addr.sin_port));

    while ((readSize = recv(client, buffer, sizeof(buffer) - 1, 0)) > 0) {
        buffer[readSize] = '\0';

        printf("\n[Client] Sent: \"%s\"\n", buffer);

        char temp[1024];
        strcpy(temp, buffer);
        int words = countWords(temp);

        // Lock to update global counter
        pthread_mutex_lock(&lock);
        totalWords += words;
        int currentTotal = totalWords;
        pthread_mutex_unlock(&lock);

        printf("Words in message: %d | Total words: %d\n",
               words, currentTotal);

        char reply[256];
        sprintf(reply,
                "Words in this message: %d | Total words: %d",
                words, currentTotal);

        send(client, reply, strlen(reply), 0);
    }

    printf("Client disconnected.\n");
    close(client);
    return NULL;
}

int main() {
    int serverSocket, clientSocket;
    struct sockaddr_in serverAddr, clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    pthread_mutex_init(&lock, NULL);

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddr.sin_port = htons(PORT);

    bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr));
    listen(serverSocket, 5);

    printf("Server waiting for connections...\n");

    while (1) {
        clientSocket = accept(serverSocket,
                              (struct sockaddr *)&clientAddr,
                              &clientLen);

        // Create thread data
        ThreadData *data = malloc(sizeof(ThreadData));
        data->clientSocket = clientSocket;
        data->clientAddr = clientAddr;

        pthread_t thread;
        pthread_create(&thread, NULL, clientHandler, data);
        pthread_detach(thread);   // thread cleans itself
    }

    close(serverSocket);
    pthread_mutex_destroy(&lock);
    return 0;
}
