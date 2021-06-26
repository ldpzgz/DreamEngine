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

	bool initShader(const char* vs,const char* ps);

	bool initShaderFromFile(const char* vsFile,const char* psFile);

	void deleteShader();
	//结果小于0表示错误
	int getAttributeLoc(const char* attrName);
	//结果小于0表示错误
	int getUniformLoc(const char* uniformName);

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
private:
	std::map<std::string,int> mAttributeLocMap;
	std::map<std::string,int> mUniformLocMap;

protected:
	//GL_VERTEX_SHADER,GL_FRAGMENT_SHADER
	GLuint loadShader ( GLenum type, const char *shaderSrc );
};
#endif /* GRAPHICSSHADER_H_ */
