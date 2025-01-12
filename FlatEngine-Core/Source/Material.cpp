#include "Material.h"
#include "VulkanManager.h"

#include <stdexcept>


namespace FlatEngine
{
	Material::Material(std::string vertexPath, std::string fragmentPath)
	{
		m_graphicsPipeline = GraphicsPipeline(vertexPath, fragmentPath);
		Init();
	}

	Material::Material()
	{
		m_graphicsPipeline = GraphicsPipeline();
		Init();
	}

	Material::~Material()
	{
	}

	void Material::Init()
	{
		m_textures = std::vector<Texture>();
		m_allocator = Allocator();
		// handles
		m_instanceHandle = VK_NULL_HANDLE;
		m_winSystem = nullptr;
		m_physicalDeviceHandle = nullptr;
		m_deviceHandle = nullptr;
		m_renderPass = nullptr;
	}

	void Material::CleanupTextures()
	{
		for (Texture& texture : m_textures)
		{
			texture.Cleanup(*m_deviceHandle);
		}
	}


	void Material::SetHandles(VkInstance instance, WinSys& winSystem, PhysicalDevice& physicalDevice, LogicalDevice& logicalDevice, RenderPass& renderPass)
	{
		m_instanceHandle = instance;
		m_winSystem = &winSystem;
		m_physicalDeviceHandle = &physicalDevice;
		m_deviceHandle = &logicalDevice;
		m_renderPass = &renderPass;
	}

	void Material::CreateMaterialResources(VkCommandPool commandPool)
	{
		CreateTextureResources(commandPool);
		m_allocator.Init(AllocatorType::DescriptorPool, static_cast<uint32_t>(m_textures.size()), *m_deviceHandle);
		CreateGraphicsPipeline();
	}

	void Material::SetVertexPath(std::string path)
	{
		m_graphicsPipeline.SetVertexPath(path);
	}

	void Material::SetFragmentPath(std::string path)
	{
		m_graphicsPipeline.SetFragmentPath(path);
	}

	void Material::CreateGraphicsPipeline()
	{
		m_graphicsPipeline.CreateGraphicsPipeline(*m_deviceHandle, *m_winSystem, *m_renderPass, m_allocator.GetDescriptorSetLayout());
	}

	VkPipeline& Material::GetGraphicsPipeline()
	{
		return m_graphicsPipeline.GetGraphicsPipeline();
	}

	VkPipelineLayout& Material::GetPipelineLayout()
	{
		return m_graphicsPipeline.GetPipelineLayout();
	}

	VkDescriptorPool Material::CreateDescriptorPool()
	{
		return m_allocator.CreateDescriptorPool();
	}

	void Material::CreateDescriptorSets(std::vector<VkDescriptorSet>& descriptorSets, Model& model, int& allocatedFrom)
	{
		m_allocator.AllocateDescriptorSets(descriptorSets, model, m_textures, allocatedFrom);
	}

	void Material::CleanupGraphicsPipeline()
	{
		m_graphicsPipeline.Cleanup(*m_deviceHandle);
	}

	void Material::AddTexture(std::string path)
	{
		Texture newTexture = Texture();
		newTexture.SetTexturePath(path);
		m_textures.push_back(newTexture);
	}

	void Material::AddTexture(Texture texture)
	{
		m_textures.push_back(texture);
	}

	std::vector<Texture>& Material::GetTextures()
	{
		return m_textures;
	}

	void Material::CreateTextureResources(VkCommandPool commandPool)
	{
		for (Texture& texture : m_textures)
		{
			if (texture.GetTexturePath() != "")
			{
				texture.CreateTextureImage(*m_winSystem, commandPool, *m_physicalDeviceHandle, *m_deviceHandle);				
			}
		}
	}

	Allocator& Material::GetAllocator()
	{
		return m_allocator;
	}
}