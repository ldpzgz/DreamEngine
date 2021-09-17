#include "material.h"
#include <fstream>
#include <set>
#include "Log.h"
#include "Utils.h"
#include <filesystem>

using namespace std;
using namespace std::filesystem;
static const string gMaterialPath = "./opengles3/material/material";

std::unordered_map<std::string, std::shared_ptr<Material>> Material::gMaterials;
std::unordered_map<std::string, std::shared_ptr<Texture>> Material::gTextures;
std::unordered_map<std::string, std::shared_ptr<Shader>> Material::gShaders;

void Material::loadAllMaterial() {
	//遍历UIimage目录
	path materialPath(gMaterialPath);
	if (!exists(materialPath)) {
		LOGE("ERROR the ui image path %s is not exist", gMaterialPath.c_str());
	}

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

Material::Material()
{

}
Material::~Material() {
	mContents.clear();
}

shared_ptr<Material> Material::loadFromFile(const string& filename) {
	auto pMaterial = make_shared<Material>();
	if (!pMaterial->parseMaterialFile(filename)) {
		pMaterial.reset();
	}
	return pMaterial;
}

string Material::getItemName(const string& key) {
	string temp;
	auto startPos = key.find(':');
	if (startPos != string::npos) {
		temp = key.substr(startPos + 1);
	}
	return temp;
}

void Material::updateMvpMatrix(const glm::mat4& pdata) {
	if (mShader) {
		mShader->setMvpMatrix(pdata);
	}
}

void Material::updateTextureMatrix(const glm::mat4& pdata) {
	if (!mpTextureMatrix) {
		mpTextureMatrix = make_shared<glm::mat4>(1.0f);
	}
	if (mpTextureMatrix) {
		*mpTextureMatrix = pdata;
	}
	if (mShader) {
		mShader->setTextureMatrix(pdata);
	}
}

void Material::setTextureMatrix() {
	if (mpTextureMatrix && mShader) {
		mShader->setTextureMatrix(*mpTextureMatrix);
	}
}

void Material::setUniformColor(const Color& color) {
	if (mShader) {
		mShader->setUniformColor(color);
	}
}

void Material::setUniformColor(float r, float g, float b, float a) {
	if (mShader) {
		mShader->setUniformColor(r,g,b,a);
	}
}

void Material::enable() {
	if (mShader) {
		mShader->enable();
	}
}
void Material::getVertexAtributeLoc(int& posLoc, int& texcoordLoc, int& colorLoc, int& normalLoc) {
	if (mShader) {
		mShader->getLocation(posLoc, texcoordLoc, colorLoc, normalLoc);
	}
}

bool Material::parseMaterialFile(const string& path) {
	bool bParseSuccess = false;
	ifstream matFile(path);
	string material((std::istreambuf_iterator<char>(matFile)), std::istreambuf_iterator<char>());
	string filename = Utils::getFileName(path);
	if (filename.empty()) {
		LOGE("error to parse material file path,can't get filename %s",path.c_str());
		return false;
	}
	std::string::size_type startPos = 0;
	std::string::size_type keyValuePos[3];
	std::string programKey;
	while (findkeyValue(material, "{","}",startPos, keyValuePos)) {
		//analsys，store key-value to mContents;
		auto temppos = material.find_first_of("\x20\r\n\t{", keyValuePos[0]);
		if (temppos != string::npos) {
			string key = material.substr(keyValuePos[0], temppos - keyValuePos[0]);
			string value = material.substr(keyValuePos[1] + 1, keyValuePos[2] - keyValuePos[1] - 1);
			
			auto tempPos = value.find_first_not_of("\x20\r\n\t");
			if (tempPos != string::npos && tempPos>0) {
				value = value.substr(tempPos);
			}
			if (!mContents.try_emplace(key, value).second) {
				LOGE("%s:%s:%s error to emplace key %s", __FILE__,__func__, __LINE__,key);
			}
			else if (key.find("texture") != string::npos) {
				auto textureName = getItemName(key);
				if (!textureName.empty()) {
					bParseSuccess = parseTexture(textureName, value);
				}
			}
			else if (key.find("program") != string::npos) {
				programKey = key;
			}
		}

		startPos = keyValuePos[2]+1;
	}

	if (!programKey.empty()) {
		auto it = mContents.find(programKey);
		if (it != mContents.end()) {
			auto programName = getItemName(programKey);
			if (!programName.empty()) {
				bParseSuccess = parseProgram(programName, it->second);
			}
			else {
				LOGE("program has no name in %s", path.c_str());
			}
		}
	}
	else {
		LOGE("ERROR! %s material has no program", path.c_str());
	}

	if (bParseSuccess) {
		if (gMaterials.try_emplace(filename, shared_from_this()).second) {
			LOGD("success to parse material %s",filename.c_str());
		}
		else {
			LOGD("failed to parse material %s", filename.c_str());
		}
	}
	//mContents.clear();
	return bParseSuccess;
}

/*
在字符串str里面冲startPos开始，查找第一个形如：key{value}
str		where to find key-value;
mid		the str between key and value;
end		the end of value;
startPos from which pos to start serach in str;
pos		is int[3]，pos[0],the start pos of the key,
pos[1]	the pos of mid
pos[2]	the pos of end
*/
bool Material::findkeyValue(const string& str, const string& mid,const string& end, std::string::size_type startPos, std::string::size_type* pos) {
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
			LOGE("findkeyValue error to parse material file");
		}
		return false;
	}
}



shared_ptr<Texture>& Material::getTexture(const std::string& name) {
	static std::shared_ptr<Texture> temp;
	auto it = gTextures.find(name);
	if (it != gTextures.end())
		return it->second;
	return temp;
}

shared_ptr<Material>& Material::getMaterial(const std::string& name) {
	static shared_ptr<Material> temp;
	auto it = gMaterials.find(name);
	if (it != gMaterials.end())
		return it->second;
	return temp;
}

shared_ptr<Shader>& Material::getShader(const std::string& name) {
	static shared_ptr<Shader> temp;
	auto it = gShaders.find(name);
	if (it != gShaders.end())
		return it->second;
	return temp;
}

std::shared_ptr<Texture> Material::createTexture(const std::string& name,int width, int height, unsigned char* pdata, GLint format, GLenum type, bool autoMipmap) {
	auto pTex = make_shared<Texture>();
	if (!pTex->load(width, height, pdata, format, type, 1,autoMipmap)) {
		LOGE("ERROR to create a texture");
		pTex.reset();
	}
	else {
		auto it = gTextures.try_emplace(name, pTex);
		if (!it.second) {
			LOGE("ERROR to add texture %s to gTexture,exist already", name.c_str());
			pTex.reset();
		}
	}
	return pTex;
}

std::shared_ptr<Texture> Material::loadTextureFromFile(const std::string& path) {
	auto texName = Utils::getFileName(path);
	auto pTexture = Utils::loadImageFromFile(path);
	if (pTexture) {
		auto it = gTextures.try_emplace(texName, pTexture);
		if (!it.second) {
			LOGE("ERROR to add texture %s to gTexture,exist already", path.c_str());
			pTexture.reset();
		}
	}
	return pTexture;
}

bool Material::parseItem(const string& value, Umapss& umap) {
	std::string::size_type pos[3]{ 0,0,0 };
	std::string::size_type startPos = 0;
	do {
		if (findkeyValue(value, "=", "\x20\r\n\t", startPos, pos) || findkeyValue(value, "{", "}", startPos, pos)) {
			auto temp = value.substr(pos[0], pos[1] - pos[0]);
			auto keyendpos = temp.find_last_not_of('\x20');
			auto realKey = temp.substr(0, keyendpos+1);
			auto tempValue = value.substr(pos[1]+1, pos[2]-pos[1]-1);
			auto valueStartPos = tempValue.find_first_not_of("\x20\r\n\t", 0);
			if (valueStartPos != string::npos) {
				auto valueEndPos = tempValue.find_last_not_of("\x20\r\n\t");//becarful ,see http://www.cplusplus.com/reference/string/string/find_last_not_of/
				auto realValue = tempValue.substr(valueStartPos, valueEndPos-valueStartPos+1);
				if (!umap.try_emplace(std::move(realKey), std::move(realValue)).second) {
					LOGE("failed to insert material key %s into unordermap", realKey.c_str());
				}
			}
			else {
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

bool Material::parseTexture(const string& textureName, const string& texture) {
	Umapss umap;
	int width;
	int height;
	int depth;
	if (parseItem(texture, umap)) {
		const auto pPath = umap.find("path");
		if (pPath != umap.cend()) {
			auto pTex = Utils::loadImageFromFile(pPath->second);
			if (pTex && gTextures.try_emplace(textureName,pTex).second) {
				LOGD("success to parse texture %s from path",textureName.c_str());
			}
			else {
				LOGE("failed to parse texture %s from path", textureName.c_str());
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
					int internalFormat = GL_RGB;
					if (depth == 4) {
						internalFormat = GL_RGBA;
					}else if (depth == 1) {
						internalFormat = GL_LUMINANCE;
					}else if (depth != 3) {
						LOGE("not support texture %s depth %d", textureName.c_str(), depth);
						return false;
					}

					auto pTex = std::make_shared<Texture>();
					if (pTex->load(width, height, nullptr, internalFormat) &&
						gTextures.try_emplace(textureName, pTex).second) {
						LOGD("success to create texture %s", textureName.c_str());
					}
					else {
						LOGE("error to create texture %s", textureName.c_str());
					}
				}
				catch (const logic_error& e) {
					LOGE("parseTexture throw exception %s from stoi() width height depth",e.what());
					return false;
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

bool Material::parseProgram(const string& programName,const string& program) {
	Umapss umap;
	string vs_key;
	string fs_key;
	int posLoc = -1;
	int texcoordLoc = -1;
	int normalLoc = -1;
	int colorLoc = -1;
	std::string mvpMatrixName;
	std::string textureMatrixName;
	std::string uniformColor;
	Umapss umapSampler;
			
	if (parseItem(program, umap)) {
		const auto pvs = umap.find("vs");
		if (pvs != umap.cend()) {
			vs_key = "vs:" + pvs->second;
		}

		const auto pfs = umap.find("fs");
		if (pfs != umap.cend()) {
			fs_key = "fs:" + pfs->second;
		}

		const auto pPosLoc = umap.find("posLoc");
		if (pPosLoc != umap.cend()) {
			try {
				posLoc = std::stoi(pPosLoc->second);
			}
			catch (exception e) {
				LOGE("parseProgram error to stoi posLoc");
				posLoc = -1;
			}
			
		}

		const auto pTexcoordLoc = umap.find("texcoordLoc");
		if (pTexcoordLoc != umap.cend()) {
			try {
				texcoordLoc = std::stoi(pTexcoordLoc->second);
			}
			catch (exception e) {
				LOGE("parseProgram error to stoi texcoordLoc");
				texcoordLoc = -1;
			}

		}
		
		const auto pColorLoc = umap.find("colorLoc");
		if (pColorLoc != umap.cend()) {
			try {
				colorLoc = std::stoi(pColorLoc->second);
			}
			catch (exception e) {
				LOGE("parseProgram error to stoi colorLoc");
				colorLoc = -1;
			}

		}

		const auto pNormalLoc = umap.find("normalLoc");
		if (pNormalLoc != umap.cend()) {
			try {
				normalLoc = std::stoi(pNormalLoc->second);
			}
			catch (exception e) {
				LOGE("parseProgram error to stoi normalLoc");
				normalLoc = -1;
			}

		}

		const auto pMvp = umap.find("mvpMatrix");
		if (pMvp != umap.cend()) {
			mvpMatrixName = pMvp->second;
		}

		const auto pTextureMatrix = umap.find("textureMatrix");
		if (pTextureMatrix != umap.cend()) {
			textureMatrixName = pTextureMatrix->second;
		}

		//由于在fs里面经常需要设置一个颜色值，所以把这个抽出来，下面的代码会拿到这个uniform的loc，
		//外部程序调用了material的updateUniformColor，设置了颜色值，每次enable的时候都会把这个颜色值设置上去。
		const auto pUColor = umap.find("uniformColor");
		if (pUColor != umap.cend()) {
			uniformColor = pUColor->second;
		}

		//为shader里面的sampler指定Texture（通过texture的名字）
		const auto pSampler = umap.find("sampler2D");
		
		if (pSampler != umap.cend()) {
			if (parseItem(pSampler->second, umapSampler)) {

			}
			else {
				LOGE("error to parse sampler2D in program %s", programName.c_str());
			}
		}
		
	}
	else {
		LOGE("ERROR TO PARSE PROGRAM");
		return false;
	}

	if (vs_key.empty() || fs_key.empty()) {
		LOGE("error to parse program %s", program.c_str());
		return false;
	}

	const auto ptrVs = mContents.find(vs_key);
	const auto ptrFs = mContents.find(fs_key);
	bool bsuccess = false;
	do {
		if (ptrVs != mContents.cend() && ptrFs != mContents.cend()) {
			mShader = std::make_shared<Shader>(programName);
			if (mShader->initShader(ptrVs->second, ptrFs->second)) {
				LOGD("initShader %s success", programName.c_str());
				
				if (gShaders.try_emplace(programName, mShader).second) {
					bsuccess = true;
					mShader->setLocation(posLoc, texcoordLoc, colorLoc, normalLoc);
					if (!mvpMatrixName.empty()) {
						mShader->getMvpMatrixLoc(mvpMatrixName);
					}
					if (!textureMatrixName.empty()) {
						mShader->getTextureMatrixLoc(textureMatrixName);
					}
					if (!uniformColor.empty()) {
						mShader->getUniformColorLoc(uniformColor);
					}
					if (!umapSampler.empty()) {
						for (auto& item : umapSampler) {
							const auto pTex = gTextures.find(item.second);
							int loc = mShader->getUniformLoc(item.first.c_str()); //
							if (loc != -1) {
								if (pTex != gTextures.end()) {
									mShader->setTextureForSampler(loc, pTex->second); //
								}
								else {
									mShader->setTextureForSampler(loc, shared_ptr<Texture>()); //
								}
							}
							else {
								LOGD("can't to find texture %s", item.second.c_str());
							}
						}
					}
				}
				else {
					LOGE("insert program %s to container failed", programName.c_str());
				}
			}
			else {
				LOGE("initShader %s failed", programName.c_str());
				mShader.reset();
			}
			break;
		}
		else {
			LOGE("can't find program's vs shader or fs shader %s", programName.c_str());
		}
	} while (false);
	
	return bsuccess;
}

int Material::getKeyAsInt(const string& key) {
	auto it = mContents.find(key);
	int ret = -1;
	if (it != mContents.end()) {
		try {
			ret = std::stoi(it->second);
		}
		catch (exception e) {
			LOGE("parseProgram error to stoi posLoc");
			ret = -1;
		}
	}
	return ret;
}

void Material::changeTexture(const string& samplerName, const shared_ptr<Texture>& pTex) {
	if (mShader) {
		int loc = mShader->getUniformLoc(samplerName.c_str());
		if (loc != -1) {
			mShader->setTextureForSampler(loc,pTex);
		}
		else {
			LOGD("Material::changeTexture no sampler %s",samplerName.c_str());
		}
	}
}