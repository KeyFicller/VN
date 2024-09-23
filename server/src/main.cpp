#include "visual_nurb.h"

#include "GLFW/glfw3.h"

#include <iostream>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

int main()
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;

    WSADATA wsa_data;
    SOCKET server_socket, client_socket;
    sockaddr_in server_addr, client_addr;
    int client_addr_size = sizeof(client_addr);

    // initialize win sock
    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0)
    {
        std::cerr << "WSAStartup failed. Error: " << WSAGetLastError() << std::endl;
        return 1;
    }

    // create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed. Error: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    // set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(VN_PORT);

    // bind socket
    if (bind(server_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        std::cerr << "Socket binding failed. Error: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    // listening
    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "Scoket listening failed. Error: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Waiting for a client_instance to connect ..." << std::endl;

    // accept connection from client_instance
    client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_size);
    if (client_socket == INVALID_SOCKET)
    {
        std::cerr << "Scoket acceptiing failed. Error: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    // accept message
    while (true)
    {
        char buffer[1024];
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received > 0)
        {
            // operations with message
            buffer[bytes_received % 1024] = '\0';
            std::cout << "Message recived: " << buffer << std::endl;
        }
    }

    // close socket
    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();
    return 0;
}