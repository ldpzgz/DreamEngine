#pragma once

#include <string>
#include <memory>
#include <thread>
#include <chrono>
#include <list>
#ifdef _GLES3
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>
#else
#include <glad/glad.h>
#endif
#include "Utils.h"
#include "Texture.h"
//pbo 就是一块显存，
//经过测试证明pbo是有用的，pbo本身是使用dma将GL_READ_BUFFER(使用glReadBuffer函数指定)中的数据读取到pbo，
//再使用glMapBufferRange，把pbo显存映射到主内存，这个过程在我的笔记本上大概就100微秒
//而如果直接使用glReadPixel将color buffer中的数据读取到内存，耗时大概是2000多微秒

/*
glReadBuffer:
	void glReadBuffer(	GLenum src); Accepted values are GL_BACK, GL_NONE, and GL_COLOR_ATTACHMENTi

	这个函数的作用是为这四个函数指定source：glReadPixels, , glCopyTexImage2D, glCopyTexSubImage2D, and glCopyTexSubImage3D
		
glReadPixel:如果当前绑定了pbo，则将指定的source（默认是缺省的back color buffer) 通过dma copy到pbo

高效复制纹理数据到内存的思路：
1、	new 一个fbo，绑定 目标纹理，作为渲染目标
2、	调用glReadBuffer指定source为GL_COLOR_ATTACHMENTi
3、	new 一个pbo，并绑定
4、	调用glReadPixel将纹理读取到pbo，
5、	调用glMapBufferRange,指定target为GL_PIXEL_PACK_BUFFER，将当前绑定的pbo映射到内存。
*/

/*
首先创建一个pbo，需要指定fomat，type，
指定要copy的colorBuffer，
然后bind pbo，调用glReadPixel将colorBuffer读取到pbo，这个过程可能会发生数据类型的转换
把colorbuffer中的internalformat指定的格式转换为pbo的format type指定的格式。
但是这个readpixel的过程是dma的，所以最好colorBuffer中的格式与pbo中的格式一样。
*/

/*
* 这个类经常与fbo联合使用，fbo绑定纹理到color attachment，
* 然后创建一个pbo，将colorbuffer pull到pbo，然后map pbo到内存。
* 这个过程中，不是什么纹理格式都能绑定到fbo的,请参考opengles3.0编程向导
* 还有pbo只是一块内存区域，本身没有格式。
* fbo可以绑定internalFormat为GL_RED等他支持的格式，但是
* pull到pbo的是GL_RGBA,最后取出来的数据有4个通道！！！
*/
class Pbo {
public:
	Pbo() = default;
	~Pbo();

	/*
	*  format:glGet and GL_IMPLEMENTATION_COLOR_READ_FORMAT
	*		and GL_RED, GL_RED_INTEGER, GL_RG, GL_RG_INTEGER, GL_RGB, 
		GL_RGB_INTEGER, GL_RGBA, GL_RGBA_INTEGER, GL_LUMINANCE_ALPHA, 
		GL_LUMINANCE, and GL_ALPHA is supported
		但是经过测试opengles3只支持GL_RGBA

		type: GL_UNSIGNED_BYTE, GL_UNSIGNED_INT, GL_INT, or GL_FLOAT
			glGet and GL_IMPLEMENTATION_COLOR_READ_TYPE is supported
	*/
	void initPbo(int width,int height,unsigned int format= GL_RGBA,unsigned int type= GL_UNSIGNED_BYTE);
	//主要用于截屏保存到图像文件
	//colorBuffer: GL_BACK, GL_NONE, and GL_COLOR_ATTACHMENTi
	void saveToFile(unsigned int buffer, const std::string& pathToSave);
	/*
	* 只能将可以添加到fbo的纹理保存到文件，
	* opengles3不支持浮点纹理
	*/
	void saveToFile(const std::shared_ptr<Texture>& pTex, const std::string& pathToSave);

	/*
	* 只能将可以添加到fbo的纹理拉取到内存，
	* opengles3不支持浮点纹理
	*/
	void pullToMem(const std::shared_ptr<Texture>& pTex, std::function<void(Pbo* pbo, void*)> func);
	void pullToMem(GLuint colorBuffer, std::function<void(Pbo* pbo, void*)> func);

	int getBpp() {
		return mBytesPerPixel;
	}
	int getWidth() {
		return mWidth;
	}
	int getHeight() {
		return mHeight;
	}
	unsigned int getFormat() {
		return mFormat;
	}
	unsigned int getType() {
		return mType;
	}
private:
	unsigned int mPbo{ 0 };
	
	int mWidth{ 0 };
	int mHeight{ 0 };
	unsigned int mFormat{ GL_RGBA };
	unsigned int mType{ GL_UNSIGNED_BYTE };
	unsigned int mBytesPerPixel{ 4 };
	
	void getBytesPerPixel(int readFormat, int readType, unsigned int& bytesPerPixel);
	
	//保存视频相关的数据
	/*int mIndex{ 0 };
	std::shared_ptr<cv::VideoWriter> mpVideo;
	std::shared_ptr<std::thread> mWritThread;
	std::mutex mMutex;
	std::condition_variable mCondition;
	std::list<std::shared_ptr<cv::Mat>> mPics;*/

	/*using TimePoint = std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds>;
	TimePoint mPreTime;*/
	/*Timer<std::chrono::milliseconds> mTimeCounter;
	int mFrameRate{ 25 };
	int mReadIndex{ -1 };
	int mWriteIndex{ -1 };
	
	bool mbExit{ false };
	std::shared_ptr<std::vector<unsigned char>> mpData;*/

	static int gPerfectFormat;
	static int gPerfectType;
	static void getPerfectParam();
};
