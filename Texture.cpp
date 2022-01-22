//#include "StdAfx.h"
#include "Texture.h"
#include "Log.h"
#include <vector>
#include "Utils.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Fbo.h"
#include "Mesh.h"
#include "Material.h"
#include "Pbo.h"
#include <iostream>

TextureP gpTextureNothing;
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

bool Texture::createCubicMap(int width, int height, GLint internalFormat, GLenum format, GLenum type) {
	mTarget = GL_TEXTURE_CUBE_MAP;
	mInternalFormat = internalFormat;
	mFormat = format;
	mType = type;
	mWidth = width;
	mHeight = height;
	glGenTextures(1, &mTextureId);
	glBindTexture(mTarget, mTextureId);
	for (unsigned int i = 0; i < 6; ++i)
	{
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, mInternalFormat, mWidth, mHeight, 0, mFormat, mType, nullptr);
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	return true;
}

bool Texture::create2DMap(int width,int height,unsigned char* pdata, GLint internalFormat,GLint format,GLenum type, int aligment,bool autoMipmap)
{
	mFormat = format;
	mInternalFormat = internalFormat;
	mWidth = width;
	mHeight = height;
	mTarget = GL_TEXTURE_2D;
	mType = type;
	glGenTextures(1, &mTextureId);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(mTarget, mTextureId);//mTarget是GL_TEXTURE_CUBE_MAP的时候要注意，
	// Set-up texture properties.
	glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(mTarget, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(mTarget, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
	// Loads image data into OpenGL.

	//int align = 0;
	//glGetIntegerv(GL_UNPACK_ALIGNMENT, &align);//默认是4，the alignment requirements for the start of each pixel row in memory
	//这个函数设置图像在内存中，每一行开头的对齐方式，按几个字节对齐1，2，4，8，16，32...
	glPixelStorei(GL_UNPACK_ALIGNMENT, aligment);
	//这个函数后面三个参数指定了纹理数据在内存中的组织方式。
	//internalformat是指opengl在显存中创建的这张纹理，是什么格式的，这个格式必须与内存中的format匹配，
	// 与mForamt，type是一对参数
	//第二个参数是mipmap等级。0最大，第六个参数是指边框的宽度，必须为0。
	glTexImage2D(mTarget, 0, mInternalFormat, mWidth, mHeight, 0,
		mFormat, mType, pdata);

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

		glGenTextures(1, &mTextureId);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(mTarget, mTextureId);
		glTexImage2D(GL_TEXTURE_2D, 0, mInternalFormat, mWidth, mHeight, 0, mFormat, mType, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
		return true;
	}
	else
	{
		LOGE("Cubemap tex failed to load at path: %s", path.c_str());
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
		glGenTextures(1, &mTextureId);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(mTarget, mTextureId);
		glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(mTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(mTarget, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
		glTexParameteri(mTarget, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
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
			LOGE("Cubemap texture %s,unknow channels: %d", path.c_str(), nrChannels);
		}
		glTexImage2D(mTarget,0, mFormat, mWidth, mHeight, 0, mFormat, mType, data);
		glGenerateMipmap(mTarget);
		stbi_image_free(data);
		return true;
	}
	else
	{
		LOGE("Cubemap tex failed to load at path: %s", path.c_str());
		return false;
	}

}

bool Texture::loadCubemap(const std::string& path) {
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
			LOGE( "Cubemap tex failed to load at path: %s", filePath.c_str());
			stbi_image_free(data);
		}
		++i;
	}
	glGenerateMipmap(mTarget);
	//int align = 0;
	//glGetIntegerv(GL_UNPACK_ALIGNMENT, &align);//默认是4，the alignment requirements for the start of each pixel row in memory

	// Set-up texture properties.
	glTexParameteri(mTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
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

TextureP Texture::genDiffuseIrrMap() {
	constexpr int irrWidth = 128;
	//由于不能渲染到浮点纹理，用于临时接收irradiance convolution的结果
	TextureP floatR = std::make_shared<Texture>();
	TextureP floatG = std::make_shared<Texture>();
	TextureP floatB = std::make_shared<Texture>();
	//保存irrdiance map到cubemap
	TextureP floatCubeIrr = std::make_shared<Texture>();

	floatR->create2DMap(irrWidth, irrWidth, nullptr, GL_RGBA, GL_RGBA);
	floatG->create2DMap(irrWidth, irrWidth, nullptr, GL_RGBA, GL_RGBA);
	floatB->create2DMap(irrWidth, irrWidth, nullptr, GL_RGBA, GL_RGBA);
	floatCubeIrr->createCubicMap(irrWidth, irrWidth, GL_RGB16F, GL_RGB, GL_FLOAT);
	

	Mesh mesh(MeshType::MESH_Cuboid);
	mesh.loadMesh();
	auto& pMaterial = Material::getMaterial("irradianceConvolution");
	pMaterial->setTextureForSampler("skybox", shared_from_this());
	mesh.setMaterial(pMaterial);

	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};
	Pbo pbo;
	pbo.initPbo(irrWidth, irrWidth);
	Fbo fbo;
	fbo.attachColorTexture(floatR, 0);
	fbo.attachColorTexture(floatG, 1);
	fbo.attachColorTexture(floatB, 2);
	
	for (int i = 0; i < 6; ++i) {
		fbo.render([&mesh, &captureProjection, &captureViews, &floatCubeIrr, &pbo, irrWidth, i] {
			mesh.render(captureProjection * captureViews[i], captureViews[i]);
			//将渲染得到的三张rgba转换成一张rgb32f
			std::vector<unsigned char> pRGB[3];
			std::vector<float> resultRGB;
			int index = 0;
			std::function<void(void*)> func = [&index, irrWidth, &pRGB](void* pdata) {
				pRGB[index++].assign((unsigned char*)pdata, (unsigned char*)pdata + irrWidth * irrWidth * 4);
			};
			pbo.pullToMem(GL_COLOR_ATTACHMENT0, func);
			pbo.pullToMem(GL_COLOR_ATTACHMENT1, func);
			pbo.pullToMem(GL_COLOR_ATTACHMENT2, func);
			//转换成rgb32f
			resultRGB.resize(3 * irrWidth * irrWidth);
			constexpr float scaleTo = 1000.0f; //shader里面float值先除了1000.0f
			constexpr float scale = 1.0f / 255.0f;
			constexpr float scale2 = 1.0f / 65025.0f;
			constexpr float scale3 = 1.0f / 16581375.0f;
			for (int i = 0; i < irrWidth; ++i) {
				for (int j = 0; j < irrWidth; ++j) {
					int i1 = 4 * (i * irrWidth + j);
					int r1 = 3 * (i * irrWidth + j);
					resultRGB[r1] = pRGB[0][i1] * scale +
						pRGB[0][i1 + 1] * scale2 +
						pRGB[0][i1 + 2] * scale3 +
						pRGB[0][i1 + 3] * scale * scale3;
					resultRGB[r1 + 1] = pRGB[1][i1] * scale +
						pRGB[1][i1 + 1] * scale2 +
						pRGB[1][i1 + 2] * scale3 +
						pRGB[1][i1 + 3] * scale * scale3;
					resultRGB[r1 + 2] = pRGB[2][i1] * scale +
						pRGB[2][i1 + 1] * scale2 +
						pRGB[2][i1 + 2] * scale3 +
						pRGB[2][i1 + 3] * scale * scale3;
					resultRGB[r1] *= scaleTo;
					resultRGB[r1 + 1] *= scaleTo;
					resultRGB[r1 + 2] *= scaleTo;
				}
			}
			//update到cubemap
			floatCubeIrr->update(0, 0, irrWidth, irrWidth, resultRGB.data(), i);
			});
	}
	//*this = std::move(*floatCubeIrr);
	return floatCubeIrr;
}

void Texture::convertHdrToCubicmap() {
	constexpr int width = 1024;
	Fbo fbo;
	//由于不能渲染到浮点纹理，搞三张rgba接收一个浮点的rgb纹理
	TextureP floatR = std::make_shared<Texture>();
	TextureP floatG = std::make_shared<Texture>();
	TextureP floatB = std::make_shared<Texture>();

	//保存hdr到cubemap
	TextureP floatCube = std::make_shared<Texture>();
	
	Mesh mesh(MeshType::MESH_Cuboid);
	mesh.loadMesh();
	auto& pMaterial = Material::getMaterial("hdrToCubicMap");
	pMaterial->setTextureForSampler("equirectangularMap", shared_from_this());
	mesh.setMaterial(pMaterial);

	floatR->create2DMap(width, width, nullptr, GL_RGBA, GL_RGBA);
	floatG->create2DMap(width, width, nullptr, GL_RGBA, GL_RGBA);
	floatB->create2DMap(width, width, nullptr, GL_RGBA, GL_RGBA);
	floatCube->createCubicMap(width, width, GL_RGB16F, GL_RGB, GL_FLOAT);
	fbo.attachColorTexture(floatR, 0);
	fbo.attachColorTexture(floatG, 1);
	fbo.attachColorTexture(floatB, 2);

	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};
	Pbo pbo;
	pbo.initPbo(width, width);
	for (int i = 0; i < 6; ++i) {
		fbo.render([&mesh, &captureProjection, &captureViews, &floatCube, &pbo, width, i]{
			mesh.render(captureProjection * captureViews[i], captureViews[i]);
			//将渲染得到的三张rgba转换成一张rgb32f
			std::vector<unsigned char> pRGB[3];
			std::vector<float> resultRGB;
			int index = 0;
			std::function<void(void*)> func = [&index,width, &pRGB](void* pdata) {
				pRGB[index++].assign((unsigned char*)pdata, (unsigned char*)pdata+width*width*4);
			};
			pbo.pullToMem(GL_COLOR_ATTACHMENT0, func);
			pbo.pullToMem(GL_COLOR_ATTACHMENT1, func);
			pbo.pullToMem(GL_COLOR_ATTACHMENT2, func);
			//转换成rgb32f
			resultRGB.resize(3 * width * width);
			constexpr float scaleTo = 100.0f; //shader里面float值先除了100.0f
			constexpr float scale = 1.0f / 255.0f;
			constexpr float scale2 = 1.0f / 65025.0f;
			constexpr float scale3 = 1.0f / 16581375.0f;
			for (int i = 0; i < width; ++i) {
				for (int j = 0; j < width; ++j) {
					int i1 = 4 * (i * width + j);
					int r1 = 3 * (i * width + j);
					resultRGB[r1] = pRGB[0][i1] * scale +
						pRGB[0][i1 + 1] * scale2 +
						pRGB[0][i1 + 2] * scale3 +
						pRGB[0][i1 + 3] * scale * scale3;
					resultRGB[r1+1] = pRGB[1][i1] * scale +
						pRGB[1][i1 + 1] * scale2 +
						pRGB[1][i1 + 2] * scale3 +
						pRGB[1][i1 + 3] * scale * scale3;
					resultRGB[r1 + 2] = pRGB[2][i1] * scale +
						pRGB[2][i1 + 1] * scale2 +
						pRGB[2][i1 + 2] * scale3 +
						pRGB[2][i1 + 3] * scale * scale3;
					resultRGB[r1] *= scaleTo;
					resultRGB[r1+1] *= scaleTo;
					resultRGB[r1+2] *= scaleTo;
				}
			}
			//update到cubemap
			floatCube->update(0, 0, width, width, resultRGB.data(), i);
		});
	}
	*this = std::move(*floatCube);
}