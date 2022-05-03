#include "Resource.h"
#include "Material.h"
#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"
#include "Log.h"
#include "helper.h"
#include "Utils.h"
#include "Node.h"
#include "Animation.h"
#include "MeshLoaderAssimp.h"
#include "MeshLoaderGltf.h"
#include "Config.h"
#include "Sampler.h"
#include <filesystem>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <any>
#include <string_view>
#include <charconv>
#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>  //rapidxml::file
#include <rapidxml_print.hpp>  //rapidxml::print

using namespace std;
using namespace std::filesystem;
using MapSMaterial = unordered_map<string, shared_ptr<Material>>;
using MapSTexture = unordered_map<string, shared_ptr<Texture>>;
using MapSShader = unordered_map<string, shared_ptr<Shader>>;
using MapSNode = unordered_map<string, NodeSP>;
using MapSAnimation = unordered_map<string, AnimationSP>;
using MapSMaterialInfo = unordered_map<string, MaterialInfo>;
using Umapss = unordered_map<string, string>;
static const string gColorsPath = "./opengles3/resource/colors.xml";
static const string gMaterialPath = "./opengles3/resource/material";
static const string gProgramPath = "./opengles3/resource/program";
static const string gDrawablePath = "./opengles3/resource/drawable";

static std::unordered_map<std::string_view, unsigned int> gBlendFuncMap{
	{"1",GL_ONE},
	{"0",GL_ZERO},
	{"sc",GL_SRC_COLOR},
	{"dc",GL_DST_COLOR},
	{"1-sc",GL_ONE_MINUS_SRC_COLOR},
	{"1-dc",GL_ONE_MINUS_DST_COLOR},
	{"sa",GL_SRC_ALPHA},
	{"da",GL_DST_ALPHA},
	{"1-sa",GL_ONE_MINUS_SRC_ALPHA},
	{"1-da",GL_ONE_MINUS_DST_ALPHA}
};

static std::unordered_map<std::string_view, unsigned int> gBlendEquationMap{
	{"add",GL_FUNC_ADD},
	{"sub",GL_FUNC_SUBTRACT},
	{"rsub",GL_FUNC_REVERSE_SUBTRACT},
	{"min",GL_MIN},
	{"max",GL_MAX}
};

class ResourceImpl {
public:
	friend Resource;

	void loadAllMaterial();

	Color getColor(const std::string& name) {
		auto it = gRColors.find(name);
		if (it != gRColors.end()) {
			return it->second;
		}
		else {
			LOGE("cannot find %s color in color resource", name.c_str());
			return Color(0.0f,0.0f,0.0f,0.0f);
		}
	}

	std::shared_ptr<Texture> getTexture(const std::string& name) {
		auto it = mTextures.find(name);
		if (it != mTextures.end()) {
			return it->second;
		}
		return nullptr;
	}

	std::shared_ptr<Material> getMaterial(const std::string& name) {
		auto it = mMaterials.find(name);
		if (it != mMaterials.end()) {
			return it->second;
		}
		return nullptr;
	}

	std::shared_ptr<Material> getMaterialDefferedGeoPass(const MaterialInfo& mInfo,bool hasNodeAnimation=false);

	std::shared_ptr<Material> getMaterialDefferedLightPass(bool hasIBL);

	std::shared_ptr<Shader> getShader(const std::string& name) {
		auto it = mShaders.find(name);
		if (it != mShaders.end()) {
			return it->second;
		}
		return nullptr;
	}

	std::shared_ptr<Node> getNode(const std::string& name) {
		auto pit = mNodes.find(name);
		if (pit != mNodes.end()) {
			return pit->second;
		}
		return nullptr;
	}

	std::shared_ptr<Animation> getAnimation(const std::string& name) {
		auto pit = gAnimations.find(name);
		if (pit != gAnimations.end()) {
			return pit->second;
		}
		return nullptr;
	}

	const std::unordered_map<std::string, std::shared_ptr<Node>>& getAllNode() {
		return mNodes;
	}

	std::shared_ptr<Texture> createTexture(const std::string& name, int width, int height,
		unsigned char* pdata,
		GLint internalFormat,
		GLint format,
		GLenum type,
		bool autoMipmap = false) {
		auto pTex = make_shared<Texture>();
		if (!pTex->create2DMap(width, height, pdata, internalFormat, format, type, 1, autoMipmap)) {
			LOGE("create a texture failed %s ", __func__);
			pTex.reset();
		}
		else {
			auto it = mTextures.try_emplace(name, pTex);
			if (!it.second) {
				LOGE("failed to add texture %s to gTexture,exist already", name.c_str());
				pTex.reset();
			}
		}
		return pTex;
	}

	static std::shared_ptr<Texture> getOrLoadTextureFromFile(const std::string& path, const std::string_view texName="");

	static std::shared_ptr<Texture> loadImageFromFile(const std::string& path, const std::string& texName);

	std::shared_ptr<Material> cloneMaterial(const std::string&);

	void parseRColors(const string& path);
	/*
	* 分析.material / .program文件
	* 每个.material / .program文件都会自动生成一个Material对象
	*/
	bool parseMaterialFile(const string& filePath);

	bool parseMeshCfgFile(const string& filePath);

	bool parseMeshCfg(const std::string& cfgValue);
	/*
	* parse 配置文件里面的material:name{matValue};
	* 内部会根据配置创建一个material
	*/
	bool parseMaterial(const std::string& matName, const std::string& matValue);

	/*
	* programName,program:后面跟的名字
	* program，要被分析的program的内容
	* 分析program，program:name{xxx},program的内容是xxx，
	* 创建名字为programName的shader，将shader放到gShader全局变量里面，
	* 拿到shader的各种uniform的loc，为shader里面的sampler指定纹理。
	*/
	bool parseProgram(std::shared_ptr<Material>& pMaterial, const string& program);

	/*
	* textureName  material文件里面texture：后面跟的纹理的名字
	* texture material文件里面texture的内容
	* texture的内容要么是一个path=xxxx，这样的路径
	* 要么就是长、宽、深度三个信息确定一个纹理。
	* 这个函数分析texture的内容，创建一个纹理，将纹理保存到gTexture全局变量里面
	*/
	bool parseTexture(const string& textureName, const string& texture);

	bool parseCubeTexture(const string& textureName, const string& texture);

	/*
	* 分析.material,.program程序里面的config配置项，只支持number与string
	*/
	bool parseConfig(const string& value);

	bool compileShader(std::shared_ptr<Material>& pMaterial, const string& programName, const std::string& vs, const std::string& fs);
	
	int getKeyAsInt(const string& key);

	const std::string_view getKeyAsStr(std::string_view key);
private:
	static bool programHandler(Material* pMat, const std::string& programName);
	static bool samplerHandler(Material* pMat, const std::string& samplerContent);
	static bool opHandler(Material* pMat, const std::string&);
	static bool opDepthHandler(Material* pMat, const std::string&);
	static bool opBlendHandler(Material* pMat, const std::string&);
	static bool opCullfaceHandler(Material* pMat, const std::string&);
	//program key value handler
	static bool posLocHandler(Material* pMat, const std::string&);
	static bool colorLocHandler(Material* pMat, const std::string&);
	static bool normalLocHandler(Material* pMat, const std::string&);
	static bool boneIdLocHandler(Material* pMat, const std::string&);
	static bool boneWeightLocHandler(Material* pMat, const std::string&);
	static bool texcoordLocHandler(Material* pMat, const std::string&);
	static bool modelMatrixHandler(Material* pMat, const std::string&);
	static bool texMatrixHandler(Material* pMat, const std::string&);
	static bool uniformColorHandler(Material* pMat, const std::string&);
	static bool materialUniformColorHandler(Material* pMat, const std::string&);
	static bool materialAlbedoColorHandler(Material* pMat, const std::string&);
	static bool albedoColorHandler(Material* pMat, const std::string&);
	static bool metallicHandler(Material* pMat, const std::string&);
	static bool roughnessHandler(Material* pMat, const std::string&);
	static bool aoHandler(Material* pMat, const std::string&);
	static bool materialMetallicHandler(Material* pMat, const std::string&);
	static bool materialRoughnessHandler(Material* pMat, const std::string&);
	static bool materialAoHandler(Material* pMat, const std::string&);
	static bool programSamplerHandler(Material* pMat, const std::string&);
	static bool programUboHandler(Material* pMat, const std::string&);
	static bool preMvpMatrixHandler(Material* pMat, const std::string&);

	static bool nodeNameHander(NodeSP& pNode, const std::string&);
	static bool meshPathHander(NodeSP& pNode, const std::string&);
	static bool meshTypeHander(NodeSP& pNode, const std::string&);
	static bool meshScaleHander(NodeSP& pNode, const std::string&);
	static bool meshMatInfoHander(NodeSP& pNode, const std::string&);
	static bool meshArmMapHander(MaterialInfo& info, const std::string&);
	static bool meshAlbedoMapHander(MaterialInfo& info, const std::string&);
	static bool meshNormalMapHander(MaterialInfo& info, const std::string&);
	static bool meshMetallicMapHander(MaterialInfo& info, const std::string&);
	static bool meshRoughnessMapHander( MaterialInfo& info, const std::string&);
	static bool meshAoMapHander(MaterialInfo& info, const std::string&);
	//static bool meshMaterialNameHander(MaterialInfo& info, const std::string&);
	

	static std::unordered_map<std::string, std::function<bool(Material* pMat, const std::string&)>> gMaterialHandlers;
	static std::unordered_map<std::string, std::function<bool(Material* pMat, const std::string&)>> gProgramKeyValueHandlers;
	static std::unordered_map<std::string, std::function<bool(NodeSP& pNode,const std::string&)>> gMeshKeyValueHandlers;
	static std::unordered_map<std::string, std::function<bool(MaterialInfo& info, const std::string&)>> gMaterialInfoHandlers;
	static unordered_map<string, Color> gRColors;
	static MapSMaterial mMaterials;
	static MapSTexture mTextures;
	static MapSShader mShaders;
	static MapSNode mNodes;
	static MapSAnimation gAnimations;
	static MapSMaterialInfo gMaterialInfos;
	std::unordered_map<std::string, std::string> mContents;//保存的是材质文件里面形如key{value}的key-value对
	std::unordered_map<std::string, std::string> mConfigValues;
	std::string_view mpVersion{ "#version 330 core\n" };
	std::string_view mpPrecision{ "precision highp float;\n" };
};

unordered_map<string, Color> ResourceImpl::gRColors;
MapSMaterial ResourceImpl::mMaterials;
MapSTexture ResourceImpl::mTextures;
MapSShader ResourceImpl::mShaders;
MapSNode ResourceImpl::mNodes;
MapSAnimation ResourceImpl::gAnimations;
MapSMaterialInfo ResourceImpl::gMaterialInfos;

/*
* program文件里面：program:testM{...}的处理函数
*/
std::unordered_map<std::string, std::function<bool(Material* pMat, const std::string&)>> ResourceImpl::gProgramKeyValueHandlers{
	{"posLoc",ResourceImpl::posLocHandler},
	{"texcoordLoc",ResourceImpl::texcoordLocHandler},
	{"colorLoc",ResourceImpl::colorLocHandler},
	{"normalLoc",ResourceImpl::normalLocHandler},
	{"boneIdLoc",ResourceImpl::boneIdLocHandler},
	{"boneWeightLoc",ResourceImpl::boneWeightLocHandler},
	{"modelMatrix",ResourceImpl::modelMatrixHandler},
	{"textureMatrix",ResourceImpl::texMatrixHandler},
	{"uniformColor",ResourceImpl::uniformColorHandler},
	{"albedo",ResourceImpl::albedoColorHandler},
	{"metallic",ResourceImpl::metallicHandler},
	{"roughness",ResourceImpl::roughnessHandler},
	{"ao",ResourceImpl::aoHandler},
	{"op",ResourceImpl::opHandler},
	{"sampler",ResourceImpl::programSamplerHandler},
	{"ubo",ResourceImpl::programUboHandler},
	{"preMvpMatrix",ResourceImpl::preMvpMatrixHandler}
};
/*
* material文件里面：material:testM{...}的处理函数
*/
std::unordered_map<std::string, std::function<bool(Material* pMat, const std::string&)>> ResourceImpl::gMaterialHandlers{
	{"program",ResourceImpl::programHandler},
	{"sampler",ResourceImpl::samplerHandler},
	{"op",ResourceImpl::opHandler},
	{"depthTest",ResourceImpl::opDepthHandler},
	{"blend",ResourceImpl::opBlendHandler},
	{"cullFace",ResourceImpl::opCullfaceHandler},

	{"uniformColor",ResourceImpl::materialUniformColorHandler},
	{"albedo",ResourceImpl::materialAlbedoColorHandler},
	{"metallic",ResourceImpl::materialMetallicHandler},
	{"roughness",ResourceImpl::materialRoughnessHandler},
	{"ao",ResourceImpl::materialAoHandler}
};
//.meshCfg文件的解析，解析这个文件加载mesh，
//meshCfg文件里面有mesh项，里面有matInfo，matInfo有个名字，约定使用albedoMap的文件名作为名字
//加载mesh的时候，一般的mesh文件里面也有材质信息，也指定了albedoMap/diffuseMap的文件名，
//就是通过这个albedo名字，让mesh匹配到自己的material
std::unordered_map<std::string, std::function<bool(NodeSP& pNode,const std::string&)>> ResourceImpl::gMeshKeyValueHandlers{
	{"nodeName",ResourceImpl::nodeNameHander},
	{"path",ResourceImpl::meshPathHander},
	{"meshType",ResourceImpl::meshTypeHander},
	{"scale",ResourceImpl::meshScaleHander},
	{"matInfo",ResourceImpl::meshMatInfoHander},
};

std::unordered_map<std::string, std::function<bool(MaterialInfo& info, const std::string&)>> ResourceImpl::gMaterialInfoHandlers{
	//{"materialName",ResourceImpl::meshMaterialNameHander},
	{"armMap",ResourceImpl::meshArmMapHander},
	{"albedoMap",ResourceImpl::meshAlbedoMapHander},
	{"normalMap",ResourceImpl::meshNormalMapHander},
	{"metallicMap",ResourceImpl::meshMetallicMapHander},
	{"roughnessMap",ResourceImpl::meshRoughnessMapHander},
	{"aoMap",ResourceImpl::meshAoMapHander}
};

bool ResourceImpl::nodeNameHander(NodeSP& pNode,const std::string& value) {
	if (pNode && !value.empty()) {
		mNodes.emplace(value, pNode);
	}
	return true;
}

bool ResourceImpl::meshTypeHander(NodeSP& pNode, const std::string& value) {
	if (pNode && !value.empty()) {
		MeshType type = MeshType::Quad;
		if (value == "quad") {
			type = MeshType::Quad;
		}
		else if (value == "cuboid") {
			type = MeshType::Cuboid;
		}
		else if (value == "shpere") {
			type = MeshType::Shpere;
		}
		else if (value == "cuboid") {
			type = MeshType::Cuboid;
		}
		auto pMesh = std::make_shared<Mesh>(MeshType::Quad);
		if (pMesh) {
			pMesh->loadMesh();
			pNode->addRenderable(pMesh);
		}
	}
	return true;
}

bool ResourceImpl::meshScaleHander(NodeSP& pNode,const std::string& value) {
	if (pNode && !value.empty()) {
		std::vector<std::string_view> scaleValue;
		Utils::splitStr(value, ",", scaleValue);
		if (scaleValue.size() != 3) {
			LOGE("meshScaleHander parse scale value error");
			return false;
		}
		try {
			float x, y, z;
			x = std::stof(std::string(scaleValue[0]));
			y = std::stof(std::string(scaleValue[1]));
			z = std::stof(std::string(scaleValue[2]));
			pNode->scale(glm::vec3(x, y, z));
		}
		catch (...) {
			LOGE("meshScaleHander parse scale value error");
			return false;
		}
	}
	return true;
}
bool ResourceImpl::meshPathHander(NodeSP& pNode,const std::string& value) {
	if (pNode && !value.empty()) {
		auto suffix = Utils::getFileSuffix(value);
		MeshLoader* pLoader=nullptr;
		MeshLoaderAssimp loader1;
		MeshLoaderGltf loader2;
		/*if (suffix == "gltf"sv) {
			pLoader = &loader2;
		}
		else {
			pLoader = &loader1;
		}*/
		pLoader = &loader1;
		if (pLoader->loadFromFile(value, pNode)) {
			LOGD("success load mesh from %s",value.c_str());
			return true;
		}
		else {
			LOGE(" to load mesh from path %s",value.c_str());
			return false;
		}
	}
	return false;
}
bool ResourceImpl::meshArmMapHander(MaterialInfo& info, const std::string& value) {
	if (!value.empty()) {
		info.armMap = value;
	}
	return true;
}
bool ResourceImpl::meshAlbedoMapHander(MaterialInfo& info, const std::string& value) {
	if (!value.empty()) {
		info.albedoMap = value;
		info.name = Utils::getFileName(value);
	}
	return true;
}
bool ResourceImpl::meshNormalMapHander(MaterialInfo& info, const std::string& value) {
	if (!value.empty()) {
		info.normalMap = value;
	}
	return true;
}

bool ResourceImpl::meshMetallicMapHander(MaterialInfo& info, const std::string& value) {
	if (!value.empty()) {
		info.metallicMap = value;
	}
	return true;
}

bool ResourceImpl::meshRoughnessMapHander(MaterialInfo& info, const std::string& value) {
	if (!value.empty()) {
		info.roughnessMap = value;
	}
	return true;
}

bool ResourceImpl::meshAoMapHander(MaterialInfo& info, const std::string& value) {
	if (!value.empty()) {
		info.aoMap = value;
	}
	return true;
}

bool ResourceImpl::meshMatInfoHander(NodeSP& pNode,const std::string& value) {
	if (!value.empty()) {
		vector<pair<std::string, std::string>> matInfoValue;
		MaterialInfo info1;
		if (Utils::parseItem(value, matInfoValue)) {
			for (const auto& pa : matInfoValue) {
				const auto itHandler = gMaterialInfoHandlers.find(pa.first);
				if (itHandler != gMaterialInfoHandlers.cend()) {
					itHandler->second(info1, pa.second);
				}
			}
			
			if (!gMaterialInfos.try_emplace(info1.name, info1).second) {
				LOGE("matInfo name duplicate in meshCfg file");
			}
			
			//info.name = info1.name;
		}
		return true;
	}
	return false;
}

//bool ResourceImpl::meshMaterialNameHander(MaterialInfo& info, const std::string& value) {
//	if (!value.empty()) {
//		info.name = value;
//		return true;
//	}
//	return false;
//}


bool ResourceImpl::posLocHandler(Material* pMat, const std::string& value) {
	try {
		int posLoc = std::stoi(value);
		pMat->getShader()->setPosLoc(posLoc);
	}
	catch (exception e) {
		LOGE("error to stoi posLoc,in material %s", pMat->getName().c_str());
	}
	return true;
}

bool ResourceImpl::colorLocHandler(Material* pMat, const std::string& value) {
	try {
		int posLoc = std::stoi(value);
		pMat->getShader()->setColorLoc(posLoc);
	}
	catch (exception e) {
		LOGE("error to stoi colorLoc,in material %s", pMat->getName().c_str());
	}
	return true;
}

bool ResourceImpl::normalLocHandler(Material* pMat, const std::string& value) {
	try {
		int norLoc = std::stoi(value);
		pMat->getShader()->setNormalLoc(norLoc);
	}
	catch (exception e) {
		LOGE("error to stoi normalLoc,in material %s", pMat->getName().c_str());
	}
	return true;
}

bool ResourceImpl::boneIdLocHandler(Material* pMat, const std::string& value) {
	try {
		int loc = std::stoi(value);
		pMat->getShader()->setBoneIdLoc(loc);
	}
	catch (exception e) {
		LOGE("error to stoi boneIdLoc,in material %s", pMat->getName().c_str());
	}
	return true;
}
bool ResourceImpl::boneWeightLocHandler(Material* pMat, const std::string& value) {
	try {
		int loc = std::stoi(value);
		pMat->getShader()->setBoneWeightLoc(loc);
	}
	catch (exception e) {
		LOGE("error to stoi BoneWeightLoc,in material %s", pMat->getName().c_str());
	}
	return true;
}


bool ResourceImpl::texcoordLocHandler(Material* pMat, const std::string& value) {
	try {
		int norLoc = std::stoi(value);
		pMat->getShader()->setTexLoc(norLoc);
	}
	catch (exception e) {
		LOGE("error to stoi texcoordLoc,in material %s", pMat->getName().c_str());
	}
	return true;
}


bool ResourceImpl::modelMatrixHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getModelMatrixLoc(value);
	}
	return true;
}

bool ResourceImpl::texMatrixHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getTextureMatrixLoc(value);
	}
	return true;
}

bool ResourceImpl::uniformColorHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getUniformColorLoc(value);
	}
	return true;
}

bool ResourceImpl::albedoColorHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getAlbedoColorLoc(value);
	}
	return true;
}
/*
* value=#xxxxxxxx或者@color/xxx
*/
bool ResourceImpl::materialUniformColorHandler(Material* pMat, const std::string& value) {
	if (!value.empty() && pMat) {
		std::string tempV;
		std::remove_copy(value.begin(), value.end(), tempV.begin(), '\20');
		Color color;
		if (Color::parseColor(tempV, color)) {
			pMat->setUniformColor(color);
			return true;
		}
		else {
			LOGE("to parse uniform color in material %s", pMat->getName().c_str());
		}
	}
	return false;
}

bool ResourceImpl::materialAlbedoColorHandler(Material* pMat, const std::string& value) {
	if (!value.empty() && pMat) {
		std::string tempV;
		std::remove_copy(value.begin(), value.end(), tempV.begin(), '\20');
		Color color;
		if (Color::parseColor(tempV, color)) {
			pMat->setAlbedoColor(color);
			return true;
		}
		else {
			LOGE("to parse albedo color in material %s", pMat->getName().c_str());
		}
	}
	return false;
}


bool ResourceImpl::materialMetallicHandler(Material* pMat, const std::string& value) {
	if (!value.empty() && pMat) {
		try {
			float f = std::stof(value);
			pMat->setMetallical(f);
			return true;
		}
		catch (...) {
			LOGE("stof failed when parse metallic in material %s", pMat->getName().c_str());
			return false;
		}
	}
	return false;
}

bool ResourceImpl::materialRoughnessHandler(Material* pMat, const std::string& value) {
	if (!value.empty() && pMat) {
		try {
			float f = std::stof(value);
			pMat->setRoughness(f);
			return true;
		}
		catch (...) {
			LOGE("stof failed when parse metallic in material %s", pMat->getName().c_str());
			return false;
		}
	}
	return false;
}

bool ResourceImpl::materialAoHandler(Material* pMat, const std::string& value) {
	if (!value.empty() && pMat) {
		try {
			float f = std::stof(value);
			pMat->setAo(f);
			return true;
		}
		catch (...) {
			LOGE("stof failed when parse metallic in material %s", pMat->getName().c_str());
			return false;
		}
	}
	return false;
}

bool ResourceImpl::metallicHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getMetallicLoc(value);
	}
	return true;
}

bool ResourceImpl::roughnessHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getRoughnessLoc(value);
	}
	return true;
}

bool ResourceImpl::aoHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getAoLoc(value);
	}
	return true;
}

bool ResourceImpl::programUboHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		vector<pair<string, string>> uniformBlock;
		//uniform block绑定到对于的uniform block bind point上
		if (Utils::parseItem(value, uniformBlock)) {
			auto& pShader = pMat->getShader();
			for (const auto& item : uniformBlock) {
				int bindPoint = -1;
				try {
					bindPoint = std::stoi(item.second);
				}
				catch (...) {
					LOGE("uniform block bindpoint is not number");
				}
				if (bindPoint >= 0) {
					pShader->bindUniformBlock(item.first.c_str(), bindPoint);
				}
			}
		}
	}
	return true;
}

bool ResourceImpl::preMvpMatrixHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getPreMvpMatrixLoc(value);
	}
	return true;
}

bool ResourceImpl::programSamplerHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		vector<pair<string,string>> umapSampler;
		//找到program里面列出来的sampler名字
		if (Utils::parseItem(value, umapSampler)) {
			if (!umapSampler.empty()) {
				auto& samplers = pMat->getShader()->getSamplerNames();
				auto& uniforms = pMat->getShader()->getUniforms();
				for (const auto& item : umapSampler) {
					//program脚本里面列出来的sampler，看看shader的uniform里面有没有
					if (uniforms.find(item.first) != uniforms.end()) {
						samplers.emplace_back(item.first);
					}
					else {
						LOGE("error in material file sampler2D %s not found in shader", item.first.c_str());
					}
					auto texName = Utils::getFileName(item.second);
					auto pTex = getOrLoadTextureFromFile(item.second, texName);
					int loc = pMat->getShader()->getUniformLoc(item.first.c_str()); //
					if (loc != -1) {
						if (pTex) {
							pMat->setTextureForSampler(loc, pTex);
						}
						else {
							pMat->setTextureForSampler(loc, shared_ptr<Texture>()); //
						}
					}
					else {
						LOGE("error in material file sampler2D %s not found in shader", item.first.c_str());
					}
				}
			}
		}
		else {
			LOGE("error to parse sampler in program %s", value.c_str());
		}
	}
	return true;
}

bool ResourceImpl::programHandler(Material* pMaterial, const std::string& programName) {
	auto it = mShaders.find(programName);
	if (it != mShaders.end()) {
		pMaterial->setShader(it->second);
		return true;
	}
	return false;
}

bool ResourceImpl::samplerHandler(Material* pMaterial, const std::string& samplerContent) {
	vector<pair<string, string>> contents;
	bool bret = false;
	do {
		if (Utils::parseItem(samplerContent, contents)) {
			for (auto& pairs : contents) {
				auto pTexture = getOrLoadTextureFromFile(pairs.second);
				if (pTexture) {
					pMaterial->setTextureForSampler(pairs.first.c_str(), pTexture);
					bret = true;
				}
				else {
					LOGE("parse material's sampler property error,cannot find or load texture %s", pairs.second.c_str());
				}
			}
		}
		else {
			LOGD("to parse material's sampler property: %s", samplerContent.c_str());
		}
	} while (false);
	return bret;
}

bool ResourceImpl::opHandler(Material* pMaterial, const std::string& opContent) {
	vector<pair<string, string>> contents;
	if (Utils::parseItem(opContent, contents)) {
		for (const auto& pairs : contents) {
			auto it = gMaterialHandlers.find(pairs.first);
			if (it != gMaterialHandlers.end()) {
				it->second(pMaterial, pairs.second);
			}
			else {
				LOGE("error parse material-op,cannot recognize key %s in material op", pairs.first.c_str());
			}
		}
	}
	else {
		LOGE("parse material's op property error,contents is %s", opContent.c_str());
		return false;
	}
	return true;
}

bool ResourceImpl::opDepthHandler(Material* pMaterial, const std::string& value) {
	if (value == "true") {
		pMaterial->setDepthTest(true);
	}
	else if (value == "false") {
		pMaterial->setDepthTest(false);
	}
	else {
		LOGE("syntax error in Material::opDepthHandler");
		return false;
	}
	return true;
}

bool ResourceImpl::opBlendHandler(Material* pMaterial, const std::string& value) {
	bool bEnable = false;
	unsigned int srcFactor = GL_SRC_ALPHA;
	unsigned int dstFactor = GL_ONE_MINUS_SRC_ALPHA;
	unsigned int equation = GL_FUNC_ADD;
	unsigned int srcFactorAlpha = GL_SRC_ALPHA;
	unsigned int dstFactorAlpha = GL_ONE_MINUS_SRC_ALPHA;
	unsigned int equationAlpha = GL_FUNC_ADD;
	bool hasError = false;
	do {
		if (value != "false") {
			std::vector<std::string_view> findResult;
			auto count = Utils::splitStr(value, ",", findResult);
			if (count != 4 && count != 7) {
				LOGE("blend op spell error,int material %s", pMaterial->getName().c_str());
				hasError = true;
				break;
			}

			if (findResult[0] == "true") {
				bEnable = true;
			}
			else if (findResult[0] == "false") {
				bEnable = false;
				break;
			}
			else {
				LOGE("blend op spell error,int material %s", pMaterial->getName().c_str());
				hasError = true;
				break;
			}
			auto srcPair = gBlendFuncMap.find(findResult[1]);
			if (srcPair != gBlendFuncMap.end()) {
				srcFactor = srcPair->second;
			}
			else {
				LOGE("blend op spell error,int material %s", pMaterial->getName().c_str());
				hasError = true;
				break;
			}

			auto dstPair = gBlendFuncMap.find(findResult[2]);
			if (dstPair != gBlendFuncMap.end()) {
				dstFactor = dstPair->second;
			}
			else {
				LOGE("blend op spell error,int material %s", pMaterial->getName().c_str());
				hasError = true;
				break;
			}

			auto equationPair = gBlendEquationMap.find(findResult[3]);
			if (equationPair != gBlendEquationMap.end()) {
				equation = equationPair->second;
			}
			else {
				LOGE("blend op spell error,int material %s", pMaterial->getName().c_str());
				hasError = true;
				break;
			}
			if (count == 7) {
				srcPair = gBlendFuncMap.find(findResult[4]);
				if (srcPair != gBlendFuncMap.end()) {
					srcFactorAlpha = srcPair->second;
				}
				else {
					LOGE("blend op spell error,int material %s", pMaterial->getName().c_str());
					hasError = true;
					break;
				}

				dstPair = gBlendFuncMap.find(findResult[5]);
				if (dstPair != gBlendFuncMap.end()) {
					dstFactorAlpha = dstPair->second;
				}
				else {
					LOGE("blend op spell error,int material %s", pMaterial->getName().c_str());
					hasError = true;
					break;
				}

				equationPair = gBlendEquationMap.find(findResult[6]);
				if (equationPair != gBlendEquationMap.end()) {
					equationAlpha = equationPair->second;
				}
				else {
					LOGE("blend op spell error,int material %s", pMaterial->getName().c_str());
					hasError = true;
					break;
				}
			}
			else {
				srcFactorAlpha = srcFactor;
				dstFactorAlpha = dstFactor;
				equationAlpha = equation;
			}
		}
	} while (false);

	if (hasError) {
		return false;
	}
	else {
		pMaterial->setBlend(bEnable, srcFactor, dstFactor, equation,
			srcFactorAlpha, dstFactorAlpha, equationAlpha);
		return true;
	}
}
bool ResourceImpl::opCullfaceHandler(Material* pMaterial, const std::string& value) {

	std::string bEnableStr;
	std::string cullWhichFaceStr;
	bool bEnable = false;
	int cullWhichFace = 1;
	bool hasError = false;
	do {
		auto pos = value.find(',');
		if (pos != std::string::npos) {
			bEnableStr = value.substr(0, pos);
			cullWhichFaceStr = value.substr(pos + 1);
			if (bEnableStr == "true") {
				bEnable = true;
			}
			else if (bEnableStr == "false") {
				bEnable = false;
			}
			else {
				hasError = true;
				break;
			}

			if (cullWhichFaceStr == "front") {
				cullWhichFace = GL_FRONT;
			}
			else if (cullWhichFaceStr == "back") {
				cullWhichFace = GL_BACK;
			}
			else if (cullWhichFaceStr == "frontAndBack") {
				cullWhichFace = GL_FRONT_AND_BACK;
			}
			else {
				hasError = true;
				break;
			}
		}
		else if (value == "false") {
			bEnable = false;
		}
		else {
			hasError = true;
			break;
		}
	} while (false);

	if (hasError) {
		LOGE("error to parse material-op-cullface value is %s", value.c_str());
		return false;
	}
	else {
		pMaterial->setCullWhichFace(bEnable, cullWhichFace);
		return true;
	}

}

Resource& Resource::getInstance() {
	static Resource gResource;
	return gResource;
}

Resource::Resource()
{
	mpImpl = make_unique<ResourceImpl>();
}

void Resource::loadAllMaterial() {
	mpImpl->loadAllMaterial();
}

Color Resource::getColor(const std::string& name) {
	return mpImpl->getColor(name);
}

std::shared_ptr<Texture> Resource::getTexture(const std::string& name) {
	return mpImpl->getTexture(name);
}

std::shared_ptr<Material> Resource::getMaterial(const std::string& name) {
	return mpImpl->getMaterial(name);
}
/*
* name: 可以是物体的名字
* mInfo：材质信息，根据里面的信息生成或者clone一个material对象
*/
std::shared_ptr<Material> Resource::getMaterialDefferedGeoPass(const MaterialInfo& mInfo, bool hasNodeAnimation) {
	return mpImpl->getMaterialDefferedGeoPass(mInfo, hasNodeAnimation);
}

std::shared_ptr<Material> Resource::getMaterialDefferedLightPass(bool hasIBL) {
	return mpImpl->getMaterialDefferedLightPass(hasIBL);
}

std::shared_ptr<Shader> Resource::getShader(const std::string& name) {
	return mpImpl->getShader(name);
}

std::shared_ptr<Node> Resource::getNode(const std::string& name) {
	return mpImpl->getNode(name);
}

const std::unordered_map<std::string, std::shared_ptr<Node>>& Resource::getAllNode() {
	return mpImpl->getAllNode();
}

//GL_RGB,GL_RGB,GL_UNSIGNED_BYTE
std::shared_ptr<Texture> Resource::createTexture(const std::string& name, int width, int height,
	unsigned char* pdata,
	int internalFormat,
	int format,
	unsigned int type,
	bool autoMipmap) {
	return mpImpl->createTexture(name, width, height, pdata, internalFormat, format, type, autoMipmap);
}

std::shared_ptr<Texture> Resource::getOrLoadTextureFromFile(const std::string& path, const std::string& texName) {
	return mpImpl->getOrLoadTextureFromFile(path, texName);
}

std::shared_ptr<Texture> Resource::loadImageFromFile(const std::string& path, const std::string& texName) {
	return mpImpl->loadImageFromFile(path, texName);
}
std::shared_ptr<Material> Resource::cloneMaterial(const std::string& name) {
	return mpImpl->cloneMaterial(name);
}
/*
* 分析配置文件里面的：material:matName{matValue}，内容
* 根据里面的内容生成名字为matName的material对象，保存到gMaterial里面。
*/
bool Resource::parseMaterial(const std::string& matName, const std::string& matValue) {
	return mpImpl->parseMaterial(matName, matValue);
}

/*
* 分析.material / .program文件
* 每个.material / .program文件都会自动生成一个Material对象
*/
bool Resource::parseMaterialFile(const string& filePath) {
	return mpImpl->parseMaterialFile(filePath);
}

bool Resource::parseMeshCfgFile(const string& filePath) {
	return mpImpl->parseMeshCfgFile(filePath);
}
//如果材质文件里面有个key对应的value是整数，可以用这个函数获取到
int Resource::getKeyAsInt(const string& key) {
	return mpImpl->getKeyAsInt(key);
}

const std::string_view Resource::getKeyAsStr(std::string_view key) {
	return mpImpl->getKeyAsStr(key);
}

bool ResourceImpl::parseMeshCfgFile(const string& filePath) {
	bool bParseSuccess = false;
	if (filePath.empty()) {
		return false;
	}

	ifstream meshFile(filePath);
	string meshCfg((std::istreambuf_iterator<char>(meshFile)), std::istreambuf_iterator<char>());

	std::string::size_type startPos = 0;
	std::string::size_type keyValuePos[3];
	std::string key;
	std::string value;
	while (Utils::findkeyValue(meshCfg, "{", "}", startPos, keyValuePos, key, value)) {
		if (key == "mesh") {
			parseMeshCfg(value);
		}
		else {
			LOGD("cannot parse the key %s in meshCfg",key.c_str());
		}
	}
	return true;
}

/*
* 解析.meshcfg文件里面的一块mesh{};
* 一个mesh{}块，会生成一个node，node下面会挂诺干个mesh。
* 每个mesh会有一个material
*/
bool ResourceImpl::parseMeshCfg(const std::string& cfgValue) {
	if (!cfgValue.empty()) {
		vector<pair<string, string>> meshValue;
		//这个node会放在全局变量里面，这个node attach了mesh
		//解析出来的material也会放在全局变量里面，所在这三个都不会自动被释放。
		NodeSP pNode = std::make_shared<Node>();
		//MaterialInfo info;
		if (Utils::parseItem(cfgValue, meshValue)) {
			//处理mesh{}里面的nodeName,matInfo等两项。
			for (const auto& p : meshValue) {
				const auto itHandler = gMeshKeyValueHandlers.find(p.first);
				if (itHandler != gMeshKeyValueHandlers.cend()) {
					itHandler->second(pNode, p.second);
				}
			}
		}
		auto& res = Resource::getInstance();
		pNode->visitNode([&res](Node* pNode) {
			if (pNode!=nullptr) {
				auto& pRend = pNode->getRenderables();
				if (pRend.empty()) {
					return;
				}
				for (const auto& it : pRend) {
					MeshSP pMesh = std::dynamic_pointer_cast<Mesh>(it.second);
					if (pMesh) {
						const std::string& matName = pMesh->getMaterialName();
						auto it = gMaterialInfos.find(matName);
						if (matName.empty() || it == gMaterialInfos.end()) {
							//给一个默认的材质
							MaterialInfo info;
							auto pMaterial = res.getMaterialDefferedGeoPass(info, pMesh->hasAnimation());
							pMesh->setMaterial(pMaterial);
						}
						else {
							//解析并创建组合出来的material，组合出来的material也有个名字，
							//getMaterialDefferedGeoPass会把组合出来的shader按照组合出来的名字insert到map
							auto& matInfo = it->second;
							MaterialSP pMat = Resource::getInstance().getMaterialDefferedGeoPass(matInfo,pMesh->hasAnimation());
							if (pMat && !matInfo.name.empty()) {
								//按照配置文件里面的名字再insert到map
								mMaterials.emplace(matInfo.name, pMat);
							}
							pMesh->setMaterial(pMat);
						}
					}
				}
			}
		});
	}
	return true;
}

void ResourceImpl::parseRColors(const string& path) {
	unique_ptr<rapidxml::file<>> pfdoc;
	try {
		pfdoc = make_unique<rapidxml::file<>>(path.c_str());
	}
	catch (std::exception e) {
		LOGE("error to parseRStrings %s file,error %s", path.c_str(), e.what());
		return;
	}
	if (pfdoc && pfdoc->size() > 0) {
		rapidxml::xml_document<> doc;// character type defaults to char
		doc.parse<0>(pfdoc->data());// 0 means default parse flags
		auto pResNode = doc.first_node("resources");
		if (pResNode != nullptr) {
			auto pColorNode = pResNode->first_node("color");
			while (pColorNode != nullptr) {
				auto attribute = pColorNode->first_attribute("name");
				if (attribute != nullptr) {
					string key = attribute->value();
					string value = pColorNode->value();//value is like #ffffffaa
					if (!key.empty() && !value.empty()) {
						Color color;
						if (Color::parseColor(value, color)) {
							auto ret = gRColors.try_emplace(key, color);
							if (!ret.second) {
								LOGD("there already has color who's name is %s in gRColors", key.c_str());
							}
						}
						else {
							LOGE("parse color %s in colors.xml", key.c_str());
						}
					}
				}
				pColorNode = pColorNode->next_sibling();
			}
		}
	}
	else {
		LOGE("error to parse resource string file %s", path.c_str());
	}
}

bool ResourceImpl::parseMaterialFile(const string& filePath) {
	bool bParseSuccess = false;
	if (filePath.empty()) {
		return false;
	}

	ifstream matFile(filePath);
	string material((std::istreambuf_iterator<char>(matFile)), std::istreambuf_iterator<char>());

	std::string::size_type startPos = 0;
	std::string::size_type keyValuePos[3];
	std::string programKey;
	std::string programName;
	std::vector<std::string> materialKeys;
	std::string key;
	std::string value;
	while (Utils::findkeyValue(material, "{", "}", startPos, keyValuePos,key,value)) {
		//analsys，store key-value to mpContents;
		if (!key.empty()&&!value.empty()) {
			string realKey, keyName;
			Utils::splitKeyValue(key, realKey, keyName);
			if (!mContents.try_emplace(key, value).second) {
				LOGE("double key %s,in %s material file", key.c_str(), filePath.c_str());
			}
			else if (realKey == "texture") {
				if (!keyName.empty()) {
					bParseSuccess = parseTexture(keyName, value);
				}
				else {
					LOGE("the texture in material file %s must has a name", filePath.c_str());
				}
			}
			else if (realKey == "cubeTexture") {
				if (!keyName.empty()) {
					bParseSuccess = parseCubeTexture(keyName, value);
				}
				else {
					LOGE("the cubetexture in material file %s must has a name", filePath.c_str());
				}
			}
			else if (realKey == "program") {
				programKey = key;
				programName = keyName;
			}
			else if (realKey == "material") {
				materialKeys.emplace_back(key);
			}
			else if (realKey == "config") {
				parseConfig(value);
			}
		}
		key.clear();
		value.clear();
	}

	if (!programKey.empty()) {
		auto it = mContents.find(programKey);
		if (it != mContents.end()) {
			if (!programName.empty()) {
				auto pMaterial = make_shared<Material>(programName);
				const auto ptrVs = mContents.find("vs");
				const auto ptrFs = mContents.find("fs");
				if (ptrVs != mContents.cend() && ptrFs != mContents.cend()) {
					bParseSuccess = compileShader(pMaterial,programName, ptrVs->second, ptrFs->second);
					if (bParseSuccess) {
						bParseSuccess = parseProgram(pMaterial,it->second);
						if (bParseSuccess) {
							if (mMaterials.try_emplace(programName, pMaterial).second) {
								LOGD("success to parse material %s", programName.c_str());
							}
							else {
								LOGD("failed to parse material %s", programName.c_str());
							}
						}
					}
					else {
						LOGE("compile shader failed in file %s", filePath.c_str());
					}
				}

			}
			else {
				LOGE("program has no name in %s material", filePath.c_str());
			}
		}
	}

	
	//parse Material
	for (auto& key : materialKeys) {
		auto it = mContents.find(key);
		if (it != mContents.end()) {
			string material, matName;
			Utils::splitKeyValue(key, material, matName);
			bParseSuccess = parseMaterial(matName, it->second);
		}
	}
	mContents.clear();//
	return bParseSuccess;
}

bool ResourceImpl::parseMaterial(const std::string& matName, const std::string& matValue) {
	vector<pair<string,string>> vec;
	if (Utils::parseItem(matValue, vec)) {
		auto pMaterial = std::make_shared<Material>(matName);
		for (const auto& pair : vec) {
			auto it = gMaterialHandlers.find(pair.first);
			if (it != gMaterialHandlers.end()) {
				if (!it->second(pMaterial.get(), pair.second))
				{
				}
			}
		}
		if (mMaterials.try_emplace(matName, pMaterial).second) {
			LOGD("success to add material %s to gMaterials", matName.c_str());
		}
		else {
			LOGD("failed to add material %s to gMaterials", matName.c_str());
		}
		return true;
	}
	else {
		LOGE("parse %s material script error,syntax error", matName.c_str());
		return false;
	}
}

bool ResourceImpl::parseProgram(std::shared_ptr<Material>& pMaterial, const string& program)
{
	vector<pair<string,string>> vec;
	bool bsuccess = false;
	if (Utils::parseItem(program, vec)) {
		for (auto& pair : vec) {
			auto it = gProgramKeyValueHandlers.find(pair.first);
			if (it != gProgramKeyValueHandlers.end()) {
				it->second(pMaterial.get(), pair.second);
			}
		}
		bsuccess = true;
	}
	else {
		LOGE(" parseProgram failed,content is %s", program.c_str());
	}
	return bsuccess;
}

bool ResourceImpl::compileShader(std::shared_ptr<Material>& pMaterial,const string& programName, const std::string& vs, const std::string& fs) {
	bool bsuccess = false;

	auto pShader = std::make_shared<Shader>(programName);
	if (pShader->initShader(vs, fs)) {
		LOGD("initShader %s success", programName.c_str());
		if (mShaders.try_emplace(programName, pShader).second) {
			bsuccess = true;
			pMaterial->setShader(pShader);
		}
		else {
			LOGE("add shader %s to gShaders failed", programName.c_str());
		}
	}
	else {
		LOGE("initShader %s failed", programName.c_str());
	}

	return bsuccess;
}

bool ResourceImpl::parseCubeTexture(const string& textureName, const string& texture) {
	Umapss umap;
	if (Utils::parseItem(texture, umap)) {
		const auto pPath = umap.find("path");
		if (pPath != umap.cend()) {
			auto pTex = std::make_shared<Texture>();
			if (pTex->loadCubemap(gDrawablePath + "/" + pPath->second) && mTextures.try_emplace(textureName, pTex).second) {
				LOGD("success to parse texture %s from path", textureName.c_str());
			}
			else {
				LOGE("failed to load cubetexture %s from path", textureName.c_str());
			}
		}
	}
	else {
		LOGE("parseCubeTexture parse error %s", textureName.c_str());
		return false;
	}
	return true;
}

bool ResourceImpl::parseConfig( const string& value) {
	vector<pair<string, string>> vec;
	if (Utils::parseItem(value, vec)) {
		for (const auto& it : vec) {
			mConfigValues.emplace(it.first, it.second);
		}
	}
	return true;
}

bool ResourceImpl::parseTexture(const string& textureName, const string& texture) {
	Umapss umap;
	int width{ 0 };
	int height{ 0 };
	int depth{ 0 };
	TextureSP pTex;
	if (Utils::parseItem(texture, umap)) {
		const auto pPath = umap.find("path");
		if (pPath != umap.cend()) {
			pTex = Texture::loadImageFromFile(gDrawablePath + "/" + pPath->second);
			if (!pTex) {
				LOGE("failed to load texture from path %s", pPath->second.c_str());
				return false;
			}
			if (!mTextures.emplace(textureName, pTex).second) {
				LOGE("there already has %s in gTextures", textureName.c_str());
				return false;
			}
		}
		else {
			const auto pWidth = umap.find("width");
			const auto pHeight = umap.find("height");
			const auto pDepth = umap.find("depth");

			if (pWidth != umap.cend() && pHeight != umap.cend() && pDepth != umap.cend()) {
				try {
					width = std::stoi(pWidth->second);
					height = std::stoi(pHeight->second);
					depth = std::stoi(pDepth->second);
				}
				catch (const logic_error& e) {
					LOGE("parseTexture throw exception %s from stoi() width height depth", e.what());
					return false;
				}
				int internalFormat = GL_RGB;
				int format = GL_RGB;
				if (depth == 4) {
					internalFormat = GL_RGBA;
					format = GL_RGBA;
				}
				else if (depth == 1) {
					internalFormat = GL_R8;
					format = GL_RED;
				}
				else if (depth != 3) {
					LOGE("not support texture %s depth %d,", textureName.c_str(), depth);
					return false;
				}

				pTex = std::make_shared<Texture>();
				if (pTex->create2DMap(width, height, nullptr, internalFormat, format) &&
					mTextures.try_emplace(textureName, pTex).second) {
					LOGD("success to create texture %s", textureName.c_str());
					return false;
				}
				else {
					LOGE("error to create texture %s", textureName.c_str());
					return false;
				}
			}
		}
		if (pTex) {
			const auto pSampler = umap.find("sampler");
			const auto pBorderColor = umap.find("borderColor");
			if (pSampler != umap.cend()) {
				int index{0};
				std::string_view str{ pSampler->second };
				auto [ptr, ec]{std::from_chars(str.data(), str.data() + str.size(), index)};
				if (ec == std::errc()){
					if (index < static_cast<int>(SamplerType::End)) {
						auto pSampler = Sampler::getSampler(static_cast<SamplerType>(index));
						pTex->setSampler(pSampler);
					}else {
						LOGE("parse texture sampler index overflow");
					}
				}else{
					LOGE("parse texture sampler index");
				}
			}
			if (pBorderColor != umap.cend()) {
				Color border;
				if (Color::parseColor(pBorderColor->second, border)){
					pTex->setBorderColor(border);
				}else {
					LOGE("parse texture borderColor error");
				}
			}
		}
	}
	else {
		LOGE("parseTexture error");
		return false;
	}
	return true;
}

const std::string_view ResourceImpl::getKeyAsStr(std::string_view key) {
	auto it = mConfigValues.find(std::string(key));
	if (it != mConfigValues.end()) {
		return it->second;
	}
	return std::string_view();
}

int ResourceImpl::getKeyAsInt(const string& key) {
	auto it = mConfigValues.find(key);
	int ret = -1;
	if (it != mConfigValues.end()) {
		try {
			int temp = std::stoi(it->second);
			return temp;
		}
		catch (exception e) {
		}
	}
	return ret;
}

std::shared_ptr<Material> ResourceImpl::cloneMaterial(const std::string& name) {
	auto& pMaterial = getMaterial(name);
	if (pMaterial) {
		return std::make_shared<Material>(*pMaterial);
	}
	else {
		return nullptr;
	}
}

std::shared_ptr<Texture> ResourceImpl::loadImageFromFile(const std::string& path, const std::string& texName) {
	std::shared_ptr<Texture> pTexture = Texture::loadImageFromFile(path);
	if (pTexture) {
		auto it = mTextures.try_emplace(texName, pTexture);
		if (!it.second) {
			LOGE("to emplace texture %s to gTexture,the path is %s", texName.c_str(), path.c_str());
			pTexture.reset();
		}
	}
	return pTexture;
}


std::shared_ptr<Texture> ResourceImpl::getOrLoadTextureFromFile(const std::string& path, const std::string_view texName)
{
	if (texName == std::string_view("none")) {
		return nullptr;
	}
	std::string RealTexName(texName);

	if (RealTexName.empty()) {
		RealTexName = Utils::getFileNameWithPath(path);
	}
	auto pTexture = mTextures.find(RealTexName);
	if (pTexture != mTextures.end()) {
		return pTexture->second;
	}
	else {
		//如果写的是drawable文件夹里面的某个文件
		auto filePath = gDrawablePath + "/" + path;
		auto pTex = loadImageFromFile(filePath, RealTexName);
		if (!pTex){
			LOGE("load texture failed %s",filePath.c_str());
		}
		return pTex;
	}
}

std::shared_ptr<Material> ResourceImpl::getMaterialDefferedLightPass(bool hasIBL) {
	std::string materialName = "defferedLightPass";
	if (hasIBL) {
		materialName += "_ibl";
	}
	//先找，没有再合成
	auto pMaterial = cloneMaterial(materialName);
	if (pMaterial) {
		return pMaterial;
	}
	else {
		pMaterial = make_shared<Material>();
	}
	std::string_view hasIBLMap{"#define HAS_IBL 1\n"};
	std::string_view hasShadowMap{ "#define HAS_SHADOW 1\n" };
	std::string vs;
	std::string fs;
	std::string program{"posLoc=0\ntexcoordLoc=1\n"};
	std::string ubo{"ubo{\nLights=1\n}"};
	fs += mpVersion;
	fs += mpPrecision;

	program += "sampler{\nposMap=none\nalbedoMap=none\nnormalMap=none\nssaoMap=none\n";	
	if (hasIBL) {
		fs += hasIBLMap;
		program += "irrMap=none\nprefilterMap=none\nbrdfLUT=none\n";
	}
	if (Config::openShadowMap) {
		fs += hasShadowMap;
		program += "shadowResult=none\n";
	}
	program += "}\n";
	program += ubo;

	vs = getKeyAsStr("defferedLightVs");
	auto dlfs = getKeyAsStr("defferedLightFs");
	fs += dlfs;
	std::string fstemp = fs.c_str();
	//LOGD(" VS:%s", vs.c_str());
	//LOGD(" FS:%s", fs.c_str());
	//LOGD(" program:%s", program.c_str());
	if (compileShader(pMaterial, materialName, vs, fs)) {
		if (parseProgram(pMaterial, program)) {
			if (!mMaterials.emplace(materialName, pMaterial).second) {
				LOGE("there are already exist %s material in gMaterials when call Material::getMaterial", materialName.c_str());
			}
		}
		else {
			LOGE(" parseProgram when call Material::getMaterial");
			pMaterial.reset();
		}
	}
	else {
		LOGE(" compile shader failed when  call Material::getMaterial");
		pMaterial.reset();
	}
	checkglerror();
	return pMaterial;
}

std::shared_ptr<Material> ResourceImpl::getMaterialDefferedGeoPass(const MaterialInfo& mInfo, bool hasNodeAnimation) {
	/*
	* 材质信息标志
	* 0: 表示是否有纹理
	* 1: albedo标志,0表示固定颜色，1表示albedo map
	* 2: normal标志，0表示顶点normal，1表示normal map
	* 3：metellic标志，0表示固定值，1表示metellic map
	* 4: roughness表示，0表示固定值，1表示roughness map
	* 5：ao标志，0表示固定值，1表示ao map
	* 6: nodeAnimation flag,1 has nodeAnimation,0 no
	*
	* 根据mInfo里面的信息计算出materialFlag，然后看看名字“defferedGeomertry_标志位”的纹理是否已经存在，
	* 如果已经存在就clone这个material，然后为它设置好各种纹理
	* 如果不存在，就根据mInfo里面的信息合成一个shader，material以名字“defferedGeomertry_标志位”保存起来。
	*/

	unsigned int materialFlag = 0;
	std::string vs;
	std::string fs;
	std::string program;
	std::string programSampler;
	std::string programUbo{ "ubo{\nMatrixes=0\n" };

	program = string_view("posLoc=0\n");
	program += string_view("\nmodelMatrix=modelMat\n");
	if (hasNodeAnimation) {
		program += string_view("boneIdLoc=3\nboneWeightLoc=4\n");
		programUbo += string_view("Bones=5\n");
	}
	if (Config::openTaa) {
		program += string_view("preMvpMatrix=preMvpMat\n");
		programUbo += string_view("ScreenWH = 2\nTaa=3\n}\n");
	}
	else {
		programUbo += string_view("}\n");
	}
	programSampler = string_view("sampler{\n");
	

	//先计算出标志，确定material的名字，然后在gMaterial里面找，能找到就用现成的
	if (!mInfo.albedoMap.empty()) {
		materialFlag |= 0x02;
	}
	if (!mInfo.normalMap.empty()) {
		materialFlag |= 0x04;
	}
	if (!mInfo.armMap.empty()) {
		materialFlag |= 0x08;
		materialFlag |= 0x10;
		materialFlag |= 0x20;
	}
	else {
		if (!mInfo.metallicMap.empty()) {
			materialFlag |= 0x08;
		}
		if (!mInfo.roughnessMap.empty()) {
			materialFlag |= 0x10;
		}
		if (!mInfo.aoMap.empty()) {
			materialFlag |= 0x20;
		}
	}
	if (hasNodeAnimation) {
		materialFlag |= 0x40;
	}

	std::stringstream tempss;
	tempss << "defferedGeomertry_" << materialFlag;
	auto materialName = tempss.str();
	auto destMat = cloneMaterial(materialName);
	//如果找到了现成的，为他们设置好参数
	if (destMat) {
		if (!mInfo.albedoMap.empty()) {
			auto pTex = getOrLoadTextureFromFile(mInfo.albedoMap);
			destMat->setTextureForSampler("albedoMap", pTex);
		}
		else {
			Color c;
			Color::parseColor(mInfo.albedo, c);
			destMat->setAlbedoColor(c);
		}
		if (!mInfo.normalMap.empty()) {
			auto pTex = getOrLoadTextureFromFile(mInfo.normalMap);
			destMat->setTextureForSampler("normalMap", pTex);
		}
		if (!mInfo.armMap.empty()) {
			auto pTex = getOrLoadTextureFromFile(mInfo.armMap);
			destMat->setTextureForSampler("armMap", pTex);
		}
		else {
			if (!mInfo.metallicMap.empty()) {
				auto pTex = getOrLoadTextureFromFile(mInfo.metallicMap);
				destMat->setTextureForSampler("metallicMap", pTex);
			}
			else {
				destMat->setMetallical(mInfo.metallic);
			}
			if (!mInfo.roughnessMap.empty()) {
				auto pTex = getOrLoadTextureFromFile(mInfo.roughnessMap);
				destMat->setTextureForSampler("roughnessMap", pTex);
			}
			else {
				destMat->setRoughness(mInfo.roughness);
			}
			if (!mInfo.aoMap.empty()) {
				auto pTex = getOrLoadTextureFromFile(mInfo.aoMap);
				destMat->setTextureForSampler("aoMap", pTex);
			}
			else {
				destMat->setAo(mInfo.ao);
			}
		}
		return destMat;
	}

	std::string allDefine;
	std::string_view hasTaa{"#define HAS_TAA 1\n"};
	std::string_view hasMap{ "#define HAS_MAP 1\n" };
	std::string_view hasNormalMap{"#define HAS_NORMAL_MAP 1\n"};
	std::string_view hasAlbedoMap{ "#define HAS_ALBEDO_MAP 1\n" };
	std::string_view hasArmMap{ "#define HAS_ARM_MAP 1\n" };
	std::string_view hasMetallicMap{ "#define HAS_METALLIC_MAP 1\n" };
	std::string_view hasRoughnessMap{ "#define HAS_ROUGHNESS_MAP 1\n" };
	std::string_view hasAoMap{ "#define HAS_AO_MAP 1\n" };
	std::string_view hasShadow{ "#define HAS_SHADOW 1\n" };
	std::string_view hasNodeAnimationDef{ "#define HAS_NODE_ANIMATION 1\n" };

	if (!mInfo.albedoMap.empty()) {
		materialFlag |= 0x02;
		allDefine += hasAlbedoMap;
		programSampler += "albedoMap="sv;
		programSampler += mInfo.albedoMap;
		programSampler += "\n"sv;
	}
	else {
		program += "albedo=albedo\n"sv;
	}

	if (!mInfo.normalMap.empty()) {
		allDefine += hasNormalMap;
		
		programSampler += "normalMap="sv;
		programSampler += mInfo.normalMap;
		programSampler += "\n"sv;
		materialFlag |= 0x04;
	}

	if (!mInfo.armMap.empty()) {
		allDefine += hasArmMap;
		programSampler += "armMap="sv;
		programSampler += mInfo.armMap;
		programSampler += "\n"sv;
		materialFlag |= 0x08;
		materialFlag |= 0x10;
		materialFlag |= 0x20;
	}
	else {
		if (!mInfo.metallicMap.empty()) {
			allDefine += hasMetallicMap;
			programSampler += "metallicMap="sv;
			programSampler += mInfo.metallicMap;
			programSampler += "\n"sv;
			materialFlag |= 0x08;
		}
		else {
			program += "metallic=metallic\n"sv;
		}

		if (!mInfo.roughnessMap.empty()) {
			allDefine += hasRoughnessMap;
			programSampler += "roughnessMap="sv;
			programSampler += mInfo.roughnessMap;
			programSampler += "\n"sv;
			materialFlag |= 0x10;
		}
		else {
			program += "roughness=roughness\n"sv;
		}
		if (!mInfo.aoMap.empty()) {
			allDefine += hasAoMap;
			programSampler += "aoMap="sv;
			programSampler += mInfo.aoMap;
			programSampler += "\n"sv;
			materialFlag |= 0x20;
		}
		else {
			program += "ao=ao\n"sv;
		}
	}
	if (Config::openShadowMap) {
		programSampler += "shadowMap="sv;
		programSampler += "none"sv;
		programSampler += "\n"sv;
	}

	programSampler += "}\n"sv;
	
	fs += mpVersion;
	fs += mpPrecision;
	vs += mpVersion;
	vs += mpPrecision;
	if (materialFlag != 0) {
		fs += hasMap;
		vs += hasMap;
		program += "texcoordLoc=1\nnormalLoc=2\n"sv;
		if (Config::openTaa) {
			fs += hasTaa;
			vs += hasTaa;
		}
	}
	else {
		program += "normalLoc=1\n"sv;
	}
	program += programSampler;
	program += programUbo;
	if (Config::openShadowMap) {
		fs += hasShadow;
		vs += hasShadow;
	}
	if (hasNodeAnimation) {
		vs += hasNodeAnimationDef;
	}
	fs += allDefine;
	vs += getKeyAsStr("defferedGeoVs");
	fs += getKeyAsStr("defferedGeoFs");

	//fs 搞定了
	auto pMaterial = std::make_shared<Material>();
	
	if (compileShader(pMaterial,materialName, vs, fs)) {
		if (parseProgram(pMaterial,program)) {
			if (!mMaterials.emplace(materialName, pMaterial).second) {
				LOGE("there are already exist %s material in gMaterials when call Material::getMaterial", materialName.c_str());
			}
			else {
				//clone一份
				auto destMat = cloneMaterial(materialName);
				return destMat;
			}
		}
		else {
			LOGE(" parseProgram when call Material::getMaterial");
			pMaterial.reset();
		}
	}
	else {
		LOGE(" compile shader failed when  call Material::getMaterial");
		pMaterial.reset();
	}
	checkglerror();
	return pMaterial;
}

void ResourceImpl::loadAllMaterial() {
	parseRColors(gColorsPath);
	path programPath(gProgramPath);
	if (!exists(gProgramPath)) {
		LOGE("ERROR the program path %s is not exist", gMaterialPath.c_str());
	}
	else {
		if (is_directory(gProgramPath)) {
			//是目录
			directory_iterator list(gProgramPath);
			//directory_entry 是一个文件夹里的某一项，可以是path，也可以是文件
			for (auto& it : list) {
				auto filePath = it.path();
				if (is_regular_file(filePath)) {
					//是文件
					auto filePathString = filePath.string();
					parseMaterialFile(filePathString);
				}
			}
		}
	}

	path materialPath(gMaterialPath);
	if (!exists(materialPath)) {
		LOGE("ERROR the ui image path %s is not exist", gMaterialPath.c_str());
	}
	else {
		if (is_directory(materialPath)) {
			//是目录
			directory_iterator list(materialPath);
			//directory_entry 是一个文件夹里的某一项，可以是path，也可以是文件
			for (auto& it : list) {
				auto filePath = it.path();
				if (is_regular_file(filePath)) {
					//是文件
					auto filePathString = filePath.string();
					parseMaterialFile(filePathString);
				}
			}
		}
	}

	auto pTex = genBrdfLut();
	if (pTex) {
		mTextures.emplace("brdfLUT", pTex);
	}
	pTex = make_shared<Texture>();
	std::array<unsigned char, 4> nullTex{0,0,0,0};
	auto pSampler = Sampler::getSampler(SamplerType::NearNearEdgeEdge);
	pTex->setSampler(pSampler);
	pTex->create2DMap(1, 1, nullTex.data(), GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	mTextures.emplace("nullTex", pTex);
}