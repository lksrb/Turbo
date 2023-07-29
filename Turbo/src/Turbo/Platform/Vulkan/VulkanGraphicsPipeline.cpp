#include "tbopch.h"
#include "VulkanGraphicsPipeline.h"

#include "Turbo/Renderer/RendererContext.h"

#include "Turbo/Platform/Vulkan/VulkanShader.h"
#include "Turbo/Platform/Vulkan/VulkanRenderPass.h"

namespace Turbo
{
    namespace Utils
    {
        static VkPrimitiveTopology TopologyToVulkanTopology(PrimitiveTopology topology)
        {
            switch (topology)
            {
                case PrimitiveTopology::Triangle:   return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
                case PrimitiveTopology::Line:       return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
            }

            TBO_ENGINE_ASSERT(false, "Unknown primitive topology!");
            return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
        }

        static VkShaderStageFlagBits ShaderStageToVulkanStage(ShaderStage stage)
        {
            switch (stage)
            {
                case ShaderStage_Vertex:     return VK_SHADER_STAGE_VERTEX_BIT;
                case ShaderStage_Fragment:   return VK_SHADER_STAGE_FRAGMENT_BIT;
            }

            TBO_ENGINE_ASSERT(false, "Unknown shader stage!");
            return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
        }


        static VkFormat AttributeTypeToVulkanFormat(AttributeType type)
        {
            switch (type)
            {
                case AttributeType::Float: return VK_FORMAT_R32_SFLOAT;
                case AttributeType::Vec2: return VK_FORMAT_R32G32_SFLOAT;
                case AttributeType::Vec3: return VK_FORMAT_R32G32B32_SFLOAT;
                case AttributeType::Vec4: return VK_FORMAT_R32G32B32A32_SFLOAT;
                case AttributeType::UInt: return VK_FORMAT_R32_UINT;
                case AttributeType::Int: return VK_FORMAT_R32_SINT;
            }


            TBO_ENGINE_ASSERT(false, "Invalid attribute type!");

            return VK_FORMAT_UNDEFINED;
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

        VkPipelineShaderStageCreateInfo shaderStagesCreateInfo[ShaderStage_Count] = {};
        for (u32 shaderStage = 0; shaderStage < ShaderStage_Count; ++shaderStage)
        {
            shaderStagesCreateInfo[shaderStage].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStagesCreateInfo[shaderStage].pNext = nullptr;
            shaderStagesCreateInfo[shaderStage].stage = Utils::ShaderStageToVulkanStage((ShaderStage)shaderStage);
            shaderStagesCreateInfo[shaderStage].pName = "main";
            shaderStagesCreateInfo[shaderStage].module = m_Config.Shader.As<VulkanShader>()->GetModule((ShaderStage)shaderStage);
        }
        // ###############################################################################################################
        // ##################                                   Vertex Input                            ##################
        // ###############################################################################################################
        //auto& layout = m_Config.Shader.As<VulkanShader>()->GetLayout();

        // Per vertex

        std::vector<VkVertexInputBindingDescription> bindingDescriptions;

        // Per vertex
        if (m_Config.Layout.AttributeCount())
        {
            auto& bindingDescription = bindingDescriptions.emplace_back();
            bindingDescription.binding = 0;
            bindingDescription.stride = m_Config.Layout.GetStride();
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        }

        // Per instance
        if (m_Config.InstanceLayout.AttributeCount())
        {
            auto& bindingDescription = bindingDescriptions.emplace_back();
            bindingDescription.binding = 1;
            bindingDescription.stride = m_Config.InstanceLayout.GetStride();
            bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
        }

        std::vector<VkVertexInputAttributeDescription> descriptions;
        descriptions.reserve(m_Config.Layout.AttributeCount());

        u32 location = 0;
        for (const auto& description : m_Config.Layout)
        {
            auto& desc = descriptions.emplace_back();
            desc.binding = 0;
            desc.location = location;
            desc.format = Utils::AttributeTypeToVulkanFormat(description.Type);
            desc.offset = description.Offset;
            location++;
        }

        for (const auto& description : m_Config.InstanceLayout)
        {
            auto& desc = descriptions.emplace_back();
            desc.binding = 1;
            desc.location = location;
            desc.format = Utils::AttributeTypeToVulkanFormat(description.Type);
            desc.offset = description.Offset;
            location++;
        }

        VkPipelineVertexInputStateCreateInfo vertexInputState = {};
        vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputState.pNext = VK_NULL_HANDLE;
        vertexInputState.vertexAttributeDescriptionCount = (u32)descriptions.size();
        vertexInputState.pVertexAttributeDescriptions = descriptions.data();
        vertexInputState.vertexBindingDescriptionCount = (u32)bindingDescriptions.size();
        vertexInputState.pVertexBindingDescriptions = bindingDescriptions.data();

        // ###############################################################################################################
        // ##################                              Index Buffer                                 ##################
        // ###############################################################################################################
        VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        inputAssembly.pNext = VK_NULL_HANDLE;
        inputAssembly.topology = Utils::TopologyToVulkanTopology(m_Config.Topology);
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
        depthAndStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        depthAndStencilState.depthTestEnable = m_Config.DepthTesting;
        depthAndStencilState.depthWriteEnable = m_Config.DepthTesting;
        depthAndStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        depthAndStencilState.depthBoundsTestEnable = VK_FALSE;
        depthAndStencilState.minDepthBounds = 0.0f; // Optional
        depthAndStencilState.maxDepthBounds = 1.0f; // Optional
#if STENCIL_TEST
        depthAndStencilState.stencilTestEnable = VK_TRUE;
        depthAndStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
        depthAndStencilState.back.failOp = VK_STENCIL_OP_REPLACE;
        depthAndStencilState.back.depthFailOp = VK_STENCIL_OP_REPLACE;
        depthAndStencilState.back.passOp = VK_STENCIL_OP_REPLACE;
        depthAndStencilState.back.compareMask = 0xff;
        depthAndStencilState.back.writeMask = 0xff;
        depthAndStencilState.back.reference = 1;
        depthAndStencilState.front = depthAndStencilState.back;
#endif

        // ###############################################################################################################
        // ##################                              Color Blending                               ##################
        // ###############################################################################################################
        const auto& framebufferAttachments = m_Config.Renderpass->GetConfig().TargetFrameBuffer->GetConfig().Attachments;

        std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
        colorBlendAttachments.reserve(framebufferAttachments.size());

        //for (u32 i = 0; i < framebufferAttachments.size(); ++i)
        {
          //  if (framebufferAttachments[i].Type != FrameBuffer::AttachmentType_Color)
          //      continue;

            auto& colorBlendAttachment = colorBlendAttachments.emplace_back();
            colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            colorBlendAttachment.blendEnable = VK_TRUE;
            colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
            colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
            colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
            colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
            colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
            colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional
#if 1
            auto& selectionBlendAttachment = colorBlendAttachments.emplace_back();
            selectionBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
            selectionBlendAttachment.blendEnable = VK_FALSE;
#endif
        }

        VkPipelineColorBlendStateCreateInfo colorBlendState = {};
        colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        colorBlendState.attachmentCount = (u32)colorBlendAttachments.size();
        colorBlendState.pAttachments = colorBlendAttachments.data();
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

        auto& shaderPushContants = m_Config.Shader.As<VulkanShader>()->GetResources().PushConstantRanges;
        std::vector<VkPushConstantRange> pushConstantRanges;
        pushConstantRanges.reserve(shaderPushContants.size());

        for (auto& pushConstant : shaderPushContants)
        {
            auto& pushConstantRange = pushConstantRanges.emplace_back();
            pushConstantRange.size = pushConstant.Size;
            pushConstantRange.offset = pushConstant.Offset;
            pushConstantRange.stageFlags = Utils::ShaderStageToVulkanStage(pushConstant.Stage);
        }
        
        pipelineLayoutInfo.pushConstantRangeCount = (u32)pushConstantRanges.size(); // Optional
        pipelineLayoutInfo.pPushConstantRanges = pushConstantRanges.data(); // Optional

        TBO_VK_ASSERT(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_PipelineLayout));

        // ###############################################################################################################
        // ##################                              Pipeline Creation                            ##################
        // ###############################################################################################################
        VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
        pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCreateInfo.pNext = nullptr;
        pipelineCreateInfo.renderPass = m_Config.Renderpass.As<VulkanRenderPass>()->GetHandle();

        pipelineCreateInfo.pStages = shaderStagesCreateInfo;
        pipelineCreateInfo.stageCount = ShaderStage_Count;
        pipelineCreateInfo.pVertexInputState = &vertexInputState;
        pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
        pipelineCreateInfo.pViewportState = &viewportState;
        pipelineCreateInfo.pRasterizationState = &rasterizer;
        pipelineCreateInfo.pMultisampleState = &multisampleState;
        pipelineCreateInfo.pDepthStencilState = &depthAndStencilState;
        pipelineCreateInfo.pColorBlendState = &colorBlendState;
        pipelineCreateInfo.pDynamicState = &dynamicStatesInfo;
        pipelineCreateInfo.layout = m_PipelineLayout;
        pipelineCreateInfo.renderPass = m_Config.Renderpass.As<VulkanRenderPass>()->GetHandle();
        pipelineCreateInfo.subpass = 0;
        pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
        pipelineCreateInfo.basePipelineIndex = -1; // Optional

        TBO_VK_ASSERT(vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineCreateInfo, nullptr, &m_Pipeline));

        // Resource free queue
        RendererContext::SubmitResourceFree([device, pipeline = m_Pipeline, pipelineLayout = m_PipelineLayout]()
        {
            vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
            vkDestroyPipeline(device, pipeline, nullptr);
        });
    }
}
