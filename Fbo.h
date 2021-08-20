/*
 * GraphicsFbo.h
 *
 *  Created on: 2015-9-29
 *      Author: liudongping
 */

#ifndef _FBO_H_
#define _FBO_H_
#include<memory>
#include "Texture.h"
/*fbo主要用于渲染到纹理，这样子是比较高效的。
	一个fbo有三个attachment：n个color attachment，一个depth attachment，一个stencil attachment。
	color Attachment 能attach rbo，纹理，纹理数组中的一个，立方体纹理中的一个面，3D纹理中的一个切片
	depth attachment 能attach rbo，纹理，纹理数组中的某一个纹理。
	stencil 只能attach rbo

	如果渲染的结果不是用于纹理，就是rbo，使用rbo的好处：
	1 支持多重采样
	2 rbo有更好的性能，更适合渲染的格式，
	3 rbo可以添加到fbo的color，depth，stencil attachment。
  fbo与窗口系统默认fbo有什么区别：
  1 像素拥有关系，fbo是全拥有，窗口系统默认的fbo，如果窗口被其他窗口遮挡了，遮挡的部分就非拥有了。
  2 fbo只支持单缓存，窗口系统的默认的fbo支持双缓存。
  3 fbo之间可以共享 stencil and depth buffers，不同的fbo可以attach相同的stencil和depth buffer
	而默认的fbo不支持这种共享，

	frame Buffer blits 从read framebuffer 复制一块矩形区域到write framebuffer
	可以使用这个从多重采样的rbo复制内容到一张纹理里面。使用过程如下
	glBindFramebuffer ( GL_DRAW_FRAMEBUFFER,defaultFramebuffer );
	glBindFramebuffer ( GL_READ_FRAMEBUFFER, userData->fbo );
	glReadBuffer ( GL_COLOR_ATTACHMENT0 );
	glBlitFramebuffer ( 0, 0,esContext->width, esContext->height,
		0, 0,esContext->width/2, esContext->height/2,GL_COLOR_BUFFER_BIT, GL_LINEAR );

	glBindFramebuffer(target,fbo);
		target:GL_FRAMEBUFFER,GL_DRAW_FRAMEBUFFER or GL_READ_FRAMEBUFFER,
			GL_FRAMEBUFFER就相当于（GL_DRAW_FRAMEBUFFER | GL_READ_FRAMEBUFFER）
	glReadBuffer(GLenum src)
		Specifies a color buffer. Accepted values are GL_BACK, GL_NONE, and GL_COLOR_ATTACHMENTi.
		指定readbuffer的fbo里面的哪个buffer为read
	glDrawBuffer(GLsizei n,const GLenum *bufs)
		defines an array of buffers into which outputs from the fragment shader data will be written
		指定drawbuffer的fbo里面的哪个buffer为write
*/
class Fbo
{
public:
	Fbo();
	virtual ~Fbo();
	
	void detachColorTexture(int attachment_n=0, GLint level=0);
	void detachDepthTexture(GLint level = 0);
	bool attachColorTexture(const std::shared_ptr<Texture>& texture,int attachment_n, GLint level=0);
	bool attachDepthTexture(const std::shared_ptr<Texture>& texture,GLint level=0);
	

	void startRender();
	void endRender();

	void setClearColor(float r, float g, float b, float a);
private:
	void deleteFbo();
	void enable();
	void disable();
	GLuint mFboId;
	GLint mPreFrameBuffer{ 0 };
	int mWidth; //render to tex ,tex width
	int mHeight;//render to tex ,tex height
	bool mbIsAttach;
	float mClearColor[4]{0.0f,0.0f,0.0f,0.0f};
};

#endif /* GRAPHICSFBO_H_ */
