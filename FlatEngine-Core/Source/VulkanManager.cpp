#pragma once
#include "VulkanManager.h"
#include "Helper.h"
#include "Material.h"

// Refer to - https://vulkan-tutorial.com/en/Uniform_buffers/Descriptor_layout_and_buffer
#include <gtc/matrix_transform.hpp> // Not used currently but might need it later

#include <chrono> // Time keeping
#include <memory>
#include <array>


namespace FlatEngine
{
    ValidationLayers VM_validationLayers = ValidationLayers();
    uint32_t VM_currentFrame = 0;

    void VulkanManager::check_vk_result(VkResult err)
    {
        if (err == 0)
        {
            return;
        }
        fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
        if (err < 0)
        {
            abort();
        }
    }

    VulkanManager::VulkanManager()
    {
        m_instance = VK_NULL_HANDLE;
        m_winSystem = WinSys();
        m_physicalDevice = PhysicalDevice();
        m_logicalDevice = LogicalDevice();

        m_mainRenderPass = RenderPass();
        m_sceneTextureRenderPass = RenderPass();
        m_imguiManager = ImGuiManager();               

        m_viewportImages = std::vector<VkImage>();
        m_viewportImageViews = std::vector<VkImageView>();
        m_viewportImageMemory = std::vector<VkDeviceMemory>();
        m_viewportSampler = VK_NULL_HANDLE;
        viewportDescriptorSets = std::vector<VkDescriptorSet>();

        // gpu communication
        m_commandPool = VK_NULL_HANDLE;
        m_imageAvailableSemaphores = std::vector<VkSemaphore>();
        m_renderFinishedSemaphores = std::vector<VkSemaphore>();
        m_inFlightFences = std::vector<VkFence>();
        m_b_framebufferResized = false;


        m_materials = std::map<std::string, std::shared_ptr<Material>>();

        std::shared_ptr<Material> blueMaterial = std::make_shared<Material>("../Shaders/compiledShaders/vert2.spv", "../Shaders/compiledShaders/frag2.spv");
        blueMaterial->AddTexture("../Textures/blue.png");
        m_materials.emplace("blue", blueMaterial);

        Mesh quad;
        quad.GetModel().SetModelPath("../Models/cube.obj");
        quad.SetMaterial(blueMaterial);
        
        std::vector<Mesh> meshes;
        meshes.push_back(quad);
        m_meshesByMaterial.emplace("quads", meshes);
    }

    VulkanManager::~VulkanManager()
    {
    }

    void VulkanManager::Cleanup()
    {
        vkDeviceWaitIdle(m_logicalDevice.GetDevice()); // This may need to be moved elsewhere potentially

        // Semaphores and Fences
        for (size_t i = 0; i < VM_MAX_FRAMES_IN_FLIGHT; i++)
        {
            vkDestroySemaphore(m_logicalDevice.GetDevice(), m_imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(m_logicalDevice.GetDevice(), m_renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(m_logicalDevice.GetDevice(), m_inFlightFences[i], nullptr);
        }

        m_winSystem.CleanupDrawingResources();

        for (std::pair<std::string, std::shared_ptr<Material>> materialPair : m_materials)
        {
            // Cleanup allocator as well??
            materialPair.second->CleanupGraphicsPipeline();
        }

        vkDestroyCommandPool(m_logicalDevice.GetDevice(), m_commandPool, nullptr);

        m_mainRenderPass.Cleanup(m_logicalDevice);
        m_sceneTextureRenderPass.Cleanup(m_logicalDevice);
        m_logicalDevice.Cleanup();
        m_physicalDevice.Cleanup();
        VM_validationLayers.Cleanup(m_instance);
        m_winSystem.CleanupSystem();

        // Destroy Vulkan instance
        vkDestroyInstance(m_instance, nullptr);
    }

    bool VulkanManager::Init()
    {
        bool b_success = true;

        if (!InitVulkan())
        {
            printf("Vulkan initialization failed!\n");
            b_success = false;
        }
        else
        {
            printf("Vulkan initialized...\n");

            if (!m_imguiManager.SetupImGui(m_instance, m_winSystem, m_physicalDevice, m_logicalDevice))
            {
                printf("ImGui-Vulkan initialization failed!\n");
                b_success = false;
            }
            else
            {
                printf("ImGui-Vulkan initialized...\n");
            }
        }

        return b_success;
    }

    bool VulkanManager::InitVulkan()
    {
        bool b_success = true;

        if (!m_winSystem.CreateGLFWWindow("FlatEngine"))
        {
            printf("GLFW initialization failed!\n");            
            b_success = false;
        }
        else
        {
            if (!CreateVulkanInstance())
            {
                printf("Failed to create Vulkan instance!\n");
                b_success = false;
            }
            else
            {
                m_winSystem.SetHandles(m_instance, m_physicalDevice, m_logicalDevice);
                m_winSystem.CreateSurface();
                VM_validationLayers.SetupDebugMessenger(m_instance);
                m_physicalDevice.Init(m_instance, m_winSystem.GetSurface());
                m_logicalDevice.Init(m_physicalDevice, m_winSystem.GetSurface());
                m_winSystem.CreateDrawingResources();
                QueueFamilyIndices indices = Helper::FindQueueFamilies(m_physicalDevice.GetDevice(), m_winSystem.GetSurface());
                m_logicalDevice.SetGraphicsIndex(indices.graphicsFamily.value());
                CreateCommandPool(m_commandPool, m_logicalDevice, indices.graphicsFamily.value());
                CreateSyncObjects();

                // Main RenderPass Configuration
                m_mainRenderPass.SetHandles(m_instance, m_winSystem, m_physicalDevice, m_logicalDevice);
                m_mainRenderPass.EnableDepthBuffering();
                m_mainRenderPass.EnableMsaa();
                m_mainRenderPass.SetDefaultRenderPassConfig();
                m_mainRenderPass.ConfigureFrameBufferImageViews(m_winSystem.GetSwapChainImageViews());
                m_mainRenderPass.Init(m_commandPool);

                // Scene texture RenderPass configuration
                m_sceneTextureRenderPass.SetHandles(m_instance, m_winSystem, m_physicalDevice, m_logicalDevice);    
                CreateSceneRenderPassResources();
                //CreateWriteToImageResources(m_viewportImages, m_viewportImageViews, m_viewportImageMemory);
                CreateViewportImages();
                CreateViewportImageViews();
                m_sceneTextureRenderPass.ConfigureFrameBufferImageViews(m_viewportImageViews);                
                m_sceneTextureRenderPass.Init(m_commandPool);              

                for (std::map<std::string, std::shared_ptr<Material>>::iterator pairIter = m_materials.begin(); pairIter != m_materials.end(); pairIter++)
                {
                    //pairIter->second->SetHandles(m_instance, m_winSystem, m_physicalDevice, m_logicalDevice, m_sceneTextureRenderPass);
                    pairIter->second->SetHandles(m_instance, m_winSystem, m_physicalDevice, m_logicalDevice, m_mainRenderPass);
                    pairIter->second->CreateMaterialResources(m_commandPool);
                }

                for (std::map<std::string, std::vector<Mesh>>::iterator iter = m_meshesByMaterial.begin(); iter != m_meshesByMaterial.end(); iter++)
                {
                    for (std::vector<Mesh>::iterator meshIter = iter->second.begin(); meshIter != iter->second.end(); meshIter++)
                    {
                        meshIter->CreateResources(m_commandPool, m_physicalDevice, m_logicalDevice);                       
                    }
                }
            }
        }

        return b_success;
    }

    void VulkanManager::CreateSceneRenderPassResources()
    {        
        m_sceneTextureRenderPass.EnableDepthBuffering();
        m_sceneTextureRenderPass.EnableMsaa();

        VkAttachmentDescription colorAttachment{};
        colorAttachment.format = m_winSystem.GetImageFormat();
        colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachment.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        VkAttachmentReference colorAttachmentRef{};
        colorAttachmentRef.attachment = 0;
        colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        m_sceneTextureRenderPass.AddRenderPassAttachment(colorAttachment, colorAttachmentRef);
        
        VkAttachmentDescription depthAttachment{};
        depthAttachment.format = Helper::FindDepthFormat(m_physicalDevice.GetDevice());
        depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
        depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        VkAttachmentReference depthAttachmentRef{};
        depthAttachmentRef.attachment = 1;
        depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        m_sceneTextureRenderPass.AddRenderPassAttachment(depthAttachment, depthAttachmentRef);

        VkAttachmentDescription colorAttachmentResolve{};
        colorAttachmentResolve.format = m_winSystem.GetImageFormat();
        colorAttachmentResolve.samples = VK_SAMPLE_COUNT_1_BIT;
        colorAttachmentResolve.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        colorAttachmentResolve.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        colorAttachmentResolve.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        colorAttachmentResolve.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        colorAttachmentResolve.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        VkAttachmentReference colorAttachmentResolveRef{};
        colorAttachmentResolveRef.attachment = 2;
        colorAttachmentResolveRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        m_sceneTextureRenderPass.AddRenderPassAttachment(colorAttachmentResolve, colorAttachmentResolveRef);

        // Create Dependency
        VkSubpassDependency dependency{};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        m_sceneTextureRenderPass.AddSubpassDependency(dependency);

        VkSubpassDescription subpass{};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;        
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &m_sceneTextureRenderPass.GetAttachmentRefs()[0];
        subpass.pDepthStencilAttachment = &m_sceneTextureRenderPass.GetAttachmentRefs()[1];
        subpass.pResolveAttachments = &m_sceneTextureRenderPass.GetAttachmentRefs()[2];        
        m_sceneTextureRenderPass.AddSubpass(subpass);
    }

    void VulkanManager::CreateWriteToImageResources(std::vector<VkImage>& images, std::vector<VkImageView>& imageViews, std::vector<VkDeviceMemory>& imageMemory)
    {        
        images.resize(m_winSystem.GetSwapChainImageViews().size());
        imageViews.resize(m_winSystem.GetSwapChainImageViews().size());
        imageMemory.resize(m_winSystem.GetSwapChainImageViews().size());
        VkExtent2D extent = m_winSystem.GetExtent();

        for (size_t i = 0; i < m_winSystem.GetSwapChainImageViews().size(); i++)
        {
            uint32_t mipLevels = 1;            
            int texWidth = extent.width;
            int texHeight = extent.height;
            VkDeviceSize imageSize = texWidth * texHeight * 4;
            VkBuffer stagingBuffer{};
            VkDeviceMemory stagingBufferMemory{};
            WinSys::CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory, m_physicalDevice, m_logicalDevice);
            WinSys::CreateImage(texWidth, texHeight, 1, VK_SAMPLE_COUNT_1_BIT, m_winSystem.GetImageFormat(), VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, images[i], imageMemory[i], m_physicalDevice, m_logicalDevice);

            VkCommandBuffer copyCmd = Helper::BeginSingleTimeCommands(m_commandPool, m_logicalDevice);
            WinSys::InsertImageMemoryBarrier(
                copyCmd,
                m_viewportImages[i],
                VK_ACCESS_TRANSFER_READ_BIT,
                VK_ACCESS_MEMORY_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
            Helper::EndSingleTimeCommands(m_commandPool, copyCmd, m_logicalDevice);

            //WinSys::TransitionImageLayout(images[i], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, m_commandPool, m_logicalDevice);
            //WinSys::CopyBufferToImage(stagingBuffer, images[i], static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, m_commandPool, m_logicalDevice);
            //WinSys::TransitionImageLayout(images[i], VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 1, m_commandPool, m_logicalDevice);

            //vkDestroyBuffer(m_logicalDevice.GetDevice(), stagingBuffer, nullptr);
            //vkFreeMemory(m_logicalDevice.GetDevice(), stagingBufferMemory, nullptr);

            WinSys::CreateImageView(imageViews[i], images[i], m_winSystem.GetImageFormat(), VK_IMAGE_ASPECT_COLOR_BIT, mipLevels, m_logicalDevice);
            WinSys::CreateTextureSampler(m_viewportSampler, mipLevels, m_physicalDevice, m_logicalDevice);
        }
    }

    void VulkanManager::CreateViewportImages()
    {
        m_viewportImages.resize(m_winSystem.GetSwapChainImageViews().size());
        m_viewportImageMemory.resize(m_winSystem.GetSwapChainImageViews().size());
        VkExtent2D extent = m_winSystem.GetExtent();

        for (uint32_t i = 0; i < m_winSystem.GetSwapChainImageViews().size(); i++)
        {
            // Create the linear tiled destination image to copy to and to read the memory from
            VkImageCreateInfo imageCreateCI{};
            imageCreateCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
            imageCreateCI.imageType = VK_IMAGE_TYPE_2D;
            // Note that vkCmdBlitImage (if supported) will also do format conversions if the swapchain color format would differ
            imageCreateCI.format = VK_FORMAT_B8G8R8A8_SRGB;
            imageCreateCI.extent.width = extent.width;
            imageCreateCI.extent.height = extent.height;
            imageCreateCI.extent.depth = 1;
            imageCreateCI.arrayLayers = 1;
            imageCreateCI.mipLevels = 1;
            imageCreateCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            imageCreateCI.samples = VK_SAMPLE_COUNT_1_BIT;
            imageCreateCI.tiling = VK_IMAGE_TILING_LINEAR;
            imageCreateCI.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
            // Create the image
            // VkImage dstImage;
            vkCreateImage(m_logicalDevice.GetDevice(), &imageCreateCI, nullptr, &m_viewportImages[i]);
            WinSys::CreateTextureSampler(m_viewportSampler, 1, m_physicalDevice, m_logicalDevice);
            // Create memory to back up the image
            VkMemoryRequirements memRequirements;
            VkMemoryAllocateInfo memAllocInfo{};
            memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            // VkDeviceMemory dstImageMemory;
            vkGetImageMemoryRequirements(m_logicalDevice.GetDevice(), m_viewportImages[i], &memRequirements);
            memAllocInfo.allocationSize = memRequirements.size;
            // Memory must be host visible to copy from
            memAllocInfo.memoryTypeIndex = Helper::FindMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_physicalDevice.GetDevice());
            vkAllocateMemory(m_logicalDevice.GetDevice(), &memAllocInfo, nullptr, &m_viewportImageMemory[i]);
            vkBindImageMemory(m_logicalDevice.GetDevice(), m_viewportImages[i], m_viewportImageMemory[i], 0);

            VkCommandBuffer copyCmd = Helper::BeginSingleTimeCommands(m_commandPool, m_logicalDevice);
            WinSys::InsertImageMemoryBarrier(
                copyCmd,
                m_viewportImages[i],
                VK_ACCESS_TRANSFER_READ_BIT,
                VK_ACCESS_MEMORY_READ_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VK_PIPELINE_STAGE_TRANSFER_BIT,
                VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 });
            Helper::EndSingleTimeCommands(m_commandPool, copyCmd, m_logicalDevice);
        }
    }

    void VulkanManager::CreateViewportImageViews()
    {
        m_viewportImageViews.resize(m_viewportImages.size());

        for (uint32_t i = 0; i < m_viewportImages.size(); i++)
        {
            WinSys::CreateImageView(m_viewportImageViews[i], m_viewportImages[i], VK_FORMAT_B8G8R8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, 1, m_logicalDevice);
        }
    }

    bool VulkanManager::CreateVulkanInstance()
    {
        bool b_success = true;

        // Validation layer setup for debugger
        if (b_ENABLE_VALIDATION_LAYERS && !VM_validationLayers.CheckSupport())
        {            
            printf("Error: Validation layers requested, but not available.\n");
            b_success = false;
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "FlatEngine";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "FlatEngine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        uint32_t ui_glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&ui_glfwExtensionCount);

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;
        createInfo.enabledExtensionCount = ui_glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
        
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{}; // DebugUtilsMessenger for CreateInstance and DestroyInstance functions (automatically destroyed by Vulkan when closed)

        if (b_ENABLE_VALIDATION_LAYERS)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(VM_validationLayers.Size());
            createInfo.ppEnabledLayerNames = VM_validationLayers.Data();
            VM_validationLayers.PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else
        {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        // IS THIS OVERRIDING THE GLFW EXTENSIONS CREATED ABOVE?
        // Get extensions for use with debug messenger
        auto extensions = VM_validationLayers.GetRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
        {            
            printf("Failed to create Vulkan instance...\n");
            b_success = false;
        }

        return b_success;
    }

    void VulkanManager::CreateCommandPool(VkCommandPool& commandPool, LogicalDevice& logicalDevice, uint32_t queueFamilyIndex, VkCommandPoolCreateFlags flags)
    {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = flags;
        poolInfo.queueFamilyIndex = queueFamilyIndex;

        if (vkCreateCommandPool(logicalDevice.GetDevice(), &poolInfo, nullptr, &commandPool) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create command pool!");
        }
    }

    void VulkanManager::CreateSyncObjects()
    {
        // More info here - https://vulkan-tutorial.com/en/Drawing_a_triangle/Drawing/Rendering_and_presentation
       
        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; // Start it signaled so the very first frame doesn't block indefinitely

        m_imageAvailableSemaphores.resize(VM_MAX_FRAMES_IN_FLIGHT);
        m_renderFinishedSemaphores.resize(VM_MAX_FRAMES_IN_FLIGHT);
        m_inFlightFences.resize(VM_MAX_FRAMES_IN_FLIGHT);

        for (size_t i = 0; i < VM_MAX_FRAMES_IN_FLIGHT; i++)
        {
            if (vkCreateSemaphore(m_logicalDevice.GetDevice(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
                vkCreateSemaphore(m_logicalDevice.GetDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
                vkCreateFence(m_logicalDevice.GetDevice(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
            {
                throw std::runtime_error("failed to create semaphores!");
            }
        }
    }

    void VulkanManager::CreateImGuiTexture(Texture& texture, std::vector<VkDescriptorSet>& descriptorSets, int& allocatedFrom)
    {
        texture.CreateTextureImage(m_winSystem, m_commandPool, m_physicalDevice, m_logicalDevice);        
        m_imguiManager.CreateDescriptorSets(texture, descriptorSets, allocatedFrom);
    }

    void VulkanManager::FreeImGuiTexture(uint32_t allocatedFrom)
    {
        m_imguiManager.FreeDescriptorSet(allocatedFrom);
    }

    void VulkanManager::DrawFrame(ImDrawData* drawData)
    {
        // More info here - https://vulkan-tutorial.com/en/Drawing_a_triangle/Drawing/Rendering_and_presentation

        if (m_winSystem.m_b_framebufferResized)
        {
            RecreateSwapChainAndFrameBuffers();
            m_winSystem.m_b_framebufferResized = false;
        }

        // At the start of the frame, we want to wait until the previous frame has finished, so that the command buffer and semaphores are available to use. To do that, we call vkWaitForFences:
        vkWaitForFences(m_logicalDevice.GetDevice(), 1, &m_inFlightFences[VM_currentFrame], VK_TRUE, UINT64_MAX);

        uint32_t imageIndex;
        VkResult aquireImageResult = vkAcquireNextImageKHR(m_logicalDevice.GetDevice(), m_winSystem.GetSwapChain(), UINT64_MAX, m_imageAvailableSemaphores[VM_currentFrame], VK_NULL_HANDLE, &imageIndex);
        if (!CheckSwapChainIntegrity(aquireImageResult, "Failed to acquire swap chain image."))
        {
            return;
        }

        // manually reset the fence to the unsignaled state with the vkResetFences call:
        vkResetFences(m_logicalDevice.GetDevice(), 1, &m_inFlightFences[VM_currentFrame]);



        ///////////////////////
        // Main RenderPass
        m_mainRenderPass.BeginRenderPass(imageIndex);
        for (std::pair<std::string, std::vector<Mesh>> meshes : m_meshesByMaterial)
        {
            for (Mesh& mesh : meshes.second)
            {
                m_mainRenderPass.RecordCommandBuffer(imageIndex, mesh);  
                m_mainRenderPass.DrawIndexed(mesh);
                mesh.GetModel().UpdateUniformBuffer(VM_currentFrame, m_winSystem, 0.5f);
            }
        }
        m_mainRenderPass.EndRenderPass();
        ///////////////////////

        /////////////////////////
        //// Draw To Texture RenderPass
        //m_sceneTextureRenderPass.BeginRenderPass(imageIndex);
        //for (std::pair<std::string, std::vector<Mesh>> meshes : m_meshesByMaterial)
        //{
        //    for (Mesh& mesh : meshes.second)
        //    {
        //        m_sceneTextureRenderPass.RecordCommandBuffer(imageIndex, mesh);
        //        m_sceneTextureRenderPass.DrawIndexed(mesh);
        //        mesh.GetModel().UpdateUniformBuffer(VM_currentFrame, m_winSystem, 0.5f);
        //    }
        //}
        //m_sceneTextureRenderPass.EndRenderPass();
        /////////////////////////

        /////////////////////////
        //// ImGui UI RenderPass
        //m_imguiManager.HandleRenderPass(imageIndex);
        /////////////////////////



        // Submit the command buffers
        std::array<VkCommandBuffer, 3> commandBuffers = { m_mainRenderPass.GetCommandBuffers()[VM_currentFrame], m_sceneTextureRenderPass.GetCommandBuffers()[VM_currentFrame], m_imguiManager.GetRenderPass().GetCommandBuffers()[VM_currentFrame] };
  
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = { m_imageAvailableSemaphores[VM_currentFrame] };
        VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &m_mainRenderPass.GetCommandBuffers()[VM_currentFrame];

        VkSemaphore signalSemaphores[] = { m_renderFinishedSemaphores[VM_currentFrame] };
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        VkResult submitResult = vkQueueSubmit(m_logicalDevice.GetGraphicsQueue(), 1, &submitInfo, m_inFlightFences[VM_currentFrame]);
        CheckSwapChainIntegrity(submitResult, "Failed to submit draw command buffer.");

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = { m_winSystem.GetSwapChain() };
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;
        presentInfo.pImageIndices = &imageIndex;
        presentInfo.pResults = nullptr;

        // Present!
        VkResult presentResult = vkQueuePresentKHR(m_logicalDevice.GetPresentQueue(), &presentInfo);

        viewportDescriptorSets.resize(VM_MAX_FRAMES_IN_FLIGHT);
        for (size_t i = 0; i < VM_MAX_FRAMES_IN_FLIGHT; i++)
        {
            //viewportDescriptorSets[i] = ImGui_ImplVulkan_AddTexture(m_viewportSampler, m_viewportImageViews[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
        }

        VM_currentFrame = (VM_currentFrame + 1) % VM_MAX_FRAMES_IN_FLIGHT;
    }

    void VulkanManager::RecreateSwapChainAndFrameBuffers()
    {
        m_winSystem.RecreateSwapChain();
        m_sceneTextureRenderPass.RecreateFrameBuffers();
        //m_imguiManager.GetRenderPass().RecreateFrameBuffers();
        // ImGui_ImplVulkan_SetMinImageCount(static_cast<uint32_t>(m_winSystem.GetSwapChainImageViews().size()));
    }

    bool VulkanManager::CheckSwapChainIntegrity(VkResult result, std::string errorMessage)
    {
        // More details here - https://vulkan-tutorial.com/en/Drawing_a_triangle/Swap_chain_recreation
        // Check on swap chain integrity after image access and after present

        bool b_swapChainGood = true;

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        {
            b_swapChainGood = false;
            RecreateSwapChainAndFrameBuffers();
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            RecreateSwapChainAndFrameBuffers();
            throw std::runtime_error(errorMessage);
        }

        return b_swapChainGood;
    }

    void VulkanManager::AddMeshesByMaterial(std::pair<std::string, std::vector<Mesh>>& mesh)
    {
        if (m_meshesByMaterial.count(mesh.first))
        {
            m_meshesByMaterial.at(mesh.first).insert(m_meshesByMaterial.at(mesh.first).end(), mesh.second.begin(), mesh.second.end());            
        }
        else
        {
            m_meshesByMaterial.emplace(mesh);
        }
    }

    void VulkanManager::SetMeshes(std::map<std::string, std::vector<Mesh>>& meshes)
    {
        m_meshesByMaterial = meshes;
    }

    WinSys& VulkanManager::GetWinSystem()
    {
        return m_winSystem;
    }

    VkInstance& VulkanManager::GetInstance()
    {
        return m_instance;
    }

    VkQueue& VulkanManager::GetGraphicsQueue()
    {
        return m_logicalDevice.GetGraphicsQueue();
    }

    void VulkanManager::FramebufferResizeCallback(GLFWwindow* window, int width, int height)
    {
        // Called when GLFW detects the window has been resized.
        auto app = reinterpret_cast<WinSys*>(glfwGetWindowUserPointer(window));
        app->m_b_framebufferResized = true;
    }
}