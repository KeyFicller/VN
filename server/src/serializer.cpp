#include "serializer.h"

namespace VN
{
    namespace Implement
    {

        class seralize_file
        {
        public:
            seralize_file(const std::string& file_path)
                : m_file_path(file_path) {}
            virtual ~seralize_file() = default;


        protected:
            std::string m_file_path;
            bool m_ended = false;
        };

        class seralize_in_file : public seralize_file
        {
        public:
            seralize_in_file(const std::string& file_path);
            ~seralize_in_file() override;

        public:
            template <typename T>
            void in(const std::string& key, T& value)
            {
                YAML::Node& top_node = m_node_stack.top();
                value = top_node[key].as<T>();
            }

            bool contains(const std::string& key)
            {
                YAML::Node& top_node = m_node_stack.top();
                return top_node[key].IsDefined();
            }

            bool scopes_in(const std::string& key)
            {
                if (!contains(key))
                    return false;

                m_node_stack.push(m_node_stack.top()[key]);
                return true;
            }

            void scopes_out()
            {
                if (!m_node_stack.empty())
                    m_node_stack.pop();
            }

        protected:
            std::stack<YAML::Node> m_node_stack;
        };

        class seralize_out_file : public seralize_file
        {
        public:
            seralize_out_file(const std::string& file_paht);
            ~seralize_out_file() override;

        public:
            template <typename T>
            void out(const std::string& key, const T& value)
            {
                m_emitter << YAML::Key << key << YAML::Value << value;
            }

            void scopes_in()
            {
                m_emitter << YAML::BeginMap;
            }

            void scopes_out()
            {
                m_emitter << YAML::EndMap;
            }

        protected:
            YAML::Emitter m_emitter;
        };

        seralize_in_file::seralize_in_file(const std::string& file_path)
            : seralize_file(file_path)
        {
            std::ifstream fin(file_path);
            std::stringstream ss;
            ss << fin.rdbuf();
            fin.close();

            m_node_stack.push(YAML::Load(ss.str()));
        }

        seralize_in_file::~seralize_in_file()
        {
            scopes_out();
        }



        seralize_out_file::seralize_out_file(const std::string& file_path)
            : seralize_file(file_path)
        {
            scopes_in();
        }

        seralize_out_file::~seralize_out_file()
        {
            scopes_out();

            std::ofstream fout(m_file_path);
            fout << m_emitter.c_str();
            fout.close();
        }
    }


}