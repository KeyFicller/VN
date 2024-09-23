#pragma once

#include "vector_matrix.h"

namespace VN
{
    class event;

    class camera
    {
    public:
        camera(float VFov, float aspectRatio, float nearClip, float FarClip);
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
    };
}