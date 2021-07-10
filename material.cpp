#include "material.h"
#include <fstream>
#include <set>
#include "Log.h"
#include "TextureUtils.h"

const set<string> gStringValueKey = {"vs","fs","program" };
const set<string> gIntValueKey = { "sampler0","sampler1","sampler2","posLoc","texcoordLoc","norLoc","colorLoc" };
const set<string> gTexObjValueKey = { "tex0","tex1", "tex2" };

std::unordered_map<std::string, std::shared_ptr<Material>> Material::gMaterials;
std::unordered_map<std::string, std::shared_ptr<Texture>> Material::gTextures;
std::unordered_map<std::string, std::shared_ptr<Shader>> Material::gShaders;
std::unordered_map<std::string, std::string> Material::mContents;

Material::Material()
{

}
Material::~Material() {

}

string getFileName(const string& path,const string& suffix) {
	string temp;
	auto endpos = path.rfind(suffix);
	auto startpos = path.find_last_of("\\/");
	if (endpos != string::npos) {
		if (startpos == string::npos) {
			temp = path.substr(0, endpos);
		}
		else if (startpos+1 < endpos) {
			temp = path.substr(startpos+1, endpos-startpos-1);
		}
	}
	return temp;
}

string Material::getItemName(const string& key) {
	string temp;
	auto startPos = key.find(':');
	if (startPos != string::npos) {
		temp = key.substr(startPos + 1);
	}
	return temp;
}

bool Material::parseMaterialFile(const string& path) {
	ifstream matFile(path);
	string material((std::istreambuf_iterator<char>(matFile)), std::istreambuf_iterator<char>());
	string filename = getFileName(path, ".material");
	if (filename.empty()) {
		LOGE("error to parse material file path,can't get filename %s",path.c_str());
		return false;
	}
	std::string::size_type startPos = 0;
	std::string::size_type keyValuePos[3];
	while (findkeyValue(material, "{","}",startPos, keyValuePos)) {
		//analsys£¬store key-value to mContents;
		auto temppos = material.find_first_of(" \r\n\t{", keyValuePos[0]);
		if (temppos != string::npos) {
			string key = material.substr(keyValuePos[0], temppos - keyValuePos[0]);
			string value = material.substr(keyValuePos[1] + 1, keyValuePos[2] - keyValuePos[1] - 1);
			if (!mContents.try_emplace(key, value).second) {
				LOGE("%s:%s:%s error to emplace key %s", __FILE__,__func__, __LINE__,key);
			}
		}

		startPos = keyValuePos[2]+1;
	}

	for (const auto& kv : mContents) {
		const auto& key = kv.first;
		if (key.find("program") != string::npos) {
			auto programName = getItemName(key);
			if (!programName.empty()) {
				parseProgram(programName, kv.second);
			}
		}
		else if (key.find("texture") != string::npos) {
			auto textureName = getItemName(key);
			if (!textureName.empty()) {
				parseTexture(textureName, kv.second);
			}
		}
	}
	return true;
}

/*
str where to find key-value;
mid the str between key and value;
end the end of value;
startPos from which pos to start serach in str;
pos is int[3]£¬pos[0],the start pos of he key,
pos[1] the pos of mid
pos[2] the pos of end
*/
bool Material::findkeyValue(const string& str, const string& mid,const string& end, std::string::size_type startPos, std::string::size_type* pos) {
	int countOfStart = 0;
	int countOfEnd = 0;
	std::string::size_type nextPos = startPos;
	pos[0] = pos[1] = pos[2] = -1;
	pos[0] = str.find_first_not_of(mid+end+" \r\n\t", nextPos);
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
					//Óï·¨´íÎóÍË³ö
					break;
				}
			}
			nextPos = tempPos+1;
		}
		else {
			break;
		}

	} while (countOfStart != countOfEnd);

	if (countOfStart == countOfEnd && countOfStart > 0 && pos[0]<pos[1] && pos[1]<pos[2]) {
		return true;
	}
	else {
		if (countOfStart > 0 || countOfEnd > 0) {
			//LOGE("error to parse material file");
		}
		return false;
	}
}



Texture* Material::getTexture(const std::string& name) {
	Texture* temp = nullptr;
	auto it = gTextures.find(name);
	if (it != gTextures.end())
		temp = it->second.get();
	return temp;
}

Material* Material::getMaterial(const std::string& name) {
	Material* temp = nullptr;
	auto it = gMaterials.find(name);
	if (it != gMaterials.end())
		temp = it->second.get();
	return temp;
}

Shader* Material::getShader(const std::string& name) {
	Shader* temp = nullptr;
	auto it = gShaders.find(name);
	if (it != gShaders.end())
		temp = it->second.get();
	return temp;
}

bool Material::parseItem(const string& value, Umapss& umap) {
	std::string::size_type pos[3]{ 0,0,0 };
	std::string::size_type startPos = 0;
	do {
		if (findkeyValue(value, "=", "\r\n", startPos, pos) || findkeyValue(value, "{", "}", startPos, pos)) {
			auto temp = value.substr(pos[0], pos[1] - pos[0]);
			auto keyendpos = value.find_first_of(' ', 0);
			auto realKey = temp.substr(0, keyendpos);
			auto tempValue = value.substr(pos[1]+1, pos[2]-pos[1]-1);
			auto valueStartPos = tempValue.find_first_not_of(" \r\n\t", 0);
			if (valueStartPos != string::npos) {
				auto valueEndPos = tempValue.find_last_not_of(" \r\n\t", valueStartPos);
				auto realValue = tempValue.substr(valueStartPos, valueEndPos);
				umap.try_emplace(std::move(realKey), std::move(realValue));
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
			auto pTex = loadImageFromFile(pPath->second);
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
					if (depth != 3 && depth != 1) {
						LOGE("not support texture %s depth %d", textureName.c_str(),depth);
						return false;
					}else if (depth == 1) {
						internalFormat = GL_LUMINANCE;
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
				catch (exception e) {
					LOGE("parseTexture error to stoi width height depth");
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
			auto sd = std::make_shared<Shader>();
			if (sd->initShader(ptrVs->second, ptrFs->second)) {
				LOGD("initShader %s success", programName.c_str());
				auto insertRes = gShaders.try_emplace(programName, sd);
				if (insertRes.second) {
					bsuccess = true;
					sd->setLocation(posLoc, texcoordLoc, colorLoc, normalLoc);
					if (!umapSampler.empty()) {
						std::for_each(umapSampler.begin(), umap.end(), [&sd](const Umapss::const_reference item) {
							sd->setTextureForSampler(item.first, item.second);
						});
					}
				}
				else {
					LOGE("insert program %s to container failed", programName.c_str());
				}
			}
			else {
				LOGE("initShader %s failed", programName.c_str());
			}
			break;
		}
		else {
			LOGE("can't find program's vsshader or fs shader %s", programName.c_str());
		}
	} while (false);
	
	return bsuccess;
}