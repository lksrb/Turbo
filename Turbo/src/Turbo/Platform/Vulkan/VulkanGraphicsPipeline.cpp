#include "tbopch.h"
#include "VulkanGraphicsPipeline.h"

#include "Turbo/Renderer/RendererContext.h"

#include "Turbo/Platform/Vulkan/VulkanShader.h"
#include "Turbo/Platform/Vulkan/VulkanRenderPass.h"

namespace Turbo
{
    namespace Utils
    {
        static VkPrimitiveTopology TboTopologyToVkTopology(PrimitiveTopology topology)
        {
            switch (topology)
            {
                case PrimitiveTopology::Triangle:   return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                case PrimitiveTopology::Line:       return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            }

            TBO_ENGINE_ASSERT(false, "Unknown primitive topology!");
            return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
        }

        static VkShaderStageFlagBits TboStageToVkStage(ShaderStage stage)
        {
            switch (stage)
            {
                case ShaderStage_Vertex:     return VK_SHADER_STAGE_VERTEX_BIT;
                case ShaderStage_Fragment:   return VK_SHADER_STAGE_FRAGMENT_BIT;
            }

            TBO_ENGINE_ASSERT(false, "Unknown shader stage!");
            return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
        }
    }

    VulkanGraphicsPipeline::VulkanGraphicsPipeline(const GraphicsPipeline::Config& config)
        : GraphicsPipeline(config)
    {
    }

    VulkanGraphicsPipeline::~VulkanGraphicsPipeline()
    {
    }

    void VulkanGraphicsPipeline::Invalidate()
    {
        VkDevice device = RendererContext::GetDevice();

        // ###############################################################################################################
        // ##################                                    Stages                                 ##################
        // ###############################################################################################################

        VkPipelineShaderStageCreateInfo shaderStagesCreateInfo[ShaderStage_Max] = {};
        for (u32 shaderStage = 0; shaderStage < ShaderStage_Max; ++shaderStage)
        {
            shaderStagesCreateInfo[shaderStage].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStagesCreateInfo[shaderStage].pNext = nullptr;
            shaderStagesCreateInfo[shaderStage].stage = Utils::TboStageToVkStage(shaderStage);
            shaderStagesCreateInfo[shaderStage].pName = "main";
            shaderStagesCreateInfo[shaderStage].module = m_Config.Shader.As<VulkanShader>()->GetModule(shaderStage);
        }
        // ###############################################################################################################
        // ##################                                   Vertex Input                            ##################
        // ###############################################################################################################
        auto& layout = m_Config.Shader.As<VulkanShader>()->GetLayout();

        VkPipelineVertexInputStateCreateInfo vertexInputState = {};
        vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputState.pNext = VK_NULL_HANDLE;
        vertexInputState.pVertexAttributeDescriptions = (VkVertexInputAttributeDescription*)layout.Descriptions.data();
        vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(layout.Descriptions.size());

        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = static_cast<uint32_t>(layout.Stride);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        vertexInputState.vertexBindingDescriptionCount = 1;
        vertexInputState.pVertexBindingDescriptions = &bindingDescription; // Optional
        vertexInputState.pVertexBindingDescriptions = &bindingDescription;
        vertexInputState.vertexBindingDescriptionCount = 1;

        // ###############################################################################################################
        // ##################                              Index Buffer                                 ##################
        // ###############################################################################################################
        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.pNext = VK_NULL_HANDLE;
        inputAssembly.topology = Utils::TboTopologyToVkTopology(m_Config.Topology);
        inputAssembly.primitiveRestartEnable = VK_FALSE;

        // ###############################################################################################################
        // ##################                               Viewport State                              ##################
        // ###############################################################################################################
        VkPipelineViewportStateCreateInfo viewportState{};
        viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewportState.viewportCount = 1;
        viewportState.pViewports = nullptr;
        viewportState.scissorCount = 1;
        viewportState.pScissors = nullptr;

        // ###############################################################################################################
        // ##################                              Rasterizer                                   ##################
        // ###############################################################################################################
        VkPipelineRasterizationStateCreateInfo rasterizer = {};
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = VK_FALSE;
        rasterizer.rasterizerDiscardEnable = VK_FALSE;

        if (m_Config.Topology == PrimitiveTopology::Triangle)
        {
            rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        }
        else if (m_Config.Topology == PrimitiveTopology::Line)
        {
            rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
            rasterizer.lineWidth = 1.0f;
        }
        rasterizer.cullMode = VK_CULL_MODE_NONE;            // For now
        rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterizer.depthBiasEnable = VK_FALSE;
        rasterizer.depthBiasConstantFactor = 0.0f;          // Optional
        rasterizer.depthBiasClamp = 0.0f;                   // Optional
        rasterizer.depthBiasSlopeFactor = 0.0f;             // Optional

        // ###############################################################################################################
        // ##################                              Multisampling                                ##################
        // ###############################################################################################################
        VkPipelineMultisampleStateCreateInfo multisampleState = {};
        multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisampleState.sampleShadingEnable = VK_FALSE;
        multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisampleState.minSampleShading = 1.0f; // Optional
        multisampleState.pSampleMask = nullptr; // Optional
        multisampleState.alphaToCoverageEnable = VK_FALSE; // Optional
        multisampleState.alphaToOneEnable = VK_FALSE; // Optional

        // ###############################################################################################################
        // ##################                          Depth and stencil testing                        ##################
        // ###############################################################################################################
        VkPipelineDepthStencilStateCreateInfo depthAndStencilState = {};
        if (m_Config.DepthTesting)
        {
            depthAndStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthAndStencilState.depthTestEnable = VK_TRUE;
            depthAndStencilState.depthWriteEnable = VK_TRUE;
            depthAndStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
            depthAndStencilState.depthBoundsTestEnable = VK_FALSE;
            depthAndStencilState.minDepthBounds = 0.0f; // Optional
            depthAndStencilState.maxDepthBounds = 1.0f; // Optional
            depthAndStencilState.stencilTestEnable = VK_FALSE;
            
           /* depthAndStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
            depthAndStencilState.depthBoundsTestEnable = VK_FALSE;
            depthAndStencilState.back.failOp = VK_STENCIL_OP_KEEP;
            depthAndStencilState.back.passOp = VK_STENCIL_OP_KEEP;
            depthAndStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
            depthAndStencilState.stencilTestEnable = VK_FALSE;
            depthAndStencilState.front = depthAndStencilState.back;*/
        }

        // ###############################################################################################################
        // ##################                              Color Blending                               ##################
        // ###############################################################################################################

        auto& targetFbConfig = m_Config.TargetFramebuffer->GetConfig();

        VkPipelineColorBlendAttachmentState colorBlendAttachments[2] = {};
        u32 attachmentCount = 1;

        auto& framebufferAttachment = targetFbConfig.ColorAttachment;
        colorBlendAttachments[0].colorWriteMask = (VkColorComponentFlags)framebufferAttachment.ColorMask;
        colorBlendAttachments[0].blendEnable = framebufferAttachment.EnableBlend;
        colorBlendAttachments[0].srcColorBlendFactor = (VkBlendFactor)framebufferAttachment.SrcBlendFactor;
        colorBlendAttachments[0].dstColorBlendFactor = (VkBlendFactor)framebufferAttachment.DstBlendFactor;
        colorBlendAttachments[0].colorBlendOp = (VkBlendOp)framebufferAttachment.BlendOperation; // Optional
        colorBlendAttachments[0].srcAlphaBlendFactor = (VkBlendFactor)framebufferAttachment.SrcBlendFactor; // Idk what is this
        colorBlendAttachments[0].dstAlphaBlendFactor = (VkBlendFactor)framebufferAttachment.DstBlendFactor;
        colorBlendAttachments[0].alphaBlendOp = (VkBlendOp)framebufferAttachment.BlendOperation; // Optional

        VkPipelineColorBlendStateCreateInfo colorBlendState{};
        colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendState.attachmentCount = attachmentCount;
        colorBlendState.pAttachments = colorBlendAttachments;
        colorBlendState.logicOpEnable = VK_FALSE;
        colorBlendState.logicOp = VK_LOGIC_OP_COPY; // Optional
        colorBlendState.blendConstants[0] = 1.0f; // Optional
        colorBlendState.blendConstants[1] = 1.0f; // Optional
        colorBlendState.blendConstants[2] = 1.0f; // Optional
        colorBlendState.blendConstants[3] = 1.0f; // Optional

        // ###############################################################################################################
        // ##################                              Dynamic States                               ##################
        // ###############################################################################################################
        std::array<VkDynamicState, 3> states = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR , VK_DYNAMIC_STATE_LINE_WIDTH };

        VkPipelineDynamicStateCreateInfo dynamicStatesInfo{};
        dynamicStatesInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicStatesInfo.pDynamicStates = states.data();
        dynamicStatesInfo.flags = 0;
        dynamicStatesInfo.dynamicStateCount = static_cast<u32>(states.size());

        // ###############################################################################################################
        // ##################                              Pipeline Layout                              ##################
        // ###############################################################################################################
        // Uniforms, samplers, etc.
        VkDescriptorSetLayout descriptorLayout = m_Config.Shader.As<VulkanShader>()->GetDescriptorSetLayout();

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.pNext = nullptr;

        // Incase no uniform or other resource are not present
        pipelineLayoutInfo.setLayoutCount = descriptorLayout ? 1 : 0; // Optional
        pipelineLayoutInfo.pSetLayouts = descriptorLayout ? &descriptorLayout : VK_NULL_HANDLE; // Optional
        
        pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
        pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

        TBO_VK_ASSERT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout));

        // ###############################################################################################################
        // ##################                              Pipeline Creation                            ##################
        // ###############################################################################################################
        VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.pNext = nullptr;
        pipelineCreateInfo.renderPass = m_Config.Renderpass.As<VulkanRenderPass>()->GetRenderPass();

        pipelineCreateInfo.pStages = shaderStagesCreateInfo;
        pipelineCreateInfo.stageCount = ShaderStage_Max;
        pipelineCreateInfo.pVertexInputState = &vertexInputState;
        pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
        pipelineCreateInfo.pViewportState = &viewportState;
        pipelineCreateInfo.pRasterizationState = &rasterizer;
        pipelineCreateInfo.pMultisampleState = &multisampleState;
        pipelineCreateInfo.pDepthStencilState = m_Config.DepthTesting ? &depthAndStencilState : VK_NULL_HANDLE;
        pipelineCreateInfo.pColorBlendState = &colorBlendState;
        pipelineCreateInfo.pDynamicState = &dynamicStatesInfo;
        pipelineCreateInfo.layout = m_PipelineLayout;
        pipelineCreateInfo.renderPass = m_Config.Renderpass.As<VulkanRenderPass>()->GetRenderPass();
        pipelineCreateInfo.subpass = 0;
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineCreateInfo.basePipelineIndex = -1; // Optional
        
        TBO_VK_ASSERT(vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineCreateInfo, nullptr, &m_Pipeline));

        // Resource free queue
        auto& resourceFreeQueue = RendererContext::GetResourceQueue();

        resourceFreeQueue.Submit(PIPELINE_LAYOUT, [device, layout = m_PipelineLayout]()
        {
            vkDestroyPipelineLayout(device, layout, nullptr);
        });

        resourceFreeQueue.Submit(PIPELINE, [device, pipeline = m_Pipeline]()
        {
            vkDestroyPipeline(device, pipeline, nullptr);
        });
    }
}
