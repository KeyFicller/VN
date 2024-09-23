#pragma once

#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>
/*
 * @brief Port for socket
 */
#define VN_PORT    54000

/*
 * @breif Macro
 */
#define VN_PLOT_CURVE(PTR, ...)      ::VN::client_instance::instace().plot_nurb_curve(PTR, plot_options{##__VA_ARGS__})
#define VN_PLOT_SURFACE(PTR, ...)    ::VN::client_instance::instace().plot_nurb_surface(PTR, plot_options{##__VA_ARGS__})

/**
 * Overall:
 * 1. Integrate method: include "visual_nurb.h" by "xxx.cpp" in target project.
 */
namespace VN
{
#ifdef VN_PROJECT
    // only use for development
    struct VsLim1
    {
        double min;
        double max;
    };

    struct VsParmDat
    {
        int closed;
        int degree;
        int num_kt;
        VsLim1 bnd;
        double* knots;
    };

    struct VsLim3
    {
        VsLim1 x;
        VsLim1 y;
        VsLim1 z;
    };

    struct VsCtrlPointData
    {
        int rat;
        int dim;
        int plane;
        int num_cp;
        VsLim3 box;
        double* list;
    };

    struct VsNurbCurv
    {
        int type;
        int mem;

        VsParmDat t;
        VsCtrlPointData cp;
    };

    struct VsProfile
    {
        int num_cv;
        int next;
        int in;
        VsNurbCurv* list_cv;
    };

    struct VsNurbSurf
    {
        int type;
        int mem;
        int out_norm;
        double offset;

        VsParmDat u;
        VsParmDat v;
        VsCtrlPointData cp;

        int num_loop;
        VsProfile* list_loop;
    };

#else
    // forward declare
    struct VsNurbCurv;
    struct VsNurbSurf;
#endif

    struct plot_options
    {
        unsigned int color = 0xFFFFFF;
    };

    struct nurb_curve_data
    {
        
    };

    struct nurb_surface_data
    {
        
    };

    class client_instance
    {
    public:
        static client_instance& instance()
        {
            static client_instance instance;
            return instance;
        }

    protected:
        client_instance()
        {
            const char* server_ip = "127.0.0.1"; // local_server

            // initialize win sock
            if (WSAStartup(MAKEWORD(2, 2), &m_wsa_data) != 0)
            {
                std::cerr << "WSAStartup failed. Error: " << WSAGetLastError() << std::endl;
                return;
            }

            // create socket
            m_client_socket = socket(AF_INET, SOCK_STREAM, 0);
            if (m_client_socket == INVALID_SOCKET)
            {
                std::cerr << "Socket creation failed. Error: " << WSAGetLastError() << std::endl;
                WSACleanup();
                return;
            }

            // set server address
            m_server_addr.sin_family = AF_INET;
            m_server_addr.sin_port = htons(VN_PORT);
            inet_pton(AF_INET, server_ip, &m_server_addr.sin_addr);

            // bind socket
            if (connect(m_client_socket, (sockaddr*)&m_server_addr, sizeof(m_server_addr)) == SOCKET_ERROR)
            {
                std::cerr << "Connection failed. Error: " << WSAGetLastError() << std::endl;
                closesocket(m_client_socket);
                WSACleanup();
                return;
            }
        }

        ~client_instance()
        {
            closesocket(m_client_socket);
            WSACleanup();
        }

    public:
        void seed()
        {
            const char* message = "Hello from Client!";
            send(m_client_socket, message, strlen(message), 0);
            std::cout << "Message sent: " << message << std::endl;
        }

        void plot_nurb_curve(void* ptr, const plot_options& opt)
        {
            VsNurbCurv* p_nurb_crv = reinterpret_cast<VsNurbCurv*>(ptr);
            if (!p_nurb_crv)
            {
                return;
            }


        }

        void plot_nurb_surface(void* ptr, const plot_options& opt)
        {

        }

        static void plot_nurb_curve_debugging()
        {
            static void* s_ptr = nullptr;
            static plot_options s_opt;

            // drag here to start
            (void)0;

            instance().plot_nurb_curve(s_ptr, s_opt);

            s_ptr = nullptr;

            // restore program processing
            (void)0;

            __debugbreak();
            while (true);
        }

        static void plot_nurb_surface_debugging()
        {
            static void* s_ptr = nullptr;
            static plot_options s_opt;

            // drag here to start, modify gobal variables by your self.
            (void)0;

            instance().plot_nurb_surface(s_ptr, s_opt);

            s_ptr = nullptr;

            // restore program processing
            (void)0;

            __debugbreak();
            while (true);
        }

    private:
        WSADATA m_wsa_data;
        SOCKET m_client_socket;
        sockaddr_in m_server_addr;
    };
}
