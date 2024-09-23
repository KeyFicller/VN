#include "visual_nurb.h"

#include "camera.h"
#include "shader.h"
#include "vector_matrix.h"

#include "GLFW/glfw3.h"
#include "GL/GLU.h"

#include <iostream>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

GLUnurbsObj* nurbs;

void initNURBS()
{
    nurbs = gluNewNurbsRenderer();
    gluNurbsProperty(nurbs, GLU_SAMPLING_TOLERANCE, 25.0);
    gluNurbsProperty(nurbs, GLU_DISPLAY_MODE, GLU_FILL);
}

void test(const ::VN::Mat4f& mat)
{
    float ctrlpoints[4][4][3] = {
        {{0100.0,270.0,0.0},//p00
        {105.0,180.0,0.0},//p01
        {110.0,160.0,0.0},//p02
        {155.0,100.0,0.0}},//p03
        {{180.0,200.0,0.0},//p10
        {190.0,130.0,0.0},//p11
        {200.0,110.0,0.0},//p12
        {240.0,70.0,0.0}},//p13
        {{310.0,200.0,0.0},//p20
        {320.0,130.0,0.0},//p21
        {330.0,110.0,0.0},//p22
        {370.0,70.0,0.0}},//p23
        {{420.0,270.0,0.0},//p30
        {430.0,180.0,0.0},//p31
        {440.0,160.0,0.0},//p32
        {490.0,120.0,1.0}}//p33
    };

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            ::VN::Vec3f pos(ctrlpoints[i][j][0], ctrlpoints[i][j][1] , ctrlpoints[i][j][2]);
            pos = (mat * ::VN::Vec4f(pos, 1.0f)).xyz();
            for (int k = 0; k < 3; k++)
                ctrlpoints[i][j][k] = pos[k];
        }
    }

    glPushMatrix();
    //绘制控制点与控制线
    glScaled(0.2, 0.2, 0.2);

    glPointSize(4.0f);
    glColor3f(0.0, 0.0, 1.0);
    glColor3f(0, 0, 1);
    glBegin(GL_POINTS);
    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
            glVertex3fv(ctrlpoints[i][j]);

    }
    glEnd();
    //绘制控制线
    glLineWidth(1.5f);
    glColor3f(0.0, 1.0, 1.0);
    for (int i = 0; i < 4; i++)
    {
        glBegin(GL_LINE_STRIP);
        for (int j = 0; j < 4; j++)
            glVertex3fv(ctrlpoints[i][j]);
        glEnd();

        glBegin(GL_LINE_STRIP);
        for (int j = 0; j < 4; j++)
            glVertex3fv(ctrlpoints[j][i]);
        glEnd();
    }
    //绘制B样条控制曲面
    GLfloat knots[8] = { 0.0,0.0,0.0,0.0,1.0,1.0,1.0,1.0 }; //B样条控制向量
    glLineWidth(1.0f);
    glColor3f(0.0, 0.0, 0.0);

    gluNurbsProperty(nurbs, GLU_SAMPLING_TOLERANCE, 10.0); //设置属性
    gluNurbsProperty(nurbs, GLU_DISPLAY_MODE, GLU_OUTLINE_POLYGON);
    gluBeginSurface(nurbs);//开始绘制
    gluNurbsSurface(nurbs,
        8, knots,
        8, knots,
        4 * 3,
        3,
        &ctrlpoints[0][0][0],
        4, 4,
        GL_MAP2_VERTEX_3);

    gluEndSurface(nurbs); //结束绘制

    for (int j = 0; j <= 8; j++)
    {
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i <= 30; i++)
            glEvalCoord2f((GLfloat)i / 30.0, (GLfloat)j / 8.0);
        glEnd();
        glBegin(GL_LINE_STRIP);
        for (int i = 0; i <= 30; i++)
            glEvalCoord2f((GLfloat)j / 8.0, (GLfloat)i / 30.0);
        glEnd();
    }
    glPopMatrix();
}

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

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        return -1;
    }

    ::VN::camera cam(window, 45.f, 1.5f, 0.1f, 1000.f);

    glfwSetWindowUserPointer(window, &cam);

    // add event callbacks
    glfwSetScrollCallback(window, [](GLFWwindow* window, double xOffset, double yOffset) {
        ::VN::mouse_scrolled_event event(static_cast<float>(xOffset), static_cast<float>(yOffset));
        ((::VN::camera*)glfwGetWindowUserPointer(window))->on_event(event);
    });

    glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
        glViewport(0, 0, width, height);
    });

    initNURBS();

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    float vertices[] = {
         0.5f,  0.5f, 0.0f,  // top right
         0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  // bottom left
        -0.5f,  0.5f, 0.0f   // top left 
    };
    unsigned int indices[] = {  // note that we start from 0!
        0, 1, 3,  // first Triangle
        1, 2, 3   // second Triangle
    };
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
    glBindBuffer(GL_ARRAY_BUFFER, 0); 

    // remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0); 

    glClearColor(0.2, 0.2, 0.2, 0.8);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        VN::nurb_surface_shader::instance().bind();

        test(cam.view_projection_matrix());

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
        std::cerr << "Socket listening failed. Error: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Waiting for a client_instance to connect ..." << std::endl;

    // accept connection from client_instance
    client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_size);
    if (client_socket == INVALID_SOCKET)
    {
        std::cerr << "Socket accepting failed. Error: " << WSAGetLastError() << std::endl;
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
            std::cout << "Message received: " << buffer << std::endl;
        }
    }

    // close socket
    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();
    return 0;
}