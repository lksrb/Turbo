#pragma once

#include "Turbo/Core/Common.h"

#include "Turbo/Renderer/Image2D.h"

namespace Turbo
{
    class RenderPass;

    class FrameBuffer
    {
    public:
        enum AttachmentType : u32
        {
            AttachmentType_Color = 0,
            AttachmentType_Depth,
            AttachmentType_Count,
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
            bool EnableDepthTesting = true; // TODO: Remove
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
