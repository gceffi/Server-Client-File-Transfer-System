#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string>
#include <filesystem>

#define PORT 12345 // Change the port number if needed
#define REPOSITORY_DIR "/mnt/c/Users/godsw/Documents/Assignment 2/Repository/" // Path to the Repository directory

std::string filename;

// Function to display error messages and exit
void error(const char *msg) {
    perror(msg);
    exit(1);
}

// Function to send a file to the client
void send_file(int sockfd, const std::string& filepath) {
    // Extract the filename from the filepath
    filename = std::filesystem::path(filepath).filename().string();

    // Open the file
    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "A client requested the file \"" << filename << "\"" << std::endl;
        std::cerr << "That file is missing!" << std::endl;
        long error_code = -1;
        write(sockfd, &error_code, sizeof(long)); // Notify the client about the error
        return;
    }

    // Get the file size
    file.seekg(0, std::ios::end);
    long file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    // Send file size to client
    write(sockfd, &file_size, sizeof(long));

    // Read and send file contents
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    while (!file.eof()) {
        file.read(buffer, BUFFER_SIZE);
        int bytes_read = file.gcount();
        if (bytes_read > 0) {
            write(sockfd, buffer, bytes_read);
        }
    }

    // Print a message indicating the file sent to the client
    std::cout << "A client requested the file \"" << filename << "\"" << std::endl;
    std::cout << "Sent " << file_size << " bytes" << std::endl;

    file.close();
}

// Function to handle client requests
void handle_client(int sockfd) {
    char buffer[256];
    bzero(buffer, 256);
    int n = read(sockfd, buffer, 255); // Read client request

    if (n < 0)
        error("ERROR reading from socket");

    if (strncmp(buffer, "get", 3) == 0) {
        // Extract filename from client request
        std::string request(buffer + 4);
        size_t pos = request.find_first_of("\n\r");
        if (pos != std::string::npos) {
            request.erase(pos);
        }

        // Construct full file path
        std::string filepath = REPOSITORY_DIR + request;

        // Send the file to the client
        send_file(sockfd, filepath);
    } else if (strncmp(buffer, "exit", 4) == 0) {
        // Handle 'exit' command
        printf("Client exited\n");
        close(sockfd);
    } else if (strncmp(buffer, "terminate", 9) == 0) {
        // Handle 'terminate' command
        printf("Goodbye!\n");
        close(sockfd);
        exit(0);
    } else {
        std::cerr << "Invalid command received: " << buffer << std::endl;
    }
}

int main() {
    int sockfd, newsockfd;
    socklen_t clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;

    // Create a socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    // Initialize server address structure
    bzero((char *)&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    // Bind the socket to the specified port
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    // Listen for incoming connections
    listen(sockfd, 5);
    clilen = sizeof(cli_addr);

    // Main loop to accept and handle client connections
    while (1) {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");

        // Handle the client request
        handle_client(newsockfd);

        close(newsockfd); // Close the connection with the client
    }

    close(sockfd); // Close the server socket
    return 0;
}
