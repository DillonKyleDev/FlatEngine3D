#pragma once
#include "Component.h"
#include "Model.h"
#include "Material.h"
#include "PhysicalDevice.h"
#include "LogicalDevice.h"
#include "RenderPass.h"

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>

#include <memory>


namespace FlatEngine
{
	class Mesh : public Component
	{
	public:
		Mesh();
		~Mesh();
		void Cleanup();

		void SetModel(Model model);
		Model& GetModel();
		void CreateModelResources(VkCommandPool commandPool, PhysicalDevice& physicalDevice, LogicalDevice& logicalDevice);
		void SetMaterial(std::shared_ptr<Material> material);
		std::shared_ptr<Material> GetMaterial();
		void CreateResources(VkCommandPool commandPool, PhysicalDevice& physicalDevice, LogicalDevice& logicalDevice);
		std::vector<VkDescriptorSet>& GetDescriptorSets();

	private:		
		Model m_model;
		std::shared_ptr<Material> m_material;
		std::vector<VkDescriptorSet> m_descriptorSets;
		int m_allocationPoolIndex;
	};
}