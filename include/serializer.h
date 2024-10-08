#pragma once

#include <string>
#include <vector>
#include <stack>
#include <fstream>
#ifdef VN_PROJECT
#include <yaml-cpp/yaml.h>
#endif

namespace VN
{
    class seralize_stream
    {
    public:
        seralize_stream() : m_index(0), m_last_index(0)
        {
            m_data.resize(10);
        }

        seralize_stream(char* data, int bytes)
            : m_index(0), m_last_index(bytes)
        {
            m_data.resize(bytes);
            memcpy_s(m_data.data(), bytes, data, bytes);
        }

    public:
        void write_bytes(const char* pValue, int bytes)
        {
            size_t capacity = m_index + static_cast<size_t>(bytes);
            while (capacity > m_data.size()) {
                auto& backUp = m_data;
                m_data.resize(2 * capacity);
                memcpy_s(m_data.data(), backUp.size(), backUp.data(), backUp.size());
            }
            memcpy_s(m_data.data() + m_index, bytes, pValue, bytes);
            m_index += bytes;
            m_last_index = (m_last_index > m_index) ? m_last_index : m_index;
        }

        void read_bytes(char* pValue, int bytes)
        {
            if (!(m_index + bytes <= m_last_index))
                return;
            memcpy_s(pValue, bytes, m_data.data() + m_index, bytes);
            m_index += bytes;
        }

        const std::vector<char>& data() const
        {
            return m_data;
        }

        template <typename T>
        void write(const T& value)
        {
            write_bytes((const char*)&value, sizeof(T));
        }

        template <typename T>
        void read(T& value)
        {
            read_bytes((char*)&value, sizeof(T));
        }

        void dump_file(const std::string& file_path)
        {
            std::ofstream fout(file_path);
            fout << (const char*)m_data.data();
        }

        void load_file(const std::string& file_path)
        {
            std::ifstream fin(file_path);
            std::stringstream ss;
            ss << fin.rdbuf();

            std::string str = ss.str();
            m_data.clear();
            m_index = 0;
            m_last_index = 0;

            write_bytes(&str[0], str.size());
        }

    private:
        std::vector<char> m_data;
        size_t m_index = 0;
        size_t m_last_index = 0;
    };
}