#include "Texture.h"
#include "FlatEngine.h"
#include "VulkanManager.h"

#include "imgui_impl_glfw.h"
#include "imgui_impl_vulkan.h"

#include <stdexcept>


namespace FlatEngine
{
	Texture::Texture(std::string path)
	{
		m_path = path;
		m_textureWidth = 0;
		m_textureHeight = 0;
		if (path != "")
		{
			LoadFromFile(path);
		}

		// Vulkan
		m_descriptorSets = std::vector<VkDescriptorSet>(2, {});
		m_allocationIndex = -1;
		m_imageView = VK_NULL_HANDLE;
		m_image = VK_NULL_HANDLE;
		m_textureImageMemory = VK_NULL_HANDLE;
		m_textureSampler = VK_NULL_HANDLE;
		m_mipLevels = 1;
	}

	Texture::~Texture()
	{
		//FreeTexture();
	}

	bool Texture::LoadFromFile(std::string path)
	{
		m_path = path;

		if (path != "")
		{
			FreeTexture();
			
			
			F_VulkanManager->CreateImGuiTexture(*this, m_descriptorSets, m_allocationIndex);

			//m_descriptorSets = std::vector<VkDescriptorSet>(2, ImGui_ImplVulkan_AddTexture(m_textureSampler, m_imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
			return m_allocationIndex != -1;
		}
		else return false;
	}

	//Creates image from font string
	bool Texture::LoadFromRenderedText(std::string textureText, SDL_Color textColor, TTF_Font* font)
	{	
		return false;
		//SDL_Surface* textSurface = TTF_RenderText_Solid(font, textureText.c_str(), textColor);
		////TTF_RenderText_Solid_Wrapped(); For wrapped text

		//if (textSurface == NULL)
		//{
		//	printf("Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError());
		//}
		//else
		//{
		//	//m_texture = SDL_CreateTextureFromSurface(F_Window->GetRenderer(), textSurface);

		//	if (m_texture == NULL)
		//	{
		//		printf("Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError());
		//	}
		//	else
		//	{
		//		m_textureWidth = textSurface->w;
		//		m_textureHeight = textSurface->h;
		//	}

		//	SDL_FreeSurface(textSurface);
		//}

		//return m_texture != NULL;
	}

	void Texture::FreeTexture()
	{			
		if (m_allocationIndex != -1)
		{
			F_VulkanManager->FreeImGuiTexture(m_allocationIndex);
			m_descriptorSets.clear();
			m_descriptorSets.resize(VM_MAX_FRAMES_IN_FLIGHT);
			m_textureWidth = 0;
			m_textureHeight = 0;
		}
	}

	VkDescriptorSet Texture::GetTexture()
	{
		return m_descriptorSets[VM_currentFrame];
	}

	int Texture::GetWidth()
	{
		return m_textureWidth;
	}
	int Texture::GetHeight()
	{
		return m_textureHeight;
	}

	void Texture::SetDimensions(int width, int height)
	{
		m_textureWidth = width;
		m_textureHeight = height;
	}







	// Vulkan
	void Texture::Cleanup(LogicalDevice& logicalDevice)
	{
		vkDestroySampler(logicalDevice.GetDevice(), m_textureSampler, nullptr);
		vkFreeMemory(logicalDevice.GetDevice(), m_textureImageMemory, nullptr);
		vkDestroyImage(logicalDevice.GetDevice(), m_image, nullptr);
		vkDestroyImageView(logicalDevice.GetDevice(), m_imageView, nullptr);
	}


	void Texture::SetTexturePath(std::string path)
	{
		m_path = path;
	}

	std::string Texture::GetTexturePath()
	{
		return m_path;
	}

	VkImageView& Texture::GetImageView()
	{
		return m_imageView;
	}

	VkImage& Texture::GetImage()
	{
		return m_image;
	}

	VkDeviceMemory& Texture::GetTextureImageMemory()
	{
		return m_textureImageMemory;
	}

	VkSampler& Texture::GetTextureSampler()
	{
		return m_textureSampler;
	}

	uint32_t Texture::GetMipLevels()
	{
		return m_mipLevels;
	}

	void Texture::CreateTextureImage(WinSys& winSystem, VkCommandPool commandPool, PhysicalDevice& physicalDevice, LogicalDevice& logicalDevice)
	{
		VkImage newImage = winSystem.CreateTextureImage(m_path, m_mipLevels, commandPool, physicalDevice, logicalDevice, m_textureImageMemory);
		WinSys::CreateImageView(m_imageView, newImage, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_ASPECT_COLOR_BIT, m_mipLevels, logicalDevice);
		WinSys::CreateTextureSampler(m_textureSampler, m_mipLevels, physicalDevice, logicalDevice);
	}
}