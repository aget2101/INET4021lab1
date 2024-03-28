#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUFFER_SIZE 4096
#define CONFIG_FILE "httpd.conf" // Configuration file name

// Configuration structure
struct Configuration {
    int max_connections;            // Maximum number of connections allowed
    char root_directory[256];       // Root directory for serving files
    char index_filename[256];       // Default index filename
    int port;                       // Port number on which the server listens
};

// Function to read the configuration from file
void read_configuration(const char* filename, struct Configuration* config) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening configuration file");
        exit(EXIT_FAILURE);
    }

    // Read configuration options from the file
    fscanf(file, "# httpd.conf configuration file\nPort: %d\nMaxConnections: %d\nRootDirectory: %s\nDefaultIndex: %s",
       &config->port,
       &config->max_connections,
       config->root_directory,
       config->index_filename);

    fclose(file);
}

// Function to get the content type based on file extension
const char* get_content_type(const char* path) {
    const char *dot = strrchr(path, '.');
    if(dot) {
        if (strcmp(dot, ".html") == 0) return "text/html";
        else if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0) return "image/jpeg";
        else if (strcmp(dot, ".gif") == 0) return "image/gif";
        // Add more content types as needed
    }
    return "application/octet-stream";
}

int main() {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed.\n");
        return 1;
    }

    // Read server configuration from the configuration file
    struct Configuration config;
    read_configuration(CONFIG_FILE, &config);
    
    // Socket and address variables
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_fd == INVALID_SOCKET) {
        perror("socket failed");
        WSACleanup();
        return 1;
    }

    // Address setup
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(config.port);

    // Bind
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) == SOCKET_ERROR) {
        perror("bind failed");
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }

    // Listen
    if (listen(server_fd, config.max_connections) == SOCKET_ERROR) {
        perror("listen");
        closesocket(server_fd);
        WSACleanup();
        return 1;
    }
    printf("Server is listening on port %d\n", config.port);

    // Accept connections and handle requests
    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (new_socket == INVALID_SOCKET) {
            perror("accept");
            closesocket(server_fd);
            WSACleanup();
            return 1;
        }
        printf("Connection established\n");

        // Read the HTTP request from the client
        char request_buffer[BUFFER_SIZE];
        int bytes_received = recv(new_socket, request_buffer, BUFFER_SIZE, 0);
        if (bytes_received == SOCKET_ERROR) {
            perror("recv failed");
            closesocket(new_socket);
            continue;
        }
        request_buffer[bytes_received] = '\0'; // Null-terminate the buffer
        
        // Extract the method and path from the request
        char method[16], path[1024];
        sscanf(request_buffer, "%s %s", method, path);

        if (strcmp(method, "GET") == 0) {
            // Construct the full path to the requested file
            char full_path[1024];
            if (strcmp(path, "/") == 0) {
                snprintf(full_path, sizeof(full_path), "%s\\%s", config.root_directory, config.index_filename);
            } else {
                snprintf(full_path, sizeof(full_path), "%s%s", config.root_directory, path);
            }
            
            printf("Attempting to serve: %s\n", full_path); // Debug print

            // Open the requested file
            FILE* file = fopen(full_path, "rb");
            if (file == NULL) {
                // File not found, send a 404 response
                const char* not_found_response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
                send(new_socket, not_found_response, strlen(not_found_response), 0);
            } else {
                // File found, send its contents as the response
                fseek(file, 0, SEEK_END);
                long file_size = ftell(file);
                fseek(file, 0, SEEK_SET);

                // Allocate memory for file content
                char* file_content = (char*)malloc(file_size);
                if (file_content == NULL) {
                    perror("malloc failed");
                    fclose(file);
                    closesocket(new_socket);
                    continue;
                }

                // Read file content into buffer
                size_t bytes_read = fread(file_content, 1, file_size, file);
                if (bytes_read != file_size) {
                    perror("fread failed");
                    free(file_content);
                    fclose(file);
                    closesocket(new_socket);
                    continue;
                }

                // Close the file
                fclose(file);

                // Construct the HTTP response header
                char response_header[1024];
                sprintf(response_header, "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\nContent-Type: %s\r\n\r\n", file_size, get_content_type(full_path));

                // Send the HTTP response header
                send(new_socket, response_header, strlen(response_header), 0);

                // Send the file content
                send(new_socket, file_content, file_size, 0);

                // Free allocated memory for file content
                free(file_content);
            }
        } else {
            // Unsupported HTTP method, send a 405 response
            const char* method_not_allowed_response = "HTTP/1.1 405 Method Not Allowed\r\nContent-Length: 0\r\n\r\n";
            send(new_socket, method_not_allowed_response, strlen(method_not_allowed_response), 0);
        }

        // Close the client socket
        closesocket(new_socket);
    }

    // Close the server socket and clean up Winsock
    closesocket(server_fd);
    WSACleanup();

    return 0;
}
