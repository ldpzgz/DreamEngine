/*
 * GraphicsShader.cpp
 *
 *  Created on: 2015-9-19
 *      Author: Administrator
 */
//#include "StdAfx.h"
#include "GraphicsShader.h"
#include "../Log.h"
#include <string>
#include <fstream>
extern void checkglerror();
GraphicsShader::GraphicsShader():mVs(0),mFs(0),mProgram(0) {
	// TODO Auto-generated constructor stub

}

GraphicsShader::~GraphicsShader()
{
	// TODO Auto-generated destructor stub
	deleteShader();
}

bool GraphicsShader::initShaderFromFile(const char* vsFile,const char* psFile)
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

bool GraphicsShader::initShader(const char* vs,const char* ps) {
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
		LOGD("GraphicsShader::initShader loadShader Failed!!\n");
		return false;
	}
	return true;
}

GLuint GraphicsShader::loadShader(GLenum type, const char *shaderSrc)
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


void GraphicsShader::useMe()
{
	glUseProgram ( mProgram );
}

//结果小于0表示错误
int GraphicsShader::getAttributeLoc(const char* attrName)
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
int GraphicsShader::getUniformLoc(const char* uniformName)
{
	return glGetUniformLocation(mProgram, uniformName);
	/*if(uniformName!=0)
	{
		std::map<std::string,int>::iterator it = mUniformLocMap.find(uniformName);
		if(it!=mUniformLocMap.end())
		{
			return it->second;
		}
	}*/
	return -1;
}

void GraphicsShader::setUniform1i(const char* uniformName,int value)
{
	int loc = glGetUniformLocation(mProgram, uniformName);
	glUniform1i(loc,value);

	checkglerror();
}
void GraphicsShader::setUniform1f(const char* uniformName,float x)
{
	int loc = glGetUniformLocation(mProgram, uniformName);
	glUniform1f(loc,x);
	checkglerror();
}
void GraphicsShader::setUniform2f(const char* uniformName,float x,float y)
{
	int loc = glGetUniformLocation(mProgram, uniformName);
	glUniform2f(loc,x,y);
}
void GraphicsShader::setUniform3f(const char* uniformName,float x,float y,float z)
{
	int loc = glGetUniformLocation(mProgram, uniformName);
	glUniform3f(loc,x,y,z);
}
void GraphicsShader::setUniform4f(const char* uniformName,float x,float y,float z,float w)
{
	int loc = glGetUniformLocation(mProgram, uniformName);
	glUniform4f(loc,x,y,z,w);
}

void GraphicsShader::deleteShader()
{
	if(mProgram>0)
	{
		glDeleteProgram(mProgram);
	}
}