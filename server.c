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

int readFile(char **data, char *filename) {
    FILE *file;
    int numBytes;

    file = fopen(filename, "r");
    if(file == NULL)
        return 1;

    fseek(file, 0, SEEK_END);
    numBytes = ftell(file);
    fseek(file, 0, SEEK_SET);

    *data = (char*)calloc(numBytes, sizeof(char));
    if(*data == NULL)
        return 1;

    fread(*data, sizeof(char), numBytes, file);
    fclose(file);

    return 0;
}

int getRoute(char **route, char *requestdata) {
    char temp[2048];
    strcpy(temp, requestdata);
    int i=0;
    while(temp[i] != '/') {
        i += 1;
    }
    *route = temp+i;
    while(temp[i] != 0x20) {
        i += 1;
    }
    temp[i] = 0x0;
    return 0;
}

int main() {
    int serverSocket;
    struct sockaddr_in serverAddress;
    const char request200[2048] = "HTTP/1.1 200 OK\r\n\n";
    const char request404[2048] = "HTTP/1.1 404 NOT FOUND\r\n\n";

    char httpHeader[2048];
    char pages[2048] = "./pages";
    

    if (bindServer(&serverSocket, serverAddress, 1337) == -1) {
        printf("Bind not successful\n");
        return 1;
    }

    if (bindServer(&serverSocket, serverAddress, 1337) == -2) {
        printf("Socket Options not successful\n");
        return 1;
    }
    
    listen(serverSocket, 5);
    int clientSocket;
    char *route;
    char requestData[2048];

    clientSocket = accept(serverSocket, NULL, NULL);
    recv(clientSocket, requestData, sizeof(requestData), 0);
    getRoute(&route, requestData);

    char *htmlData;
    if (readFile(&htmlData, strcat(pages, route)) != 0) {
        strncpy(httpHeader, request404, 2048);
        send(clientSocket, httpHeader, sizeof(httpHeader), 0);
        return 1;
    }

    strncpy(httpHeader, request200, 2048);
    strncat(httpHeader, htmlData, 2048);

    send(clientSocket, httpHeader, sizeof(httpHeader), 0);
    close(clientSocket);
    return 0;
}
