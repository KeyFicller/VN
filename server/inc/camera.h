#pragma once

#include "vector_matrix.h"
#include "events.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace VN
{

    class camera
    {
    public:
        camera(GLFWwindow* window, float v_fov, float aspect_ratio, float near_clip, float far_clip);
        ~camera();

    public:
        void on_event(event& e);
        void on_update(double ts);
        const Mat4f view_matrix() const;
        const Mat4f projection_matrix() const;
        const Mat4f view_projection_matrix() const;

    public:
        float distance() const;
        void set_distance(float dist);
        const Vec2f viewport_size() const;
        void set_viewport_size(const Vec2f& size);
        const Vec3f up_direction() const;
        const Vec3f right_direction() const;
        const Vec3f forward_direction() const;
        const Vec3f eye() const;
        const Vec3f lookAt() const;

    protected:
        void update_view_matrix();
        void update_projection_matrix();
        void update_camera_position();
        bool on_mouse_scrolled(mouse_scrolled_event& e);
        void on_mouse_paned(const Vec2f& delta);
        void on_mouse_rotated(const Vec2f& delta);
        void on_mouse_zoomed(float delta);
        const std::pair<float, float> pan_speed() const;
        float rotation_speed() const;
        float zoom_speed() const;
        const glm::quat orientation() const;

    protected:
        GLFWwindow* m_window = nullptr;
        float m_vertical_fov = 45.f;
        float m_aspect_ratio = 1.5f;
        float m_near_clip = 0.1f;
        float m_far_clip = 1000.f;
        float m_distance = 10.f;
        Vec2f m_viewport_size = {1200.f, 800.f};
        glm::mat4 m_view_matrix = glm::mat4(1.0f);
        glm::mat4 m_projection_matrix = glm::mat4(1.0f);
        glm::vec3 m_eye = {0.0f, 0.0f, 10.f};
        glm::vec3 m_look_at = {0.0f, 0.0f, 10.f};
        float m_pitch = 0.0f;
        float m_yaw = 0.0f;
        float m_roll = 0.0f;
        Vec2f m_mouse_position_before = {0.f, 0.f};
    };
}