#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main() {

    char serverMessage[256] = "You have reached the server";

    int serverSocket;
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in serverAddress;  
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(1337);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if(bind(serverSocket, (struct sockaddr*) &serverAddress, sizeof(serverAddress)) == -1) {
        printf("Bind not successful");
        return 1;
    }

    listen(serverSocket, 5);
    
    int clientSocket;
    clientSocket = accept(serverSocket, NULL, NULL);

    send(clientSocket, serverMessage, sizeof(serverMessage), 0);

    close(serverSocket);
    return 0;
}
