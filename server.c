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

    if(bind(*serverSocket, (struct sockaddr*) &server, sizeof(server)) == -1) {
        return 1;
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

int main() {
    
    char *htmlFile;
    char *htmlData;
    char *responseData;
    int serverSocket;
    struct sockaddr_in serverAddress;
    char httpHeader[2048] = "HTTP/1.1 200 OK\r\n\n";
    
    if (readFile(&htmlData, "./pages/index.html") != 0) 
        return 1;

    strcat(httpHeader, htmlData);
    free(htmlData);

    if (bindServer(&serverSocket, serverAddress, 1337) != 0) {
        printf("Bind not successful");
        return 1;
    }
    
    listen(serverSocket, 5);
    int clientSocket;
    while(1) {
        clientSocket = accept(serverSocket, NULL, NULL);
        send(clientSocket, httpHeader, sizeof(httpHeader), 0);
        close(clientSocket);
    }

    free(responseData);
    return 0;
}
