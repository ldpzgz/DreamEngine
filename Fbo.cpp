/*
 * Fbo.cpp
 *
 *  Created on: 2015-9-29
 *      Author: ldp
 */

//#include "StdAfx.h"
#include "Fbo.h"
#include "Log.h"

extern void checkglerror();
Fbo::Fbo()
{
	// TODO Auto-generated constructor stub
}

Fbo::~Fbo()
{
	// TODO Auto-generated destructor stub
	if (mRbo != 0) {
		glDeleteRenderbuffers(1, &mRbo);
		mRbo = 0;
	}
	deleteFbo();
}

void Fbo::enable()
{
	if(mFboId==0)
	{
		glGenFramebuffers(1,&mFboId);
	}
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mPreFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER,mFboId);
}

void Fbo::detachColorTextureMS(unsigned int attachment_n) {
	enable();
	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0 + attachment_n,
		GL_TEXTURE_2D_MULTISAMPLE,
		0,
		0);
	mAttachments.erase(mAttachments.begin() + attachment_n);
	disable();
}

void Fbo::detachColorTexture(unsigned int attachment_n, GLint level) {
	enable();
	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0 + attachment_n,
		GL_TEXTURE_2D,
		0,
		level);

	mAttachments.erase(mAttachments.begin()+attachment_n);
	disable();
}

void Fbo::detachDepthTextureMS() {
	enable();
	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D_MULTISAMPLE,
		0,
		0);
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

bool Fbo::attachDepthRbo(int width, int height) {
	if (mWidth != 0 && mWidth != width) {
		LOGE("ERROR to attach depth rbo,the width error");
		return false;
	}
	else {
		mWidth = width;
		mHeight = height;
	}
	if (mRbo != 0) {
		glDeleteRenderbuffers(1, &mRbo);
	}
	glGenRenderbuffers(1, &mRbo);
	glBindRenderbuffer(GL_RENDERBUFFER, mRbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
	enable();
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, mRbo);
	bool ret = checkFrameBuffer();
	disable();
	return ret;
}

bool Fbo::attachColorRbo(int attachment_n, int width, int height) {
	if (mRbo == 0 || mWidth != width || mHeight != height) {
		if (mRbo != 0) {
			glDeleteRenderbuffers(1, &mRbo);
			mRbo = 0;
		}
		glGenRenderbuffers(1, &mRbo);
		if (mRbo == 0) {
			LOGE("ERROR to create rbo");
			return false;
		}
	}
	mWidth = width;
	mHeight = height;
	glBindRenderbuffer(GL_RENDERBUFFER, mRbo);
	GLint maxRenderbufferSize = 0;
	glGetIntegerv(GL_MAX_RENDERBUFFER_SIZE, &maxRenderbufferSize);
	LOGD("MAX rbo sample size is %d", maxRenderbufferSize);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_RGBA8, width, height);
	enable();
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment_n, GL_RENDERBUFFER, mRbo);

	GLint sampleBuf = 0;
	glGetIntegerv(GL_SAMPLE_BUFFERS, &sampleBuf);

	bool bret = checkFrameBuffer();
	if (bret) {
		unsigned int s = static_cast<unsigned int>(mAttachments.size());
		mAttachments.emplace_back(GL_COLOR_ATTACHMENT0 + s);
	}
	disable();
	return bret;
}

void Fbo::detachDepthRbo() {
	enable();
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, 0);
	if (mRbo > 0) {
		glDeleteRenderbuffers(1, &mRbo);
		mRbo = 0;
	}
	mbEnableDepthTest = false;
	disable();
}

void Fbo::detachColorRbo(int attachment_n) {
	enable();
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + attachment_n, GL_RENDERBUFFER, 0);
	if (mRbo > 0) {
		glDeleteRenderbuffers(1, &mRbo);
		mRbo = 0;
	}
	mAttachments.erase(mAttachments.begin() + attachment_n);
	disable();
}

bool Fbo::checkFrameBuffer() {
	bool ret = true;
	GLenum error = glCheckFramebufferStatus(GL_FRAMEBUFFER);

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
	return ret;
}

bool Fbo::attachColorTextureMS(const std::shared_ptr<Texture>& texture, int attachment_n) {
	bool ret = false;
	if (!texture)
	{
		LOGE("ERROR Fbo::attachColorTextureMS texture == nullptr");
		return false;
	}
	else if(texture->getTexTarget() != GL_TEXTURE_2D_MULTISAMPLE){
		LOGE("ERROR Fbo::attachColorTextureMS textureTarget is not GL_TEXTURE_2D_MULTISAMPLE");
		return false;
	}
	enable();

	mWidth = texture->getWidth();
	mHeight = texture->getHeight();
	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0 + attachment_n,
		GL_TEXTURE_2D_MULTISAMPLE,
		texture->getId(),
		0);

	ret = checkFrameBuffer();
	if (ret) {
		unsigned int s = static_cast<unsigned int>(mAttachments.size());
		mAttachments.emplace_back(GL_COLOR_ATTACHMENT0 + s);
	}
	disable();
	return ret;
}

bool Fbo::replaceColorTexture(const std::shared_ptr<Texture>& texture, int attachment_n, int cubicFace, GLint level) {
	bool ret = false;
	if (!texture || texture->getWidth() != mWidth || texture->getHeight() != mHeight)
	{
		LOGD("Fbo::replaceColorTexture texture has different size with pre texture");
		return false;
	}
	enable();
	
	auto texTarget = texture->getTexTarget();
	if (texTarget == GL_TEXTURE_CUBE_MAP) {
		texTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubicFace;
	}
	else if (texTarget != GL_TEXTURE_2D) {
		LOGE("ERROR fbo can only atach texture_2d or cubic map face");
		return false;
	}
	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_COLOR_ATTACHMENT0 + attachment_n,
		texTarget,
		texture->getId(),
		level);

	//ret = checkFrameBuffer();
	disable();
	return ret;
}

bool Fbo::attachColorTexture(const std::shared_ptr<Texture>& texture, int attachment_n, int cubicFace,GLint level)
{
	bool ret = false;
	if (!texture)
	{
		LOGD("Fbo::attachColorTexture texture == nullptr");
		return false;
	}
	enable();

	mWidth = texture->getWidth();
	mHeight = texture->getHeight();
	auto texTarget = texture->getTexTarget();
	if (texTarget == GL_TEXTURE_CUBE_MAP) {
		texTarget = GL_TEXTURE_CUBE_MAP_POSITIVE_X + cubicFace;
	}
	else if (texTarget != GL_TEXTURE_2D) {
		LOGE("ERROR fbo can only atach texture_2d or cubic map face");
		return false;
	}
	glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0 + attachment_n,
			texTarget,
			texture->getId(),
			level);

	ret = checkFrameBuffer();
	if (ret) {
		unsigned int s = static_cast<unsigned int>(mAttachments.size());
		mAttachments.emplace_back(GL_COLOR_ATTACHMENT0 + s);
	}
	disable();
	return ret;
}

bool Fbo::attachDepthTextureMS(const std::shared_ptr<Texture>& texture) {
	if (!texture)
	{
		LOGE("ERROR Fbo::attachDepthTexture texture == 0");
		return false;
	} 
	else if(texture->getTexTarget() != GL_TEXTURE_2D_MULTISAMPLE){
		
		LOGE("ERROR Fbo::attachDepthTextureMS textureTarget is not GL_TEXTURE_2D_MULTISAMPLE");
		return false;
	}
	glFramebufferTexture2D(
		GL_FRAMEBUFFER,
		GL_DEPTH_ATTACHMENT,
		GL_TEXTURE_2D_MULTISAMPLE,
		texture->getId(),
		0);
	bool bret = checkFrameBuffer();
	if (bret) {
		mbEnableDepthTest = true;
	}
	return bret;
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
	bool bret = checkFrameBuffer();
	if (bret) {
		mbEnableDepthTest = true;
	}
	return bret;
}

void Fbo::disable()
{
	glBindFramebuffer(GL_FRAMEBUFFER, mPreFrameBuffer);
}

void Fbo::deleteFbo()
{
	if(mFboId!=0)
	{
		glDeleteFramebuffers(1,&mFboId);
		mFboId = 0;
	}
}

void Fbo::render(std::function<void()> func) {
	float preClearColor[4]{ 0.0f,0.0f,0.0f,0.0f };
	float preClearDepth = 1.0f;
	int preDepthFunc = GL_LESS;
	GLboolean bpreDepthTest;
	GLboolean bpreBlend;
	int presFactorRgb = GL_ONE;
	int predFactorRgb = GL_ONE;
	int presFactorAlpha = GL_ONE;
	int predFactorAlpha = GL_ONE;
	int premodelRgb, premodelAlpha;
	int preViewport[4];
	//这两个必定会改动
	glGetIntegerv(GL_VIEWPORT, preViewport);
	glGetFloatv(GL_COLOR_CLEAR_VALUE, preClearColor);
	
	if (mbEnableBlend) {
		glGetBooleanv(GL_BLEND, &bpreBlend);
		glGetIntegerv(GL_BLEND_SRC_RGB, &presFactorRgb);
		glGetIntegerv(GL_BLEND_DST_RGB, &predFactorRgb);
		glGetIntegerv(GL_BLEND_SRC_ALPHA, &presFactorAlpha);
		glGetIntegerv(GL_BLEND_DST_ALPHA, &predFactorAlpha);
		glGetIntegerv(GL_BLEND_EQUATION_RGB, &premodelRgb);
		glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &premodelAlpha);
	}
	if (mbEnableDepthTest) {
		glGetBooleanv(GL_DEPTH_TEST, &bpreDepthTest);
		glGetIntegerv(GL_DEPTH_FUNC, &preDepthFunc);
		glGetFloatv(GL_DEPTH_CLEAR_VALUE, &preClearDepth);
	}
	else {
		glGetBooleanv(GL_DEPTH_TEST, &bpreDepthTest);
	}
	GLboolean preCullFace = false;
	int preCullFaceModel = GL_BACK;
	int preFrontFace = GL_CCW;
	glGetBooleanv(GL_CULL_FACE, &preCullFace);
	glGetIntegerv(GL_CULL_FACE_MODE, &preCullFaceModel);
	glGetIntegerv(GL_FRONT_FACE, &preFrontFace);
	if (mbCullFace) {
		glEnable(GL_CULL_FACE);
		glCullFace(mCullFaceMode);
		glFrontFace(mFrontFace);
	}
	else {
		glDisable(GL_CULL_FACE);
	}

	enable();

	glViewport(0, 0, mWidth, mHeight);
	if (mbClearColor) {
		glClearColor(mClearColor[0], mClearColor[1], mClearColor[2], mClearColor[3]);
		glClear(GL_COLOR_BUFFER_BIT);//GL_DEPTH_BUFFER_BIT
		if (mbClearColorOnlyOnce) {
			mbClearColor = false;
		}
	}
	
	if (mbEnableDepthTest) {
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		glClearDepthf(1.0f);
		glClear(GL_DEPTH_BUFFER_BIT);
	}
	else {
		glDisable(GL_DEPTH_TEST);
	}

	if (mbEnableBlend) {
		glEnable(GL_BLEND);
		glBlendFuncSeparate(msFactorRgb, mdFactorRgb, msFactorAlpha, mdFactorAlpha);
		glBlendEquationSeparate(mModelRgb, mModelAlpha);
	}

	glDrawBuffers(static_cast<GLsizei>(mAttachments.size()), mAttachments.data());

	if (func) {
		func();
	}

	disable();
	//恢复之前的状态
	glViewport(preViewport[0], preViewport[1], preViewport[2], preViewport[3]);
	glClearColor(preClearColor[0], preClearColor[1], preClearColor[2], preClearColor[3]);
	if (bpreDepthTest) {
		glEnable(GL_DEPTH_TEST);
	}
	else {
		glDisable(GL_DEPTH_TEST);
	}
	if (mbEnableDepthTest) {
		glClearDepthf(preClearDepth);
		glDepthFunc(preDepthFunc);
	}
	

	if (mbEnableBlend) {
		if (bpreBlend) {
			glEnable(GL_BLEND);
		}
		else {
			glDisable(GL_BLEND);
		}
		glBlendFuncSeparate(presFactorRgb, predFactorRgb, presFactorAlpha, predFactorAlpha);
		glBlendEquationSeparate(premodelRgb, premodelAlpha);
	}

	if (preCullFace) {
		glEnable(GL_CULL_FACE);
		glCullFace(preCullFaceModel);
		glFrontFace(preFrontFace);
	}
	else {
		glDisable(GL_CULL_FACE);
	}
}

void Fbo::setClearColorValue(float r, float g, float b, float a)
{
	mClearColor[0] = r;
	mClearColor[1] = g;
	mClearColor[2] = b;
	mClearColor[3] = a;
}

void Fbo::setBlend(bool b) {
	mbEnableBlend = b;
}
void Fbo::setBlendValue(int sFactorRgb, int dFactorRgb, int sFactorAlpha, int dFactorAlpha, int modelRgb, int modelAlpha)
{
	msFactorRgb = sFactorRgb;
	mdFactorRgb = dFactorRgb;
	msFactorAlpha = sFactorAlpha;
	mdFactorAlpha = dFactorAlpha;
	mModelRgb = modelRgb;
	mModelAlpha = modelAlpha;
}

bool Fbo::blitDepthBufToWin() {
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, mFboId);
	glBlitFramebuffer(0, 0,mWidth, mHeight,0, 0,mWidth, mHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	checkglerror();
	return true;
}

bool Fbo::blitFbo(const Fbo& src, const Rect<int>& srcRect, 
	const Fbo& dst, const Rect<int>& dstRect) {
	// set the default framebuffer for writing
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
		dst.mFboId);
	// set the fbo with four color attachments for reading
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src.mFboId);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(srcRect.x, srcRect.y,
		srcRect.x+srcRect.width, srcRect.y+srcRect.height,
		dstRect.x, dstRect.y,
		dstRect.x+dstRect.width, dstRect.y+dstRect.height,
		GL_COLOR_BUFFER_BIT, GL_NEAREST);
	checkglerror();
	return true;
}

bool Fbo::blitFbo(const Fbo& src, const Fbo& dst) {
	// set the default framebuffer for writing
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER,
		dst.mFboId);
	GLenum b[1] = {GL_COLOR_ATTACHMENT0};
	glDrawBuffers(1, b);
	// set the fbo with four color attachments for reading
	glBindFramebuffer(GL_READ_FRAMEBUFFER, src.mFboId);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glBlitFramebuffer(0, 0,
		src.mWidth, src.mHeight,
		0, 0,
		dst.mWidth, dst.mHeight,
		GL_COLOR_BUFFER_BIT, GL_NEAREST);
	checkglerror();
	return true;
}
