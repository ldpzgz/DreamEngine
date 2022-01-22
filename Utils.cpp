#include "Utils.h"
#include "Texture.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/types_c.h"
using namespace std;
namespace Utils {
	/*std::shared_ptr<Texture> loadImageFromFile(const std::string& path) {
		shared_ptr<Texture> retTex;
		cv::Mat inImage;
		int internalFormat = GL_RGB;
		inImage = cv::imread(path);

		if (!inImage.empty()) {
			int width = inImage.cols;
			int height = inImage.rows;
			int channels = inImage.channels();
			cout << "image channels " << channels << " image width " << width << " height  " << height << endl;

			if (channels == 3) {
				cv::cvtColor(inImage, inImage, CV_RGB2BGR);
				internalFormat = GL_RGB;
			}
			else if (channels == 4) {
				cv::cvtColor(inImage, inImage, CV_RGBA2BGRA);
				internalFormat = GL_RGBA;
			}
			else if (channels == 1) {
				internalFormat = GL_LUMINANCE;
			}
			else {
				return retTex;
			}

			cv::flip(inImage, inImage, 0);
			unsigned char* pdata = inImage.data;


			retTex = make_shared<Texture>();
			if (!retTex->create2DMap(width, height, pdata, internalFormat,GL_UNSIGNED_BYTE,1,true)) {
				retTex.reset();
			}
		}
		return retTex;
	}*/

	std::string nowTime()
	{
		std::stringstream ss;
		auto now = chrono::system_clock::now();
		auto nowTime = chrono::system_clock::to_time_t(now);
		ss << std::put_time(std::localtime(&nowTime), "%Y-%m-%d %X");
		auto seconds = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch());
		auto milliSeconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch());
		ss << "." << std::setfill('0') << std::setw(3) << (milliSeconds - seconds).count();
		return ss.str();
	}

	std::string getFileNameWithPath(const std::string& path) {
		auto endpos = path.find_last_of(".");
		if (endpos > 0 && endpos != string::npos) {
			return path.substr(0, endpos);
		}
		else {
			return path;
		}
	}

	string getFileName(const string& path) {
		auto startpos = path.find_last_of("/\\");
		if (startpos != string::npos) {
			auto endpos = path.find_last_of(".");
			if (endpos > 0 && endpos != string::npos) {
				return path.substr(startpos+1, endpos-startpos-1);
			}
			else {
				return path.substr(startpos+1);
			}
		}
		else {
			auto endpos = path.find_last_of(".");
			if (endpos > 0 && endpos != string::npos) {
				return path.substr(0, endpos);
			}
			else {
				return path;
			}
		}
	}

	string getFileSuffix(const string& path) {
		string temp;
		auto startpos = path.find_last_of(".");
		if (startpos != string::npos) {
			return path.substr(startpos+1);
		}
		return temp;
	}

	void forEachFile(const std::string pathName, const std::string suffix, std::function<void(const std::string& path)> func) {
		path upPath(pathName);
		if (!exists(upPath)) {
			return;
		}
		if (is_directory(upPath)) {
			//是目录
			std::filesystem::directory_iterator list(upPath);
			for (auto& it : list) {
				auto& filePath = it.path();
				if (is_regular_file(filePath)) {
					//是文件
					auto filename = filePath.string();
					if (getFileSuffix(filename) == suffix) {
						func(filename);
					}
				}
			}
		}
	}
}
