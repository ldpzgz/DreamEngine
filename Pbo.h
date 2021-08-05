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

//��������֤��pbo�����õģ�pbo������ʹ��dma��color buffer�е����ݶ�ȡ��pbo��
//��ʹ��glMapBufferRange����pbo�Դ�ӳ�䵽�ڴ棬����������ҵıʼǱ��ϴ�ž�100΢��
//�����ֱ��ʹ��glReadPixel��color buffer�е����ݶ�ȡ���ڴ棬��ʱ�����2000��΢��

/*
glReadBuffer:
	void glReadBuffer(	GLenum src); Accepted values are GL_BACK, GL_NONE, and GL_COLOR_ATTACHMENTi

	���������������Ϊ���ĸ�����ָ��source��glReadPixels, , glCopyTexImage2D, glCopyTexSubImage2D, and glCopyTexSubImage3D
		
glReadPixel:�����ǰ����pbo����ָ����source��Ĭ����ȱʡ��back color buffer) ͨ��dma��ֵ��pbo

��Ч�����������ݵ��ڴ��˼·��
1��	new һ��fbo���� Ŀ����������Ϊ��ȾĿ��
2��	����glReadBufferָ��sourceΪGL_COLOR_ATTACHMENTi
3��	new һ��pbo������
4��	����glReadPixel��������ȡ��pbo��
5��	����glMapBufferRange,ָ��targetΪGL_PIXEL_PACK_BUFFER������ǰ�󶨵�pboӳ�䵽�ڴ档
*/

/*
�������pbo��ʵ���˽�ȱʡ��colorbuffer ��ÿ��25֡���ٶ�¼�Ƶ�һ����Ƶ����
��������pbo�����ٵĽ�color buffer���Ƶ�pbo��Ȼ��pbo map���ڴ棬�������� ����¼���̡߳�
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

	//������Ƶ��ص�����
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