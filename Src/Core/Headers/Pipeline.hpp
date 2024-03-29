#pragma once
#include "../Resources/Headers/Model.hpp"

#include <vulkan/vulkan.h>
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
    class Pipeline
    {
    public:
        struct PipelineConfig
        {
            VkPipelineShaderStageCreateInfo shaderStages[2] = {{},{}};
            VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
            std::vector<VkVertexInputBindingDescription> bindingDescriptions = Model::Vertex::GetBindingDescription();
            std::vector<VkVertexInputAttributeDescription> attributeDescriptions = Model::Vertex::GetAttributeDescription();
            VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
            VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
            VkPipelineViewportStateCreateInfo viewportState{};
            VkPipelineRasterizationStateCreateInfo rasterizer{};
            VkPipelineMultisampleStateCreateInfo multiSampling{};
            VkPipelineColorBlendAttachmentState colorBlendAttachment{};
            VkPipelineColorBlendStateCreateInfo colorBlending{};
            VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
            VkGraphicsPipelineCreateInfo pipelineInfo{};
            std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
            static PipelineConfig GetDefault(VkShaderModule vertexShaderModule, VkShaderModule fragShaderModule,
                VkPipelineLayout &pipelineLayout, VkRenderPass &renderPass, const VkPrimitiveTopology vertexTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            {
                PipelineConfig dConfig;
                dConfig.shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                dConfig.shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
                dConfig.shaderStages[0].module = vertexShaderModule;
                dConfig.shaderStages[0].pName = "main";

                dConfig.shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                dConfig.shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
                dConfig.shaderStages[1].module = fragShaderModule;
                dConfig.shaderStages[1].pName = "main";

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
                dConfig.inputAssembly.primitiveRestartEnable = VK_FALSE;

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
                dConfig.multiSampling.sampleShadingEnable = VK_FALSE;
                dConfig.multiSampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
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
                dConfig.pipelineInfo.stageCount = 2;
                dConfig.pipelineInfo.pStages = dConfig.shaderStages;

                dConfig.pipelineInfo.pVertexInputState = &dConfig.vertexInputInfo;
                dConfig.pipelineInfo.pInputAssemblyState = &dConfig.inputAssembly;
                dConfig.pipelineInfo.pViewportState = &dConfig.viewportState;
                dConfig.pipelineInfo.pRasterizationState = &dConfig.rasterizer;
                dConfig.pipelineInfo.pMultisampleState = &dConfig.multiSampling;
                dConfig.pipelineInfo.pDepthStencilState = &dConfig.depthStencilInfo;
                dConfig.pipelineInfo.pColorBlendState = &dConfig.colorBlending;
                dConfig.pipelineInfo.pDynamicState = &dConfig.dynamicStateInfo;

                dConfig.pipelineInfo.layout = pipelineLayout;

                dConfig.pipelineInfo.renderPass = renderPass;
                dConfig.pipelineInfo.subpass = 0;
                return dConfig;
            }
            static PipelineConfig GetForDepthTest(VkShaderModule vertexShaderModule,
                VkPipelineLayout &pipelineLayout, VkRenderPass &renderPass, const VkPrimitiveTopology vertexTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST)
            {
                PipelineConfig dConfig;
                dConfig.shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
                dConfig.shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
                dConfig.shaderStages[0].module = vertexShaderModule;
                dConfig.shaderStages[0].pName = "main";

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
                dConfig.inputAssembly.primitiveRestartEnable = VK_FALSE;

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
                dConfig.rasterizer.depthBiasEnable = VK_TRUE;
                dConfig.rasterizer.depthBiasConstantFactor = 0.0f; // Optional
                dConfig.rasterizer.depthBiasClamp = 0.0f;          // Optional
                dConfig.rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional

                dConfig.multiSampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
                dConfig.multiSampling.sampleShadingEnable = VK_FALSE;
                dConfig.multiSampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
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
                dConfig.colorBlending.attachmentCount = 0;
                dConfig.colorBlending.blendConstants[0] = 0.0f; // Optional
                dConfig.colorBlending.blendConstants[1] = 0.0f; // Optional
                dConfig.colorBlending.blendConstants[2] = 0.0f; // Optional
                dConfig.colorBlending.blendConstants[3] = 0.0f; // Optional

                dConfig.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
                dConfig.depthStencilInfo.depthTestEnable = VK_TRUE;
                dConfig.depthStencilInfo.depthWriteEnable = VK_TRUE;
                dConfig.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
                dConfig.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
                dConfig.depthStencilInfo.minDepthBounds = 0.0f; // Optional
                dConfig.depthStencilInfo.maxDepthBounds = 1.0f; // Optional
                dConfig.depthStencilInfo.stencilTestEnable = VK_FALSE;
                dConfig.depthStencilInfo.front = {};            // Optional
                dConfig.depthStencilInfo.back = {};             // Optional
            
                dConfig.pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
                dConfig.pipelineInfo.stageCount = 1;
                dConfig.pipelineInfo.pStages = dConfig.shaderStages;

                dConfig.pipelineInfo.pVertexInputState = &dConfig.vertexInputInfo;
                dConfig.pipelineInfo.pInputAssemblyState = &dConfig.inputAssembly;
                dConfig.pipelineInfo.pViewportState = &dConfig.viewportState;
                dConfig.pipelineInfo.pRasterizationState = &dConfig.rasterizer;
                dConfig.pipelineInfo.pMultisampleState = &dConfig.multiSampling;
                dConfig.pipelineInfo.pDepthStencilState = &dConfig.depthStencilInfo;
                dConfig.pipelineInfo.pColorBlendState = &dConfig.colorBlending;
                dConfig.pipelineInfo.pDynamicState = &dConfig.dynamicStateInfo;

                dConfig.pipelineInfo.layout = pipelineLayout;

                dConfig.pipelineInfo.renderPass = renderPass;
                dConfig.pipelineInfo.subpass = 0;
                return dConfig;
            }

        };
        Pipeline(Device& mDevice, const char* vertShaderFileName, const char* fragShaderFileName, VkRenderPass &mRenderPass, VkPipelineLayout &mPipelineLayoutconst, const VkPrimitiveTopology vertexTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        Pipeline(Device& mDevice, const std::vector<unsigned char>& vertShaderCode, const std::vector<unsigned char>& fragShaderCode, VkRenderPass &mRenderPass, VkPipelineLayout &mPipelineLayoutconst, const VkPrimitiveTopology vertexTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        Pipeline(Device& mDevice, const std::vector<unsigned char>& vertShaderCode, VkRenderPass &mRenderPass, VkPipelineLayout &mPipelineLayoutconst, const VkPrimitiveTopology vertexTopology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
        Pipeline(Device& mDevice, PipelineConfig pipelineConfig);
        Pipeline(Device& mDevice, const char* computeShaderFile, VkPipelineLayout &mPipelineLayout);
        Pipeline(Device& mDevice, const std::vector<unsigned char>& computeShaderCode, VkPipelineLayout &mPipelineLayout);

        ~Pipeline();

        void Bind(VkCommandBuffer commandBuffer);

        VkPipelineLayout& GetLayout()
        {
            return pipelineLayout;
        }

    private:
        Device& aDevice;
        VkRenderPass renderPass;

        VkPipelineLayout pipelineLayout;
        VkPipeline pipeline;//Graphics Pipeline
        void createGraphicsPipeline(const std::string &vertShaderFileName, const std::string &fragShaderFileName, const VkPrimitiveTopology vertexTopology);
        void createGraphicsPipeline(const std::vector<unsigned char>& vertShaderCode, const std::vector<unsigned char>& fragShaderCode, const VkPrimitiveTopology vertexTopology);
        void createGraphicsPipeline(const std::vector<unsigned char>& vertShaderCode, const VkPrimitiveTopology vertexTopology);
        void createGraphicsPipeline(PipelineConfig pipelineConfig);
        void createComputePipeline(const std::string &computeShaderFileName);
        void createComputePipeline(const std::vector<unsigned char>& computeShaderCode);

        VkShaderModule createShaderModule(const std::vector<unsigned char> &code);
    };
}
