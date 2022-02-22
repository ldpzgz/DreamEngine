#include "material.h"
#include <fstream>
#include <set>
#include "Log.h"
#include "Utils.h"
#include "helper.h"
#include <filesystem>

using namespace std;
using namespace std::filesystem;

MaterialP gpMaterialNothing;

static const string gMaterialPath = "./opengles3/material/material";
static const string gProgramPath = "./opengles3/material/program";
static const string gDrawablePath = "./opengles3/material/drawable";

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

std::unordered_map<std::string, std::shared_ptr<Material>> Material::gMaterials;
std::unordered_map<std::string, std::shared_ptr<Texture>> Material::gTextures;
std::unordered_map<std::string, std::shared_ptr<Shader>> Material::gShaders;

//std::unordered_map<std::string, std::function<bool(Material* pMat, const std::string&)>> Material::gMatFileHandlers{
//	{"material",},
//	{"texture",},
//	{"cubeTexture",},
//	{"program",},
//	{"config",},
//}

/*
* program文件里面：program:testM{...}的处理函数
*/
std::unordered_map<std::string, std::function<bool(Material* pMat, const std::string&)>> Material::gProgramKeyValueHandlers{
	{"posLoc",Material::posLocHandler},
	{"texcoordLoc",Material::texcoordLocHandler},
	{"colorLoc",Material::colorLocHandler},
	{"normalLoc",Material::normalLocHandler},
	{"tangentLoc",Material::tangentLocHandler},
	{"mvpMatrix",Material::mvpMatrixHandler},
	{"mvMatrix",Material::mvMatrixHandler},
	{"viewMatrix",Material::vMatrixHandler},
	{"textureMatrix",Material::texMatrixHandler},
	{"lightPos",Material::lightPosHandler},
	{"lightColor",Material::lightColorHandler},
	{"viewPos",Material::viewPosHandler},
	{"uniformColor",Material::uniformColorHandler},
	{"albedo",Material::albedoColorHandler},
	{"sampler",Material::programSamplerHandler},
	{"metallic",Material::metallicHandler},
	{"roughness",Material::roughnessHandler},
	{"ao",Material::aoHandler},
	{"op",Material::opHandler}
};
/*
* material文件里面：material:testM{...}的处理函数
*/
std::unordered_map<std::string, std::function<bool(Material* pMat, const std::string&)>> Material::gMaterialHandlers{
	{"program",Material::programHandler},
	{"sampler",Material::samplerHandler},
	{"op",Material::opHandler},
	{"depthTest",Material::opDepthHandler},
	{"blend",Material::opBlendHandler},
	{"cullFace",Material::opCullfaceHandler}
};

bool Material::posLocHandler(Material* pMat, const std::string& value) {
	try {
		int posLoc = std::stoi(value);
		pMat->getShader()->setPosLoc(posLoc);
	}
	catch (exception e) {
		LOGE("error to stoi posLoc,in material %s",pMat->mName.c_str());
	}
	return true;
}

bool Material::colorLocHandler(Material* pMat, const std::string& value) {
	try {
		int posLoc = std::stoi(value);
		pMat->getShader()->setColorLoc(posLoc);
	}
	catch (exception e) {
		LOGE("error to stoi colorLoc,in material %s", pMat->mName.c_str());
	}
	return true;
}

bool Material::normalLocHandler(Material* pMat, const std::string& value) {
	try {
		int norLoc = std::stoi(value);
		pMat->getShader()->setNormalLoc(norLoc);
	}
	catch (exception e) {
		LOGE("error to stoi normalLoc,in material %s", pMat->mName.c_str());
	}
	return true;
}

bool Material::texcoordLocHandler(Material* pMat, const std::string& value) {
	try {
		int norLoc = std::stoi(value);
		pMat->getShader()->setTexLoc(norLoc);
	}
	catch (exception e) {
		LOGE("error to stoi texcoordLoc,in material %s", pMat->mName.c_str());
	}
	return true;
}

bool Material::tangentLocHandler(Material* pMat, const std::string& value) {
	try {
		int norLoc = std::stoi(value);
		pMat->getShader()->setTangentLoc(norLoc);
	}
	catch (exception e) {
		LOGE("error to stoi tangentLoc,in material %s", pMat->mName.c_str());
	}
	return true;
}

bool Material::mvpMatrixHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getMvpMatrixLoc(value);
	}
	return true;
}

bool Material::mvMatrixHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getMvMatrixLoc(value);
	}
	return true;
}

bool Material::vMatrixHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getViewMatrixLoc(value);
	}
	return true;
}

bool Material::texMatrixHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getTextureMatrixLoc(value);
	}
	return true;
}

bool Material::uniformColorHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getUniformColorLoc(value);
	}
	return true;
}

bool Material::albedoColorHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getAlbedoColorLoc(value);
	}
	return true;
}

bool Material::viewPosHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getViewPosLoc(value);
	}
	return true;
}

bool Material::lightPosHandler(Material* pMat, const std::string& value){
	if (!value.empty()) {
		pMat->getShader()->getLightPosLoc(value);
	}
	return true;
}

bool Material::lightColorHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getLightColorLoc(value);
	}
	return true;
}

bool Material::metallicHandler(Material* pMat, const std::string& value){
	if (!value.empty()) {
		pMat->getShader()->getMetallicLoc(value);
	}
	return true;
}

bool Material::roughnessHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getRoughnessLoc(value);
	}
	return true;
}

bool Material::aoHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		pMat->getShader()->getAoLoc(value);
	}
	return true;
}

bool Material::programSamplerHandler(Material* pMat, const std::string& value) {
	if (!value.empty()) {
		Umapss umapSampler;
		//找到program里面列出来的sampler名字
		if (parseItem(value, umapSampler)) {
			if (!umapSampler.empty()) {
				auto& samplers = pMat->getShader()->getSamplerNames();
				auto& uniforms = pMat->getShader()->getUniforms();
				for (auto& item : umapSampler) {
					//program脚本里面列出来的sampler，看看shader的uniform里面有没有
					if (uniforms.find(item.first) != uniforms.end()) {
						samplers.emplace_back(item.first);
					}
					else {
						LOGE("error in material file sampler2D has no %s", item.first.c_str());
					}

					const auto pTex = gTextures.find(item.second);
					int loc = pMat->getShader()->getUniformLoc(item.first.c_str()); //
					if (loc != -1) {
						if (pTex != gTextures.end()) {
							pMat->mSamplerName2Texture.emplace(loc, pTex->second);
						}
						else {
							pMat->mSamplerName2Texture.emplace(loc, shared_ptr<Texture>()); //
						}
					}
					else {
						LOGD("can't to find texture %s", item.second.c_str());
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

bool Material::programHandler(Material* pMaterial, const std::string& programName) {
	auto it = gShaders.find(programName);
	if (it != gShaders.end()) {
		pMaterial->setShader(it->second);
		return true;
	}
	return false;
}

bool Material::samplerHandler(Material* pMaterial, const std::string& samplerContent) {
	Umapss contents;
	if (parseItem(samplerContent, contents)) {
		for (auto& pairs : contents) {
			auto pTexture = gTextures.find(pairs.second);
			if (pTexture != gTextures.end()) {
				pMaterial->setTextureForSampler(pairs.first, pTexture->second);
			}
			else {
				//如果写的是drawable文件夹里面的某个文件
				auto filePath = gDrawablePath + "/" + pairs.second;
				auto texname = Utils::getFileNameWithPath(pairs.second);
				auto pTex = Material::loadImageFromFile(filePath, texname);
				if (pTex) {
					pMaterial->setTextureForSampler(pairs.first, pTex);
				}
				else {
					LOGE("parse material's sampler property error,cannot find texture %s", pairs.second.c_str());
					return false;
				}
			}
		}
	}
	else {
		LOGD("ERROR to parse material's sampler property: %s",samplerContent.c_str());
		return false;
	}
	return true;
}

bool Material::opHandler(Material* pMaterial, const std::string& opContent) {
	Umapss contents;
	if (parseItem(opContent, contents)) {
		for (auto pairs : contents) {
			auto it = gMaterialHandlers.find(pairs.first);
			if (it != gMaterialHandlers.end()) {
				it->second(pMaterial, pairs.second);
			}
			else {
				LOGE("error parse material-op,cannot recognize key %s in material op",pairs.first.c_str());
			}
		}
	}
	else {
		LOGE("parse material's op property error,contents is %s", opContent.c_str());
		return false;
	}
	return true;
}

bool Material::opDepthHandler(Material* pMaterial, const std::string& value) {
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

bool Material::opBlendHandler(Material* pMaterial, const std::string& value) {
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
				LOGE("blend op spell error,int material %s", pMaterial->mName.c_str());
				hasError = true;
				break;
			}
			
			if (findResult[0] == "true") {
				bEnable = true;
			}
			else if(findResult[0] == "false") {
				bEnable = false;
				break;
			}
			else {
				LOGE("blend op spell error,int material %s", pMaterial->mName.c_str());
				hasError = true;
				break;
			}
			auto srcPair = gBlendFuncMap.find(findResult[1]);
			if (srcPair != gBlendFuncMap.end()) {
				srcFactor = srcPair->second;
			}
			else {
				LOGE("blend op spell error,int material %s", pMaterial->mName.c_str());
				hasError = true;
				break;
			}

			auto dstPair = gBlendFuncMap.find(findResult[2]);
			if (dstPair != gBlendFuncMap.end()) {
				dstFactor = dstPair->second;
			}
			else {
				LOGE("blend op spell error,int material %s", pMaterial->mName.c_str());
				hasError = true;
				break;
			}

			auto equationPair = gBlendEquationMap.find(findResult[3]);
			if (equationPair != gBlendEquationMap.end()) {
				equation = equationPair->second;
			}
			else {
				LOGE("blend op spell error,int material %s", pMaterial->mName.c_str());
				hasError = true;
				break;
			}
			if (count == 7) {
				srcPair = gBlendFuncMap.find(findResult[4]);
				if (srcPair != gBlendFuncMap.end()) {
					srcFactorAlpha = srcPair->second;
				}
				else {
					LOGE("blend op spell error,int material %s", pMaterial->mName.c_str());
					hasError = true;
					break;
				}

				dstPair = gBlendFuncMap.find(findResult[5]);
				if (dstPair != gBlendFuncMap.end()) {
					dstFactorAlpha = dstPair->second;
				}
				else {
					LOGE("blend op spell error,int material %s", pMaterial->mName.c_str());
					hasError = true;
					break;
				}

				equationPair = gBlendEquationMap.find(findResult[6]);
				if (equationPair != gBlendEquationMap.end()) {
					equationAlpha = equationPair->second;
				}
				else {
					LOGE("blend op spell error,int material %s", pMaterial->mName.c_str());
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
			srcFactorAlpha,dstFactorAlpha,equationAlpha);
		return true;
	}
}
bool Material::opCullfaceHandler(Material* pMaterial, const std::string& value) {
	
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
		LOGE("error to parse material-op-cullface value is %s",value.c_str());
		return false;
	}
	else {
		pMaterial->setCullWhichFace(bEnable, cullWhichFace);
		return true;
	}
	
}

void Material::setDepthTest(bool b) {
	if (!mMyOpData) {
		mMyOpData = std::make_shared< OpData >();
	}
	mMyOpData->mbDepthTest = b;
}

void Material::setCullWhichFace(bool b, int whichFace) {
	if (!mMyOpData) {
		mMyOpData = std::make_shared< OpData >();
	}
	mMyOpData->mbCullFace = b;
	mMyOpData->mCullWhichFace = whichFace;
}

void Material::setBlend(bool b, unsigned int srcFactor, unsigned int destFactor,unsigned int blendOp,
	unsigned int srcFactorA, unsigned int destFactorA, unsigned int blendOpA) {
	if (!mMyOpData) {
		mMyOpData = std::make_unique< OpData >();
	}
	mMyOpData->mbBlendTest = b;
	mMyOpData->mBlendSrcFactor = srcFactor;
	mMyOpData->mBlendDstFactor = destFactor;
	mMyOpData->mBlendEquation = blendOp;
	mMyOpData->mBlendAlphaSrcFactor = srcFactorA;
	mMyOpData->mBlendAlphaDstFactor = destFactorA;
	mMyOpData->mBlendAlphaEquation = blendOpA;
}



void Material::loadAllMaterial() {
	path programPath(gProgramPath);
	if (!exists(gProgramPath)) {
		LOGE("ERROR the ui image path %s is not exist", gMaterialPath.c_str());
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
					Material::loadFromFile(filePathString);
				}
			}
		}
	}
	
	path materialPath(gMaterialPath);
	if (!exists(materialPath)) {
		LOGE("ERROR the ui image path %s is not exist", gMaterialPath.c_str());
	}else{
		if (is_directory(materialPath)) {
			//是目录
			directory_iterator list(materialPath);
			//directory_entry 是一个文件夹里的某一项，可以是path，也可以是文件
			for (auto& it : list) {
				auto filePath = it.path();
				if (is_regular_file(filePath)) {
					//是文件
					auto filePathString = filePath.string();
					Material::loadFromFile(filePathString);
				}
			}
		}
	}

	genBrdfLut();
}

Material::Material()
{
}

Material::~Material() {
}

Material::Material(const Material& pMat) {
	mName = pMat.mName;
	mConfigValues = pMat.mConfigValues;
	mShader = pMat.mShader;
	mSamplerName2Texture = pMat.mSamplerName2Texture;
	mMyOpData = pMat.mMyOpData;
	mContents = pMat.mContents;
	mMetallical = pMat.mMetallical;
	mRoughness = pMat.mRoughness;
	mAo = pMat.mAo;
	if(pMat.mpUniformColor)
		mpUniformColor = std::make_shared<Color>(*pMat.mpUniformColor);
}

shared_ptr<Material> Material::loadFromFile(const string& filename) {
	auto pMaterial = make_shared<Material>();
	if (!pMaterial->parseMaterialFile(filename)) {
		pMaterial.reset();
	}
	return pMaterial;
}

void Material::splitKeyAndName(const string& key,string& realKey, string& keyName) {
	auto pos = key.find(':');
	if (pos != string::npos) {
		realKey = key.substr(0,pos);
		keyName = key.substr(pos + 1);
	}
	else {
		realKey = key;
	}
}

//深度测试，blend，cullface等操作
void Material::setMyRenderOperation() {
	if (mMyOpData) {
		if (!mOthersOpData) {
			mOthersOpData = std::make_shared<Material::OpData>();
		}
		if (mMyOpData->mbDepthTest >= 0) {
			GLboolean bDepthTest = true;
			glGetBooleanv(GL_DEPTH_TEST, &bDepthTest);
			mOthersOpData->mbDepthTest = bDepthTest;

			if (mMyOpData->mbDepthTest == 0) {
				glDisable(GL_DEPTH_TEST);
			}
			else {
				glEnable(GL_DEPTH_TEST);
			}
		}

		if (mMyOpData->mbBlendTest >= 0) {
			GLboolean bBlendTest = false;
			glGetBooleanv(GL_BLEND, &bBlendTest);
			mOthersOpData->mbBlendTest = bBlendTest;
			glGetIntegerv(GL_BLEND_SRC_RGB, &mOthersOpData->mBlendSrcFactor);
			glGetIntegerv(GL_BLEND_DST_RGB, &mOthersOpData->mBlendDstFactor);
			glGetIntegerv(GL_BLEND_EQUATION_RGB, &mOthersOpData->mBlendEquation);
			glGetIntegerv(GL_BLEND_SRC_ALPHA, &mOthersOpData->mBlendAlphaSrcFactor); 
			glGetIntegerv(GL_BLEND_DST_ALPHA, &mOthersOpData->mBlendAlphaDstFactor);
			glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &mOthersOpData->mBlendAlphaEquation);
			if (mMyOpData->mbBlendTest == 0) {
				glDisable(GL_BLEND);
			}
			else {
				glEnable(GL_BLEND);
				glBlendFuncSeparate(mMyOpData->mBlendSrcFactor, mMyOpData->mBlendDstFactor,
					mMyOpData->mBlendAlphaSrcFactor, mMyOpData->mBlendAlphaDstFactor);
				glBlendEquationSeparate(mMyOpData->mBlendEquation,mMyOpData->mBlendAlphaEquation);
			}
		}
		
		if (mMyOpData->mbCullFace >= 0) {
			GLboolean preCullFace = false;
			int preCullFaceModel = GL_BACK;
			glGetBooleanv(GL_CULL_FACE, &preCullFace);
			mOthersOpData->mbCullFace = preCullFace;
			glGetIntegerv(GL_CULL_FACE_MODE, &mOthersOpData->mCullWhichFace);
			if (mMyOpData->mbCullFace == 0) {
				glDisable(GL_CULL_FACE);
			}
			else {
				glEnable(GL_CULL_FACE);
				glCullFace(mMyOpData->mCullWhichFace);
			}
		}
	}
}

//恢复之前的深度测试，blend，cullface等操作
void Material::restoreRenderOperation() {
	if (!mOthersOpData) {
		return;
	}
	if (mOthersOpData->mbDepthTest == 0) {
		glDisable(GL_DEPTH_TEST);
	}
	else if (mOthersOpData->mbDepthTest == 1) {
		glEnable(GL_DEPTH_TEST);
	}

	if (mOthersOpData->mbBlendTest == 0) {
		glDisable(GL_BLEND);
	}
	else if(mOthersOpData->mbBlendTest == 1) {
		glEnable(GL_BLEND);
		glBlendFuncSeparate(mOthersOpData->mBlendSrcFactor, mOthersOpData->mBlendDstFactor,
			mOthersOpData->mBlendAlphaSrcFactor, mOthersOpData->mBlendAlphaDstFactor);
		glBlendEquationSeparate(mOthersOpData->mBlendEquation, mOthersOpData->mBlendAlphaEquation);
	}

	if (mOthersOpData->mbCullFace== 0) {
		glDisable(GL_CULL_FACE);
	}
	else if(mOthersOpData->mbCullFace == 1) {
		glEnable(GL_CULL_FACE);
		glCullFace(mOthersOpData->mCullWhichFace);
	}
}

void Material::enable() {
	if (mShader) {
		if (mpUniformColor) {
			mShader->setUniformColor(*mpUniformColor);
		}
		mShader->setMetallic(mMetallical);
		mShader->setRoughness(mRoughness);
		mShader->setAo(mAo);
		mShader->enable();
		int texNum = 0;
		for (auto it = mSamplerName2Texture.begin(); it != mSamplerName2Texture.end(); it++) {
			if (it->second) {
				it->second->active(GL_TEXTURE0 + texNum);
				glUniform1i(it->first, texNum);
				++texNum;
			}
		}
	}
}
//void Material::getVertexAtributeLoc(int& posLoc, int& texcoordLoc, int& colorLoc, int& normalLoc,int& tangentloc) {
//	if (mShader) {
//		mShader->getLocation(posLoc, texcoordLoc, colorLoc, normalLoc, tangentloc);
//	}
//}

bool Material::parseMaterialFile(const string& path) {
	bool bParseSuccess = false;
	if (path.empty()) {
		return false;
	}

	mName = Utils::getFileName(path);
	if (mName.empty()) {
		LOGE("error to parse material file path,can't get filename %s", path.c_str());
		return false;
	}

	ifstream matFile(path);
	string material((std::istreambuf_iterator<char>(matFile)), std::istreambuf_iterator<char>());
	
	std::string::size_type startPos = 0;
	std::string::size_type keyValuePos[3];
	std::string programKey;
	std::string programName;
	std::vector<std::string> materialKeys;
	while (findkeyValue(material, "{","}",startPos, keyValuePos)) {
		//analsys，store key-value to mpContents;
		auto temppos = material.find_first_of("\x20\r\n\t{", keyValuePos[0]);
		if (temppos != string::npos) {
			string key = material.substr(keyValuePos[0], temppos - keyValuePos[0]);
			string value = material.substr(keyValuePos[1] + 1, keyValuePos[2] - keyValuePos[1] - 1);
			
			auto tempPos = value.find_first_not_of("\x20\r\n\t");
			if (tempPos != string::npos && tempPos>0) {
				value = value.substr(tempPos);
			}

			string realKey, keyName;
			splitKeyAndName(key, realKey, keyName);

			if (!mContents.try_emplace(key, value).second) {
				LOGE("double key %s,in %s material file", key.c_str(),mName.c_str());
			}
			else if (realKey=="texture") {
				if (!keyName.empty()) {
					bParseSuccess = parseTexture(keyName, value);
				}
				else {
					LOGE("the texture in material file %s must has a name",mName.c_str());
				}
			}
			else if (realKey == "cubeTexture") {
				if (!keyName.empty()) {
					bParseSuccess = parseCubeTexture(keyName, value);
				}else {
					LOGE("the cubetexture in material file %s must has a name", mName.c_str());
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
		startPos = keyValuePos[2]+1;
	}

	if (!programKey.empty()) {
		auto it = mContents.find(programKey);
		if (it != mContents.end()) {
			if (!programName.empty()) {
				bParseSuccess = parseProgram(programName, it->second);
			}
			else {
				LOGE("program has no name in %s material", mName.c_str());
			}
		}
	}

	if (bParseSuccess) {
		if (gMaterials.try_emplace(mName, shared_from_this()).second) {
			LOGD("success to parse material %s", mName.c_str());
		}
		else {
			LOGD("failed to parse material %s", mName.c_str());
		}
	}
	//parse Material
	for (auto& key : materialKeys) {
		auto it = mContents.find(key);
		if (it != mContents.end()) {
			string material,matName;
			splitKeyAndName(key, material, matName);
			bParseSuccess = parseMaterial(matName, it->second);
		}
	}
	mContents.clear();//
	return bParseSuccess;
}

bool Material::parseMaterial(const string& matName, const string& material) {
	Umapss umap;
	bool bParseSuccess = true;
	if (parseItem(material, umap)) {
		auto pMaterial = std::make_shared<Material>(matName);
		for (auto& pair : umap) {
			auto it = gMaterialHandlers.find(pair.first);
			if (it != gMaterialHandlers.end()) {
				if (!it->second(pMaterial.get(), pair.second))
				{
					bParseSuccess = false;
					break;
				}
			}
		}
		if (bParseSuccess) {
			if (gMaterials.try_emplace(matName, pMaterial).second) {
				LOGD("success to add material %s to gMaterials", matName.c_str());
			}
			else {
				LOGD("failed to add material %s to gMaterials", matName.c_str());
			}
		}
	}
	else {
		LOGE("parse %s material script error,syntax error", matName.c_str());
		bParseSuccess = false;
	}
	
	return bParseSuccess;
}

/*
在字符串str里面从startPos开始，查找第一个形如：key{value}
str		where to find key-value;
mid		the str between key and value;
end		the end of value;
startPos from which pos to start serach in str;
pos		is int[3]，pos[0],the start pos of the key,
pos[1]	the pos of mid
pos[2]	the pos of end
*/
bool Material::findkeyValue(const string& str, const string& mid,const string& end, std::string::size_type startPos, std::string::size_type* pos) {
	auto len = str.length();
	if (startPos >= len) {
		return false;
	}
	int countOfStart = 0;
	int countOfEnd = 0;
	std::string::size_type nextPos = startPos;
	pos[0] = pos[1] = pos[2] = -1;
	pos[0] = str.find_first_not_of(mid+end+"\x20\r\n\t", nextPos);
	
	if (pos[0] == string::npos) {
		return false;
	}
	nextPos = pos[0]+1;
	do {
		auto tempPos = str.find_first_of(mid + end, nextPos);
		if (tempPos != string::npos) {
			if (mid.find(str[tempPos])!=string::npos) {
				++countOfStart;
				if (pos[1] == -1) {
					pos[1] = tempPos;
				}
			}
			else if(end.find(str[tempPos]) != string::npos){
				pos[2] = tempPos;
				++countOfEnd;
				if (countOfEnd > countOfStart) {
					//语法错误退出
					break;
				}
			}
			nextPos = tempPos+1;
		}
		else {
			if (pos[1] > 0 && pos[1] < str.size() - 1)
			{
				++countOfEnd;
				pos[2] = str.size();
			}
			break;
		}

	} while (countOfStart != countOfEnd);

	if (countOfStart == countOfEnd && countOfStart > 0 && pos[0]<pos[1] && pos[1]<pos[2]) {
		return true;
	}
	else {
		if (countOfStart > 0 || countOfEnd > 0) {
			LOGE("findkeyValue ,syntax error");
		}
		return false;
	}
}

bool Material::emplaceTexture(const std::string& name, shared_ptr<Texture>& pTex) {
	if (!name.empty() && pTex) {
		auto res = gTextures.emplace(name, pTex);
		return res.second;
	}
	return false;
}

shared_ptr<Texture>& Material::getTexture(const std::string& name) {
	auto it = gTextures.find(name);
	if (it != gTextures.end())
		return it->second;
	return gpTextureNothing;
}

shared_ptr<Material>& Material::getMaterial(const std::string& name) {
	auto it = gMaterials.find(name);
	if (it != gMaterials.end())
		return it->second;
	return gpMaterialNothing;
}

shared_ptr<Material> Material::clone(const Material& pMat) {
	return std::make_shared<Material>(pMat);
}

shared_ptr<Material> Material::clone(const std::string& name) {
	auto& pMaterial = getMaterial(name);
	if (pMaterial) {
		return std::make_shared<Material>(*pMaterial);
	}
	else {
		return gpMaterialNothing;
	}
}

shared_ptr<Shader>& Material::getShader(const std::string& name) {
	auto it = gShaders.find(name);
	if (it != gShaders.end())
		return it->second;
	LOGE("cannot find shader %s",name.c_str());
	return gpShaderNothing;
}

std::shared_ptr<Texture> Material::createTexture(const std::string& name,int width, int height, unsigned char* pdata, GLint internalFormat,GLint format, GLenum type, bool autoMipmap) {
	auto pTex = make_shared<Texture>();
	if (!pTex->create2DMap(width, height, pdata, internalFormat,format, type, 1,autoMipmap)) {
		LOGE("create a texture failed %s ",__func__);
		pTex.reset();
	}
	else {
		auto it = gTextures.try_emplace(name, pTex);
		if (!it.second) {
			LOGE("failed to add texture %s to gTexture,exist already", name.c_str());
			pTex.reset();
		}
	}
	return pTex;
}

std::shared_ptr<Texture> Material::loadImageFromFile(const std::string& path, std::string texName) {
	if (texName.empty()) {
		texName = Utils::getFileName(path);
	}
	std::shared_ptr<Texture> pTexture = Texture::loadImageFromFile(path);
	if (pTexture) {
		auto it = gTextures.try_emplace(texName, pTexture);
		if (!it.second) {
			LOGE("to add texture %s to gTexture,exist already", path.c_str());
			pTexture.reset();
		}
	}
	return pTexture;
}

bool Material::parseItem(const string& value, Umapss& umap) {
	std::string::size_type pos[3]{ 0,0,0 };
	std::string::size_type startPos = 0;
	do {
		bool findKV = false;
		auto tpos = value.find_first_of("={", startPos);
		if (tpos != string::npos) {
			if (value[tpos] == '=') {
				findKV = findkeyValue(value, "=", "\r\n", startPos, pos);//"\x20\r\n\t"
			}
			else {
				findKV = findkeyValue(value, "{", "}", startPos, pos);
			}
		}
		if (findKV) {
			auto temp = value.substr(pos[0], pos[1] - pos[0]);
			auto keyendpos = temp.find_last_not_of("\x20\t");
			if (keyendpos == string::npos) {
				LOGE("find a empty key!!!");
				return false;
			}
			auto realKey = temp.substr(0, keyendpos+1);
			auto tempValue = value.substr(pos[1]+1, pos[2]-pos[1]-1);
			auto valueStartPos = tempValue.find_first_not_of("\x20\r\n\t", 0);
			if (valueStartPos != string::npos) {
				auto valueEndPos = tempValue.find_last_not_of("\x20\r\n\t");//becarful ,see http://www.cplusplus.com/reference/string/string/find_last_not_of/
				auto realValue = tempValue.substr(valueStartPos, valueEndPos-valueStartPos+1);
				if (!umap.try_emplace(realKey, std::move(realValue)).second) {
					LOGE("failed to insert material key %s into unordermap", realKey.c_str());
				}
			}
			else {
				LOGE("find a empty value!!!");
				return false;
			}
			startPos = pos[2] + 1;
		}
		else {
			break;
		}
	}while (true);
	return true;
}

bool Material::parseCubeTexture(const string& textureName, const string& texture) {
	Umapss umap;
	if (parseItem(texture, umap)) {
		const auto pPath = umap.find("path");
		if (pPath != umap.cend()) {
			auto pTex = std::make_shared<Texture>();
			if (pTex->loadCubemap(gDrawablePath + "/" + pPath->second) && gTextures.try_emplace(textureName, pTex).second) {
				LOGD("success to parse texture %s from path", textureName.c_str());
			}
			else {
				LOGE("failed to load cubetexture %s from path,in material %s", textureName.c_str(),mName.c_str());
			}
		}
	}
	else {
		LOGE("parseCubeTexture parse error %s,in material %s",textureName.c_str(), mName.c_str());
		return false;
	}
	return true;
}

bool Material::parseConfig(const string& cfgName, const string& value) {
	Umapss umap;
	if (parseItem(value, umap)) {
		for (auto& it : umap) {
			auto pos = it.second.find_first_of('\"');
			if (pos != string::npos) {
				auto endPos = it.second.find_last_of('\"');
				if (endPos != string::npos && pos<endPos) {
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

bool Material::parseTexture(const string& textureName, const string& texture) {
	Umapss umap;
	int width;
	int height;
	int depth;
	if (parseItem(texture, umap)) {
		const auto pPath = umap.find("path");
		if (pPath != umap.cend()) {
			auto pTex = Texture::loadImageFromFile(gDrawablePath + "/"+ pPath->second);
			if (!pTex) {
				LOGE("failed to load texture from path %s,in material %s", pPath->second.c_str(),mName.c_str());
			}
			if (!gTextures.emplace(textureName, pTex).second) {
				LOGE("there already has %s in gTextures,in material %s", textureName.c_str(),mName.c_str());
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
				}else if (depth == 1) {
					internalFormat = GL_R8;
					format = GL_RED;
				}else if (depth != 3) {
					LOGE("not support texture %s depth %d,,in material %s", textureName.c_str(), depth,mName.c_str());
					return false;
				}

				auto pTex = std::make_shared<Texture>();
				if (pTex->create2DMap(width, height, nullptr, internalFormat,format) &&
					gTextures.try_emplace(textureName, pTex).second) {
					LOGD("success to create texture %s", textureName.c_str());
				}
				else {
					LOGE("error to create texture %s,in material %s", textureName.c_str(),mName.c_str());
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

bool Material::parseProgram(const string& programName,const string& programContent) {
	Umapss umap;
	string vs_key{ "vs" };
	string fs_key{ "fs" };
	bool bsuccess = false;
	if (parseItem(programContent, umap)) {
		const auto ptrVs = mContents.find(vs_key);
		const auto ptrFs = mContents.find(fs_key);
		if (ptrVs != mContents.cend() && ptrFs != mContents.cend()) {
			mShader = std::make_shared<Shader>(programName);
			if (mShader->initShader(ptrVs->second, ptrFs->second)) {
				LOGD("initShader %s success", programName.c_str());
				if (gShaders.try_emplace(programName, mShader).second) {
					bsuccess = true;
				}
				for (auto& pair : umap) {
					auto it = gProgramKeyValueHandlers.find(pair.first);
					if (it != gProgramKeyValueHandlers.end()) {
						it->second(this, pair.second);
					}
				}
			}
			else {
				LOGE("initShader %s failed", programName.c_str());
				mShader.reset();
			}
		}
		else {
			LOGE("can't find program's vs shader or fs shader %s", programName.c_str());
		}
	}
	return bsuccess;
}

int Material::getKeyAsInt(const string& key) {
	auto it = mConfigValues.find(key);
	int ret = -1;
	if (it != mConfigValues.end()) {
		try {
			ret = std::any_cast<float>(it->second);
		}
		catch (exception e) {
			LOGE("Material::getKeyAsInt error to conver %s",key.c_str());
			ret = -1;
		}
	}
	return ret;
}

shared_ptr<Texture>& Material::getTextureOfSampler(const string& samplerName) {
	if (mShader) {
		int samplerLoc = mShader->getUniformLoc(samplerName.c_str());
		if (samplerLoc != -1) {
			auto it = mSamplerName2Texture.find(samplerLoc);
			if (it != mSamplerName2Texture.end()) {
				return it->second;
			}
		}
	}
	return gpTextureNothing;
}

void Material::setTextureForSampler(const string& samplerName, const shared_ptr<Texture>& pTex) {
	if (mShader) {
		int samplerLoc = mShader->getUniformLoc(samplerName.c_str());
		if (samplerLoc != -1) {
			//mShader->setTextureForSampler(samplerLoc,pTex);
			mSamplerName2Texture[samplerLoc] = pTex;
		}
		else {
			LOGD("no sampler %s,in material %s",samplerName.c_str(),mName.c_str());
		}
	}
}