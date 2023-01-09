#pragma once

#include "Turbo/Core/Common.h"

//#include "Turbo/Renderer/Image2D.h"
//#include "Turbo/Renderer/RenderPass.h"

#define TBO_FRAMEBUFFER_MAX_ATTACHMENTS 4

namespace Turbo
{
    class Image2D;
    class RenderPass;

    class FrameBuffer
    {
    public:
        enum ColorWriteMask_ : u32
        {
            ColorWriteMask_R = 0x00000001,
            ColorWriteMask_G = 0x00000002,
            ColorWriteMask_B = 0x00000004,
            ColorWriteMask_A = 0x00000008,
            ColorWriteMask_RGBA = ColorWriteMask_R | ColorWriteMask_G | ColorWriteMask_B | ColorWriteMask_A
        };
        using ColorWriteMask = u32;

        enum BlendFactor : u32
        {
            BlendFactor_SrcAlpha = 6,
            BlendFactor_OneMinus_SrcAlpha = 7,
        };

        enum BlendOperation : u32
        {
            BlendOperation_Add = 0,
            BlendOperation_Substract
        };

        struct Attachment
        {
            Image2D* Image;

            ColorWriteMask ColorMask;
            bool EnableBlend;
            BlendFactor SrcBlendFactor;
            BlendFactor DstBlendFactor;
            BlendOperation BlendOperation;
        };

        struct Config
        {
            RenderPass* Renderpass;
            Attachment Attachments[TBO_FRAMEBUFFER_MAX_ATTACHMENTS];
            u32 AttachmentsCount;
            Ptr<Image2D> DepthBuffer;
        };

        virtual ~FrameBuffer();

        static FrameBuffer* Create(const FrameBuffer::Config& config);
        
        const FrameBuffer::Config& GetConfig() const { return m_Config; }

        virtual void Invalidate(u32 width, u32 height) = 0;
    protected:
        FrameBuffer(const FrameBuffer::Config& config);

        FrameBuffer::Config m_Config;
    };
}
