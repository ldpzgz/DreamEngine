#include "MeshLoaderAssimp.h"
#include "Node.h"
#include "Log.h"
#include "Utils.h"
#include "Material.h"
#include "Mesh.h"
#include "Utils.h"
#include "Resource.h"
// assimp include files. These three are usually needed.
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/DefaultLogger.hpp>
#include <assimp/LogStream.hpp>

//to map image filenames to textureIds
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <map>
#include <sstream>
#include <string>
#include <unordered_set>

class MeshLoaderAssimpImpl {
public:
	bool loadFromFile(const std::string& filename, std::shared_ptr<Node>& pRootNode);
	void recursive_parse(const struct aiScene* sc, const struct aiNode* nd, std::shared_ptr<Node>& pRootNode);

	aiScene* mpAiScene{ nullptr };
	
};


void MeshLoaderAssimpImpl::recursive_parse(const struct aiScene* sc, const struct aiNode* nd,std::shared_ptr<Node>& pNode) {

	if (nd == nullptr) {
		return;
	}
	unsigned int n = 0;
	unsigned int t = 0;
	aiMatrix4x4 model = nd->mTransformation;//本节点的model矩阵
	if (!model.IsIdentity()) {
		pNode->setMatrix(glm::mat4{ model.a1,model.b1,model.c1,model.d1,
			model.a2,model.b2,model.c2,model.d2 ,
			model.a3,model.b3,model.c3,model.d3 ,
			model.a4,model.b4,model.c4,model.d4 });
		std::cout << "node transform is not identity" << std::endl;
	}

	for (n = 0; n < nd->mNumMeshes; ++n)//遍历本节点下的mesh
	{
		vector<float> mPos;//保存顶点坐标
		vector<float> mNormals;//保存normal
		vector<float> mTexcoords;//保存纹理坐标值
		vector<int> mBoneIds;//保存影响顶点的骨头id，一个顶点最多被4个骨头影响。
		vector<float> mBoneWeight;//保存骨头对顶点影响的权重，4个权重加起来要==1
		vector<unsigned int> mIndexes;//保存索引值
		unordered_map<unsigned int, unsigned int> mIndexMap;
		vector<int> boneCounts;

		const struct aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];
		auto pMeshMaterial = sc->mMaterials[mesh->mMaterialIndex];
		int texCount = 0;
		std::unordered_map<unsigned int,unsigned int> indexMap;
		auto primitivType = mesh->mPrimitiveTypes;
		if (primitivType && aiPrimitiveType_TRIANGLE) {
			/*
			* getMaterial,diffuse map的文件名作为material name
			*/
			aiString textureFilePath;
			pMeshMaterial->GetTexture((aiTextureType)1, 0, &textureFilePath);
			std::string pathStr(textureFilePath.C_Str());
			std::string_view materialName = Utils::getFileName(pathStr);

			unsigned int index = 0;
			for (t = 0; t < mesh->mNumFaces; ++t) {
				const struct aiFace* face = &mesh->mFaces[t];
				for (int i = 0; i < face->mNumIndices; i++) {
					int vertexIndex = face->mIndices[i];
					auto cit = indexMap.find(vertexIndex);
					if (cit == indexMap.end()) {
						if (mesh->mNormals != nullptr && mesh->mVertices != nullptr) {
							mPos.emplace_back(mesh->mVertices[vertexIndex].x);
							mPos.emplace_back(mesh->mVertices[vertexIndex].y);
							mPos.emplace_back(mesh->mVertices[vertexIndex].z);
							mNormals.emplace_back(mesh->mNormals[vertexIndex].x);
							mNormals.emplace_back(mesh->mNormals[vertexIndex].y);
							mNormals.emplace_back(mesh->mNormals[vertexIndex].z);
							if (mesh->HasTextureCoords(0)) {
								mTexcoords.emplace_back(mesh->mTextureCoords[0][vertexIndex].x);
								mTexcoords.emplace_back(mesh->mTextureCoords[0][vertexIndex].y);
							}
						}
						mIndexes.emplace_back(index);
						indexMap.emplace(vertexIndex,index);
						++index;
					}
					else {
						mIndexes.emplace_back(cit->second);
					}
				}

			}

			if (mesh->HasBones()) {
				mBoneIds.assign(index * 4,0);
				mBoneWeight.assign(index * 4,0.0f);
				boneCounts.assign(index,0);
				//一个mesh被多少根骨头影响
				for (int i = 0; i < mesh->mNumBones; ++i) {
					auto pBone = mesh->mBones[i];
					if (pBone != nullptr && pBone->mWeights!=nullptr) {
						//每个骨头影响多少个顶点：mNumWeights
						for (int j = 0; j < pBone->mNumWeights; ++j) {
							int vertexIndex = pBone->mWeights[j].mVertexId;
							auto it = indexMap.find(vertexIndex);
							if (it != indexMap.end()) {
								//目前的shader设计，一个顶点最多只能被4个骨头影响
								//超过4个不支持
								int boneCount = boneCounts[it->second];
								mBoneIds[4 * it->second + boneCount] = i;
								mBoneWeight[4 * it->second + boneCount] = pBone->mWeights[j].mWeight;
								boneCounts[it->second] += 1;
								if (boneCounts[it->second] > 4) {
									LOGE("bones per vertex is exceed 4");
								}
							}
							else {
								LOGE("bone vs vexter error,cannot find the vertex index");
							}
						}
					}
				}
			}

			MeshSP pMesh = make_shared<Mesh>(MeshType::DIY);
			if (pMesh->loadMesh(mPos, mTexcoords, mNormals, mIndexes)) {
				if (!mBoneIds.empty()) {
					pMesh->loadBoneData(mBoneIds.data(), mBoneIds.size() * sizeof(int),
						mBoneWeight.data(), mBoneWeight.size() * sizeof(float));
				}
				pNode->addRenderable(pMesh);
				pMesh->setMaterialName(materialName);
			}
			else {
				LOGE("%s load mesh failed", __func__);
			}
			
		}
		else {
			LOGE(" primitive type is not triangle when import mesh from file");
		}
	}
	
	// recursive all children
	for (n = 0; n < nd->mNumChildren; ++n)
	{
		auto pChildNode = pNode->newAChild();
		recursive_parse(sc, nd->mChildren[n], pChildNode);
	}
}

bool MeshLoaderAssimp::loadFromFile(const std::string& path, std::shared_ptr<Node>& pRootNode) {
	MeshLoaderAssimpImpl impl;
	return impl.loadFromFile(path, pRootNode);
}

bool MeshLoaderAssimpImpl::loadFromFile(const std::string& filename, std::shared_ptr<Node>& pRootNode) {
	Assimp::Importer importer;
	// Check if file exists
	std::ifstream fin(filename.c_str());
	if (!fin.fail())
	{
		fin.close();
	}
	else
	{
		//MessageBox(nullptr, UTFConverter("Couldn't open file: " + pFile).c_wstr(), TEXT("ERROR"), MB_OK | MB_ICONEXCLAMATION);
		LOGE("import mesh file error %s,info: %s", filename.c_str(), importer.GetErrorString());
		return false;
	}

	auto mpAiScene = importer.ReadFile(filename, aiProcessPreset_TargetRealtime_Quality);

	// If the import failed, report it
	if (!mpAiScene)
	{
		LOGE(" importer.ReadFile %s,info:%s", filename.c_str(), importer.GetErrorString());
		return false;
	}
	//拿到mesh的文件名，用于给mesh的材质命名
	//mFileName = Utils::getFileName(filename);
	

	auto pNode = mpAiScene->mRootNode;
	recursive_parse(mpAiScene, pNode, pRootNode);

	return true;
}
