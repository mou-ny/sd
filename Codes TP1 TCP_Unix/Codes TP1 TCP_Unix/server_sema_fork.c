#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <signal.h>
#include <sys/wait.h>

#define PORT 1234
#define SHM_KEY 12345
#define SEM_KEY 54321

int countWords(char *str) {
    int count = 0;
    char *t = strtok(str, " ,;:");
    while (t != NULL) {
        count++;
        t = strtok(NULL, " ,;:");
    }
    return count;
}

void sem_lock(int semid) {
    struct sembuf op = {0, -1, 0};
    semop(semid, &op, 1);
}

void sem_unlock(int semid) {
    struct sembuf op = {0, 1, 0};
    semop(semid, &op, 1);
}

void handle_sigchld(int sig) {
    (void)sig;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    int dfs_serveur, dfs_client;
    struct sockaddr_in adresse_serveur, adresse_client;
    socklen_t long_adr_client;
    char buffer[1024];

    signal(SIGCHLD, handle_sigchld);

    int shm_id = shmget(SHM_KEY, sizeof(int), IPC_CREAT | 0666);
    if (shm_id == -1) {
        perror("Shared memory error");
        exit(1);
    }
    int *totalWords = (int *)shmat(shm_id, NULL, 0);
    *totalWords = 0; 

    int sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (sem_id == -1) {
        perror("Semaphore error");
        exit(1);
    }
    semctl(sem_id, 0, SETVAL, 1);

    dfs_serveur = socket(AF_INET, SOCK_STREAM, 0);
    if (dfs_serveur == -1) {
        perror("socket error");
        exit(1);
    }

    adresse_serveur.sin_family = AF_INET;
    adresse_serveur.sin_addr.s_addr = inet_addr("127.0.0.1");
    adresse_serveur.sin_port = htons(PORT);

    if (bind(dfs_serveur, (struct sockaddr *)&adresse_serveur, sizeof(adresse_serveur)) == -1) {
        perror("bind error");
        exit(1);
    }

    listen(dfs_serveur, 5);
    printf("Server waiting for connections...\n");

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

                sem_lock(sem_id);
                *totalWords += words;
                int currentTotal = *totalWords;
                sem_unlock(sem_id);

                printf("[Client %d] Words in message: %d | Total words received from all clients: %d\n",
                       getpid(), words, currentTotal);

                char reply[256];
                sprintf(reply, "Words in this message: %d | Total words received from all clients: %d",
                        words, currentTotal);

                send(dfs_client, reply, strlen(reply), 0);
            }

            printf("[Client %d] disconnected.\n", getpid());
            close(dfs_client);
            shmdt(totalWords);
            exit(0);
        } else {
            close(dfs_client);
        }
    }

    close(dfs_serveur);
    shmdt(totalWords);
    shmctl(shm_id, IPC_RMID, NULL);
    semctl(sem_id, 0, IPC_RMID);
    return 0;
}
