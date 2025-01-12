///////////////////////
//// Global Assets ////
///////////////////////
#include "Application.h"

#define GLFW_INCLUDE_VULKAN
#include <glfw3.h>
#include <glm.hpp>

#include <map>
#include <string>
#include <memory>



namespace FlatEngine
{
	class Vector4;
	class Texture;

	class AssetManager
	{
	public:
		AssetManager();
		~AssetManager();	
		void UpdateProjectDirs(std::string projectPath);
		void CollectDirectories(DirectoryType dirType);
		void CollectColors();
		void CollectTextures();
		std::string GetFailedToLoadImagePath();
		std::string GetDir(std::string dirName);
		std::string GetFilePath(std::string fileName);
		Vector4 GetColor(std::string colorName);
		Uint32 GetColor32(std::string colorName);
		std::shared_ptr<Texture>GetTextureObject(std::string textureName);
		VkDescriptorSet GetTexture(std::string textureName);		
	private:
		std::map<std::string, std::string> m_directories;
		std::map<std::string, std::string> m_files;
		std::map<std::string, Vector4> m_colors;
		std::map<std::string, std::shared_ptr<Texture>> m_textures;
		std::string m_resourceFailedToLoadImagePath;
		std::shared_ptr<Texture> m_errorTexture;
		Vector4 m_errorColor;
	};
}