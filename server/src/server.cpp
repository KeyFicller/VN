#include "server.h"

#include "visual_nurb.h"

namespace VN
{
    namespace
    {
        double delta_time()
        {
            static bool counting_started = false;
            static long long last_tick = 0;

            auto now = std::chrono::high_resolution_clock::now();
            long long cur_tick = std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
            double ts((cur_tick - last_tick) / 1000.0);
            last_tick = cur_tick;
            if (!counting_started)
            {
                counting_started = true;
                return 0.0;
            }

            return ts;
        }
    }

    server_instance::~server_instance()
    {
        gluDeleteNurbsRenderer(m_nurbs);
        delete m_cam;
        glfwTerminate();

        // close socket
        closesocket(client_socket);
        closesocket(server_socket);
        WSACleanup();
    }

    void server_instance::init()
    {
        init_plot_window_and_camera();
        init_socket();
        init_nurb_renderer();
    }

    void server_instance::exec()
    {
        glClearColor(0.2, 0.2, 0.2, 0.8);

        /* Loop until the user closes the window */
        while (!glfwWindowShouldClose(m_window))
        {
            on_message_reviced();

            glClear(GL_COLOR_BUFFER_BIT);

            m_cam->on_update(delta_time());

            // VN::nurb_surface_shader::instance().bind();
            draw_nurbs();

            /* Swap front and back buffers */
            glfwSwapBuffers(m_window);

            /* Poll for and process events */
            glfwPollEvents();
        }
    }

    int server_instance::init_plot_window_and_camera()
    {
        /* Initialize the library */
        if (!glfwInit())
            return -1;

        /* Create a windowed mode window and its OpenGL context */
        m_window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
        if (!m_window)
        {
            glfwTerminate();
            return -1;
        }

        /* Make the window's context current */
        glfwMakeContextCurrent(m_window);

        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            return -1;
        }

        /* Initial camera controller */
        m_cam = new camera(m_window, 45.f, 1.5f, 0.1f, 1000.f);

        glfwSetWindowUserPointer(m_window, m_cam);

        // add event callbacks
        glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xOffset, double yOffset) {
            ::VN::mouse_scrolled_event event(static_cast<float>(xOffset), static_cast<float>(yOffset));
            ((::VN::camera*)glfwGetWindowUserPointer(window))->on_event(event);
        });

        glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
            glViewport(0, 0, width, height);
        });

        return 0;
    }

    int server_instance::init_nurb_renderer()
    {
        m_nurbs = gluNewNurbsRenderer();
        gluNurbsProperty(m_nurbs, GLU_SAMPLING_TOLERANCE, 25.0);
        gluNurbsProperty(m_nurbs, GLU_DISPLAY_MODE, GLU_FILL);

        return 0;
    }

    int server_instance::init_socket()
    {
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

        return 0;
    }

    int server_instance::on_message_reviced()
    {
        char buffer[1024];
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received > 0)
        {
            seralize_stream ss(buffer, bytes_received);

            //int type = VN_FLAG_SURFACE;
            //ss.read(type);
            //if (type == VN_FLAG_CURVE)
            //{
            //    // TODO:
            //}
            //if (type == VN_FLAG_SURFACE)
            //{
            //    VsNurbSurf nurb;
            //    ss.read(nurb);
            //}
        }

        return 0;
    }

    int server_instance::draw_nurbs()
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(m_cam->m_vertical_fov, m_cam->m_aspect_ratio, m_cam->m_near_clip, m_cam->m_far_clip);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(m_cam->eye().x(), m_cam->eye().y(), m_cam->eye().z(), m_cam->look_at().x(), m_cam->look_at().y(), m_cam->look_at().z(),
            m_cam->up_direction().x(), m_cam->up_direction().y(), m_cam->up_direction().z());

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

        gluNurbsProperty(m_nurbs, GLU_SAMPLING_TOLERANCE, 25); //设置属性
        gluNurbsProperty(m_nurbs, GLU_DISPLAY_MODE, GLU_OUTLINE_POLYGON);
        gluBeginSurface(m_nurbs);//开始绘制
        gluNurbsSurface(m_nurbs,
            8, knots,
            8, knots,
            4 * 3,
            3,
            &ctrlpoints[0][0][0],
            4, 4,
            GL_MAP2_VERTEX_3);

        gluEndSurface(m_nurbs); //结束绘制

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

        return 0;
    }

}