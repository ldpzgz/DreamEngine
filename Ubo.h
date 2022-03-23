#pragma once

#ifdef _GLES3
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>
#else
#include <glad/glad.h>
#endif
#include<unordered_map>
#include<string>
/*
* 整个程序创建一个ubo就可以了，ubo的不同部分可以绑定到bindpoint
*/

class Ubo {
public:
	static constexpr int totalUboSize = 2048;
	Ubo();
	~Ubo();
	void bind(const char* ubName, int sizeInByte, int bindPoint) noexcept;
	void update(const std::string& ubName, void* pdata, int sizeInByte) noexcept;
	//获取系统的最大bindPoint个数，最少有24个bindpoint
	static int getMaxBindPoint();
	inline static Ubo& getInstance() {
		static Ubo gUbo;
		if (gUbo.mId == 0) {
			glGenBuffers(1, &gUbo.mId);
			glBindBuffer(GL_UNIFORM_BUFFER, gUbo.mId);
			glBufferData(GL_UNIFORM_BUFFER, totalUboSize, NULL, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_UNIFORM_BUFFER, 0);
		}
		return gUbo;
	}
private:
	unsigned int mId{0};
	int mCurSize{ 0 };
	//pair保存的是offset，size
	std::unordered_map<std::string, std::pair<int, int>> mUniformBlocks;
};