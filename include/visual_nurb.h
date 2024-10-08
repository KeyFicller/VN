#pragma once

#include "serializer.h"

#include <iostream>
#include <winsock2.h>
#include <WS2tcpip.h>
/*
 * @brief Port for socket
 */
#define VN_PORT    54000

#define VN_FLAG_CURVE            ((int)0)
#define VN_FLAG_SURFACE          ((int)1)

#define VN_FLAG_DISCONNECT       ((int)6)

/*
 * @breif Macro
 */
#define VN_PLOT_CURVE(PTR, ...)      ::VN::client_instance::instance().plot_nurb_curve(PTR, ::VN::plot_options{##__VA_ARGS__})
#define VN_PLOT_SURFACE(PTR, ...)    ::VN::client_instance::instance().plot_nurb_surface(PTR, ::VN::plot_options{##__VA_ARGS__})


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
        double* knots = nullptr;

        ~VsParmDat() { delete[] knots; }
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
        double* list = nullptr;

        ~VsCtrlPointData() { delete[] list; }
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
        // TODO: need to deal with index form `in`.
        int num_cv;
        int next;
        int in;
        VsNurbCurv* list_cv = nullptr;

        ~VsProfile() { delete[] list_cv; }
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
        VsProfile* list_loop = nullptr;

        ~VsNurbSurf() { delete[] list_loop; }
    };

#else
    //// forward declare
    //struct VsNurbCurv;
    //struct VsNurbSurf;
#endif

    template <>
    inline void seralize_stream::write(const VsParmDat& value)
    {
        write(value.closed);
        write(value.degree);
        write(value.num_kt);
        write(value.bnd);

        for (int i = 0; i < value.num_kt; i++)
        {
            write(value.knots[i]);
        }
    }

    template <>
    inline void seralize_stream::read(VsParmDat& value)
    {
        read(value.closed);
        read(value.degree);
        read(value.num_kt);
        read(value.bnd);

        if (value.knots)
            delete[] value.knots;
        value.knots = new double[value.num_kt];

        for (int i = 0; i < value.num_kt; i++)
        {
            read(value.knots[i]);
        }
    }

    template <>
    inline void seralize_stream::write(const VsCtrlPointData& value)
    {
        write(value.rat);
        write(value.dim);
        write(value.plane);
        write(value.num_cp);
        write(value.box);

        for (int i = 0; i < value.dim * value.num_cp; i++)
        {
            write(value.list[i]);
        }
    }

    template <>
    inline void seralize_stream::read(VsCtrlPointData& value)
    {
        read(value.rat);
        read(value.dim);
        read(value.plane);
        read(value.num_cp);
        read(value.box);

        if (value.list)
            delete[] value.list;
        value.list = new double[value.dim * value.num_cp];

        for (int i = 0; i < value.dim * value.num_cp; i++)
        {
            read(value.list[i]);
        }
    }

    template <>
    inline void seralize_stream::write(const VsNurbCurv& value)
    {
        write(value.type);
        write(value.mem);
        write(value.t);
        write(value.cp);
    }

    template <>
    inline void seralize_stream::read(VsNurbCurv& value)
    {
        read(value.type);
        read(value.mem);
        read(value.t);
        read(value.cp);
    }

    template <>
    inline void seralize_stream::write(const VsProfile& value)
    {
        write(value.num_cv);
        write(value.next);
        write(value.in);

        for (int i = 0; i < value.num_cv; i++)
        {
            write(value.list_cv[i]);
        }
    }

    template <>
    inline void seralize_stream::read(VsProfile& value)
    {
        read(value.num_cv);
        read(value.next);
        read(value.in);

        if (value.list_cv)
            delete[] value.list_cv;
        value.list_cv = new VsNurbCurv[value.num_cv];

        for (int i = 0; i < value.num_cv; i++)
        {
            read(value.list_cv[i]);
        }
    }

    template <>
    inline void seralize_stream::write(const VsNurbSurf& value)
    {
        write(value.type);
        write(value.mem);
        write(value.out_norm);
        write(value.offset);
        write(value.u);
        write(value.v);
        write(value.cp);
        write(value.num_loop);

        for (int i = 0; i < value.num_loop; i++)
        {
            write(value.list_loop[i]);
        }
    }

    template <>
    inline void seralize_stream::read(VsNurbSurf& value)
    {
        read(value.type);
        read(value.mem);
        read(value.out_norm);
        read(value.offset);
        read(value.u);
        read(value.v);
        read(value.cp);
        read(value.num_loop);

        if (value.list_loop)
            delete[] value.list_loop;
        value.list_loop = new VsProfile[value.num_loop];

        for (int i = 0; i < value.num_loop; i++)
        {
            read(value.list_loop[i]);
        }
    }

    struct plot_options
    {
        unsigned int color = 0xFFFFFF;
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

            //// set unlock
            //static unsigned long mode = 1;
            //if (ioctlsocket(m_client_socket, FIONBIO, &mode) != 0)
            //{
            //    closesocket(m_client_socket);
            //    return;
            //}

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
            seralize_stream ss;
            ss.write(VN_FLAG_DISCONNECT);
            send(m_client_socket, ss.data().data(), ss.data().size(), 0);

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

            seralize_stream ss;
            ss.write(VN_FLAG_CURVE);
            ss.write(*p_nurb_crv);

            send(m_client_socket, ss.data().data(), ss.data().size(), 0);
        }

        void plot_nurb_surface(void* ptr, const plot_options& opt)
        {
            VsNurbSurf* p_nurb_srf = reinterpret_cast<VsNurbSurf*>(ptr);
            if (!p_nurb_srf)
            {
                return;
            }

            seralize_stream ss;
            ss.write(VN_FLAG_SURFACE);
            ss.write(*p_nurb_srf);

            send(m_client_socket, ss.data().data(), ss.data().size(), 0);
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

            // drag here to start, modify global variables by your self.
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