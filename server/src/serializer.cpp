#include "serializer.h"

#include "visual_nurb.h"
#include "yaml-cpp/yaml.h"

namespace YAML
{
    template <>
    struct convert<::VN::VsLim1>
    {
        static Node encode(const ::VN::VsLim1& rhs)
        {
            Node node;
            node.push_back(rhs.min);
            node.push_back(rhs.max);

            return node;
        }

        static bool decode(const Node& node, ::VN::VsLim1& rhs)
        {
            if (!node.IsSequence() || node.size() != 2)
                return false;

            rhs.min = node[0].as<double>();
            rhs.max = node[1].as<double>();

            return true;
        }
    };

    template <>
    struct convert<::VN::VsLim3>
    {
        static Node encode(const ::VN::VsLim3& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);

            return node;
        }

        static bool decode(const Node& node, ::VN::VsLim3& rhs)
        {
            if (!node.IsSequence() || node.size() != 3)
                return false;

            rhs.x = node[0].as<::VN::VsLim1>();
            rhs.y = node[1].as<::VN::VsLim1>();
            rhs.z = node[2].as<::VN::VsLim1>();

            return true;
        }
    };

    template <>
    struct convert<::VN::VsParmDat>
    {
        static Node encode(const ::VN::VsParmDat& rhs)
        {
            Node node;
            node.push_back(rhs.closed);
            node.push_back(rhs.degree);
            node.push_back(rhs.num_kt);
            node.push_back(Node(rhs.bnd));
            for (int i = 0; i < rhs.num_kt; i++)
                node.push_back(rhs.knots[i]);

            return node;
        }

        static bool decode(const Node& node, ::VN::VsParmDat& rhs)
        {
            if (!(node.size() >= 4))
                return false;

            rhs.closed = node[0].as<int>();
            rhs.degree = node[1].as<int>();
            rhs.num_kt = node[2].as<int>();
            rhs.bnd = node[3].as<::VN::VsLim1>();

            rhs.knots.resize(rhs.num_kt);
            for (int i = 0; i < rhs.num_kt; i++)
                rhs.knots[i] = node[4 + i].as<double>();

            return true;
        }
    };

    template <>
    struct convert<::VN::VsCtrlPointData>
    {
        static Node encode(const ::VN::VsCtrlPointData& rhs)
        {
            Node node;
            node.push_back(rhs.rat);
            node.push_back(rhs.dim);
            node.push_back(rhs.plane);
            node.push_back(rhs.num_cp);
            node.push_back(rhs.box);

            for (int i = 0; i < rhs.dim * rhs.num_cp; i++)
                node.push_back(rhs.list[i]);

            return node;
        }

        static bool decode(const Node& node, ::VN::VsCtrlPointData& rhs)
        {
            if (!(node.size() >= 5))
                return false;

            rhs.rat = node[0].as<int>();
            rhs.dim = node[1].as<int>();
            rhs.plane = node[2].as<int>();
            rhs.num_cp = node[3].as<int>();
            rhs.box = node[4].as<::VN::VsLim3>();

            rhs.list.resize(rhs.dim * rhs.num_cp);
            for (int i = 0; i < rhs.dim * rhs.num_cp; i++)
                rhs.list[i] = node[5 + i].as<double>();

            return true;
        }

    };
}

#define IN_DEFAULT(OBJ, ELEMENT)          sf.in(#ELEMENT, OBJ.ELEMENT)
#define OUT_DEFAULT(OBJ, ELEMENT)         sf.out(#ELEMENT, OBJ.ELEMENT)

namespace VN
{
    YAML::Emitter& operator << (YAML::Emitter& emitter, const VsParmDat& value)
    {
        emitter << YAML::Flow;
        emitter << YAML::BeginSeq;
        emitter << value.closed;
        emitter << value.degree;
        emitter << value.num_kt;
        emitter << YAML::Node(value.bnd);
        for (int i = 0; i < value.knots.size(); i++)
            emitter << value.knots[i];
        emitter << YAML::EndSeq;

        return emitter;
    }

    YAML::Emitter& operator << (YAML::Emitter& emitter, const VsCtrlPointData& value)
    {
        emitter << YAML::Flow;
        emitter << YAML::BeginSeq;
        emitter << value.rat;
        emitter << value.dim;
        emitter << value.plane;
        emitter << value.num_cp;
        emitter << YAML::Node(value.box);
        for (int i = 0; i < value.list.size(); i++)
            emitter << value.list[i];
        emitter << YAML::EndSeq;

        return emitter;
    }

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
            friend class yaml_serializer;
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
            friend class yaml_serializer;
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


    void yaml_serializer::dump(const std::string& file_path, const VsNurbCurv& crv)
    {
        Implement::seralize_out_file sf(file_path);

        OUT_DEFAULT(crv, type);
        OUT_DEFAULT(crv, mem);

        OUT_DEFAULT(crv, t);
        OUT_DEFAULT(crv, cp);
    }

    void yaml_serializer::load(const std::string& file_path, VsNurbCurv& crv)
    {
        Implement::seralize_in_file sf(file_path);

        IN_DEFAULT(crv, type);
        IN_DEFAULT(crv, mem);

        IN_DEFAULT(crv, t);
        IN_DEFAULT(crv, cp);
    }

    void yaml_serializer::dump(const std::string& file_path, const VsNurbSurf& srf)
    {
        Implement::seralize_out_file sf(file_path);

        OUT_DEFAULT(srf, type);
        OUT_DEFAULT(srf, mem);
        OUT_DEFAULT(srf, out_norm);
        OUT_DEFAULT(srf, offset);

        OUT_DEFAULT(srf, u);
        OUT_DEFAULT(srf, v);
        OUT_DEFAULT(srf, cp);

        OUT_DEFAULT(srf, num_loop);

        sf.m_emitter << YAML::Key << "profiles" << YAML::Value << YAML::BeginSeq;
        for (int i = 0; i < srf.num_loop; i++)
        {
            OUT_DEFAULT(srf.list_loop[i], num_cv);
            OUT_DEFAULT(srf.list_loop[i], next);
            OUT_DEFAULT(srf.list_loop[i], in);

            sf.m_emitter << YAML::Key << "curves" << YAML::Value << YAML::BeginSeq;

            for (int j = 0; j < srf.list_loop[i].list_cv.size(); j++)
            {
                OUT_DEFAULT(srf.list_loop[i].list_cv[j], type);
                OUT_DEFAULT(srf.list_loop[i].list_cv[j], mem);
                OUT_DEFAULT(srf.list_loop[i].list_cv[j], t);
                OUT_DEFAULT(srf.list_loop[i].list_cv[j], cp);

            }

            sf.m_emitter << YAML::EndSeq;
        }

        sf.m_emitter << YAML::EndSeq;
    }

    void yaml_serializer::load(const std::string& file_path, VsNurbSurf& srf)
    {
        Implement::seralize_in_file sf(file_path);

        IN_DEFAULT(srf, type);
        IN_DEFAULT(srf, mem);
        IN_DEFAULT(srf, out_norm);
        IN_DEFAULT(srf, offset);

        IN_DEFAULT(srf, u);
        IN_DEFAULT(srf, v);
        IN_DEFAULT(srf, cp);

        IN_DEFAULT(srf, num_loop);

        sf.scopes_in("profiles");
        srf.list_loop.resize(srf.num_loop);
        int i = 0;
        for (auto node_loop : sf.m_node_stack.top())
        {
            IN_DEFAULT(srf.list_loop[i], num_cv);
            IN_DEFAULT(srf.list_loop[i], next);
            IN_DEFAULT(srf.list_loop[i], in);

            sf.scopes_in("curves");
            srf.list_loop[i].list_cv.resize(srf.list_loop[i].num_cv);
            int j = 0;
            for (auto node_curve : sf.m_node_stack.top())
            {
                IN_DEFAULT(srf.list_loop[i].list_cv[j], type);
                IN_DEFAULT(srf.list_loop[i].list_cv[j], mem);
                IN_DEFAULT(srf.list_loop[i].list_cv[j], t);
                IN_DEFAULT(srf.list_loop[i].list_cv[j], cp);
                j++;
            }
            sf.scopes_out();
            i++;
        }
        sf.scopes_out();
    }


}