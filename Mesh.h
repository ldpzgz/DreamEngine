/*
 * Mesh.h
 *
 *  Created on: 2015-9-19
 *      Author: ldp
 */

#pragma once

#ifdef _GLES3
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>
#else
#include <glad/glad.h>
#endif

#include <glm/vec3.hpp>           // vec3
#include <glm/vec4.hpp>           // vec3
#include <glm/mat4x4.hpp>         // mat4
#include "aabb.h"
#include "Renderable.h"
#include <memory>
#include <string_view>
#include <unordered_map>
#include "Vbo.h"

class Material;
class SkeletonAnimation;
class Skeleton;
enum class MeshType
{
	None,
	Triangle,
	Quad,//矩形，坐标范围是[-1，1]
	Rectangle,//纯色矩形,坐标范围是[0,1]
	TrianglePost,//用于后处理的triangle
	FilledRect,//可以填充纯色，可贴图，也可以绘制为线框的矩形
	RoundedRectangle,//带center color的rounded rectangle
	Cuboid,//立方体
	Circle,//填充的圆，或者圆圈
	Shpere,
	LineStrip,
	CubicSpline,
	CubicHermiterCurves,//hermiter曲线，后面的贝塞尔曲线，cr_spline都可以由这个曲线实现
		//一段3次hermiter曲线，由两个顶点，以及这两个顶点处的切线经过cubic插值而成。曲线会经过这两个顶点
		//切线有方向和大小，用于控制曲线的形状。如果两段hermiter曲线需要平滑连接，连接处的顶点的切线方向必须相同。
	CatmullRomSplines, 
		//这个是过n点，具有C1连续性质的曲线，在hermiter曲线原理的基础上，其几何矩阵由4个顶点组成，根据c1连续，推导出基矩阵，
		//给定点集[P0,P1,P2.....Pn],绘制出过点[P1,P2.....P(n-1)]的曲线
		//这种曲线，改变其中一个点的位置，只影响邻近的两个点之间的曲线。
	BezierCurves,
		//n个控制点[P1,P2,....Pn]，通过控制点生成一段曲线，曲线会经过P1,Pn,逼近P2,P3...Pn-1。
		//顶点越多，阶数越高
	UniformBCurves,
		//通过n个控制点，绘制n-3条曲线，曲线不经过任何控制点，只逼近但是曲线之间是C2连续的
	NURBS,
	DIY,
};

enum class DrawType {
	Points,
	Lines,
	LineLoop,
	LineStrip,
	Triangles,
	TriangleStrip,
	TriangleFan,
};
//class Vbo;
class Mesh : public Renderable
{
public:
	explicit Mesh(MeshType meshType);
	Mesh(MeshType meshType,DrawType drawType);
	Mesh() = default;
	Mesh(Mesh&&) noexcept;
	Mesh(const Mesh&) = delete; //防止拷贝
	Mesh & operator = (const Mesh&) = delete; //防止赋值
	virtual ~Mesh();

	//MESH_Rectangle_Tex,+Circle,调用这个函数初始化
	virtual void loadMesh();

	//nurbs
	virtual void loadMesh(const std::vector<glm::vec3>& p, const std::vector<int>& knots, const std::vector<float>& w) {

	}

	/*
	* for hermit,cubic,CRSpline,Bezier curves
	*/
	virtual void loadMesh(const std::vector<glm::vec3>& p,int num) {

	}
	/*
	* for hermit,cubic,CRSpline,Bezier curves
	*/
	virtual void loadMesh(const std::vector<glm::vec2>& p, int num) {

	}
	/*
	* for line
	*/
	virtual void loadMesh(const std::vector<glm::vec2>& p) {

	}
	/*
	* for line
	*/
	virtual void loadMesh(const std::vector<glm::vec3>& p) {

	}

	virtual void loadMesh(const std::vector<glm::vec3>& pos, 
		const std::vector<glm::uvec3>& index);

	//virtual bool loadMesh(const std::string meshFilePath);

	virtual bool loadMesh(const std::vector<float>& pos,
		const std::vector<float>& texcoord,
		const std::vector<float>& normal,
		const std::vector<unsigned int>& index);

	virtual bool loadMesh(const std::vector<float>& pos,
		const std::vector<float>& texcoord,
		const std::vector<float>& normal,
		const std::vector<unsigned int>& index,
		const std::vector<int>& boneId,
		const std::vector<float>& boneWeight
		);

	//uniform b spline
	//void loadUBS(const std::vector<float>& points);

	//更新pos vbo，如果要更新的数据超过原来vbo的大小，先删除vbo，再创建一个新的vbo
	bool updataPos(float* pos,int byteOffset,int size);
	//更新纹理坐标vbo，如果要更新的数据超过原来vbo的大小，先删除vbo，再创建一个新的vbo
	bool updataTexcoord(float* tex, int byteOffset, int size);
    //更新索引vbo
	bool updataIndex(GLuint* pIndex, int byteOffset, int size);

	//更新color vbo，如果要更新的数据超过原来vbo的大小，先删除vbo，再创建一个新的vbo
	bool updateColor(float* color, int byteOffset, int size);

	//更新法向量 vbo，如果要更新的数据超过原来vbo的大小，先删除vbo，再创建一个新的vbo
	bool updateNormal(float* normal, int byteOffset, int size);

	bool updateBoneId(GLint* pId, int byteOffset, int size);
	bool updateBoneWeight(float* pWeight, int byteOffset, int size);

	void draw(int posloc, int texloc, int norloc = -1, int colorloc = -1, int boneIdLoc = -1, int boneWeightLoc = -1) override;
	void draw(const glm::mat4* modelMat, 
		const glm::mat4* texMat=nullptr, 
		const glm::mat4* projViewMat = nullptr ) override;

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

	void setMaterialName(std::string_view name) {
		mMaterialName = name;
	}

	const std::string& getMaterialName() {
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

	void setPosSizeOffset(int size, int offset,int stride,int count,int vboIndex=0) {
		mPosByteSize = size;
		mPosOffset = offset;
		mPosStride = stride;
		mCountOfVertex = count;
		mPosVboIndex = vboIndex;
	}
	void setTexSizeOffset(int size, int offset, int stride=0, int vboIndex = 0) {
		mTexByteSize = size;
		mTexOffset = offset;
		mTexStride = stride;
	}
	void setNorSizeOffset(int size, int offset, int stride = 0, int vboIndex = 0) {
		mNorByteSize = size;
		mNorOffset = offset;
		mNorStride = stride;
		mNorVboIndex = vboIndex;
	}
	void setColorSizeOffset(int size, int offset, int stride = 0, int vboIndex = 0) {
		mColorByteSize = size;
		mColorOffset = offset;
		mColorStride = stride;
		mColorVboIndex = vboIndex;
	}
	void setIndexSizeOffset(int size, int offset,int stride,int count, int vboIndex = 0) {
		mIndexByteSize = size;
		mIndexOffset = offset;
		mIndexStride = stride;
		mCountOfIndex = count;
		mIndexVboIndex = vboIndex;
	}
	void setBoneIdSizeOffset(int size, int offset, int stride = 0, int vboIndex = 0) {
		mBoneIdByteSize = size;
		mBoneIdOffset = offset;
		if (mBoneIdByteSize>0) {
			mHasSkin = true;
		}
		mBoneIdStride = stride;
		mBoneIdVboIndex = vboIndex;
	}
	void setBoneWeightSizeOffset(int size, int offset, int stride = 0, int vboIndex = 0) {
		mBoneWeightByteSize = size;
		mBoneWeightOffset = offset;
		mBoneWeightStride = stride;
		mBoneWeightVboIndex = vboIndex;
	}

	//load bone id and bone weight into vbo
	/*bool loadBoneData(const int* pBoneIds, int idByteSize,
		const GLfloat* pWeights, int wByteSize, int drawType = GL_STATIC_DRAW);*/
	/*
	* offsetMatrix: bone offsetMatrix
	* nameIndexMap: get bone id from bone name(node name)
	*/
	//void initBoneInfo(std::vector<glm::mat4>&& offsetMatrix, std::unordered_map<std::string, int>&& nameIndexMap);
	/*
	* add SkeletonAnimation which can affect this mesh
	*/
	void addNodeAnimationAffectMe(const std::string& animatName) {
		mSkeletonAnimationsAffectMe.emplace_back(animatName);
	}

	const auto& getNodeAnimationAffectMe() {
		return mSkeletonAnimationsAffectMe;
	}

	virtual bool hasAnimation() {
		return mHasSkin;
	}

	void setAABB(float x1, float y1, float z1, float x2, float y2, float z2);

	/*const std::unordered_map<std::string, int>& getBoneNameIndex();

	const std::vector<glm::mat4>& getBonesOffsetMatrix();*/

	void setSkeleton(const std::shared_ptr<Skeleton>& ps);

	int pushVbo(const std::shared_ptr<Vbo>& pVbo);

	void setDrawType(int type) {
		mDrawType = static_cast<DrawType>(type);
	}

	//这四个函数都是创建vbo，ebo，并从内存上传数据到vbo的显存,如果之前存在vbo了，先删除
	//bool setPosData(const GLfloat* pos, int sizeInbyte, unsigned int drawType = GL_STATIC_DRAW);
	//bool setTexcoordData(const GLfloat* tex, int sizeInbyte, unsigned int drawType = GL_STATIC_DRAW);
	//bool setNormalData(const GLfloat* nor, int sizeInbyte, unsigned int drawType = GL_STATIC_DRAW);
	//bool setBoneIdData(const int* boneIds, int sizeInbyte, unsigned int drawType = GL_STATIC_DRAW);
	//bool setBoneWeightData(const GLfloat* weight, int sizeInbyte, unsigned int drawType = GL_STATIC_DRAW);
	bool setColorData(const GLfloat* nor, int sizeInbyte, unsigned int drawType = GL_STATIC_DRAW);
	//bool setIndexData(const GLuint* index, int indexByteSize, unsigned int drawType = GL_STATIC_DRAW);

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
		const GLint* pBoneId = nullptr, int boneIdSize = 0,
		const GLfloat* pBoneWeight = nullptr, int boneWeightSize = 0,
		int drawType = GL_STATIC_DRAW);

	

	//当做三角形绘制GL_TRIANGLES
	void drawTriangles(int posloc = -1, int texloc = -1, int norloc = -1,int colorloc = -1, int boneIdLoc = -1, int boneWeightLoc = -1);

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
	GLuint mVAO{ 0 };//这个是vao，顶点数组对象，opengles3.0才支持，是一个集合。把设定顶点属性的过程打包到一起，简化绘制流程。
	GLuint mColorVbo{ 0 };
	std::vector<std::shared_ptr<Vbo>> mpVbo;
	
	GLfloat mLineWidth{ 1.0f };

	int mposLocation{ -1 };//顶点的位置属性在shader中的location
	int mnorLocation{ -1 };//顶点的法向量属性在shader中的location
	int mtexLocation{ -1 };//顶点的纹理坐标属性在shader中的location
	int mBoneIdLocation{ -1 };
	int mBoneWeightLocation{ -1 };
	int mColorLoc{ -1 };
	
	int mPosByteSize{ 0 };
	int mCountOfVertex{ 0 };//vertex的个数，这里默认pos，texcoord，normal，color等属性的顶点个数都是一样的;
	int mPosStride{ 0 };
	int mPosOffset{ 0 };
	int mPosVboIndex{ 0 };

	int mTexByteSize{ 0 };
	int mTexOffset{ 0 };
	int mTexStride{ 0 };
	int mTexVboIndex{ 0 };

	int mNorByteSize{ 0 };
	int mNorOffset{ 0 };
	int mNorStride{ 0 };
	int mNorVboIndex{ 0 };

	int mColorByteSize{ 0 };
	int mColorStride{ 0 };
	int mColorOffset{ 0 };
	int mColorVboIndex{ 0 };

	int mBoneIdByteSize{ 0 };
	int mBoneIdStride{ 0 };
	int mBoneIdOffset{ 0 };
	int mBoneIdVboIndex{ 0 };

	int mBoneWeightByteSize{ 0 };
	int mBoneWeightStride{ 0 };
	int mBoneWeightOffset{ 0 };
	int mBoneWeightVboIndex{ 0 };

	int mIndexByteSize{ 0 };
	int mIndexStride{ 0 };
	int mIndexOffset{ 0 };
	int mCountOfIndex{0};
	int mIndexVboIndex{ 0 };
	std::string mMaterialName;//解析第三方的mesh格式的时候会设置这个名字，一个mesh文件里面可能会有多个mesh
							//每个mesh的material是不一样的，默认把mesh的albedoMap文件的名字作为material名字，
							//解析完了mesh文件之后，再遍历一遍mesh，根据名字找到对应的material。
	MeshType mMeshType{ MeshType::None };
	DrawType mDrawType{ DrawType::Triangles };
	unsigned int mId{ 0 };
	std::shared_ptr<Material> mpMaterial;
	std::unique_ptr<AABB> mpAabb;
	std::unique_ptr< glm::mat4> mpPreMvpMatrix;

	//node name to boneIndex
	//std::unordered_map<std::string, int> mBoneNameIndex;
	//offsetMatrix and finalMatrix should be placed in SkeletonAnimation
	//because a SkeletonAnimation may be affect several meshes;
	//it data is shared by these meshes;
	//std::vector<glm::mat4> mBonesOffsetMatrix;//这个矩阵可能是几个mesh共用的
	
	std::vector<std::string> mSkeletonAnimationsAffectMe;
	std::shared_ptr<Skeleton> mpSkeleton;
	bool mHasSkin{ false };

	//如果函数内部创建了vao就返回true
	bool createVaoIfNeed(int posloc=-1, int texloc=-1, int norloc=-1,int colorLoc=-1);
};
using MeshSP = std::shared_ptr<Mesh>;

