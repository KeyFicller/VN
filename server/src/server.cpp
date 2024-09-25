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
        delete m_srf;

        gluDeleteNurbsRenderer(m_nurbs);
        delete m_cam;
        glfwTerminate();

        terminate_socket();
    }

    void server_instance::init()
    {
        ::VN::VsNurbSurf srf;
        mesh_uv_coordiante result = mesher::uv_mesh(&srf);

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

            if (m_need_reconnect)
            {
                terminate_socket();
                init_socket();
                m_need_reconnect = false;
            }

            glClear(GL_COLOR_BUFFER_BIT);

            m_cam->on_update(delta_time());

            // VN::nurb_surface_shader::instance().bind();
            // draw_nurbs_example();
            if (m_srf)
                draw_nurbs_surf();

            if (m_crv)
                draw_nurbs_curve(m_crv);

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

        static unsigned long mode = 1;
        if (ioctlsocket(server_socket, FIONBIO, &mode) != 0)
        {
            closesocket(server_socket);
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
        }

        std::cout << "Waiting for a client_instance to connect ..." << std::endl;

        // accept connection from client_instance
        while (true)
        {

            client_socket = accept(server_socket, (sockaddr*)&client_addr, &client_addr_size);
            if (client_socket != INVALID_SOCKET)
            {
                break;
            }
        }

        return 0;
    }

    int server_instance::terminate_socket()
    {
        // close socket
        closesocket(client_socket);
        closesocket(server_socket);
        WSACleanup();

        return 0;
    }

    int server_instance::on_message_reviced()
    {
        char buffer[1024];
        int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
        if (bytes_received > 0)
        {
            seralize_stream ss(buffer, bytes_received);

            int type = -1;
            ss.read(type);
            if (type == VN_FLAG_CURVE)
            {
                delete m_crv;
                m_crv = new VsNurbCurv;
                ss.read(*m_crv);
            }
            if (type == VN_FLAG_SURFACE)
            {
                delete m_srf;
                m_srf = new VsNurbSurf;
                ss.read(*m_srf);
            }
            else if (type == VN_FLAG_DISCONNECT)
            {
                //TODO: temporary solution
                m_need_reconnect = true;
            }
        }
        else
        {
            std::cout << "hanging." << std::endl;
        }

        return 0;
    }

    int server_instance::draw_nurbs_surf()
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(m_cam->m_vertical_fov, m_cam->m_aspect_ratio, m_cam->m_near_clip, m_cam->m_far_clip);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(m_cam->eye().x(), m_cam->eye().y(), m_cam->eye().z(), m_cam->look_at().x(), m_cam->look_at().y(), m_cam->look_at().z(),
            m_cam->up_direction().x(), m_cam->up_direction().y(), m_cam->up_direction().z());

        std::vector<float> ctrl_points;
        int u_ctrl_num = 0;
        int v_ctrl_num = 0;
        std::vector<float> u_knots;
        int u_knot_num = 0;
        std::vector<float> v_knots;
        int v_knot_num = 0;
        int u_order = 0;
        int v_order = 0;
        int dim = 3;


        u_knot_num = m_srf->u.num_kt;
        for (int i = 0; i < m_srf->u.num_kt; i++)
            u_knots.emplace_back(m_srf->u.knots[i]);

        v_knot_num = m_srf->v.num_kt;
        for (int i = 0; i < m_srf->v.num_kt; i++)
            v_knots.emplace_back(m_srf->v.knots[i]);

        u_order = m_srf->u.degree + 1;
        v_order = m_srf->v.degree + 1;
        u_ctrl_num = u_knot_num - u_order;
        v_ctrl_num = v_knot_num - v_order;

        switch (m_srf->cp.dim)
        {
        case 2:
        {

            for (int i = 0; i < m_srf->cp.num_cp; i++)
            {
                for (int j = 0; j < 2; j++)
                    ctrl_points.emplace_back(m_srf->cp.list[i * 2 + j]);
                ctrl_points.emplace_back(0.0);
            }
            break;
        }
        case 3:
        {
            for (int i = 0; i < m_srf->cp.num_cp; i++)
            {
                for (int j = 0; j < 3; j++)
                    ctrl_points.emplace_back(m_srf->cp.list[i * 3 + j]);
            }
            break;
        }
        case 4:
        {
            for (int i = 0; i < m_srf->cp.num_cp; i++)
            {
                for (int j = 0; j < 3; j++)
                    ctrl_points.emplace_back(m_srf->cp.list[i * 4 + j] * m_srf->cp.list[i * 4 + 3]);
            }
            break;
        }
        }

        int u_stride = v_ctrl_num * dim;
        int v_stride = dim;

        glPushMatrix();
        //绘制控制点与控制线
        glScaled(0.2, 0.2, 0.2);

        glPointSize(4.0f);
        glColor3f(0.0, 0.0, 1.0);
        glColor3f(0, 0, 1);
        glBegin(GL_POINTS);
        for (int i = 0; i < u_ctrl_num; i++)
        {
            for (int j = 0; j < v_ctrl_num; j++)
                glVertex3fv(&ctrl_points[i * v_ctrl_num * dim + j * dim]);
        }
        glEnd();
        //绘制控制线
        glLineWidth(1.5f);
        glColor3f(0.0, 1.0, 1.0);
        for (int i = 0; i < u_ctrl_num; i++)
        {
            glBegin(GL_LINE_STRIP);
            for (int j = 0; j < v_ctrl_num; j++)
                glVertex3fv(&ctrl_points[i * v_ctrl_num * dim + j * dim]);
            glEnd();
        }
        for (int i = 0; i < v_ctrl_num; i++)
        {
            glBegin(GL_LINE_STRIP);
            for (int j = 0; j < u_ctrl_num; j++)
                glVertex3fv(&ctrl_points[j * v_ctrl_num * dim + i * dim]);
            glEnd();
        }

        gluNurbsProperty(m_nurbs, GLU_SAMPLING_TOLERANCE, 25); //设置属性
        gluNurbsProperty(m_nurbs, GLU_DISPLAY_MODE, GLU_OUTLINE_POLYGON);
        gluBeginSurface(m_nurbs);//开始绘制
        gluNurbsSurface(m_nurbs,
            u_knot_num, u_knots.data(),
            v_knot_num, v_knots.data(),
            u_stride,
            v_stride,
            ctrl_points.data(),
            u_order, v_order,
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

        for (int i = 0; i < m_srf->num_loop; i++)
        {
            for (int j = 0; j < m_srf->list_loop[i].num_cv; j++)
            {
                draw_nurbs_curve(&m_srf->list_loop[i].list_cv[j]);
            }
        }

        return 0;
    }

    int server_instance::draw_nurbs_curve(VsNurbCurv* crv)
    {
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        gluPerspective(m_cam->m_vertical_fov, m_cam->m_aspect_ratio, m_cam->m_near_clip, m_cam->m_far_clip);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        gluLookAt(m_cam->eye().x(), m_cam->eye().y(), m_cam->eye().z(), m_cam->look_at().x(), m_cam->look_at().y(), m_cam->look_at().z(),
            m_cam->up_direction().x(), m_cam->up_direction().y(), m_cam->up_direction().z());

        std::vector<float> ctrl_points;
        std::vector<float> knots;
        int knot_num = 0;
        int stride = 0;
        int order = 0;

        knot_num = crv->t.num_kt;
        for (int i = 0; i < crv->t.num_kt; i++)
            knots.emplace_back(crv->t.knots[i]);

        order = crv->t.degree + 1;
        stride = 3;

        switch (crv->cp.dim)
        {
        case 2:
            {
            for (int i = 0; i < crv->cp.num_cp; i++)
            {
                for (int j = 0; j < 2; j++)
                    ctrl_points.emplace_back(crv->cp.list[i * 2 + j]);
                ctrl_points.emplace_back(0.0);
            }
            break;
            }
        case 3:
        {
            for (int i = 0; i < crv->cp.num_cp; i++)
            {
                for (int j = 0; j < 3; j++)
                    ctrl_points.emplace_back(crv->cp.list[i * 3 + j]);
            }
            break;
        }
        case 4:
        {
            for (int i = 0; i < crv->cp.num_cp; i++)
            {
                for (int j = 0; j < 3; j++)
                    ctrl_points.emplace_back(crv->cp.list[i * 4 + j] * crv->cp.list[i * 4 + 3]);
            }
            break;
        }
        }

        glPushMatrix();
        //绘制控制点与控制线
        glScaled(0.2, 0.2, 0.2);

        glLineWidth(1.5f);
        glColor3f(1.0, 0.0, 0.0);

        gluNurbsProperty(m_nurbs, GLU_SAMPLING_TOLERANCE, 5); //设置属性
        gluNurbsProperty(m_nurbs, GLU_DISPLAY_MODE, GLU_OUTLINE_POLYGON);
        gluBeginCurve(m_nurbs);//开始绘制
        gluNurbsCurve(m_nurbs,
            knot_num,
            knots.data(),
            stride,
            ctrl_points.data(),
            order,
            GL_MAP1_VERTEX_3);

        gluEndCurve(m_nurbs); //结束绘制

        glPopMatrix();

        return 0;
    }

}