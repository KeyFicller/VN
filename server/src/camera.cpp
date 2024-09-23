#include "camera.h"

#include "keycodes_and_mouse_buttons.h"
#include <glm/gtc/matrix_transform.hpp>


namespace VN
{
    namespace
    {
        bool is_key_pressed(GLFWwindow* window, int key_code)
        {
            auto state = glfwGetKey(window, key_code);
            return state == GLFW_PRESS || state == GLFW_REPEAT;
        }

        bool is_mouse_button_pressed(GLFWwindow* window, int mouse_button)
        {
            auto state = glfwGetMouseButton(window, mouse_button);
            return state == GLFW_PRESS;
        }

        Vec2f mouse_position(GLFWwindow* window)
        {
            double x, y;
            glfwGetCursorPos(window, &x, &y);
            return Vec2f(static_cast<float>(x), static_cast<float>(y));
        }
    }


    camera::camera(GLFWwindow* window, float v_fov, float aspect_ratio, float near_clip, float far_clip)
        : m_window(window), m_vertical_fov(v_fov), m_aspect_ratio(aspect_ratio), m_near_clip(near_clip), m_far_clip(far_clip)
    {
        update_projection_matrix();
        update_view_matrix();
    }

    camera::~camera()
    {

    }

    void camera::on_event(event& e)
    {
        if (e.handled())
            return;

        if (e.type() == mouse_scrolled_event::static_type())
        {
            on_mouse_scrolled((mouse_scrolled_event&)e);
            e.set_handled(true);
        }
    }

    void camera::on_update(double ts)
    {
        if (is_key_pressed(m_window, KEY_LEFT_CONTROL))
        {
            Vec2f mouse_pos = mouse_position(m_window);
            Vec2f delta = (mouse_pos - m_mouse_position_before) * 0.003f;
            m_mouse_position_before = mouse_pos;

            if (is_mouse_button_pressed(m_window, MOUSE_BUTTON_LEFT)) {
                on_mouse_paned(delta);
            }
            else if (is_mouse_button_pressed(m_window, MOUSE_BUTTON_RIGHT)) {
                on_mouse_rotated(delta);
            }
            else if (is_mouse_button_pressed(m_window, MOUSE_BUTTON_MIDDLE)) {
                on_mouse_zoomed(delta.y());
            }
        }
        update_view_matrix();
    }

    const Mat4f camera::view_matrix() const
    {
        return m_view_matrix;
    }

    const Mat4f camera::projection_matrix() const
    {
        return m_projection_matrix;
    }

    const Mat4f camera::view_projection_matrix() const
    {
        return m_projection_matrix * m_view_matrix;
    }

    float camera::distance() const
    {
        return m_distance;
    }

    void camera::set_distance(float dist)
    {
        m_distance = dist;
    }

    const Vec2f camera::viewport_size() const
    {
        return m_viewport_size;
    }

    void camera::set_viewport_size(const Vec2f& size)
    {
        m_viewport_size = size;
    }

    const Vec3f camera::up_direction() const
    {
        return glm::rotate(orientation(), glm::vec3(0.0f, 1.0f, 0.0f));
    }

    const Vec3f camera::right_direction() const
    {
        return glm::rotate(orientation(), glm::vec3(1.0f, 0.0f, 0.0f));
    }

    const Vec3f camera::forward_direction() const
    {
        return glm::rotate(orientation(), glm::vec3(0.0f, 0.0f, -1.0f));
    }

    const Vec3f camera::eye() const
    {
        return m_eye;
    }

    const Vec3f camera::lookAt() const
    {
        return m_look_at;
    }

    void camera::update_view_matrix()
    {
        update_camera_position();
        auto InverViewMat = glm::translate(glm::mat4(1.0f), m_eye) * glm::toMat4(orientation());
        m_view_matrix = glm::inverse(InverViewMat);
    }

    void camera::update_projection_matrix()
    {
        m_aspect_ratio = m_viewport_size.y() / m_viewport_size.x();
        m_projection_matrix = glm::perspective(glm::radians(m_vertical_fov), m_aspect_ratio, m_near_clip, m_far_clip);
    }

    void camera::update_camera_position()
    {
        m_eye = Vec3f(m_look_at) - forward_direction() * m_distance;
    }

    bool camera::on_mouse_scrolled(mouse_scrolled_event& e)
    {
        float delta = e.y() * 0.1f;
        on_mouse_zoomed(delta);
        update_view_matrix();
        return false;
    }

    void camera::on_mouse_paned(const Vec2f& delta)
    {
        auto [x_speed, y_speed] = pan_speed();
        m_look_at += (-glm::vec3(right_direction()) * delta.x() * x_speed * distance());
        m_look_at += glm::vec3(up_direction() * delta.y() * y_speed * distance());
    }

    void camera::on_mouse_rotated(const Vec2f& delta)
    {
        float yawSign = up_direction().y() < 0.0f ? -1.0f : 1.0f;
        m_yaw += yawSign * delta.x() * rotation_speed();
        m_pitch += delta.y() * rotation_speed();
    }

    void camera::on_mouse_zoomed(float delta)
    {
        m_distance -= delta * zoom_speed();
        if (m_distance < 1.0f) {
            m_look_at += glm::vec3(forward_direction());
            m_distance = 1.0f;
        }
    }

    const std::pair<float, float> camera::pan_speed() const
    {
        float x = (std::min)(m_viewport_size.x() / 1000.f, 2.4f);
        float x_factor = 0.0366f * (x * x) - 0.1788f * x + 0.3021f;

        float y = (std::min)(m_viewport_size.y() / 1000.f, 2.4f);
        float y_factor = 0.0366f * (y * y) - 0.1788f * y + 0.3021f;

        return { x_factor, y_factor };
    }

    float camera::rotation_speed() const
    {
        return 0.8f;
    }

    float camera::zoom_speed() const
    {
        float dist = m_distance * 0.2f;
        dist = (std::max)(dist, 0.0f);
        float speed = dist * dist;
        speed = (std::min)(speed, 100.f);
        return speed;
    }

    const glm::quat camera::orientation() const
    {
        return glm::quat(glm::vec3(-m_pitch, -m_yaw, -m_roll));
    }

}