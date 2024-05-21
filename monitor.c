#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     // For close()
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>      // For struct hostent

// Function to extract the hostname from the URL
void parse_url(char *url, char *host, char **resource, char *protocol);

// Function to extract the status code from the HTTP response
int extract_status_code(const char *response);

// Function to get the status code message based on the status code
char* get_status_code_message(int status_code);

// Function to handle the HTTP response
void handle_response(const char *response);

// Function to connect to the URL and retrieve the HTTP response
int connect_to_url(const char *url);

/**
 * Function to extract the hostname from the URL.
 * 
 * @param url The URL to parse.
 * @param host The buffer to store the hostname.
 * @param resource The pointer to store the resource path.
 */
void parse_url(char *url, char *host, char **resource, char *protocol) {
    sscanf(url, "http://%s", host);
    *resource = strchr(host, '/');
    if (*resource) {
        **resource = '\0';
        (*resource)++;
    }

    // Remove trailing newline if it exists
    size_t len = strlen(host);
    if (len > 0 && host[len - 1] == '\n') {
        host[len - 1] = '\0';
    }

    // Check if the URL contains the protocol
    if (strstr(url, "http://") == url) {
        strcpy(protocol, "http");
    } else if (strstr(url, "https://") == url) {
        strcpy(protocol, "https");
    } else { // Default to HTTP
        strcpy(protocol, "http");
    }

    
}

/**
 * Function to extract the status code from the HTTP response.
 * 
 * @param response The HTTP response.
 * @return The status code extracted from the response.
 */
int extract_status_code(const char *response) {
    const char *status_line = strstr(response, "HTTP/");  // Find the start of the status line
    if (status_line) {
        int status_code;
        sscanf(status_line, "HTTP/%*d.%*d %d", &status_code);  // Extract the status code
        return status_code;
    } else {
        printf("No valid HTTP status line found.\n");
    }
}

/**
 * Function to get the status code message based on the status code.
 * 
 * @param status_code The status code.
 * @return The status code message.
 */
char* get_status_code_message(int status_code) {
    switch (status_code) {
        case 200:
            return "OK";
        case 301:
            return "Moved Permanently";
        case 302:
            return "Found";
        case 400:
            return "Bad Request";
        case 401:
            return "Unauthorized";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        case 500:
            return "Internal Server Error";
        case 501:
            return "Not Implemented";
        case 502:
            return "Bad Gateway";
        case 503:
            return "Service Unavailable";
        default:
            return "Unknown";
    }
}

/**
 * Function to handle the HTTP response.
 * 
 * @param response The HTTP response.
 */
void handle_response(const char *response) {
 
}

/**
 * Function to connect to the URL and retrieve the HTTP response.
 * 
 * @param url The URL to connect to.
 * @return 0 if successful, otherwise an error code.
 */
int connect_to_url(const char *url) {
    int sockfd;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    char host[256], *resource;
    int port;
    char protocol[10];
    char request[1024], response[4096];
    int total_received, bytes_received;

    parse_url((char *)url, host, &resource, protocol);

    if (strcmp(protocol, "http") == 0) {
        port = 80;
    } else if (strcmp(protocol, "https") == 0) {
        port = 443;
    } else {
        printf("Status: Invalid Protocol\n");
        return 1;
    }
    //printf("Host: %s\n", host);
    //printf("Resource: %s\n", resource ? resource : "/");
    //printf("Protocol: %s\n", protocol);

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        printf("Status: Network Error\n");
        return 1;
    }

    server = gethostbyname(host);
    if (server == NULL) {
        printf("Status: Network Error\n");
        close(sockfd);
        return 2;
    }


    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("Status: Network Error\n");
        close(sockfd);
        return 3;
    }

    sprintf(request, "GET /%s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", resource ? resource : "", host);
    if (send(sockfd, request, strlen(request), 0) < 0) {
        printf("Status: Network Error\n");
        close(sockfd);
        return 4;
    }

    total_received = 0;
    while ((bytes_received = recv(sockfd, response + total_received, sizeof(response) - total_received - 1, 0)) > 0) {
        total_received += bytes_received;
    }
    if (bytes_received < 0) {
        printf("Status: Network Error\n");
        close(sockfd);
        return 5;
    }
    response[total_received] = '\0';
    

    // handle the response 
    
    int status_code = extract_status_code(response);
    char* status_message = get_status_code_message(status_code);
    printf("Status: %d %s\n", status_code, status_message);


    if (status_code >= 200 && status_code < 300) { // handle OK
        const char *img_tag;
        const char *src_start;
        const char *src_end;
        char reference_url[1024];  // Assuming reference_url won't be longer than 1024 characters

        img_tag = response;
        while ((img_tag = strstr(img_tag, "<img"))) {  // Find start of <img> tag
            src_start = strstr(img_tag, "src=\"");     // Find start of src attribute
            if (src_start) {
                src_start += 5;  // Move past "src=\""
                src_end = strchr(src_start, '\"');      // Find end of URL
                if (src_end) {
                    strncpy(reference_url, src_start, src_end - src_start);
                    reference_url[src_end - src_start] = '\0';  // Null-terminate the URL string
                    
                    // if the url has no host, add the host
                    if (strstr(reference_url, "http://") == NULL && strstr(reference_url, "https://") == NULL) {
                        char new_url[1024];
                        sprintf(new_url, "%s://%s%s", protocol, host, reference_url);
                        printf("Referenced URL: %s\n", new_url);
                        //printf("Found image: %s\n", new_url);
                        connect_to_url(new_url);
                    } else {
                        //printf("Found image: %s\n", reference_url);
                        printf("Referenced URL: %s\n", reference_url);
                        connect_to_url(reference_url);
                    }
                }
            }


            img_tag = src_end;  // Move past this img tag for the next iteration
        }
    }

    if (status_code >= 300 && status_code < 400) { // handle redirects
        const char *location = strstr(response, "Location: ");
        if (location) {
            location += strlen("Location: ");
            const char *end = strstr(location, "\r\n");
            if (end) {
                char redirect_url[4096];
                strncpy(redirect_url, location, end - location);
                redirect_url[end - location] = '\0';
                printf("Redirected URL: %s\n", redirect_url);
                connect_to_url(redirect_url);
            }
        }
    }


    handle_response(response); // Handle the response
    close(sockfd);
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error opening file");
        return 1;
    }

    const size_t max_line_length = 1024; // urls are usually around 60 characters long but can sometimes be north of 2000 characters
    char line[max_line_length];

    while (fgets(line, sizeof(line), file)) {
        // Remove trailing newline if it exists
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') {
            line[len - 1] = '\0';
        }
        printf("URL: %s\n", line);
        connect_to_url(line);
        printf("\n");
    }

    fclose(file);

    return 0;
}
