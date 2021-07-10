//#include "StdAfx.h"
#include "Texture.h"
#include "Log.h"

extern void checkglerror();

Texture::Texture():
mTextureId(0),
	mWidth(0),
	mHeight(0),
	mTarget(GL_TEXTURE_2D)
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

bool Texture::load(int width,int height,unsigned char* pdata,GLint format,GLenum type,bool autoMipmap)
{
	mFormat = format;
	mWidth = width;
	mHeight = height;

	glGenTextures(1, &mTextureId);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mTextureId);
	// Set-up texture properties.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
		GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
		GL_CLAMP_TO_EDGE);
	// Loads image data into OpenGL.
	int internalformat = mFormat;//only 1,2,3,4
	/*if (internalformat == GL_BGR)
	{
		internalformat = GL_RGB;
	}*/
	//这个函数后面三个参数指定了纹理数据在内存中的组织方式。
	//internalformat是指opengl在显存中创建的这张纹理，是什么格式的，这个格式必须与内存中的format匹配，
	// 与mForamt，type是一对参数
	//也就是说这个函数不会进行数据格式转换。
	//第二个参数是mipmap等级。0最大，第六个参数是指边框的宽度，必须为0。
	glTexImage2D(GL_TEXTURE_2D, 0, internalformat, mWidth, mHeight, 0,
		mFormat, type, pdata);

	checkglerror();

	if(autoMipmap)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}

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
	mFormat = 0;
}

void Texture::active(GLint textPoint)
{
	glActiveTexture(textPoint);
	glBindTexture(GL_TEXTURE_2D, mTextureId);
	checkglerror();
}

void Texture::update(int xoffset,int yoffset,int width,int height,void* data,
	int level)
{
	//active(activeIndex);
	glBindTexture(GL_TEXTURE_2D, mTextureId);
	glTexSubImage2D(GL_TEXTURE_2D,level,xoffset,yoffset,width,height,mFormat,GL_UNSIGNED_BYTE,data);
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

//GraphicsImage::GraphicsImage():
//mpTex(0),
//	mx(0),
//	my(0),
//	mWidth(0),
//	mHeight(0)
//{
//
//}
//GraphicsImage::~GraphicsImage()
//{
//
//}
//void GraphicsImage::initImage(Texture* pTex,int left,int bottom,int width,int height)
//{
//	//float w,h;
//	if(pTex!=0)
//	{
//		mpTex=pTex;
//		mx = left;
//		my = bottom;
//		mWidth = width;
//		mHeight = height;
//
//	}
//}
//
//void GraphicsImage::update(void* data)
//{
//	if(mpTex!=0)
//	{
//		mpTex->update(mx,my,mWidth,mHeight,data);
//	}
//}
//
//void GraphicsImage::getTexCoord(float* pTexpos)
//{
//	float fx = (float)mx/(float)mpTex->getWidth();
//	float fy = (float)my/(float)mpTex->getHeight();
//	float fw = (float)mWidth/(float)mpTex->getWidth();
//	float fh = (float)mHeight/(float)mpTex->getHeight();
//	pTexpos[0]=fx;
//	pTexpos[1]=fy+fh;
//
//	pTexpos[2]=fx;
//	pTexpos[3]=fy;
//
//	pTexpos[4]=fx+fw;
//	pTexpos[5]=fy;
//
//	pTexpos[6]=fx+fw;
//	pTexpos[7]=fy+fh;
//}
//
//void GraphicsImage::active(GLint texPoint)
//{
//	if(mpTex!=0)
//	{
//		mpTex->active(GL_TEXTURE0+texPoint);
//	}
//}
//
//GraphicsImageManager::GraphicsImageManager()
//{
//
//
//}
//GraphicsImageManager::~GraphicsImageManager()
//{
//
//}

//bool GraphicsImageManager::loadFromFile(const char* fname)
//{
//	using namespace rapidxml;
//	xml_document<> lXmlDocument;
//	xml_node<>* lTexturesNode, *lTextureNode,*lImageNode;
//	char* lFileBuffer = 0;
//	int32_t lLength = 0;
//	//Resource lres(fname);
//	const void* lFileBufferTmp=0;
//
//	Texture* lGTex;
//	xml_attribute<>* texFilename = 0;
//	GraphicsImage* lGImage;
//	xml_attribute<>* imagename,*left,*bottom,*width,*height;
//	char* imageName=0;
//	int imageLeft=0;
//	int imageBottom=0;
//	int imageWidth=0;
//	int imageHeight=0;
//
//	FILE* pfile = fopen(fname,"r");
//	if(pfile!=0)
//	{
//		fseek(pfile,0L,SEEK_END);
//		long flen=ftell(pfile);
//		lFileBuffer = new char[flen+1];
//		memset(lFileBuffer,0,flen+1);
//		if(lFileBuffer==0)
//		{
//			fclose(pfile);
//			return false;
//		}
//		fseek(pfile,0L,SEEK_SET);
//		fread(lFileBuffer,flen,1,pfile); 
//		lFileBuffer[flen]=0; 
//	}
//	else
//	{
//		return false;
//	}
//
//	lXmlDocument.parse<parse_default>(lFileBuffer);
//	lTexturesNode = lXmlDocument.first_node("Textures");
//	if(lTexturesNode!=0)
//	{
//		lTextureNode = lTexturesNode->first_node("Texture");
//		while(lTextureNode!=0)
//		{
//			texFilename = lTextureNode->first_attribute("filename");
//			if(texFilename!=0)
//			{
//				lGTex = new Texture();
//				char* texfilename = texFilename->value();
//				if(lGTex->load((const char*)(texfilename))==STATUS_OK)
//				{
//
//					if(mTextureMap.insert(std::make_pair(std::string(texfilename),lGTex)).second==false)
//					{
//						LOGD("load texture %s is already exist\n",texfilename);
//						lTextureNode = lTextureNode->next_sibling("Texture");
//						continue;
//					}
//					lImageNode = lTextureNode->first_node("Image");
//					while(lImageNode!=0)
//					{
//
//						imagename = lImageNode->first_attribute("name");
//						if(imagename!=0)
//						{
//							imageName = imagename->value();
//
//						}
//						else
//						{
//							LOGE("xml Image node no name\n");
//							lImageNode = lImageNode->next_sibling("Image");
//							continue;
//						}
//						left = lImageNode->first_attribute("left");
//						if(left!=0)
//						{
//							imageLeft = atoi(left->value());
//						}
//						else
//						{
//							LOGE("xml Image node no left\n");
//							lImageNode = lImageNode->next_sibling("Image");
//							continue;
//						}
//						bottom = lImageNode->first_attribute("bottom");
//						if(bottom!=0)
//						{
//							imageBottom = atoi(bottom->value());
//						}
//						else
//						{
//							LOGE("xml Image node no bottom\n");
//							lImageNode = lImageNode->next_sibling("Image");
//							continue;
//						}
//						width = lImageNode->first_attribute("width");
//						if(width!=0)
//						{
//							imageWidth = atoi(width->value());
//						}
//						else
//						{
//							LOGE("xml Image node no width\n");
//							lImageNode = lImageNode->next_sibling("Image");
//							continue;
//						}
//						height = lImageNode->first_attribute("height");
//						if(height!=0)
//						{
//							imageHeight = atoi(height->value());
//						}
//						else
//						{
//							LOGE("xml Image node no height\n");
//							lImageNode = lImageNode->next_sibling("Image");
//							continue;
//						}
//						lGImage = new GraphicsImage();
//
//						lGImage->initImage(lGTex,imageLeft,imageBottom,
//							imageWidth,imageHeight);
//
//						if(mImageMap.insert(std::make_pair(std::string(imageName),lGImage)).second == false)
//						{
//							LOGD("load image %s is already exist\n",imageName);
//						}
//						lImageNode = lImageNode->next_sibling("Image");
//					}
//				}
//			}
//			else
//			{
//				LOGE("xml Texture node no filename\n");
//
//			}
//			lTextureNode = lTextureNode->next_sibling("Texture");
//		}
//	}
//	if(lFileBuffer!=0)
//	{
//		delete[] lFileBuffer;
//	}
//
//	return true;
//
//}
//GraphicsImage* GraphicsImageManager::findImage(const char* imageName)
//{
//	auto it = mImageMap.find(imageName);
//	if(it!=mImageMap.end())
//	{
//		return it->second;
//	}
//	return 0;
//}
//Texture* GraphicsImageManager::findTexture(const char* texName)
//{
//
//	auto it = mTextureMap.find(texName);
//	if(it!=mTextureMap.end())
//	{
//		return it->second;
//	}
//	return 0;
//}
//
//GraphicsImage* GraphicsImageManager::createImage(const char* name,
//	int32_t width,int32_t height,GLint format)
//{
//	Texture* ptex = new Texture();
//	ptex->load(width,height,0,format);
//
//	GraphicsImage* pimg = new GraphicsImage();
//	pimg->initImage(ptex,0.0f,0.0f,width,height);
//
//	if(ptex!=0 && pimg!=0)
//	{
//		if(mTextureMap.insert(std::make_pair(name,ptex)).second==false)
//		{
//			LOGD("Texture name %s already exist\n",name);
//			delete ptex;
//			delete pimg;
//			return 0;
//		}
//		if(mImageMap.insert(std::make_pair(name,pimg)).second == false)
//		{
//			LOGD("Image name %s already exist\n",name);
//			delete ptex;
//			delete pimg;
//			return 0;
//		}
//	}
//	
//	return pimg;
//}
//GraphicsImageManager* GraphicsImageManager::gImageManagerInstance = 0;
//GraphicsImageManager* GraphicsImageManager::getInstance()
//{
//	if(gImageManagerInstance==0)
//	{
//		gImageManagerInstance = new GraphicsImageManager();
//	}
//	return gImageManagerInstance;
//}
//
//
//UITexture::UITexture():Texture()
//{
//
//}
//
//UITexture::~UITexture()
//{
//
//}
//
//void UITexture::addImage(const std::string name, int x, int y, int width, int height)
//{
//	GraphicsImage image;
//	image.initImage(this, x, y, width, height);
//	mImageMap.insert(std::make_pair(name, image));
//}
//
//TextTexture::TextTexture():Texture()
//{
//}
//
//TextTexture::~TextTexture()
//{
//}
//
//void TextTexture::addImage(wchar_t text, int x, int y, int width, int height)
//{
//	GraphicsImage image;
//	image.initImage(this, x, y, width, height);
//	mImageMap.insert(std::make_pair(text, image));
//}