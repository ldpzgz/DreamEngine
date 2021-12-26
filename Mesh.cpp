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
#include <fstream>
#include "LdpMesh.h"
//extern void checkglerror();
void checkglerror()
{
	/*
	GL_INVALID_ENUM, 0x0500-----1280
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

Mesh::Mesh(MeshType meshType) noexcept:
	mMeshType(meshType)
{
	// TODO Auto-generated constructor stub

}

Mesh::Mesh(MeshType meshType, DrawType drawType) noexcept:
	mMeshType(meshType),
	mDrawType(drawType)
{

}

void Mesh::reset() {
	mPosVbo = 0;
	mTexVbo = 0;
	mNorVbo = 0;
	mTangentVbo = 0;
	mBiNormalVbo = 0;
	mColorVbo = 0;
	mIndexVbo = 0;
	mposLocation = 0;
	mtexLocation = 0;
	mnorLocation = 0;
	mVAO = 0;
	mLineWidth = 1.0f;
	mPosByteSize = 0;
	mNorByteSize = 0;
	mColorByteSize = 0;
	mTexByteSize = 0;
	mIndexByteSize = 0;
	mTangentByteSize = 0;
	mBiNormalByteSize = 0;
	mMeshType = MeshType::MESH_None;
	mCountOfVertex = 0;
	mDrawType = DrawType::Triangles;
}

Mesh::Mesh(const Mesh&& temp) noexcept:
	mPosVbo(temp.mPosVbo),
	mTexVbo(temp.mTexVbo),
	mNorVbo(temp.mNorVbo),
	mColorVbo(temp.mColorVbo),
	mIndexVbo(temp.mIndexVbo),
	mIndexByteSize(temp.mIndexByteSize),
	mPosByteSize(temp.mPosByteSize),
	mNorByteSize(temp.mNorByteSize),
	mColorByteSize(temp.mColorByteSize),
	mTexByteSize(temp.mTexByteSize),
	mMeshType(temp.mMeshType),
	mposLocation(temp.mposLocation),
	mtexLocation(temp.mtexLocation),
	mnorLocation(temp.mnorLocation),
	mVAO(temp.mVAO),
	mLineWidth(temp.mLineWidth),
	mCountOfVertex(temp.mCountOfVertex),
	mDrawType(temp.mDrawType)
{
}

Mesh::~Mesh()
{
	// TODO Auto-generated destructor stub
	unLoadMesh();
}

//void Mesh::loadUBS(const std::vector<float>& points) {
//	if (mMeshType == MeshType::MESH_Uniform_B_Curves) {
//		int index = 1;
//		auto size = points.size();
//		auto endIndex = size / 3;
//		std::vector<float> pos;
//
//		glm::mat4x4 Mh(
//			1.0f, -3.0f, 3.0f, -1.0f,
//			4.0f, 0.0f, -6.0f, 3.0f,
//			1.0f, 3.0f, 3.0f, -3.0f,
//			0.0f, 0.0f, 0.0f, 1.0f
//		);
//		Mh *= (1.0f/6.0f);
//		
//
//		while (index <= endIndex - 3) {
//
//			glm::mat3x4 G(
//				points[3 * (index - 1)], points[3 * index], points[3 * (index + 1)], points[3 * (index + 2)],
//				points[3 * (index - 1) + 1], points[3 * index + 1], points[3 * (index + 1) + 1], points[3 * (index + 2) + 1],
//				points[3 * (index - 1) + 2], points[3 * index + 2], points[3 * (index + 1) + 2], points[3 * (index + 2) + 2]
//			);
//
//			for (int i = 0; i < 50; ++i) {
//				glm::vec4 t;
//				t.x = 1.0f;
//				t.y = i*0.02f;
//				t.z = t.y*t.y;
//				t.w = t.z*t.y;
//
//				auto v = t*Mh*G;
//				pos.emplace_back(v.x);
//				pos.emplace_back(v.y);
//				pos.emplace_back(v.z);
//			}
//			++index;
//		}
//		//for (auto xyz : points) {
//		//	pos.emplace_back(xyz);
//		//}
//		bool b = createBufferObject(pos.data(), sizeof(float)*pos.size(), nullptr, 0);
//		if (!b)
//		{
//			LOGD("error to loadCRSplines\n");
//		}
//	}
//}

void Mesh::loadMesh()
{
	if (mMeshType == MeshType::MESH_Rectangle)
	{
		std::vector<Vec3> pos{ 
			{1.0f,1.0f,0.0f},
			{0.0f,1.0f,0.0f},
			{0.0f,0.0f,0.0f},
			{1.0f,0.0f,0.0f},
			 };
		GLuint indexes[] = { 0,1,2,0,2,3 };
		GLfloat tex[] = { 1.0f,1.0f,0.0f,1.0f,0.0f,0.0f,1.0f,0.0f };
		createBufferObject((GLfloat*)pos.data(), pos.size()*sizeof(Vec3),4, indexes, sizeof(indexes), tex, sizeof(tex));
	}
	else if (mMeshType == MeshType::MESH_Triangle) {
		GLfloat pos[] = { 0.0f,1.0f,0.0f,
			-1.0f,-1.0f,0.0f,
			1.0f,-1.0f,0.0f};
		GLuint indexes[] = { 0,1,2 };
		GLfloat tex[] = { 0.5f,1.0f,0.0f,0.0f,1.0f,0.0f };
		createBufferObject(pos, sizeof(pos), 3,indexes, sizeof(indexes), tex, sizeof(tex));
	}
	else if (mMeshType == MeshType::MESH_Cuboid) {
		GLfloat pos[] = { 
			-1.0f,-1.0f,1.0f,
			1.0f,-1.0f,1.0f,
			1.0f,1.0f,1.0f,
			-1.0f,1.0f,1.0f,

			-1.0f,1.0f,1.0f,
			1.0f,1.0f,1.0f,
			1.0f,1.0f,-1.0f,
			-1.0f,1.0f,-1.0f,

			-1.0f,1.0f,1.0f,
			-1.0f,1.0f,-1.0f,
			-1.0f,-1.0f,-1.0f,
			-1.0f,-1.0f,1.0f,

			1.0f,-1.0f,1.0f,
			1.0f,-1.0f,-1.0f,
			1.0f,1.0f,-1.0f,
			1.0f,1.0f,1.0f,

			-1.0f,-1.0f,-1.0f,
			1.0f,-1.0f,-1.0f,
			1.0f,-1.0f,1.0f,
			-1.0f,-1.0f,1.0f,

			1.0f,-1.0f,-1.0f,
			-1.0f,-1.0f,-1.0f,
			-1.0f,1.0f,-1.0f,
			1.0f,1.0f,-1.0f
		};
		GLuint indexes[] = {
			0,1,2,0,2,3,
			4,5,6,4,6,7, 
			8,9,10,8,10,11,
			12,13,14,12,14,15,
			16,17,18,16,18,19,
			20,21,22,20,22,23
		};
		GLfloat tex[] = { 
			0.0f,0.0f,1.0f,0.0f,1.0f,1.0f,0.0f,1.0f,
			0.0f,0.0f,1.0f,0.0f,1.0f,1.0f,0.0f,1.0f,
			0.0f,0.0f,1.0f,0.0f,1.0f,1.0f,0.0f,1.0f,
			0.0f,0.0f,1.0f,0.0f,1.0f,1.0f,0.0f,1.0f,
			0.0f,0.0f,1.0f,0.0f,1.0f,1.0f,0.0f,1.0f,
			0.0f,0.0f,1.0f,0.0f,1.0f,1.0f,0.0f,1.0f
		};
		createBufferObject(pos, sizeof(pos), 24,indexes, sizeof(indexes), tex, sizeof(tex));
	}
}

void Mesh::loadMesh(const std::vector<Vec3>& pos, const std::vector<Vec3ui>& index) {
	createBufferObject((float*)pos.data(), pos.size() * sizeof(Vec3), pos.size(), (GLuint*)index.data() , index.size()*sizeof(Vec3ui));
}

bool Mesh::loadMesh(const std::string meshFilePath) {
	std::ifstream infile;
	infile.open(meshFilePath, std::ifstream::in | std::ifstream::binary);
	if (infile.good()) {
		LdpMesh mesh;
		infile.read((char*)&mesh, sizeof(mesh));
		if (infile.gcount() == sizeof(mesh)) {
			//创建mesh的aabb
			mpAabb = make_unique<AABB>(mesh.xmin,mesh.xmax,mesh.ymin,mesh.ymax,mesh.zmin,mesh.zmax);
			//获取mesh的材质的名字
			if (mesh.materialNameSize > 0) {
				mMaterialName.resize(mesh.materialNameSize);
				infile.read(mMaterialName.data(), mesh.materialNameSize);
				assert(infile.gcount() == mesh.materialNameSize);
				mpMaterial = Material::getMaterial(mMaterialName);
				if (!mpMaterial) {
					LOGE("ERROR to find material %s,when load mesh from file", mMaterialName.c_str());
				}
			}
			std::vector<float> pos;
			std::vector<float> texcoord;
			std::vector<float> normal;
			std::vector<float> tangent;
			//std::vector<float> bitangent;
			std::vector<unsigned int> index;

			//读取顶点数据
			if (mesh.vertexCount > 0) {
				pos.reserve(mesh.vertexCount*3);
				infile.read((char*)pos.data(), mesh.vertexLength);
				assert(infile.gcount() == mesh.vertexLength);
			}
			//读取纹理坐标数据
			if (mesh.texcoordLength > 0) {
				texcoord.reserve(mesh.vertexCount * 2);
				infile.read((char*)texcoord.data(), mesh.texcoordLength);
				assert(infile.gcount() == mesh.texcoordLength);
			}
			//读取法线数据
			if (mesh.normalLength > 0) {
				normal.reserve(mesh.vertexCount * 3);
				infile.read((char*)normal.data(), mesh.normalLength);
				assert(infile.gcount() == mesh.normalLength);
			}

			if (mesh.tangentsLength > 0) {
				tangent.reserve(mesh.vertexCount * 3);
				infile.read((char*)tangent.data(), mesh.tangentsLength);
				assert(infile.gcount() == mesh.tangentsLength);
			}

			/*if (mesh.bitangentsLength > 0) {
				bitangent.reserve(mesh.vertexCount * 3);
				infile.read((char*)bitangent.data(), mesh.bitangentsLength);
				assert(infile.gcount() == mesh.bitangentsLength);
			}*/
			//读取索引数据
			if (mesh.indexLength > 0) {
				index.reserve(mesh.indexLength/sizeof(unsigned int));
				infile.read((char*)index.data(), mesh.indexLength);
				assert(infile.gcount() == mesh.indexLength);
			}

			createBufferObject(pos.data(), mesh.vertexLength, mesh.vertexCount, 
				index.data(), mesh.indexLength, 
				texcoord.data(), mesh.texcoordLength,
				normal.data(),mesh.normalLength,
				nullptr,0,
				tangent.data(),mesh.tangentsLength);
		}
		return true;
	}
	return false;
}

bool Mesh::createBufferObject(GLfloat* pos,int posByteSize, int countOfVertex, 
	GLuint* index,int indexByteSize,
	GLfloat* tex,int texByteSize,
	GLfloat* nor,int norByteSize, 
	GLfloat* color, int colorByteSize,
	GLfloat* tangent, int tangentByteSize,
	int drawType)
{
	if(pos != nullptr && posByteSize>0)
	{
		mCountOfVertex = countOfVertex;
		setPosData(pos,posByteSize, drawType);
		checkglerror();
	}
	if(tex != nullptr && texByteSize > 0)
	{
		setTexcoordData(tex,texByteSize, drawType);
	}
	if(nor != nullptr && norByteSize > 0)
	{
		setNormalData(nor,norByteSize, drawType);
	}
	if (color != nullptr && colorByteSize > 0) {
		setColorData(color, colorByteSize, drawType);
	}
	if (tangent != nullptr && tangentByteSize > 0) {
		setTangentData(tangent, tangentByteSize, drawType);
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
bool Mesh::updataPos(float* pos, int byteOffset, int size,int numOfVertex)
{
	if (size + byteOffset > mPosByteSize) {
		if (byteOffset > 0) {
			LOGE("ERROR to update mesh pos data, the size + byteOffset is greater then vbo size");
			return false;
		}
		mCountOfVertex = numOfVertex;
		setPosData(pos, size);
	}
	else {
		glBindBuffer(GL_ARRAY_BUFFER, mPosVbo);
		glBufferSubData(GL_ARRAY_BUFFER, byteOffset, size, pos);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	
	return true;
}

bool Mesh::updateColor(float* color, int offsetInByte, int sizeInByteToBeReplaced) {
	if (sizeInByteToBeReplaced + offsetInByte > mColorByteSize)
	{
		if (offsetInByte > 0) {
			LOGE("ERROR to update mesh color data, the size + byteOffset is greater then vbo size");
			return false;
		}
		setColorData(color, sizeInByteToBeReplaced);
	}
	glBindBuffer(GL_ARRAY_BUFFER, mColorVbo);
	glBufferSubData(GL_ARRAY_BUFFER, offsetInByte, sizeInByteToBeReplaced, color);
	checkglerror();
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}

bool Mesh::updateNormal(float* normal, int byteOffset, int size) {
	if (size + byteOffset > mNorByteSize)
	{
		if (byteOffset > 0) {
			LOGE("ERROR to update mesh normal data, the size + byteOffset is greater then vbo size");
			return false;
		}
		setNormalData(normal, size);
	}
	glBindBuffer(GL_ARRAY_BUFFER, mNorVbo);
	glBufferSubData(GL_ARRAY_BUFFER, byteOffset, size, normal);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}

//bool Mesh::updateBiNormal(float* biNormal, int byteOffset, int size) {
//	if (size + byteOffset > mBiNormalByteSize)
//	{
//		if (byteOffset > 0) {
//			LOGE("ERROR to update mesh normal data, the size + byteOffset is greater then vbo size");
//			return false;
//		}
//		setBiTangentData(biNormal, size);
//	}
//	glBindBuffer(GL_ARRAY_BUFFER, mBiNormalVbo);
//	glBufferSubData(GL_ARRAY_BUFFER, byteOffset, size, biNormal);
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//	return true;
//}

bool Mesh::updateTangent(float* tangent, int byteOffset, int size) {
	if (size + byteOffset > mTangentByteSize)
	{
		if (byteOffset > 0) {
			LOGE("ERROR to update mesh normal data, the size + byteOffset is greater then vbo size");
			return false;
		}
		setNormalData(tangent, size);
	}
	glBindBuffer(GL_ARRAY_BUFFER, mTangentVbo);
	glBufferSubData(GL_ARRAY_BUFFER, byteOffset, size, tangent);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}
//更新纹理坐标vbo
bool Mesh::updataTexcoord(float* tex, int byteOffset, int size)
{
	if (size + byteOffset > mTexByteSize)
	{
		if (byteOffset > 0) {
			LOGE("ERROR to update mesh texcoord data, the size + byteOffset is greater then vbo size");
			return false;
		}
		setTexcoordData(tex, size);
	}
	glBindBuffer(GL_ARRAY_BUFFER, mTexVbo);
	glBufferSubData(GL_ARRAY_BUFFER, byteOffset, size, tex);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;
}
//更新索引vbo
bool Mesh::updataIndex(GLuint* pIndex, int byteOffset, int size)
{
	if (size + byteOffset > mIndexByteSize)
	{
		if (byteOffset > 0) {
			LOGE("ERROR to update mesh index data, the size + byteOffset is greater then vbo size");
			return false;
		}
		setIndexData(pIndex, size);
	}
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
			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexVbo);
			glEnableVertexAttribArray(posloc);
			assert(mCountOfVertex != 0);
			int componentOfPos = mPosByteSize / (sizeof(GLfloat) * mCountOfVertex);
			glVertexAttribPointer(posloc, componentOfPos, GL_FLOAT, GL_FALSE, 0, 0);
		}
		else {

		}
		glBindVertexArray(0);
	}
	
	glBindVertexArray(mVAO);
	glLineWidth(mLineWidth);
	glDrawArrays(GL_LINE_STRIP, 0, mCountOfVertex);
	glBindVertexArray(0);
	//glDrawElements(GL_LINE_LOOP, mNumOfIndex, GL_UNSIGNED_INT, (const void*)0);
}

void Mesh::drawTriangleFan(int posloc, int texloc,int norloc,int colorloc, int tangentloc)
{
	//glFrontFace(GL_CW);
	if (createVaoIfNeed(posloc)){
		glBindVertexArray(mVAO);
		if (posloc >= 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER, mPosVbo);
			glEnableVertexAttribArray(posloc);
			assert(mCountOfVertex != 0);
			int componentOfPos = mPosByteSize / (sizeof(GLfloat) * mCountOfVertex);
			glVertexAttribPointer(posloc, componentOfPos, GL_FLOAT, GL_FALSE, 0, 0);
		}
		if (texloc >= 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER, mTexVbo);
			//indicate a vertexAttrib space 2*float,in mTexVbo
			glEnableVertexAttribArray(texloc);
			int componentOfTexcoord = mTexByteSize / (sizeof(GLfloat) * mCountOfVertex);
			glVertexAttribPointer(texloc, componentOfTexcoord, GL_FLOAT, GL_FALSE, 0, 0);

		}

		if (norloc >= 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER, mNorVbo);
			glEnableVertexAttribArray(norloc);
			int componentOfNormal = mNorByteSize / (sizeof(GLfloat) * mCountOfVertex);
			glVertexAttribPointer(norloc, componentOfNormal, GL_FLOAT, GL_FALSE, 0, 0);

		}

		if (tangentloc > 0) {
			glBindBuffer(GL_ARRAY_BUFFER, mTangentVbo);
			glEnableVertexAttribArray(tangentloc);
			int componentOfTangent = mTangentByteSize / (sizeof(GLfloat) * mCountOfVertex);
			glVertexAttribPointer(tangentloc, componentOfTangent, GL_FLOAT, GL_FALSE, 0, 0);
		}

		
		//这个好像不用绑定了？
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexVbo);

		glBindVertexArray(0);
	}

	glBindVertexArray(mVAO);
	if (colorloc >= 0) {
		glBindBuffer(GL_ARRAY_BUFFER, mColorVbo);
		glEnableVertexAttribArray(colorloc);
		int componentOfColor = mColorByteSize / (sizeof(GLfloat) * mCountOfVertex);
		glVertexAttribPointer(colorloc, componentOfColor, GL_FLOAT, GL_FALSE, 0, 0);
	}
	glDrawArrays(GL_TRIANGLE_FAN, 0, mCountOfVertex);
	glBindVertexArray(0);
	//glFrontFace(GL_CCW);
	//glDrawElements(GL_TRIANGLE_FAN, mNumOfIndex, GL_UNSIGNED_SHORT, (const void*)0);
}


void Mesh::drawTriangles(int posloc,int texloc,int norloc,int colorloc, int tangentloc)
{
	if (createVaoIfNeed(posloc)) {
		glBindVertexArray(mVAO);
		if (posloc >= 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER, mPosVbo);
			glEnableVertexAttribArray(posloc);
			assert(mCountOfVertex != 0);
			int componentOfPos = mPosByteSize / (sizeof(GLfloat) * mCountOfVertex);
			glVertexAttribPointer(posloc, componentOfPos, GL_FLOAT, GL_FALSE, 0, 0);
			//glVertexAttribDivisor(posloc, 1);//这个函数一调用，shader 里面posloc这个位置的顶点属性，就会变成uniform属性了，
			//渲染一个instance，只取一个值出来，渲染下一个instance的时候再取下一个值出来。
		}

		if (texloc >= 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER, mTexVbo);
			glEnableVertexAttribArray(texloc);
			int componentOfTexcoord = mTexByteSize / (sizeof(GLfloat) * mCountOfVertex);
			//indicate a vertexAttrib space 2*float,in mTexVbo
			glVertexAttribPointer(texloc, componentOfTexcoord, GL_FLOAT, GL_FALSE, 0, 0);

		}
		if (norloc >= 0)
		{
			glBindBuffer(GL_ARRAY_BUFFER, mNorVbo);
			glEnableVertexAttribArray(norloc);
			int componentOfNormal = mNorByteSize / (sizeof(GLfloat) * mCountOfVertex);
			glVertexAttribPointer(norloc, componentOfNormal, GL_FLOAT, GL_FALSE, 0, 0);

		}

		if (tangentloc > 0) {
			glBindBuffer(GL_ARRAY_BUFFER, mTangentVbo);
			glEnableVertexAttribArray(tangentloc);
			int componentOfTangent = mTangentByteSize / (sizeof(GLfloat) * mCountOfVertex);
			glVertexAttribPointer(tangentloc, componentOfTangent, GL_FLOAT, GL_FALSE, 0, 0);
		}

		if (colorloc >= 0) {
			glBindBuffer(GL_ARRAY_BUFFER, mColorVbo);
			glEnableVertexAttribArray(colorloc);
			int componentOfColor = mColorByteSize / (sizeof(GLfloat) * mCountOfVertex);
			glVertexAttribPointer(colorloc, componentOfColor, GL_FLOAT, GL_FALSE, 0, 0);
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexVbo);//glDrawElements会用到这个
		glBindVertexArray(0);
	}
	
	glBindVertexArray(mVAO);
	glDrawElements(GL_TRIANGLES, mIndexByteSize/sizeof(GLuint), GL_UNSIGNED_INT, (const void*)0);
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

void Mesh::draw(int posloc, int texloc, int norloc, int colorloc, int tangentloc)
{
	if (mDrawType == DrawType::Triangles)
	{
		drawTriangles(posloc, texloc, norloc,colorloc, tangentloc);
	}
	else if (mDrawType == DrawType::TriangleFan)
	{
		drawTriangleFan(posloc, texloc, norloc, colorloc, tangentloc);
	}
	else if (mDrawType == DrawType::LineStrip)
	{
		drawLineStrip(posloc);
	}
}

void Mesh::render(const glm::mat4& mvpMat, const glm::mat4& mvMat, const Vec3& lightPos, const Vec3& viewPos) {
	if (mpMaterial) {
		auto& pShader = mpMaterial->getShader();
		if (pShader) {
			pShader->setMvpMatrix(mvpMat);
			pShader->setMvMatrix(mvMat);
			//pShader->setViewMatrix(viewMat);
			//pShader->setTextureMatrix();
			pShader->setLightPos(lightPos);
			pShader->setViewPos(viewPos);

			mpMaterial->enable();
			mpMaterial->setMyRenderOperation();
			int posloc = -1;
			int texloc = -1;
			int norloc = -1;
			int colorloc = -1;
			int tangentloc = -1;
			pShader->getLocation(posloc, texloc, colorloc, norloc,tangentloc);
			draw(posloc, texloc, norloc, colorloc,tangentloc);
			mpMaterial->restoreRenderOperation();
		}
		else {
			LOGE("mesh has no shader,can't render");
		}
		
	}
	else {
		//LOGE("mesh has no material,can't render");
	}
}

void Mesh::render(const glm::mat4& mvpMat, const glm::mat4& texMat) {
	if (mpMaterial) {
		auto& pShader = mpMaterial->getShader();
		if (pShader) {
			pShader->setMvpMatrix(mvpMat);
			pShader->setTextureMatrix(texMat);
			mpMaterial->enable();
			mpMaterial->setMyRenderOperation();
			int posloc = -1;
			int texloc = -1;
			int norloc = -1;
			int colorloc = -1;
			int tangentloc = -1;
			pShader->getLocation(posloc, texloc, colorloc, norloc, tangentloc);
			draw(posloc, texloc, norloc, colorloc, tangentloc);
			mpMaterial->restoreRenderOperation();
		}
		else {
			LOGE("mesh has no shader 2,can't render");
		}
	}
	else {
		LOGE("mesh has no material,can't render");
	}
}

void Mesh::render(const glm::mat4& mvpMat) {
	if (mpMaterial) {
		auto& pShader = mpMaterial->getShader();
		if (pShader) {
			pShader->setMvpMatrix(mvpMat);
			mpMaterial->enable();
			mpMaterial->setMyRenderOperation();
			int posloc = -1;
			int texloc = -1;
			int norloc = -1;
			int colorloc = -1;
			int tangentloc = -1;
			pShader->getLocation(posloc, texloc, colorloc, norloc, tangentloc);
			draw(posloc, texloc, norloc, colorloc, tangentloc);
			mpMaterial->restoreRenderOperation();
		}
		else {
			LOGE("mesh has no shader 3,can't render");
		}
	}
	else {
		LOGE("mesh has no material,can't render");
	}
}


bool Mesh::setPosData(GLfloat* pos, int size, unsigned int drawType)
{
	if (mPosVbo > 0) {
		glDeleteBuffers(1, &mPosVbo);
		if (mVAO != 0) {
			//先删除原来的vao
			glDeleteVertexArrays(1, &mVAO);
			mVAO = 0;
		}
	}
	glGenBuffers(1, &mPosVbo);
	glBindBuffer(GL_ARRAY_BUFFER, mPosVbo);
	glBufferData(GL_ARRAY_BUFFER, size, pos, drawType);
	mPosByteSize = size;
	return true;
}
bool Mesh::setTexcoordData(GLfloat* tex, int size, unsigned int drawType)
{
	if (mTexVbo > 0) {
		glDeleteBuffers(1, &mTexVbo);
		if (mVAO != 0) {
			//先删除原来的vao
			glDeleteVertexArrays(1, &mVAO);
			mVAO = 0;
		}
	}
	glGenBuffers(1, &mTexVbo);
	glBindBuffer(GL_ARRAY_BUFFER, mTexVbo);
	glBufferData(GL_ARRAY_BUFFER, size, tex, drawType);
	mTexByteSize = size;
	return true;
}

bool Mesh::setNormalData(GLfloat* nor, int sizeInbyte, unsigned int drawType)
{
	if (mNorVbo > 0) {
		glDeleteBuffers(1, &mNorVbo);
		if (mVAO != 0) {
			//先删除原来的vao
			glDeleteVertexArrays(1, &mVAO);
			mVAO = 0;
		}
	}
	glGenBuffers(1, &mNorVbo);
	glBindBuffer(GL_ARRAY_BUFFER, mNorVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeInbyte, nor, drawType);
	mNorByteSize = sizeInbyte;
	return true;
}

bool Mesh::setTangentData(GLfloat* tangent, int sizeInbyte, unsigned int drawType) {
	if (mTangentVbo > 0) {
		glDeleteBuffers(1, &mTangentVbo);
		if (mVAO != 0) {
			//先删除原来的vao
			glDeleteVertexArrays(1, &mVAO);
			mVAO = 0;
		}
	}
	glGenBuffers(1, &mTangentVbo);
	glBindBuffer(GL_ARRAY_BUFFER, mTangentVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeInbyte, tangent, drawType);
	mTangentByteSize = sizeInbyte;
	return true;
}

//bool Mesh::setBiTangentData(GLfloat* binormal, int sizeInbyte, unsigned int drawType) {
//	if (mBiNormalVbo > 0) {
//		glDeleteBuffers(1, &mBiNormalVbo);
//		if (mVAO != 0) {
//			//先删除原来的vao
//			glDeleteVertexArrays(1, &mVAO);
//			mVAO = 0;
//		}
//	}
//	glGenBuffers(1, &mBiNormalVbo);
//	glBindBuffer(GL_ARRAY_BUFFER, mBiNormalVbo);
//	glBufferData(GL_ARRAY_BUFFER, sizeInbyte, binormal, drawType);
//	mBiNormalByteSize = sizeInbyte;
//	return true;
//}

bool Mesh::setColorData(GLfloat* nor, int size, unsigned int drawType)
{
	if (mColorVbo > 0) {
		glDeleteBuffers(1, &mColorVbo);
		if (mVAO != 0) {
			//先删除原来的vao
			glDeleteVertexArrays(1, &mVAO);
			mVAO = 0;
		}
	}
	glGenBuffers(1, &mColorVbo);
	glBindBuffer(GL_ARRAY_BUFFER, mColorVbo);
	glBufferData(GL_ARRAY_BUFFER, size, nor, drawType);
	mColorByteSize = size;
	return true;
}

bool Mesh::setIndexData(GLuint* index, int size, unsigned int drawType)
{
	if (mIndexVbo > 0) {
		glDeleteBuffers(1, &mIndexVbo);
	}
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

	if (mTangentVbo != 0)
	{
		glDeleteBuffers(1, &mTangentVbo);
		mTangentVbo = 0;
		mTangentByteSize = 0;
	}

	if (mBiNormalVbo != 0)
	{
		glDeleteBuffers(1, &mBiNormalVbo);
		mBiNormalVbo = 0;
		mBiNormalByteSize = 0;
	}

	if (mTexVbo != 0)
	{
		glDeleteBuffers(1, &mTexVbo);
		mTexVbo = 0;
		mTexByteSize = 0;
	}
	if (mColorVbo != 0) {
		glDeleteBuffers(1, &mColorVbo);
		mColorVbo = 0;
		mColorByteSize = 0;
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
	mCountOfVertex = 0;
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
