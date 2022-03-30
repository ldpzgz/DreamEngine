﻿/*
 * Mesh.h
 *
 *  Created on: 2015-9-19
 *      Author: ldp
 */

#ifndef GRAPHICSMESH_H_
#define GRAPHICSMESH_H_
#ifdef _GLES3
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>
#else
#include <glad/glad.h>
#endif
#include "material.h"

#include <glm/vec3.hpp>           // vec3
#include <glm/vec4.hpp>           // vec3
#include <glm/mat4x4.hpp>         // mat4
#include <glm/trigonometric.hpp>  //sin cos,tan,radians,degree
#include <glm/ext/matrix_transform.hpp> // perspective, translate, rotate
#include <glm/gtc/type_ptr.hpp> // value_ptr,make_vec,make_mat

#include "Renderable.h"
#include "Rect.h"
#include <memory>
#include "aabb.h"
enum class MeshType //枚举类型定义加了class就是强类型枚举，不能隐式转换为其他类型，
{
	MESH_None,
	MESH_Triangle,
	MESH_Quad,//矩形，坐标范围是[-1，1]
	MESH_Rectangle,//纯色矩形,坐标范围是[0,1]
	MESH_TrianglePost,//用于后处理的triangle
	MESH_FilledRect,//可以填充纯色，可贴图，也可以绘制为线框的矩形
	MESH_Rounded_Rectangle,//带center color的rounded rectangle
	MESH_Cuboid,//立方体
	MESH_Circle,//填充的圆，或者圆圈
	Mesh_Shpere,
	MESH_Line_strip,
	MESH_Cubic_Spline,
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
	MESH_DIY,
};

enum class DrawType {
	Triangles,
	TriangleFan,
	TriangleStrip,
	Lines,
	LineStrip,
	Points,
};
class Mesh : public Renderable
{
public:
	explicit Mesh(MeshType meshType) noexcept;
	Mesh(MeshType meshType,DrawType drawType) noexcept;
	Mesh() = default;
	Mesh(const Mesh&&) noexcept;
	Mesh(const Mesh&) = delete; //防止拷贝
	Mesh & operator = (const Mesh&) = delete; //防止赋值
	virtual ~Mesh();

	//MESH_Rectangle_Tex,+MESH_Circle,调用这个函数初始化
	virtual void loadMesh();

	//nurbs
	virtual void loadMesh(const std::vector<glm::vec3>& p, const std::vector<int>& knots, const std::vector<float>& w) {

	}

	virtual void loadMesh(const std::vector<glm::vec3>& p,int num) {

	}

	virtual void loadMesh(const std::vector<glm::vec2>& p, int num) {

	}

	virtual void loadMesh(const std::vector<glm::vec2>& p) {

	}

	virtual void loadMesh(const std::vector<glm::vec3>& p) {

	}

	virtual void loadMesh(const std::vector<glm::vec3>& pos, const std::vector<glm::uvec3>& index);

	virtual bool loadMesh(const std::string meshFilePath);

	virtual bool loadMesh(const std::vector<float>& pos,
		const std::vector<float>& texcoord,
		const std::vector<float>& normal,
		const std::vector<unsigned int>& index);

	//uniform b spline
	//void loadUBS(const std::vector<float>& points);

	//更新pos vbo，如果要更新的数据超过原来vbo的大小，先删除vbo，再创建一个新的vbo
	bool updataPos(float* pos,int byteOffset,int size, int numOfVertex);
	//更新纹理坐标vbo，如果要更新的数据超过原来vbo的大小，先删除vbo，再创建一个新的vbo
	bool updataTexcoord(float* tex, int byteOffset, int size);
    //更新索引vbo
	bool updataIndex(GLuint* pIndex, int byteOffset, int size);

	//更新color vbo，如果要更新的数据超过原来vbo的大小，先删除vbo，再创建一个新的vbo
	bool updateColor(float* color, int byteOffset, int size);

	//更新法向量 vbo，如果要更新的数据超过原来vbo的大小，先删除vbo，再创建一个新的vbo
	bool updateNormal(float* normal, int byteOffset, int size);

	//bool updateBiNormal(float* normal, int byteOffset, int size);

	bool updateTangent(float* normal, int byteOffset, int size);

	void draw(int posloc, int texloc = -1, int norloc = -1, int colorloc = -1, int tangentloc = -1) override;
	void draw(const glm::mat4* projMat, 
		const glm::mat4* modelMat,
		const glm::mat4* viewMat = nullptr,
		const glm::mat4* texMat=nullptr,
		const std::vector<glm::vec3>* lightPos = nullptr, 
		const std::vector<glm::vec3>* lightColor = nullptr, 
		const glm::vec3* viewPos = nullptr ) override;

	//void render(const glm::mat4& mvpMat, const glm::mat4& mvMat);

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
	void setMaterial(const std::shared_ptr<Material>& p) {
		mpMaterial = p;
	}

	std::shared_ptr<Material>& getMaterial() {
		return mpMaterial;
	}

	std::unique_ptr<AABB>& getAabb() {
		return mpAabb;
	}
	std::string& getMaterialName() {
		return mMaterialName;
	}
	
	void setCastShadow(bool b) {
		mbCastShadow = b;
	}
	bool getCastShadow() {
		return mbCastShadow;
	}
	void setReceiveShadow(bool b) {
		mbReceiveShadow = b;
	}
	bool getReceiveShadow() {
		return mbReceiveShadow;
	}

	//这四个函数都是创建vbo，ebo，并从内存上传数据到vbo的显存,如果之前存在vbo了，先删除
	bool setPosData(const GLfloat* pos, int sizeInbyte, unsigned int drawType = GL_STATIC_DRAW);
	bool setTexcoordData(const GLfloat* tex, int sizeInbyte, unsigned int drawType = GL_STATIC_DRAW);
	bool setNormalData(const GLfloat* nor, int sizeInbyte, unsigned int drawType = GL_STATIC_DRAW);
	bool setTangentData(const GLfloat* nor, int sizeInbyte, unsigned int drawType = GL_STATIC_DRAW);
	//bool setBiTangentData(GLfloat* nor, int sizeInbyte, unsigned int drawType = GL_STATIC_DRAW);
	bool setColorData(const GLfloat* nor, int sizeInbyte, unsigned int drawType = GL_STATIC_DRAW);
	bool setIndexData(const GLuint* index, int indexByteSize, unsigned int drawType = GL_STATIC_DRAW);

	static void getMaxNumVertexAttr();
	static void getLineWidthRange();
	static void getPointSizeRange();
protected:
	//根据指定的顶点坐标数据，纹理坐标数据，法向量坐标数据，顶点索引数据，以及他们的大小（字节为单位）
	//创建对应的vbo，类型为静态GL_STATIC_DRAW
	bool createBufferObject(const GLfloat* pos, int posByteSize, int countOfVertex,
		const GLuint* index, int indexByteSize,
		const GLfloat* tex = 0, int texByteSize = 0,
		const GLfloat* nor = 0, int norByteSize = 0,
		const GLfloat* color = 0, int colorByteSize = 0,
		const GLfloat* tangent = 0, int tangentByteSize = 0,
		int drawType = GL_STATIC_DRAW);

	//当做三角形绘制GL_TRIANGLES
	void drawTriangles(int posloc = -1, int texloc = -1, int norloc = -1,int colorloc = -1, int tangentloc = -1);

	//当做三角形绘制GL_TRIANGLE_strip
	//void drawTriangleStrip(int posloc = -1, int texloc = -1, int norloc = -1, int colorloc = -1, int tangentloc = -1);

	//当做直线绘制GL_LINE_LOOP
	virtual void drawLineStrip(int posloc);

	//当做三角形扇绘制GL_TRIANGLE_FAN
	//virtual void drawTriangleFan(int posloc, int texloc = -1, int norloc = -1, int colorloc = -1, int tangentloc = -1);
protected:
	void reset();
	bool mbCastShadow{true};
	bool mbReceiveShadow{ true };
	//4个vbo对象
	GLuint mPosVbo{ 0 };
	GLuint mTexVbo{ 0 }; 
	GLuint mNorVbo{ 0 };
	GLuint mTangentVbo{ 0 };
	GLuint mBiNormalVbo{ 0 };
	GLuint mColorVbo{ 0 };
	GLuint mIndexVbo{ 0 }; 
	GLuint mVAO{ 0 };//这个是vao，顶点数组对象，opengles3.0才支持，是一个集合。把设定顶点属性的过程打包到一起，简化绘制流程。
	GLfloat mLineWidth{ 1.0f };

	int mposLocation{ -1 };//顶点的位置属性在shader中的location
	int mnorLocation{ -1 };//顶点的法向量属性在shader中的location
	int mtexLocation{ -1 };//顶点的纹理坐标属性在shader中的location
	int mColorLoc{ -1 };
	
	int mPosByteSize{ 0 };
	int mTexByteSize{ 0 };
	int mNorByteSize{ 0 };
	int mTangentByteSize{ 0 };
	int mBiNormalByteSize{ 0 };
	int mColorByteSize{ 0 };
	int mIndexByteSize{ 0 };
	std::string mMaterialName;
	MeshType mMeshType{ MeshType::MESH_None };
	DrawType mDrawType{ DrawType::Triangles };
	int mCountOfVertex{ 0 };//vertex的个数，这里默认pos，texcoord，normal，color等属性的顶点个数都是一样的;
	unsigned int mId{ 0 };
	std::shared_ptr<Material> mpMaterial;
	std::unique_ptr<AABB> mpAabb;
	std::unique_ptr< glm::mat4> mpPreMvpMatrix;
	//如果函数内部创建了vao就返回true
	bool createVaoIfNeed(int posloc=-1, int texloc=-1, int norloc=-1,int colorLoc=-1);
};
using MeshSP = std::shared_ptr<Mesh>;

#endif /* GRAPHICSMESH_H_ */
