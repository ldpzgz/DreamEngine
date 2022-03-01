#ifndef _METERIAL_H_
#define _METERIAL_H_
#include <iostream>
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include <any>

using namespace std;
class Texture;
class Shader;
class Color;
class Material : public enable_shared_from_this<Material> {
public:
	class MaterialInfo {
	public:
		//颜色必须要，要么一个color，要么一个map
		std::string albedo{"#ffffff"};
		std::string albedoMap;

		//normal可以没有，可以有：要么normalmap，要么顶点normal
		bool hasNormal{true};
		std::string normalMap;

		//aoMap不为0，就有
		std::string aoMap;

		//粗糙度，要么是一个固定值，要么是map
		float roughness{ 0.5f };
		std::string roughnessMap;

		//金属性，要么是一个固定值，要么是map
		float metallic{ 0.5f };
		std::string metallicMap;
	};

	explicit Material(const std::string name):mName(name) {

	}
	Material();
	~Material();
	Material(const Material& mat);
	/*
	* 分析.material / .program文件
	* 每个.material / .program文件都会自动生成一个Material对象
	*/
	bool parseMaterialFile(const string&);
	void enable();

	//深度测试，blend，cullface等操作
	void setMyRenderOperation();

	//恢复之前的深度测试，blend，cullface等操作
	void restoreRenderOperation();

	//如果材质文件里面有个key对应的value是整数，可以用这个函数获取到
	int getKeyAsInt(const string& key);

	//改变shader里面sampler对应的纹理
	void setTextureForSampler(const string& samplerName, const shared_ptr<Texture>& pTex);

	shared_ptr<Texture>& getTextureOfSampler(const string& samplerName);

	void setShader(const std::shared_ptr<Shader>& ps) {
		mShader = ps;
	}
	std::shared_ptr<Shader>& getShader() {
		return mShader;
	}
	/*
	* 给物体设置一种颜色，物体以这种纯色显示，用于线条、网格的绘制
	* 如果shader里面没有相应的uniformColor变量，也可以调用这个函数，只是不会生效而已
	*/
	void setUniformColor(const Color& color);
	/*
	* 给物体设置一种颜色，物体以这种纯色显示，用于线条、网格的绘制
	* 如果shader里面没有相应的uniformColor变量，也可以调用这个函数，只是不会生效而已
	*/
	void setUniformColor(float r, float g, float b, float a);

	void setAlbedoColor(float r, float g, float b);

	void setAlbedoColor(const Color& c) {
		setUniformColor(c);
	}
	/*
	*	m: 0.0f-1.0f 1.0f表示纯金属，0.0f表示非金属
	*/
	void setMetallical(float m) {
		mMetallical = m;
	}
	/*
	*  r: 0.0f-1.0f 表面粗糙度
	*/
	void setRoughness(float r) {
		mRoughness = r;
	}

	void setAo(float ao) {
		mAo = ao;
	}

	void setName(const std::string name) {
		mName = name;
	}

	const std::string& getName() {
		return mName;
	}
	struct OpData {
		OpData() = default;
		OpData(const OpData& o) = default;
		void operator=(const OpData& o) {
			mbDepthTest = o.mbDepthTest;
			mbBlendTest = o.mbBlendTest;
			mbCullFace = o.mbCullFace;
			mCullWhichFace = o.mCullWhichFace;
			mBlendSrcFactor = o.mBlendSrcFactor;
			mBlendDstFactor = o.mBlendDstFactor;
			mBlendEquation = o.mBlendEquation;
			mBlendAlphaSrcFactor = o.mBlendAlphaSrcFactor;
			mBlendAlphaDstFactor = o.mBlendAlphaDstFactor;
			mBlendAlphaEquation = o.mBlendAlphaEquation;
		}
		int mbDepthTest{-1};//0 关闭，1 开启
		int mbBlendTest{ -1 };//0 关闭，1 开启
		int mbCullFace{ -1 }; //0 关闭，1开启
		int mCullWhichFace{ 0 };
		int mBlendSrcFactor{ 0 };
		int mBlendDstFactor{ 0 };
		int mBlendEquation{ 0 };
		int mBlendAlphaSrcFactor{ 0 };
		int mBlendAlphaDstFactor{ 0 };
		int mBlendAlphaEquation{ 0 };
	};

	static shared_ptr<Texture>& getTexture(const std::string&);
	static bool emplaceTexture(const std::string&, shared_ptr<Texture>&);
	static shared_ptr<Material>& getMaterial(const std::string&);
	/*
	* name: 可以是物体的名字
	* mInfo：材质信息，根据里面的信息生成或者clone一个material对象
	*/
	static shared_ptr<Material> getMaterial(const std::string& name, const MaterialInfo& mInfo);
	static shared_ptr<Shader>& getShader(const std::string&);

	static shared_ptr<Material> loadFromFile(const string& filename);
	static std::shared_ptr<Texture> createTexture(const std::string& name,int width, int height, unsigned char* pdata, GLint internalFormat = GL_RGB, GLint format = GL_RGB, GLenum type = GL_UNSIGNED_BYTE, bool autoMipmap = false);
	static std::shared_ptr<Texture> getOrLoadTextureFromFile(const std::string& path, const std::string& texName = "");
	static std::shared_ptr<Texture> loadImageFromFile(const std::string& path, const std::string& texName = "");
	static void loadAllMaterial();
	static shared_ptr<Material> clone(const std::string&);
	/*
	* 分析配置文件里面的：material:matName{material}，内容
	* 根据里面的内容生成名字为matName的material对象，保存到gMaterial里面。
	*/
	static bool parseMaterial(const string& matName, const string& material);
private:
	using Umapss = std::unordered_map<std::string, std::string>;
	static shared_ptr<Material> clone(const Material&);

	//find key value from startPos at str,
	//if success set the start position of the key,the pos of '{', the pos of '}' into pos seprately and return true,
	//else return false
	static bool findkeyValue(const string& str, const string& mid, const string& end, std::string::size_type startPos, std::string::size_type* pos);
	
	/*
	* 将value字符串里面形如key=value、或者key{value}格式的，key和value字符串解析出来，存储到umap里面
	*/
	static bool parseItem(const string& value, Umapss& umap);
	/*
	* 获得:后面的名字
	* key 形如：program:rectangleTex
	*/
	static void splitKeyAndName(const string& key,string& realKey,string& keyName);

	bool compileShader(const string& programName, const std::string& vs,const std::string& fs);
	/*
	* programName,program:后面跟的名字
	* program，要被分析的program的内容
	* 分析program，program:name{xxx},program的内容是xxx，
	* 创建名字为programName的shader，将shader放到gShader全局变量里面，
	* 拿到shader的各种uniform的loc，为shader里面的sampler指定纹理。
	*/
	bool parseProgram(const string& program);
	/*
	* textureName  material文件里面texture：后面跟的纹理的名字
	* texture material文件里面texture的内容
	* texture的内容要么是一个path=xxxx，这样的路径
	* 要么就是长、宽、深度三个信息确定一个纹理。
	* 这个函数分析texture的内容，创建一个纹理，将纹理保存到gTexture全局变量里面
	*/
	bool parseTexture(const string& textureName, const string& texture);

	/*
	* 分析.material,.program程序里面的config配置项，只支持number与string
	*/
	bool parseConfig(const string& cfgName, const string& value);

	/*
	* textureName  material文件里面texture：后面跟的纹理的名字
	* texture material文件里面texture的内容
	* texture的内容要么是一个path=xxxx，这样的路径
	* 要么就是长、宽、深度三个信息确定一个纹理。
	* 这个函数分析texture的内容，创建一个纹理，将纹理保存到gTexture全局变量里面
	*/
	bool parseCubeTexture(const string& textureName, const string& texture);
	void setDepthTest(bool b);
	void setCullWhichFace(bool b, int fontface);
	void setBlend(bool b, unsigned int srcFactor, unsigned int destFactor, unsigned int blendOp,
		unsigned int srcFactorA, unsigned int destFactorA, unsigned int blendOpA);
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
	static bool mvpMatrixHandler(Material* pMat, const std::string&);
	static bool mvMatrixHandler(Material* pMat, const std::string&);
	static bool vMatrixHandler(Material* pMat, const std::string&);
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

	std::unordered_map<std::string, std::string> mContents;//保存的是材质文件里面形如key{value}的key-value对
	std::shared_ptr<Shader> mShader;
	std::shared_ptr<Color> mpUniformColor;//纯色物体设置这个
	float mMetallical{ 0.5f }; //金属还是非金属（0.0f-1.0f);
	float mRoughness{ 0.5f };	//粗糙程度（0.0f-1.0f);
	float mAo{ 0.1f };
	std::unordered_map<int, std::shared_ptr<Texture>> mSamplerName2Texture;

	std::string mName;

	std::shared_ptr <OpData> mOthersOpData;
	std::shared_ptr <OpData> mMyOpData;
	std::unordered_map<std::string, std::any> mConfigValues;

	static std::unordered_map<std::string, std::shared_ptr<Material>> gMaterials;
	static std::unordered_map<std::string, std::shared_ptr<Texture>> gTextures;
	static std::unordered_map<std::string, std::shared_ptr<Shader>> gShaders;
	static std::unordered_map<std::string, std::function<bool(Material* pMat, const std::string&)>> gMaterialHandlers;
	static std::unordered_map<std::string, std::function<bool(Material* pMat,const std::string&)>> gProgramKeyValueHandlers;
	//static std::unordered_map<std::string, std::function<bool(Material* pMat, const std::string&)>> gMatFileHandlers;
};

using MaterialP = std::shared_ptr<Material>;

extern MaterialP gpMaterialNothing;
#endif