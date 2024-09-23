#pragma once

#include <initializer_list>
#include <sstream>
#include <functional>

#ifdef USE_GLM_IN_MODULE
#include <glm/glm.hpp>
#endif

#ifdef USE_IMGUI_IN_MODULE
#include <imgui.h>
#endif

namespace VN
{
    template <typename D, int N>
    struct VecImpl_t
    {
        VecImpl_t()
        {
            memset(m_Data, 0, sizeof(VecImpl_t));
        }

        VecImpl_t(const std::initializer_list<D>& elements)
        {
            // ASSERT(elements.size() <= N, "There are %d elements assigned to %d component vector.", elements.size(), N);
            memset(m_Data, 0, sizeof(VecImpl_t));
            int i = 0;
            for (const auto& element : elements) {
                m_Data[i++] = element;
            }
        }

        template <typename = std::enable_if_t<N == 2>>
        VecImpl_t(const D& x, const D& y)
            : VecImpl_t({ x, y }) {}

        template <typename = std::enable_if_t<N == 3>>
        VecImpl_t(const D& x, const D& y, const D& z)
            : VecImpl_t({ x, y, z }) {}

        template <typename = std::enable_if_t<N == 4>>
        VecImpl_t(const D& x, const D& y, const D& z, const D& w)
            : VecImpl_t({ x, y, z, w }) {}

        D& operator [] (int index)
        {
            // ASSERT(index >= 0 && index < N, "Index out of range.");
            return m_Data[index];
        }

        const D operator [] (int index) const
        {
            // ASSERT(index >= 0 && index < N, "Index out of range.");
            return m_Data[index];
        }

        bool operator == (const VecImpl_t& other)
        {
            return !memcmp(m_Data, other.m_Data, sizeof(VecImpl_t));
        }

        bool operator != (const VecImpl_t& other)
        {
            return !(operator == (other));
        }

#define VNT_VECTOR_COMPONENT_SHORT_CUT(SHORT_CUT, INDEX)                                  \
        template<typename = std::enable_if_t<N >= INDEX>>                                 \
        D& SHORT_CUT()                                                                    \
        {                                                                                 \
            return m_Data[INDEX - 1];                                                     \
        }                                                                                 \
        template<typename = std::enable_if_t<N >= INDEX>>                                 \
        const D& SHORT_CUT() const                                                        \
        {                                                                                 \
            return m_Data[INDEX - 1];                                                     \
        }

        VNT_VECTOR_COMPONENT_SHORT_CUT(x, 1);
        VNT_VECTOR_COMPONENT_SHORT_CUT(y, 2);
        VNT_VECTOR_COMPONENT_SHORT_CUT(z, 3);
        VNT_VECTOR_COMPONENT_SHORT_CUT(w, 4);
        VNT_VECTOR_COMPONENT_SHORT_CUT(r, 1);
        VNT_VECTOR_COMPONENT_SHORT_CUT(g, 2);
        VNT_VECTOR_COMPONENT_SHORT_CUT(b, 3);
        VNT_VECTOR_COMPONENT_SHORT_CUT(a, 4);

#undef VNT_VECTOR_COMPONENT_SHORT_CUT

        explicit VecImpl_t(const VecImpl_t<D, N - 1>& origin, const D appended = static_cast<D>(0))
        {
            memcpy_s(m_Data, sizeof(VecImpl_t<D, N - 1>), origin.m_Data, sizeof(VecImpl_t<D, N - 1>));
            m_Data[N - 1] = appended;
        }

        template <typename = std::enable_if_t<N == 3 || N == 4>>
        VecImpl_t<D, 2> xy() const
        {
            return VecImpl_t<D, 2>(m_Data[0], m_Data[1]);
        }

        template <typename = std::enable_if_t<N == 4>>
        VecImpl_t<D, 3> xyz() const
        {
            return VecImpl_t<D, 3>(m_Data[0], m_Data[1], m_Data[2]);
        }


// for example for later development
#ifdef USE_GLM_IN_MODULE
        template <typename = std::enable_if_t<std::is_same<D, float>::value>, typename = std::enable_if_t<N == 2>>
        VecImpl_t(const glm::vec2& vec2f)
            : VecImpl_t({ vec2f.x, vec2f.y })
        {

        }

        template <typename = std::enable_if_t<std::is_same<D, float>::value>, typename = std::enable_if_t<N == 3>>
        VecImpl_t(const glm::vec3& vec3f)
            : VecImpl_t({vec3f.x, vec3f.y, vec3f.z})
        {

        }

        template <typename = std::enable_if_t<std::is_same<D, float>::value>, typename = std::enable_if_t<N == 4>>
        VecImpl_t(const glm::vec4& vec4f)
            : VecImpl_t({ vec4f.x, vec4f.y, vec4f.z, vec4f.w })
        {

        }

        template <typename = std::enable_if_t<std::is_same<D, float>::value>, typename = std::enable_if_t<N == 2>>
        operator glm::vec2() const
        {
            return glm::vec2(m_Data[0], m_Data[1]);
        }

        template <typename = std::enable_if_t<std::is_same<D, float>::value>, typename = std::enable_if_t<N == 3>>
        operator glm::vec3() const
        {
            return glm::vec3(m_Data[0], m_Data[1], m_Data[2]);
        }

        template <typename = std::enable_if_t<std::is_same<D, float>::value>, typename = std::enable_if_t<N == 4>>
        operator glm::vec4() const
        {
            return glm::vec4(m_Data[0], m_Data[1], m_Data[2], m_Data[3]);
        }
#endif // USING_GLM_IN_MODULE

#ifdef USE_IMGUI_IN_MODULE
        template <typename = std::enable_if_t<std::is_same<D, float>::value>, typename = std::enable_if_t<N == 2>>
        VecImpl_t(const ImVec2& vec2f)
            : VecImpl_t({vec2f.x, vec2f.y})
        {

        }

        template <typename = std::enable_if_t<std::is_same<D, float>::value>, typename = std::enable_if_t<N == 2>>
        operator ImVec2() const
        {
            return ImVec2(m_Data[0], m_Data[1]);
        }

        template <typename = std::enable_if_t<std::is_same<D, float>::value>, typename = std::enable_if_t<N == 4>>
        VecImpl_t(const ImVec4& vec4f)
            : VecImpl_t({ vec4f.x, vec4f.y, vec4f.y, vec4f.z })
        {

        }

        template <typename = std::enable_if_t<std::is_same<D, float>::value>, typename = std::enable_if_t<N == 4>>
        operator ImVec4() const
        {
            return ImVec4(x(), y(), z(), w());
        }
#endif

        D m_Data[N];
    };

    template <typename D, int N>
    struct MatImpl_t
    {
        using Vec = VecImpl_t<D, N>;

        MatImpl_t()
        {
            memset(m_Data, 0, sizeof(MatImpl_t));
        }

        MatImpl_t(const std::initializer_list<Vec>& elements)
        {
            // ASSERT(elements.size() <= N, "There are %d elements assigned to %d component matrix.", elements.size(), N);
            memset(m_Data, 0, sizeof(MatImpl_t));
            int i = 0;
            for (const auto& element : elements) {
                m_Data[i++] = element;
            }
        }

        template <typename = std::enable_if_t<N == 2>>
        MatImpl_t(const Vec& col1, const Vec& col2)
            : MatImpl_t({ col1, col2 }) {}

        template <typename = std::enable_if_t<N == 3>>
        MatImpl_t(const Vec& col1, const Vec& col2, const Vec& col3)
            : MatImpl_t({ col1, col2, col3 }) {}

        template <typename = std::enable_if_t<N == 4>>
        MatImpl_t(const Vec& col1, const Vec& col2, const Vec& col3, const Vec& col4)
            : MatImpl_t({ col1, col2, col3, col4 }) {}

        Vec& operator [] (int index)
        {
            // ASSERT(index >= 0 && index < N, "Index out of range.");
            return m_Data[index];
        }

        const Vec operator [] (int index) const
        {
            // ASSERT(index >= 0 && index < N, "Index out of range.");
            return m_Data[index];
        }

#ifdef USE_GLM_IN_MODULE
        template <typename = std::enable_if_t<std::is_same<D, float>::value>, typename = std::enable_if_t<N == 3>>
        MatImpl_t(const glm::mat3& mat3f)
        {
            m_Data[0] = Vec3f(mat3f[0]);
            m_Data[1] = Vec3f(mat3f[1]);
            m_Data[2] = Vec3f(mat3f[2]);
        }

        template <typename = std::enable_if_t<std::is_same<D, float>::value>, typename = std::enable_if_t<N == 3>>
        operator glm::mat3()
        {
            return *reinterpret_cast<glm::mat3*>(m_Data);
        }

        template <typename = std::enable_if_t<std::is_same<D, float>::value>, typename = std::enable_if_t<N == 4>>
        MatImpl_t(const glm::mat4& mat4f)
        {
            m_Data[0] = Vec4f(mat4f[0]);
            m_Data[1] = Vec4f(mat4f[1]);
            m_Data[2] = Vec4f(mat4f[2]);
            m_Data[3] = Vec4f(mat4f[3]);
        }

        template <typename = std::enable_if_t<std::is_same<D, float>::value>, typename = std::enable_if_t<N == 4>>
        operator glm::mat4()
        {
            return *reinterpret_cast<glm::mat4*>(m_Data);
        }
#endif

        MatImpl_t& transpose()
        {
            *this = transposed();
            return *this;
        }

        static MatImpl_t identity()
        {
            MatImpl_t result;
            for (int i = 0; i < N; i++) {
                result[i][i] = static_cast<D>(1);
            }
            return result;
        }

        MatImpl_t transposed() const
        {
            MatImpl_t result;
            for (int i = 0; i < N; i++) {
                for (int j = 0; j < N; j++) {
                    result.m_Data[i][j] = m_Data[j][i];
                }
            }
            return result;
        }

        VecImpl_t<D, N> m_Data[N];
    };


    template <typename T, typename ...Args>
    using Callback_t = std::function<T(Args...)>;

    template <typename T>
    using Vec2_t = VecImpl_t<T, 2>;

    template <typename T>
    using Vec3_t = VecImpl_t<T, 3>;

    template<typename T>
    using Vec4_t = VecImpl_t<T, 4>;

#define VNT_VECTOR_TYPE_SHORT_CUT(SHORTCUT, T)                  \
    using Vec2##SHORTCUT = Vec2_t<T>;                           \
    using Vec3##SHORTCUT = Vec3_t<T>;                           \
    using Vec4##SHORTCUT = Vec4_t<T>

    VNT_VECTOR_TYPE_SHORT_CUT(i, int);
    VNT_VECTOR_TYPE_SHORT_CUT(ui, unsigned int);
    VNT_VECTOR_TYPE_SHORT_CUT(f, float);
    VNT_VECTOR_TYPE_SHORT_CUT(d, double);

#undef VNT_VECTOR_TYPE_SHORT_CUT

    template <typename T>
    using Mat2_t = MatImpl_t<T, 2>;

    template <typename T>
    using Mat3_t = MatImpl_t<T, 3>;

    template <typename T>
    using Mat4_t = MatImpl_t<T, 4>;

    using Mat2f = Mat2_t<float>;
    using Mat3f = Mat3_t<float>;
    using Mat4f = Mat4_t<float>;
}