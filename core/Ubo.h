#pragma once

#ifdef _GLES3
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>
#else
#include <glad/glad.h>
#endif
#include <unordered_map>
#include <string>
/*
* ubo的bindpoint 是数量有限的资源，最少支持24个bindpoint，
* 但是ubo只是一块显存，ubo可以分配多个，
*/

class Ubo {
public:
	static constexpr int totalUboSize = 4096+8192;
	Ubo();
	~Ubo();
	//
	void bind(const char* ubName, int sizeInByte, int bindPoint) noexcept;
	void update(const std::string& ubName, void* pdata, int sizeInByte, int offset = 0) noexcept;
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