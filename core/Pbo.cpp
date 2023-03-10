#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <future>
#include "Pbo.h"
#include "Log.h"
#include "Fbo.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"
using namespace std;
static std::vector<std::future<void>> gPboFutures;
int Pbo::gPerfectFormat = GL_RGBA;
int Pbo::gPerfectType = GL_UNSIGNED_BYTE;

void Pbo::getPerfectParam() {
	glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT,&gPerfectFormat);
	glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &gPerfectType);
	LOGD("pbo perfect format and type is 0x%X,0x%X", gPerfectFormat, gPerfectType);
}

void Pbo::getBytesPerPixel(int readFormat,int readType, unsigned int& bytesPerPixel) {
	switch (readType)
	{
	case GL_UNSIGNED_BYTE:
	case GL_BYTE:
		switch (readFormat)
		{
		case GL_RGBA:
			bytesPerPixel = 4;
			break;
		case GL_RGB:
		case GL_RGB_INTEGER:
			bytesPerPixel = 3;
			break;
		case GL_RG:
		case GL_RG_INTEGER:
		case GL_LUMINANCE_ALPHA:
			bytesPerPixel = 2;
			break;
		case GL_RED:
		case GL_RED_INTEGER:
		case GL_ALPHA:
		case GL_LUMINANCE:
			bytesPerPixel = 1;
			break;
		default:
			// Undetected format/error 
			break;
		}
		break;
	case GL_FLOAT:
	case GL_UNSIGNED_INT:
	case GL_INT:
		switch (readFormat)
		{
		case GL_RGBA:
		case GL_RGBA_INTEGER:
			bytesPerPixel = 16;
			break;
		case GL_RGB:
		case GL_RGB_INTEGER:
			bytesPerPixel = 12;
			break;
		case GL_RG:
		case GL_RG_INTEGER:
			bytesPerPixel = 8;
			break;
		case GL_RED:
		case GL_RED_INTEGER:
		case GL_DEPTH_COMPONENT:
			bytesPerPixel = 4;
			break;
		default:
			// Undetected format/error 
			break;
		}
		break;
	case GL_HALF_FLOAT:
	case GL_UNSIGNED_SHORT:
	case GL_SHORT:
		switch (readFormat)
		{
		case GL_RGBA:
		case GL_RGBA_INTEGER:
			bytesPerPixel = 8;
			break;
		case GL_RGB:
		case GL_RGB_INTEGER:
			bytesPerPixel = 6;
			break;
		case GL_RG:
		case GL_RG_INTEGER:
			bytesPerPixel = 4;
			break;
		case GL_RED:
		case GL_RED_INTEGER:
			bytesPerPixel = 2;
			break;
		default:
			// Undetected format/error
			break;
		}
		break;
	case GL_FLOAT_32_UNSIGNED_INT_24_8_REV: // GL_DEPTH_STENCIL
		bytesPerPixel = 8;
		break;
		// GL_RGBA, GL_RGBA_INTEGER format
	case GL_UNSIGNED_INT_2_10_10_10_REV:
	case GL_UNSIGNED_INT_10F_11F_11F_REV: // GL_RGB format
	case GL_UNSIGNED_INT_5_9_9_9_REV: // GL_RGB format
	case GL_UNSIGNED_INT_24_8: // GL_DEPTH_STENCIL format
		bytesPerPixel = 4;
		break;
	case GL_UNSIGNED_SHORT_4_4_4_4: // GL_RGBA format
	case GL_UNSIGNED_SHORT_5_5_5_1: // GL_RGBA format
	case GL_UNSIGNED_SHORT_5_6_5: // GL_RGB format
		bytesPerPixel = 2;
		break;
	default:
		break;
	}
}

Pbo::~Pbo() {
	if (mPbo > 0) {
		glDeleteBuffers(1, &mPbo);
	}
}
void Pbo::saveToFile(unsigned int buffer, const std::string& pathToSave) {
	pullToMem(buffer, [pathToSave](Pbo* pbo, void* pData) {
		if (pData) {
			auto pSData = make_unique<std::vector<unsigned char>>();
			pSData->assign(static_cast<unsigned char*>(pData),
				static_cast<unsigned char*>(pData) + pbo->mWidth * pbo->mHeight * pbo->mBytesPerPixel);
			int w = pbo->mWidth;
			int h = pbo->mHeight;
			int bpp = pbo->mBytesPerPixel;
			auto fut = std::async(std::launch::async, [imageData = std::move(pSData), w, h, bpp, savePath = pathToSave]{
				auto pColor = imageData->data();
				stbi_flip_vertically_on_write(1);
				auto ret = stbi_write_tga(savePath.c_str(), w, h, bpp, pColor);
				if (ret == 0) {
					LOGE("error to write file %s", savePath.c_str());
				}
				else if (ret > 0) {
					LOGD("success to write pbo to file %s", savePath.c_str());
				}
				});
			gPboFutures.emplace_back(std::move(fut));
		}
		});
}
void Pbo::saveToFile(const std::shared_ptr<Texture>& pTex,const std::string& pathToSave) {
	Fbo fbo;
	fbo.attachColorTexture(pTex);
	fbo.enable();
	saveToFile(GL_COLOR_ATTACHMENT0, pathToSave);
	fbo.disable();
}

void Pbo::pullToMem(const std::shared_ptr<Texture>& pTex, std::function<void(Pbo* pbo, void*)> func) {
	Fbo fbo;
	fbo.attachColorTexture(pTex);
	fbo.enable();
	pullToMem(GL_COLOR_ATTACHMENT0, func);
	fbo.disable();
}

void Pbo::initPbo(int w,int h,unsigned int format, unsigned int type) {
	if (mPbo > 0) {
		glDeleteBuffers(1, &mPbo);
	}
	mWidth = w;
	mHeight = h;
	mFormat = format;
	mType = type;
	getBytesPerPixel(mFormat, mType, mBytesPerPixel);
	glGenBuffers(1, &mPbo);
	if (mPbo > 0) {
		glBindBuffer(GL_PIXEL_PACK_BUFFER, mPbo);
		glBufferData(GL_PIXEL_PACK_BUFFER, mWidth * mHeight * mBytesPerPixel, 0, GL_STREAM_READ);
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	}
	else {
		LOGE("ERROR to create pbo");
		return;
	}
}

void Pbo::pullToMem(GLuint colorBuffer,std::function<void(Pbo* pbo, void*)> func) {
	glReadBuffer(colorBuffer);
	if (mPbo > 0) {
		//glPixelStorei(GL_PACK_ALIGNMENT, 1);//only 1,2,4,8 is supported
		glBindBuffer(GL_PIXEL_PACK_BUFFER, mPbo);
		//dma ?????????colorBuffer?????????pbo
		glReadPixels(0, 0, mWidth, mHeight, mFormat, mType, nullptr);
		//_sleep(3000);
		void* pData = glMapBufferRange(GL_PIXEL_PACK_BUFFER,
			0, mWidth * mHeight * mBytesPerPixel, GL_MAP_READ_BIT);
		if (func) {
			func(this,pData);
		}
		glUnmapBuffer(GL_PIXEL_PACK_BUFFER); //????????????
		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
	}
	else {
		LOGE(" please call initPbo before pullToMem");
	}
	glReadBuffer(GL_NONE);
}

//void Pbo::pullColorBufferToMemory(int x,int y,int width,int height) {
//	if ( mTimeCounter.elapse(1000.0f / mFrameRate).first) {
//		std::cout << "Pbo::pullColorBufferToMemory " << Utils::nowTime()<<std::endl;
//		if (mWriteIndex < 0) {
//			mWriteIndex = 0;
//		}
//		glBindBuffer(GL_PIXEL_PACK_BUFFER, mPbo[mWriteIndex]);
//		glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE,nullptr);
//		if (mReadIndex >= 0) {
//			glBindBuffer(GL_PIXEL_PACK_BUFFER, mPbo[mReadIndex]); //??????pbo
//			
//			void*   data = glMapBufferRange(GL_PIXEL_PACK_BUFFER,0, mWidth*mHeight*mBytesPerPixel,GL_MAP_READ_BIT); //?????????map?????????PBO?????????????????????data??????????????????
//
//			if (data) {
//				auto pPic = std::make_shared<cv::Mat>();
//				pPic->create(mHeight, mWidth, CV_8UC3);
//				memcpy((void*)pPic->data, data, mWidth*mHeight*mBytesPerPixel);
//				mMutex.lock();
//				mPics.emplace_back(std::move(pPic));
//				mMutex.unlock();
//				mCondition.notify_all();
//				if (!mWritThread) {
//					mWritThread = std::make_shared<std::thread>([this]() {
//						while (!mbExit) {
//							std::unique_lock<std::mutex> l(mMutex);
//							if (!mPics.empty()) {
//								auto it = mPics.begin();
//								auto pPic = *it;
//								mPics.erase(it);
//								l.unlock();
//								cv::cvtColor(*pPic, *pPic, CV_BGR2RGB);
//								cv::flip(*pPic, *pPic, 0);
//								mpVideo->write(*pPic);
//								cv::waitKey(5);
//							}
//							else {
//								mCondition.wait(l);
//							}
//						}
//					});
//				}
//			}
//			else {
//				LOGE("ERROR to map pbo to memory");
//			}
//			glUnmapBuffer(GL_PIXEL_PACK_BUFFER); //????????????
//			std::swap(mReadIndex, mWriteIndex);
//		}
//		else {
//			mReadIndex = 0;
//			mWriteIndex = 1;
//		}
//	}
//}
