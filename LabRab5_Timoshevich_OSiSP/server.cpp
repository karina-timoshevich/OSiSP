#include <iostream>
#include <winsock2.h>
#include <map>
#include <thread>
#include <string>
#include <sstream>
#include <queue>
#pragma comment(lib, "ws2_32.lib")

#define PORT 12345
#define BUFFER_SIZE 1024

std::map<SOCKET, std::string> clients;
std::map<std::string, std::queue<std::string>> message_queue;

void send_to_client(const std::string& message, const std::string& receiver_name) {
    bool found = false;
    for (const auto& client : clients) {
        if (client.second == receiver_name) {
            send(client.first, message.c_str(), message.size(), 0);
            found = true;
            break;
        }
    }

    if (!found) {
        message_queue[receiver_name].push(message);
    }
}

void handle_client(SOCKET client_socket) {
    char buffer[BUFFER_SIZE];
    std::string client_name;
    int recv_size = recv(client_socket, buffer, BUFFER_SIZE, 0);
    if (recv_size > 0) {
        buffer[recv_size] = '\0';
        client_name = buffer;
        clients[client_socket] = client_name;
        std::cout << client_name << " connected\n";
        while (!message_queue[client_name].empty()) {
            send(client_socket, message_queue[client_name].front().c_str(), message_queue[client_name].front().size(), 0);
            message_queue[client_name].pop();
        }
    }

    while (true) {
        int recv_size = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (recv_size <= 0) {
            std::cout << client_name << " disconnected\n";
            closesocket(client_socket);
            clients.erase(client_socket);
            break;
        }
        buffer[recv_size] = '\0';

        std::stringstream ss(buffer);
        std::string sender, receiver, message;
        std::getline(ss, sender, '|');
        std::getline(ss, receiver, '|');
        std::getline(ss, message);
        std::string full_message = sender + ": " + message;
        send_to_client(full_message, receiver);
    }
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);
    bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_socket, SOMAXCONN);

    std::cout << "Server started and waiting for connections...\n";

    while (true) {
        SOCKET client_socket = accept(server_socket, nullptr, nullptr);
        std::thread client_thread(handle_client, client_socket);
        client_thread.detach();
    }

    closesocket(server_socket);
    WSACleanup();
    return 0;
}
