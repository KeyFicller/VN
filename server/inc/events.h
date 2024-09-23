#pragma once

#include <string>

namespace VN
{
    enum event_category : unsigned int
    {
        kApplication = 0x0100,
        kInput = 0x0200,
        kKeyboard = 0x0400,
        kMouse = 0x0800,
        kMouseButton = 0x0100
    };

    enum event_type : unsigned int
    {
        kNone = 0,
        kWindowClosed = event_category::kApplication | 0x01,
        kWindowResized,
        kWindowFocused,
        kWindowLoseFocused,
        kWindowMoved,
        kApplicationTicked,
        kApplicationUpdated,
        kApplicationRendered,
        kKeyPressed = event_category::kInput | event_category::kKeyboard | 0x01,
        kKeyReleased,
        kKeyTyped,
        kMouseMoved = event_category::kInput | event_category::kMouse,
        kMouseScrolled,
        kMouseButtonPressed = event_category::kInput | event_category::kMouseButton,
        kMouseButtonReleased
    };

    class event
    {
    public:
        event() = default;
        virtual ~event() = default;

    public:
        virtual event_type type() const = 0;

        bool handled() const
        {
            return m_handled;
        }

        void set_handled(bool handled = true)
        {
            m_handled = handled;
        }

    protected:
        bool m_handled = false;
    };


#define VN_EVENT_TYPE(TYPE)                                     \
    public:                                                     \
        static event_type static_type()                         \
        {                                                       \
            return event_type::TYPE;                            \
        }                                                       \
        event_type type() const override                        \
        {                                                       \
            return static_type();                               \
        }

    class mouse_scrolled_event : public event
    {
        VN_EVENT_TYPE(kMouseScrolled)
    public:
        mouse_scrolled_event(float x, float y)
            : m_x(x), m_y(y) {}

    public:
        float x() const
        {
            return m_x;
        }

        float y() const
        {
            return m_y;
        }

    protected:
        float m_x;
        float m_y;
    };
}