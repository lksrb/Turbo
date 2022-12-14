#include "tbopch.h"

#include "VulkanShader.h"

#include "Turbo/Core/Platform.h"
#include "Turbo/Benchmark/ScopeTimer.h"

#include "Turbo/Renderer/RendererContext.h"

#include <fstream>
#include <sstream>
#include <filesystem>
#include <thread>

#include <vulkan/vulkan.h>
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_cross.hpp>

namespace Turbo
{
    namespace Utils
    {
        static ShaderStage ShaderTypeFromString(const std::string& type)
        {
            if (type == "vertex")
                return ShaderStage_Vertex;
            if (type == "fragment" || type == "pixel")
                return ShaderStage_Fragment;

            TBO_ENGINE_ASSERT(false, "Unknown shader!");

            return ShaderStage_Max;
        }

        static const char* ShaderTypeToString(ShaderStage stage)
        {
            switch (stage)
            {
                case ShaderStage_Vertex:    return "Vertex";
                case ShaderStage_Fragment:  return "Fragment";
            }

            TBO_ENGINE_ASSERT(false);
            return "";
        }

        static shaderc_shader_kind GLShaderStageToShaderC(ShaderStage stage)
        {
            switch (stage)
            {
                case ShaderStage_Vertex:   return shaderc_glsl_vertex_shader;
                case ShaderStage_Fragment: return shaderc_glsl_fragment_shader;
            }

            TBO_ENGINE_ASSERT(false);
            return (shaderc_shader_kind)0;
        }

        static const char* GLShaderStageCachedVulkanFileExtension(ShaderStage stage)
        {
            switch (stage)
            {
                case ShaderStage_Vertex:    return ".cached_vulkan.vert";
                case ShaderStage_Fragment:  return ".cached_vulkan.frag";
            }

            TBO_ENGINE_ASSERT(false);
            return "";
        }

        static uint32_t ShaderCBaseTypeSize(spirv_cross::SPIRType::BaseType type)
        {
            switch (type)
            {
                case spirv_cross::SPIRType::BaseType::Float:     return 4 * 1;
                case spirv_cross::SPIRType::BaseType::Int:       return 4 * 1;
                case spirv_cross::SPIRType::BaseType::UInt:      return 4 * 1;
            }

            TBO_ENGINE_ASSERT(false, "Invalid type!");
            return 0;
        }

        static VkFormat ShaderDataTypeToVulkanFormat(spirv_cross::SPIRType::BaseType type, uint32_t size)
        {

            if (type == spirv_cross::SPIRType::BaseType::Float)
            {
                switch (size)
                {
                    case 4 * 1: return VK_FORMAT_R32_SFLOAT;
                    case 4 * 2: return VK_FORMAT_R32G32_SFLOAT;
                    case 4 * 3: return VK_FORMAT_R32G32B32_SFLOAT;
                    case 4 * 4: return VK_FORMAT_R32G32B32A32_SFLOAT;
                }
            }
            else if (type == spirv_cross::SPIRType::BaseType::Int)
            {
                switch (size)
                {
                    case 4 * 1: return VK_FORMAT_R32_SINT;
                    case 4 * 2: return VK_FORMAT_R32G32_SINT;
                    case 4 * 3: return VK_FORMAT_R32G32B32_SINT;
                    case 4 * 4: return VK_FORMAT_R32G32B32A32_SINT;
                }
            }
            else if (type == spirv_cross::SPIRType::BaseType::UInt)
            {
                switch (size)
                {
                    case 4 * 1: return VK_FORMAT_R32_UINT;
                    case 4 * 2: return VK_FORMAT_R32G32_UINT;
                    case 4 * 3: return VK_FORMAT_R32G32B32_UINT;
                    case 4 * 4: return VK_FORMAT_R32G32B32A32_UINT;
                }
            }

            TBO_ENGINE_ASSERT(false, "Undefined format!");

            return VK_FORMAT_UNDEFINED;
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

    VulkanShader::VulkanShader(const Shader::Config& config)
        : Shader(config), m_DescriptorSetLayout(VK_NULL_HANDLE), m_DescriptorPool(VK_NULL_HANDLE), m_DescriptorSet(VK_NULL_HANDLE), m_Layout()
    {
        CheckIfUpToDate();
        ReadAndPreprocess();
        CompileOrGetCompiledShaders();
        Reflect();
        GenerateDescriptors();
        CreateModules();
    }

    VulkanShader::~VulkanShader()
    {
        VkDevice device = RendererContext::GetDevice();
        for (ShaderStage shaderStage = 0; shaderStage < ShaderStage_Max; ++shaderStage)
        {
            vkDestroyShaderModule(device, m_ShaderModules[shaderStage], nullptr);
        }
    }

    void VulkanShader::Reload()
    {
        TBO_ENGINE_ASSERT(false, "Not implemented yet.");
    }

    void VulkanShader::ReadAndPreprocess()
    {
        std::string sourceCode;
        std::ifstream in(m_Config.ShaderPath.c_str(), std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
        if (in)
        {
            in.seekg(0, std::ios::end);
            size_t size = in.tellg();
            if (size != -1)
            {
                sourceCode.resize(size);
                in.seekg(0, std::ios::beg);
                in.read(&sourceCode[0], size);
            }
            else
            {
                TBO_ENGINE_ERROR("Could not read from file '{0}'", m_Config.ShaderPath.c_str());
            }
        }
        else
        {
            TBO_ENGINE_ERROR("Could not open file '{0}'", m_Config.ShaderPath.c_str());
        }

        const char* typeToken = "#type";
        size_t typeTokenLength = strlen(typeToken);
        size_t pos = sourceCode.find(typeToken, 0); //Start of shader type declaration line
        while (pos != std::string::npos)
        {
            size_t eol = sourceCode.find_first_of("\r\n", pos); //End of shader type declaration line
            TBO_ENGINE_ASSERT(eol != std::string::npos, "Syntax error");
            size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
            std::string type = sourceCode.substr(begin, eol - begin);
            TBO_ENGINE_ASSERT(Utils::ShaderTypeFromString(type) != ShaderStage_Max, "Invalid shader type specified");

            size_t nextLinePos = sourceCode.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
            TBO_ENGINE_ASSERT(nextLinePos != std::string::npos, "Syntax error");
            pos = sourceCode.find(typeToken, nextLinePos); //Start of next shader type declaration line

            m_ShaderSources[Utils::ShaderTypeFromString(type)] = (pos == std::string::npos) ? sourceCode.substr(nextLinePos) : sourceCode.substr(nextLinePos, pos - nextLinePos);
        }
    }

    void VulkanShader::CheckIfUpToDate()
    {
        Filepath metaDataFilepath = m_Config.ShaderPath;
        metaDataFilepath.Append(".metadata");

        std::ifstream stream(metaDataFilepath.c_str(), std::ios_base::in);

        const char* token = "LastTimeWrite=";
        size_t tokenLength = strlen(token);
        size_t cachedLastTimeWrite = 0;
        std::string line;
        while (std::getline(stream, line))
        {
            if (line.find(token) != std::string::npos)
            {
                cachedLastTimeWrite = std::stoull(line.substr(tokenLength));
                break;
            }
        }

        stream.close();

        size_t lastTimeWrite = std::filesystem::last_write_time(m_Config.ShaderPath.c_str()).time_since_epoch().count();
        // Changed or newly created

        if (cachedLastTimeWrite != lastTimeWrite)
        {
            m_Compile = true;
            std::ofstream metaDataStream(metaDataFilepath.c_str(), std::ios_base::trunc);
            metaDataStream << "[MetaData]\n";
            metaDataStream << "LastTimeWrite=" << lastTimeWrite << "\n";
            metaDataStream.close();
        }
    }

    void VulkanShader::CompileOrGetCompiledShaders()
    {
        Filepath cachedPath = "assets\\Shaders\\cached";

        if (!std::filesystem::exists(cachedPath.c_str()))
        {
            // Cache folder does not exists, create one and compile shaders
            std::filesystem::create_directory(cachedPath.c_str());
            m_Compile = true;
        }

        if (m_Compile)
        {
            Benchmark::ScopeTimer timer("Shader compilation");

            TBO_ENGINE_WARN("Changes detected! Compiling...");

            // Clear shaders
            for (ShaderStage shaderStage = 0; shaderStage < ShaderStage_Max; ++shaderStage)
                m_CompiledShaders[shaderStage].clear();

            std::thread compileJobMaker[ShaderStage_Max];
            for (u32 shaderStage = 0; shaderStage < ShaderStage_Max; ++shaderStage)
            {
                compileJobMaker[shaderStage] = std::thread(&VulkanShader::CompileShader, this, shaderStage);
            }

            // Wait for all threads
            for (ShaderStage shaderStage = 0; shaderStage < ShaderStage_Max; ++shaderStage)
            {
                compileJobMaker[shaderStage].join();
            }

            TBO_ENGINE_INFO("Compilation complete!");
        }
        else // Retrieve shaders 
        {
            for (ShaderStage shaderStage = 0; shaderStage < ShaderStage_Max; ++shaderStage)
            {
                TBO_ENGINE_WARN("[{0}] Shader {1} is up-to-date!", Utils::ShaderTypeToString(shaderStage), m_Config.ShaderPath.c_str());

                Filepath shaderFilepath = cachedPath / m_Config.ShaderPath.Filename().Append(Utils::GLShaderStageCachedVulkanFileExtension(shaderStage));
                std::ifstream stream(shaderFilepath.c_str(), std::ios::in | std::ios::binary);

                if (stream)
                {
                    stream.seekg(0, std::ios::end);
                    size_t size = stream.tellg();
                    stream.seekg(0, std::ios::beg);

                    auto& data = m_CompiledShaders[shaderStage];
                    data.resize(size / sizeof(uint32_t));
                    stream.read((char*)data.data(), size);
                }
                else
                {
                    Benchmark::ScopeTimer timer("Shader compilation(not thread)"); // TODO: Rewrite this to be multithreaded
                    TBO_ENGINE_WARN("Changes detected! Compiling...");
                    CompileShader(shaderStage);
                    TBO_ENGINE_INFO("Compilation complete!");
                }
            }
        }
    }

    void VulkanShader::CompileShader(ShaderStage shaderStage)
    {
        Filepath cachedPath = "assets\\Shaders\\cached";

        TBO_ENGINE_WARN("[{0}]...", Utils::ShaderTypeToString(shaderStage));

        shaderc::Compiler compiler;
        shaderc::CompileOptions options;
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
        options.SetGenerateDebugInfo();
        constexpr bool optimize = true;

        if (optimize)
            options.SetOptimizationLevel(shaderc_optimization_level_performance);

        shaderc::SpvCompilationResult result = compiler.CompileGlslToSpv(m_ShaderSources[shaderStage], Utils::GLShaderStageToShaderC(shaderStage), m_Config.ShaderPath.c_str(), options);

        if (result.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            TBO_ENGINE_ERROR(result.GetErrorMessage());
            TBO_ENGINE_ASSERT(false, "");
        }

        m_CompiledShaders[shaderStage] = std::vector<uint32_t>(result.cbegin(), result.cend());

        Filepath shaderFilepath = cachedPath / m_Config.ShaderPath.Filename().Append(Utils::GLShaderStageCachedVulkanFileExtension(shaderStage));
        // This should overwrite the contents of previous cached shader
        std::ofstream out(shaderFilepath.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
        if (out.is_open())
        {
            auto& data = m_CompiledShaders[shaderStage];
            out.write((char*)data.data(), data.size() * sizeof(uint32_t));
            out.flush();
            out.close();
        }
    }

    void VulkanShader::Reflect()
    {
        Benchmark::ScopeTimer timer("Shader reflection");

        TBO_ENGINE_TRACE("-----------------------------------------------");
        for (ShaderStage shaderStage = 0; shaderStage < ShaderStage_Max; ++shaderStage)
        {
            ReflectStage(shaderStage);
        }
    }

    void VulkanShader::ReflectStage(ShaderStage shaderStage)
    {
        spirv_cross::Compiler compiler(m_CompiledShaders[shaderStage]);
        spirv_cross::ShaderResources& resources = compiler.get_shader_resources();

        TBO_ENGINE_INFO("GLSLShader::Reflect - {0} {1}", Utils::ShaderTypeToString(shaderStage), m_Config.ShaderPath.c_str());
        TBO_ENGINE_TRACE("    {0} uniform buffer(s)", resources.uniform_buffers.size());
        TBO_ENGINE_TRACE("    {0} resource(s)", resources.sampled_images.size());

        TBO_ENGINE_TRACE("Attributes: ");

        // Attribute Inputs
        // Attribute Inputs
        // Attribute Inputs
        // Compare lambda 
        auto asceningOrderLambda = [&compiler](spirv_cross::Resource& a, spirv_cross::Resource& b)
        {
            uint32_t location1 = compiler.get_decoration(a.id, spv::DecorationLocation);
            uint32_t location2 = compiler.get_decoration(b.id, spv::DecorationLocation);

            return location1 < location2;
        };

        // Sort attributes by location
        // NOTE: For some reason stage inputs are not in the order as they are written in shader.
        std::sort(resources.stage_inputs.begin(), resources.stage_inputs.end(), asceningOrderLambda);

        // Local variable for offsetting attributes in the buffer
        uint32_t attributeOffset = 0;
        for (const auto& resource : resources.stage_inputs)
        {
            const auto& bufferType = compiler.get_type(resource.base_type_id);
            uint32_t location = compiler.get_decoration(resource.id, spv::DecorationLocation);
            uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);

            // Query for type of the attribute
            const auto& type = compiler.get_type_from_variable(resource.id);
            // Calculating actual size of the attribute with base type of @type and 
            uint32_t actualDataTypeSize = type.basetype == spirv_cross::SPIRType::Struct ? 0 : Utils::ShaderCBaseTypeSize(type.basetype) * type.vecsize;

            // Struct
            for (auto& member : type.member_types)
            {
                actualDataTypeSize += Utils::ShaderCBaseTypeSize(compiler.get_type(member).basetype) * type.vecsize;
            }

            // Printing out useful informations(debug only)
            TBO_ENGINE_ERROR("  {0}", compiler.get_name(resource.id));
            TBO_ENGINE_TRACE("    Location = {0}", location);
            TBO_ENGINE_TRACE("    Size = {0} byte(s)", actualDataTypeSize);

            // Building shader vertex buffer layout
            if (shaderStage == ShaderStage_Vertex)
            {
                VertexInputAttribute& attributeDesc = m_Layout.Descriptions.emplace_back();
                attributeDesc.Binding = binding; // Not sure
                attributeDesc.Location = location;
                attributeDesc.Offset = attributeOffset;
                attributeDesc.Format = Utils::ShaderDataTypeToVulkanFormat(type.basetype, actualDataTypeSize);
                attributeOffset += actualDataTypeSize;
                m_Layout.Stride += actualDataTypeSize;
            }
        }

        TBO_ENGINE_TRACE("-----------------------------------------------");

        // Create descriptors

        // Resources
        {
            // Uniform buffers 
            for (const auto& uniformBuffer : resources.uniform_buffers)
            {
                const auto& bufferType = compiler.get_type(uniformBuffer.base_type_id);

                // Create uniform buffer for each stage
                auto& rUniformBuffer = m_Resources.UniformBuffers.emplace_back();
                rUniformBuffer.Binding = compiler.get_decoration(uniformBuffer.id, spv::DecorationBinding);
                rUniformBuffer.Size = (uint32_t)compiler.get_declared_struct_size(bufferType);
                rUniformBuffer.Name = compiler.get_name(uniformBuffer.id);
                rUniformBuffer.Stage = shaderStage;
            }

            // Texture samplers
            for (const auto& textureSampler : resources.sampled_images)
            {
                const auto& bufferType = compiler.get_type(textureSampler.type_id); // TODO: Rewrite this to handle more situation, namely more "sampled_images"

                m_Resources.TextureSamplerArray.Binding = compiler.get_decoration(textureSampler.id, spv::DecorationBinding);
                m_Resources.TextureSamplerArray.Size = bufferType.array[0];
                m_Resources.TextureSamplerArray.Name = compiler.get_name(textureSampler.id);
            }
        }
    }

    void VulkanShader::CreateModules()
    {
        VkDevice device = RendererContext::GetDevice();

        for (ShaderStage shaderStage = 0; shaderStage < ShaderStage_Max; ++shaderStage)
        {
            VkShaderModuleCreateInfo moduleCreateInfo = {};
            moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
            moduleCreateInfo.pNext = nullptr;
            moduleCreateInfo.codeSize = m_CompiledShaders[shaderStage].size() * sizeof(u32);
            moduleCreateInfo.pCode = m_CompiledShaders[shaderStage].data();

            TBO_VK_ASSERT(vkCreateShaderModule(device, &moduleCreateInfo, nullptr, &m_ShaderModules[shaderStage]));
        }
    }

    void VulkanShader::GenerateDescriptors()
    {
        VkDevice device = RendererContext::GetDevice();

        std::vector<VkDescriptorSetLayoutBinding> descriptorSetLayoutBindings;

        // Uniform buffers
        for (auto& uniformBufferResource : m_Resources.UniformBuffers)
        {
            auto& descriptorBinding = descriptorSetLayoutBindings.emplace_back();
            descriptorBinding = {};
            descriptorBinding.binding = uniformBufferResource.Binding;
            descriptorBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorBinding.stageFlags = Utils::TboStageToVkStage(uniformBufferResource.Stage);
            descriptorBinding.descriptorCount = 1;
            descriptorBinding.pImmutableSamplers = nullptr; // Optional
        }

        // Texture samplers
        if (m_Resources.TextureSamplerArray.Size)
        {
            auto& descriptorBinding = descriptorSetLayoutBindings.emplace_back();
            descriptorBinding = {};
            descriptorBinding.binding = m_Resources.TextureSamplerArray.Binding;
            descriptorBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
            descriptorBinding.descriptorCount = m_Resources.TextureSamplerArray.Size;
            descriptorBinding.pImmutableSamplers = nullptr; // Optional
        }

        // Pool
        {
            // Pool size
            std::vector<VkDescriptorPoolSize> poolSizes;

            if (m_Resources.UniformBuffers.size() > 0)
                poolSizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<u32>(m_Resources.UniformBuffers.size()) });
            if (m_Resources.TextureSamplerArray.Size > 0)
                poolSizes.push_back({ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, m_Resources.TextureSamplerArray.Size });

            if (poolSizes.size() == 0)
                return; // Do not create any descriptor sets or layouts

            VkDescriptorPoolCreateInfo poolCreateInfo = {};
            poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
            poolCreateInfo.poolSizeCount = (uint32_t)poolSizes.size();
            poolCreateInfo.pPoolSizes = poolSizes.data();
            poolCreateInfo.maxSets = 1;

            TBO_VK_ASSERT(vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, &m_DescriptorPool));
        }

        // Descriptor set layout
        {
            VkDescriptorSetLayoutCreateInfo layoutInfo = {};
            layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
            layoutInfo.bindingCount = static_cast<u32>(descriptorSetLayoutBindings.size());
            layoutInfo.pBindings = descriptorSetLayoutBindings.data();

            TBO_VK_ASSERT(vkCreateDescriptorSetLayout(device,
                &layoutInfo, nullptr, &m_DescriptorSetLayout));
        }

        // Descriptor set
        {
            VkDescriptorSetAllocateInfo allocInfo{};
            allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
            allocInfo.descriptorPool = m_DescriptorPool;
            allocInfo.descriptorSetCount = 1;
            allocInfo.pSetLayouts = &m_DescriptorSetLayout;

            TBO_VK_ASSERT(vkAllocateDescriptorSets(device, &allocInfo, &m_DescriptorSet));
        }

        // Resource free queue
        auto& resourceFreeQueue = RendererContext::GetResourceQueue();

        resourceFreeQueue.Submit(DESCRIPTOR_POOL, [device, descriptorPool = m_DescriptorPool]()
        {
            vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        });

        resourceFreeQueue.Submit(DESCRIPTOR_SET_LAYOUT, [device, descriptorSetLayout = m_DescriptorSetLayout]()
        {
            vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
        });
    }
}
