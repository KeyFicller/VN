#include "visual_nurb.h"

#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

int main() {
    WSADATA wsa_data;
    SOCKET client_socket;
    sockaddr_in server_addr;
    const char* server_ip = "127.0.0.1"; // local_server

    // initialize win sock
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        std::cerr << "WSAStartup failed. Error: " << WSAGetLastError() << std::endl;
        return 1;
    }

    // create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed. Error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(VN_PORT);
    inet_pton(AF_INET, server_ip, &server_addr.sin_addr);

    // bind socket
    if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        std::cerr << "Connection failed. Error: " << WSAGetLastError() << std::endl;
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }

    // sending
    while (true)
    {
        const char* message = "Hello from Client!";
        send(client_socket, message, strlen(message), 0);
        std::cout << "Message sent: " << message << std::endl;
    }

    // accept connection from client
    closesocket(client_socket);
    WSACleanup();
    return 0;
}