/*
 * Mesh.h
 *
 *  Created on: 2015-9-19
 *      Author: ldp
 */

#ifndef GRAPHICSMESH_H_
#define GRAPHICSMESH_H_
#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "material.h"
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp> // vec3, vec4, ivec4, mat4
#include <glm/mat4x4.hpp>
#include <glm/mat4x3.hpp>
#include <glm/gtc/type_ptr.hpp> // value_ptr
#include "Attachable.h"
enum class MeshType //枚举类型定义加了class就是强类型枚举，不能隐式转换为其他类型，
{
	MESH_Triangle,
	MESH_Rectangle,
	MESH_Cuboid,
	MESH_Triangular_Pyramid,
	MESH_Circle,
	MESH_Lines,
	MESH_Line_strip,
	MESH_DIY,
	MESH_Cubic_Hermiter_Curves,//hermiter曲线，后面的贝塞尔曲线，cr_spline都可以由这个曲线实现
		//一段3次hermiter曲线，由两个顶点，以及这两个顶点处的切线经过cubic插值而成。曲线会经过这两个顶点
		//切线有方向和大小，用于控制曲线的形状。如果两段hermiter曲线需要平滑连接，连接处的顶点的切线方向必须相同。
	MESH_Catmull_Rom_Splines, 
		//这个是过n点，具有C1连续性质的曲线，在hermiter曲线原理的基础上，其几何矩阵由4个顶点组成，根据c1连续，推导出基矩阵，
		//给定点集[P0,P1,P2.....Pn],绘制出过点[P1,P2.....P(n-1)]的曲线
		//这种曲线，改变其中一个点的位置，只影响邻近的两个点之间的曲线。
	MESH_Bezier_Curves,
		//n个控制点[P1,P2,....Pn]，通过控制点生成一段曲线，曲线会经过P1,Pn,逼近P2,P3...Pn-1。
		//顶点越多，阶数越高
	MESH_Uniform_B_Curves,
		//通过n个控制点，绘制n-3条曲线，曲线不经过任何控制点，只逼近但是曲线之间是C2连续的
	MESH_NURBS,
		//这个在设计软件中最通用

	MESH_FONTS, //渲染文字的时候用的，一个文字
	MESH_Rect,//渲染具有Rect的背景的view用的
};

class Mesh : public Attachable
{
public:
	explicit Mesh(MeshType meshType );
	Mesh();
	Mesh(Mesh&&);
	Mesh(const Mesh&) = delete; //防止拷贝
	Mesh & operator = (const Mesh&) = delete; //防止赋值
	virtual ~Mesh();

	//MESH_Rectangle,MESH_Circle,调用这个函数初始化
	virtual void loadMesh();


	virtual void loadMesh(const std::vector<float>& p, const std::vector<int>& knots, const std::vector<float>& w) {

	}

	virtual void loadMesh(const std::vector<float>& p) {

	}

	//uniform b spline
	//void loadUBS(const std::vector<float>& points);

	//更新pos vbo
	bool updataPos(float* pos,int byteOffset,int size);
	//更新纹理坐标vbo
	bool updataTexcoord(float* tex, int byteOffset, int size);
    //更新索引vbo
	bool updataIndex(float* pIndex, int byteOffset, int size);

	//更新color vbo
	bool updateColor(float* color, int byteOffset, int size);

	//更新法向量 vbo
	bool updateNormal(float* normal, int byteOffset, int size);

	void render(const glm::mat4& projviewMat);

	void render(const glm::mat4& mvpMat, const glm::mat4& texMat);

	void unLoadMesh();

	void setLineWidth(GLfloat);
	GLfloat getLineWidth() {
		return mLineWidth;
	}

	void setId(unsigned int id) {
		mId = id;
	}
	unsigned int getId() {
		return mId;
	}
	void setMaterial(std::shared_ptr<Material>& p) {
		mpMaterial = p;
	}

	std::shared_ptr<Material>& getMaterial() {
		return mpMaterial;
	}

	static void getMaxNumVertexAttr();
	static void getLineWidthRange();
	static void getPointSizeRange();
protected:
	//根据指定的顶点坐标数据，纹理坐标数据，法向量坐标数据，顶点索引数据，以及他们的大小（字节为单位）
	//创建对应的vbo，类型为静态GL_STATIC_DRAW
	bool createBufferObject(GLfloat* pos, int posByteSize, GLuint* index, int indexByteSize,
		GLfloat* tex = 0, int texByteSize = 0, GLfloat* nor = 0, int norByteSize = 0, GLfloat* color = 0, int colorByteSize = 0, int drawType = GL_STATIC_DRAW);

	//当做三角形绘制GL_TRIANGLES
	void drawTriangles(int posloc = -1, int texloc = -1, int norloc = -1,int colorloc = -1);

	//当做直线绘制GL_LINE_LOOP
	void drawLineStrip(int posloc);

	//当做三角形扇绘制GL_TRIANGLE_FAN
	void drawTrangleFan(int posloc, int texloc = -1, int norloc = -1, int colorloc = -1);

	virtual void draw(int posloc = -1, int texloc = -1, int norloc = -1, int colorloc = -1);
private:
	void reset();

	//4个vbo对象
	GLuint mPosVbo;//这个是vbo
	GLuint mTexVbo; //这个是vbo
	GLuint mNorVbo;//这个是vbo
	GLuint mColorVbo;//这个是vbo
	GLuint mIndexVbo; //这个别人叫ebo
	GLuint mVAO;//这个是vao，顶点数组对象，opengles3.0才支持，是一个集合。把设定顶点属性的过程打包到一起，简化绘制流程。
	GLfloat mLineWidth;

	int mposLocation;//顶点的位置属性在shader中的location
	int mnorLocation;//顶点的法向量属性在shader中的location
	int mtexLocation;//顶点的纹理坐标属性在shader中的location
	
	unsigned int mPosByteSize;
	unsigned int mTexByteSize;
	unsigned int mNorByteSize;
	unsigned int mColorByteSize;
	unsigned int mIndexByteSize;
	MeshType mMeshType;
	int mCounts;//for line_strip,triangle_fan,the count of points;
	unsigned int mId;
	std::shared_ptr<Material> mpMaterial;

	//这四个函数都是创建vbo，ebo，并从内存上传数据到vbo的显存
	bool setPosData(GLfloat* pos,int size,unsigned int drawType = GL_STATIC_DRAW);
	bool setTexcoordData(GLfloat* tex,int size, unsigned int drawType = GL_STATIC_DRAW);
	bool setNormalData(GLfloat* nor,int size, unsigned int drawType = GL_STATIC_DRAW);
	bool setColorData(GLfloat* nor, int size, unsigned int drawType = GL_STATIC_DRAW);
	bool setIndexData(GLuint* index,int indexByteSize, unsigned int drawType = GL_STATIC_DRAW);
	//如果函数内部创建了vao就返回true
	bool createVaoIfNeed(int posloc=-1, int texloc=-1, int norloc=-1);
};

#endif /* GRAPHICSMESH_H_ */
