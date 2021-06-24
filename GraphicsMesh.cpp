/*
 * GraphicsMesh.cpp
 *
 *  Created on: 2015-9-19
 *      Author: Administrator
 */
//#include "StdAfx.h"
#include "GraphicsMesh.h"
#include "../Log.h"
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
GraphicsMesh::GraphicsMesh(int meshType):
		mPosVbo(0),
		mTexVbo(0),
		mIndexVbo(0),
		mIndexByteSize(0),
		mMeshType(meshType)

{
	// TODO Auto-generated constructor stub

}

GraphicsMesh::GraphicsMesh() :
	mPosVbo(0),
	mTexVbo(0),
	mIndexVbo(0),
	mIndexByteSize(0),
	mMeshType(MESH_DIY)
{

}

GraphicsMesh::~GraphicsMesh()
{
	// TODO Auto-generated destructor stub
	unLoadMesh();
}

void GraphicsMesh::loadMesh()
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

bool GraphicsMesh::loadMesh(GLfloat* pos,int posByteSize,GLushort* index,int indexByteSize,
	GLfloat* tex,int texByteSize,GLfloat* nor,int norByteSize,int drawType)
{
	glGenVertexArrays(1, &mVAO);
	glBindVertexArray(mVAO);

	if(pos!=0)
	{
		setPosData(pos,posByteSize, drawType);
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
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glBindVertexArray(0);
	return true;
}

//更新pos vbo
bool GraphicsMesh::updataPos(float* pos, int byteOffset, int size)
{
	glBindBuffer(GL_ARRAY_BUFFER, mPosVbo);
	glBufferSubData(GL_ARRAY_BUFFER, byteOffset, size, pos);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}
//更新纹理坐标vbo
bool GraphicsMesh::updataTexcoord(float* tex, int byteOffset, int size)
{
	glBindBuffer(GL_ARRAY_BUFFER, mTexVbo);
	glBufferSubData(GL_ARRAY_BUFFER, byteOffset, size, tex);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}
//更新索引vbo
bool GraphicsMesh::updataIndex(float* pIndex, int byteOffset, int size)
{
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexVbo);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, byteOffset, size, pIndex);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	return true;
}

void GraphicsMesh::drawLineStrip(int posloc)
{
	if(posloc>=0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, mPosVbo);
		glVertexAttribPointer(posloc, 3,GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(posloc);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mIndexVbo);
	glDrawArrays(GL_LINE_STRIP, 0, mCounts);
	//glDrawElements(GL_LINE_LOOP, mNumOfIndex, GL_UNSIGNED_SHORT, (const void*)0);
}

void GraphicsMesh::drawTrangleFan(int posloc, int texloc)
{
	//glFrontFace(GL_CW);
	if(posloc>=0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, mPosVbo);
		glVertexAttribPointer(posloc, 3,GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(posloc);
	}
	if (texloc >= 0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, mTexVbo);
		//indicate a vertexAttrib space 2*float,in mTexVbo
		glVertexAttribPointer(texloc, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(texloc);
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mIndexVbo);
	glDrawArrays(GL_TRIANGLE_FAN, 0, mCounts);
	//glFrontFace(GL_CCW);
	//glDrawElements(GL_TRIANGLE_FAN, mNumOfIndex, GL_UNSIGNED_SHORT, (const void*)0);
}


void GraphicsMesh::drawTriangles(int posloc,int texloc,int norloc)
{

	if(posloc>=0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, mPosVbo);
		//indicate a vertexAttrib space 3*float,in mPosVbo
		glVertexAttribPointer(posloc, 3,GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(posloc);
	}

	checkglerror();
	if(texloc>=0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, mTexVbo);
		//indicate a vertexAttrib space 2*float,in mTexVbo
		glVertexAttribPointer(texloc,2,GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(texloc);
	}
	checkglerror();
	if(norloc>=0)
	{
		glBindBuffer(GL_ARRAY_BUFFER, mNorVbo);
		glVertexAttribPointer(norloc, 3,GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(norloc);
	}
	checkglerror();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mIndexVbo);

	//第一个参数指定绘制类型，
	//第二个参数：是index数组里面的元素的个数，不是字节数，如果index是unsigned short类型 totalbyte/sizeof(unsigned short);
	//第三个参数，单个index的数据类型，
	//第四个参数，是一个指针，从index数组开头偏移多少，index数组是使用glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, index, drawType);
	//这个函数上传到显卡的数据。
	//
	glDrawElements(GL_TRIANGLES, mIndexByteSize/sizeof(GL_UNSIGNED_SHORT), GL_UNSIGNED_SHORT, (const void*)0);
	checkglerror();
}

int GraphicsMesh::getMaxNumVertexAttr()
{
	GLint maxVertexAttribs; // n will be >= 8
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &maxVertexAttribs);
	return maxVertexAttribs;
}

void GraphicsMesh::draw(int posloc, int texloc, int norloc)
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


bool GraphicsMesh::setPosData(GLfloat* pos, int size, unsigned int drawType)
{
	glGenBuffers(1, &mPosVbo);
	glBindBuffer(GL_ARRAY_BUFFER, mPosVbo);
	glBufferData(GL_ARRAY_BUFFER, size, pos, drawType);
	return true;
}
bool GraphicsMesh::setTexcoordData(GLfloat* tex, int size, unsigned int drawType)
{
	glGenBuffers(1, &mTexVbo);
	glBindBuffer(GL_ARRAY_BUFFER, mTexVbo);
	glBufferData(GL_ARRAY_BUFFER, size, tex, drawType);
	return true;
}

bool GraphicsMesh::setNormalData(GLfloat* nor, int size, unsigned int drawType)
{
	glGenBuffers(1, &mNorVbo);
	glBindBuffer(GL_ARRAY_BUFFER, mNorVbo);
	glBufferData(GL_ARRAY_BUFFER, size, nor, drawType);
	return true;
}

bool GraphicsMesh::setIndexData(GLushort* index, int size, unsigned int drawType)
{
	glGenBuffers(1, &mIndexVbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexVbo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, index, drawType);
	mIndexByteSize = size;
	return true;
}

void GraphicsMesh::unLoadMesh()
{
	if (mPosVbo != 0)
	{
		glDeleteBuffers(1, &mPosVbo);
	}
	if (mNorVbo != 0)
	{
		glDeleteBuffers(1, &mNorVbo);
	}
	if (mTexVbo != 0)
	{
		glDeleteBuffers(1, &mTexVbo);
	}
	if (mIndexVbo != 0)
	{
		glDeleteBuffers(1, &mIndexVbo);
	}
}
