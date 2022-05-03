#ifndef _METERIAL_H_
#define _METERIAL_H_
#include <iostream>
#include <string>
#include <memory>
#include <functional>
#include <unordered_map>
#include <any>
#include <string_view>
#include "Rect.h"
#include "Color.h"

using namespace std;
class Texture;
class Shader;

class MaterialInfo {
public:
	std::string name;
	//ao roughness metallic,三合一map
	std::string armMap;

	//颜色必须要，要么一个color，要么一个map
	std::string albedo{ "#ffffff" };
	Color albedoColor;
	std::string albedoMap;

	//normal可以没有，可以有：要么normalmap，要么顶点normal
	bool hasNormal{ true };
	std::string normalMap;

	//aoMap不为0，就有
	float ao{ 0.2f };
	std::string aoMap;

	//粗糙度，要么是一个固定值，要么是map
	float roughness{ 0.8f };
	std::string roughnessMap;

	//金属性，要么是一个固定值，要么是map
	float metallic{ 0.0f };
	std::string metallicMap;
};

class Material{
public:
	explicit Material(const std::string name):mName(name) {

	}
	Material();
	~Material();
	Material(const Material& mat);
	
	void enable();

	void disable();
	//深度测试，blend，cullface等操作
	void setMyRenderOperation();

	//恢复之前的深度测试，blend，cullface等操作
	void restoreRenderOperation();

	//改变shader里面sampler对应的纹理
	void setTextureForSampler(const char* samplerName, const shared_ptr<Texture>& pTex);
	//改变shader里面sampler对应的纹理
	void setTextureForSampler(int loc, const shared_ptr<Texture>& pTex);

	shared_ptr<Texture> getTextureOfSampler(const string& samplerName);

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

	void setUniform1fv(const std::string& name, int count, float* pdata);

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

	void setDepthTest(bool b);
	void setCullWhichFace(bool b, int fontface);
	void setBlend(bool b, unsigned int srcFactor, unsigned int destFactor, unsigned int blendOp,
		unsigned int srcFactorA, unsigned int destFactorA, unsigned int blendOpA);

private:
	using Umapss = std::unordered_map<std::string, std::string>;
	
	std::shared_ptr<Shader> mShader;
	Color mUniformColor{1.0f,1.0f,1.0f,1.0f};//纯色物体设置这个
	float mMetallical{ 0.0f }; //金属还是非金属（0.0f-1.0f);
	float mRoughness{ 0.5f };	//粗糙程度（0.0f-1.0f);
	float mAo{ 0.5f };
	std::unordered_map<int, std::shared_ptr<Texture>> mSamplerName2Texture;

	std::string mName;

	std::shared_ptr <OpData> mOthersOpData;
	std::shared_ptr <OpData> mMyOpData;
};
using MaterialSP = std::shared_ptr<Material>;
#endif