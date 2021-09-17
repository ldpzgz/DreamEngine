#ifndef _UTILS_H_
#define _UTILS_H_

#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <ctime>
#include "Log.h"
#ifndef Texture
class Texture;
#endif

namespace Utils {
	std::shared_ptr<Texture> loadImageFromFile(const std::string& path);

	std::string nowTime();

	std::string getFileName(const std::string& path);
	std::string getFileSuffix(const std::string& path);
}

/*
T	std::chrono::milliseconds,std::chrono::microseconds,..��ʱ�䵥λ��
*/
template<typename T>
class TimeCounter {
public:
	using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock, T>;
	
	/*
	���ܣ����ε���elapse���ϴε���elapse��ʱ����Ƿ񳬹���deltaCount��
	deltaCount��ʱ�䵥λ��T
	����ֵ�����������deltaCount������<true,�������ϴε�ʱ����>,����<false,�������ϴε�ʱ����>
	*/
	std::pair<bool,int64_t> elapse(int deltaCount) {
		if (!mbStart) {
			start();
			return std::make_pair<bool, int64_t>(false,0ll);
		}
		else {
			auto curTime = std::chrono::time_point_cast<T>(std::chrono::high_resolution_clock::now());
			auto count = (curTime - mPreTime).count();
			if (count >= deltaCount) {
				mPreTime = curTime;
				return std::make_pair<bool, int64_t>(true, std::move(static_cast<int64_t>(count)));
			}
			else {
				return std::make_pair<bool, int64_t>(false, std::move(static_cast<int64_t>(count)));
			}
		}
	}
	//��start���������˶���ʱ�䣬�����û�е��ù�start������0
	int64_t elapse() {
		if (mbStart) {
			if (mbPause) {
				return mElapseWhenPause;
			}
			else {
				auto curTime = std::chrono::time_point_cast<T>(std::chrono::high_resolution_clock::now());
				auto count = (curTime - mPreTime).count();
				return count;
			}
		}
		else {
			LOGD("warning the timercount is not start");
			return 0;
		}
	}

	void start() {
		mPreTime = std::chrono::time_point_cast<T>(std::chrono::high_resolution_clock::now());
		mbStart = true;
	}

	void stop() {
		mbStart = false;
	}

	void pause() {
		if (mbStart) {
			auto curTime = std::chrono::time_point_cast<T>(std::chrono::high_resolution_clock::now());
			mElapseWhenPause = (curTime - mPreTime).count();
			mbPause = true;
		}
	}

	void resume() {
		if (mbPause) {
			mPreTime = std::chrono::time_point_cast<T>(std::chrono::high_resolution_clock::now());
			mPreTime -= T(mElapseWhenPause);
			mbPause = false;
		}
	}
private:
	TimePoint mPreTime;
	int64_t mElapseWhenPause{ 0 };
	bool mbStart{ false };
	bool mbPause{ false };
};

using TimeCounterMil = TimeCounter<std::chrono::milliseconds>;
using TimeCounterMic = TimeCounter<std::chrono::microseconds>;

namespace UtfConvert
{
	/*
	UTF-32 to UTF-8
	*/

	inline static size_t utf(char32_t src, unsigned char* des)
	{
		if (src == 0) return 0;

		static const unsigned char PREFIX[] = { 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC };
		static const char32_t CODE_UP[] =
		{
			0x80,           // U+00000000 - U+0000007F  
			0x800,          // U+00000080 - U+000007FF  
			0x10000,        // U+00000800 - U+0000FFFF  
			0x200000,       // U+00010000 - U+001FFFFF  
			0x4000000,      // U+00200000 - U+03FFFFFF  
			0x80000000      // U+04000000 - U+7FFFFFFF  
		};

		size_t i, len = sizeof(CODE_UP) / sizeof(char32_t);
		for (i = 0; i < len; ++i)
			if (src < CODE_UP[i]) break;

		if (i == len) return 0; // the src is invalid  

		len = i + 1;
		if (des)
		{
			for (; i > 0; --i)
			{
				des[i] = static_cast<unsigned char>((src & 0x3F) | 0x80);
				src >>= 6;
			}
			des[0] = static_cast<unsigned char>(src | PREFIX[len - 1]);
		}
		return len;
	}

	/*
	UTF-8 to UTF-32
	*/

	inline static size_t utf(const unsigned char* src, char32_t& des)
	{
		if (!src || (*src) == 0) return 0;

		unsigned char b = *(src++);

		if (b < 0x80)
		{
			des = b;
			return 1;
		}

		if (b < 0xC0 || b > 0xFD) return 0; // the src is invalid  

		size_t len;

		if (b < 0xE0)
		{
			des = b & 0x1F;
			len = 2;
		}
		else
			if (b < 0xF0)
			{
				des = b & 0x0F;
				len = 3;
			}
			else
				if (b < 0xF8)
				{
					des = b & 0x07;
					len = 4;
				}
				else
					if (b < 0xFC)
					{
						des = b & 0x03;
						len = 5;
					}
					else
					{
						des = b & 0x01;
						len = 6;
					}

		size_t i = 1;
		for (; i < len; ++i)
		{
			b = *(src++);
			if (b < 0x80 || b > 0xBF) return 0; // the src is invalid  
			des = (des << 6) + (b & 0x3F);
		}
		return len;
	}

	/*
	UTF-32 to UTF-16
	*/

	inline static size_t utf(char32_t src, char16_t* des)
	{
		if (src == 0) return 0;

		if (src <= 0xFFFF)
		{
			if (des) (*des) = static_cast<char16_t>(src);
			return 1;
		}
		else
			if (src <= 0xEFFFF)
			{
				if (des)
				{
					des[0] = static_cast<char16_t>(0xD800 + (src >> 10) - 0x40);  // high  
					des[1] = static_cast<char16_t>(0xDC00 + (src & 0x03FF));      // low  
				}
				return 2;
			}
		return 0;
	}

	/*
	UTF-16 to UTF-32
	*/

	inline static size_t utf(const char16_t* src, char32_t& des)
	{
		if (!src || (*src) == 0) return 0;

		char16_t w1 = src[0];
		if (w1 >= 0xD800 && w1 <= 0xDFFF)
		{
			if (w1 < 0xDC00)
			{
				char16_t w2 = src[1];
				if (w2 >= 0xDC00 && w2 <= 0xDFFF)
				{
					des = (w2 & 0x03FF) + (((w1 & 0x03FF) + 0x40) << 10);
					return 2;
				}
			}
			return 0; // the src is invalid  
		}
		else
		{
			des = w1;
			return 1;
		}
	}

	/*
	UTF-16 to UTF-8
	*/

	inline static size_t utf(char16_t src, unsigned char* des)
	{
		// make utf-16 to utf-32  
		char32_t tmp;
		if (utf(&src, tmp) != 1) return 0;
		// make utf-32 to utf-8  
		return utf(tmp, des);
	}

	/*
	UTF-8 to UTF-16
	*/

	inline static size_t utf(const unsigned char* src, char16_t& des)
	{
		// make utf-8 to utf-32  
		char32_t tmp;
		size_t len = utf(src, tmp);
		if (len == 0) return 0;
		// make utf-32 to utf-16  
		if (utf(tmp, &des) != 1) return 0;
		return len;
	}
}



#endif
