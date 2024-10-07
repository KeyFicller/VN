#pragma once

#include "core.h"
#include "serializer.h"

#include <winsock2.h>
#include <WS2tcpip.h>
#include <thread>
#include <atomic>
#include <functional>
#include <iostream>
#include <mutex>

#define VN_PORT    54000
#define MAXIMUM_MSG_BYTES     (1024 * 16)

#define VN_FLAG_CURVE            ((int)0)
#define VN_FLAG_SURFACE          ((int)1)

#define VN_FLAG_DISCONNECT       ((int)6)
#define VN_FLAG_HEART_BEAT       ((int)7)

// send a heart beat message per Xms
#define VN_HEART_BEAT_STRIDE_SEND    100

#define VN_HEART_BEAT_STRIDE_RECV    (VN_HEART_BEAT_STRIDE_SEND * 5)

namespace VN
{
    class client_server_base
    {
    public:
        static long long current_tick()
        {
            auto now = std::chrono::high_resolution_clock::now();
            return std::chrono::time_point_cast<std::chrono::milliseconds>(now).time_since_epoch().count();
        }

        static bool is_valid_socket(SOCKET sock)
        {
            return sock != INVALID_SOCKET && sock != 0;
        }

        client_server_base() = default;
        virtual ~client_server_base() = default;

    public:
        void main_loop()
        {
            vn_log("CONNECTION", "Start.");

            long long cur_tick = 0;

            while (true)
            {
                cur_tick = current_tick();

                if (m_last_send_tick != 0 && cur_tick - m_last_send_tick > VN_HEART_BEAT_STRIDE_SEND)
                {
                    if (is_valid_socket(m_target_socket))
                    {
                        seralize_stream ss;
                        ss.write(VN_FLAG_HEART_BEAT);
                        send(m_target_socket, ss.data().data(), ss.data().size(), 0);
                        m_last_send_tick = cur_tick;
                    }
                }

                if (m_last_recv_tick != 0 && cur_tick - m_last_recv_tick > VN_HEART_BEAT_STRIDE_RECV)
                {
                    vn_log("CONNECTION", "Heart beat failed.");
                    disconnect();
                }

                handle_connection_request();

                if (!is_valid_socket(m_target_socket))
                    continue;

                char buff[MAXIMUM_MSG_BYTES];
                int bytes_received = recv(m_target_socket, buff, MAXIMUM_MSG_BYTES, 0);
                if (bytes_received > 0)
                {
                    seralize_stream ss(buff, bytes_received);
                    int flag = -1;

                    ss.read(flag);
                    switch (flag)
                    {
                    case VN_FLAG_DISCONNECT:
                        {
                            disconnect();
                            break;
                        }
                    case VN_FLAG_HEART_BEAT:
                        {
                            m_last_recv_tick = cur_tick;
                            break;
                        }
                    default:
                        {
                            handle_message(buff, bytes_received);
                            break;
                        }
                    }

                }
            }
            vn_log("CONNECTION", "End.");
        }

        virtual void disconnect()
        {
            seralize_stream ss;
            ss.write(VN_FLAG_DISCONNECT);

            if (is_valid_socket(m_target_socket))
            {
                send(m_target_socket, ss.data().data(), ss.data().size(), 0);
            }

            closesocket(m_self_socket);
            closesocket(m_target_socket);
            WSACleanup();

            m_self_socket = 0;
            m_target_socket = 0;

            m_last_send_tick = 0;
            m_last_recv_tick = 0;

            set_connected(false);
        }

        virtual void handle_connection_request() = 0;
        virtual void handle_message(const char* buffer, int nbytes) = 0;
        virtual bool is_client() const = 0;

    protected:
        void set_connected(bool set = true)
        {
            if (m_connected != set)
            {
                vn_log("CONNECTION", (set ? "Connected" : "Disconnected"));
                m_connected = set;
            }
        }

        bool is_connected() const
        {
            return m_connected;
        }

    private:
        std::atomic_bool m_connected = false;
    protected:
        WSADATA m_wsa_data;
        SOCKET m_self_socket;
        SOCKET m_target_socket;
        long long m_last_send_tick = 0;
        long long m_last_recv_tick = 0;
        std::mutex m_mutex;
    };

    class client_instance : public  client_server_base
    {
    public:
        client_instance()
        {
            std::thread(std::bind(&client_server_base::main_loop, this)).detach();
        }

    protected:
        void handle_connection_request() override
        {
            if (is_connected() != m_connected_to)
            {
                if (!is_connected())
                {
                    const char* server_ip = "127.0.0.1"; // local_server
                    WSAStartup(MAKEWORD(2, 2), &m_wsa_data);
                    m_self_socket = socket(AF_INET, SOCK_STREAM, 0);

                    //static unsigned long mode = 1;
                    //ioctlsocket(m_self_socket, FIONBIO, &mode);

                    m_server_addr.sin_family = AF_INET;
                    m_server_addr.sin_port = htons(VN_PORT);
                    inet_pton(AF_INET, server_ip, &m_server_addr.sin_addr);

                    if (connect(m_self_socket, (sockaddr*)&m_server_addr, sizeof(m_server_addr)) != SOCKET_ERROR)
                    {
                        m_last_send_tick = current_tick();
                        m_last_recv_tick = current_tick();
                        set_connected(true);
                        m_target_socket = m_self_socket;
                    }
                    else
                    {
                        vn_log("CONNECTION", "Connect request failed.");
                        m_connected_to = false;
                    }
                }
                else
                {
                    disconnect();
                }
            }
        }

        void set_connected_to(bool value = true)
        {
            m_connected_to = value;
        }

        void disconnect() override
        {
            client_server_base::disconnect();
            m_connected_to = false;
        }

        void handle_message(const char* buffer, int nbytes) override {}
        bool is_client() const override { return true; }

    protected:
        sockaddr_in m_server_addr;
        bool m_connected_to = false;
    };

    class server_instance : public client_server_base
    {
    public:
        server_instance()
        {
            std::thread(std::bind(&client_server_base::main_loop, this)).detach();
        }

    protected:
        void handle_connection_request() override
        {
            if (!is_valid_socket(m_self_socket))
            {
                WSAStartup(MAKEWORD(2, 2), &m_wsa_data);
                m_self_socket = socket(AF_INET, SOCK_STREAM, 0);

                static unsigned long mode = 1;
                ioctlsocket(m_self_socket, FIONBIO, &mode);

                m_server_addr.sin_family = AF_INET;
                m_server_addr.sin_addr.s_addr = INADDR_ANY;
                m_server_addr.sin_port = htons(VN_PORT);

                bind(m_self_socket, (sockaddr*)&m_server_addr, sizeof(m_server_addr));
            }

            if (!is_connected())
            {
                if (listen(m_self_socket, SOMAXCONN) != SOCKET_ERROR)
                {
                    int client_addr_size = sizeof(m_client_addr);
                    m_target_socket = accept(m_self_socket, (sockaddr*)&m_client_addr, &client_addr_size);
                    if (is_valid_socket(m_target_socket))
                    {
                        m_last_send_tick = current_tick();
                        m_last_recv_tick = current_tick();
                        set_connected(true);
                    }
                }
            }
        }

        void handle_message(const char* buffer, int nbytes) override {}

        bool is_client() const override { return false; }

    protected:
        sockaddr_in m_server_addr;
        sockaddr_in m_client_addr;
    };
}
