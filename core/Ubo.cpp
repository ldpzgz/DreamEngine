#include "Ubo.h"
#include "Log.h"

/*
* shader里面出现的uniform block如下
//16*4*3=192字节，3个mat4，一个mat4占64字节
layout(std140) uniform Matrixes
{
	mat4 projMat;
	mat4 viewMat;
	mat4 shadowMat;
};

layout(std140) uniform Lights
{
	vec3 lightPositions[15];
	vec3 lightColors[15];
	int lightCounts;
}

//这个占了一个vec4 16个字节
layout(std140) uniform ScreenWH
{
	float screenWidth;
	float screenHeight;
};

//这个占了一个vec4 16个字节
layout(std140) uniform Taa
{
	int frameCount;
	int offsetIndex;
};

//64*16=1024,64个vec4
layout(std140) uniform SampleArray
{
	vec3 samples[64];
};

//128*64 bytes
layout(std140) uniform Bones
{
	mat4 bones[128];
};

ubo{
	Matrixes = 0
	Lights = 1
	ScreenWH = 2
	Taa = 3
	SampleArray = 4
	Bones = 5
}
*/

//注释
Ubo::Ubo() = default;

Ubo::~Ubo() {
	if (mId > 0) {
		glDeleteBuffers(1, &mId);
	}
}

void Ubo::bind(const char* ubName, int sizeInByte, int bindPoint)  noexcept{
	static int alignment = -1;
	if (alignment == -1) {
		glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &alignment);
	}
	if (mUniformBlocks.emplace(ubName, std::make_pair(mCurSize, sizeInByte)).second) {
		if (mCurSize + sizeInByte <= totalUboSize) {
			glBindBufferRange(GL_UNIFORM_BUFFER, bindPoint, mId, mCurSize, sizeInByte);
			mCurSize += sizeInByte;
			int left = mCurSize % alignment;
			if (left != 0) {
				mCurSize += (alignment - left);
			}
			checkglerror();
		}
	}
	else {
		LOGD("duplicate ubo binding, the uniform block %s has bound to bindPoint %d", ubName, bindPoint);
	}
}

void Ubo::update(const std::string& ubName, void* pdata, int sizeInByte, int offset)  noexcept{
	auto it = mUniformBlocks.find(ubName);
	if (it != mUniformBlocks.end()) {
		if (sizeInByte > it->second.second) {
			LOGE("uniform block %s update error,exceed it's size",ubName.c_str());
			return;
		}
		glBindBuffer(GL_UNIFORM_BUFFER, mId);
		//第二个参数是偏移
		glBufferSubData(GL_UNIFORM_BUFFER, it->second.first + offset, sizeInByte, pdata);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}
	else {
		LOGE("uniform block %s update error,hasn't bound",ubName.c_str());
	}
}

int Ubo::getMaxBindPoint() {
	int counts = 24;
	glGetIntegerv(GL_MAX_UNIFORM_BUFFER_BINDINGS, &counts);
	return counts;
}