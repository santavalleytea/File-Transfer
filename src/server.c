#include <stdio.h>
#include <stdlib.h> // exit() and malloc()
#include <string.h> // memset() and string ops
#include <arpa/inet.h> // sockaddr_in and inet_pton()
#include <unistd.h> // close()

#define BUFFER_SIZE 1024

int main (int argc, char *argv[]) {
    // Check for valid arguments
    if (argc != 3) {
        printf("Enter the following parameters: %s <port> <output-file>\n", argv[0]); // output-file; where the sent file will save
        return EXIT_FAILURE;
    }

    int server_port = atoi(argv[1]);
    const char *output_file = argv[2];

    // Create server socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        printf("Creating socket failed\n");
        return EXIT_FAILURE;
    }

    // Set up server address structure
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    server_address.sin_addr.s_addr = INADDR_ANY; // Listen on any available interfaces

    // Bind socket to the specified port
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Bind Failed");
        close(server_socket);
        return EXIT_FAILURE;
    }

    // Listen for connections
    if (listen(server_socket, 5) < 0) {
        perror("Failed to listen for incoming connections.\n");
        close(server_socket);
        return EXIT_FAILURE;
    }
    printf("Server is listening on port %d...\n", server_port);

    // Accept connection from client
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    int client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_len);
    if (client_socket < 0) {
        perror("Connection failed\n");
        close(server_socket);
        return EXIT_FAILURE;
    }
    printf("Client connected from: %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

    // Open output file
    FILE *file = fopen(output_file, "wb");
    if (!file) {
        perror("Failed to open file.\n");
        close(client_socket);
        close(server_socket);
        return EXIT_FAILURE;
    }

    // Receive data and write to file
    char buffer[BUFFER_SIZE];
    size_t received_bytes;
    while ((received_bytes = recv(client_socket, buffer, BUFFER_SIZE, 0)) > 0) {
        fwrite(buffer, 1, received_bytes, file);
    }

    if (received_bytes < 0) {
        perror("Data reception failed.\n");
    } else {
        printf("File received successfully.\n");
    }

    fclose(file);
    close(client_socket);
    close(server_socket);
    return EXIT_SUCCESS;
} 