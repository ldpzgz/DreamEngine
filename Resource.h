#pragma once
#include <memory>
#include <string>
class Texture;
class Material;
class MaterialInfo;
class Shader;
class ResourceImpl;
class MaterialInfo;

class Resource {
public:
	static Resource& getInstance();
	void loadAllMaterial();

	std::shared_ptr<Texture> getTexture(const std::string& name);

	std::shared_ptr<Material> getMaterial(const std::string& name);
	/*
	* name: 可以是物体的名字
	* mInfo：材质信息，根据里面的信息生成或者clone一个material对象
	*/
	std::shared_ptr<Material> getMaterial(const MaterialInfo& mInfo);

	std::shared_ptr<Shader> getShader(const std::string& name);

	//GL_RGB,GL_RGB,GL_UNSIGNED_BYTE
	std::shared_ptr<Texture> createTexture(const std::string& name, int width, int height,
		unsigned char* pdata,
		int internalFormat,
		int format,
		unsigned int type,
		bool autoMipmap = false);
	std::shared_ptr<Texture> getOrLoadTextureFromFile(const std::string& path, const std::string& texName = "");
	std::shared_ptr<Texture> loadImageFromFile(const std::string& path, const std::string& texName);
	std::shared_ptr<Material> cloneMaterial(const std::string& name);
	/*
	* 分析配置文件里面的：material:matName{matValue}，内容
	* 根据里面的内容生成名字为matName的material对象，保存到gMaterial里面。
	*/
	bool parseMaterial(const std::string& matName, const std::string& matValue);
	/*
	* 分析.material / .program文件
	* 每个.material / .program文件都会自动生成一个Material对象
	*/
	bool parseMaterialFile(const std::string& filePath);

	//如果材质文件里面有个key对应的value是整数，可以用这个函数获取到
	int getKeyAsInt(const std::string& key);
private:
	Resource();
	std::unique_ptr<ResourceImpl> mpImpl;
};