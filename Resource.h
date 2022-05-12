#pragma once
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include "Color.h"
class Texture;
class Material;
class MaterialInfo;
class Shader;
class ResourceImpl;
class MaterialInfo;
class Node;
class Animation;

class Resource {
public:
	static Resource& getInstance();
	void loadAllMaterial();

	Color getColor(const std::string& name);

	std::shared_ptr<Texture> getTexture(const std::string& name);

	std::shared_ptr<Material> getMaterial(const std::string& name);
	//get deffered rendering geometry pass shader accord to material info
	std::shared_ptr<Material> getMaterialDefferedGeoPass(const MaterialInfo& mInfo,bool hasNodeAnimation = false);
	//get deffered rendering lighting pass shader
	std::shared_ptr<Material> getMaterialDefferedLightPass(bool hasIBL);

	std::shared_ptr<Shader> getShader(const std::string& name);
	//获得某个mesh
	std::shared_ptr<Node> getNode(const std::string& name);

	//获得所有mesh
	const std::unordered_map<std::string, std::shared_ptr<Node>>& getAllNode();

	//GL_RGB,GL_RGB,GL_UNSIGNED_BYTE
	std::shared_ptr<Texture> createTexture(const std::string& name, int width, int height,
		unsigned char* pdata,
		int internalFormat,
		int format,
		unsigned int type,
		bool autoMipmap = false);

	/*
	* path is a image file in drawable directory,
	* if texName is empty the stem of image file will be the texName;
	*/
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

	/*
	* 分析.meshCfg文件,加载mesh
	*/
	bool parseMeshCfgFile(const std::string& filePath);

	//将config配置里面的某个key对应的value转换为int，
	int getKeyAsInt(const std::string& key);
	const std::string_view getKeyAsStr(std::string_view key);
private:
	Resource();
	std::unique_ptr<ResourceImpl> mpImpl;
};