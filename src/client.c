#include <stdio.h>
#include <stdlib.h> // exit() and malloc()
#include <string.h> // memset() and string ops
#include <arpa/inet.h> // sockaddr_in and inet_pton()
#include <unistd.h> // close()
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/dh.h>

#define BUFFER_SIZE 1024
#define AES_KEY_SIZE 32
#define AES_BLOCK_SIZE 16

int aes_encrypt (const unsigned char *plaintext, unsigned char *ciphertext, int plaintext_len, unsigned char *key, unsigned char *iv) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int ciphertext_len;

    // Create encryption context
    if (!(ctx = EVP_CIPHER_CTX_new())) {
        perror("EVP_CIPHER_CTX_new Failed\n.");
        return -1;
    }

    // Initialize AES Encryption
    if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        perror("EVP_EncryptInit_ex failed");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    // Data encryption
    if (EVP_EncryptUpdate(ctx, ciphertext, &len, plaintext, plaintext_len) != 1) {
        perror("EVP_EncryptUpdate failed");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    ciphertext_len = len;

    // Finalize encryption
    if (EVP_EncryptFinal_ex(ctx, ciphertext + len, &len) != 1) {
        perror("EVP_EncryptFinal_ex failed");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    ciphertext_len += len;

    // Free the context
    EVP_CIPHER_CTX_free(ctx);

    return ciphertext_len;
}

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

    unsigned char aes_key[AES_KEY_SIZE] = "thisisaverysecurekey1234567890";
    unsigned char iv[AES_BLOCK_SIZE];
    RAND_bytes(iv, AES_BLOCK_SIZE);

    send(client_socket, iv, AES_BLOCK_SIZE, 0);

    // Open file to send
    FILE *file = fopen(file_to_send, "rb");
    if (!file) {
        perror("Failed to open file\n");
        close(client_socket);
        return EXIT_FAILURE;
    }
        unsigned char buffer[AES_BLOCK_SIZE];
    unsigned char encrypted_buffer[AES_BLOCK_SIZE + AES_BLOCK_SIZE]; // Encryption output buffer
    int bytes_read;

    // Read and send file data
    while ((bytes_read = fread(buffer, 1, AES_BLOCK_SIZE, file)) > 0) {
        int encrypted_len = aes_encrypt(buffer, encrypted_buffer, bytes_read, aes_key, iv);
        if (encrypted_len < 0) {
            perror("Encryption failed");
            break;
        }

        if (send(client_socket, encrypted_buffer, encrypted_len, 0) < 0) {
            perror("File send failed");
            break;
        }
    }

    fclose(file);
    close(client_socket);
    printf("File sent successfully.\n");
    return EXIT_SUCCESS;
}