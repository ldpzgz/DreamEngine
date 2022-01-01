/*
 * Shader.h
 *
 *  Created on: 2015-9-19
 *      Author: ldp
 */

#ifndef GRAPHICSSHADER_H_
#define GRAPHICSSHADER_H_
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <iostream>
#include <map>
#include <string>
#include "Texture.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <array>
#include <vector>
#include "Log.h"
#include "Rect.h"

#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>
 // Include all GLM extensions
#include <glm/ext.hpp> // perspective, translate, rotate
#include <glm/gtx/matrix_transform_2d.hpp>


/*
 这个类的用法是：
 	 Shader gs;
 	 if(gs.initShader())
 	 	 ;

 	 //在渲染前调用
 	 gs.useMe();
 	 gs.setVertexPointer(pointer,num);
	 gs.startRender();
 */
class Shader {
public:
	explicit Shader(const std::string& name);
	virtual ~Shader();

	bool initShader(const std::string& vs, const std::string& ps);

	bool initShader(const char* vs, const char* ps);

	bool initShaderFromFile(const char* vsFile, const char* psFile);
	void setPosLoc(int pos) {
		mPosLoc = pos;
	}
	void setTexLoc(int tex) {
		mTexcoordLoc = tex;
	}
	void setColorLoc(int color) {
		mColorLoc = color;
	}
	void setNormalLoc(int nor) {
		mNormalLoc = nor;
	}
	void setTangentLoc(int tangent) {
		mTangentLoc = tangent;
	}
	void setLocation(int pos, int tex = -1, int color = -1, int nor = -1, int tangent = -1) {
		mPosLoc = pos;
		mTexcoordLoc = tex;
		mColorLoc = color;
		mNormalLoc = nor;
		mTangentLoc = tangent;
	}

	void getLocation(int& posLoc, int& texcoordLoc, int& colorLoc, int& normalLoc, int& tangentLoc) {
		posLoc = mPosLoc;
		texcoordLoc = mTexcoordLoc;
		colorLoc = mColorLoc;
		normalLoc = mNormalLoc;
		tangentLoc = mTangentLoc;
	}

	int getPosLoc() {
		return mPosLoc;
	}

	int getTexcoordLoc() {
		return mTexcoordLoc;
	}

	int getColorLoc() {
		return mColorLoc;
	}

	int getNormalLoc() {
		return mNormalLoc;
	}

	/*int getSamplerCount() {
		return mSamplerCount;
	}

	void setSamplerCount(int count) {
		mSamplerCount = count;
	}*/

	void deleteShader();
	//结果小于0表示错误
	int getAttributeLoc(const std::string& attrName);
	//结果小于0表示错误
	int getUniformLoc(const std::string& uniformName);

	//void setTextureForSampler(int samplerLoc, const std::shared_ptr<Texture>& pTexture) {
	//	auto it = mSamplerToTex.find(samplerLoc);
	//	if (it != mSamplerToTex.end()) {
	//		it->second = pTexture;
	//	}else{
	//		if (!mSamplerToTex.try_emplace(samplerLoc, pTexture).second) {
	//			LOGD("changeTexture emplace failed");
	//		}
	//	}
	//}

	void setUniform1i(const char* uniformName, int value);
	void setUniform1f(const char* uniformName, float x);
	void setUniform2f(const char* uniformName, float x, float y);
	void setUniform3f(const char* uniformName, float x, float y, float z);
	void setUniform4f(const char* uniformName, float x, float y, float z, float w);
	void setMvpMatrix(const glm::mat4&);
	void setTextureMatrix(const glm::mat4&);
	void setMvMatrix(const glm::mat4&);
	void setViewMatrix(const glm::mat4&);
	void setUniformColor(float r, float g, float b, float a);
	void setUniformColor(Color color);
	void setLightPos(const std::vector<Vec3>& lightPos);
	void setViewPos(const Vec3& viewPos);
	void setLightColor(const std::vector<Vec3>& lightColor);
	void setMetallic(float m) {
		mMetallic = m;
	}
	void setRoughness(float r) {
		mRoughness = r;
	}

	void enable();

	void getMvpMatrixLoc(const std::string& mvpMatrixNameInShader);
	void getMvMatrixLoc(const std::string& mvMatrixNameInShader);
	void getViewMatrixLoc(const std::string& viewMatrixNameInShader);
	void getLightPosLoc(const std::string& lightPosNameInShader);
	void getViewPosLoc(const std::string& viewPosNameInShader);
	void getLightColorLoc(const std::string& lightColorNameInShader);
	void getTextureMatrixLoc(const std::string& textureMatrixNameInShader);
	void getUniformColorLoc(const std::string& uniformColorNameInShader);
	void getMetallicLoc(const std::string& value);
	void getRoughnessLoc(const std::string& value);
	//void getDiffuseTextureLoc(const std::string& diffuseSamplerInShader);
	//void getNormalTextureLoc(const std::string& normalSamplerInShader);

	std::vector<std::string>& getSamplerNames() {
		return mSamplerNames;
	}

	std::map<std::string, int>& getUniforms() {
		return mUniformLocMap;
	}
	//std::shared_ptr<Texture>& getTexture(const std::string& samplerName) {
	//	auto loc = getUniformLoc(samplerName);
	//	if (loc >= 0) {
	//		auto p = mSamplerToTex.find(loc);
	//		if (p != mSamplerToTex.end()) {
	//			return p->second;
	//		}
	//	}
	//	return gpTextureNothing;
	//}
private:
	GLuint mVs{ 0 };
	GLuint mFs{ 0 };
	GLuint mProgram{ 0 };
	int mPosLoc{ -1 };
	int mTexcoordLoc{ -1 };
	int mColorLoc{ -1 };
	int mNormalLoc{ -1 };
	int mTangentLoc{ -1 };

	int mMvpMatrixLoc{ -1 };
	int mMvMatrixLoc{ -1 };
	int mViewMatrixLoc{ -1 };
	int mTextureMatrixLoc{ -1 };
	int mLightPosLoc{ -1 };
	int mViewPosLoc{ -1 };
	int mLightColorLoc{ -1 };
	int mUniformColorLoc{ -1 }; //在fs里面可以有个uniform vec4 color，用于设置输出固定颜色
	int mMetallicLoc{ -1 };
	int mRoughnessLoc{ -1 };
	std::string mName;	//只是用来输出日志
	std::unique_ptr<glm::mat4> mpMvpMatrix;
	std::unique_ptr<glm::mat4> mpMvMatrix;
	std::unique_ptr<glm::mat4> mpViewMatrix;
	std::unique_ptr<glm::mat4> mpTextureMatrix;
	//int mSamplerCount;
	std::map<std::string, int> mAttributeLocMap;
	std::map<std::string, int> mUniformLocMap;
	std::vector<std::string> mSamplerNames;
	//std::unordered_map<int, std::shared_ptr<Texture>> mSamplerToTex;//Sampler in shader to Texture;
	Color mUniformColor{ 0.0f,0.0f,0.0f,0.0f };
	Vec3 mViewPos{ 0.0f,100.0f,0.0f };
	std::vector<Vec3> mLightPositions{ {0.0f,100.0f,0.0f} };
	std::vector<Vec3> mLightColors{ {1.0f,1.0f,1.0f} };
	float mMetallic{ 0.5f };
	float mRoughness{ 0.5f };
protected:
	//GL_VERTEX_SHADER,GL_FRAGMENT_SHADER
	GLuint loadShader ( GLenum type, const char *shaderSrc );
};

using PShader = std::shared_ptr<Shader>;
extern PShader gpShaderNothing;
#endif /* GRAPHICSSHADER_H_ */
