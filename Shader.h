/*
 * Shader.h
 *
 *  Created on: 2015-9-19
 *      Author: ldp
 */

#ifndef GRAPHICSSHADER_H_
#define GRAPHICSSHADER_H_
#include <GLES3/gl3.h>
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
#include "Log.h"


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

	void setTextureForSampler(int samplerLoc, std::shared_ptr<Texture>& pTexture) {
		if (!mSamplerToTex.try_emplace(samplerLoc, pTexture).second) {
			LOGD("setTextureForSampler emplace failed");
		}
	}

	void setUniform1i(const char* uniformName,int value);
	void setUniform1f(const char* uniformName,float x);
	void setUniform2f(const char* uniformName,float x,float y);
	void setUniform3f(const char* uniformName,float x,float y,float z);
	void setUniform4f(const char* uniformName,float x,float y,float z,float w);
	void setMvpMatrix(const float*);

	void enable();

	void getMvpMatrixLoc(const std::string& mvpMatrixNameInShader);
private:
	GLuint mVs;
	GLuint mFs;
	GLuint mProgram;
	int mPosLoc;
	int mTexcoordLoc;
	int mColorLoc;
	int mNormalLoc;
	int mMvpMatrixLoc;
	std::string mName;
	std::unique_ptr<std::array<float,16>> mMvpMatrix;
	//int mSamplerCount;
	std::map<std::string,int> mAttributeLocMap;
	std::map<std::string,int> mUniformLocMap;
	std::unordered_map<int, std::shared_ptr<Texture>> mSamplerToTex;//Sampler in shader to Texture;
protected:
	//GL_VERTEX_SHADER,GL_FRAGMENT_SHADER
	GLuint loadShader ( GLenum type, const char *shaderSrc );
};
#endif /* GRAPHICSSHADER_H_ */
