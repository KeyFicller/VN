#pragma once

#include "glad/glad.h"
#include "GLFW/glfw3.h"

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

    protected:
        unsigned int m_renderer_id = -1;
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