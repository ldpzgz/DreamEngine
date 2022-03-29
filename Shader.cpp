/*
 * Shader.cpp
 *
 *  Created on: 2015-9-19
 *      Author: Administrator
 */
//#include "StdAfx.h"
#include "Shader.h"
#include <glm/trigonometric.hpp>  //sin cos,tan,radians,degree
#include <glm/ext/matrix_transform.hpp> // perspective, translate, rotate
#include <glm/gtc/type_ptr.hpp> // value_ptr
#include "Log.h"
#include <string>
#include <fstream>
extern void checkglerror();

Shader::Shader(const std::string& name) :
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
				//c++17才支持
				auto infoLog = std::make_unique<char[]>((sizeof(char)* infoLen));
				glGetProgramInfoLog(mProgram, infoLen, NULL, infoLog.get());
				LOGD("Error linking program:\n%s\n", infoLog.get());
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
			auto tempName = std::make_unique<char[]>(maxNameLength);
			GLsizei namelength;
			for(int i=0; i<activeNum; ++i)
			{
				glGetActiveUniform(mProgram,i,maxNameLength,&namelength,&size,&type,tempName.get());
				loc = glGetUniformLocation(mProgram,tempName.get());
				mUniformLocMap.emplace(tempName.get(), loc);
				//Log::info("loc: %s,%d",tempName,loc);
			}
			
			mAttributeLocMap.clear();
			glGetProgramiv(mProgram, GL_ACTIVE_ATTRIBUTES, &activeNum);
			glGetProgramiv(mProgram, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH,&maxNameLength);
			tempName = std::make_unique<char[]>(maxNameLength);
			for(int i=0; i<activeNum; ++i)
			{
				glGetActiveAttrib(mProgram,i,maxNameLength,&namelength,&size,&type,tempName.get());
				loc = glGetAttribLocation(mProgram,tempName.get());
				mAttributeLocMap.emplace(tempName.get(), loc);
				//Log::info("loc: %s,%d",tempName,loc);
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
			auto infoLog = std::make_unique<char[]>((sizeof(char) * infoLen));

			glGetShaderInfoLog(shader, infoLen, NULL, infoLog.get());
			LOGD("Error compiling shader:\n%s\n", infoLog.get());
		}

		glDeleteShader(shader);
		return 0;
	}

	return shader;
}

void Shader::disable() {
	glUseProgram(0);
}
void Shader::enable()
{
	glUseProgram ( mProgram );

	if (mUniformColorLoc >= 0) {
		glUniform4f(mUniformColorLoc, mUniformColor[0], mUniformColor[1], mUniformColor[2], mUniformColor[3]);
	}

	if (mMetallicLoc >= 0) {
		glUniform1f(mMetallicLoc, mMetallic);
	}

	if (mRoughnessLoc >= 0) {
		glUniform1f(mRoughnessLoc, mRoughness);
	}

	if (mAlbedoColorLoc >= 0) {
		glUniform4f(mAlbedoColorLoc, mUniformColor[0], mUniformColor[1], mUniformColor[2], mUniformColor[3]);
	}

	if (mAoLoc >= 0) {
		glUniform1f(mAoLoc, mAo);
	}
}

//结果小于0表示错误
int Shader::getAttributeLoc(const std::string& attrName)
{
	if(!attrName.empty())
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
int Shader::getUniformLoc(const std::string& uniformName)
{
	//return glGetUniformLocation(mProgram, uniformName);
	if (!uniformName.empty())
	{
		auto it = mUniformLocMap.find(uniformName);
		if (it != mUniformLocMap.end())
		{
			return it->second;
		}
	}
	return -1;
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
void Shader::setPreMvpMatrix(const glm::mat4& pMat) {
	if (mPreMvpMatrixLoc >= 0) {
		glUniformMatrix4fv(mPreMvpMatrixLoc, 1, GL_FALSE, glm::value_ptr(pMat));
	}
	else {
		LOGD("the mPreMvpMatrixLoc of shader %s has not been got", mName.c_str());
	}
}
void Shader::setMvpMatrix(const glm::mat4& pMatrix) {
	if (mMvpMatrixLoc >= 0 ) {
			glUniformMatrix4fv(mMvpMatrixLoc, 1, GL_FALSE, glm::value_ptr(pMatrix));
	}
	else {
		LOGD("the mMvpMatrixLoc of shader %s has not been got",mName.c_str());
	}
}

void Shader::setTextureMatrix(const glm::mat4& pMatrix) {
	if (mTextureMatrixLoc >= 0) {
		glUniformMatrix4fv(mTextureMatrixLoc, 1, GL_FALSE, glm::value_ptr(pMatrix));
	}
	else {
		LOGD("the mTextureMatrixLoc of shader %s has not been got", mName.c_str());
	}
}

void Shader::setUniformColor(float r, float g, float b, float a)
{
	mUniformColor[0] = r;
	mUniformColor[1] = g;
	mUniformColor[2] = b;
	mUniformColor[3] = a;
}

void Shader::setAlbedoColor(float r, float g, float b)
{
	mUniformColor[0] = r;
	mUniformColor[1] = g;
	mUniformColor[2] = b;
}

void Shader::setUniformColor(Color color) {
	mUniformColor = color;
}
void Shader::setLightCount(int count) {
	if (mLightCountLoc >= 0) {
		glUniform1i(mLightCountLoc,count);
	}
}
void Shader::setLightPos(const std::vector<glm::vec3>& lightPos) {
	if (mLightPosLoc >= 0) {
		glUniform3fv(mLightPosLoc, lightPos.size(), (const float*)lightPos.data());
	}
}
void Shader::setViewPos(const glm::vec3& viewPos) {
	if (mViewPosLoc >= 0) {
		glUniform3f(mViewPosLoc, viewPos.x, viewPos.y, viewPos.z);
	}
}
void Shader::setLightColor(const std::vector<glm::vec3>& lightColor) {
	if (mLightColorLoc >= 0) {
		glUniform3fv(mLightColorLoc, lightColor.size(), (const float*)lightColor.data());
	}
}
void Shader::setMvMatrix(const glm::mat4& m) {
	if (mMvMatrixLoc >= 0) {
		glUniformMatrix4fv(mMvMatrixLoc, 1, GL_FALSE, glm::value_ptr(m));
	}
}
void Shader::setViewMatrix(const glm::mat4& m) {
	if (mViewMatrixLoc >= 0) {
		glUniformMatrix4fv(mViewMatrixLoc, 1, GL_FALSE, glm::value_ptr(m));
	}
}

void Shader::setProjMatrix(const glm::mat4& m) {
	if (mProjMatrixLoc >= 0) {
		glUniformMatrix4fv(mProjMatrixLoc, 1, GL_FALSE, glm::value_ptr(m));
	}
}

void Shader::setModelMatrix(const glm::mat4& m) {
	if (mModelMatrixLoc >= 0) {
		glUniformMatrix4fv(mModelMatrixLoc, 1, GL_FALSE, glm::value_ptr(m));
	}
}

void Shader::getProjMatrixLoc(const std::string& projectMatrixNameInShader) {
	mProjMatrixLoc = glGetUniformLocation(mProgram, projectMatrixNameInShader.c_str());
	if (mProjMatrixLoc < 0) {
		LOGE("the shader %s  has no %s uniform member", mName.c_str(), projectMatrixNameInShader.c_str());
	}
}

void Shader::getModelMatrixLoc(const std::string& modelMatrixNameInShader) {
	mModelMatrixLoc = glGetUniformLocation(mProgram, modelMatrixNameInShader.c_str());
	if (mModelMatrixLoc < 0) {
		LOGE("the shader %s  has no %s uniform member", mName.c_str(), modelMatrixNameInShader.c_str());
	}
}

void Shader::getMvpMatrixLoc(const std::string& mvpMatrixNameInShader) {
	mMvpMatrixLoc = glGetUniformLocation(mProgram, mvpMatrixNameInShader.c_str());
	if (mMvpMatrixLoc < 0) {
		LOGE("the shader %s  has no %s uniform member", mName.c_str(), mvpMatrixNameInShader.c_str());
	}
}

void Shader::getMvMatrixLoc(const std::string& mvMatrixNameInShader) {
	mMvMatrixLoc = glGetUniformLocation(mProgram, mvMatrixNameInShader.c_str());
	if (mMvMatrixLoc < 0) {
		LOGE("the shader %s  has no %s uniform member", mName.c_str(), mvMatrixNameInShader.c_str());
	}
}

void Shader::getPreMvpMatrixLoc(const std::string& preMvpMatrixNameInShader) {
	mPreMvpMatrixLoc = glGetUniformLocation(mProgram, preMvpMatrixNameInShader.c_str());
	if (mPreMvpMatrixLoc < 0) {
		LOGE("the shader %s  has no %s uniform member", mName.c_str(), preMvpMatrixNameInShader.c_str());
	}
}

void Shader::getViewMatrixLoc(const std::string& viewMatrixNameInShader) {
	mViewMatrixLoc = glGetUniformLocation(mProgram, viewMatrixNameInShader.c_str());
	if (mViewMatrixLoc < 0) {
		LOGE("the shader %s  has no %s uniform member", mName.c_str(), viewMatrixNameInShader.c_str());
	}
}
void Shader::getLightCountLoc(const std::string& lightCountNameInShader) {
	mLightCountLoc = glGetUniformLocation(mProgram, lightCountNameInShader.c_str());
	if (mLightCountLoc < 0) {
		LOGE("the shader %s  has no %s uniform member", mName.c_str(), lightCountNameInShader.c_str());
	}
}
void Shader::getLightPosLoc(const std::string& lightPosNameInShader) {
	mLightPosLoc = glGetUniformLocation(mProgram, lightPosNameInShader.c_str());
	if (mLightPosLoc < 0) {
		LOGE("the shader %s  has no %s uniform member", mName.c_str(), lightPosNameInShader.c_str());
	}
}
void Shader::getViewPosLoc(const std::string& viewPosNameInShader) {
	mViewPosLoc = glGetUniformLocation(mProgram, viewPosNameInShader.c_str());
	if (mViewPosLoc < 0) {
		LOGE("the shader %s  has no %s uniform member", mName.c_str(), viewPosNameInShader.c_str());
	}
}
void Shader::getLightColorLoc(const std::string& lightColorNameInShader) {
	mLightColorLoc = glGetUniformLocation(mProgram, lightColorNameInShader.c_str());
	if (mLightColorLoc < 0) {
		LOGE("the shader %s  has no %s uniform member", mName.c_str(), lightColorNameInShader.c_str());
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

void Shader::getAlbedoColorLoc(const std::string& albedoNameInShader) {
	mAlbedoColorLoc = glGetUniformLocation(mProgram, albedoNameInShader.c_str());
	if (mAlbedoColorLoc < 0) {
		LOGE("the shader %s  has no %s uniform member", mName.c_str(), albedoNameInShader.c_str());
	}
}

void Shader::getMetallicLoc(const std::string& value) {
	mMetallicLoc = glGetUniformLocation(mProgram, value.c_str());
	if (mMetallicLoc < 0) {
		LOGE("the shader %s  has no %s uniform member", mName.c_str(), value.c_str());
	}
}

void Shader::getRoughnessLoc(const std::string& value) {
	mRoughnessLoc = glGetUniformLocation(mProgram, value.c_str());
	if (mRoughnessLoc < 0) {
		LOGE("the shader %s  has no %s uniform member", mName.c_str(), value.c_str());
	}
}

void Shader::getAoLoc(const std::string& value) {
	mAoLoc = glGetUniformLocation(mProgram, value.c_str());
	if (mAoLoc < 0) {
		LOGE("the shader %s  has no %s uniform member", mName.c_str(), value.c_str());
	}
}
void Shader::deleteShader()
{
	if(mProgram>0)
	{
		glDeleteProgram(mProgram);
	}
}


//只bind一次，渲染的时候就不用设置了，渲染过程中只改ubo中的值就好了
void Shader::bindUniformBlock(const char* uboName, unsigned int bindPoint) {
	//shader的uniform block有一个index，让这个index与某个uniform buffer bindpoint联系起来
	unsigned int ubIndex = glGetUniformBlockIndex(mProgram,uboName);
	if (GL_INVALID_INDEX != ubIndex)
	{
		//先把自己bind到bindPoint
		glUniformBlockBinding(mProgram, ubIndex, bindPoint);
		int sizeInBytes = 0;
		glGetActiveUniformBlockiv(mProgram, ubIndex, GL_UNIFORM_BLOCK_DATA_SIZE, &sizeInBytes);
		if (sizeInBytes > 0) {
			//再把ubo绑定到bindPoint
			Ubo::getInstance().bind(uboName, sizeInBytes, bindPoint);
		}
		else {
			LOGE("cannot get uniform block size %s", uboName);
		}
		checkglerror();
	}
	else {
		LOGE("cannot get uniform block index %s",uboName);
	}
}

//void Shader::updateUniformBlock(const char* uboName, void* pdata, int sizeInByte) {
//	Ubo::getInstance().update(uboName, pdata, sizeInByte);
//}