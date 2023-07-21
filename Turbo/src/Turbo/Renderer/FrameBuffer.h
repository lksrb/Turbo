#pragma once

#include "Turbo/Core/Common.h"

#include "Turbo/Renderer/Image2D.h"

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
            ColorWriteMask ColorMask;
            bool EnableBlend;
            BlendFactor SrcBlendFactor;
            BlendFactor DstBlendFactor;
            BlendOperation BlendOperation;
        };

        struct Config
        {
            Attachment ColorAttachment;
            bool EnableDepthTesting = true;
            u32 Width;
            u32 Height;
        };

        virtual ~FrameBuffer();

        static Ref<FrameBuffer> Create(const FrameBuffer::Config& config);
        
        void SetRenderPass(Ref<RenderPass> renderPass) { m_RenderPass = renderPass; }

        const FrameBuffer::Config& GetConfig() const { return m_Config; }

        virtual Ref<Image2D> GetColorAttachment() const = 0;

        virtual void Invalidate(u32 width, u32 height) = 0;
    protected:
        FrameBuffer(const FrameBuffer::Config& config);

        Ref<RenderPass> m_RenderPass;

        FrameBuffer::Config m_Config;
    };
}
