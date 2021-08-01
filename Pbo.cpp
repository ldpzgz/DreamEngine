#include "Pbo.h"
#include <GLES3/gl3.h>
#include "Log.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/types_c.h"

using namespace std::chrono;
using namespace cv;

int Pbo::gPerfectFormat = GL_RGB;
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

Pbo::Pbo(const std::string& pathToSave, int width, int height):
	mWidth(width),
	mHeight(height),
	mFormat(GL_RGB),
	mType(GL_UNSIGNED_BYTE)
{
	getBytesPerPixel(mFormat, mType, mBytesPerPixel);
	glGenBuffers(2, mPbo);
	glBindBuffer(GL_PIXEL_PACK_BUFFER, mPbo[0]);
	glBufferData(GL_PIXEL_PACK_BUFFER, mWidth*mHeight*mBytesPerPixel, 0, GL_DYNAMIC_READ);

	glBindBuffer(GL_PIXEL_PACK_BUFFER, mPbo[1]);
	glBufferData(GL_PIXEL_PACK_BUFFER, mWidth*mHeight*mBytesPerPixel, 0, GL_DYNAMIC_READ);

	mpVideo = std::make_shared<VideoWriter>(pathToSave, VideoWriter::fourcc('M', 'J', 'P', 'G'), mFrameRate, Size(width, height));//
	if (mpVideo->isOpened()) {
		LOGD("success open videowriter");
	}

	mpData = std::make_shared<std::vector<unsigned char>>(mWidth*mHeight*mBytesPerPixel);
	//mPreTime = time_point_cast<milliseconds>(steady_clock::now());
}

Pbo::~Pbo() {
	if (mPbo[0] >= 0) {
		glDeleteBuffers(1, &mPbo[0]);
	}
	if (mPbo[1] >= 0) {
		glDeleteBuffers(1, &mPbo[1]);
	}

	if (mWritThread) {
		mbExit = true;
		mCondition.notify_all();
		mWritThread->join();
	}
}
TimeCounter<std::chrono::microseconds> tempTimeCounter;
void Pbo::pullColorBufferToMemory(int x,int y,int width,int height) {
	//auto curTime = time_point_cast<milliseconds>(steady_clock::now());
	if ( mTimeCounter.elpase(1000.0f / mFrameRate).first) {
		std::cout << "Pbo::pullColorBufferToMemory " << nowTime()<<std::endl;
		if (mWriteIndex < 0) {
			mWriteIndex = 0;
		}
		glBindBuffer(GL_PIXEL_PACK_BUFFER, mPbo[mWriteIndex]);
		glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE,nullptr);
		if (mReadIndex >= 0) {
			glBindBuffer(GL_PIXEL_PACK_BUFFER, mPbo[mReadIndex]); //指定pbo
			tempTimeCounter.reset();
			void*   data = glMapBufferRange(GL_PIXEL_PACK_BUFFER,0, mWidth*mHeight*mBytesPerPixel,GL_MAP_READ_BIT); //做一个map映射把PBO的数据和内存的data指针进行关联
			std::cout << "glMapBufferRange cost time " << tempTimeCounter.elapseFromeReset() << " micro" << std::endl;
			if (data) {
				//save(WIDTH, HEIGHT, (char*)data, WIDTH * HEIGHT * 4);
				auto pPic = std::make_shared<cv::Mat>();
				pPic->create(mHeight, mWidth, CV_8UC3);
				memcpy((void*)pPic->data, data, mWidth*mHeight*mBytesPerPixel);
				mMutex.lock();
				mPics.emplace_back(std::move(pPic));
				mMutex.unlock();
				mCondition.notify_all();
				if (!mWritThread) {
					mWritThread = std::make_shared<std::thread>([this]() {
						while (!mbExit) {
							std::unique_lock<std::mutex> l(mMutex);
							if (!mPics.empty()) {
								auto it = mPics.begin();
								auto pPic = *it;
								mPics.erase(it);
								l.unlock();
								cv::cvtColor(*pPic, *pPic, CV_BGR2RGB);
								cv::flip(*pPic, *pPic, 0);
								mpVideo->write(*pPic);
								cv::waitKey(5);
							}
							else {
								mCondition.wait(l);
							}
						}
					});
				}
			}
			else {
				LOGE("ERROR to map pbo to memory");
			}
			glUnmapBuffer(GL_PIXEL_PACK_BUFFER); //取消映射
			std::swap(mReadIndex, mWriteIndex);
		}
		else {
			mReadIndex = 0;
			mWriteIndex = 1;
		}

		glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
		tempTimeCounter.reset();
		glReadPixels(x, y, width, height, GL_RGB, GL_UNSIGNED_BYTE, (void*)mpData->data());
		std::cout << "glReadPixels cost time " << tempTimeCounter.elapseFromeReset() << " micro" << std::endl;
	}
}
