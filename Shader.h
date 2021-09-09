﻿/*
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

	bool initShader(const char* vs,const char* ps);

	bool initShaderFromFile(const char* vsFile,const char* psFile);

	void setLocation(int pos, int tex=-1, int color=-1, int nor=-1) {
		mPosLoc = pos;
		mTexcoordLoc = tex;
		mColorLoc = color;
		mNormalLoc = nor;
	}

	void getLocation(int& posLoc, int& texcoordLoc, int& colorLoc, int& normalLoc) {
		posLoc = mPosLoc;
		texcoordLoc = mTexcoordLoc;
		colorLoc = mColorLoc;
		normalLoc = mNormalLoc;
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
	int getAttributeLoc(const char* attrName);
	//结果小于0表示错误
	int getUniformLoc(const char* uniformName);

	void setTextureForSampler(int samplerLoc, const std::shared_ptr<Texture>& pTexture) {
		auto it = mSamplerToTex.find(samplerLoc);
		if (it != mSamplerToTex.end()) {
			it->second = pTexture;
		}else{
			if (!mSamplerToTex.try_emplace(samplerLoc, pTexture).second) {
				LOGD("changeTexture emplace failed");
			}
		}
	}

	void setUniform1i(const char* uniformName,int value);
	void setUniform1f(const char* uniformName,float x);
	void setUniform2f(const char* uniformName,float x,float y);
	void setUniform3f(const char* uniformName,float x,float y,float z);
	void setUniform4f(const char* uniformName,float x,float y,float z,float w);
	void setMvpMatrix(const glm::mat4&);
	void setTextureMatrix(const glm::mat4&);
	void setUniformColor(float r, float g, float b, float a);
	void setUniformColor(Color color);

	void enable();

	void getMvpMatrixLoc(const std::string& mvpMatrixNameInShader);
	void getTextureMatrixLoc(const std::string& textureMatrixNameInShader);
	void getUniformColorLoc(const std::string& uniformColorNameInShader);

	std::unordered_map<int, std::shared_ptr<Texture>>& getTexture() {
		return mSamplerToTex;
	}
private:
	GLuint mVs{ 0 };
	GLuint mFs{ 0 };
	GLuint mProgram{ 0 };
	int mPosLoc{ -1 };
	int mTexcoordLoc{ -1 };
	int mColorLoc{ -1 };
	int mNormalLoc{ -1 };

	int mMvpMatrixLoc{ -1 };
	int mTextureMatrixLoc{ -1 };
	int mUniformColorLoc{ -1 }; //在fs里面可以有个uniform vec4 color，用于设置输出固定颜色
	std::string mName;
	glm::mat4 mMvpMatrix;
	glm::mat4 mTextureMatrix;
	//int mSamplerCount;
	std::map<std::string,int> mAttributeLocMap;
	std::map<std::string,int> mUniformLocMap;
	std::unordered_map<int, std::shared_ptr<Texture>> mSamplerToTex;//Sampler in shader to Texture;
	Color mUniformColor{ 0.0f,0.0f,0.0f,0.0f };
protected:
	//GL_VERTEX_SHADER,GL_FRAGMENT_SHADER
	GLuint loadShader ( GLenum type, const char *shaderSrc );
};
#endif /* GRAPHICSSHADER_H_ */
