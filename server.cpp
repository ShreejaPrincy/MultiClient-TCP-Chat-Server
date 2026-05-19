#include <iostream>
#include <thread>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <mutex>

using namespace std;

vector<int> clients;
mutex clientsMutex;

void broadcastMessage(string message, int senderSocket = -1) {
    lock_guard<mutex> lock(clientsMutex);

    for (int clientSocket : clients) {
        if (clientSocket != senderSocket) {
            send(clientSocket, message.c_str(), message.length(), 0);
        }
    }
}

void handleClient(int clientSocket) {
    char buffer[1024];

    while (true) {
        memset(buffer, 0, sizeof(buffer));

        int bytesReceived = recv(clientSocket, buffer, sizeof(buffer), 0);

        if (bytesReceived <= 0) {
            cout << "Client disconnected." << endl;

            {
                lock_guard<mutex> lock(clientsMutex);
                for (auto it = clients.begin(); it != clients.end(); ++it) {
                    if (*it == clientSocket) {
                        clients.erase(it);
                        break;
                    }
                }
            }

            close(clientSocket);
            break;
        }

        string message = "Client: ";
        message += buffer;

        cout << message << endl;
        broadcastMessage(message, clientSocket);
    }
}

void serverSendMessages() {
    string message;

    while (true) {
        getline(cin, message);

        if (message == "exit") {
            cout << "Server shutting down..." << endl;
            exit(0);
        }

        string serverMessage = "Server: " + message;
        broadcastMessage(serverMessage);
    }
}

int main() {
    int serverSocket;
    sockaddr_in serverAddress;

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (serverSocket == -1) {
        cout << "Socket creation failed." << endl;
        return 1;
    }

    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(8080);
    serverAddress.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        cout << "Binding failed." << endl;
        return 1;
    }

    if (listen(serverSocket, 5) < 0) {
        cout << "Listening failed." << endl;
        return 1;
    }

    cout << "Server started on port 8080..." << endl;
    cout << "Waiting for clients..." << endl;
    cout << "Server can also send broadcast messages." << endl;

    thread serverInputThread(serverSendMessages);
    serverInputThread.detach();

    while (true) {
        sockaddr_in clientAddress;
        socklen_t clientSize = sizeof(clientAddress);

        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddress, &clientSize);

        if (clientSocket < 0) {
            cout << "Client connection failed." << endl;
            continue;
        }

        {
            lock_guard<mutex> lock(clientsMutex);
            clients.push_back(clientSocket);
        }

        cout << "New client connected." << endl;

        string welcomeMessage = "Server: New client joined the chat.";
        broadcastMessage(welcomeMessage, clientSocket);

        thread clientThread(handleClient, clientSocket);
        clientThread.detach();
    }

    close(serverSocket);
    return 0;
}
