#include "frame.h"

#include <glad/glad.h>

#include <map>

namespace VN
{
    namespace
    {
        bool is_depth_format(frame_buffer_texture_format format)
        {
            switch (format)
            {
            case frame_buffer_texture_format::kDepth:
                return true;
            case frame_buffer_texture_format::kNone:
            case frame_buffer_texture_format::kRGBA8:
            case frame_buffer_texture_format::kRedInteger:
            default:
                return false;
            }
        }

        GLenum target_texture(bool multiSampled)
        {
            return multiSampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
        }

        void attach_color_texture(unsigned int id, const frame_buffer_specification& spec, GLenum internal_format, GLenum format, int index)
        {
            bool multi_sampled = spec.samples > 1;
            if (multi_sampled) {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, spec.samples, internal_format, spec.width, spec.height, GL_FALSE);
            }
            else {
                glTexImage2D(GL_TEXTURE_2D, 0, internal_format, spec.width, spec.height, 0, format, GL_UNSIGNED_BYTE, nullptr);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, target_texture(multi_sampled), id, 0);
        }

        void attachDepthTexture(unsigned int id, const frame_buffer_specification& spec, GLenum format, GLenum attachmentType)
        {
            bool multiSampled = spec.samples > 1;
            if (multiSampled) {
                glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, spec.samples, format, spec.width, spec.height, GL_FALSE);
            }
            else {
                glTexStorage2D(GL_TEXTURE_2D, 1, format, spec.width, spec.height);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            }
            glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, target_texture(multiSampled), id, 0);
        }

        GLenum formatEnumToGLenum(frame_buffer_texture_format format)
        {
            switch (format)
            {
            case frame_buffer_texture_format::kNone:
                return GL_NONE;
            case frame_buffer_texture_format::kRGBA8:
                return GL_RGBA8;
            case frame_buffer_texture_format::kRedInteger:
                return GL_RED_INTEGER;
            case frame_buffer_texture_format::kDepth:
            default:
                return GL_NONE;
            }
        }
    }

    frame_buffer::frame_buffer(const frame_buffer_specification& spec)
    {
        for (auto& attach : m_specification.attachments) {
            if (is_depth_format(attach)) {
                m_depth_attachment_specification = attach;
            }
            else {
                m_color_attachment_specifications.push_back(attach);
            }
        }
        invalidate();
    }

    frame_buffer::~frame_buffer()
    {
        glDeleteFramebuffers(1, &m_renderer_id);
        glDeleteTextures((GLsizei)m_color_attachments.size(), m_color_attachments.data());
        glDeleteTextures(1, &m_depth_attachment);
    }

    const frame_buffer_specification& frame_buffer::specification() const
    {
        return m_specification;
    }

    void frame_buffer::bind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, m_renderer_id);
        glViewport(0, 0, m_specification.width, m_specification.height);
    }

    void frame_buffer::unbind() const
    {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void frame_buffer::on_resize(unsigned int width, unsigned int height)
    {
        static unsigned maximum_framebuffer_size = 4096;
        if (width == 0 || width >= maximum_framebuffer_size || height == 0 || height >= maximum_framebuffer_size) {
            return;
        }
        m_specification.width = width;
        m_specification.height = height;
        invalidate();
    }

    unsigned int frame_buffer::color_attachment_renderer_id(unsigned int index) const
    {
        return m_color_attachments[index];
    }

    void frame_buffer::clear_attachment(unsigned int attachmentId, int value)
    {
        auto& spec = m_color_attachment_specifications[attachmentId];
        glClearTexImage(m_color_attachments[attachmentId], 0, formatEnumToGLenum(spec), GL_INT, &value);
    }


    void frame_buffer::invalidate()
    {
        if (m_renderer_id) {
            glDeleteFramebuffers(1, &m_renderer_id);
            glDeleteTextures((GLsizei)m_color_attachments.size(), m_color_attachments.data());
            glDeleteTextures(1, &m_depth_attachment);
            m_color_attachments.clear();
            m_depth_attachment = 0;
        }

        glCreateFramebuffers(1, &m_renderer_id);
        glBindFramebuffer(GL_FRAMEBUFFER, m_renderer_id);

        bool multiSampled = m_specification.samples > 1;
        if (m_color_attachment_specifications.size()) {
            m_color_attachments.resize(m_color_attachment_specifications.size());
            glCreateTextures(target_texture(multiSampled), (GLsizei)m_color_attachments.size(), m_color_attachments.data());
            for (unsigned int i = 0; i < m_color_attachments.size(); i++) {
                glBindTexture(target_texture(multiSampled), m_color_attachments[i]);
                switch (m_color_attachment_specifications[i])
                {
                case frame_buffer_texture_format::kRGBA8:
                    attach_color_texture(m_color_attachments[i], m_specification, GL_RGBA8, GL_RGBA, i);
                    break;
                case frame_buffer_texture_format::kRedInteger:
                    attach_color_texture(m_color_attachments[i], m_specification, GL_R32I, GL_RED_INTEGER, i);
                    break;
                }
            }
        }

        if (m_depth_attachment_specification != frame_buffer_texture_format::kNone) {
            glCreateTextures(target_texture(multiSampled), 1, &m_depth_attachment);
            glBindTexture(target_texture(multiSampled), m_depth_attachment);
            switch (m_depth_attachment_specification)
            {
            case frame_buffer_texture_format::kDepth:
                attachDepthTexture(m_depth_attachment, m_specification, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT);
                break;
            }
        }

        if (m_color_attachments.size() > 0) {
            GLenum buffers[4] = {
                GL_COLOR_ATTACHMENT0,
                GL_COLOR_ATTACHMENT1,
                GL_COLOR_ATTACHMENT2,
                GL_COLOR_ATTACHMENT3
            };
            glDrawBuffers(4, buffers);
        }
        else {
            glDrawBuffer(GL_NONE);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

}