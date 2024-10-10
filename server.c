#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>



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
    char filename[2048] = "./pages";
    strncat(filename, route, 2048);
    FILE *file;
    int numBytes;

    file = fopen(filename, "r");
    if(file == NULL) {
        printf("File not found \n");
        return 1;
    }

    fseek(file, 0, SEEK_END);
    numBytes = ftell(file);
    fseek(file, 0, SEEK_SET);

    *data = (char*)calloc(numBytes, sizeof(char));
    if(*data == NULL) {
        printf("Allocation Failed \n");
        return 1;
    }

    fread(*data, sizeof(char), numBytes, file);
    fclose(file);

    return 0;
}

int getRoute(char **route, char *requestdata) {
    char temp[2048];
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

int sendResponse(int socket, const char* header, char* body) {
    char response[2048]; 
    strncpy(response, header, 2048);
    strncat(response, body, 2048);
    send(socket, response, sizeof(response), 0);
    return 0;
}

int main() {
    const char response200[2048] = "HTTP/1.1 200 OK\r\n\n";
    const char response404[2048] = "HTTP/1.1 404 NOT FOUND\r\n\n";
    const char response500[2048] = "HTTP/1.1 Internal Server Error\r\n\n";

    struct sockaddr_in serverAddress;
    int serverSocket;
    int bind = bindServer(&serverSocket, serverAddress, 1337);

    if (bind == -1) {
        printf("Bind not successful");
        return 1;
    }

    if (bind == -2) {
        printf("Socket options not sucessful");
        return 1;
    }
    
    int clientSocket;
    int file;
    char requestData[2048];
    char *htmlData;
    char *route = (char*) calloc(2048, sizeof(char));
    const char* header;

    listen(serverSocket, 5);

    while(1)  {
        clientSocket = accept(serverSocket, NULL, NULL);
        recv(clientSocket, requestData, sizeof(requestData), 0);
        getRoute(&route, requestData);
        
        if(strcmp(route, "/") == 0) {
            if (readFile(&htmlData, "/index.html") == 0) {
                header = response200;
                sendResponse(clientSocket, header, htmlData);
                close(clientSocket);
                continue;
            }
        }

        file = readFile(&htmlData, route);

        if (file == 1) {
            if (readFile(&htmlData, "/404.html") == 0) {
                header = response404;
                sendResponse(clientSocket, header, htmlData);
                close(clientSocket);
                continue;
            }
        }

        if (file == 0) {
            header = response200;
            sendResponse(clientSocket, header, htmlData);
            close(clientSocket);
            continue;
        }

        header = response500;
        sendResponse(clientSocket, header, htmlData);
        close(clientSocket);
    }
    free(htmlData);
    free(route);
    return 0;
}
