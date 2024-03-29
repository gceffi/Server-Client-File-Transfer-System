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
#define SAVE_PATH "/mnt/c/Users/godsw/Documents/Assignment 2/Received/" // Saves the files in here

void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Function to receive a file from the server
void receive_file(int sockfd, const std::string& filename) {
    long file_size;
    int n = read(sockfd, &file_size, sizeof(long));
    if (n < 0)
        error("ERROR reading file size from socket");

    if (file_size < 0) {
        printf("File not found on server!\n");
        return;
    }

    // Open file for writing
    std::string filepath = SAVE_PATH + filename;
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        perror("Error creating file");
        return;
    }

    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];

    // Read file contents from the socket and write them to the file
    long bytes_received = 0;
    while (bytes_received < file_size) {
        int bytes_to_read = std::min(BUFFER_SIZE, static_cast<int>(file_size - bytes_received));
        n = read(sockfd, buffer, bytes_to_read);
        if (n < 0)
            error("ERROR reading file from socket");

        file.write(buffer, n);
        bytes_received += n;
    }

    file.close();

    // Print message indicating the file received
    std::cout << "Received file \"" << filename << "\" (" << file_size << " bytes)" << std::endl;
}


int main(int argc, char *argv[]) {
    int sockfd, n; // Socket file descriptor and read/write buffer size
    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256]; // Buffer to store user input and received data

    // Check if port number is provided
    if (argc < 2) {
        fprintf(stderr, "usage %s port_number\n", argv[0]);
        exit(0);
    }

    int portno = atoi(argv[1]); // Get port number from line arguments

    // Creates a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    //Get server information
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

        //Checks if user wants to exit or terminate
        if (strncmp(buffer, "exit", 4) == 0 || strncmp(buffer, "terminate", 9) == 0) {
            close(sockfd);
            printf("Connection closed.\n");
            break;
        }
        
        //Checks if user wants to get a file
        if (strncmp(buffer, "get", 3) == 0) {
            // Extract filename from command
            std::string filename(buffer + 4);
            size_t pos = filename.find_first_of("\n\r");
            if (pos != std::string::npos) {
                filename.erase(pos);
            }

            // Receive file from server
            receive_file(sockfd, filename);
        }
    }

    return 0;
}