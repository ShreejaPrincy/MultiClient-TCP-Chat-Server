#include <iostream>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

void receiveMessages(int clientSocket) {
    char buffer[1024];

    while (true) {
        memset(buffer, 0, sizeof(buffer));

        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesReceived <= 0) {
            cout << "Disconnected from server." << endl;
            close(clientSocket);
            exit(0);
        }

        cout << buffer << endl;
    }
}

int main() {
    int clientSocket;
    sockaddr_in serverAddress;

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (clientSocket == -1) {
        cout << "Socket creation failed." << endl;
        return 1;
    }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        cout << "Connection failed." << endl;
        return 1;
    }

    cout << "Connected to server." << endl;
    cout << "Type messages below:" << endl;

    thread receiveThread(receiveMessages, clientSocket);
    receiveThread.detach();

    string message;

    while (true) {
        getline(cin, message);

        if (message == "exit") {
            cout << "Disconnecting..." << endl;
            close(clientSocket);
            break;
        }

        send(clientSocket, message.c_str(), message.length(), 0);
    }

    return 0;
}
