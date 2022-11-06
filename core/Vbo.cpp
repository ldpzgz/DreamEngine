#include "Vbo.h"
#ifdef _GLES3
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>
#else
#include <glad/glad.h>
#endif
#include "Log.h"
Vbo::~Vbo() {
	if (mId > 0) {
		glDeleteBuffers(1, &mId);
		mId = 0;
	}
}

bool Vbo::initVbo(void* pdata, int sizeInByte) {
	if (mId > 0) {
		LOGD("vbo has inited before");
		glDeleteBuffers(1, &mId);
		mId = 0;
	}
	mTotalSize = sizeInByte;
	glGenBuffers(1, &mId);
	glBindBuffer(GL_ARRAY_BUFFER, mId);
	glBufferData(GL_ARRAY_BUFFER, sizeInByte, pdata, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}

bool Vbo::updateVbo(int byteOffset, void* pdata, int sizeInByte) {
	if (byteOffset + sizeInByte > mTotalSize) {
		LOGE("data overflow when update vbo");
		return false;
	}
	glBindBuffer(GL_ARRAY_BUFFER, mId);
	glBufferSubData(GL_ARRAY_BUFFER, byteOffset, sizeInByte, pdata);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}

void Vbo::bindArray(bool b) {
	if(b)
		glBindBuffer(GL_ARRAY_BUFFER, mId);
	else
		glBindBuffer(GL_ARRAY_BUFFER, 0);
}
void Vbo::bindElement(bool b) {
	if (b)
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mId);
	else
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}