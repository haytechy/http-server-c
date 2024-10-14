#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAXSIZE 2048

int bindServer(int *serverSocket, struct sockaddr_in server, int port) {
    *serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = INADDR_ANY;

    int reuse = 1;
    if (setsockopt(*serverSocket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        return -2;
    }

    if(bind(*serverSocket, (struct sockaddr*) &server, sizeof(server)) == -1) {
        return -1;
    }

    return 0;
}

int readFile(char **data, char *route) {
    char filename[MAXSIZE] = "./pages";
    strncat(filename, route, MAXSIZE);
    FILE *file;
    int numBytes;

    if(strcmp(strchr(filename, '.'), ".gif") == 0) {
        file = fopen(filename, "rb");
    }
    else {
        file = fopen(filename, "r");
    }

    if(file == NULL) {
        printf("File not found \n");
        return -1;
    }

    fseek(file, 0, SEEK_END);
    numBytes = ftell(file);
    rewind(file);

    *data = (char*)calloc(numBytes, sizeof(char));
    if(*data == NULL) {
        printf("Allocation Failed \n");
        return -2;
    }

    fread(*data, numBytes, 1, file);
    fclose(file);

    return numBytes;
}


int getRoute(char **route, char *requestdata) {
    char temp[MAXSIZE];
    strcpy(temp, requestdata);
    int i=0;
    int base=0;
    while(temp[i] != '/') {
        i += 1;
    }
    base = i;
    while(temp[i] != 0x20) {
        i += 1;
    }
    temp[i] = 0x0;
    strcpy(*route, temp+base);
    return 0;
}

int sendResponse(int socket, const char* header, int headerLen, const char* body, int bodyLen) {
    char* response = (char*) calloc(headerLen+bodyLen, sizeof(char)); 
    memcpy(response, header, headerLen);
    memcpy(response+headerLen, body, bodyLen);
    send(socket, response, headerLen+bodyLen, 0);
    free(response);
    return 0;
}


int main() {
    const char response200[MAXSIZE] = "HTTP/1.1 200 OK\r\n\r\n";
    const char response404[MAXSIZE] = "HTTP/1.1 404 NOT FOUND\r\n\r\n";
    const char response500[MAXSIZE] = "HTTP/1.1 500 Internal Server Error\r\n\r\n";

    struct sockaddr_in serverAddress;
    int serverSocket;
    int bind = bindServer(&serverSocket, serverAddress, 1337);

    if (bind == -1) {
        printf("Bind not successful\n");
        return 1;
    }

    if (bind == -2) {
        printf("Socket options not sucessful\n");
        return 1;
    }

    printf("Starting Server on http://localhost:1337\n");
    fflush(stdout);

    
    int clientSocket;
    int fileBytes;
    char requestData[MAXSIZE];
    char *htmlData;
    char *route = (char*) calloc(MAXSIZE, sizeof(char));
    const char* header;

    listen(serverSocket, 5);

    while(1)  {
        clientSocket = accept(serverSocket, NULL, NULL);
        requestData[0] = 0;
        if (recv(clientSocket, requestData, sizeof(requestData), 0) == -1) 
            continue;
        getRoute(&route, requestData);
        
        if(strcmp(route, "/") == 0) {
            strcpy(route, "/index.html");
        }

        fileBytes = readFile(&htmlData, route);

        if (fileBytes == -1) {
            fileBytes = readFile(&htmlData, "/404.html");
            if (fileBytes >= 0) {
                header = response404;
                sendResponse(clientSocket, header, strlen(header), htmlData, fileBytes);
                close(clientSocket);
                continue;
            }
        }

        if (fileBytes >= 0) {
            header = response200;
            sendResponse(clientSocket, header, strlen(header), htmlData, fileBytes);
            close(clientSocket);
            continue;
        }

        header = response500;
        sendResponse(clientSocket, header, strlen(header) , htmlData, fileBytes);
        close(clientSocket);
    }
    free(htmlData);
    free(route);
    return 0;
}
