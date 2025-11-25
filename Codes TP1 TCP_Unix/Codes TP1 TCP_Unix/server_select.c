#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>

#define PORT 1234

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
    int server_fd, client_fd, max_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    fd_set read_fds, master_fds;

    char buffer[1024];
    int totalWords = 0;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket error");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind error");
        exit(1);
    }

    listen(server_fd, 5);
    printf("Server started. Waiting for clients...\n");

    FD_ZERO(&master_fds);
    FD_SET(server_fd, &master_fds);
    max_fd = server_fd;

    while (1) {
        read_fds = master_fds;

        if (select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0) {
            perror("select error");
            exit(1);
        }

        //checking all file descriptors
        for (int fd = 0; fd <= max_fd; fd++) {

            if (FD_ISSET(fd, &read_fds)) {

                if (fd == server_fd) {
                    client_len = sizeof(client_addr);
                    client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

                    if (client_fd < 0) {
                        perror("accept error");
                        continue;
                    }

                    FD_SET(client_fd, &master_fds);
                    if (client_fd > max_fd) max_fd = client_fd;

                    printf("New client connected: %s:%d\n",
                           inet_ntoa(client_addr.sin_addr),
                           ntohs(client_addr.sin_port));
                }
                else {
                    int read_size = recv(fd, buffer, sizeof(buffer) - 1, 0);

                    if (read_size <= 0) {
                        printf("Client disconnected (fd=%d)\n", fd);
                        close(fd);
                        FD_CLR(fd, &master_fds);
                        continue;
                    }

                    buffer[read_size] = '\0';
                    printf("[Client %d] Sent: %s\n", fd, buffer);

                    char temp[1024];
                    strcpy(temp, buffer);
                    int words = countWords(temp);

                    totalWords += words;

                    printf("Words in message: %d | Total words from ALL clients: %d\n",
                           words, totalWords);

                    char reply[256];
                    sprintf(reply,
                            "Words in this message: %d | Total words (all clients): %d",
                            words, totalWords);

                    send(fd, reply, strlen(reply), 0);
                }
            }
        }
    }

    close(server_fd);
    return 0;
}
