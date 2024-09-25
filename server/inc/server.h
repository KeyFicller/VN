#pragma once

#include "camera.h"
#include "shader.h"
#include "vector_matrix.h"

#include "GLFW/glfw3.h"
#include "GL/GLU.h"

#include <iostream>
#include <chrono>
#include <WinSock2.h>

#pragma comment(lib, "ws2_32.lib")

namespace VN
{
    class VsNurbSurf;
    class VsNurbCurv;

    class server_instance
    {
    public:
        static server_instance& instance()
        {
            static server_instance s_instance;
            return s_instance;
        }

        ~server_instance();

        void init();

        void exec();

    private:
        int init_plot_window_and_camera();
        int init_nurb_renderer();
        int init_socket();

        int terminate_socket();

        int on_message_reviced();

        int draw_nurbs_surf();
        int draw_nurbs_curve(VsNurbCurv* crv);

    private:
        GLFWwindow* m_window = nullptr;
        GLUnurbsObj* m_nurbs = nullptr;
        camera* m_cam = nullptr;

        WSADATA wsa_data;
        SOCKET server_socket, client_socket;
        sockaddr_in server_addr, client_addr;

        VsNurbSurf* m_srf = nullptr;
        VsNurbCurv* m_crv = nullptr;

        bool m_need_reconnect = false;
    };
}