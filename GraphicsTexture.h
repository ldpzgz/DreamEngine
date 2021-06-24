#ifndef _PACKT_GRAPHICSTEXTURE_HPP_
#define _PACKT_GRAPHICSTEXTURE_HPP_
#include "glew.h"
#include<string>
#include <map>
#ifndef int32_t
#define int32_t int
#endif

class GraphicsTexture
{
public:
	GraphicsTexture();
	virtual ~GraphicsTexture();

	int32_t getHeight();
	int32_t getWidth();
	GLuint getId()
	{
		return mTextureId;
	}
	

	//format:GL_RGB,GL_RGBA,GL_DEPTH_COMPONENT
	//type:GL_UNSIGNED_BYTE,GL_UNSIGNED_SHORT,GL_UNSIGNED_SHORT_5_6_5
	int load(int width,int height,unsigned char* pdata,GLint format=GL_RGB,GLenum type=GL_UNSIGNED_BYTE,bool autoMipmap=false);
	void unload();

	//参数为是否自动为这个纹理生存mipmap
	int load(const char* mPath,bool autoMipmap=false);

	//GL_TEXTURE0
	void active(GLint texPoint);


	void update(int xoffset,int yoffset,int width,int height,void* data,
		int level=0);


	//支持的最大的纹理单元
	static int maxTexunit();
	//支持几种压缩纹理格式：
	static int numOfCompressFormat();
	//传入一个数组，用于获取支持的压缩纹理格式
	static void getCompressFormat(GLint* formats);

protected:
	unsigned char* loadImage(const char* mPath);

	GLuint mTextureId;
	int32_t mWidth;
	int32_t mHeight;
	GLint mFormat;
	int mTarget;//GL_TEXTIURE2D,TEXTURE_CUBE_MAP_POSITIVE_X...
};

class GraphicsImage
{
public:
	GraphicsImage();
	~GraphicsImage();
	void initImage(GraphicsTexture* pTex,int left,int bottom,int width,int height);
	//GL_TEXTURE0
	void active(GLint texPoint);
	void update(void* data);
	//根据image的位置获得纹理坐标
	void getTexCoord(float* pTexpos);

	GraphicsTexture* getTexture()
	{
		return mpTex;
	}
protected:
	GraphicsTexture* mpTex;

	int mx;
	int my;
	int mHeight;
	int mWidth;
};

class UITexture:public GraphicsTexture
{
public:
	UITexture();
	~UITexture();

	void addImage(const std::string name,int x,int y,int width,int height);
private:
	std::map<std::string, GraphicsImage> mImageMap;
};

class TextTexture :public GraphicsTexture
{
public:
	TextTexture();
	~TextTexture();
	void addImage(wchar_t text, int x, int y, int width, int height);
private:
	std::map<wchar_t, GraphicsImage> mImageMap;
};
//
//
class GraphicsImageManager
{
public:
	typedef std::map<std::string,GraphicsImage*> Mmap;
	typedef std::map<std::string,GraphicsImage*>::iterator MmapIt;
	typedef std::pair< std::map<std::string,GraphicsImage*>::iterator,bool > TMapInsertRet;//用于判断插入是否成功

	typedef std::map<std::string,GraphicsTexture*> MTextureMap;
	typedef std::map<std::string,GraphicsTexture*>::iterator MTextureMapIt;
	typedef std::pair< std::map<std::string,GraphicsTexture*>::iterator,bool > IMapInsertRet;

	GraphicsImageManager();
	~GraphicsImageManager();

	//从配置文件加载纹理和image
	bool loadFromFile(const char* fname);
	//根据image的名字获取image
	GraphicsImage* findImage(const char* imageName);
	//根据texture的名字获取texture
	GraphicsTexture* findTexture(const char* texName);

	//创建一个texture和对应名字的image。
	//format:GL_RGB,GL_RGBA,GL_DEPTH_COMPONENT
	GraphicsImage* createImage(const char* name,int32_t width,int32_t height,GLint format);
	static GraphicsImageManager* getInstance();
private:
	Mmap mImageMap;
	MTextureMap mTextureMap;
	static GraphicsImageManager* gImageManagerInstance;
};

#endif
