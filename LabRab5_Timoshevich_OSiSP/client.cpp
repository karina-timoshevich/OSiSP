#include <iostream>
#include <winsock2.h>
#include <thread>
#include <string>
#pragma comment(lib, "ws2_32.lib")

#define PORT 12345
#define BUFFER_SIZE 1024

SOCKET client_socket;

void receive_messages() {
    char buffer[BUFFER_SIZE];
    while (true) {
        int recv_size = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (recv_size <= 0) {
            std::cout << "Connection lost with the server\n";
            break;
        }
        buffer[recv_size] = '\0';
        std::cout << "\nNew message: " << buffer << "\n";
    }
}

int main() {
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    server_addr.sin_port = htons(PORT);

    if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Failed to connect to the server.\n";
        return 1;
    }

    std::string name;
    std::cout << "Enter your name: ";
    std::getline(std::cin, name);
    send(client_socket, name.c_str(), name.size(), 0);
    std::thread receiver(receive_messages);
    receiver.detach();

    while (true) {
        std::string receiver_name, message;
        std::cout << "Enter recipient's name: ";
        std::getline(std::cin, receiver_name);
        std::cout << "Enter your message: ";
        std::getline(std::cin, message);
        std::string full_message = name + "|" + receiver_name + "|" + message;
        send(client_socket, full_message.c_str(), full_message.size(), 0);
    }

    closesocket(client_socket);
    WSACleanup();
    return 0;
}
