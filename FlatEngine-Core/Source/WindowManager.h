#pragma once
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>

//#define APP_USE_UNLIMITED_FRAME_RATE
#ifdef _DEBUG
#define APP_USE_VULKAN_DEBUG_REPORT
#endif
#include <SDL.h>
#include <string>
#include <stdio.h>          // printf, fprintf
#include <stdlib.h>         // abort

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>


namespace FlatEngine
{
	class WindowManager {
	public:
		VkAllocationCallbacks*   m_Allocator;
		VkInstance               m_Instance;
		VkPhysicalDevice         m_PhysicalDevice;
		VkDevice                 m_Device;
		uint32_t                 m_QueueFamily;
		VkQueue                  m_Queue;
		VkDebugReportCallbackEXT m_DebugReport;
		VkPipelineCache          m_PipelineCache;
		VkDescriptorPool         m_DescriptorPool;
		ImGui_ImplVulkanH_Window m_MainWindowData;
		uint32_t                 m_MinImageCount;
		bool                     m_SwapChainRebuild;

		WindowManager();
		~WindowManager();
		void Close();
		void CleanupVulkan();
		void CleanupVulkanWindow();

		GLFWwindow* GetWindow();
		VkSurfaceKHR GetSurface();
		SDL_Renderer* GetRenderer();
		bool Init(std::string title, int width, int height, ImVector<const char*> instance_extensions);
		void SetScreenDimensions(int width, int height);
		void SetTitle(std::string title);
		void SetFullscreen(bool b_isFullscreen);

		static VKAPI_ATTR VkBool32 VKAPI_CALL debug_report(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object, size_t location, int32_t messageCode, const char* pLayerPrefix, const char* pMessage, void* pUserData);
		static void glfw_error_callback(int error, const char* description);
		static void check_vk_result(VkResult err);
		bool IsExtensionAvailable(const ImVector<VkExtensionProperties>& properties, const char* extension);
		void SetupVulkan(ImVector<const char*> instance_extensions);
		void SetupVulkanWindow(int width, int height);
		void FrameRender(ImDrawData* draw_data);
		void FramePresent();


	private:
		bool CreateNewWindow(std::string title, int width, int height);

		std::string m_title;
		int m_windowWidth;
		int m_windowHeight;
		bool m_b_isFullscreen;
		GLFWwindow* m_window;
		SDL_Renderer* m_renderer;
		VkSurfaceKHR m_surface;
	};
}