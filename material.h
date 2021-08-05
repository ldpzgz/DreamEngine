#ifndef _METERIAL_H_
#define _METERIAL_H_
#include <iostream>
#include <string>
#include <memory>
#include <unordered_map>
#include "Shader.h"
#include "Texture.h"
using namespace std;
class Material : public enable_shared_from_this<Material> {
public:
	Material();
	~Material();

	bool parseMaterialFile(const string&);
	void enable();
	void updateMvpMatrix(const float* pdata);
	void getVertexAtributeLoc(int& posLoc, int& texcoordLoc, int& colorLoc, int& normalLoc);

	static shared_ptr<Texture> getTexture(const std::string&);
	static shared_ptr<Material> getMaterial(const std::string&);
	static shared_ptr<Shader> getShader(const std::string&);

	static std::shared_ptr<Texture> createTexture(const std::string& name,int width, int height, unsigned char* pdata, GLint format = GL_RGB, GLenum type = GL_UNSIGNED_BYTE, bool autoMipmap = false);
private:
	//find key value from startPos at str,
	//if success set the start position of the key,the pos of '{', the pos of '}' into pos seprately and return true,
	//else return false
	bool findkeyValue(const string& str, const string& mid, const string& end, std::string::size_type startPos, std::string::size_type* pos);
	string getItemName(const string& key);
	bool parseProgram(const string& programName,const string& program);
	bool parseTexture(const string& textureName, const string& texture);
	using Umapss = std::unordered_map<std::string, std::string>;
	bool parseItem(const string& value, Umapss& umap);

	std::unordered_map<std::string, std::string> mContents;
	std::shared_ptr<Shader> mShader;

	static std::unordered_map<std::string, std::shared_ptr<Material>> gMaterials;
	static std::unordered_map<std::string, std::shared_ptr<Texture>> gTextures;
	static std::unordered_map<std::string, std::shared_ptr<Shader>> gShaders;
};

#endif