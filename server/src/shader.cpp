#include "shader.h"

namespace VN
{
    const char* vertex_shader_source = R"(
        #version 440 core
        layout (location = 0) in vec3 aPosition;
        
        layout (std140, binding = 0) uniform Camera
        {
            mat4 u_ViewProjection;
        };

        void main()
        {
           gl_Position = u_ViewProjection * vec4(aPosition.x, aPosition.y, aPosition.z, 1.0f);
        }
    )";

    const char* fragment_shader_source = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\n\0";

    shader::~shader()
    {
        glDeleteProgram(m_renderer_id);
    }

    void shader::bind()
    {
        glUseProgram(m_renderer_id);
    }

    void shader::unbind()
    {
        glUseProgram(0);
    }

    void shader::compile(const std::unordered_map<GLenum, std::string>& shader_srcs)
    {
        GLuint program = glCreateProgram();
        std::vector<GLenum> glShaderIds;
        glShaderIds.reserve(shader_srcs.size());
        for (const auto& shaderSrc : shader_srcs) {
            GLenum type = shaderSrc.first;
            const std::string& src = shaderSrc.second;
            GLuint shader = glCreateShader(type);
            const GLchar* srcCStyle = src.c_str();
            glShaderSource(shader, 1, &srcCStyle, 0);
            glCompileShader(shader);
            GLint isCompiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);

            if (isCompiled == GL_FALSE) {
                GLint logLength = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
                std::vector<GLchar> log(logLength);
                glGetShaderInfoLog(shader, logLength, &logLength, log.data());
                glDeleteProgram(shader);

                // ASSERT(false, "Shader source compile failed: %s.", log.data());
                __debugbreak();
                break;
            }

            glAttachShader(program, shader);
            glShaderIds.push_back(shader);
        }

        glLinkProgram(program);
        GLint isLinked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
        if (isLinked == GL_FALSE) {
            GLint logLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<GLchar> log(logLength);
            glGetProgramInfoLog(program, logLength, &logLength, log.data());
            glDeleteProgram(program);
            for (auto id : glShaderIds) {
                glDeleteShader(id);
            }

            // ASSERT(false, "Shader program link failed: %s.", log.data());
            __debugbreak();
            return;
        }

        for (auto id : glShaderIds) {
            glDetachShader(program, id);
            glDeleteShader(id);
        }

        m_renderer_id = program;
    }

    GLint shader::location(const std::string& key)
    {
        if (m_location_map.find(key) == m_location_map.end())
        {
            GLint location = glGetUniformLocation(m_renderer_id, key.c_str());
            //ASSERT(location != -1, "Key '%s' not found.", key.c_str());
            m_location_map[key] = location;
        }
        return m_location_map[key];
    }

    void shader::set_matrix4(const std::string& key, const Mat4f& value)
    {
        glUniformMatrix4fv(location(key), 1, GL_FALSE, &(value.m_Data[0].m_Data[0]));
    }

    nurb_curve_shader& nurb_curve_shader::instance()
    {
        static std::unordered_map<GLenum, std::string> s_shader_srcs =
        {
            {GL_VERTEX_SHADER, R"(...)"},
            {GL_FRAGMENT_SHADER,  R"(...)"}
        };

        static nurb_curve_shader s_shader;
        if (s_shader.m_renderer_id <= 0)
        {
            s_shader.compile(s_shader_srcs);
        }

        return s_shader;
    }

    nurb_surface_shader& nurb_surface_shader::instance()
    {
        static std::unordered_map<GLenum, std::string> s_shader_srcs =
        {
            {GL_VERTEX_SHADER, vertex_shader_source},
            {GL_FRAGMENT_SHADER, fragment_shader_source}
        };

        static nurb_surface_shader s_shader;
        if (s_shader.m_renderer_id <= 0)
        {
            s_shader.compile(s_shader_srcs);
        }

        return s_shader;
    }

}