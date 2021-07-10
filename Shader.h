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
	Shader();
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

	void setTextureForSampler(const std::string& samplerName, const std::string& textureName) {
		if (!mSamplerToTex.try_emplace(std::move(samplerName), std::move(textureName)).second) {
			LOGD("setTextureForSampler emplace %s failed",samplerName.c_str());
		}
	}

	void setUniform1i(const char* uniformName,int value);
	void setUniform1f(const char* uniformName,float x);
	void setUniform2f(const char* uniformName,float x,float y);
	void setUniform3f(const char* uniformName,float x,float y,float z);
	void setUniform4f(const char* uniformName,float x,float y,float z,float w);

	void enable();

protected:
	GLuint mVs;
	GLuint mFs;
	GLuint mProgram;
	int mPosLoc;
	int mTexcoordLoc;
	int mColorLoc;
	int mNormalLoc;
	//int mSamplerCount;
private:
	std::map<std::string,int> mAttributeLocMap;
	std::map<std::string,int> mUniformLocMap;
	std::unordered_map<std::string, std::string> mSamplerToTex;//Sampler in shader to Texture;
protected:
	//GL_VERTEX_SHADER,GL_FRAGMENT_SHADER
	GLuint loadShader ( GLenum type, const char *shaderSrc );
};
#endif /* GRAPHICSSHADER_H_ */
