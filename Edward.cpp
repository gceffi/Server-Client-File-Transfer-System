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

// Function to display error messages and exit
void error(const char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char *argv[]) {
    int sockfd, n; // Socket file descriptor and read/write buffer size
    struct sockaddr_in serv_addr; // Server address structure
    struct hostent *server; // Host entity structure for storing server information

    char buffer[256]; // Buffer to store user input and received data

    // Check if port number is provided
    if (argc < 2) {
        fprintf(stderr, "usage %s port_number\n", argv[0]);
        exit(0);
    }

    int portno = atoi(argv[1]); // Get port number from command line arguments

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    // Get server information
    server = gethostbyname(SERVER_IP);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }

    // Initialize server address structure
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    // Connect to the server
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    // Main loop for user interaction
    while (1) {
        printf("Please enter the command (get filename/exit/terminate): ");
        bzero(buffer, 256);
        fgets(buffer, 255, stdin);

        // Write user command to the server
        n = write(sockfd, buffer, strlen(buffer));
        if (n < 0)
            error("ERROR writing to socket");

        // Check if user wants to exit
        if (strncmp(buffer, "exit", 4) == 0) {
            // Send a message to the server indicating the connection is closing
            const char* exit_message = "EXIT";
            write(sockfd, exit_message, strlen(exit_message));

            // Close the socket connection
            close(sockfd);
            printf("Connection closed.\n");
            break;
        }

        // Check if user wants to terminate
        if (strncmp(buffer, "terminate", 9) == 0) {
            // Send a message to the server indicating termination
            const char* terminate_message = "TERMINATE";
            write(sockfd, terminate_message, strlen(terminate_message));

            // Close the socket connection
            close(sockfd);
            printf("Connection terminated.\n");
            exit(0);
        }

        // Check if user wants to get a file
        if (strncmp(buffer, "get", 3) == 0) {
            long file_size;
            // Read file size from the server
            n = read(sockfd, &file_size, sizeof(long));
            if (n < 0)
                error("ERROR reading file size from socket");

            // Check if file is found on the server
            if (file_size < 0) {
                printf("File not found on server!\n");
                break;
            }

            char file_buffer[file_size];
            // Read file contents from the server
            n = read(sockfd, file_buffer, file_size);
            if (n < 0)
                error("ERROR reading file from socket");

            // Print information about the received file
            printf("Received file %s (%ld bytes)\n", buffer + 4, file_size);

            // Write the received file to disk
            std::ofstream file(buffer + 4, std::ios::binary);
            if (!file) {
                perror("Error creating file");
                continue;
            }

            file.write(file_buffer, file_size);
            file.close();

            // Break out of the loop after processing a single command
            break;
        }
    }

    // Close the socket connection
    close(sockfd);

    return 0;
}
