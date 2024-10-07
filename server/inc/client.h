#pragma once

#include "camera.h"
#include "shader.h"
#include "mesher.h"
#include "vector_matrix.h"

#include "GLFW/glfw3.h"
#include "GL/GLU.h"

#include <iostream>
#include <chrono>
#include <WinSock2.h>

#include "connections.h"

#pragma comment(lib, "ws2_32.lib")

namespace VN
{
    class VsNurbSurf;
    class VsNurbCurv;

    class vn_client_instance : public client_instance
    {
    public:
        static vn_client_instance& instance()
        {
            static vn_client_instance s_instance;
            return s_instance;
        }

        ~vn_client_instance() override;

        void init();

        void exec();

    private:
        int init_plot_window_and_camera();
        int init_nurb_renderer();
        int init_gui();

        void render_gui();

        int draw_nurbs_surf();
        int draw_nurbs_curve(VsNurbCurv* crv);

        void handle_message(const char* buffer, int nbytes) override;

    private:
        GLFWwindow* m_window = nullptr;
        GLUnurbsObj* m_nurbs = nullptr;
        camera* m_cam = nullptr;

        // TODO: warp with a mutex.
        VsNurbSurf* m_srf = nullptr;
        VsNurbCurv* m_crv = nullptr;
    };
}