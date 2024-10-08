#pragma once

#include <vector>

namespace VN
{
    enum class frame_buffer_texture_format
    {
        kNone,
        kRGBA8,
        kRedInteger,
        kDepth,
        kDepth24Stencil8 = kDepth
    };

    struct frame_buffer_specification
    {
        unsigned int width;
        unsigned int height;
        unsigned int samples = 1;
        bool swap_chain_target = false;
        std::vector<frame_buffer_texture_format> attachments;
    };

    class frame_buffer
    {
    public:
        frame_buffer(const frame_buffer_specification& spec);
        ~frame_buffer();

    public:
        const frame_buffer_specification& specification() const;
        void bind() const;
        void unbind() const;
        void on_resize(unsigned int width, unsigned int height);
        unsigned int color_attachment_renderer_id(unsigned int index) const;
        void clear_attachment(unsigned int attachmentId, int value);

    private:
        void invalidate();

    protected:
        unsigned int m_renderer_id = -1;
        frame_buffer_specification m_specification;
        std::vector<frame_buffer_texture_format> m_color_attachment_specifications;
        frame_buffer_texture_format m_depth_attachment_specification;
        std::vector<unsigned int> m_color_attachments;
        unsigned int m_depth_attachment = 0;
    };
}