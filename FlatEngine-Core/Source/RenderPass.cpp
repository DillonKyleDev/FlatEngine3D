#include "RenderPass.h"
#include "Helper.h"
#include "VulkanManager.h"
#include "GameObject.h"
#include "Transform.h"
#include "Vector3.h"

#include <glm.hpp>
#include <gtc/type_ptr.hpp>

#include <stdexcept>
#include <chrono>


namespace FlatEngine
{
	RenderPass::RenderPass()
	{
		m_renderPass = VK_NULL_HANDLE;
        m_framebuffers = std::vector<VkFramebuffer>();
        m_imageViews = std::vector<VkImageView>(); 
        m_commandBuffers = std::vector<VkCommandBuffer>();           
        m_renderPassAttachments = std::vector<VkAttachmentDescription>();
        m_renderPassAttachmentRefs = std::vector<VkAttachmentReference>();
        m_subpasses = std::vector<VkSubpassDescription>();
        m_subpassDependencies = std::vector<VkSubpassDependency>();
        m_b_defaultRenderPassConfig = false;
        // antialiasing
        m_msaaSamples = VK_SAMPLE_COUNT_1_BIT;
        m_colorImage = VK_NULL_HANDLE;
        m_colorImageMemory = VK_NULL_HANDLE;
        m_colorImageView = VK_NULL_HANDLE;
        m_b_msaaEnabled = false;
        // depth testing
        m_depthImage = VK_NULL_HANDLE;
        m_depthImageMemory = VK_NULL_HANDLE;
        m_depthImageView = VK_NULL_HANDLE;
        m_b_depthBuffersEnabled = false;
        // handles
        m_instanceHandle = VK_NULL_HANDLE;
        m_winSystem = nullptr;
        m_physicalDeviceHandle = nullptr;
        m_deviceHandle = nullptr;
	}

	RenderPass::~RenderPass()
	{
	}

    void RenderPass::Cleanup(LogicalDevice& logicalDevice)
    {
        vkDestroyRenderPass(logicalDevice.GetDevice(), m_renderPass, nullptr);
    }


    void RenderPass::SetHandles(VkInstance instance, WinSys& winSystem, PhysicalDevice& physicalDevice, LogicalDevice& logicalDevice)
    {
        m_instanceHandle = instance;
        m_winSystem = &winSystem;
        m_physicalDeviceHandle = &physicalDevice;
        m_deviceHandle = &logicalDevice;
    }

    void RenderPass::Init(VkCommandPool commandPool)
    {      
        if (m_b_msaaEnabled)
        {
            CreateColorResources();
        }
        if (m_b_depthBuffersEnabled)
        {
            CreateDepthResources();
        }
        CreateRenderPass();
        CreateFrameBuffers();
        CreateCommandBuffers(commandPool);
    }

    void RenderPass::SetDefaultRenderPassConfig()
    {
        m_b_defaultRenderPassConfig = true;

        EnableDepthBuffering();
        EnableMsaa();

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_winSystem->GetImageFormat();
        colorAttachment.samples = m_msaaSamples;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0; // (layout = 0) in shader
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        AddRenderPassAttachment(colorAttachment, colorAttachmentRef);

        // Depth attachment
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = Helper::FindDepthFormat(m_physicalDeviceHandle->GetDevice());
        depthAttachment.samples = m_msaaSamples;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        AddRenderPassAttachment(depthAttachment, depthAttachmentRef);

        // Resolve attachment for MSAA
        VkAttachmentDescription colorAttachmentResolve{};
        colorAttachmentResolve.format = m_winSystem->GetImageFormat();
        colorAttachmentResolve.samples = m_msaaSamples;
        colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        VkAttachmentReference colorAttachmentResolveRef{};
        colorAttachmentResolveRef.attachment = 2;
        colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        AddRenderPassAttachment(colorAttachmentResolve, colorAttachmentResolveRef);

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        // The index of the attachment in this array is directly referenced from the fragment shader with the layout(location = 0) out vec4 outColor directive!
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &m_renderPassAttachmentRefs[0];
        subpass.pDepthStencilAttachment = &m_renderPassAttachmentRefs[1];
        subpass.pResolveAttachments = &m_renderPassAttachmentRefs[2];
        AddSubpass(subpass);        

        // Create Dependency ( not used in default config )
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        // AddSubpassDependency(dependency);
    }

    void RenderPass::AddRenderPassAttachment(VkAttachmentDescription description, VkAttachmentReference reference)
    {
        m_renderPassAttachments.push_back(description);
        m_renderPassAttachmentRefs.push_back(reference);        
    }

    std::vector<VkAttachmentReference>& RenderPass::GetAttachmentRefs()
    {
        return m_renderPassAttachmentRefs;
    }

    void RenderPass::AddSubpass(VkSubpassDescription subpass)
    {
        m_subpasses.push_back(subpass);
    }

    void RenderPass::AddSubpassDependency(VkSubpassDependency dependency)
    {
        m_subpassDependencies.push_back(dependency);
    }

    void RenderPass::CreateRenderPass()
    {
        // More info here - https://vulkan-tutorial.com/en/Drawing_a_triangle/Graphics_pipeline_basics/Render_passes

        VkRenderPassCreateInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        renderPassInfo.attachmentCount = (uint32_t)m_renderPassAttachments.size();
        renderPassInfo.pAttachments = m_renderPassAttachments.data();
        renderPassInfo.subpassCount = (uint32_t)m_subpasses.size();
        renderPassInfo.pSubpasses = m_subpasses.data();
        renderPassInfo.dependencyCount = (uint32_t)m_subpassDependencies.size();
        renderPassInfo.pDependencies = m_subpassDependencies.data();

        if (vkCreateRenderPass(m_deviceHandle->GetDevice(), &renderPassInfo, nullptr, &m_renderPass) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create render pass!");
        }
    }

    void RenderPass::DestroyRenderPass()
    {      
        m_renderPassAttachments.clear();
        m_renderPassAttachmentRefs.clear();
        m_subpasses.clear();
        m_subpassDependencies.clear();
        vkDestroyRenderPass(m_deviceHandle->GetDevice(), m_renderPass, nullptr);
    }

    VkRenderPass& RenderPass::GetRenderPass()
    {
        return m_renderPass;
    }

    void RenderPass::ConfigureFrameBufferImageViews(std::vector<VkImageView>& imageViews)
    {
        m_imageViews = imageViews;        
    }

    void RenderPass::CreateFrameBuffers()
    {
        // More info here - https://vulkan-tutorial.com/en/Drawing_a_triangle/Drawing/Framebuffers
      
        m_framebuffers.resize(m_winSystem->GetSwapChainImageViews().size());

        for (size_t i = 0; i < m_winSystem->GetSwapChainImageViews().size(); i++)
        {
            std::vector<VkImageView> attachments = {};
            if (m_b_msaaEnabled)
            {
                attachments.push_back(m_colorImageView);
            }
            if (m_b_depthBuffersEnabled)
            {
                attachments.push_back(m_depthImageView);
            }
            attachments.push_back(m_imageViews[i]);

            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass = m_renderPass;
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
            framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = m_winSystem->GetExtent().width;
            framebufferInfo.height = m_winSystem->GetExtent().height;
            framebufferInfo.layers = 1;

            if (vkCreateFramebuffer(m_deviceHandle->GetDevice(), &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create framebuffer!");
            }
        }
    }

    void RenderPass::DestroyFrameBuffers()
    {
        for (VkFramebuffer framebuffer : m_framebuffers)
        {
            vkDestroyFramebuffer(m_deviceHandle->GetDevice(), framebuffer, nullptr);
        }
    }

    void RenderPass::RecreateFrameBuffers()
    {
        DestroyFrameBuffers();
        //DestroyRenderPass();

        if (m_b_msaaEnabled)
        {
            DestroyColorResources();
            CreateColorResources();
        }
        if (m_b_depthBuffersEnabled)
        {
            DestroyDepthResources();
            CreateDepthResources();
        }
       
        //CreateRenderPass();
        CreateFrameBuffers();        
    }

    std::vector<VkFramebuffer>& RenderPass::GetFrameBuffers()
    {
        return m_framebuffers;
    }

    void RenderPass::EnableMsaa()
    {
        CreateColorResources();
        m_b_msaaEnabled = true;
    }

    void RenderPass::EnableDepthBuffering()
    {
        DestroyColorResources();
        m_b_depthBuffersEnabled = true;
    }

    void RenderPass::CreateColorResources()
    {
        // Refer to - https://vulkan-tutorial.com/Multisampling
        VkFormat colorFormat = m_winSystem->GetImageFormat();

        WinSys::CreateImage(m_winSystem->GetExtent().width, m_winSystem->GetExtent().height, 1, m_msaaSamples, colorFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_colorImage, m_colorImageMemory, *m_physicalDeviceHandle, *m_deviceHandle);
        WinSys::CreateImageView(m_colorImageView, m_colorImage, colorFormat, VK_IMAGE_ASPECT_COLOR_BIT, 1, *m_deviceHandle);
    }

    void RenderPass::DestroyColorResources()
    {
        if (m_deviceHandle != nullptr)
        {
            vkDestroyImageView(m_deviceHandle->GetDevice(), m_colorImageView, nullptr);
            vkDestroyImage(m_deviceHandle->GetDevice(), m_colorImage, nullptr);
            vkFreeMemory(m_deviceHandle->GetDevice(), m_colorImageMemory, nullptr);
        }
    }

    VkSampleCountFlagBits RenderPass::GetMsaa()
    {
        return m_msaaSamples;
    }

    void RenderPass::CreateDepthResources()
    {
        // Refer to - https://vulkan-tutorial.com/en/Depth_buffering
        // and for msaa - https://vulkan-tutorial.com/Multisampling

        VkFormat depthFormat = Helper::FindDepthFormat(m_physicalDeviceHandle->GetDevice());
        uint32_t singleMipLevel = 1;

        WinSys::CreateImage(m_winSystem->GetExtent().width, m_winSystem->GetExtent().height, 1, m_msaaSamples, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImage, m_depthImageMemory, *m_physicalDeviceHandle, *m_deviceHandle);
        WinSys::CreateImageView(m_depthImageView, m_depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT, singleMipLevel, *m_deviceHandle);
    }

    void RenderPass::DestroyDepthResources()
    {
        if (m_deviceHandle != nullptr)
        {
            vkDestroyImageView(m_deviceHandle->GetDevice(), m_depthImageView, nullptr);
            vkDestroyImage(m_deviceHandle->GetDevice(), m_depthImage, nullptr);
            vkFreeMemory(m_deviceHandle->GetDevice(), m_depthImageMemory, nullptr);
        }
    }

    void RenderPass::BeginRenderPass(uint32_t imageIndex)
    {
        // Reset to make sure it is able to be recorded
        vkResetCommandBuffer(m_commandBuffers[VM_currentFrame], 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = 0;
        beginInfo.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(m_commandBuffers[VM_currentFrame], &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }

        std::vector<VkClearValue> clearValues;        
        VkClearValue clearColor;
        clearColor.color = { {0.0f, 0.0f, 0.0f, 0.0f} };        
        clearValues.push_back(clearColor);
        if (m_b_depthBuffersEnabled)
        {
            VkClearValue depth;
            depth.depthStencil = { 1.0f, 0 };
            clearValues.push_back(depth);
        }
        if (m_b_msaaEnabled)
        {
            VkClearValue msaa;
            msaa.color = { {0.0f, 0.0f, 0.0f, 0.0f} };
            clearValues.push_back(msaa);
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_renderPass;
        renderPassInfo.framebuffer = m_framebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_winSystem->GetExtent();
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        // we did specify viewport and scissor state for this pipeline to be dynamic. So we need to set them in the command buffer before issuing our draw command:
        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(m_winSystem->GetExtent().width);
        viewport.height = static_cast<float>(m_winSystem->GetExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(m_commandBuffers[VM_currentFrame], 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = { 0, 0 };
        scissor.extent = m_winSystem->GetExtent();
        vkCmdSetScissor(m_commandBuffers[VM_currentFrame], 0, 1, &scissor);

        vkCmdBeginRenderPass(m_commandBuffers[VM_currentFrame], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void RenderPass::EndRenderPass()
    {
        // End render pass
        vkCmdEndRenderPass(m_commandBuffers[VM_currentFrame]);

        // Finish recording the command buffer
        if (vkEndCommandBuffer(m_commandBuffers[VM_currentFrame]) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer!");
        }
    }

    void RenderPass::RecordCommandBuffer(uint32_t imageIndex, Mesh& mesh)
    {
        VkPipeline& graphicsPipeline = mesh.GetMaterial()->GetGraphicsPipeline();
        VkPipelineLayout& pipelineLayout = mesh.GetMaterial()->GetPipelineLayout();
        //Transform* transform = mesh.GetParent()->GetTransform();
        //Vector3 position = transform->GetPosition3D();

        // Push constants - eventually push transformation matrix from Transform Component with rotation and translation
        static auto startTime = std::chrono::high_resolution_clock::now();
        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
        float cosTime = cosf(time);
        float sineTime = sinf(time);
        //std::vector<float> pushConstants = { position.x, position.y, position.z }; // where sizeof(float) == 4 bytes
        std::vector<float> pushConstants = { sineTime, cosTime, 0 };
        uint32_t offset = 0;
        uint32_t size = pushConstants.size() * sizeof(float);
        vkCmdPushConstants(m_commandBuffers[VM_currentFrame], pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, offset, size, pushConstants.data());

        vkCmdBindPipeline(m_commandBuffers[VM_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    }

    void RenderPass::DrawIndexed(Mesh& mesh)
    {
        VkPipelineLayout& pipelineLayout = mesh.GetMaterial()->GetPipelineLayout();
        VkDescriptorSet& descriptorSet = mesh.GetDescriptorSets()[VM_currentFrame];
        VkBuffer& vertexBuffer = mesh.GetModel().GetVertexBuffer();
        VkBuffer& indexBuffer = mesh.GetModel().GetIndexBuffer();
        std::vector<uint32_t>& indices = mesh.GetModel().GetIndices();

        VkBuffer vertexBuffers[] = { vertexBuffer };
        VkDeviceSize offsets[] = { 0 };
        vkCmdBindVertexBuffers(m_commandBuffers[VM_currentFrame], 0, 1, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(m_commandBuffers[VM_currentFrame], indexBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(m_commandBuffers[VM_currentFrame], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, nullptr);

        // Refer to - https://vulkan-tutorial.com/en/Vertex_buffers/Index_buffer
        vkCmdDrawIndexed(m_commandBuffers[VM_currentFrame], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0); // reusing vertices with index buffers.
        // NOTE FROM THE WIKI: The previous chapter already mentioned that you should allocate multiple resources like buffers from a single memory allocation, but in fact you should go a step further. Driver developers recommend that you also store multiple buffers, like the vertex and index buffer, into a single VkBuffer and use offsets in commands like vkCmdBindVertexBuffers. The advantage is that your data is more cache friendly in that case, because it's closer together. It is even possible to reuse the same chunk of memory for multiple resources if they are not used during the same render operations, provided that their data is refreshed, of course. This is known as aliasing and some Vulkan functions have explicit flags to specify that you want to do this.
    }

    void RenderPass::CreateCommandBuffers(VkCommandPool commandPool)
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = VM_MAX_FRAMES_IN_FLIGHT;

        m_commandBuffers.resize(VM_MAX_FRAMES_IN_FLIGHT);

        if (vkAllocateCommandBuffers(m_deviceHandle->GetDevice(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void RenderPass::DestroyCommandBuffers()
    {

    }

    std::vector<VkCommandBuffer>& RenderPass::GetCommandBuffers()
    {
        return m_commandBuffers;
    }
}
