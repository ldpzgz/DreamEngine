//#include "StdAfx.h"
#include "Texture.h"
#include "Log.h"
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
TextureP gpTextureNothing;
extern void checkglerror();

Texture::Texture()
{

}
Texture::~Texture()
{
	unload();
}


int32_t Texture::getHeight() {
	return mHeight;
}

int32_t Texture::getWidth() {
	return mWidth;
}


//int Texture::load(const char* mPath,bool autoMipmap) {
//	unsigned char* lImageBuffer = loadImage(mPath);
//	if (lImageBuffer == NULL)
//	{
//		return STATUS_KO;
//	}
//
//	// Creates a new OpenGL texture.
//	//GLenum lErrorResult;
//	glGenTextures(1, &mTextureId);
//	glBindTexture(GL_TEXTURE_2D, mTextureId);
//	// Set-up texture properties.
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
//		GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
//		GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
//		GL_CLAMP_TO_EDGE);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
//		GL_CLAMP_TO_EDGE);
//	// Loads image data into OpenGL.
//	int internalFormat = mFormat;//png load this value is equel to mFormat;
//	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, mWidth, mHeight, 0,
//		mFormat, GL_UNSIGNED_BYTE, lImageBuffer);
//
//	if(autoMipmap)
//	{
//		glGenerateMipmap(GL_TEXTURE_2D);
//	}
//	if(lImageBuffer!=0)
//	{
//		delete[] lImageBuffer;
//		lImageBuffer = 0;
//	}
//	
//	if (glGetError() != GL_NO_ERROR)
//	{
//		LOGE("Error loading texture into OpenGL.");
//		unload();
//		return STATUS_KO;
//	}
//	return STATUS_OK;
//}

bool Texture::createMStexture(int width,int height,int samples,unsigned int internalformat) {
	mInternalFormat = internalformat;
	mWidth = width;
	mHeight = height;
	mNumOfSamples = samples;
	mTarget = GL_TEXTURE_2D_MULTISAMPLE;
	GLint maxSamples = 0;
	glGetInternalformativ(GL_TEXTURE_2D_MULTISAMPLE, GL_RGBA8, GL_SAMPLES, 1, &maxSamples);
	if (mNumOfSamples > maxSamples) {
		mNumOfSamples = maxSamples;
	}
	if (mInternalFormat == GL_RGB8) {
		mFormat = GL_RGB;
	}
	else if (mInternalFormat == GL_RGBA8) {
		mFormat = GL_RGBA;
	}

	glGenTextures(1, &mTextureId);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(mTarget, mTextureId);
	// Set-up texture properties.
	glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(mTarget, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(mTarget, GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

	glTexStorage2DMultisample(mTarget,
		mNumOfSamples,
		mInternalFormat,
		mWidth,
		mHeight,
		GL_FALSE);
	return true;
}

bool Texture::load(int width,int height,unsigned char* pdata,GLint format,GLenum type, int aligment,bool autoMipmap)
{
	mFormat = format;
	mWidth = width;
	mHeight = height;
	mTarget = GL_TEXTURE_2D;
	glGenTextures(1, &mTextureId);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(mTarget, mTextureId);//mTarget是GL_TEXTURE_CUBE_MAP的时候要注意，
	// Set-up texture properties.
	glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(mTarget, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(mTarget, GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	// Loads image data into OpenGL.
	
	int internalformat = mFormat;//only 1,2,3,4

	//int align = 0;
	//glGetIntegerv(GL_UNPACK_ALIGNMENT, &align);//默认是4，the alignment requirements for the start of each pixel row in memory

	glPixelStorei(GL_UNPACK_ALIGNMENT, aligment);
	//这个函数后面三个参数指定了纹理数据在内存中的组织方式。
	//internalformat是指opengl在显存中创建的这张纹理，是什么格式的，这个格式必须与内存中的format匹配，
	// 与mForamt，type是一对参数
	//也就是说这个函数不会进行数据格式转换。
	//第二个参数是mipmap等级。0最大，第六个参数是指边框的宽度，必须为0。
	glTexImage2D(mTarget, 0, internalformat, mWidth, mHeight, 0,
		mFormat, type, pdata);

	checkglerror();

	if(autoMipmap)
	{
		glGenerateMipmap(mTarget);
	}

	if (glGetError() != GL_NO_ERROR)
	{
		LOGE("Error loading texture into OpenGL.");
		unload();
		return false;
	}
	return true;
}

bool Texture::loadFromeFile(const std::string& path) {
	mTarget = GL_TEXTURE_2D;
	glGenTextures(1, &mTextureId);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(mTarget, mTextureId);
	glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(mTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(mTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	int nrChannels;
	unsigned char* data = stbi_load(path.c_str(), &mWidth, &mHeight, &nrChannels, 0);
	if (data)
	{
		if (nrChannels == 3) {
			mFormat = GL_RGB;
		}
		else if (nrChannels == 4) {
			mFormat = GL_RGBA;
		}
		else if (nrChannels == 1) {
			mFormat = GL_LUMINANCE;
		}
		else {
			LOGE("Cubemap texture %s,unknow channels: d", path.c_str(), nrChannels);
		}
		glTexImage2D(mTarget,0, mFormat, mWidth, mHeight, 0, mFormat, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);
		return true;
	}
	else
	{
		LOGE("Cubemap tex failed to load at path: %s", path.c_str());
		stbi_image_free(data);
		unload();
		return false;
	}

}

bool Texture::loadCubemap(const std::string& path) {
	mTarget = GL_TEXTURE_CUBE_MAP;
	glGenTextures(1, &mTextureId);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(mTarget, mTextureId);

	std::vector<std::string> filename{ "/right.jpg","/left.jpg","/top.jpg","/bottom.jpg","/front.jpg","/back.jpg" };

	int nrChannels;
	int i = 0;
	for (auto& afile:filename)
	{
		auto filePath = path + afile;
		unsigned char* data = stbi_load(filePath.c_str(), &mWidth, &mHeight, &nrChannels, 0);
		if (data)
		{
			if (nrChannels == 3) {
				mFormat = GL_RGB;
			}
			else if (nrChannels == 4) {
				mFormat = GL_RGBA;
			}
			else if (nrChannels == 1) {
				mFormat = GL_LUMINANCE;
			}
			else {
				LOGE("Cubemap texture %s,unknow channels: d", filePath.c_str(), nrChannels);
			}
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, mFormat, mWidth, mHeight, 0, mFormat, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			LOGE( "Cubemap tex failed to load at path: %s", filePath.c_str());
			stbi_image_free(data);
		}
		++i;
	}

	//int align = 0;
	//glGetIntegerv(GL_UNPACK_ALIGNMENT, &align);//默认是4，the alignment requirements for the start of each pixel row in memory

	// Set-up texture properties.
	glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(mTarget, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameteri(mTarget, GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexParameteri(mTarget, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	if (glGetError() != GL_NO_ERROR)
	{
		LOGE("Error loading texture into OpenGL.");
		unload();
		return false;
	}
	return true;
}

void Texture::unload()
{
	if (mTextureId != 0)
	{
		glDeleteTextures(1, &mTextureId);
		mTextureId = 0;
	}
	mWidth = 0;
	mHeight = 0;
	mFormat = GL_RGB;
}

void Texture::active(GLint textPoint)
{
	glActiveTexture(textPoint);
	glBindTexture(mTarget, mTextureId);
	//checkglerror();
}

void Texture::update(int xoffset,int yoffset,int width,int height,void* data,
	int aligment,int mipmapLevel)
{
	if (mTarget == GL_TEXTURE_2D_MULTISAMPLE || mTarget == GL_TEXTURE_2D_MULTISAMPLE_ARRAY) {
		LOGE("ERROR 2D_MULTISAMPLE cannot update");
		return;
	}
	glBindTexture(mTarget, mTextureId);
	glPixelStorei(GL_UNPACK_ALIGNMENT, aligment);
	glTexSubImage2D(mTarget, mipmapLevel,xoffset,yoffset,width,height,mFormat,GL_UNSIGNED_BYTE,data);
	checkglerror();
}


int Texture::maxTexunit()
{
	int num=0;
	//glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,&num);//2.0
	glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &num);//3.0
	return num;
}
int Texture::numOfCompressFormat()
{
	int num=0;
	glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS,&num);
	return num;
}
void Texture::getCompressFormat(GLint* formats)
{
	glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS,formats);
}

std::unique_ptr<Texture> Texture::loadImageFromFile(const std::string& path) {
	auto pTex = std::make_unique<Texture>();
	if (pTex) {
		pTex->loadFromeFile(path);
	}
	return pTex;
}
