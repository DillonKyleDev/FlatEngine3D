#include "Mesh.h"


namespace FlatEngine
{
	Mesh::Mesh()
	{
		m_model = Model();
		m_material = nullptr;
		m_descriptorSets = std::vector<VkDescriptorSet>();
		m_allocationPoolIndex = -1;
	}

	Mesh::~Mesh()
	{
	}

	void Mesh::Cleanup()
	{
		m_material->GetAllocator().SetFreed(m_allocationPoolIndex);
	}


	void Mesh::SetModel(Model model)
	{
		m_model = model;
	}

	Model& Mesh::GetModel()
	{
		return m_model;
	}

	void Mesh::CreateModelResources(VkCommandPool commandPool, PhysicalDevice& physicalDevice, LogicalDevice& logicalDevice)
	{
		m_model.LoadModel();
		m_model.CreateVertexBuffer(commandPool, physicalDevice, logicalDevice);
		m_model.CreateIndexBuffer(commandPool, physicalDevice, logicalDevice);
		m_model.CreateUniformBuffers(physicalDevice, logicalDevice);
	}

	void Mesh::SetMaterial(std::shared_ptr<Material> material)
	{
		m_material = material;
	}

	std::shared_ptr<Material> Mesh::GetMaterial()
	{
		return m_material;
	}

	void Mesh::CreateResources(VkCommandPool commandPool, PhysicalDevice& physicalDevice, LogicalDevice& logicalDevice)
	{
		if (m_model.GetModelPath() != "")
		{
			CreateModelResources(commandPool, physicalDevice, logicalDevice);
		}
		if (m_material != nullptr)
		{
			m_material->CreateDescriptorSets(m_descriptorSets, m_model, m_allocationPoolIndex);
		}
	}

	std::vector<VkDescriptorSet>& Mesh::GetDescriptorSets()
	{
		return m_descriptorSets;
	}
}