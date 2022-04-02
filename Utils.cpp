#include "Utils.h"
#include "Texture.h"
#include <sstream>
//#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/imgproc/types_c.h"
using namespace std;

void checkglerror()
{
	/*
	GL_INVALID_ENUM, 0x0500-----1280
	GL_INVALID_VALUE, 0x0501
	GL_INVALID_OPERATION, 0x0502
	GL_STACK_OVERFLOW, 0x0503
	GL_STACK_UNDERFLOW, 0x0504
	GL_OUT_OF_MEMORY, 0x0505
	GL_INVALID_FRAMEBUFFER_OPERATION, 0x0506
	GL_CONTEXT_LOST, 0x0507 (with OpenGL 4.5 or ARB_KHR_robustness)
	GL_TABLE_TOO_LARGE1, 0x8031
	*/
	int error = 0;
	for (error = glGetError(); error != GL_NO_ERROR; error = glGetError())
	{
		LOGD("gl error is %d\n", error);
	}

}

namespace Utils {

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

	string_view getFileName(const string_view path) {
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

	string_view getFileSuffix(const string_view path) {
		auto startpos = path.find_last_of(".");
		if (startpos != string::npos) {
			return path.substr(startpos+1);
		}
		return string_view();
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

	bool splitKeyValue(const std::string_view content, std::string& key, std::string& value) {
		auto pos = content.find(':');
		if (pos != string::npos) {
			auto keyEndPos = content.find_last_not_of("\20",pos-1);
			if (keyEndPos != std::string::npos) {
				keyEndPos += 1;
			}
			else {
				keyEndPos = pos;
			}
			key = content.substr(0, keyEndPos);
			auto valueStartPos = content.find_first_not_of("\20", pos + 1);
			if (valueStartPos == std::string::npos) {
				valueStartPos = pos;
			}
			value = content.substr(valueStartPos);
			return true;
		}
		else {
			key = content;
			value.clear();
			return false;
		}
	}

	int splitStr(const std::string_view str, const std::string_view separator, std::vector<std::string_view>& result) {
		int count = 0;
		if (!str.empty() && !separator.empty()) {
			size_t startPos = str.find_first_not_of(separator);
			size_t endPos = 0;
			size_t strLen = str.length();
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

	bool parseItem(const string& value, std::function<bool(const std::string&, const std::string&)> func) {
		std::string::size_type pos[3]{ 0,0,0 };
		std::string::size_type startPos = 0;
		do {
			bool findKV = false;
			auto tpos = value.find_first_of("={", startPos);
			std::string realKey;
			std::string realValue;
			if (tpos != string::npos) {
				if (value[tpos] == '=') {
					//查找x=v
					findKV = findkeyValue(value, "=", "\r\n", startPos, pos, realKey, realValue);//"\x20\r\n\t"
				}
				else {
					//查找x{v}
					findKV = findkeyValue(value, "{", "}", startPos, pos, realKey, realValue);
				}
				if (findKV) {
					if (!realKey.empty() && !realValue.empty()) {
						if (!func(realKey, realValue)) {
							LOGE("parseItem cannot embrace item to container when parse config file");
						}
					}
					else {
						LOGD("find a empty value!!!");
					}
				}
				else {
					//语法错误
					LOGE("parseItem syntactic error");
					return false;
				}
			}
			else {
				break;
			}
		} while (true);
		return true;
	}

	bool parseItem(const string& value, std::vector<std::pair<std::string, std::string>>& vec) {
		return parseItem(value, [&vec](const std::string& key, const std::string& value)->bool{
			vec.emplace_back(key, value);
			return true;
			});
	}

	bool parseItem(const string& value, std::unordered_map<std::string,std::string>& umap) {
		return parseItem(value, [&umap](const std::string& key, const std::string& value)->bool {
			return umap.emplace(key, value).second;
			});
	}

	bool parseItem(const string& value, std::multimap<std::string, std::string>& umap) {
		return parseItem(value, [&umap](const std::string& key, const std::string& value)->bool {
			umap.emplace(key, value);
			return true;
			});
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
	bool findkeyValue(const string& str, const string& mid, const string& end, std::string::size_type& startPos, std::string::size_type* pos,std::string& key, std::string& value) {
		const std::string emptyChar{ "\x20\r\n\t" };
		auto len = str.length();
		if (startPos >= len) {
			return false;
		}
		if (str.empty()||mid.empty()||end.empty() ) {
			LOGE("utils findkeyValue func's string param is empty ");
			return false;
		}
		
		int countOfStart = 0;
		int countOfEnd = 0;
		pos[0] = pos[1] = pos[2] = -1;
		std::string::size_type nextPos = startPos;
		pos[0] = str.find_first_not_of(mid + end + emptyChar, nextPos);

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
			auto keyEnd = str.find_last_not_of(emptyChar, pos[1]-1);
			if (keyEnd != string::npos) {
				key = str.substr(pos[0], keyEnd - pos[0] + 1);

				auto valueStart = str.find_first_not_of(emptyChar, pos[1] + 1);
				auto valueEnd = str.find_last_not_of(emptyChar, pos[2] - 1);

				if (valueStart != string::npos && valueEnd != string::npos && valueStart <= valueEnd) {
					value = str.substr(valueStart, valueEnd - valueStart + 1);
				}
			}
			startPos = pos[2] + 1;
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
