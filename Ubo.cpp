#include "Ubo.h"
#include "Log.h"
extern void checkglerror();
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
	if (mCurSize+sizeInByte <= totalUboSize && mUniformBlocks.emplace(ubName, std::make_pair(mCurSize, sizeInByte)).second) {
		glBindBufferRange(GL_UNIFORM_BUFFER, bindPoint, mId, mCurSize, sizeInByte);
		mCurSize += sizeInByte;
		int left = mCurSize % alignment;
		if (left != 0) {
			mCurSize += (alignment - left);
		}
		checkglerror();
	}
	else {
		LOGE("the memory of ubo is not enough or duplicate ubo binding,the uniform block %s has bound to bindPoint %d", ubName,bindPoint);
	}
}

void Ubo::update(const std::string& ubName, void* pdata, int sizeInByte)  noexcept{
	auto it = mUniformBlocks.find(ubName);
	if (it != mUniformBlocks.end()) {
		if (sizeInByte > it->second.second) {
			LOGE("uniform block %s update error,exceed it's size",ubName.c_str());
			return;
		}
		glBindBuffer(GL_UNIFORM_BUFFER, mId);
		glBufferSubData(GL_UNIFORM_BUFFER, it->second.first, sizeInByte, pdata);
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