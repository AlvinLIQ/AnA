#pragma once
#include "Device.hpp"

#include <initializer_list>
#include <volk.h>
#include <vector>
#include <string>

#define TRIANGLE_LIST_PIPELINE 0
#define LINE_LIST_PIPELINE 1
#define POINT_LIST_PIPELINE 2
#define COMPUTE_PIPELINE 3
#define SHADOW_MAPPING_PIPELINE 4

#define PIPELINE_COUNT 5

namespace AnA
{
    struct ShaderInfo
    {
        std::vector<unsigned char> codes;
        VkPipelineCreateFlags flag;
        VkShaderStageFlagBits stage;
        bool hasGBuffer{false};
        std::initializer_list<int> constants;
    };
    class Pipeline
    {
    public:
        struct PipelineConfig
        {
            VkPipelineShaderStageCreateInfo shaderStages[3] = {{},{}, {}};
            VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
            std::vector<VkVertexInputBindingDescription> bindingDescriptions{};// = Model::Vertex::GetBindingDescription();
            std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};// = Model::Vertex::GetAttributeDescription();
            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
            VkPipelineViewportStateCreateInfo viewportState{};
            VkPipelineRasterizationStateCreateInfo rasterizer{};
            VkPipelineMultisampleStateCreateInfo multiSampling{};
            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            const VkPipelineColorBlendAttachmentState noBlendAttachment
                                {.blendEnable = VK_FALSE,
                                    .srcColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                                    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
                                    .colorBlendOp = VK_BLEND_OP_ADD,
                                    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                                    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
                                    .alphaBlendOp = VK_BLEND_OP_ADD,
                                    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                    VK_COLOR_COMPONENT_G_BIT |
                                    VK_COLOR_COMPONENT_B_BIT |
                                    VK_COLOR_COMPONENT_A_BIT};
            VkPipelineColorBlendAttachmentState colorBlendAttachments[3] = { noBlendAttachment, noBlendAttachment, noBlendAttachment };
            VkPipelineColorBlendStateCreateInfo colorBlending{};
            VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
            VkFormat colorAttachmentFormats[3]{};
            VkFormat depthAttachmentFormat{};
            VkGraphicsPipelineCreateInfo pipelineInfo{};
            VkPipelineRenderingCreateInfoKHR pipelineRenderingInfo{};
            bool hasComputeShader = false;
            bool hasMeshShader = false;
            std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR, VK_DYNAMIC_STATE_POLYGON_MODE_EXT, VK_DYNAMIC_STATE_PRIMITIVE_TOPOLOGY };
            std::vector<VkSpecializationInfo> specializationInfos{};
            std::vector<std::vector<VkSpecializationMapEntry>> specializationMapEntries{};
            VkPipelineCreateFlags2CreateInfo flags2;

            static PipelineConfig GetForDynamicRendering(Device* aDevice, std::vector<ShaderInfo> shaderInfos,
                VkPipelineLayout &pipelineLayout, VkFormat colorFormat, VkFormat depthFormat, VkSampleCountFlagBits msaaSamplers,
                VkShaderDescriptorSetAndBindingMappingInfoEXT* pDescriptorSetMappingInfo,
                const VkPrimitiveTopology vertexTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            {
                PipelineConfig dConfig;
                assert(shaderInfos.size() <= 3);
                bool isMeshShader = false, hasFragmentShader = false, hasGBuffer = false;
                dConfig.pipelineInfo.layout = pipelineLayout;
                if (aDevice->DescriptorBufferSupport())
                    dConfig.pipelineInfo.flags = VK_PIPELINE_CREATE_DESCRIPTOR_BUFFER_BIT_EXT;
                dConfig.specializationInfos.resize(shaderInfos.size());
                dConfig.specializationMapEntries.resize(shaderInfos.size());
                for (size_t i = 0; i < shaderInfos.size(); i++)
                {
                    dConfig.shaderStages[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                    dConfig.shaderStages[i].flags = shaderInfos[i].flag;
                    dConfig.shaderStages[i].stage = shaderInfos[i].stage;
                    dConfig.shaderStages[i].module = aDevice->CreateShaderModule(shaderInfos[i].codes);
                    dConfig.shaderStages[i].pName = "main";

                    if (shaderInfos[i].constants.size())
                    {
                        dConfig.specializationMapEntries[i].resize(shaderInfos[i].constants.size());
                        for (size_t j = 0; j < dConfig.specializationMapEntries[i].size(); j++)
                        {
                            auto& entry = dConfig.specializationMapEntries[i][j];
                            entry.constantID = uint32_t(j);
                            entry.offset = uint32_t(j * sizeof(int));
                            entry.size = sizeof(int);
                        }
                        dConfig.specializationInfos[i].dataSize = shaderInfos[i].constants.size() * sizeof(int);
                        dConfig.specializationInfos[i].pData = shaderInfos[i].constants.begin();
                        dConfig.specializationInfos[i].mapEntryCount = uint32_t(dConfig.specializationMapEntries[i].size());
                        dConfig.specializationInfos[i].pMapEntries = dConfig.specializationMapEntries[i].data();
                        dConfig.shaderStages[i].pSpecializationInfo = &dConfig.specializationInfos[i];
                    }
                    if (aDevice->DescriptorHeapSupport() && pDescriptorSetMappingInfo)
                    {
                        dConfig.shaderStages[i].pNext = pDescriptorSetMappingInfo;
                    }
                    dConfig.hasMeshShader = isMeshShader = isMeshShader || (shaderInfos[i].stage &
                        (VK_SHADER_STAGE_MESH_BIT_EXT | VK_SHADER_STAGE_TASK_BIT_EXT |
                        VK_SHADER_STAGE_MESH_BIT_NV | VK_SHADER_STAGE_TASK_BIT_NV));
                    hasFragmentShader = hasFragmentShader || (shaderInfos[i].stage & VK_SHADER_STAGE_FRAGMENT_BIT);
                    hasGBuffer = hasGBuffer || shaderInfos[i].hasGBuffer;
                    dConfig.hasComputeShader = dConfig.hasComputeShader || (shaderInfos[i].stage & VK_SHADER_STAGE_COMPUTE_BIT);
                }
                if (dConfig.hasComputeShader)
                    return dConfig;
                if (isMeshShader)
                    dConfig.dynamicStates.pop_back();
                if (!hasFragmentShader)
                    dConfig.dynamicStates.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);

                dConfig.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
                dConfig.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dConfig.dynamicStates.size());
                dConfig.dynamicStateInfo.pDynamicStates = dConfig.dynamicStates.data();

                dConfig.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
                dConfig.vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(dConfig.bindingDescriptions.size());
                dConfig.vertexInputInfo.pVertexBindingDescriptions = dConfig.bindingDescriptions.data();
                dConfig.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(dConfig.attributeDescriptions.size());
                dConfig.vertexInputInfo.pVertexAttributeDescriptions = dConfig.attributeDescriptions.data();

                dConfig.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
                dConfig.inputAssembly.topology = vertexTopology;
                dConfig.inputAssembly.primitiveRestartEnable = VK_TRUE;

                dConfig.viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
                dConfig.viewportState.viewportCount = 1;
                // dConfig.viewportState.pViewports = &viewport;
                dConfig.viewportState.scissorCount = 1;
                // dConfig.viewportState.pScissors = &scissor;

                dConfig.rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
                dConfig.rasterizer.depthClampEnable = VK_FALSE;
                dConfig.rasterizer.rasterizerDiscardEnable = VK_FALSE;
                dConfig.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
                dConfig.rasterizer.lineWidth = 1.0f;
                dConfig.rasterizer.cullMode = VK_CULL_MODE_NONE;
                dConfig.rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
                dConfig.rasterizer.depthBiasEnable = VK_FALSE;
                dConfig.rasterizer.depthBiasConstantFactor = 0.0f; // Optional
                dConfig.rasterizer.depthBiasClamp = 0.0f;          // Optional
                dConfig.rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional

                dConfig.multiSampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                dConfig.multiSampling.sampleShadingEnable = VK_TRUE;
                dConfig.multiSampling.rasterizationSamples = msaaSamplers;
                dConfig.multiSampling.minSampleShading = 1.0f;          // Optional
                dConfig.multiSampling.pSampleMask = nullptr;            // Optional
                dConfig.multiSampling.alphaToCoverageEnable = VK_FALSE; // Optional
                dConfig.multiSampling.alphaToOneEnable = VK_FALSE;      // Optional

                dConfig.colorBlendAttachment.colorWriteMask =
                    VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
                dConfig.colorBlendAttachment.blendEnable = VK_TRUE;
                dConfig.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
                dConfig.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                dConfig.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
                dConfig.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
                dConfig.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
                dConfig.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

                dConfig.colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
                dConfig.colorBlending.logicOpEnable = VK_FALSE;
                dConfig.colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
                dConfig.colorBlending.attachmentCount = 1;
                dConfig.colorBlending.pAttachments = &dConfig.colorBlendAttachment;
                dConfig.colorBlending.blendConstants[0] = 0.0f; // Optional
                dConfig.colorBlending.blendConstants[1] = 0.0f; // Optional
                dConfig.colorBlending.blendConstants[2] = 0.0f; // Optional
                dConfig.colorBlending.blendConstants[3] = 0.0f; // Optional

                dConfig.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                dConfig.depthStencilInfo.depthTestEnable = VK_TRUE;
                dConfig.depthStencilInfo.depthWriteEnable = VK_TRUE;
                dConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
                dConfig.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
                dConfig.depthStencilInfo.minDepthBounds = 0.0f; // Optional
                dConfig.depthStencilInfo.maxDepthBounds = 1.0f; // Optional
                dConfig.depthStencilInfo.stencilTestEnable = VK_FALSE;
                dConfig.depthStencilInfo.front = {};            // Optional
                dConfig.depthStencilInfo.back = {};             // Optional

                dConfig.pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                dConfig.pipelineInfo.stageCount = uint32_t(shaderInfos.size());
                dConfig.pipelineInfo.pStages = dConfig.shaderStages;

                dConfig.pipelineInfo.pVertexInputState = &dConfig.vertexInputInfo;
                dConfig.pipelineInfo.pInputAssemblyState = &dConfig.inputAssembly;
                dConfig.pipelineInfo.pViewportState = &dConfig.viewportState;
                dConfig.pipelineInfo.pRasterizationState = &dConfig.rasterizer;
                dConfig.pipelineInfo.pMultisampleState = &dConfig.multiSampling;
                dConfig.pipelineInfo.pDepthStencilState = &dConfig.depthStencilInfo;
                dConfig.pipelineInfo.pColorBlendState = &dConfig.colorBlending;
                dConfig.pipelineInfo.pDynamicState = &dConfig.dynamicStateInfo;

                dConfig.pipelineInfo.pNext = &dConfig.pipelineRenderingInfo;

                dConfig.pipelineRenderingInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;

                dConfig.depthAttachmentFormat = depthFormat;
                if (hasGBuffer)
                {
                    dConfig.colorAttachmentFormats[0] = VK_FORMAT_R16G16B16A16_SFLOAT;
                    dConfig.colorAttachmentFormats[1] = VK_FORMAT_R16G16B16A16_SFLOAT;
                    dConfig.colorAttachmentFormats[2] = VK_FORMAT_R8G8B8A8_UNORM;
                    dConfig.pipelineRenderingInfo.colorAttachmentCount = 3;

                    dConfig.colorBlending.attachmentCount = 3;
                    dConfig.colorBlending.pAttachments = dConfig.colorBlendAttachments;

                    dConfig.multiSampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
                }
                else
                {
                    dConfig.colorAttachmentFormats[0] = colorFormat;
                    dConfig.pipelineRenderingInfo.colorAttachmentCount = 1;
                }
                dConfig.pipelineRenderingInfo.pColorAttachmentFormats = dConfig.colorAttachmentFormats;
                dConfig.pipelineRenderingInfo.depthAttachmentFormat = dConfig.depthAttachmentFormat;
                dConfig.pipelineRenderingInfo.stencilAttachmentFormat = VK_FORMAT_UNDEFINED;

                if (aDevice->DescriptorHeapSupport() && pDescriptorSetMappingInfo)
                {
                    dConfig.flags2 = {
                        .sType = VK_STRUCTURE_TYPE_PIPELINE_CREATE_FLAGS_2_CREATE_INFO,
                        .pNext = nullptr,
                        .flags = VK_PIPELINE_CREATE_2_DESCRIPTOR_HEAP_BIT_EXT,
                    };
                    dConfig.pipelineRenderingInfo.pNext = &dConfig.flags2;
                }

                dConfig.pipelineInfo.subpass = 0;
                return dConfig;
            }
            static void CleanupPipelineConfig(Device* device, PipelineConfig& config)
            {
                for (int i = 0 ; i < 3; i++)
                {
                    if (!config.shaderStages[i].module)
                        break;
                    vkDestroyShaderModule(device->GetLogicalDevice(), config.shaderStages[i].module, VK_NULL_HANDLE);
                    config.shaderStages[i].module = VK_NULL_HANDLE;
                }
            }
        };
        Pipeline()
        {

        }
        Pipeline(Device* mDevice, PipelineConfig& pipelineConfig);

        ~Pipeline();

        Pipeline(Pipeline& newPipeline)
        {
            aDevice = newPipeline.aDevice;
            pipeline = newPipeline.pipeline;
            pipelineLayout = newPipeline.pipelineLayout;
            renderPass = newPipeline.renderPass;
            bindPoint = newPipeline.bindPoint;
            newPipeline.pipeline = VK_NULL_HANDLE;
            newPipeline.pipelineLayout = VK_NULL_HANDLE;
            newPipeline.renderPass = VK_NULL_HANDLE;
        }

        Pipeline& operator=(Pipeline&& newPipeline)
        {
            if (&newPipeline == this)
                return* this;
            aDevice = newPipeline.aDevice;
            pipeline = newPipeline.pipeline;
            pipelineLayout = newPipeline.pipelineLayout;
            renderPass = newPipeline.renderPass;
            bindPoint = newPipeline.bindPoint;
            newPipeline.pipeline = VK_NULL_HANDLE;
            newPipeline.pipelineLayout = VK_NULL_HANDLE;
            newPipeline.renderPass = VK_NULL_HANDLE;

            return *this;
        }

        Pipeline& operator=(Pipeline& newPipeline)
        {
            if (&newPipeline == this)
                return* this;
            aDevice = newPipeline.aDevice;
            pipeline = newPipeline.pipeline;
            pipelineLayout = newPipeline.pipelineLayout;
            renderPass = newPipeline.renderPass;
            bindPoint = newPipeline.bindPoint;
            newPipeline.pipeline = VK_NULL_HANDLE;
            newPipeline.pipelineLayout = VK_NULL_HANDLE;
            newPipeline.renderPass = VK_NULL_HANDLE;

            return *this;
        }

        void Bind(VkCommandBuffer commandBuffer) const;
        void Cleanup();

        VkPipelineLayout GetLayout() const
        {
            return pipelineLayout;
        }
    private:
        Device* aDevice;
        VkRenderPass renderPass{VK_NULL_HANDLE};

        VkPipelineLayout pipelineLayout{VK_NULL_HANDLE};
        VkPipeline pipeline{VK_NULL_HANDLE};//Graphics Pipeline
        void createGraphicsPipeline(PipelineConfig& pipelineConfig);
        void createComputePipeline(const std::string &computeShaderFileName);
        void createComputePipeline(const std::vector<unsigned char>& computeShaderCode);
        void createComputePipeline(PipelineConfig& pipelineConfig);

        VkPipelineBindPoint bindPoint;
    };
}
