/*
 * GraphicsFbo.h
 *
 *  Created on: 2015-9-29
 *      Author: Administrator
 */

#ifndef GRAPHICSFBO_H_
#define GRAPHICSFBO_H_
#include<memory>
#include "Texture.h"
//fbo主要用于渲染到纹理，这样子是比较高效的。
//如果不用fbo，有两种方式，可以实现渲染到纹理，
//1 利用glCopyTexImage2D,glCopyTexSubImage2D复制窗口系统的渲染缓存
//	图像到主内存。
//2 渲染到pbuffer。
//这两种方式效率都不高。
//fbo是单缓存的，不支持多重采样缓存
//一个fbo有三个attachment：color，depth，stencil。
//stencil 只能挂rbo
class Fbo
{
public:
	Fbo();
	virtual ~Fbo();
	
	bool attachColorTexture(const std::shared_ptr<Texture>& texture,GLint level=0);
	bool attachDepthTexture(const std::shared_ptr<Texture>& texture,GLint level=0);
	

	void startRender();
	void endRender();

	void setClearColor(float r, float g, float b, float a);
private:
	void deleteFbo();
	void enable();
	void disable();
	GLuint mFboId;
	int mWidth; //render to tex ,tex width
	int mHeight;//render to tex ,tex height
	bool mbIsAttach;
	float mClearColor[4]{0.0f,0.0f,0.0f,0.0f};
};

#endif /* GRAPHICSFBO_H_ */
