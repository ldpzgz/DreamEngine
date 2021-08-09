﻿/*
 * Shader.cpp
 *
 *  Created on: 2015-9-19
 *      Author: Administrator
 */
//#include "StdAfx.h"
#include "Shader.h"
#include "Log.h"
#include <string>
#include <fstream>
extern void checkglerror();
Shader::Shader(const std::string& name) :
	mVs(0),
	mFs(0),
	mProgram(0),
	mPosLoc(-1),
	mTexcoordLoc(-1),
	mColorLoc(-1),
	mNormalLoc(-1),
	mMvpMatrixLoc(-1),
	mTextureMatrixLoc(-1),
	mName(name)
{
	// TODO Auto-generated constructor stub

}

Shader::~Shader()
{
	// TODO Auto-generated destructor stub
	deleteShader();
}

bool Shader::initShaderFromFile(const char* vsFile,const char* psFile)
{
	bool ret = false;
	std::ifstream vsStream(vsFile);
	std::ifstream psStream(psFile);

	std::string vsTemp((std::istreambuf_iterator<char>(vsStream)),
		std::istreambuf_iterator<char>());
	std::string psTemp((std::istreambuf_iterator<char>(psStream)),
		std::istreambuf_iterator<char>());

	const char* vsStr = vsTemp.c_str();
	const char* psStr = psTemp.c_str();

	if(vsStr && psStr)
	{
		ret = initShader((const char*)vsStr,(const char*)psStr);
	}
	
	return ret;
}

bool Shader::initShader(const std::string& vs, const std::string& ps) {
	return initShader(vs.c_str(), ps.c_str());
}

bool Shader::initShader(const char* vs,const char* ps) {
	GLint linked;
	
	mVs = loadShader(GL_VERTEX_SHADER, vs);
	mFs = loadShader(GL_FRAGMENT_SHADER, ps);

	if (mVs != 0 && mFs != 0)
	{
		mProgram = glCreateProgram();
		if (mProgram == 0)
		{
			return false;
		}
		glAttachShader(mProgram, mVs);
		glAttachShader(mProgram, mFs);


		// Link the program
		glLinkProgram (mProgram);


		// Check the link status
		glGetProgramiv(mProgram, GL_LINK_STATUS, &linked);

		if (!linked)
		{
			GLint infoLen = 0;

			glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &infoLen);

			if (infoLen > 1)
			{
				char* infoLog = (char*)malloc(sizeof(char) * infoLen);

				glGetProgramInfoLog(mProgram, infoLen, NULL, infoLog);
				LOGD("Error linking program:\n%s\n", infoLog);

				free(infoLog);
			}

			glDeleteProgram(mProgram);
			return false;
		}
		else
		{
			//获取program里面的uniform，attribute等属性的location，分别保存到map里面。
			int maxNameLength;
			int activeNum;
			GLint size;
			GLenum type;
			int loc;
			mUniformLocMap.clear();
			glGetProgramiv(mProgram,GL_ACTIVE_UNIFORM_MAX_LENGTH,&maxNameLength);
			glGetProgramiv(mProgram,GL_ACTIVE_UNIFORMS,&activeNum);
			char* tempName = new char[maxNameLength];
			GLsizei namelength;
			for(int i=0; i<activeNum; ++i)
			{
				glGetActiveUniform(mProgram,i,maxNameLength,&namelength,&size,&type,tempName);
				loc = glGetUniformLocation(mProgram,(const char*)tempName);
				mUniformLocMap.insert(std::make_pair(std::string(tempName),loc));
				//Log::info("loc: %s,%d",tempName,loc);
			}
			if(tempName!=0)
			{
				delete []tempName;
			}
			
			mAttributeLocMap.clear();
			glGetProgramiv(mProgram, GL_ACTIVE_ATTRIBUTES, &activeNum);
			glGetProgramiv(mProgram, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH,&maxNameLength);
			tempName = new char[maxNameLength];
			for(int i=0; i<activeNum; ++i)
			{
				glGetActiveAttrib(mProgram,i,maxNameLength,&namelength,&size,&type,tempName);
				loc = glGetAttribLocation(mProgram,(const char*)tempName);
				mAttributeLocMap.insert(std::make_pair(std::string(tempName),loc));
				//Log::info("loc: %s,%d",tempName,loc);
			}
			if(tempName!=0)
			{
				delete []tempName;
			}
			glDeleteShader(mFs);
			glDeleteShader(mVs);
		}
	}
	else
	{
		LOGD("Shader::initShader loadShader Failed!!\n");
		if (mVs != 0) {
			glDeleteShader(mVs);
			mVs = 0;
		}
		if (mFs != 0) {
			glDeleteShader(mFs);
			mFs = 0;
		}
		return false;
	}
	return true;
}

GLuint Shader::loadShader(GLenum type, const char *shaderSrc)
{
	GLuint shader;
	GLint compiled;

	// Create the shader object
	shader = glCreateShader(type);

	if (shader == 0)
		return 0;

	// Load the shader source
	glShaderSource(shader, 1, &shaderSrc, NULL);

	// Compile the shader
	glCompileShader(shader);

	// Check the compile status
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

	if (!compiled) {
		GLint infoLen = 0;

		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

		if (infoLen > 1) {
			char* infoLog = (char*) malloc(sizeof(char) * infoLen);

			glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
			LOGD("Error compiling shader:\n%s\n", infoLog);

			free(infoLog);
		}

		glDeleteShader(shader);
		return 0;
	}

	return shader;
}


void Shader::enable()
{
	glUseProgram ( mProgram );
	int texNum = 0;
	for (auto it = mSamplerToTex.begin(); it != mSamplerToTex.end(); it++) {
		it->second->active(GL_TEXTURE0 + texNum);
		glUniform1i(it->first, texNum);
	}

	if (mMvpMatrixLoc >= 0 && mMvpMatrix) {
		glUniformMatrix4fv(mMvpMatrixLoc, 1, GL_FALSE, mMvpMatrix->data());
	}

	if (mTextureMatrixLoc >= 0 && mTextureMatrix) {
		glUniformMatrix3fv(mTextureMatrixLoc, 1, GL_FALSE, mTextureMatrix->data());
	}

	if (mUniformColorLoc >= 0) {
		glUniform4f(mUniformColorLoc, mUniformColor.r, mUniformColor.g, mUniformColor.b, mUniformColor.a);
	}
}

//结果小于0表示错误
int Shader::getAttributeLoc(const char* attrName)
{
	if(attrName!=0)
	{
		std::map<std::string,int>::iterator it = mAttributeLocMap.find(attrName);
		if(it!=mAttributeLocMap.end())
		{
			return it->second;
		}
	}
	return -1;
}
//结果小于0表示错误
int Shader::getUniformLoc(const char* uniformName)
{
	return glGetUniformLocation(mProgram, uniformName);
}

void Shader::setUniform1i(const char* uniformName,int value)
{
	int loc = glGetUniformLocation(mProgram, uniformName);
	glUniform1i(loc,value);

	//checkglerror();
}
void Shader::setUniform1f(const char* uniformName,float x)
{
	int loc = glGetUniformLocation(mProgram, uniformName);
	glUniform1f(loc,x);
	checkglerror();
}
void Shader::setUniform2f(const char* uniformName,float x,float y)
{
	int loc = glGetUniformLocation(mProgram, uniformName);
	glUniform2f(loc,x,y);
}
void Shader::setUniform3f(const char* uniformName,float x,float y,float z)
{
	int loc = glGetUniformLocation(mProgram, uniformName);
	glUniform3f(loc,x,y,z);
}
void Shader::setUniform4f(const char* uniformName,float x,float y,float z,float w)
{
	int loc = glGetUniformLocation(mProgram, uniformName);
	glUniform4f(loc,x,y,z,w);
}

void Shader::setMvpMatrix(const float* pMatrix) {
	if (mMvpMatrixLoc >= 0 && pMatrix != nullptr) {
		if (!mMvpMatrix) {
			mMvpMatrix = std::make_unique<std::vector<float>>(16);
		}else{
			for (int i = 0; i < 16; ++i) {
				(*mMvpMatrix)[i] = pMatrix[i];
			}
		}
	}
	else {
		LOGD("the mMvpMatrixLoc of shader %s has not been got",mName.c_str());
	}
}

void Shader::setTextureMatrix(const float* pMatrix) {
	if (mTextureMatrixLoc >= 0 && pMatrix != nullptr) {
		if (!mTextureMatrix) {
			mTextureMatrix = std::make_unique<std::vector<float>>(9);
		}
		else {
			for (int i = 0; i < 9; ++i) {
				(*mTextureMatrix)[i] = pMatrix[i];
			}
		}
	}
}

void Shader::setUniformColor(float r, float g, float b, float a)
{
	mUniformColor.r = r;
	mUniformColor.g = g;
	mUniformColor.b = b;
	mUniformColor.a = a;
}

void Shader::setUniformColor(Color color) {
	mUniformColor = color;
}


void Shader::getMvpMatrixLoc(const std::string& mvpMatrixNameInShader) {
	mMvpMatrixLoc = glGetUniformLocation(mProgram, mvpMatrixNameInShader.c_str());
	if (mMvpMatrixLoc < 0) {
		LOGE("the shader %s  has no %s uniform member", mName.c_str(), mvpMatrixNameInShader.c_str());
	}
}

void Shader::getTextureMatrixLoc(const std::string& textureMatrixNameInShader) {
	mTextureMatrixLoc = glGetUniformLocation(mProgram, textureMatrixNameInShader.c_str());
	if (mTextureMatrixLoc < 0) {
		LOGE("the shader %s  has no %s textureMatrix", mName.c_str(), textureMatrixNameInShader.c_str());
	}
}

void Shader::getUniformColorLoc(const std::string& uniformColorNameInShader) {
	mUniformColorLoc = glGetUniformLocation(mProgram, uniformColorNameInShader.c_str());
	if (mUniformColorLoc < 0) {
		LOGE("the shader %s  has no %s uniform member", mName.c_str(), uniformColorNameInShader.c_str());
	}
}

void Shader::deleteShader()
{
	if(mProgram>0)
	{
		glDeleteProgram(mProgram);
	}
}