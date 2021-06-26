/*
 * Mesh.cpp
 *
 *  Created on: 2015-9-19
 *      Author: Administrator
 */
//#include "StdAfx.h"
#include "Mesh.h"
#include "Log.h"
#include <cmath>
//extern void checkglerror();
void checkglerror()
{
	/*
	GL_INVALID_ENUM, 0x0500
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
Mesh::Mesh(int meshType):
	mPosVbo(0),
	mTexVbo(0),
	mNorVbo(0),
	mIndexVbo(0),
	mIndexByteSize(0),
	mPosByteSize(0),
	mNorByteSize(0),
	mTexByteSize(0),
	mMeshType(meshType),
	mposLocation(0),
	mtexLocation(0),
	mnorLocation(0),
	mVAO(0),
	mLineWidth(1.0f),
	mCounts(0),
	mId(0)
{
	// TODO Auto-generated constructor stub

}

Mesh::Mesh() :
	mPosVbo(0),
	mTexVbo(0),
	mNorVbo(0),
	mIndexVbo(0),
	mIndexByteSize(0),
	mPosByteSize(0),
	mNorByteSize(0),
	mTexByteSize(0),
	mMeshType(MESH_DIY),
	mposLocation(0),
	mtexLocation(0),
	mnorLocation(0),
	mVAO(0),
	mLineWidth(1.0f),
	mCounts(0),
	mId(0)
{

}

void Mesh::reset() {
	mPosVbo = 0;
	mTexVbo = 0;
	mNorVbo = 0;
	mIndexVbo = 0;
	mposLocation = 0;
	mtexLocation = 0;
	mnorLocation = 0;
	mVAO = 0;
	mLineWidth = 1.0f;
	mPosByteSize = 0;
	mNorByteSize = 0;
	mTexByteSize = 0;
	mIndexByteSize = 0;
	mMeshType = MESH_DIY;
	mCounts = 0;
}

Mesh::Mesh(Mesh&& temp):
	mPosVbo(temp.mPosVbo),
	mTexVbo(temp.mTexVbo),
	mNorVbo(temp.mNorVbo),
	mIndexVbo(temp.mIndexVbo),
	mIndexByteSize(temp.mIndexByteSize),
	mPosByteSize(temp.mPosByteSize),
	mNorByteSize(temp.mNorByteSize),
	mTexByteSize(temp.mTexByteSize),
	mMeshType(temp.mMeshType),
	mposLocation(temp.mposLocation),
	mtexLocation(temp.mtexLocation),
	mnorLocation(temp.mnorLocation),
	mVAO(temp.mVAO),
	mLineWidth(temp.mLineWidth),
	mCounts(temp.mCounts)
{
	temp.reset();
}

Mesh::~Mesh()
{
	// TODO Auto-generated destructor stub
	unLoadMesh();
}

void Mesh::loadMesh()
{
	if (mMeshType == MESH_Rectangle)
	{
		GLfloat pos[] = { -1.0f,1.0f,0.0f,
			-1.0f,-1.0f,0.0f,
			1.0f,-1.0f,0.0f,
			1.0f,1.0f,0.0f };
		GLushort indexes[] = { 0,1,2,0,2,3 };
		GLfloat tex[] = { 0.0f,1.0f,0.0f,0.0f,1.0f,0.0f,1.0f,1.0f };
		loadMesh(pos, sizeof(pos), indexes, sizeof(indexes), tex, sizeof(tex));
	}
	else if (mMeshType == MESH_Circle)
	{
		mCounts = 1026;
		GLfloat* pos = new GLfloat[mCounts *3];
		GLfloat* tex = new GLfloat[mCounts * 2];
		if (pos && tex)
		{
			float delta = 2.0f*3.1415926535897932f / 1024.0f;
			pos[0] = pos[1] = pos[2] = 0.0f;
			tex[0] = tex[1] = 0.5f;
			for (int i = 1; i < mCounts; ++i)
			{
				pos[i * 3] = cos(3.1415926535897932f-(i-1)*delta);
				pos[i * 3+1] = sin(3.1415926535897932f- (i - 1)*delta);
				pos[i * 3 + 2] = 0;

				tex[i * 2] = pos[i * 3] * 0.5f + 0.5f;
				tex[i * 2+1] = pos[i * 3 + 1] * 0.5f + 0.5f;
			}
			bool b = loadMesh(pos, mCounts * 3*sizeof(GLfloat), 0, 0, tex, mCounts * 2*sizeof(GLfloat),0,0);
			if (!b)
			{
				LOGD("error to load circle mesh data\n");
			}
			delete[] pos;
			delete[] tex;
		}
	}
}

bool Mesh::loadMesh(GLfloat* pos,int posByteSize,GLushort* index,int indexByteSize,
	GLfloat* tex,int texByteSize,GLfloat* nor,int norByteSize,int drawType)
{
	if(pos!=0)
	{
		setPosData(pos,posByteSize, drawType);
		checkglerror();
		mCounts = (posByteSize / (sizeof(float) * 3));
	}
	if(tex!=0)
	{
		setTexcoordData(tex,texByteSize, drawType);
	}
	if(nor!=0)
	{
		setNormalData(nor,norByteSize, drawType);
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if(index!=0)
	{
		setIndexData(index,indexByteSize, drawType);
		checkglerror();
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	return true;
}

//更新pos vbo
bool Mesh::updataPos(float* pos, int byteOffset, int size)
{
	glBindBuffer(GL_ARRAY_BUFFER, mPosVbo);
	glBufferSubData(GL_ARRAY_BUFFER, byteOffset, size, pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}
//更新纹理坐标vbo
bool Mesh::updataTexcoord(float* tex, int byteOffset, int size)
{
	glBindBuffer(GL_ARRAY_BUFFER, mTexVbo);
	glBufferSubData(GL_ARRAY_BUFFER, byteOffset, size, tex);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}
//更新索引vbo
bool Mesh::updataIndex(float* pIndex, int byteOffset, int size)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexVbo);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, byteOffset, size, pIndex);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	return true;
}

void Mesh::drawLineStrip(int posloc)
{
	if (createVaoIfNeed(posloc)) {
		glBindVertexArray(mVAO);
		if (mposLocation >= 0) {
			glBindBuffer(GL_ARRAY_BUFFER, mPosVbo);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexVbo);
			glEnableVertexAttribArray(posloc);
			glVertexAttribPointer(posloc, 3, GL_FLOAT, GL_FALSE, 0, 0);
		}
		else {

		}
		glBindVertexArray(0);
	}
	
	glBindVertexArray(mVAO);
	glLineWidth(mLineWidth);
	glDrawArrays(GL_LINE_STRIP, 0, mCounts);
	glBindVertexArray(0);
	//glDrawElements(GL_LINE_LOOP, mNumOfIndex, GL_UNSIGNED_SHORT, (const void*)0);
}

void Mesh::drawTrangleFan(int posloc, int texloc)
{
	//glFrontFace(GL_CW);
	if (createVaoIfNeed(posloc)){
		glBindVertexArray(mVAO);
		if (posloc >= 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER, mPosVbo);
			glEnableVertexAttribArray(posloc);
			glVertexAttribPointer(posloc, 3, GL_FLOAT, GL_FALSE, 0, 0);
		}
		if (texloc >= 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER, mTexVbo);
			//indicate a vertexAttrib space 2*float,in mTexVbo
			glEnableVertexAttribArray(texloc);
			glVertexAttribPointer(texloc, 2, GL_FLOAT, GL_FALSE, 0, 0);

		}
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexVbo);

		glBindVertexArray(0);
	}

	glBindVertexArray(mVAO);
	glDrawArrays(GL_TRIANGLE_FAN, 0, mCounts);
	glBindVertexArray(0);
	//glFrontFace(GL_CCW);
	//glDrawElements(GL_TRIANGLE_FAN, mNumOfIndex, GL_UNSIGNED_SHORT, (const void*)0);
}


void Mesh::drawTriangles(int posloc,int texloc,int norloc)
{
	if (createVaoIfNeed(posloc)) {
		glBindVertexArray(mVAO);
		if (posloc >= 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER, mPosVbo);
			glEnableVertexAttribArray(posloc);
			//indicate a vertexAttrib space 3*float,in mPosVbo
			glVertexAttribPointer(posloc, 3, GL_FLOAT, GL_FALSE, 0, 0);
			checkglerror();
		}

		if (texloc >= 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER, mTexVbo);
			glEnableVertexAttribArray(texloc);
			//indicate a vertexAttrib space 2*float,in mTexVbo
			glVertexAttribPointer(texloc, 2, GL_FLOAT, GL_FALSE, 0, 0);

		}
		if (norloc >= 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER, mNorVbo);
			glEnableVertexAttribArray(norloc);
			glVertexAttribPointer(norloc, 3, GL_FLOAT, GL_FALSE, 0, 0);

		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexVbo);//glDrawElements会用到这个
		glBindVertexArray(0);
	}
	
	glBindVertexArray(mVAO);
	glDrawElements(GL_TRIANGLES, mIndexByteSize/sizeof(GLushort), GL_UNSIGNED_SHORT, (const void*)0);
	glBindVertexArray(0);
}

void Mesh::getMaxNumVertexAttr()
{
	GLint maxVertexAttribs; // es2.0 n will be >= 8,es3.0 >=16
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
	LOGD("gl global Param: GL_MAX_VERTEX_ATTRIBS %d", maxVertexAttribs);
}

void Mesh::getLineWidthRange() {
	GLfloat lineWidthRange[2];
	glGetFloatv(GL_ALIASED_LINE_WIDTH_RANGE, lineWidthRange);
	LOGD("gl global Param: GL_ALIASED_LINE_WIDTH_RANGE %f,%f", lineWidthRange[0], lineWidthRange[1]);
}

//获取点精灵，半径大小，点精灵（GL_POINTS primitive）其实是个和屏幕对齐的小正方形。
void Mesh::getPointSizeRange() {
	GLfloat pointSizeRange[2];
	glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, pointSizeRange);
	LOGD("gl global Param: GL_ALIASED_POINT_SIZE_RANGE %f,%f", pointSizeRange[0], pointSizeRange[1]);
}

void Mesh::draw(int posloc, int texloc, int norloc)
{
	if (mMeshType == MESH_Rectangle || mMeshType == MESH_DIY)
	{
		drawTriangles(posloc, texloc, norloc);
	}
	else if (mMeshType == MESH_Circle)
	{
		drawTrangleFan(posloc, texloc);
	}
	else if (mMeshType == MESH_Line_strip)
	{
		drawLineStrip(posloc);
	}
}


bool Mesh::setPosData(GLfloat* pos, int size, unsigned int drawType)
{
	glGenBuffers(1, &mPosVbo);
	glBindBuffer(GL_ARRAY_BUFFER, mPosVbo);
	glBufferData(GL_ARRAY_BUFFER, size, pos, drawType);
	mPosByteSize = size;
	return true;
}
bool Mesh::setTexcoordData(GLfloat* tex, int size, unsigned int drawType)
{
	glGenBuffers(1, &mTexVbo);
	glBindBuffer(GL_ARRAY_BUFFER, mTexVbo);
	glBufferData(GL_ARRAY_BUFFER, size, tex, drawType);
	mTexByteSize = size;
	return true;
}

bool Mesh::setNormalData(GLfloat* nor, int size, unsigned int drawType)
{
	glGenBuffers(1, &mNorVbo);
	glBindBuffer(GL_ARRAY_BUFFER, mNorVbo);
	glBufferData(GL_ARRAY_BUFFER, size, nor, drawType);
	mNorByteSize = size;
	return true;
}

bool Mesh::setIndexData(GLushort* index, int size, unsigned int drawType)
{
	glGenBuffers(1, &mIndexVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexVbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, index, drawType);
	mIndexByteSize = size;
	return true;
}

void Mesh::unLoadMesh()
{
	if (mPosVbo != 0)
	{
		glDeleteBuffers(1, &mPosVbo);
		mPosVbo = 0;
		mPosByteSize = 0;
	}
	if (mNorVbo != 0)
	{
		glDeleteBuffers(1, &mNorVbo);
		mNorVbo = 0;
		mNorByteSize = 0;
	}
	if (mTexVbo != 0)
	{
		glDeleteBuffers(1, &mTexVbo);
		mTexVbo = 0;
		mTexByteSize = 0;
	}
	if (mIndexVbo != 0)
	{
		glDeleteBuffers(1, &mIndexVbo);
		mIndexVbo = 0;
		mIndexByteSize = 0;
	}
	if (mVAO != 0) {
		glDeleteVertexArrays(1, &mVAO);
		mVAO = 0;
	}
}

bool Mesh::createVaoIfNeed(int posloc, int texloc, int norloc) {
	if (mposLocation != posloc || mtexLocation != texloc || mnorLocation != norloc) {
		if (mVAO != 0) {
			//location有变化，先删除原来的vao
			glDeleteVertexArrays(1, &mVAO);
			mVAO = 0;
		}

		mposLocation = posloc;
		mtexLocation = texloc;
		mnorLocation = norloc;

		//使用vao把设置顶点属性的流程打包。
		glGenVertexArrays(1, &mVAO);
		return true;
	}
	return false;
}

void Mesh::setLineWidth(GLfloat width) {
	mLineWidth = width;
}
