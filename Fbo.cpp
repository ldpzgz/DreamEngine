/*
 * Fbo.cpp
 *
 *  Created on: 2015-9-29
 *      Author: ldp
 */

//#include "StdAfx.h"
#include "Fbo.h"
#include "Log.h"


Fbo::Fbo():
	mWidth(0),
	mHeight(0)
{
	// TODO Auto-generated constructor stub
}

Fbo::~Fbo()
{
	// TODO Auto-generated destructor stub
	deleteFbo();
}

void Fbo::enable()
{
	if(mFboId==0)
	{
		glGenFramebuffers(1,&mFboId);
	}
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mPreFrameBuffer);
	glGetFloatv(GL_COLOR_CLEAR_VALUE, mPreClearColor);
	glGetBooleanv(GL_DEPTH_TEST, &mPrebDepthTest);
	glBindFramebuffer(GL_FRAMEBUFFER,mFboId);
}

void Fbo::detachColorTexture(int attachment_n, GLint level) {
	enable();
	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0 + attachment_n,
		GL_TEXTURE_2D,
		0,
		level);
	disable();
}

void Fbo::detachDepthTexture(GLint level) {
	enable();
	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D,
		0,
		level);
	disable();
}

bool Fbo::attachColorTexture(const std::shared_ptr<Texture>& texture, int attachment_n, GLint level)
{
	bool ret = false;
	if (!texture)
	{
		LOGD("Fbo::attachColorTexture texture == nullptr");
		return false;
	}

	enable();

	/*if(mbIsAttach)
	{
		glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0 + attachment_n,
			GL_TEXTURE_2D,
			0,
			level);
		mbIsAttach = false;
	}*/

	mWidth = texture->getWidth();
	mHeight = texture->getHeight();
	glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0 + attachment_n,
			GL_TEXTURE_2D,
			texture->getId(),
			level);

	GLenum error = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	do
	{
		if (error == GL_FRAMEBUFFER_COMPLETE)
		{
			LOGD("fbo completed \n");
			//mbIsAttach = true;
			ret = true;
		}
		else if (error == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
		{
			LOGD("fbo incomplete attachment\n");
			ret = false;
		}
		else if (error == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
		{
			LOGD("fbo missing attachment\n");
			ret = false;
		}
		//else if(error == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS)
		//{
		//	//Log::info("fbo imcomplete dimensions\n");
		//	return false;
		//}

		else if (error == GL_FRAMEBUFFER_UNSUPPORTED)
		{
			LOGD("fbo unsupported\n");
			ret = false;
		}
	} while (false);
	disable();
	return ret;
}
bool Fbo::attachDepthTexture(const std::shared_ptr<Texture>& texture,GLint level)
{
	if (!texture)
	{
		LOGD("Fbo::attachDepthTexture texture == 0");
		return false;
	}
	glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				GL_DEPTH_ATTACHMENT,
				GL_TEXTURE_2D,
				texture->getId(),
				level);
	GLenum error = glCheckFramebufferStatus(GL_FRAMEBUFFER);
	if(error == GL_FRAMEBUFFER_COMPLETE)
	{
		return true;
	}
	else if(error == GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT)
	{
		LOGD("fbo attachDepthTexture incomplete attachment\n");
		return false;
	}
	else if(error == GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT)
	{
		LOGD("fbo attachDepthTexture missing attachment\n");
		return false;
	}
	//else if(error == GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS)
	//{
	//	//Log::info("fbo imcomplete dimensions\n");
	//	return false;
	//}

	else if(error == GL_FRAMEBUFFER_UNSUPPORTED)
	{
		LOGD("fbo attachDepthTexture unsupported\n");
		return false;
	}
	return false;
}
void Fbo::disable()
{
	glBindFramebuffer(GL_FRAMEBUFFER, mPreFrameBuffer);
	glClearColor(mPreClearColor[0], mPreClearColor[1], mPreClearColor[2], mPreClearColor[3]);
	if (mPrebDepthTest) {
		glEnable(GL_DEPTH_TEST);
	}
	else {
		glDisable(GL_DEPTH_TEST);
	}
}

void Fbo::deleteFbo()
{
	if(mFboId!=0)
	{
		glDeleteFramebuffers(1,&mFboId);
		mFboId = 0;
	}
}

void Fbo::startRender()
{
	enable();
	glViewport(0, 0, mWidth, mHeight);
	glClearColor(mClearColor[0], mClearColor[1], mClearColor[2], mClearColor[3]);
	if (mbEnableDepthTest) {
		glEnable(GL_DEPTH_TEST);
	}
	else {
		glDisable(GL_DEPTH_TEST);
	}
	if (mbClearColor) {
		glClear(GL_COLOR_BUFFER_BIT);//GL_DEPTH_BUFFER_BIT
	}
}
void Fbo::endRender()
{
	disable();
}

void Fbo::setClearColorValue(float r, float g, float b, float a)
{
	mClearColor[0] = r;
	mClearColor[1] = g;
	mClearColor[2] = b;
	mClearColor[3] = a;
}
