#pragma once
#include "GraphicsPipeline.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "WinSys.h"
#include "RenderPass.h"
#include "Model.h"
#include "Texture.h"
#include "Allocator.h"

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>

#include <string>
#include <vector>


namespace FlatEngine
{
	class Material
	{
	public:
		Material(std::string vertexPath, std::string fragmentPath);
		Material();
		~Material();
		void Init();
		void CleanupGraphicsPipeline();	
		void CleanupTextures();
		
		void SetHandles(VkInstance instance, WinSys& winSystem, PhysicalDevice& physicalDevice, LogicalDevice& logicalDevice, RenderPass& renderPass);
		void CreateMaterialResources(VkCommandPool commandPool);
		void SetVertexPath(std::string path);
		void SetFragmentPath(std::string path);
		void CreateGraphicsPipeline();
		VkPipeline& GetGraphicsPipeline();
		VkPipelineLayout& GetPipelineLayout();
		VkDescriptorPool CreateDescriptorPool();		
		void CreateDescriptorSets(std::vector<VkDescriptorSet>& descriptorSets, Model& model, int& allocatedFrom);
		void AddTexture(std::string path);
		void AddTexture(Texture texture);
		std::vector<Texture>& GetTextures();
		void CreateTextureResources(VkCommandPool commandPool);
		Allocator& GetAllocator();

	private:
		GraphicsPipeline m_graphicsPipeline;
		std::vector<Texture> m_textures;
		Allocator m_allocator;
		// handles
		VkInstance m_instanceHandle;
		WinSys* m_winSystem;
		PhysicalDevice* m_physicalDeviceHandle;
		LogicalDevice* m_deviceHandle;
		RenderPass* m_renderPass;
	};
}