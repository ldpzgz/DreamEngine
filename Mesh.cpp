/*
 * Mesh.cpp
 *
 *  Created on: 2015-9-19
 *      Author: Administrator
 */
//#include "StdAfx.h"
#include "Mesh.h"
#include "Log.h"
#include "LdpMesh.h"
#include "Shader.h"
#include "Resource.h"
#include "aabb.h"
#include "Material.h"
#include "Ubo.h"
#include "animation/SkeletonAnimation.h"
#include "animation/Skeleton.h"
#include "Vbo.h"

#include <glm/trigonometric.hpp>  //sin cos,tan,radians,degree
#include <glm/ext/matrix_transform.hpp> // perspective, translate, rotate
#include <glm/gtc/type_ptr.hpp> // value_ptr,make_vec,make_mat

#include <cmath>
#include <fstream>

Mesh::Mesh(MeshType meshType):
	mMeshType(meshType)
{
	// TODO Auto-generated constructor stub

}

Mesh::Mesh(MeshType meshType, DrawType drawType):
	mMeshType(meshType),
	mDrawType(drawType)
{

}

void Mesh::reset() {
	/*mPosVbo = 0;
	mTexVbo = 0;
	mNorVbo = 0;
	mColorVbo = 0;
	mIndexVbo = 0;*/
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
	mMeshType = MeshType::None;
	mCountOfVertex = 0;
	mDrawType = DrawType::Triangles;
}

Mesh::Mesh(Mesh&& temp) noexcept:
	/*mPosVbo(temp.mPosVbo),
	mTexVbo(temp.mTexVbo),
	mNorVbo(temp.mNorVbo),
	mColorVbo(temp.mColorVbo),
	mIndexVbo(temp.mIndexVbo),*/
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
	mpVbo(temp.mpVbo),
	mLineWidth(temp.mLineWidth),
	mCountOfVertex(temp.mCountOfVertex),
	mDrawType(temp.mDrawType),
	mSkeletonAnimationsAffectMe(std::move(temp.mSkeletonAnimationsAffectMe)),
	mpSkeleton(temp.mpSkeleton),
	mHasSkin(temp.mHasSkin),
	mpMaterial(temp.mpMaterial),
	mpAabb(std::move(temp.mpAabb)),
	mpPreMvpMatrix(std::move(temp.mpPreMvpMatrix))
{
	temp.mVAO = 0;
	for (auto& vbo : temp.mpVbo) {
		vbo.reset();
	}
	temp.mpSkeleton = nullptr;
}

Mesh::~Mesh()
{
	// TODO Auto-generated destructor stub
	unLoadMesh();
}

//void Mesh::loadUBS(const std::vector<float>& points) {
//	if (mMeshType == MeshType::UniformBCurves) {
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
	if (mMeshType == MeshType::Rectangle)
	{
		std::vector<glm::vec3> pos{ 
			{1.0f,1.0f,0.0f},
			{0.0f,1.0f,0.0f},
			{0.0f,0.0f,0.0f},
			{1.0f,0.0f,0.0f},
			 };
		std::vector<glm::vec3> nor{
			{0.0f,0.0f,1.0f},
			{0.0f,0.0f,1.0f},
			{0.0f,0.0f,1.0f},
			{0.0f,0.0f,1.0f},
		};
		GLuint indexes[] = { 0,1,2,0,2,3 };
		GLfloat tex[] = { 1.0f,1.0f,0.0f,1.0f,0.0f,0.0f,1.0f,0.0f };
		createBufferObject((GLfloat*)pos.data(), pos.size()*sizeof(glm::vec3),4, 
			indexes, sizeof(indexes), 
			tex, sizeof(tex),(GLfloat*)nor.data(), nor.size() * sizeof(glm::vec3));
	}
	else if (mMeshType == MeshType::TrianglePost) {
		std::vector<glm::vec3> pos{
			{-1.0f,3.0f,0.0f},
			{-1.0f,-1.0f,0.0f},
			{3.0f,-1.0f,0.0f}
		};
		GLfloat tex[] = { 0.0f,2.0f,0.0f,0.0f,2.0f,0.0f };
		GLuint indexes[] = { 0,1,2 };
		createBufferObject((GLfloat*)pos.data(), pos.size() * sizeof(glm::vec3), 3,
			indexes, sizeof(indexes),
			tex, sizeof(tex));
	}
	else if (mMeshType == MeshType::Quad) {
		std::vector<glm::vec3> pos{
			{1.0f,1.0f,0.0f},
			{-1.0f,1.0f,0.0f},
			{-1.0f,-1.0f,0.0f},
			{1.0f,-1.0f,0.0f},
		};
		std::vector<glm::vec3> nor{
			{0.0f,0.0f,1.0f},
			{0.0f,0.0f,1.0f},
			{0.0f,0.0f,1.0f},
			{0.0f,0.0f,1.0f},
		};
		GLuint indexes[] = { 0,1,2,0,2,3 };
		GLfloat tex[] = { 1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,1.0f,1.0f };
		createBufferObject((GLfloat*)pos.data(), pos.size() * sizeof(glm::vec3), 4,
			indexes, sizeof(indexes),
			tex, sizeof(tex), (GLfloat*)nor.data(), nor.size() * sizeof(glm::vec3));
	}
	else if (mMeshType == MeshType::Triangle) {
		GLfloat pos[] = { 0.0f,1.0f,0.0f,
			-1.0f,-1.0f,0.0f,
			1.0f,-1.0f,0.0f};
		GLuint indexes[] = { 0,1,2 };
		GLfloat tex[] = { 0.5f,1.0f,0.0f,0.0f,1.0f,0.0f };
		createBufferObject(pos, sizeof(pos), 3,indexes, sizeof(indexes), tex, sizeof(tex));
	}
	else if (mMeshType == MeshType::Cuboid) {
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
	else if (mMeshType == MeshType::Shpere) {
		mDrawType = DrawType::TriangleStrip;
		std::vector<glm::vec3> positions;
		std::vector<glm::vec2> uv;
		std::vector<glm::vec3> normals;
		std::vector<unsigned int> indices;

		const unsigned int X_SEGMENTS = 64;
		const unsigned int Y_SEGMENTS = 64;
		const float PI = 3.14159265359f;
		for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
		{
			for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
			{
				float xSegment = (float)x / (float)X_SEGMENTS;
				float ySegment = (float)y / (float)Y_SEGMENTS;
				float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
				float yPos = std::cos(ySegment * PI);
				float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

				positions.emplace_back(xPos, yPos, zPos);
				uv.emplace_back(xSegment, ySegment);
				normals.emplace_back(xPos, yPos, zPos);
			}
		}

		bool oddRow = false;
		for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
		{
			if (!oddRow) // even rows: y == 0, y == 2; and so on
			{
				for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
				{
					indices.emplace_back(y * (X_SEGMENTS + 1) + x);
					indices.emplace_back((y + 1) * (X_SEGMENTS + 1) + x);
				}
			}
			else
			{
				for (int x = X_SEGMENTS; x >= 0; --x)
				{
					indices.emplace_back((y + 1) * (X_SEGMENTS + 1) + x);
					indices.emplace_back(y * (X_SEGMENTS + 1) + x);
				}
			}
			oddRow = !oddRow;
		}
		createBufferObject((GLfloat*)positions.data(), positions.size() * sizeof(glm::vec3), 
			positions.size(), indices.data(), indices.size() * sizeof(unsigned int), 
			(GLfloat*)uv.data(), uv.size()*sizeof(glm::vec2),
			(GLfloat*)normals.data(),normals.size() * sizeof(glm::vec3));
	}
}

void Mesh::loadMesh(const std::vector<glm::vec3>& pos, const std::vector<glm::uvec3>& index) {
	createBufferObject((float*)pos.data(), pos.size() * sizeof(glm::vec3), pos.size(), (GLuint*)index.data() , index.size()*sizeof(glm::uvec3));
}

bool Mesh::loadMesh(const std::vector<float>& pos,
	const std::vector<float>& texcoord,
	const std::vector<float>& normal,
	const std::vector<unsigned int>& index) {
	float xmin, xmax, ymin, ymax, zmin, zmax;
	if (pos.size() >= 9) {
		xmin = xmax = pos[0];
		ymin = ymax = pos[1];
		zmin = zmax = pos[2];
		for (size_t i = 0; i < pos.size(); i += 3) {
			if (xmin > pos[i]) {
				xmin = pos[i];
			}		   
			if (xmax < pos[i]) {
				xmax = pos[i];
			}		   
			if (ymin > pos[i + 1]) {
				ymin = pos[i + 1];
			}		   
			if (ymax < pos[i + 1]) {
				ymax = pos[i + 1];
			}		   
			if (zmin > pos[i + 2]) {
				zmin = pos[i + 2];
			}
			if (zmax < pos[i + 2]) {
				zmax = pos[i + 2];
			}
		}
		mpAabb = std::make_unique<AABB>(xmin, xmax, ymin, ymax, zmin, zmax);
	}
	
	return createBufferObject(&pos[0], pos.size()*sizeof(float), pos.size()/3,
		index.data(), index.size()*sizeof(unsigned int),
		texcoord.data(), texcoord.size() * sizeof(float),
		normal.data(), normal.size() * sizeof(float));
}

bool Mesh::loadMesh(const std::vector<float>& pos,
	const std::vector<float>& texcoord,
	const std::vector<float>& normal,
	const std::vector<unsigned int>& index,
	const std::vector<int>& boneId,
	const std::vector<float>& boneWeight
)
{
	float xmin, xmax, ymin, ymax, zmin, zmax;
	if (pos.size() >= 9) {
		xmin = xmax = pos[0];
		ymin = ymax = pos[1];
		zmin = zmax = pos[2];
		for (size_t i = 0; i < pos.size(); i += 3) {
			if (xmin > pos[i]) {
				xmin = pos[i];
			}
			if (xmax < pos[i]) {
				xmax = pos[i];
			}
			if (ymin > pos[i + 1]) {
				ymin = pos[i + 1];
			}
			if (ymax < pos[i + 1]) {
				ymax = pos[i + 1];
			}
			if (zmin > pos[i + 2]) {
				zmin = pos[i + 2];
			}
			if (zmax < pos[i + 2]) {
				zmax = pos[i + 2];
			}
		}
		mpAabb = std::make_unique<AABB>(xmin, xmax, ymin, ymax, zmin, zmax);
	}
	if (!boneId.empty()) {
		mHasSkin = true;
	}
	return createBufferObject(&pos[0], pos.size() * sizeof(float), pos.size() / 3,
		index.data(), index.size() * sizeof(unsigned int),
		texcoord.data(), texcoord.size() * sizeof(float),
		normal.data(), normal.size() * sizeof(float),
		nullptr,0,boneId.data(),boneId.size()*sizeof(int),
		boneWeight.data(),boneWeight.size()*sizeof(float));
}

//bool Mesh::loadMesh(const std::string meshFilePath) {
//	std::ifstream infile;
//	infile.open(meshFilePath, std::ifstream::in | std::ifstream::binary);
//	if (infile.good()) {
//		LdpMesh mesh;
//		infile.read((char*)&mesh, sizeof(mesh));
//		if (infile.gcount() == sizeof(mesh)) {
//			//创建mesh的aabb
//			mpAabb = make_unique<AABB>(mesh.xmin,mesh.xmax,mesh.ymin,mesh.ymax,mesh.zmin,mesh.zmax);
//			//获取mesh的材质的名字
//			if (mesh.materialNameSize > 0) {
//				mMaterialName.resize(mesh.materialNameSize);
//				infile.read(mMaterialName.data(), mesh.materialNameSize);
//				assert(infile.gcount() == mesh.materialNameSize);
//				mpMaterial = Resource::getInstance().getMaterial(mMaterialName);
//				if (!mpMaterial) {
//					LOGE("ERROR to find material %s,when load mesh from file", mMaterialName.c_str());
//				}
//			}
//			std::vector<float> pos;
//			std::vector<float> texcoord;
//			std::vector<float> normal;
//			std::vector<float> tangent;
//			//std::vector<float> bitangent;
//			std::vector<unsigned int> index;
//
//			//读取顶点数据
//			if (mesh.vertexCount > 0) {
//				pos.reserve(mesh.vertexCount*3);
//				infile.read((char*)pos.data(), mesh.vertexLength);
//				assert(infile.gcount() == mesh.vertexLength);
//			}
//			//读取纹理坐标数据
//			if (mesh.texcoordLength > 0) {
//				texcoord.reserve(mesh.vertexCount * 2);
//				infile.read((char*)texcoord.data(), mesh.texcoordLength);
//				assert(infile.gcount() == mesh.texcoordLength);
//			}
//			//读取法线数据
//			if (mesh.normalLength > 0) {
//				normal.reserve(mesh.vertexCount * 3);
//				infile.read((char*)normal.data(), mesh.normalLength);
//				assert(infile.gcount() == mesh.normalLength);
//			}
//
//			if (mesh.tangentsLength > 0) {
//				tangent.reserve(mesh.vertexCount * 3);
//				infile.read((char*)tangent.data(), mesh.tangentsLength);
//				assert(infile.gcount() == mesh.tangentsLength);
//			}
//
//			//读取索引数据
//			if (mesh.indexLength > 0) {
//				index.reserve(mesh.indexLength/sizeof(unsigned int));
//				infile.read((char*)index.data(), mesh.indexLength);
//				assert(infile.gcount() == mesh.indexLength);
//			}
//
//			createBufferObject(pos.data(), mesh.vertexLength, mesh.vertexCount, 
//				index.data(), mesh.indexLength, 
//				texcoord.data(), mesh.texcoordLength,
//				normal.data(),mesh.normalLength,
//				nullptr,0,
//				tangent.data(),mesh.tangentsLength);
//		}
//		return true;
//	}
//	return false;
//}
//初始化bone matrix size，bone name-id map
//void Mesh::initBoneInfo(vector<glm::mat4>&& offsetMatrix, unordered_map<std::string, int>&& nameIndexMap) {
//	mBonesOffsetMatrix = std::move(offsetMatrix);
//	mBoneNameIndex = std::move(nameIndexMap);
//	mBonesFinalMatrix.resize(mBonesOffsetMatrix.size());
//}

////初始化boneInfo per vertex
//bool Mesh::loadBoneData(const int* pBoneIds, int idByteSize,
//	const GLfloat* pWeights, int wByteSize, int drawType) {
//	if (pBoneIds != nullptr && idByteSize > 0)
//	{
//		setBoneIdData(pBoneIds, idByteSize, drawType);
//		checkglerror();
//	}
//	if (pWeights != nullptr && wByteSize > 0)
//	{
//		setBoneWeightData(pWeights, wByteSize, drawType);
//		checkglerror();
//	}
//	glBindBuffer(GL_ARRAY_BUFFER, 0);
//	return true;
//}

bool Mesh::createBufferObject(const GLfloat* pos,int posByteSize, int countOfVertex, 
	const GLuint* index,int indexByteSize,
	const GLfloat* tex,int texByteSize,
	const GLfloat* nor,int norByteSize,
	const GLfloat* color, int colorByteSize,
	const GLint* pBoneId, int boneIdSize,
	const GLfloat* pBoneWeight, int boneWeightSize,
	int drawType)
{
	int totalSize = posByteSize + texByteSize + norByteSize 
		+ colorByteSize + boneIdSize + boneWeightSize + indexByteSize;
	auto pTempData = std::make_unique<char[]>(totalSize);
	int offset = 0;
	
	if(pos != nullptr && posByteSize>0)
	{
		mCountOfVertex = countOfVertex;
		mPosByteSize = posByteSize;
		mPosOffset = offset;
		mPosStride = 0;
		memcpy(&pTempData[0]+ offset, pos, posByteSize);
		//mpVbo->updateVbo(offset, (void*)pos, posByteSize);
		offset += posByteSize;
		//setPosData(pos,posByteSize, drawType);
		//checkglerror();
	}
	if(tex != nullptr && texByteSize > 0)
	{
		mTexByteSize = texByteSize;
		mTexOffset = offset;
		mTexStride = 0;
		memcpy(&pTempData[0] + offset, tex, texByteSize);
		//mpVbo->updateVbo(offset, (void*)tex, texByteSize);
		offset += texByteSize;
		//setTexcoordData(tex,texByteSize, drawType);
	}
	if(nor != nullptr && norByteSize > 0)
	{
		mNorByteSize = norByteSize;
		mNorOffset = offset;
		mNorStride = 0;
		memcpy(&pTempData[0] + offset, nor, norByteSize);
		//mpVbo->updateVbo(offset, (void*)nor, norByteSize);
		offset += norByteSize;
		//setNormalData(nor,norByteSize, drawType);
	}
	if (color != nullptr && colorByteSize > 0) {
		//setColorData(color, colorByteSize, drawType);
		mColorByteSize = colorByteSize;
		mColorOffset = offset;
		mColorStride = 0;
		memcpy(&pTempData[0] + offset, color, colorByteSize);
		//mpVbo->updateVbo(offset, (void*)color, colorByteSize);
		offset += colorByteSize;
		//setColorData(color, colorByteSize, drawType);
	}
	//glBindBuffer(GL_ARRAY_BUFFER, 0);

	if(index!=nullptr)
	{
		mIndexStride = 4;
		mIndexByteSize = indexByteSize;
		mCountOfIndex = indexByteSize/sizeof(GLuint);
		mIndexOffset = offset;
		mIndexStride = 0;
		memcpy(&pTempData[0] + offset, index, indexByteSize);
		//mpVbo->updateVbo(offset,(void*)index, indexByteSize);
		offset += indexByteSize;
		//setIndexData(index,indexByteSize, drawType);
		//checkglerror();
	}

	if (pBoneId != nullptr) {
		mBoneIdByteSize = boneIdSize;
		mBoneIdOffset = offset;
		mBoneIdStride = 0;
		memcpy(&pTempData[0] + offset, pBoneId, boneIdSize);
		//mpVbo->updateVbo(offset, (void*)pBoneId, boneIdSize);
		offset += boneIdSize;
	}
	if (pBoneWeight != nullptr) {
		mBoneWeightByteSize = boneWeightSize;
		mBoneWeightOffset = offset;
		mBoneWeightStride = 0;
		memcpy(&pTempData[0] + offset, pBoneWeight, boneWeightSize);
		//mpVbo->updateVbo(offset, (void*)pBoneWeight, boneWeightSize);
	}
	if (totalSize > 0) {
		auto pVbo = std::make_shared<Vbo>();
		pVbo->initVbo((void*)&pTempData[0], totalSize);
		mpVbo.emplace_back(pVbo);
	}
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	return true;
}

//更新pos vbo
bool Mesh::updataPos(float* pos, int byteOffset, int size)
{
	if (size + byteOffset > mPosByteSize) {
		LOGE("ERROR to update mesh pos data, the size + byteOffset is greater then vbo size");
	}
	else if(mpVbo[mPosVboIndex]) {
		//int offset = byteOffset;
		mpVbo[mPosVboIndex]->updateVbo(mPosOffset+byteOffset, (void*)pos, byteOffset);
	}
	else {
		LOGE("the mpVbo is null when updatePos");
	}
	return true;
}

bool Mesh::updateColor(float* color, int byteOffset, int size) {
	if (size + byteOffset > mColorByteSize)
	{
		LOGE("ERROR to update mesh color data, the size + byteOffset is greater then vbo size");
	}    
	/*glBindBuffer(GL_ARRAY_BUFFER, mColorVbo);
	glBufferSubData(GL_ARRAY_BUFFER, offsetInByte, sizeInByteToBeReplaced, color);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return true;*/
	else if (mpVbo[mColorVboIndex]) {
		mpVbo[mColorVboIndex]->updateVbo(mColorOffset+ byteOffset, (void*)color, size);
	}
	else {
		LOGE("the mpVbo is null when updateNormal");
	}
	return true;
}

bool Mesh::updateNormal(float* normal, int byteOffset, int size) {
	if (size + byteOffset > mNorByteSize)
	{
		LOGE("ERROR to update mesh normal data, the size + byteOffset is greater then vbo size");
		return false;
	}
	else if (mpVbo[mNorVboIndex]) {
		//int offset = mPosByteSize + mTexByteSize + byteOffset;
		mpVbo[mNorVboIndex]->updateVbo(mNorOffset + byteOffset, (void*)normal, size);
	}
	else {
		LOGE("the mpVbo is null when updateNormal");
	}
	return true;
}

bool Mesh::updateBoneId(GLint* pId, int byteOffset, int size) {
	if (size + byteOffset > mBoneIdByteSize)
	{
		LOGE("ERROR to update mesh BoneId data, the size + byteOffset is greater then vbo size");
		return false;
	}
	else if (mpVbo[mBoneIdVboIndex]) {
		/*int offset = mPosByteSize + mTexByteSize + mNorByteSize
			+ mIndexByteSize + byteOffset;*/
		mpVbo[mBoneIdVboIndex]->updateVbo(mBoneIdOffset+byteOffset, (void*)pId, size);
	}
	else {
		LOGE("the mpVbo is null when updateBoneId");
	}
	return true;
}

bool Mesh::updateBoneWeight(float* pWeight, int byteOffset, int size) {
	if (size + byteOffset > mBoneWeightByteSize)
	{
		LOGE("ERROR to update mesh normal data, the size + byteOffset is greater then vbo size");
		return false;
	}
	else if (mpVbo[mBoneWeightVboIndex]) {
		/*int offset = mPosByteSize + mTexByteSize + mNorByteSize
			+ mIndexByteSize + mBoneIdByteSize + byteOffset;*/
		mpVbo[mBoneWeightVboIndex]->updateVbo(mBoneWeightOffset+byteOffset, (void*)pWeight, size);
	}
	else {
		LOGE("the mpVbo is null when updateBoneWeight");
	}
	return true;
}

//更新纹理坐标vbo
bool Mesh::updataTexcoord(float* tex, int byteOffset, int size)
{
	if (size + byteOffset > mTexByteSize)
	{
		LOGE("ERROR to update mesh texcoord data, the size + byteOffset is greater then vbo size");
		return false;
	}
	else if (mpVbo[mTexVboIndex]) {
		//int offset = mPosByteSize + byteOffset;
		mpVbo[mTexVboIndex]->updateVbo(mTexOffset+byteOffset, (void*)tex, size);
	}
	else {
		LOGE("the mpVbo is null when updataTexcoord");
	}
	return true;
}
//更新索引vbo
bool Mesh::updataIndex(GLuint* pIndex, int byteOffset, int size)
{
	if (size + byteOffset > mIndexByteSize)
	{
		LOGE("ERROR to update mesh index data, the size + byteOffset is greater then vbo size");
		return false;
	}
	else if (mpVbo[mIndexVboIndex]) {
		//int offset = mPosByteSize + mTexByteSize + mNorByteSize + byteOffset;
		mpVbo[mIndexVboIndex]->updateVbo(mIndexOffset+byteOffset, (void*)pIndex, size);
	}
	else {
		LOGE("the mpVbo is null when updataTexcoord");
	}
	return true;
}

void Mesh::drawLineStrip(int posloc)
{
	if (createVaoIfNeed(posloc) && !mpVbo.empty()) {
		glBindVertexArray(mVAO);
		if (mposLocation >= 0) {
			mpVbo[mPosVboIndex]->bindArray(true);
			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexVbo);
			glEnableVertexAttribArray(posloc);
			assert(mCountOfVertex != 0);
			int componentOfPos = mPosByteSize / (sizeof(GLfloat) * mCountOfVertex);
			glVertexAttribPointer(posloc, componentOfPos, GL_FLOAT, GL_FALSE, 0, 0);
		}
		glBindVertexArray(0);
	}
	
	glBindVertexArray(mVAO);
	glLineWidth(mLineWidth);
	glDrawArrays(GL_LINE_STRIP, 0, mCountOfVertex);
	glBindVertexArray(0);
	mpVbo[mPosVboIndex]->bindArray(false);
	//glDrawElements(GL_LINE_LOOP, mNumOfIndex, GL_UNSIGNED_INT, (const void*)0);
}

//void Mesh::drawTriangleFan(int posloc, int texloc,int norloc,int colorloc, int tangentloc)
//{
//	//glFrontFace(GL_CW);
//	if (createVaoIfNeed(posloc)){
//		glBindVertexArray(mVAO);
//		if (posloc >= 0)
//		{
//			glBindBuffer(GL_ARRAY_BUFFER, mPosVbo);
//			glEnableVertexAttribArray(posloc);
//			assert(mCountOfVertex != 0);
//			int componentOfPos = mPosByteSize / (sizeof(GLfloat) * mCountOfVertex);
//			glVertexAttribPointer(posloc, componentOfPos, GL_FLOAT, GL_FALSE, 0, 0);
//		}
//		if (texloc >= 0)
//		{
//			glBindBuffer(GL_ARRAY_BUFFER, mTexVbo);
//			//indicate a vertexAttrib space 2*float,in mTexVbo
//			glEnableVertexAttribArray(texloc);
//			int componentOfTexcoord = mTexByteSize / (sizeof(GLfloat) * mCountOfVertex);
//			glVertexAttribPointer(texloc, componentOfTexcoord, GL_FLOAT, GL_FALSE, 0, 0);
//
//		}
//
//		if (norloc >= 0)
//		{
//			glBindBuffer(GL_ARRAY_BUFFER, mNorVbo);
//			glEnableVertexAttribArray(norloc);
//			int componentOfNormal = mNorByteSize / (sizeof(GLfloat) * mCountOfVertex);
//			glVertexAttribPointer(norloc, componentOfNormal, GL_FLOAT, GL_FALSE, 0, 0);
//
//		}
//
//		if (tangentloc > 0) {
//			glBindBuffer(GL_ARRAY_BUFFER, mTangentVbo);
//			glEnableVertexAttribArray(tangentloc);
//			int componentOfTangent = mTangentByteSize / (sizeof(GLfloat) * mCountOfVertex);
//			glVertexAttribPointer(tangentloc, componentOfTangent, GL_FLOAT, GL_FALSE, 0, 0);
//		}
//
//		
//		//这个好像不用绑定了？
//		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexVbo);
//
//		glBindVertexArray(0);
//	}
//
//	glBindVertexArray(mVAO);
//	if (colorloc >= 0) {
//		glBindBuffer(GL_ARRAY_BUFFER, mColorVbo);
//		glEnableVertexAttribArray(colorloc);
//		int componentOfColor = mColorByteSize / (sizeof(GLfloat) * mCountOfVertex);
//		glVertexAttribPointer(colorloc, componentOfColor, GL_FLOAT, GL_FALSE, 0, 0);
//	}
//	glDrawArrays(GL_TRIANGLE_FAN, 0, mCountOfVertex);
//	glBindVertexArray(0);
//	//glFrontFace(GL_CCW);
//	//glDrawElements(GL_TRIANGLE_FAN, mNumOfIndex, GL_UNSIGNED_SHORT, (const void*)0);
//}
//
//void Mesh::drawTriangleStrip(int posloc = -1, int texloc = -1, int norloc = -1, int colorloc = -1, int tangentloc = -1) {
//	if (createVaoIfNeed(posloc)) {
//		glBindVertexArray(mVAO);
//		if (posloc >= 0)
//		{
//			glBindBuffer(GL_ARRAY_BUFFER, mPosVbo);
//			glEnableVertexAttribArray(posloc);
//			assert(mCountOfVertex != 0);
//			int componentOfPos = mPosByteSize / (sizeof(GLfloat) * mCountOfVertex);
//			glVertexAttribPointer(posloc, componentOfPos, GL_FLOAT, GL_FALSE, 0, 0);
//			//glVertexAttribDivisor(posloc, 1);//这个函数一调用，shader 里面posloc这个位置的顶点属性，就会变成uniform属性了，
//			//渲染一个instance，只取一个值出来，渲染下一个instance的时候再取下一个值出来。
//		}
//
//		if (texloc >= 0)
//		{
//			glBindBuffer(GL_ARRAY_BUFFER, mTexVbo);
//			glEnableVertexAttribArray(texloc);
//			int componentOfTexcoord = mTexByteSize / (sizeof(GLfloat) * mCountOfVertex);
//			//indicate a vertexAttrib space 2*float,in mTexVbo
//			glVertexAttribPointer(texloc, componentOfTexcoord, GL_FLOAT, GL_FALSE, 0, 0);
//
//		}
//		if (norloc >= 0)
//		{
//			glBindBuffer(GL_ARRAY_BUFFER, mNorVbo);
//			glEnableVertexAttribArray(norloc);
//			int componentOfNormal = mNorByteSize / (sizeof(GLfloat) * mCountOfVertex);
//			glVertexAttribPointer(norloc, componentOfNormal, GL_FLOAT, GL_FALSE, 0, 0);
//
//		}
//
//		if (tangentloc > 0) {
//			glBindBuffer(GL_ARRAY_BUFFER, mTangentVbo);
//			glEnableVertexAttribArray(tangentloc);
//			int componentOfTangent = mTangentByteSize / (sizeof(GLfloat) * mCountOfVertex);
//			glVertexAttribPointer(tangentloc, componentOfTangent, GL_FLOAT, GL_FALSE, 0, 0);
//		}
//
//		if (colorloc >= 0) {
//			glBindBuffer(GL_ARRAY_BUFFER, mColorVbo);
//			glEnableVertexAttribArray(colorloc);
//			int componentOfColor = mColorByteSize / (sizeof(GLfloat) * mCountOfVertex);
//			glVertexAttribPointer(colorloc, componentOfColor, GL_FLOAT, GL_FALSE, 0, 0);
//		}
//
//		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexVbo);//glDrawElements会用到这个
//		glBindVertexArray(0);
//	}
//
//	glBindVertexArray(mVAO);
//	glDrawElements(GL_TRIANGLE_STRIP, mIndexByteSize / sizeof(GLuint), GL_UNSIGNED_INT, (const void*)0);
//	glBindVertexArray(0);
//}

void Mesh::drawTriangles(int posloc,int texloc,int norloc,int colorloc,int boneIdLoc,int boneWeightLoc)
{
	if (createVaoIfNeed(posloc,texloc,norloc,colorloc) && !mpVbo.empty()) {
		glBindVertexArray(mVAO);
		
		if (posloc >= 0)
		{
			mpVbo[mPosVboIndex]->bindArray(true);
			glEnableVertexAttribArray(posloc);
			assert(mCountOfVertex != 0);
			int componentOfPos = mPosByteSize / (sizeof(GLfloat) * mCountOfVertex);
			glVertexAttribPointer(posloc, componentOfPos, GL_FLOAT, GL_FALSE, mPosStride, (const void*)mPosOffset);
			//glVertexAttribDivisor(posloc, 1);//这个函数一调用，shader 里面posloc这个位置的顶点属性，就会变成uniform属性了，
			//渲染一个instance，只取一个值出来，渲染下一个instance的时候再取下一个值出来。
		}

		if (texloc >= 0)
		{
			mpVbo[mTexVboIndex]->bindArray(true);
			//int offset = mPosByteSize;
			glEnableVertexAttribArray(texloc);
			int componentOfTexcoord = mTexByteSize / (sizeof(GLfloat) * mCountOfVertex);
			//indicate a vertexAttrib space 2*float,in mTexVbo
			glVertexAttribPointer(texloc, componentOfTexcoord, GL_FLOAT, GL_FALSE, mTexStride, (const void*)mTexOffset);
			if (mTexByteSize == 0) {
				LOGE("render mesh,the shader has texcoord attribute,but there are no texccord data");
			}
		}
		if (norloc >= 0)
		{
			mpVbo[mNorVboIndex]->bindArray(true);
			//int offset = mPosByteSize + mTexByteSize;
			glEnableVertexAttribArray(norloc);
			int componentOfNormal = mNorByteSize / (sizeof(GLfloat) * mCountOfVertex);
			glVertexAttribPointer(norloc, componentOfNormal, GL_FLOAT, GL_FALSE, mNorStride, (const void*)mNorOffset);
			if (mNorByteSize == 0) {
				LOGE("render mesh,the shader has normal attribute,but there are no normal data");
			}
		}
		if (colorloc >= 0) {
			mpVbo[mColorVboIndex]->bindArray(true);
			glEnableVertexAttribArray(colorloc);
			int componentNum = mColorByteSize / (sizeof(float) * mCountOfVertex);
			glVertexAttribPointer(colorloc, componentNum, GL_FLOAT, GL_FALSE, mColorStride, (const void*)mColorOffset);
			if (mColorByteSize == 0) {
				LOGE("render mesh,the shader has color attribute,but there are no color data");
			}
		}
		if (boneIdLoc >= 0) {
			mpVbo[mBoneIdVboIndex]->bindArray(true);
			/*int offset = mPosByteSize + mTexByteSize + mNorByteSize+
				mColorByteSize + mIndexByteSize;*/
			glEnableVertexAttribArray(boneIdLoc);
			int componentSize = mBoneIdByteSize / (4 * mCountOfVertex);
			int type = GL_UNSIGNED_INT;
			if (componentSize == 2) {
				type = GL_UNSIGNED_SHORT;
			}
			else if (componentSize == 1) {
				type = GL_UNSIGNED_BYTE;
			}
			glVertexAttribIPointer(boneIdLoc, 4, type, mBoneIdStride, (const void*)mBoneIdOffset);
			if (mBoneIdByteSize == 0) {
				LOGE("render mesh,the shader has boneId attribute,but there are no boneId data");
			}
		}
		if (boneWeightLoc>=0) {
			mpVbo[mBoneWeightVboIndex]->bindArray(true);
			/*int offset = mPosByteSize + mTexByteSize + mNorByteSize +
				mColorByteSize + mIndexByteSize + mBoneIdByteSize;*/
			glEnableVertexAttribArray(boneWeightLoc);
			int componentNum = mBoneWeightByteSize / (sizeof(GLfloat) * mCountOfVertex);
			glVertexAttribPointer(boneWeightLoc, componentNum, GL_FLOAT, GL_FALSE, mBoneWeightStride, (const void*)mBoneWeightOffset);
			if (mBoneWeightByteSize == 0) {
				LOGE("render mesh,the shader has boneWeight attribute,but there are no boneWeight data");
			}
		}
		
		if (mDrawType == DrawType::Triangles || mDrawType == DrawType::TriangleStrip) {
			mpVbo[mIndexVboIndex]->bindElement(true);
			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexVbo);//glDrawElements会用到这个
		}
		glBindVertexArray(0);
	}
	
	glBindVertexArray(mVAO);
	/*if (colorloc >= 0) {
		glBindBuffer(GL_ARRAY_BUFFER, mColorVbo);
		glEnableVertexAttribArray(colorloc);
		int componentOfColor = mColorByteSize / (sizeof(GLfloat) * mCountOfVertex);
		glVertexAttribPointer(colorloc, componentOfColor, GL_FLOAT, GL_FALSE, 0, 0);
		if (mColorByteSize == 0) {
			LOGE("render mesh,the shader has color attribute,but there are no color data");
		}
	}*/
	int componentSize = mIndexByteSize / mCountOfIndex;
	int type = GL_UNSIGNED_INT;
	if (componentSize == 2) {
		type = GL_UNSIGNED_SHORT;
	}
	else if (componentSize == 1) {
		type = GL_UNSIGNED_BYTE;
	}
	if (mDrawType == DrawType::Triangles) {
		glDrawElements(GL_TRIANGLES, mCountOfIndex, type, (const void*)mIndexOffset);
	}
	else if (mDrawType == DrawType::TriangleStrip) {
		glDrawElements(GL_TRIANGLE_STRIP, mCountOfIndex, type, (const void*)mIndexOffset);
	}
	else {
		glDrawArrays(GL_TRIANGLE_FAN, 0, mCountOfVertex);
	}
	glBindVertexArray(0);
	mpVbo[mIndexVboIndex]->bindElement(false);
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

void Mesh::draw(int posloc, int texloc, int norloc, int colorloc,int boneIdLoc,int boneWeightLoc)
{
	//shader has boneid vertex attribute
	if (boneIdLoc != -1 && mpSkeleton) {
		mpSkeleton->updateToUbo();
	}
	switch (mDrawType) {
	case DrawType::Triangles:
	case DrawType::TriangleFan:
	case DrawType::TriangleStrip:
		drawTriangles(posloc, texloc, norloc, colorloc,boneIdLoc, boneWeightLoc);
		break;
	case DrawType::LineStrip:
		drawLineStrip(posloc);
		break;
	default:
		break;
	}
}

void Mesh::draw(const glm::mat4* modelMat, const glm::mat4* texMat, const glm::mat4* projViewMat) {
	if (mpMaterial) {
		auto& pShader = mpMaterial->getShader();
		if (pShader) {
			mpMaterial->enable();
			mpMaterial->setMyRenderOperation();
			if (modelMat) {
				pShader->setModelMatrix(*modelMat);
			}
			if (texMat) {
				pShader->setTextureMatrix(*texMat);
			}
			if (pShader->hasPreMvpMat()&& modelMat!=nullptr && projViewMat!=nullptr) {
				if (!mpPreMvpMatrix) {
					mpPreMvpMatrix = std::make_unique<glm::mat4>((*projViewMat) * (*modelMat));
				}
				if(mpPreMvpMatrix) {
					pShader->setPreMvpMatrix(*mpPreMvpMatrix);
					*mpPreMvpMatrix = (*projViewMat) * (*modelMat);
				}
			}
			
			int posloc = -1;
			int texloc = -1;
			int norloc = -1;
			int colorloc = -1;
			int boneIdLoc = -1;
			int boneWeightLoc = -1;
			pShader->getLocation(posloc, texloc, colorloc, norloc, boneIdLoc, boneWeightLoc);
			draw(posloc, texloc, norloc, colorloc, boneIdLoc,boneWeightLoc);
			mpMaterial->restoreRenderOperation();
		}
		else {
			LOGE("mesh has no shader,can't render");
		}
	}
	else {
		LOGE("mesh has no material,can't render");
	}
}

//bool Mesh::setPosData(const GLfloat* pos, int size, unsigned int drawType)
//{
//	if (mPosVbo > 0) {
//		glDeleteBuffers(1, &mPosVbo);
//		if (mVAO != 0) {
//			//先删除原来的vao
//			glDeleteVertexArrays(1, &mVAO);
//			mVAO = 0;
//		}
//	}
//	glGenBuffers(1, &mPosVbo);
//	glBindBuffer(GL_ARRAY_BUFFER, mPosVbo);
//	glBufferData(GL_ARRAY_BUFFER, size, pos, drawType);
//	mPosByteSize = size;
//	return true;
//}
//bool Mesh::setTexcoordData(const GLfloat* tex, int size, unsigned int drawType)
//{
//	if (mTexVbo > 0) {
//		glDeleteBuffers(1, &mTexVbo);
//		if (mVAO != 0) {
//			//先删除原来的vao
//			glDeleteVertexArrays(1, &mVAO);
//			mVAO = 0;
//		}
//	}
//	glGenBuffers(1, &mTexVbo);
//	glBindBuffer(GL_ARRAY_BUFFER, mTexVbo);
//	glBufferData(GL_ARRAY_BUFFER, size, tex, drawType);
//	mTexByteSize = size;
//	return true;
//}

//bool Mesh::setNormalData(const GLfloat* nor, int sizeInbyte, unsigned int drawType)
//{
//	if (mNorVbo > 0) {
//		glDeleteBuffers(1, &mNorVbo);
//		if (mVAO != 0) {
//			//先删除原来的vao
//			glDeleteVertexArrays(1, &mVAO);
//			mVAO = 0;
//		}
//	}
//	glGenBuffers(1, &mNorVbo);
//	glBindBuffer(GL_ARRAY_BUFFER, mNorVbo);
//	glBufferData(GL_ARRAY_BUFFER, sizeInbyte, nor, drawType);
//	mNorByteSize = sizeInbyte;
//	return true;
//}

//bool Mesh::setBoneIdData(const int* boneIds, int sizeInbyte, unsigned int drawType) {
//	if(mBoneIdVbo>0){
//		glDeleteBuffers(1, &mBoneIdVbo);
//		if (mVAO != 0) {
//			//先删除原来的vao
//			glDeleteVertexArrays(1, &mVAO);
//			mVAO = 0;
//		}
//	}
//	glGenBuffers(1, &mBoneIdVbo);
//	glBindBuffer(GL_ARRAY_BUFFER, mBoneIdVbo);
//	glBufferData(GL_ARRAY_BUFFER, sizeInbyte, boneIds, drawType);
//	mBoneIdByteSize = sizeInbyte;
//	return true;
//}

//bool Mesh::setBoneWeightData(const GLfloat* weight, int sizeInbyte, unsigned int drawType) {
//	if (mBoneWeightVbo > 0) {
//		glDeleteBuffers(1, &mBoneWeightVbo);
//		if (mVAO != 0) {
//			//先删除原来的vao
//			glDeleteVertexArrays(1, &mVAO);
//			mVAO = 0;
//		}
//	}
//	glGenBuffers(1, &mBoneWeightVbo);
//	glBindBuffer(GL_ARRAY_BUFFER, mBoneWeightVbo);
//	glBufferData(GL_ARRAY_BUFFER, sizeInbyte, weight, drawType);
//	mBoneWeightByteSize = sizeInbyte;
//	return true;
//}

bool Mesh::setColorData(const GLfloat* nor, int size, unsigned int drawType)
{
	if (mColorVbo > 0) {
		glDeleteBuffers(1, &mColorVbo);
	}
	glGenBuffers(1, &mColorVbo);
	glBindBuffer(GL_ARRAY_BUFFER, mColorVbo);
	glBufferData(GL_ARRAY_BUFFER, size, nor, drawType);
	mColorByteSize = size;
	return true;
}

//bool Mesh::setIndexData(const GLuint* index, int size, unsigned int drawType)
//{
//	if (mIndexVbo > 0) {
//		glDeleteBuffers(1, &mIndexVbo);
//	}
//	glGenBuffers(1, &mIndexVbo);
//	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexVbo);
//	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, index, drawType);
//	mIndexByteSize = size;
//	return true;
//}

void Mesh::unLoadMesh()
{
	if (mColorVbo != 0) {
		glDeleteBuffers(1, &mColorVbo);
		mColorVbo = 0;
		mColorByteSize = 0;
	}
	if (mVAO != 0) {
		glDeleteVertexArrays(1, &mVAO);
		mVAO = 0;
	}
	for (auto& v : mpVbo) {
		v.reset();
	}
	mCountOfVertex = 0;
}

bool Mesh::createVaoIfNeed(int posloc, int texloc, int norloc, int colorLoc) {
	if (mposLocation != posloc || mtexLocation != texloc 
		|| mnorLocation != norloc || mColorLoc!=colorLoc) {
		if (mVAO != 0) {
			//location有变化，先删除原来的vao
			glDeleteVertexArrays(1, &mVAO);
			mVAO = 0;
		}

		mposLocation = posloc;
		mtexLocation = texloc;
		mnorLocation = norloc;
		mColorLoc = colorLoc;

		//使用vao把设置顶点属性的流程打包。
		glGenVertexArrays(1, &mVAO);
		return true;
	}
	return false;
}

void Mesh::setLineWidth(GLfloat width) {
	mLineWidth = width;
}

//const std::unordered_map<std::string, int>& Mesh::getBoneNameIndex() {
//	if (mpSkeleton) {
//		return mpSkeleton->getBoneName2Index();
//	}
//}

//const std::vector<glm::mat4>& Mesh::getBonesOffsetMatrix() {
//	if (mpSkeleton) {
//		return mpSkeleton->getOffsetMatrix();
//	}
//}

void Mesh::setSkeleton(const std::shared_ptr<Skeleton>& ps) {
	if (mHasSkin) {
		mpSkeleton = ps;
	}
	else {
		LOGD("the mesh has no skin,setSkeleton make no sense");
	}
}

int Mesh::pushVbo(const std::shared_ptr<Vbo>& pVbo) {
	int i = 0;
	bool bfound = false;
	for (const auto& v : mpVbo) {
		if (v == pVbo) {
			bfound = true;
			break;
		}
		++i;
	}
	if (!bfound) {
		mpVbo.emplace_back(pVbo);
	}
	return i;
}

void Mesh::setAABB(float x1, float y1, float z1, float x2, float y2, float z2) {
	if (!mpAabb) {
		mpAabb = std::make_unique<AABB>(x1, y1, z1, x2, y2, z2);
	}
}

