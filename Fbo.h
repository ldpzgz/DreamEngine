/*
 * GraphicsFbo.h
 *
 *  Created on: 2015-9-29
 *      Author: liudongping
 */
#pragma once

#include<memory>
#include "Texture.h"
#include "Rect.h"
#include <functional>
#include <vector>
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
	void setClearColorValue(float r, float g, float b, float a);
	//设置是否需要clear color，默认会clearcolor，如果下次不需要了，要设置为false
	void setClearColor(bool b,bool once = true) {
		mbClearColor = b;
		mbClearColorOnlyOnce = once;
	}
	void setDepthTest(bool b) {
		mbEnableDepthTest = b;
	}
	void setClearDepth(bool b) {
		mbClearDepth = b;
	}

	//blend选项，在material里面也可以设置，每个物体可以根据自身特点单独设置
	//这里提供全局的blend设置，比如ui渲染，全部ui都是一样的blend配置，避免每个view单独配置
	void setBlend(bool b);
	void setBlendValue(int sFactorRgb, int dFactorRgb, int sFactorAlpha, int dFactorAlpha, int modelRgb, int modelAlpha);
	
	//这里提供cullface的配置，理由同上面的blend
	void setCullFace(bool b,int model = GL_BACK,int frontFace=GL_CCW) {
		mbCullFace = b;
		mCullFaceMode = model;
		mFrontFace = frontFace;
	}

	//GL_COLOR_ATTACHMENT0 + attachment_n
	void detachColorTexture(unsigned int attachment_n=0, GLint level=0);
	void detachColorTextureMS(unsigned int attachment_n = 0);
	void detachDepthTexture(GLint level = 0);
	void detachDepthTextureMS();
	/*
	* 指定fbo渲染到纹理，只能atach到texture2d 或者cubicmap，
	* attachment_n：GL_COLOR_ATTACHMENT0 + attachment_n
	* cubicFace：0，1，2，3，4，5，分别代表cubicmap的x,-x,y,-y,z,-z面
	*/
	bool attachColorTexture(const std::shared_ptr<Texture>& texture,int attachment_n = 0, int cubicFace = 0,GLint level=0);
	bool attachColorTextureMS(const std::shared_ptr<Texture>& texture, int attachment_n = 0);
	bool attachDepthTexture(const std::shared_ptr<Texture>& texture,GLint level=0,bool noColorBuffer = false,int cubicFace=0);
	bool attachDepthTextureMS(const std::shared_ptr<Texture>& texture);

	bool replaceColorTexture(const std::shared_ptr<Texture>& texture, int attachment_n, int cubicFace = 0, GLint level = 0);

	bool attachColorRbo(int attachment_n, int width, int height);
	bool attachDepthRbo(int width, int height);
	void detachColorRbo(int attachment_n = 0);
	void detachDepthRbo();
	
	void render(std::function<void()> func);

	void enable();
	void disable();
	bool blitDepthBufToWin();
	static bool blitFbo(const Fbo& src,const Rect<int>& srcRect, const Fbo& dst, const Rect<int>& dstRect);
	static bool blitFbo(const Fbo& src, const Fbo& dst);
private:
	void deleteFbo();
	bool checkFrameBuffer();
	GLuint mFboId{ 0 };
	GLint mPreFrameBuffer{ 0 };
	
	GLboolean mPrebDepthTest{ true };
	int mWidth{ 0 }; //render to tex ,tex width
	int mHeight{ 0 };//render to tex ,tex height
	bool mbNoColorBuffer{false};
	bool mbEnableDepthTest{ true };
	bool mbClearDepth{ true };
	bool mbClearColor{ true };
	bool mbClearColorOnlyOnce{ false };
	float mClearColor[4]{0.0f,0.0f,0.0f,0.0f};
	GLuint mRbo{ 0 };
	GLboolean mbEnableBlend{ 0 };
	int msFactorRgb{ GL_ONE };
	int mdFactorRgb{ GL_ONE };
	int msFactorAlpha{ GL_ONE };
	int mdFactorAlpha{ GL_ONE };
	int mModelRgb{ GL_FUNC_ADD };
	int	mModelAlpha{ GL_FUNC_ADD };
	GLboolean mbCullFace{ false };
	int mFrontFace{ GL_CCW };
	int mCullFaceMode{ GL_BACK };
	std::vector<unsigned int> mAttachments;
};

