﻿/*
 * GraphicsFbo.cpp
 *
 *  Created on: 2015-9-29
 *      Author: ldp
 */

//#include "StdAfx.h"
#include "GraphicsFbo.h"
#include "Log.h"


GraphicsFbo::GraphicsFbo():mFboId(0),
	mWidth(0),
	mHeight(0),
	mbIsAttach(false)
{
	// TODO Auto-generated constructor stub
	mClearColor[0] = mClearColor[1] = mClearColor[2] = mClearColor[3] = 0.0f;
}

GraphicsFbo::~GraphicsFbo()
{
	// TODO Auto-generated destructor stub
	deleteFbo();
}

void GraphicsFbo::enable()
{
	if(mFboId==0)
	{
		glGenFramebuffers(1,&mFboId);
	}
	glBindFramebuffer(GL_FRAMEBUFFER,mFboId);
}
bool GraphicsFbo::attachColorTexture(GraphicsTexture* texture,GLint level)
{
	bool ret = false;
	if (texture == 0)
	{
		LOGD("GraphicsFbo::attachColorTexture texture == 0");
		return false;
	}

	enable();

	if(mbIsAttach)
	{
		glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D,
			0,
			level);
		mbIsAttach = false;
	}

	mWidth = texture->getWidth();
	mHeight = texture->getHeight();
	glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D,
			texture->getId(),
			level);

	GLenum error = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	do
	{
		if (error == GL_FRAMEBUFFER_COMPLETE)
		{
			LOGD("fbo completed \n");
			mbIsAttach = true;
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
bool GraphicsFbo::attachDepthTexture(GraphicsTexture* texture,GLint level)
{
	if (texture == 0)
	{
		LOGD("GraphicsFbo::attachDepthTexture texture == 0");
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
void GraphicsFbo::disable()
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void GraphicsFbo::deleteFbo()
{
	if(mFboId!=0)
	{
		glDeleteFramebuffers(1,&mFboId);
		mFboId = 0;
	}
}

void GraphicsFbo::startRender()
{
	enable();
	glViewport(0, 0, mWidth, mHeight);
	glClearColor(mClearColor[0], mClearColor[1], mClearColor[2], mClearColor[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
void GraphicsFbo::endRender()
{
	disable();
}

void GraphicsFbo::setClearColor(float r, float g, float b, float a)
{
	mClearColor[0] = r;
	mClearColor[1] = g;
	mClearColor[2] = b;
	mClearColor[3] = a;
}
