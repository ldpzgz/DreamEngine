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
	Material(const Material& mat);
	bool parseMaterialFile(const string&);
	void enable();

	//如果材质文件里面有个key对应的value是整数，可以用这个函数获取到
	int getKeyAsInt(const string& key);

	//改变shader里面sampler对应的纹理
	void setTextureForSampler(const string& samplerName, const shared_ptr<Texture>& pTex);


	std::shared_ptr<Shader>& getShader() {
		return mShader;
	}
	/*
	* 给物体设置一种颜色，物体以这种纯色显示，用于线条、网格的绘制
	* 如果shader里面没有相应的uniformColor变量，也可以调用这个函数，只是不会生效而已
	*/
	void setUniformColor(const Color& color) {
		if (!mpUniformColor) {
			mpUniformColor = make_unique<Color>(color);
		}
		else {
			*mpUniformColor = color;
		}
	}
	/*
	* 给物体设置一种颜色，物体以这种纯色显示，用于线条、网格的绘制
	* 如果shader里面没有相应的uniformColor变量，也可以调用这个函数，只是不会生效而已
	*/
	void setUniformColor(float r, float g, float b, float a) {
		if (!mpUniformColor) {
			mpUniformColor = make_unique<Color>(r, g, b, a);
		}
		else {
			*mpUniformColor = Color(r, g, b, a);
		}
	}

	void setName(const std::string name) {
		mName = name;
	}

	static shared_ptr<Texture>& getTexture(const std::string&);
	static shared_ptr<Material> getMaterial(const std::string&);
	static shared_ptr<Shader>& getShader(const std::string&);

	static shared_ptr<Material> loadFromFile(const string& filename);
	static std::shared_ptr<Texture> createTexture(const std::string& name,int width, int height, unsigned char* pdata, GLint format = GL_RGB, GLenum type = GL_UNSIGNED_BYTE, bool autoMipmap = false);
	static std::shared_ptr<Texture> loadTextureFromFile(const std::string& name);
	static void loadAllMaterial();
private:
	static shared_ptr<Material> clone(const Material&);
	//find key value from startPos at str,
	//if success set the start position of the key,the pos of '{', the pos of '}' into pos seprately and return true,
	//else return false
	bool findkeyValue(const string& str, const string& mid, const string& end, std::string::size_type startPos, std::string::size_type* pos);
	/*
	* 获得:后面的名字
	* key 形如：program:rectangleTex
	*/
	string getItemName(const string& key);

	/*
	* programName,program:后面跟的名字
	* program，要被分析的program的内容
	* 分析program，创建shader，将shader放到gShader全局变量里面，
	* 拿到shader的各种uniform的loc，为shader里面的sampler指定纹理。
	*/
	bool parseProgram(const string& programName,const string& program);
	/*
	* textureName  material文件里面texture：后面跟的纹理的名字
	* texture material文件里面texture的内容
	* texture的内容要么是一个path=xxxx，这样的路径
	* 要么就是长、宽、深度三个信息确定一个纹理。
	* 这个函数分析texture的内容，创建一个纹理，将纹理保存到gTexture全局变量里面
	*/
	bool parseTexture(const string& textureName, const string& texture);
	using Umapss = std::unordered_map<std::string, std::string>;

	bool parseMaterial(const string& matName, const string& material);
	/*
	* 将value字符串里面形如key=value、或者key{value}格式的，key和value字符串解析出来，存储到umap里面
	*/
	bool parseItem(const string& value, Umapss& umap);

	std::shared_ptr < std::unordered_map<std::string, std::string>> mpContents;//保存的是材质文件里面形如key{value}的key-value对
	
	std::shared_ptr<Shader> mShader;

	std::unique_ptr<Color> mpUniformColor;

	std::unordered_map<int, std::shared_ptr<Texture>> mSamplerName2Texture;

	std::string mName;

	static std::unordered_map<std::string, std::shared_ptr<Material>> gMaterials;
	static std::unordered_map<std::string, std::shared_ptr<Texture>> gTextures;
	static std::unordered_map<std::string, std::shared_ptr<Shader>> gShaders;
};

using MaterialP = std::shared_ptr<Material>;

extern MaterialP gpMaterialNothing;
#endif