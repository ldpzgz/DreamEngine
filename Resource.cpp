#include "Resource.h"
#include "Material.h"
#include "Texture.h"
#include "Shader.h"
#include "Mesh.h"
#include "Log.h"
#include "helper.h"
#include "Utils.h"
#include "Node.h"
#include "MeshLoaderAssimp.h"
#include <filesystem>
#include <unordered_map>
#include <sstream>
#include <fstream>
#include <any>

using namespace std;
using namespace std::filesystem;
using MapSMaterial = unordered_map<string, shared_ptr<Material>>;
using MapSTexture = unordered_map<string, shared_ptr<Texture>>;
using MapSShader = unordered_map<string, shared_ptr<Shader>>;
using MapSNode = unordered_map<string, SP_Node>;
using Umapss = unordered_map<string, string>;
extern void checkglerror();
static const string gMaterialPath = "./opengles3/resource/material";
static const string gProgramPath = "./opengles3/resource/program";
static const string gDrawablePath = "./opengles3/resource/drawable";

static std::unordered_map<std::string, unsigned int> gBlendFuncMap{
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

static std::unordered_map<std::string, unsigned int> gBlendEquationMap{
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

	std::shared_ptr<Texture> getTexture(const std::string& name);

	std::shared_ptr<Material> getMaterial(const std::string& name);

	std::shared_ptr<Material> getMaterial(const MaterialInfo& mInfo);

	std::shared_ptr<Shader> getShader(const std::string& name);

	SP_Node getNode(const std::string& name);

	std::shared_ptr<Texture> createTexture(const std::string& name, int width, int height,
		unsigned char* pdata,
		GLint internalFormat,
		GLint format,
		GLenum type,
		bool autoMipmap = false);

	static std::shared_ptr<Texture> getOrLoadTextureFromFile(const std::string& path, const std::string& texName="");

	static std::shared_ptr<Texture> loadImageFromFile(const std::string& path, const std::string& texName);

	std::shared_ptr<Material> cloneMaterial(const std::string&);

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
	bool parseConfig(const string& cfgName, const string& value);

	bool compileShader(std::shared_ptr<Material>& pMaterial, const string& programName, const std::string& vs, const std::string& fs);
	
	int getKeyAsInt(const string& key);
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
	static bool texcoordLocHandler(Material* pMat, const std::string&);
	static bool tangentLocHandler(Material* pMat, const std::string&);
	static bool projMatrixHandler(Material* pMat, const std::string&);
	static bool modelMatrixHandler(Material* pMat, const std::string&);
	static bool mvpMatrixHandler(Material* pMat, const std::string&);
	static bool mvMatrixHandler(Material* pMat, const std::string&);
	static bool viewMatrixHandler(Material* pMat, const std::string&);
	static bool texMatrixHandler(Material* pMat, const std::string&);
	static bool uniformColorHandler(Material* pMat, const std::string&);
	static bool materialUniformColorHandler(Material* pMat, const std::string&);
	static bool materialAlbedoColorHandler(Material* pMat, const std::string&);
	static bool albedoColorHandler(Material* pMat, const std::string&);
	static bool viewPosHandler(Material* pMat, const std::string&);
	static bool lightPosHandler(Material* pMat, const std::string&);
	static bool lightColorHandler(Material* pMat, const std::string&);
	static bool metallicHandler(Material* pMat, const std::string&);
	static bool roughnessHandler(Material* pMat, const std::string&);
	static bool aoHandler(Material* pMat, const std::string&);
	static bool materialMetallicHandler(Material* pMat, const std::string&);
	static bool materialRoughnessHandler(Material* pMat, const std::string&);
	static bool materialAoHandler(Material* pMat, const std::string&);
	static bool programSamplerHandler(Material* pMat, const std::string&);

	static bool meshNameHander(SP_Node& pNode, MaterialInfo& info, const std::string&);
	static bool meshPathHander(SP_Node& pNode, MaterialInfo& info, const std::string&);
	static bool meshArmMapHander(SP_Node& pNode, MaterialInfo& info, const std::string&);
	static bool meshAlbedoMapHander(SP_Node& pNode, MaterialInfo& info, const std::string&);
	static bool meshNormalMapHander(SP_Node& pNode, MaterialInfo& info, const std::string&);
	static bool meshMetallicMapHander(SP_Node& pNode, MaterialInfo& info, const std::string&);
	static bool meshRoughnessMapHander(SP_Node& pNode, MaterialInfo& info, const std::string&);
	static bool meshAoMapHander(SP_Node& pNode, MaterialInfo& info, const std::string&);

	static std::unordered_map<std::string, std::function<bool(Material* pMat, const std::string&)>> gMaterialHandlers;
	static std::unordered_map<std::string, std::function<bool(Material* pMat, const std::string&)>> gProgramKeyValueHandlers;
	static std::unordered_map<std::string, std::function<bool(SP_Node& pNode, MaterialInfo& info,const std::string&)>> gMeshKeyValueHandlers;

	static MapSMaterial mMaterials;
	static MapSTexture mTextures;
	static MapSShader mShaders;
	static MapSNode mMeshes;
	std::unordered_map<std::string, std::string> mContents;//保存的是材质文件里面形如key{value}的key-value对
	std::unordered_map<std::string, std::any> mConfigValues;
};

MapSMaterial ResourceImpl::mMaterials;
MapSTexture ResourceImpl::mTextures;
MapSShader ResourceImpl::mShaders;
MapSNode ResourceImpl::mMeshes;


/*
* program文件里面：program:testM{...}的处理函数
*/
std::unordered_map<std::string, std::function<bool(Material* pMat, const std::string&)>> ResourceImpl::gProgramKeyValueHandlers{
	{"posLoc",ResourceImpl::posLocHandler},
	{"texcoordLoc",ResourceImpl::texcoordLocHandler},
	{"colorLoc",ResourceImpl::colorLocHandler},
	{"normalLoc",ResourceImpl::normalLocHandler},
	{"tangentLoc",ResourceImpl::tangentLocHandler},
	{"projMatrix",ResourceImpl::projMatrixHandler},
	{"modelMatrix",ResourceImpl::modelMatrixHandler},
	{"mvpMatrix",ResourceImpl::mvpMatrixHandler},
	{"mvMatrix",ResourceImpl::mvMatrixHandler},
	{"viewMatrix",ResourceImpl::viewMatrixHandler},
	{"textureMatrix",ResourceImpl::texMatrixHandler},
	{"lightPos",ResourceImpl::lightPosHandler},
	{"lightColor",ResourceImpl::lightColorHandler},
	{"viewPos",ResourceImpl::viewPosHandler},
	{"uniformColor",ResourceImpl::uniformColorHandler},
	{"albedo",ResourceImpl::albedoColorHandler},
	{"sampler",ResourceImpl::programSamplerHandler},
	{"metallic",ResourceImpl::metallicHandler},
	{"roughness",ResourceImpl::roughnessHandler},
	{"ao",ResourceImpl::aoHandler},
	{"op",ResourceImpl::opHandler}
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

std::unordered_map<std::string, std::function<bool(SP_Node& pNode, MaterialInfo& info, const std::string&)>> ResourceImpl::gMeshKeyValueHandlers{
	{"name",ResourceImpl::meshNameHander},
	{"path",ResourceImpl::meshPathHander},
	{"armMap",ResourceImpl::meshArmMapHander},
	{"albedoMap",ResourceImpl::meshAlbedoMapHander},
	{"normalMap",ResourceImpl::meshNormalMapHander},
	{"metallicMap",ResourceImpl::meshMetallicMapHander},
	{"roughnessMap",ResourceImpl::meshRoughnessMapHander},
	{"aoMap",ResourceImpl::meshAoMapHander}
};

bool ResourceImpl::meshNameHander(SP_Node& pNode, MaterialInfo& info, const std::string& value) {
	if (pNode && !value.empty()) {
		mMeshes.emplace(value, pNode);
	}
	return true;
}
bool ResourceImpl::meshPathHander(SP_Node& pNode, MaterialInfo& info, const std::string& value) {
	if (pNode && !value.empty()) {
		MeshLoaderAssimp loader;
		if (loader.loadFromFile(value, pNode)) {
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
bool ResourceImpl::meshArmMapHander(SP_Node& pNode, MaterialInfo& info, const std::string& value) {
	if (!value.empty()) {
		info.armMap = value;
	}
	return true;
}
bool ResourceImpl::meshAlbedoMapHander(SP_Node& pNode, MaterialInfo& info, const std::string& value) {
	if (!value.empty()) {
		info.albedoMap = value;
	}
	return true;
}
bool ResourceImpl::meshNormalMapHander(SP_Node& pNode, MaterialInfo& info, const std::string& value) {
	if (!value.empty()) {
		info.normalMap = value;
	}
	return true;
}

bool ResourceImpl::meshMetallicMapHander(SP_Node& pNode, MaterialInfo& info, const std::string& value) {
	if (!value.empty()) {
		info.metallicMap = value;
	}
	return true;
}

bool ResourceImpl::meshRoughnessMapHander(SP_Node& pNode, MaterialInfo& info, const std::string& value) {
	if (!value.empty()) {
		info.roughnessMap = value;
	}
	return true;
}

bool ResourceImpl::meshAoMapHander(SP_Node& pNode, MaterialInfo& info, const std::string& value) {
	if (!value.empty()) {
		info.aoMap = value;
	}
	return true;
}


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

bool ResourceImpl::tangentLocHandler(Material* pMat, const std::string& value) {
	try {
		int norLoc = std::stoi(value);
		pMat->getShader()->setTangentLoc(norLoc);
	}
	catch (exception e) {
		LOGE("error to stoi tangentLoc,in material %s", pMat->getName().c_str());
	}
	return true;
}

bool ResourceImpl::projMatrixHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getProjMatrixLoc(value);
	}
	return true;
}

bool ResourceImpl::modelMatrixHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getModelMatrixLoc(value);
	}
	return true;
}

bool ResourceImpl::mvpMatrixHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getMvpMatrixLoc(value);
	}
	return true;
}

bool ResourceImpl::mvMatrixHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getMvMatrixLoc(value);
	}
	return true;
}

bool ResourceImpl::viewMatrixHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getViewMatrixLoc(value);
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


bool ResourceImpl::viewPosHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getViewPosLoc(value);
	}
	return true;
}

bool ResourceImpl::lightPosHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getLightPosLoc(value);
	}
	return true;
}

bool ResourceImpl::lightColorHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getLightColorLoc(value);
	}
	return true;
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

bool ResourceImpl::programSamplerHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		Umapss umapSampler;
		//找到program里面列出来的sampler名字
		if (Utils::parseItem(value, umapSampler)) {
			if (!umapSampler.empty()) {
				auto& samplers = pMat->getShader()->getSamplerNames();
				auto& uniforms = pMat->getShader()->getUniforms();
				for (auto& item : umapSampler) {
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
	Umapss contents;
	bool bret = false;
	do {
		if (Utils::parseItem(samplerContent, contents)) {
			for (auto& pairs : contents) {
				auto pTexture = getOrLoadTextureFromFile(pairs.second);
				if (pTexture) {
					pMaterial->setTextureForSampler(pairs.first, pTexture);
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
	Umapss contents;
	if (Utils::parseItem(opContent, contents)) {
		for (auto pairs : contents) {
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
			std::vector<std::string> findResult;
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
std::shared_ptr<Material> Resource::getMaterial(const MaterialInfo& mInfo) {
	return mpImpl->getMaterial(mInfo);
}

std::shared_ptr<Shader> Resource::getShader(const std::string& name) {
	return mpImpl->getShader(name);
}

SP_Node Resource::getNode(const std::string& name) {
	return mpImpl->getNode(name);
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

bool ResourceImpl::parseMeshCfg(const std::string& cfgValue) {
	if (!cfgValue.empty()) {
		std::vector<std::pair<std::string, std::string>> meshValue;
		//这个node会放在全局变量里面，这个node attach了mesh
			//解析出来的material也会放在全局变量里面，所在这三个都不会自动被释放。
		SP_Node pNode = std::make_shared< NodeMat4>();
		MaterialInfo info;
		if (Utils::parseItem(cfgValue, meshValue)) {
			//处理meshCfg配置项
			for (const auto& p : meshValue) {
				const auto itHandler = gMeshKeyValueHandlers.find(p.first);
				if (itHandler != gMeshKeyValueHandlers.cend()) {
					itHandler->second(pNode, info, p.second);
				}
			}
			//给mesh创建material，
			MaterialP pMat = Resource::getInstance().getMaterial(info);
			//给每一个mesh都设置相同的材质
			if (pMat) {
				auto& attachments = pNode->getAttachments();
				for (const auto& pMesh : attachments) {
					auto pM = std::dynamic_pointer_cast<Mesh>(pMesh.second);
					if (pM) {
						pM->setMaterial(pMat);
					}
				}
			}
		}
	}
	return true;
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
			Utils::splitKeyAndName(key, realKey, keyName);
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
				parseConfig(keyName, value);
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
			Utils::splitKeyAndName(key, material, matName);
			bParseSuccess = parseMaterial(matName, it->second);
		}
	}
	mContents.clear();//
	return bParseSuccess;
}

bool ResourceImpl::parseMaterial(const std::string& matName, const std::string& matValue) {
	Umapss umap;
	if (Utils::parseItem(matValue, umap)) {
		auto pMaterial = std::make_shared<Material>(matName);
		for (auto& pair : umap) {
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
	Umapss umap;
	bool bsuccess = false;
	if (Utils::parseItem(program, umap)) {
		for (auto& pair : umap) {
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

bool ResourceImpl::parseConfig(const string& cfgName, const string& value) {
	Umapss umap;
	if (Utils::parseItem(value, umap)) {
		for (auto& it : umap) {
			auto pos = it.second.find_first_of('\"');
			if (pos != string::npos) {
				auto endPos = it.second.find_last_of('\"');
				if (endPos != string::npos && pos < endPos) {
					mConfigValues.emplace(it.first, it.second.substr(pos + 1, endPos - pos - 1));
				}
			}
			else {
				try {
					float temp = std::stof(it.second);
					mConfigValues.emplace(it.first, temp);
				}
				catch (exception e) {
				}
			}
		}
	}
	return true;
}

bool ResourceImpl::parseTexture(const string& textureName, const string& texture) {
	Umapss umap;
	int width;
	int height;
	int depth;
	if (Utils::parseItem(texture, umap)) {
		const auto pPath = umap.find("path");
		if (pPath != umap.cend()) {
			auto pTex = Texture::loadImageFromFile(gDrawablePath + "/" + pPath->second);
			if (!pTex) {
				LOGE("failed to load texture from path %s", pPath->second.c_str());
			}
			if (!mTextures.emplace(textureName, pTex).second) {
				LOGE("there already has %s in gTextures", textureName.c_str());
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

				auto pTex = std::make_shared<Texture>();
				if (pTex->create2DMap(width, height, nullptr, internalFormat, format) &&
					mTextures.try_emplace(textureName, pTex).second) {
					LOGD("success to create texture %s", textureName.c_str());
				}
				else {
					LOGE("error to create texture %s", textureName.c_str());
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

int ResourceImpl::getKeyAsInt(const string& key) {
	auto it = mConfigValues.find(key);
	int ret = -1;
	if (it != mConfigValues.end()) {
		try {
			ret = std::any_cast<float>(it->second);
		}
		catch (exception e) {
			LOGE("Material::getKeyAsInt error to conver %s", key.c_str());
			ret = -1;
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

std::shared_ptr<Texture> ResourceImpl::getOrLoadTextureFromFile(const std::string& path, const std::string& texName)
{
	std::string RealTexName = texName;
	if (RealTexName.empty()) {
		RealTexName = Utils::getFileName(path);
	}
	auto pTexture = mTextures.find(RealTexName);
	if (pTexture != mTextures.end()) {
		return pTexture->second;
	}
	else {
		//如果写的是drawable文件夹里面的某个文件
		auto filePath = gDrawablePath + "/" + path;
		auto pTex = loadImageFromFile(filePath, RealTexName);
		return pTex;
	}
}

std::shared_ptr<Shader> ResourceImpl::getShader(const std::string& name) {
	auto it = mShaders.find(name);
	if (it != mShaders.end()) {
		return it->second;
	}
	return nullptr;
}

std::shared_ptr<Texture> ResourceImpl::createTexture(const std::string& name, int width, int height,
	unsigned char* pdata,
	GLint internalFormat,
	GLint format,
	GLenum type,
	bool autoMipmap)
{
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

SP_Node ResourceImpl::getNode(const std::string& name) {
	auto pit = mMeshes.find(name);
	if (pit != mMeshes.end()) {
		return pit->second;
	}
	return nullptr;
}

std::shared_ptr<Material> ResourceImpl::getMaterial(const MaterialInfo& mInfo) {
	/*
	* 材质信息标志
	* 0: pos标志，总是1
	* 1: albedo标志,0表示固定颜色，1表示albedo map
	* 2: normal标志，0表示顶点normal，1表示normal map
	* 3：metellic标志，0表示固定值，1表示metellic map
	* 4: roughness表示，0表示固定值，1表示roughness map
	* 5：ao标志，0表示固定值，1表示ao map
	*
	* 根据mInfo里面的信息计算出materialFlag，然后看看名字“defferedGeomertry_标志位”的纹理是否已经存在，
	* 如果已经存在就clone这个material，然后为它设置好各种纹理
	* 如果不存在，就根据mInfo里面的信息合成一个shader，material以名字“defferedGeomertry_标志位”保存起来。
	*/

	unsigned int materialFlag = 1;
	std::string vs;
	std::string vsAttr;
	std::string vsUniform;
	std::string vsOut;
	std::string vsMain;

	std::string fs;
	std::string fsOut;
	std::string fsIn;
	std::string fsUniform;
	std::string fsGetNormal;
	std::string fsMain;
	std::string program;
	std::string programSampler;


	constexpr char* pVersion = "#version 330 core\n";
	constexpr char* pPrecision = "precision highp float;\n";

	constexpr char* getNormalFromMap = "vec3 getNormalFromMap(sampler2D nmap,vec2 texcoord,vec3 worldPos,vec3 normal){\n \
		vec3 tangentNormal = texture(nmap, texcoord).xyz * 2.0f - 1.0f;\n \
		vec3 Q1 = dFdx(worldPos); \n \
		vec3 Q2 = dFdy(worldPos); \n \
		vec2 st1 = dFdx(texcoord); \n \
		vec2 st2 = dFdy(texcoord); \n \
		vec3 N = normalize(normal);\n \
		vec3 T = normalize(Q1 * st2.t - Q2 * st1.t);\n \
		vec3 B = -normalize(cross(N, T));\n \
		mat3 TBN = mat3(T, B, N);\n \
		return normalize(TBN * tangentNormal);\n \
	}\n";

	constexpr char* pFsOut = "layout (location = 0) out vec4 outPosMap;\n \
		layout(location = 1) out vec4 outNormalMap;\n \
	layout(location = 2) out vec4 outAlbedoMap;\n";

	vsAttr = "layout(location = 0) in vec3 inPos;\n";
	program = "posLoc=0\n";

	vsUniform = "uniform mat4 projMat;\n";
	vsUniform += "uniform mat4 mvMat;\n";

	vsOut = "out vec3 worldPos;\n";
	vsOut += "out vec3 worldNormal;\n";

	vsMain = "void main(){\n \
			gl_Position = projMat * mvMat * vec4(inPos,1.0f);\n";

	fsMain = "void main(){\n \
		outPosMap.rgb = worldPos;\n";

	programSampler = "sampler{\n";

	fs += pVersion;
	fs += pPrecision;
	fsOut = pFsOut;
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

	if (!mInfo.albedoMap.empty()) {
		vsAttr += "layout(location = 1) in vec2 inTexcoord;\n";
		vsAttr += "layout(location = 2) in vec3 inNormal;\n";

		vsOut += "out vec2 fsTexcoord;\n";

		vsMain += "fsTexcoord = inTexcoord;\n";

		fsIn += "in vec2 fsTexcoord;\n";

		program += "texcoordLoc=1\n";
		program += "normalLoc=2\n";

		//fs相关的
		fsUniform += "uniform sampler2D albedoMap;\n";
		programSampler += "albedoMap=";
		programSampler += mInfo.albedoMap;
		programSampler += "\n";
		fsMain += "outAlbedoMap.rgb = texture(albedoMap,fsTexcoord).rgb;\n";
	}
	else {
		vsAttr += "layout(location = 1) in vec3 inNormal;\n";
		program += "normalLoc=1\n";
		//if (!mInfo.normalMap.empty()) {
		//	LOGE(" %s no albedomap,but has normal map",__func__);
		//	return nullptr;
		//}
		//fs相关的
		fsUniform += "uniform vec3 albedo;\n";
		fsMain += "outAlbedoMap.rgb = albedo;\n";
		program += "uniformColor=albedo\n";
	}

	vsMain += "worldNormal=mat3(mvMat)*inNormal;\n \
		worldPos = vec3(mvMat * vec4(inPos, 1.0));\n";
	vsMain += "}\n";

	vs += pVersion;
	vs += pPrecision;
	vs += (vsAttr + vsUniform + vsOut + vsMain);
	//vs 已经搞定
	fsIn += "in vec3 worldPos;\n \
		in vec3 worldNormal;\n";

	program += "projMatrix=projMat\n";
	program += "mvMatrix=mvMat\n";

	if (!mInfo.normalMap.empty()) {
		fsUniform += "uniform sampler2D normalMap;\n";
		programSampler += "normalMap=";
		programSampler += mInfo.normalMap;
		programSampler += "\n";
		fsGetNormal = getNormalFromMap;
		fsMain += "outNormalMap.rgb = getNormalFromMap(normalMap,fsTexcoord,worldPos,worldNormal);\n";
		materialFlag |= 0x04;
	}
	else {
		fsMain += "outNormalMap.rgb = normalize(worldNormal);\n";
	}

	if (!mInfo.armMap.empty()) {
		fsUniform += "uniform sampler2D armMap;\n";
		programSampler += "armMap=";
		programSampler += mInfo.armMap;
		programSampler += "\n";
		fsMain += "vec3 arm = texture(armMap,fsTexcoord).rgb;\n";
		fsMain += "outPosMap.a = arm.r;\n";
		fsMain += "outNormalMap.a = arm.b;\n";
		fsMain += "outAlbedoMap.a = arm.g;\n";
		materialFlag |= 0x08;
		materialFlag |= 0x10;
		materialFlag |= 0x20;
	}
	else {
		if (!mInfo.metallicMap.empty()) {
			fsUniform += "uniform sampler2D metallicMap;\n";
			programSampler += "metallicMap=";
			programSampler += mInfo.metallicMap;
			programSampler += "\n";
			fsMain += "outNormalMap.a = texture(metallicMap,fsTexcoord).r;\n";
			materialFlag |= 0x08;
		}
		else {
			fsUniform += "uniform float metallic;\n";
			fsMain += "outNormalMap.a = metallic;\n";
			program += "metallic=metallic\n";
		}

		if (!mInfo.roughnessMap.empty()) {
			fsUniform += "uniform sampler2D roughnessMap;\n";
			programSampler += "roughnessMap=";
			programSampler += mInfo.roughnessMap;
			programSampler += "\n";
			fsMain += "outAlbedoMap.a = texture(roughnessMap,fsTexcoord).r;\n";
			materialFlag |= 0x10;
		}
		else {
			fsUniform += "uniform float roughness;\n";
			fsMain += "outAlbedoMap.a = roughness;\n";
			program += "roughness=roughness\n";
		}
		if (!mInfo.aoMap.empty()) {
			fsUniform += "uniform sampler2D aoMap;\n";
			programSampler += "aoMap=";
			programSampler += mInfo.aoMap;
			programSampler += "\n";
			fsMain += "outPosMap.a = texture(aoMap,fsTexcoord).r;\n";
			materialFlag |= 0x20;
		}
		else {
			fsUniform += "uniform float ao;\n";
			fsMain += "outPosMap.a = ao;\n";
			program += "ao=ao\n";
		}
	}


	fsMain += "}\n";
	programSampler += "}\n";

	fs += (fsOut + fsIn + fsUniform + fsGetNormal + fsMain);
	program += programSampler;
	//fs 搞定了
	auto pMaterial = std::make_shared<Material>();
	LOGD(" VS:%s", vs.c_str());
	LOGD(" FS:%s", fs.c_str());
	LOGD(" program:%s", program.c_str());
	if (compileShader(pMaterial,materialName, vs, fs)) {
		if (parseProgram(pMaterial,program)) {
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

std::shared_ptr<Material> ResourceImpl::getMaterial(const std::string& name) {
	auto it = mMaterials.find(name);
	if (it != mMaterials.end()) {
		return it->second;
	}
	return nullptr;
}
std::shared_ptr<Texture> ResourceImpl::getTexture(const std::string& name) {
	auto it = mTextures.find(name);
	if (it != mTextures.end()) {
		return it->second;
	}
	return nullptr;
}

void ResourceImpl::loadAllMaterial() {
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
}