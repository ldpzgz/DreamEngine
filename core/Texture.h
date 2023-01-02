#pragma once

#include<string>
#include <map>
#include <memory>
#ifdef _GLES3
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>
#else
#include <glad/gl.h>
#endif
#include <array>
#include "Color.h"

class Sampler;
class Texture : public std::enable_shared_from_this<Texture>
{
public:
	explicit Texture(const std::string name) {
		mName = name;
	}
	Texture();
	Texture(Texture&& o) noexcept {
		mTextureId = o.mTextureId;
		mWidth = o.mWidth;
		mHeight = o.mHeight;
		mInternalFormat = o.mInternalFormat;
		mFormat = o.mFormat;
		mType = o.mType;
		mNumOfSamples = o.mNumOfSamples;
		mTarget = o.mTarget;
		o.mTextureId = 0;
		mName = std::move(o.mName);
		mpSampler = o.mpSampler;
	}
	void operator=(Texture&& o) noexcept{
		mTextureId = o.mTextureId;
		mWidth = o.mWidth;
		mHeight = o.mHeight;
		mInternalFormat = o.mInternalFormat;
		mFormat = o.mFormat;
		mType = o.mType;
		mNumOfSamples = o.mNumOfSamples;
		mTarget = o.mTarget;
		o.mTextureId = 0;
		mName = o.mName;
		mpSampler = o.mpSampler;
	}
	virtual ~Texture();

	int getHeight();
	int getWidth();
	GLuint getId()
	{
		return mTextureId;
	}
	int getTexTarget() {
		return mTarget;
	}
	int getInternalFormat() {
		return mInternalFormat;
	}
	int getFormat() {
		return mFormat;
	}
	int getType() {
		return mType;
	}

	void setSampler(Sampler* pSampler) {
		mpSampler = pSampler;
	}

	//创建一个多重采样纹理，用于添加到fbo，
	//多重采用纹理，不需要设置sampler
	bool createMStexture(int width, int height, int samples=4, unsigned int internalformat= GL_RGBA8);
	
	/*
	* 创建一个空的cubicmap
	* internalFormat:该纹理在显存中的格式，GL_RGB,GL_RGBA
	* format:pdata指向的内存数据的格式，GL_RGB,GL_RGBA
	* type:pdata指向的内存，比如rgb中的r的类型，GL_UNSIGNED_BYTE
	* opengles3.0的浮点纹理不支持三线性过滤，不支持glGenMipmap（可renderable，filterable的纹理才支持）
	*/
	bool createCubicMap(int width,int height, GLint internalFormat = GL_RGB,GLenum format= GL_RGB,GLenum type= GL_UNSIGNED_BYTE,bool autoMipmap = false);
	
	//internalFormat:该纹理在显存中的格式，GL_RGB,GL_RGBA
	//format:pdata指向的内存数据的格式，GL_RGB,GL_RGBA
	//type:pdata指向的内存，比如rgb中的r的类型，GL_UNSIGNED_BYTE
	bool create2DMap(int width,int height,const unsigned char* pdata, GLint internalFormat = GL_RGB, GLint format=GL_RGB,GLenum type =GL_UNSIGNED_BYTE, bool autoMipmap=false,int aligment=1);
	
	/*
	* path:指定了六个cubemap文件所在的路径，这个路径里面有"right.jpg","left.jpg","top.jpg","bottom.jpg","front.jpg","back.jpg"
	*/
	bool loadCubemap(const std::string& path);

	bool loadFromFile(const std::string& path);

	bool loadHdrFile(const std::string& path);

	void unload();

	void genMipmap();

	////参数为是否自动为这个纹理生存mipmap
	//int load(const char* mPath,bool autoMipmap=false);

	//GL_TEXTURE0
	void active(GLint texPoint);

	void deActive(GLint texPoint);

	/*
	* face: 只针对cubemap，对于2dmap设置为0
	*/
	void update(int xoffset, int yoffset, int width, int height, void* data,
		int face = 0, int aligment = 1, int mimmapLevel = 0);

	void saveToFile(const std::string& path);

	const std::string& getName() {
		return mName;
	}

	void setBorderColor(float* pColor) {
		mBorderColor[0] = pColor[0];
		mBorderColor[1] = pColor[1];
		mBorderColor[2] = pColor[2];
		mBorderColor[3] = pColor[3];
	}

	void setBorderColor(const Color& color) {
		mBorderColor[0] = color.r;
		mBorderColor[1] = color.g;
		mBorderColor[2] = color.b;
		mBorderColor[3] = color.a;
	}

	//支持的最大的纹理单元
	static int maxTexunit();
	//支持几种压缩纹理格式：
	static int numOfCompressFormat();
	//传入一个数组，用于获取支持的压缩纹理格式
	static void getCompressFormat(GLint* formats);

	static std::shared_ptr<Texture> loadImageFromFile(const std::string& path,const std::string& name);
protected:
	GLuint mTextureId{ 0 };
	int mWidth{ 0 };
	int mHeight{ 0 };
	GLint mInternalFormat{ GL_RGB };//纹理数据上传到显存后的格式
	GLint mFormat{ GL_RGB };//纹理数据在内存中的格式
	GLint mType{ GL_UNSIGNED_BYTE };//纹理数据在内存中的格式
	int mNumOfSamples{ 0 };//多重采样纹理的采样个数
	int mTarget{ GL_TEXTURE_2D };//GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_2D_ARRAY, or GL_TEXTURE_CUBE_MAP

	Sampler* mpSampler{ nullptr }; //use sampler to manager tex min filter,max filter etc.
	std::array<float, 4> mBorderColor{ 0.0f,0.0f,0.0f,0.0f };
	std::string mName;
};
using TextureSP = std::shared_ptr<Texture>;

