#include "Texture.h"
#include "Log.h"
#include <vector>
#include "Utils.h"
#include "Sampler.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include <filesystem>

namespace fs = std::filesystem;

Texture::Texture()
{
}
Texture::~Texture()
{
	unload();
}


int Texture::getHeight() {
	return mHeight;
}

int Texture::getWidth() {
	return mWidth;
}
/*
* 调用这个函数生成的texture，不需要设置sampler
*/
bool Texture::createMStexture(int width,int height,int samples,unsigned int internalformat) {
	mInternalFormat = internalformat;
	mWidth = width;
	mHeight = height;
	mNumOfSamples = samples;
	mTarget = GL_TEXTURE_2D_MULTISAMPLE;
	GLint maxSamples = 0;
	checkglerror();
	glGetInternalformativ(GL_TEXTURE_2D_MULTISAMPLE, GL_RGBA8, GL_SAMPLES, 1, &maxSamples);
	checkglerror();
	if (mNumOfSamples > maxSamples) {
		mNumOfSamples = maxSamples;
	}
	if (mInternalFormat == GL_RGB8) {
		mFormat = GL_RGB;
		glTexParameteri(mTarget, GL_TEXTURE_SWIZZLE_A, GL_ONE);
	}
	else if (mInternalFormat == GL_RGBA8) {
		mFormat = GL_RGBA;
	}
	unload();
	glGenTextures(1, &mTextureId);
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(mTarget, mTextureId);
	checkglerror();
	// 多重采样纹理，opengl3.3如果设置这些属性会报错，"无效枚举值"(es3.1不会报错），
	// 不需要设置这些参数
	//glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(mTarget, GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	//glTexParameteri(mTarget, GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	
	glTexStorage2DMultisample(mTarget,
		mNumOfSamples,
		mInternalFormat,
		mWidth,
		mHeight,
		GL_FALSE);
	checkglerror();
	return true;
}

bool Texture::createCubicMap(int width, int height, GLint internalFormat, GLenum format, GLenum type, bool autoMipmap) {
	mTarget = GL_TEXTURE_CUBE_MAP;
	mInternalFormat = internalFormat;
	mFormat = format;
	mType = type;
	mWidth = width;
	mHeight = height;
	unload();
	glGenTextures(1, &mTextureId);
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(mTarget, mTextureId);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, mInternalFormat, mWidth, mHeight, 0, mFormat, mType, nullptr);
		/*if (autoMipmap && mInternalFormat==GL_RGB16F) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 1, mInternalFormat, mWidth / 2, mHeight / 2, 0, mFormat, mType, nullptr);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 2, mInternalFormat, mWidth / 4, mHeight / 4, 0, mFormat, mType, nullptr);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 3, mInternalFormat, mWidth / 8, mHeight / 8, 0, mFormat, mType, nullptr);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 4, mInternalFormat, mWidth / 16, mHeight / 16, 0, mFormat, mType, nullptr);
		}*/
		checkglerror();
	}
	if (autoMipmap) {
		glGenerateMipmap(mTarget);
		if (!mpSampler) {//有这个就有cubemap
			mpSampler = Sampler::getSampler(GL_LINEAR_MIPMAP_LINEAR,GL_LINEAR);
		}
	}
	else {
		if (!mpSampler) {
			mpSampler = Sampler::getSampler(GL_LINEAR,GL_LINEAR);
		}
	}
	return true;
}

bool Texture::create2DMap(int width,int height,const unsigned char* pdata, GLint internalFormat,GLint format,GLenum type, bool autoMipmap,int aligment)
{
	mFormat = format;
	mInternalFormat = internalFormat;
	mWidth = width;
	mHeight = height;
	mTarget = GL_TEXTURE_2D;
	mType = type;
	unload();
	glGenTextures(1, &mTextureId);
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(mTarget, mTextureId);
	// Loads image data into OpenGL.
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, aligment);
	glTexImage2D(mTarget, 0, mInternalFormat, mWidth, mHeight, 0,
		mFormat, mType, pdata);
	if (autoMipmap) {
		if(pdata != nullptr)
			glGenerateMipmap(mTarget);
		if (!mpSampler) {
			mpSampler = Sampler::getSampler(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		}
	}
	else {
		if (!mpSampler) {
			mpSampler = Sampler::getSampler(GL_LINEAR, GL_LINEAR);
		}
	}
	
	checkglerror();
	return true;
}
/*
* 浮点纹理默认没有生成mipmap
*/
bool Texture::loadHdrFile(const std::string& path) {
	//stbi_set_flip_vertically_on_load(true);
	int nrComponents;
	float* data = stbi_loadf(path.c_str(), &mWidth, &mHeight, &nrComponents, 0);
	//stbi_set_flip_vertically_on_load(false);
	/*float minest = 11111110.0f;
	float maxest = 0.0f;*/
	if (data)
	{
		/*for (int i = 0; i < mHeight; ++i) {
			for (int j = 0; j < nrComponents * mWidth; ++j) {
				float temp = data[i * nrComponents * mWidth + j];
				if (minest > temp) {
					minest = temp;
				}
				if (maxest < temp) {
					maxest = temp;
				}
			}
		}
		std::cout << "hdr image min max is :" << minest << "," << maxest << std::endl;*/
		mTarget = GL_TEXTURE_2D;
		if (nrComponents == 3) {
			mInternalFormat = GL_RGB16F;
			mFormat = GL_RGB;
		}
		else if (nrComponents == 4) {
			mInternalFormat = GL_RGBA16F;
			mFormat = GL_RGBA;
		}
		else {
			LOGE("ERROR to load hdr image file,the components is unknow");
			stbi_image_free(data);
			return false;
		}
		mType = GL_FLOAT;
		unload();
		glGenTextures(1, &mTextureId);
		//glActiveTexture(GL_TEXTURE0);
		glBindTexture(mTarget, mTextureId);
		glTexImage2D(GL_TEXTURE_2D, 0, mInternalFormat, mWidth, mHeight, 0, mFormat, mType, data);
		stbi_image_free(data);
		if (!mpSampler) {
			mpSampler = Sampler::getSampler(GL_LINEAR, GL_LINEAR);
		}
		checkglerror();
		return true;
	}
	else
	{
		LOGE("loadHdrFile failed at path: %s", path.c_str());
		return false;
	}
}
/*
* 默认生成mipmap
*/
bool Texture::loadFromFile(const std::string& path) {
	int nrChannels;
	//stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path.c_str(), &mWidth, &mHeight, &nrChannels, 0);
	if (data)
	{
		mTarget = GL_TEXTURE_2D;
		unload();
		glGenTextures(1, &mTextureId);
		//glActiveTexture(GL_TEXTURE0);
		glBindTexture(mTarget, mTextureId);
		glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		if (nrChannels == 3) {
			mFormat = GL_RGB;
		}
		else if (nrChannels == 4) {
			mFormat = GL_RGBA;
		}
		else if (nrChannels == 1) {
			mFormat = GL_LUMINANCE;//this will swizzle rgba to red
		}
		else {
			LOGE("load texture %s,unknow channels: %d", path.c_str(), nrChannels);
		}
		glTexImage2D(mTarget,0, mFormat, mWidth, mHeight, 0, mFormat, mType, data);
		glGenerateMipmap(mTarget);
		if (!mpSampler) {
			mpSampler = Sampler::getSampler(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR,GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
		}
		stbi_image_free(data);
		return true;
	}
	else
	{
		LOGE("load texture from file: %s", path.c_str());
		return false;
	}
}

void Texture::genMipmap() {
	if (mTextureId > 0) {
		glBindTexture(mTarget, mTextureId);
		glGenerateMipmap(mTarget);
		checkglerror();
	}
	else {
		LOGD("texture has not initialized when genMipmap");
	}
}
/*
* 默认生成mipmap
*/
bool Texture::loadCubemap(const std::string& path) {
	unload();
	mTarget = GL_TEXTURE_CUBE_MAP;
	glGenTextures(1, &mTextureId);
	//glActiveTexture(GL_TEXTURE0);
	glBindTexture(mTarget, mTextureId);

	std::vector<std::string> filename{ "/right.jpg","/left.jpg","/top.jpg","/bottom.jpg","/front.jpg","/back.jpg" };
	//stbi_set_flip_vertically_on_load(true);
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
				mFormat = GL_LUMINANCE;//this will swizzle rgba to red
			}
			else {
				LOGE("Cubemap texture %s,unknow channels: %d", filePath.c_str(), nrChannels);
			}
			glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, mFormat, mWidth, mHeight, 0, mFormat, mType, data
			);
			stbi_image_free(data);
		}
		else
		{
			LOGE( "load Cubemap failed path: %s", filePath.c_str());
			stbi_image_free(data);
		}
		++i;
	}
	glGenerateMipmap(mTarget);
	if (!mpSampler) {
		mpSampler = Sampler::getSampler(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
	}
	//int align = 0;
	//glGetIntegerv(GL_UNPACK_ALIGNMENT, &align);//默认是4，the alignment requirements for the start of each pixel row in memory

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
}

void Texture::active(GLint textPoint)
{
	glActiveTexture(textPoint);
	glBindTexture(mTarget, mTextureId);
	if (mpSampler) {
		mpSampler->bindTex(textPoint, &mBorderColor[0]);
	}
}

void Texture::deActive(GLint texPoint) {
	if (mpSampler) {
		glActiveTexture(texPoint);
		mpSampler->unBind(texPoint);
	}
}

void Texture::update(int xoffset,int yoffset,int width,int height,void* data,
	int face,int aligment,int mipmapLevel)
{
	if (mTarget == GL_TEXTURE_2D_MULTISAMPLE || mTarget == GL_TEXTURE_2D_MULTISAMPLE_ARRAY) {
		LOGE("ERROR 2D_MULTISAMPLE cannot update");
		return;
	}
	
	glBindTexture(mTarget, mTextureId);
	glPixelStorei(GL_UNPACK_ALIGNMENT, aligment);
	unsigned int target = mTarget;
	if (mTarget == GL_TEXTURE_CUBE_MAP) {
		target = GL_TEXTURE_CUBE_MAP_POSITIVE_X + face;
	}
	glTexSubImage2D(target, mipmapLevel,xoffset,yoffset,width,height,mFormat, mType,data);
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

std::shared_ptr<Texture> Texture::loadImageFromFile(const std::string& path, const std::string& name) {
	auto pTex = std::make_shared<Texture>(name);
	if (pTex) {
		bool b = false;
		if (fs::path(path).extension() == ".hdr") {
			b = pTex->loadHdrFile(path);
			/*if (b) {
				pTex->convertHdrToCubicmap();
			}*/
		}
		else {
			b = pTex->loadFromFile(path);
		}
		if (!b) {
			pTex.reset();
		}
	}
	return pTex;
}

void Texture::saveToFile(const std::string& path) {

}