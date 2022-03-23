#include "Texture.h"
#include "Log.h"
#include <vector>
#include "Utils.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

extern void checkglerror();

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
	}
	else if (mInternalFormat == GL_RGBA8) {
		mFormat = GL_RGBA;
	}
	unload();
	glGenTextures(1, &mTextureId);
	glActiveTexture(GL_TEXTURE0);
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
	glActiveTexture(GL_TEXTURE0);
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
		mMinFilter = GL_LINEAR_MIPMAP_LINEAR;
	}

	glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, mMinFilter);
	glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, mMagFilter);
	glTexParameteri(mTarget, GL_TEXTURE_WRAP_S, mWrapParamS);
	glTexParameteri(mTarget, GL_TEXTURE_WRAP_T, mWrapParamT);
	glTexParameteri(mTarget, GL_TEXTURE_WRAP_R, mWrapParamR);

	
	checkglerror();
	return true;
}

void Texture::setParam(int minFilter, int magFilter, int wrapS, int wrapT,int wrapR) {
	mMinFilter = minFilter;
	mMagFilter = magFilter;
	mWrapParamS = wrapS;
	mWrapParamT = wrapT;
	mWrapParamR = wrapR;
	if (mTextureId > 0) {
		glBindTexture(mTarget, mTextureId);
		glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, mMinFilter);
		glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, mMagFilter);
		glTexParameteri(mTarget, GL_TEXTURE_WRAP_S, mWrapParamS);
		glTexParameteri(mTarget, GL_TEXTURE_WRAP_T, mWrapParamT);
		if (mTarget == GL_TEXTURE_CUBE_MAP) {
			glTexParameteri(mTarget, GL_TEXTURE_WRAP_R, mWrapParamR);
		}
		glBindTexture(mTarget, 0);
	}
}

bool Texture::create2DMap(int width,int height,unsigned char* pdata, GLint internalFormat,GLint format,GLenum type, int aligment,bool autoMipmap)
{
	mFormat = format;
	mInternalFormat = internalFormat;
	mWidth = width;
	mHeight = height;
	mTarget = GL_TEXTURE_2D;
	mType = type;
	unload();
	glGenTextures(1, &mTextureId);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(mTarget, mTextureId);//mTarget是GL_TEXTURE_CUBE_MAP的时候要注意，
	// Set-up texture properties.
	mMinFilter = autoMipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
	glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, mMinFilter);
	glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER,mMagFilter);
	glTexParameteri(mTarget, GL_TEXTURE_WRAP_S, mWrapParamS);
	glTexParameteri(mTarget, GL_TEXTURE_WRAP_T, mWrapParamT);
	// Loads image data into OpenGL.

	glPixelStorei(GL_UNPACK_ALIGNMENT, aligment);
	
	glTexImage2D(mTarget, 0, mInternalFormat, mWidth, mHeight, 0,
		mFormat, mType, pdata);

	if(autoMipmap)
	{
		//可以renderable he filterable的纹理才可以生成mipmap
		glGenerateMipmap(mTarget);
	}
	checkglerror();
	/*if (glGetError() != GL_NO_ERROR)
	{
		LOGE("Error loading texture into OpenGL.");
		unload();
		return false;
	}*/
	return true;
}

bool Texture::loadHdrFile(const std::string& path) {
	stbi_set_flip_vertically_on_load(true);
	int nrComponents;
	float* data = stbi_loadf(path.c_str(), &mWidth, &mHeight, &nrComponents, 0);
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
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(mTarget, mTextureId);
		glTexImage2D(GL_TEXTURE_2D, 0, mInternalFormat, mWidth, mHeight, 0, mFormat, mType, data);

		glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, mMinFilter);
		glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, mMagFilter);
		glTexParameteri(mTarget, GL_TEXTURE_WRAP_S, mWrapParamS);
		glTexParameteri(mTarget, GL_TEXTURE_WRAP_T, mWrapParamT);

		stbi_image_free(data);
		return true;
	}
	else
	{
		LOGE("loadHdrFile failed at path: %s", path.c_str());
		return false;
	}
}

bool Texture::loadFromFile(const std::string& path) {
	int nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path.c_str(), &mWidth, &mHeight, &nrChannels, 0);
	if (data)
	{
		mTarget = GL_TEXTURE_2D;
		unload();
		glGenTextures(1, &mTextureId);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(mTarget, mTextureId);
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
			LOGE("load texture %s,unknow channels: %d", path.c_str(), nrChannels);
		}
		glTexImage2D(mTarget,0, mFormat, mWidth, mHeight, 0, mFormat, mType, data);
		glGenerateMipmap(mTarget);
		mMinFilter = GL_LINEAR_MIPMAP_LINEAR;
		glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, mMinFilter);
		glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, mMagFilter);
		glTexParameteri(mTarget, GL_TEXTURE_WRAP_S, mWrapParamS);
		glTexParameteri(mTarget, GL_TEXTURE_WRAP_T, mWrapParamT);
		checkglerror();
		stbi_image_free(data);
		return true;
	}
	else
	{
		LOGE("load texture from file: %s", path.c_str());
		return false;
	}

}

bool Texture::loadCubemap(const std::string& path) {
	unload();
	mTarget = GL_TEXTURE_CUBE_MAP;
	glGenTextures(1, &mTextureId);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(mTarget, mTextureId);

	std::vector<std::string> filename{ "/right.jpg","/left.jpg","/top.jpg","/bottom.jpg","/front.jpg","/back.jpg" };
	stbi_set_flip_vertically_on_load(true);
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
	//int align = 0;
	//glGetIntegerv(GL_UNPACK_ALIGNMENT, &align);//默认是4，the alignment requirements for the start of each pixel row in memory

	// Set-up texture properties.
	mMinFilter = GL_LINEAR_MIPMAP_LINEAR;
	glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, mMinFilter);
	glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, mMagFilter);
	glTexParameteri(mTarget, GL_TEXTURE_WRAP_S, mWrapParamS);
	glTexParameteri(mTarget, GL_TEXTURE_WRAP_T, mWrapParamT);
	glTexParameteri(mTarget, GL_TEXTURE_WRAP_R, mWrapParamR);

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
	//checkglerror();
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

std::shared_ptr<Texture> Texture::loadImageFromFile(const std::string& path) {
	auto pTex = std::make_shared<Texture>();
	if (pTex) {
		bool b = false;
		if (Utils::getFileSuffix(path) == "hdr") {
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