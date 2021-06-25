/*
 * GraphicsMesh.h
 *
 *  Created on: 2015-9-19
 *      Author: ldp
 */

#ifndef GRAPHICSMESH_H_
#define GRAPHICSMESH_H_
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
//#include "Drawable.h"
enum
{
	MESH_Rectangle,
	MESH_Circle,
	MESH_Lines,
	MESH_Line_strip,
	MESH_DIY
};

class GraphicsMesh
{
public:
	explicit GraphicsMesh(int meshType );
	GraphicsMesh();
	virtual ~GraphicsMesh();

	//MESH_Rectangle,MESH_Circle,调用这个函数初始化
	void loadMesh();

	//根据指定的顶点坐标数据，纹理坐标数据，法向量坐标数据，顶点索引数据，以及他们的大小（字节为单位）
	//创建对应的vbo，类型为静态GL_STATIC_DRAW
	bool loadMesh(GLfloat* pos,int posByteSize,GLushort* index,int indexByteSize,
		GLfloat* tex=0,int texByteSize=0,GLfloat* nor=0,int norByteSize=0,int drawType = GL_STATIC_DRAW);

	//更新pos vbo
	bool updataPos(float* pos,int byteOffset,int size);
	//更新纹理坐标vbo
	bool updataTexcoord(float* tex, int byteOffset, int size);
    //更新索引vbo
	bool updataIndex(float* pIndex, int byteOffset, int size);

	void draw(int posloc = -1, int texloc = -1, int norloc = -1);

	void unLoadMesh();

	static int getMaxNumVertexAttr();
private:
	//4个vbo对象
	GLuint mPosVbo;//这个是vbo
	GLuint mNorVbo;//这个是vbo
	GLuint mTexVbo; //这个是vbo
	GLuint mIndexVbo; //这个别人叫ebo
	GLuint mVAO;//这个是vao，顶点数组对象，opengles3.0才支撑，是一个集合。把设定顶点属性的过程打包到一起，简化绘制流程。

	int mposLocation;//顶点的位置属性在shader中的location
	int mtexLocation;//顶点的纹理坐标属性在shader中的location
	int mnorLocation;//顶点的法向量属性在shader中的location

	int mIndexByteSize;
	int mMeshType;
	int mCounts;//for line_strip,triangle_fan,the count of points;

	//这四个函数都是创建vbo，ebo，并冲内存上传数据到vbo的显存
	bool setPosData(GLfloat* pos,int size,unsigned int drawType = GL_STATIC_DRAW);
	bool setTexcoordData(GLfloat* tex,int size, unsigned int drawType = GL_STATIC_DRAW);
	bool setNormalData(GLfloat* nor,int size, unsigned int drawType = GL_STATIC_DRAW);
	bool setIndexData(GLushort* index,int indexByteSize, unsigned int drawType = GL_STATIC_DRAW);
	//如果函数内部创建了vao就返回true
	bool createVaoIfNeed(int posloc=-1, int texloc=-1, int norloc=-1);

	//当做三角形绘制GL_TRIANGLES
	void drawTriangles(int posloc = -1, int texloc = -1, int norloc = -1);

	//当做直线绘制GL_LINE_LOOP
	void drawLineStrip(int posloc);

	//当做三角形扇绘制GL_TRIANGLE_FAN
	void drawTrangleFan(int posloc, int texloc = -1);
};

#endif /* GRAPHICSMESH_H_ */
