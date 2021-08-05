#ifndef _PBO_H_
#define _PBO_H_
#include <string>
#include <memory>
#include <thread>
#include <chrono>
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp> 
#include <list>
#include "Utils.h"

//经过测试证明pbo是有用的，pbo本身是使用dma将color buffer中的数据读取到pbo，
//再使用glMapBufferRange，把pbo显存映射到内存，这个过程在我的笔记本上大概就100微秒
//而如果直接使用glReadPixel将color buffer中的数据读取到内存，耗时大概是2000多微秒

/*
glReadBuffer:
	void glReadBuffer(	GLenum src); Accepted values are GL_BACK, GL_NONE, and GL_COLOR_ATTACHMENTi

	这个函数的作用是为这四个函数指定source：glReadPixels, , glCopyTexImage2D, glCopyTexSubImage2D, and glCopyTexSubImage3D
		
glReadPixel:如果当前绑定了pbo，则将指定的source（默认是缺省的back color buffer) 通过dma赋值到pbo

高效复制纹理数据到内存的思路：
1、	new 一个fbo，绑定 目标纹理，作为渲染目标
2、	调用glReadBuffer指定source为GL_COLOR_ATTACHMENTi
3、	new 一个pbo，并绑定
4、	调用glReadPixel将纹理读取到pbo，
5、	调用glMapBufferRange,指定target为GL_PIXEL_PACK_BUFFER，将当前绑定的pbo映射到内存。
*/

/*
下面这个pbo，实现了将缺省的colorbuffer 以每秒25帧的速度录制到一个视频里面
开了两个pbo，快速的将color buffer复制到pbo，然后将pbo map到内存，复制数据 丢给录制线程。
*/
class Pbo {
public:
	Pbo(const std::string& pathToSave, int width, int height);
	~Pbo();
	void pullColorBufferToMemory(int x, int y, int width, int height);
	static void getPerfectParam();
private:
	unsigned int mPbo[2]{ 0,0 };
	int mIndex{ 0 };
	int mWidth{ 0 };
	int mHeight{ 0 };
	int mFormat;
	int mType;
	static int gPerfectFormat;
	static int gPerfectType;
	void getBytesPerPixel(int readFormat, int readType, unsigned int& bytesPerPixel);

	//保存视频相关的数据
	std::shared_ptr<cv::VideoWriter> mpVideo;
	std::shared_ptr<std::thread> mWritThread;
	std::mutex mMutex;
	std::condition_variable mCondition;
	std::list<std::shared_ptr<cv::Mat>> mPics;

	/*using TimePoint = std::chrono::time_point<std::chrono::steady_clock, std::chrono::milliseconds>;
	TimePoint mPreTime;*/
	TimeCounter<std::chrono::milliseconds> mTimeCounter;
	int mFrameRate{ 25 };
	int mReadIndex{ -1 };
	int mWriteIndex{ -1 };
	unsigned int mBytesPerPixel{ 0 };
	bool mbExit{ false };
	std::shared_ptr<std::vector<unsigned char>> mpData;
};

#endif