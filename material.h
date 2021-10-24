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

	//下面这几个函数设置或者更新material里面program定义的变量
	void setMvpMatrix(const glm::mat4& pdata);
	void setMvMatrix(const glm::mat4& pdata);
	void setViewMatrix(const glm::mat4& pdata);
	void setTextureMatrix(const glm::mat4& pdata);
	void setTextureMatrix();
	void setUniformColor(const Color& color);
	void setUniformColor(float r,float g,float b,float a);
	void setLightPos(const Vec3& lightPos);
	void setViewPos(const Vec3& viewPos);
	void setLightColor(const Vec3& lightColor);
	//-------------------------------------------------------------

	void getVertexAtributeLoc(int& posLoc, int& texcoordLoc, int& colorLoc, int& normalLoc,int& tangentloc);

	//如果材质文件里面有个key对应的value是整数，可以用这个函数获取到
	int getKeyAsInt(const string& key);

	//改变material文件里fs shader里面sampler对应的纹理
	void changeTexture(const string& samplerName, const shared_ptr<Texture>& pTex);

	std::shared_ptr<Shader>& getShader() {
		return mShader;
	}

	static shared_ptr<Texture>& getTexture(const std::string&);
	static shared_ptr<Material>& getMaterial(const std::string&);
	static shared_ptr<Shader>& getShader(const std::string&);

	static shared_ptr<Material> loadFromFile(const string& filename);
	static std::shared_ptr<Texture> createTexture(const std::string& name,int width, int height, unsigned char* pdata, GLint format = GL_RGB, GLenum type = GL_UNSIGNED_BYTE, bool autoMipmap = false);
	static std::shared_ptr<Texture> loadTextureFromFile(const std::string& name);
	static void loadAllMaterial();
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

	std::unordered_map<std::string, std::string> mContents;//保存的是材质文件里面形如key{value}的key-value对
	std::shared_ptr<Shader> mShader;
	std::shared_ptr<glm::mat4> mpTextureMatrix;
	static std::unordered_map<std::string, std::shared_ptr<Material>> gMaterials;
	static std::unordered_map<std::string, std::shared_ptr<Texture>> gTextures;
	static std::unordered_map<std::string, std::shared_ptr<Shader>> gShaders;
};

using MaterialP = std::shared_ptr<Material>;

#endif