#include <stdio.h>
#include <stdlib.h> // exit() and malloc()
#include <string.h> // memset() and string ops
#include <arpa/inet.h> // sockaddr_in and inet_pton()
#include <unistd.h> // close()

#define BUFFER_SIZE 1024

int main (int argc, char *argv[]) {
    // Program name (./client), Port, IP, File-to-Send
    if (argc != 4) {
        printf("Enter the following parameters: %s <server_ip> <port> <file-to-send>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *server_ip = argv[1];
    int client_port = atoi(argv[2]); // Convert string to int
    const char *file_to_send = argv[3];

    // Create socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        printf("Creating socket failed.\n");
        return EXIT_FAILURE;
    }

    // Set up server address structure
    struct sockaddr_in server_address; // Declares server_address variable of type struct sockaddr_in
    memset(&server_address, 0, sizeof(server_address)); // Initialize all bytes in server_address to 0
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(client_port);

    // Convert IP from text to binary
    if (inet_pton(AF_INET, server_ip, &server_address.sin_addr) <= 0) {
        printf("Invalid IP address\n");
        close(client_socket);
        return EXIT_FAILURE;
    }

    // Connect to server
    if (connect(client_socket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        printf("Connection failed\n");
        close(client_socket);
        return EXIT_FAILURE;
    }
    printf("Connected to server at %s:%d\n", server_ip, client_port);

    // Open file to send
    FILE *file = fopen(file_to_send, "rb");
    if (!file) {
        perror("Failed to open file\n");
        close(client_socket);
        return EXIT_FAILURE;
    }

    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (send(client_socket, buffer, bytes_read, 0) < 0) {
            perror("File send failed.\n");
            fclose(file);
            close(client_socket);
            return EXIT_FAILURE;
        }
    }


    fclose(file);
    close(client_socket);
    return EXIT_SUCCESS;
}