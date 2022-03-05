#include "Utils.h"
#include "Texture.h"
#include <sstream>
//#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/imgproc/types_c.h"
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

	void splitKeyAndName(const string& key, string& realKey, string& keyName) {
		auto pos = key.find(':');
		if (pos != string::npos) {
			realKey = key.substr(0, pos);
			keyName = key.substr(pos + 1);
		}
		else {
			realKey = key;
		}
	}

	int splitStr(const std::string& str, const std::string& separator, std::vector<std::string>& result) {
		int count = 0;
		if (!str.empty() && !separator.empty()) {
			size_t startPos = str.find_first_not_of(separator);
			size_t endPos = 0;
			int strLen = str.length();
			do {
				auto tempPos = str.find_first_of(separator,startPos);
				if (tempPos != std::string::npos) {
					++count;
					result.emplace_back(str.substr(startPos, tempPos - startPos));
					startPos = str.find_first_not_of(separator, tempPos + 1);
					if (startPos != std::string::npos) {
						continue;
					}
					else {
						break;
					}
				}
				else {
					if (startPos < strLen) {
						++count;
						result.emplace_back(str.substr(startPos));
					}
					break;
				}
			} while (true);
		}
		return count;
	}

	bool parseItem(const string& value, std::unordered_map<std::string,std::string>& umap) {
		std::string::size_type pos[3]{ 0,0,0 };
		std::string::size_type startPos = 0;
		do {
			bool findKV = false;
			auto tpos = value.find_first_of("={", startPos);
			if (tpos != string::npos) {
				if (value[tpos] == '=') {
					findKV = findkeyValue(value, "=", "\r\n", startPos, pos);//"\x20\r\n\t"
				}
				else {
					findKV = findkeyValue(value, "{", "}", startPos, pos);
				}
			}
			if (findKV) {
				auto temp = value.substr(pos[0], pos[1] - pos[0]);
				auto keyendpos = temp.find_last_not_of("\x20\t");
				if (keyendpos == string::npos) {
					LOGE("find a empty key!!!");
					return false;
				}
				auto realKey = temp.substr(0, keyendpos + 1);
				auto tempValue = value.substr(pos[1] + 1, pos[2] - pos[1] - 1);
				auto valueStartPos = tempValue.find_first_not_of("\x20\r\n\t", 0);
				if (valueStartPos != string::npos) {
					auto valueEndPos = tempValue.find_last_not_of("\x20\r\n\t");//becarful ,see http://www.cplusplus.com/reference/string/string/find_last_not_of/
					auto realValue = tempValue.substr(valueStartPos, valueEndPos - valueStartPos + 1);
					if (!umap.try_emplace(realKey, std::move(realValue)).second) {
						LOGE("failed to insert material key %s into unordermap", realKey.c_str());
					}
				}
				else {
					LOGE("find a empty value!!!");
					return false;
				}
				startPos = pos[2] + 1;
			}
			else {
				break;
			}
		} while (true);
		return true;
	}

	/*
	在字符串str里面从startPos开始，查找第一个形如：key{value}
	str		where to find key-value;
	mid		the str between key and value;
	end		the end of value;
	startPos from which pos to start serach in str;
	pos		is int[3]，pos[0],the start pos of the key,
	pos[1]	the pos of mid
	pos[2]	the pos of end
	*/
	bool findkeyValue(const string& str, const string& mid, const string& end, std::string::size_type startPos, std::string::size_type* pos) {
		auto len = str.length();
		if (startPos >= len) {
			return false;
		}
		int countOfStart = 0;
		int countOfEnd = 0;
		std::string::size_type nextPos = startPos;
		pos[0] = pos[1] = pos[2] = -1;
		pos[0] = str.find_first_not_of(mid + end + "\x20\r\n\t", nextPos);

		if (pos[0] == string::npos) {
			return false;
		}
		nextPos = pos[0] + 1;
		do {
			auto tempPos = str.find_first_of(mid + end, nextPos);
			if (tempPos != string::npos) {
				if (mid.find(str[tempPos]) != string::npos) {
					++countOfStart;
					if (pos[1] == -1) {
						pos[1] = tempPos;
					}
				}
				else if (end.find(str[tempPos]) != string::npos) {
					pos[2] = tempPos;
					++countOfEnd;
					if (countOfEnd > countOfStart) {
						//语法错误退出
						break;
					}
				}
				nextPos = tempPos + 1;
			}
			else {
				if (pos[1] > 0 && pos[1] < str.size() - 1)
				{
					++countOfEnd;
					pos[2] = str.size();
				}
				break;
			}

		} while (countOfStart != countOfEnd);

		if (countOfStart == countOfEnd && countOfStart > 0 && pos[0] < pos[1] && pos[1] < pos[2]) {
			return true;
		}
		else {
			if (countOfStart > 0 || countOfEnd > 0) {
				LOGE("findkeyValue ,syntax error");
			}
			return false;
		}
	}
}
