#pragma once
#include "ImGuiManager.h"
#include "Structs.h"
#include "ValidationLayers.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "WinSys.h"
#include "RenderPass.h"
#include "Mesh.h"
#include "Material.h"
#include "Vector2.h"

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>
#include <glm.hpp>

#include <memory>
#include <vector>
#include <map>


namespace FlatEngine
{
    // For device extensions required to present images to the window system (swap chain usage)
    const std::vector<const char*> DEVICE_EXTENSIONS = 
    {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    #ifdef NDEBUG
        const bool b_ENABLE_VALIDATION_LAYERS = false;
    #else
        const bool b_ENABLE_VALIDATION_LAYERS = true;
    #endif

    extern ValidationLayers VM_validationLayers;
    const int VM_MAX_FRAMES_IN_FLIGHT = 2;
    extern uint32_t VM_currentFrame;

    class VulkanManager
    {
    public:
        VulkanManager();
        ~VulkanManager();
        void Cleanup();

        bool Init();
        WinSys& GetWinSystem();
        VkInstance& GetInstance();
        VkQueue& GetGraphicsQueue();             
        void AddMeshesByMaterial(std::pair<std::string, std::vector<Mesh>>& mesh);
        void SetMeshes(std::map<std::string, std::vector<Mesh>>& meshes);
        void CreateImGuiTexture(Texture& texture, std::vector<VkDescriptorSet>& descriptorSets, int& allocatedFrom); // TEMPORARY WORK AROUND BEFORE FULL IMPLIMENATION OF 2D TEXTURES
        void FreeImGuiTexture(uint32_t allocatedFrom);       
        void DrawFrame(ImDrawData* draw_data);

        static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);
        static void check_vk_result(VkResult err);
        static void CreateCommandPool(VkCommandPool& commandPool, LogicalDevice& logicalDevice, uint32_t queueFamilyIndices, VkCommandPoolCreateFlags flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);

        std::vector<VkImage> m_viewportImages;
        std::vector<VkImageView> m_viewportImageViews;
        std::vector<VkDeviceMemory> m_viewportImageMemory;
        VkSampler m_viewportSampler;
        std::vector<VkDescriptorSet> viewportDescriptorSets;

    private:
        bool InitVulkan();
        bool CheckSwapChainIntegrity(VkResult result, std::string errorMessage);
        void RecreateSwapChainAndFrameBuffers();
        bool CreateVulkanInstance();    
        void CreateSyncObjects();
        void CreateWriteToImageResources(std::vector<VkImage>& images, std::vector<VkImageView>& imageViews, std::vector<VkDeviceMemory>& imageMemory);
        void CreateSceneRenderPassResources();
        void CreateViewportImages();
        void CreateViewportImageViews();

        // To be moved into FlatEngine implimentation eventually
        std::map<std::string, std::vector<Mesh>> m_meshesByMaterial;
        std::map<std::string, std::shared_ptr<Material>> m_materials;

        RenderPass m_mainRenderPass;
        RenderPass m_sceneTextureRenderPass;
        ImGuiManager m_imguiManager;

        VkInstance m_instance;
        WinSys m_winSystem;
        PhysicalDevice m_physicalDevice;
        LogicalDevice m_logicalDevice;
        bool m_b_framebufferResized;
        VkCommandPool m_commandPool;
        std::vector<VkSemaphore> m_imageAvailableSemaphores;
        std::vector<VkSemaphore> m_renderFinishedSemaphores;
        std::vector<VkFence> m_inFlightFences;
    };
}