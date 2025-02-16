#include <stdio.h>
#include <stdlib.h>  // exit(), malloc()
#include <string.h>  // memset(), string ops
#include <arpa/inet.h>  // sockaddr_in, inet_pton()
#include <unistd.h>  // close()
#include <openssl/evp.h>
#include <openssl/rand.h>

#define AES_KEY_SIZE 32
#define AES_BLOCK_SIZE 16

// AES Decryption using EVP API
int aes_decrypt(const unsigned char *ciphertext, unsigned char *plaintext, int ciphertext_len, unsigned char *key, unsigned char *iv) {
    EVP_CIPHER_CTX *ctx;
    int len;
    int plaintext_len;

    // Create decryption context
    if (!(ctx = EVP_CIPHER_CTX_new())) {
        perror("EVP_CIPHER_CTX_new failed");
        return -1;
    }

    // Initialize AES-256-CBC decryption
    if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, key, iv) != 1) {
        perror("EVP_DecryptInit_ex failed");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }

    // Perform decryption
    if (EVP_DecryptUpdate(ctx, plaintext, &len, ciphertext, ciphertext_len) != 1) {
        perror("EVP_DecryptUpdate failed");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plaintext_len = len;

    // Finalize decryption
    if (EVP_DecryptFinal_ex(ctx, plaintext + len, &len) != 1) {
        perror("EVP_DecryptFinal_ex failed");
        EVP_CIPHER_CTX_free(ctx);
        return -1;
    }
    plaintext_len += len;

    EVP_CIPHER_CTX_free(ctx);
    return plaintext_len;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: %s <port> <output-file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int server_port = atoi(argv[1]);
    const char *output_file = argv[2];

    // Create server socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    // Set up server address
    struct sockaddr_in server_address;
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(server_port);
    server_address.sin_addr.s_addr = INADDR_ANY;

    // Bind socket
    if (bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Bind failed");
        close(server_socket);
        return EXIT_FAILURE;
    }

    // Listen for connections
    if (listen(server_socket, 5) < 0) {
        perror("Listen failed");
        close(server_socket);
        return EXIT_FAILURE;
    }
    printf("Server is listening on port %d...\n", server_port);

    // Accept connection
    struct sockaddr_in client_address;
    socklen_t client_address_len = sizeof(client_address);
    int client_socket = accept(server_socket, (struct sockaddr*)&client_address, &client_address_len);
    if (client_socket < 0) {
        perror("Connection failed");
        close(server_socket);
        return EXIT_FAILURE;
    }
    printf("Client connected from: %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));

    // Open output file
    FILE *file = fopen(output_file, "wb");
    if (!file) {
        perror("Failed to open file");
        close(client_socket);
        close(server_socket);
        return EXIT_FAILURE;
    }

    // Receive IV from client
    unsigned char iv[AES_BLOCK_SIZE];
    if (recv(client_socket, iv, AES_BLOCK_SIZE, 0) != AES_BLOCK_SIZE) {
        perror("Failed to receive IV");
        fclose(file);
        close(client_socket);
        close(server_socket);
        return EXIT_FAILURE;
    }

    // Initialize AES decryption
    unsigned char aes_key[AES_KEY_SIZE] = "thisisaverysecurekey1234567890"; 
    unsigned char buffer[AES_BLOCK_SIZE * 2];
    unsigned char decrypted_buffer[AES_BLOCK_SIZE * 2];
    int received_bytes;

    // Receive and decrypt the file
    while ((received_bytes = recv(client_socket, buffer, AES_BLOCK_SIZE, 0)) > 0) {
        // Decrypt received data
        int decrypted_len = aes_decrypt(buffer, decrypted_buffer, received_bytes, aes_key, iv);
        if (decrypted_len < 0) {
            perror("Decryption failed");
            break;
        }
        // Write decrypted data to file
        fwrite(decrypted_buffer, 1, received_bytes, file);
    }

    if (received_bytes < 0) {
        perror("Data reception failed");
    } else {
        printf("File received and decrypted successfully.\n");
    }

    fclose(file);
    close(client_socket);
    close(server_socket);
    return EXIT_SUCCESS;
}
