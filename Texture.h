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

class Texture
{
public:
	Texture();
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
	//format:GL_RGB,GL_RGBA,GL_DEPTH_COMPONENT
	//type:GL_UNSIGNED_BYTE,GL_UNSIGNED_SHORT,GL_UNSIGNED_SHORT_5_6_5
	bool load(int width,int height,unsigned char* pdata,GLint format=GL_RGB,GLenum type=GL_UNSIGNED_BYTE, int aligment = 4,bool autoMipmap=false);
	
	/*
	* path:指定了六个cubemap文件所在的路径，这个路径里面有"right.jpg","left.jpg","top.jpg","bottom.jpg","front.jpg","back.jpg"
	*/
	bool loadCubemap(const std::string& path);

	bool loadFromeFile(const std::string& path);

	void unload();

	////参数为是否自动为这个纹理生存mipmap
	//int load(const char* mPath,bool autoMipmap=false);

	//GL_TEXTURE0
	void active(GLint texPoint);


	void update(int xoffset,int yoffset,int width,int height,void* data,
		int aligment = 1,int mimmapLevel = 0);


	//支持的最大的纹理单元
	static int maxTexunit();
	//支持几种压缩纹理格式：
	static int numOfCompressFormat();
	//传入一个数组，用于获取支持的压缩纹理格式
	static void getCompressFormat(GLint* formats);

	static std::unique_ptr<Texture> loadImageFromFile(const std::string& path);
protected:
	//unsigned char* loadImage(const char* mPath);

	GLuint mTextureId{ 0 };
	int32_t mWidth{ 0 };
	int32_t mHeight{ 0 };
	GLint mInternalFormat{ GL_RGB };
	GLint mFormat{ GL_RGB };
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
