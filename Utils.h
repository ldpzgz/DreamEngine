#ifndef _TEXTUREUTILS_H_
#define _TEXTUREUTILS_H_

#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#include <chrono>
#include <iostream>
#include<iomanip>
#include<ctime>
#ifndef Texture
class Texture;
#endif
std::shared_ptr<Texture> loadImageFromFile(const std::string& path);

std::string nowTime();

template<typename T>
class TimeCounter {
public:
	using TimePoint = std::chrono::time_point<std::chrono::high_resolution_clock, T>;
	
	std::pair<bool,int64_t> elpase(int deltaCount) {
		if (!mbInit) {
			mPreTime = time_point_cast<T>(std::chrono::high_resolution_clock::now());
			mbInit = true;
			return std::make_pair<bool, int64_t>(false,0ll);
		}
		else {
			auto curTime = time_point_cast<T>(std::chrono::high_resolution_clock::now());
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

	int64_t elapseFromeReset() {
		auto curTime = time_point_cast<T>(std::chrono::high_resolution_clock::now());
		auto count = (curTime - mPreTime).count();
		return count;
	}

	void reset() {
		mPreTime = time_point_cast<T>(std::chrono::high_resolution_clock::now());
		mbInit = true;
	}
private:
	TimePoint mPreTime;
	bool mbInit{ false };
};
#endif
