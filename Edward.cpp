#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT 12345 // The port number, change if needed
#define SERVER_IP "127.0.0.1" // The server IP address

void error(const char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]) {
    int sockfd, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 2) {
        fprintf(stderr, "usage %s port_number\n", argv[0]);
        exit(0);
    }

    int portno = atoi(argv[1]);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    server = gethostbyname(SERVER_IP);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    while (1) {
        printf("Please enter the command (get filename/exit/terminate): ");
        bzero(buffer, 256);
        fgets(buffer, 255, stdin);

        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0)
            error("ERROR writing to socket");

        if (strncmp(buffer, "exit", 4) == 0) {
    // Send a message to the server indicating the connection is closing
    const char* exit_message = "EXIT";
    write(sockfd, exit_message, strlen(exit_message));

    // Close the socket connection
    close(sockfd);
    printf("Connection closed.\n");
    break;
        }      


        if (strncmp(buffer, "terminate", 9) == 0) {
    // Send a message to the server indicating termination
    const char* terminate_message = "TERMINATE";
    write(sockfd, terminate_message, strlen(terminate_message));

    // Close the socket connection
    close(sockfd);
    printf("Connection terminated.\n");
    exit(0);
}

        if (strncmp(buffer, "get", 3) == 0) {
            long file_size;
            n = read(sockfd, &file_size, sizeof(long));
            if (n < 0)
                error("ERROR reading file size from socket");

            if (file_size < 0) {
                printf("File not found on server!\n");
                break;
            }

            char file_buffer[file_size];
            n = read(sockfd, file_buffer, file_size);
            if (n < 0)
                error("ERROR reading file from socket");

            printf("Received file %s (%ld bytes)\n", buffer + 4, file_size);

            std::ofstream file(buffer + 4, std::ios::binary);
            if (!file) {
                perror("Error creating file");
                continue;
            }

            file.write(file_buffer, file_size);
            file.close();

            // Break out the loop after processing a single command
            break;
        }
    }

    // Close the socket connection
    close(sockfd);

    return 0;
}
