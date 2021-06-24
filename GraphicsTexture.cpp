//#include "StdAfx.h"
#include "GraphicsTexture.h"
#include "../Log.h"
#include "../rapidxml.hpp"

#include "../png/png.h"

#define STATUS_OK 0
#define STATUS_KO 1

extern void checkglerror();

GraphicsTexture::GraphicsTexture():
mTextureId(0),
	mWidth(0),
	mHeight(0),
	mTarget(GL_TEXTURE_2D)
{

}
GraphicsTexture::~GraphicsTexture()
{
	unload();
}


int32_t GraphicsTexture::getHeight() {
	return mHeight;
}

int32_t GraphicsTexture::getWidth() {
	return mWidth;
}


int GraphicsTexture::load(const char* mPath,bool autoMipmap) {
	unsigned char* lImageBuffer = loadImage(mPath);
	if (lImageBuffer == NULL)
	{
		return STATUS_KO;
	}

	// Creates a new OpenGL texture.
	//GLenum lErrorResult;
	glGenTextures(1, &mTextureId);
	glBindTexture(GL_TEXTURE_2D, mTextureId);
	// Set-up texture properties.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
		GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
		GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
		GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
		GL_CLAMP_TO_EDGE);
	// Loads image data into OpenGL.
	int internalFormat = mFormat;//png load this value is equel to mFormat;
	glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, mWidth, mHeight, 0,
		mFormat, GL_UNSIGNED_BYTE, lImageBuffer);

	if(autoMipmap)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	if(lImageBuffer!=0)
	{
		delete[] lImageBuffer;
		lImageBuffer = 0;
	}
	
	if (glGetError() != GL_NO_ERROR)
	{
		LOGE("Error loading texture into OpenGL.");
		unload();
		return STATUS_KO;
	}
	return STATUS_OK;
}

int GraphicsTexture::load(int width,int height,unsigned char* pdata,GLint format,GLenum type,bool autoMipmap)
{
	mFormat = format;
	mWidth = width;
	mHeight = height;

	glGenTextures(1, &mTextureId);
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
	if (internalformat == GL_BGR)
	{
		internalformat = GL_RGB;
	}
	//这个函数后面三个参数指定了纹理数据在内存中的组织方式。
	//internalformat是指opengl在显存中创建的这张纹理，是什么格式的，这个格式必须与内存中的format匹配，
	//也就是说这个函数不会进行数据格式转换。
	//第二个参数是mipmap等级。0最大，第六个参数是指边框的宽带，必须为0。
	glTexImage2D(GL_TEXTURE_2D, 0, internalformat, mWidth, mHeight, 0,
		mFormat, type, pdata);

	checkglerror();

	if(autoMipmap)
	{
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	if (glGetError() != GL_NO_ERROR)
	{
		//LOGE("Error loading texture into OpenGL.");
		unload();
		return STATUS_KO;
	}
	return STATUS_OK;
}

void GraphicsTexture::unload()
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

void GraphicsTexture::active(GLint textPoint)
{
	glActiveTexture(textPoint);
	glBindTexture(GL_TEXTURE_2D, mTextureId);
	checkglerror();
}

void GraphicsTexture::update(int xoffset,int yoffset,int width,int height,void* data,
	int level)
{
	//active(activeIndex);
	glBindTexture(GL_TEXTURE_2D, mTextureId);
	glTexSubImage2D(GL_TEXTURE_2D,level,xoffset,yoffset,width,height,mFormat,GL_UNSIGNED_BYTE,data);
	checkglerror();
}


int GraphicsTexture::maxTexunit()
{
	int num=0;
	glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,&num);
	return num;
}
int GraphicsTexture::numOfCompressFormat()
{
	int num=0;
	glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS,&num);
	return num;
}
void GraphicsTexture::getCompressFormat(GLint* formats)
{
	glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS,formats);
}

GraphicsImage::GraphicsImage():
mpTex(0),
	mx(0),
	my(0),
	mWidth(0),
	mHeight(0)
{

}
GraphicsImage::~GraphicsImage()
{

}
void GraphicsImage::initImage(GraphicsTexture* pTex,int left,int bottom,int width,int height)
{
	//float w,h;
	if(pTex!=0)
	{
		mpTex=pTex;
		mx = left;
		my = bottom;
		mWidth = width;
		mHeight = height;

	}
}

void GraphicsImage::update(void* data)
{
	if(mpTex!=0)
	{
		mpTex->update(mx,my,mWidth,mHeight,data);
	}
}

void GraphicsImage::getTexCoord(float* pTexpos)
{
	float fx = (float)mx/(float)mpTex->getWidth();
	float fy = (float)my/(float)mpTex->getHeight();
	float fw = (float)mWidth/(float)mpTex->getWidth();
	float fh = (float)mHeight/(float)mpTex->getHeight();
	pTexpos[0]=fx;
	pTexpos[1]=fy+fh;

	pTexpos[2]=fx;
	pTexpos[3]=fy;

	pTexpos[4]=fx+fw;
	pTexpos[5]=fy;

	pTexpos[6]=fx+fw;
	pTexpos[7]=fy+fh;
}

void GraphicsImage::active(GLint texPoint)
{
	if(mpTex!=0)
	{
		mpTex->active(GL_TEXTURE0+texPoint);
	}
}

GraphicsImageManager::GraphicsImageManager()
{


}
GraphicsImageManager::~GraphicsImageManager()
{

}

bool GraphicsImageManager::loadFromFile(const char* fname)
{
	using namespace rapidxml;
	xml_document<> lXmlDocument;
	xml_node<>* lTexturesNode, *lTextureNode,*lImageNode;
	char* lFileBuffer = 0;
	int32_t lLength = 0;
	//Resource lres(fname);
	const void* lFileBufferTmp=0;

	GraphicsTexture* lGTex;
	xml_attribute<>* texFilename = 0;
	GraphicsImage* lGImage;
	xml_attribute<>* imagename,*left,*bottom,*width,*height;
	char* imageName=0;
	int imageLeft=0;
	int imageBottom=0;
	int imageWidth=0;
	int imageHeight=0;

	FILE* pfile = fopen(fname,"r");
	if(pfile!=0)
	{
		fseek(pfile,0L,SEEK_END);
		long flen=ftell(pfile);
		lFileBuffer = new char[flen+1];
		memset(lFileBuffer,0,flen+1);
		if(lFileBuffer==0)
		{
			fclose(pfile);
			return false;
		}
		fseek(pfile,0L,SEEK_SET);
		fread(lFileBuffer,flen,1,pfile); 
		lFileBuffer[flen]=0; 
	}
	else
	{
		return false;
	}
	/*if (lres.open() != STATUS_OK)
	{
		lres.close();
		return false;
	}
	lLength = lres.getLength();
	if (lLength <= 0)
	{
		lres.close();
		return false;
	}
	lFileBufferTmp = lres.bufferize();
	if(lFileBufferTmp==0)
	{
		lres.close();
		return false;
	}
	lFileBuffer = new char[lLength + 1];
	memcpy(lFileBuffer, lFileBufferTmp, lLength);
	lFileBuffer[lLength] = '\0';
	lres.close();*/

	lXmlDocument.parse<parse_default>(lFileBuffer);
	lTexturesNode = lXmlDocument.first_node("Textures");
	if(lTexturesNode!=0)
	{
		lTextureNode = lTexturesNode->first_node("Texture");
		while(lTextureNode!=0)
		{
			texFilename = lTextureNode->first_attribute("filename");
			if(texFilename!=0)
			{
				lGTex = new GraphicsTexture();
				char* texfilename = texFilename->value();
				if(lGTex->load((const char*)(texfilename))==STATUS_OK)
				{

					if(mTextureMap.insert(std::make_pair(std::string(texfilename),lGTex)).second==false)
					{
						LOGD("load texture %s is already exist\n",texfilename);
						lTextureNode = lTextureNode->next_sibling("Texture");
						continue;
					}
					lImageNode = lTextureNode->first_node("Image");
					while(lImageNode!=0)
					{

						imagename = lImageNode->first_attribute("name");
						if(imagename!=0)
						{
							imageName = imagename->value();

						}
						else
						{
							LOGE("xml Image node no name\n");
							lImageNode = lImageNode->next_sibling("Image");
							continue;
						}
						left = lImageNode->first_attribute("left");
						if(left!=0)
						{
							imageLeft = atoi(left->value());
						}
						else
						{
							LOGE("xml Image node no left\n");
							lImageNode = lImageNode->next_sibling("Image");
							continue;
						}
						bottom = lImageNode->first_attribute("bottom");
						if(bottom!=0)
						{
							imageBottom = atoi(bottom->value());
						}
						else
						{
							LOGE("xml Image node no bottom\n");
							lImageNode = lImageNode->next_sibling("Image");
							continue;
						}
						width = lImageNode->first_attribute("width");
						if(width!=0)
						{
							imageWidth = atoi(width->value());
						}
						else
						{
							LOGE("xml Image node no width\n");
							lImageNode = lImageNode->next_sibling("Image");
							continue;
						}
						height = lImageNode->first_attribute("height");
						if(height!=0)
						{
							imageHeight = atoi(height->value());
						}
						else
						{
							LOGE("xml Image node no height\n");
							lImageNode = lImageNode->next_sibling("Image");
							continue;
						}
						lGImage = new GraphicsImage();

						lGImage->initImage(lGTex,imageLeft,imageBottom,
							imageWidth,imageHeight);

						if(mImageMap.insert(std::make_pair(std::string(imageName),lGImage)).second == false)
						{
							LOGD("load image %s is already exist\n",imageName);
						}
						lImageNode = lImageNode->next_sibling("Image");
					}
				}
			}
			else
			{
				LOGE("xml Texture node no filename\n");

			}
			lTextureNode = lTextureNode->next_sibling("Texture");
		}
	}
	if(lFileBuffer!=0)
	{
		delete[] lFileBuffer;
	}

	return true;

}
GraphicsImage* GraphicsImageManager::findImage(const char* imageName)
{
	auto it = mImageMap.find(imageName);
	if(it!=mImageMap.end())
	{
		return it->second;
	}
	return 0;
}
GraphicsTexture* GraphicsImageManager::findTexture(const char* texName)
{

	auto it = mTextureMap.find(texName);
	if(it!=mTextureMap.end())
	{
		return it->second;
	}
	return 0;
}

GraphicsImage* GraphicsImageManager::createImage(const char* name,
	int32_t width,int32_t height,GLint format)
{
	GraphicsTexture* ptex = new GraphicsTexture();
	ptex->load(width,height,0,format);

	GraphicsImage* pimg = new GraphicsImage();
	pimg->initImage(ptex,0.0f,0.0f,width,height);

	if(ptex!=0 && pimg!=0)
	{
		if(mTextureMap.insert(std::make_pair(name,ptex)).second==false)
		{
			LOGD("Texture name %s already exist\n",name);
			delete ptex;
			delete pimg;
			return 0;
		}
		if(mImageMap.insert(std::make_pair(name,pimg)).second == false)
		{
			LOGD("Image name %s already exist\n",name);
			delete ptex;
			delete pimg;
			return 0;
		}
	}
	
	return pimg;
}
GraphicsImageManager* GraphicsImageManager::gImageManagerInstance = 0;
GraphicsImageManager* GraphicsImageManager::getInstance()
{
	if(gImageManagerInstance==0)
	{
		gImageManagerInstance = new GraphicsImageManager();
	}
	return gImageManagerInstance;
}

void callback_read(png_structp pStruct,png_bytep pData, png_size_t pSize)
{
	FILE* pFile = ((FILE*) png_get_io_ptr(pStruct));
	if(pFile!=0)
	{
		if (fread(pData,1,pSize,pFile) == 0) 
		{
			fclose(pFile);
		}
	}
}

unsigned char* GraphicsTexture::loadImage(const char* mPath)
{
	LOGD("Loading texture %s", mPath);

	png_byte lHeader[8];
	png_structp lPngPtr = NULL;
	png_infop lInfoPtr = NULL;
	png_byte* lImageBuffer = NULL;
	png_bytep* lRowPtrs = NULL;
	png_int_32 lRowSize;
	bool lTransparency;
	//Resource mResource(mPath);
	FILE* pFile = fopen(mPath,"rb");
	// Opens and checks image signature (first 8 bytes).
	if (pFile == 0) goto ERROR1;

	if (fread(lHeader, 1,sizeof(lHeader),pFile) == 0)
		goto ERROR1;
	if (png_sig_cmp(lHeader, 0, 8) != 0) goto ERROR1;

	// Creates required structures.
	//packt_Log_debug("Creating required structures.");
	lPngPtr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL);
	if (!lPngPtr) goto ERROR1;
	lInfoPtr = png_create_info_struct(lPngPtr);
	if (!lInfoPtr) goto ERROR1;

	// Prepares reading operation by setting-up a read callback.
	png_set_read_fn(lPngPtr, pFile, callback_read);
	// Set-up error management. If an error occurs while reading,
	// code will come back here and jump
	if (setjmp(png_jmpbuf(lPngPtr))) goto ERROR1;

	// Ignores first 8 bytes already read and processes header.
	png_set_sig_bytes(lPngPtr, 8);
	png_read_info(lPngPtr, lInfoPtr);
	// Retrieves PNG info and updates PNG struct accordingly.
	png_int_32 lDepth, lColorType;
	png_uint_32 lWidth, lHeight;
	png_get_IHDR(lPngPtr, lInfoPtr, &lWidth, &lHeight,
		&lDepth, &lColorType, NULL, NULL, NULL);
	mWidth = lWidth; mHeight = lHeight;

	// Creates a full alpha channel if transparency is encoded as
	// an array of palette entries or a single transparent color.
	lTransparency = false;
	if (png_get_valid(lPngPtr, lInfoPtr, PNG_INFO_tRNS)) {
		png_set_tRNS_to_alpha(lPngPtr);
		lTransparency = true;
		goto ERROR1;
	}
	// Expands PNG with less than 8bits per channel to 8bits.
	if (lDepth < 8) {
		png_set_packing (lPngPtr);
		// Shrinks PNG with 16bits per color channel down to 8bits.
	} else if (lDepth == 16) {
		png_set_strip_16(lPngPtr);
	}
	// Indicates that image needs conversion to RGBA if needed.
	switch (lColorType) {
	case PNG_COLOR_TYPE_PALETTE:
		png_set_palette_to_rgb(lPngPtr);
		mFormat = lTransparency ? GL_RGBA : GL_RGB;
		break;
	case PNG_COLOR_TYPE_RGB:
		mFormat = lTransparency ? GL_RGBA : GL_RGB;
		break;
	case PNG_COLOR_TYPE_RGBA:
		mFormat = GL_RGBA;
		break;
	case PNG_COLOR_TYPE_GRAY:
		png_set_expand_gray_1_2_4_to_8(lPngPtr);
		mFormat = lTransparency ? GL_LUMINANCE_ALPHA:GL_LUMINANCE;
		break;
	case PNG_COLOR_TYPE_GA:
		png_set_expand_gray_1_2_4_to_8(lPngPtr);
		mFormat = GL_LUMINANCE_ALPHA;
		break;
	}
	// Validates all tranformations.
	png_read_update_info(lPngPtr, lInfoPtr);

	// Get row size in bytes.
	lRowSize = png_get_rowbytes(lPngPtr, lInfoPtr);
	if (lRowSize <= 0) goto ERROR1;
	// Ceates the image buffer that will be sent to OpenGL.
	lImageBuffer = new png_byte[lRowSize * lHeight];
	if (!lImageBuffer) goto ERROR1;
	// Pointers to each row of the image buffer. Row order is
	// inverted because different coordinate systems are used by
	// OpenGL (1st pixel is at bottom left) and PNGs (top-left).
	lRowPtrs = new png_bytep[lHeight];
	if (!lRowPtrs) goto ERROR1;
	for (int32_t i = 0; i < lHeight; ++i) {
		lRowPtrs[lHeight - (i + 1)] = lImageBuffer + i * lRowSize;
	}
	// Reads image content.
	png_read_image(lPngPtr, lRowPtrs);

	// Frees memory and resources.
	if(pFile!=0)
	{
		fclose(pFile);
	}
	png_destroy_read_struct(&lPngPtr, &lInfoPtr, NULL);
	delete[] lRowPtrs;
	return lImageBuffer;

ERROR1:
	LOGE("Error while reading PNG file");
	if(pFile!=0)
	{
		fclose(pFile);
	}
	delete[] lRowPtrs; delete[] lImageBuffer;
	if (lPngPtr != NULL) {
		png_infop* lInfoPtrP = lInfoPtr != NULL ? &lInfoPtr: NULL;
		png_destroy_read_struct(&lPngPtr, lInfoPtrP, NULL);
	}
	return NULL;
}


UITexture::UITexture():GraphicsTexture()
{

}

UITexture::~UITexture()
{

}

void UITexture::addImage(const std::string name, int x, int y, int width, int height)
{
	GraphicsImage image;
	image.initImage(this, x, y, width, height);
	mImageMap.insert(std::make_pair(name, image));
}

TextTexture::TextTexture():GraphicsTexture()
{
}

TextTexture::~TextTexture()
{
}

void TextTexture::addImage(wchar_t text, int x, int y, int width, int height)
{
	GraphicsImage image;
	image.initImage(this, x, y, width, height);
	mImageMap.insert(std::make_pair(text, image));
}