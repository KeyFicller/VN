#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "vector_matrix.h"

#include <unordered_map>

namespace VN
{
    class shader
    {
    protected:
        virtual ~shader();

    public:
        void bind();
        void unbind();

    protected:
        void compile(const std::unordered_map<GLenum, std::string>& shader_srcs);

        void set_matrix4(const std::string& key, const Mat4f& value);
        GLint location(const std::string& key);

    protected:
        unsigned int m_renderer_id = -1;
        std::unordered_map<std::string, GLint> m_location_map;
    };

    class nurb_curve_shader : public shader
    {
    public:
        static nurb_curve_shader& instance();
    };

    class nurb_surface_shader : public shader
    {
    public:
        static nurb_surface_shader& instance();
    };
}