#pragma once

#include "Turbo/Renderer/Image2D.h"

namespace Turbo
{
    class RenderPass;

    class FrameBuffer : public RefCounted
    {
    public:
        enum AttachmentType : u32
        {
            AttachmentType_Color = 0,
            AttachmentType_SelectionBuffer = 1,
            AttachmentType_Depth = 2,

            AttachmentType_Count = 3
        };

        struct Attachment
        {
            AttachmentType Type;
            u32 Count = 1;

            Attachment(AttachmentType type) : Type(type) {}
        };

        // NOTE: This setup means there cannot be two attachments of the same type
        // For now its completely okay
        struct Config
        {
            std::vector<Attachment> Attachments;
            glm::vec4 ClearColor = { 0.0f, 0.0f, 0.0f, 1.0f };

            u32 Width;
            u32 Height;
        };

        virtual ~FrameBuffer();

        static Ref<FrameBuffer> Create(const FrameBuffer::Config& config);
        
        void SetRenderPass(Ref<RenderPass> renderPass) { m_RenderPass = renderPass; }

        const FrameBuffer::Config& GetConfig() const { return m_Config; }

        virtual Ref<Image2D> GetAttachment(AttachmentType type, u32 index = 0) const = 0;

        virtual void Invalidate(u32 width, u32 height) = 0;
    protected:
        FrameBuffer(const FrameBuffer::Config& config);

        Ref<RenderPass> m_RenderPass;

        FrameBuffer::Config m_Config;
    };
}
