#pragma once

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
	bool metallicRoughnessWorkFlow{true};//false is specularGlossnessWorkflow

	//for metallic/roughness workflow
	std::string armMap;
	//std::shared_ptr<Texture> pMetaRoughMap;

	//金属性，要么是一个固定值，要么是map
	float metallic{ 0.0f };
	std::string metallicMap;
	//std::shared_ptr<Texture> pMetaTex;

	//粗糙度，要么是一个固定值，要么是map
	float roughness{ 0.8f };//=1.0f-glossness
	std::string roughnessMap;
	//std::shared_ptr<Texture> pRoughTex;

	/*specularglossness map，for pbr specular / glossness workflow
	* this workflow has two rgb textures,the one is specular the other is diffuse
	* the specular record the reflective of metal,and the f0 value of non-metal.
	*	reflective Of metal will be a higher value(180-255srgb), some is colorful,such as 铜，
	*	F0 for dielectrics will be a darker value(40-75srgb)
	* the diffuse(albedo),The areas that indicate raw metal will be zero,
	* 	oxidation of metal area will has color,the same dust,dirt.
	*	for non-metal,the lowest value should not be lower than 30 sRGB (tolerant range) or 50 sRGB (strict range),
	*	Bright values should not be higher than 240 sRGB
	*/
	std::string specGlosMapPath;
	//std::shared_ptr<Texture> pSpecGlosMap;
	//std::shared_ptr<Texture> pSpecTex;
	Color specularColor;

	//albedo,for specular/glossness workflow this diffuse,metal has no diffuse to the diffuse is zero,
	//std::string albedo;// { "#ffffff" };
	Color albedoColor{1.0f,1.0f,1.0f,1.0f};
	std::string albedoMap;//or diffuseMap
	bool hasVertexColor{false};
	//std::shared_ptr<Texture> pAlbedoTex;

	//normal
	bool hasNormal{ true };
	std::string normalMap;
	//std::shared_ptr<Texture> pNormalTex;

	//ao
	float ao{ 0.2f };
	std::string aoMap;
	//std::shared_ptr<Texture> pAoTex;
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
