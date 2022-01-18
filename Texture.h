#ifndef _PACKT_GRAPHICSTEXTURE_HPP_
#define _PACKT_GRAPHICSTEXTURE_HPP_
#include<string>
#include <map>
#include <memory>
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>
/*
void glTexImage2D(	GLenum target,
	GLint level,
	GLint internalFormat,
	GLsizei width,
	GLsizei height,
	GLint border,
	GLenum format,
	GLenum type,
	const void * data);

	internalFormat: Specifies the number of color components in the texture
	type：a single color component's type ：signed or unsigned bytes, shorts, or longs, or single-precision floating-point values
	format:these values are grouped into sets of one, two, three, or four values, depending on format

*/

class Texture : public std::enable_shared_from_this<Texture>
{
public:
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
	}
	virtual ~Texture();

	int32_t getHeight();
	int32_t getWidth();
	GLuint getId()
	{
		return mTextureId;
	}
	int getTexTarget() {
		return mTarget;
	}
	//创建一个多重采样纹理，用于添加到fbo，
	bool createMStexture(int width, int height, int samples=4, unsigned int internalformat= GL_RGBA8);
	
	/*
	* 创建一个空的cubicmap
	* internalFormat:该纹理在显存中的格式，GL_RGB,GL_RGBA
	* format:pdata指向的内存数据的格式，GL_RGB,GL_RGBA
	* type:pdata指向的内存，比如rgb中的r的类型，GL_UNSIGNED_BYTE
	*/
	bool createCubicMap(int width,int height, GLint internalFormat = GL_RGB,GLenum format= GL_RGB,GLenum type= GL_UNSIGNED_BYTE);
	
	//internalFormat:该纹理在显存中的格式，GL_RGB,GL_RGBA
	//format:pdata指向的内存数据的格式，GL_RGB,GL_RGBA
	//type:pdata指向的内存，比如rgb中的r的类型，GL_UNSIGNED_BYTE
	bool create2DMap(int width,int height,unsigned char* pdata, GLint internalFormat = GL_RGB, GLint format=GL_RGB,GLenum type =GL_UNSIGNED_BYTE, int aligment = 1,bool autoMipmap=false);
	
	/*
	* path:指定了六个cubemap文件所在的路径，这个路径里面有"right.jpg","left.jpg","top.jpg","bottom.jpg","front.jpg","back.jpg"
	*/
	bool loadCubemap(const std::string& path);

	bool loadFromFile(const std::string& path);

	bool loadHdrFile(const std::string& path);

	void unload();

	////参数为是否自动为这个纹理生存mipmap
	//int load(const char* mPath,bool autoMipmap=false);

	//GL_TEXTURE0
	void active(GLint texPoint);

	/*
	* face: 只针对cubemap，对于2dmap设置为0
	*/
	void update(int xoffset, int yoffset, int width, int height, void* data,
		int face = 0, int aligment = 1, int mimmapLevel = 0);

	void saveToFile(const std::string& path);

	void convertHdrToCubicmap();

	//支持的最大的纹理单元
	static int maxTexunit();
	//支持几种压缩纹理格式：
	static int numOfCompressFormat();
	//传入一个数组，用于获取支持的压缩纹理格式
	static void getCompressFormat(GLint* formats);

	static std::shared_ptr<Texture> loadImageFromFile(const std::string& path);
protected:
	//unsigned char* loadImage(const char* mPath);
	
	GLuint mTextureId{ 0 };
	int mWidth{ 0 };
	int mHeight{ 0 };
	GLint mInternalFormat{ GL_RGB };//纹理数据上传到显存后的格式
	GLint mFormat{ GL_RGB };//纹理数据在内存中的格式
	GLint mType{ GL_UNSIGNED_BYTE };//纹理数据在内存中的格式
	int mNumOfSamples{ 0 };
	int mTarget{ GL_TEXTURE_2D };//GL_TEXTURE_2D, GL_TEXTURE_3D, GL_TEXTURE_2D_ARRAY, or GL_TEXTURE_CUBE_MAP
};

using TextureP = std::shared_ptr<Texture>;
extern TextureP gpTextureNothing;

//class GraphicsImage
//{
//public:
//	GraphicsImage();
//	~GraphicsImage();
//	void initImage(Texture* pTex,int left,int bottom,int width,int height);
//	//GL_TEXTURE0
//	void active(GLint texPoint);
//	void update(void* data);
//	//根据image的位置获得纹理坐标
//	void getTexCoord(float* pTexpos);
//
//	Texture* getTexture()
//	{
//		return mpTex;
//	}
//protected:
//	Texture* mpTex;
//
//	int mx;
//	int my;
//	int mHeight;
//	int mWidth;
//};
//
//class UITexture:public Texture
//{
//public:
//	UITexture();
//	~UITexture();
//
//	void addImage(const std::string name,int x,int y,int width,int height);
//private:
//	std::map<std::string, GraphicsImage> mImageMap;
//};
//
//class TextTexture :public Texture
//{
//public:
//	TextTexture();
//	~TextTexture();
//	void addImage(wchar_t text, int x, int y, int width, int height);
//private:
//	std::map<wchar_t, GraphicsImage> mImageMap;
//};
////
////
//class GraphicsImageManager
//{
//public:
//	typedef std::map<std::string,GraphicsImage*> Mmap;
//	typedef std::map<std::string,GraphicsImage*>::iterator MmapIt;
//	typedef std::pair< std::map<std::string,GraphicsImage*>::iterator,bool > TMapInsertRet;//用于判断插入是否成功
//
//	typedef std::map<std::string,Texture*> MTextureMap;
//	typedef std::map<std::string,Texture*>::iterator MTextureMapIt;
//	typedef std::pair< std::map<std::string,Texture*>::iterator,bool > IMapInsertRet;
//
//	GraphicsImageManager();
//	~GraphicsImageManager();
//
//	//从配置文件加载纹理和image
//	bool loadFromFile(const char* fname);
//	//根据image的名字获取image
//	GraphicsImage* findImage(const char* imageName);
//	//根据texture的名字获取texture
//	Texture* findTexture(const char* texName);
//
//	//创建一个texture和对应名字的image。
//	//format:GL_RGB,GL_RGBA,GL_DEPTH_COMPONENT
//	GraphicsImage* createImage(const char* name,int32_t width,int32_t height,GLint format);
//	static GraphicsImageManager* getInstance();
//private:
//	Mmap mImageMap;
//	MTextureMap mTextureMap;
//	static GraphicsImageManager* gImageManagerInstance;
//};

#endif
