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

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <map>
#include <sstream>
#include <string>
#include <unordered_set>
#include <algorithm>
#include "animation/NodeAnimation.h"

class MeshLoaderAssimpImpl {
public:
	bool loadFromFile(const std::string& filename, std::shared_ptr<Node>& pRootNode);
	void recursive_parse(const struct aiScene* sc, const struct aiNode* nd, std::shared_ptr<Node>& pRootNode);
	void parseAnimationInfo(const struct aiScene* pScene,std::shared_ptr<Node>& pRootNode);
	aiScene* mpAiScene{ nullptr };
	std::unordered_map<std::string,std::shared_ptr<NodeAnimation>> mAnimations;
	
};

void MeshLoaderAssimpImpl::parseAnimationInfo(const struct aiScene* pScene,
	std::shared_ptr<Node>& pRootNode) {
	if (pScene != nullptr) {
		unsigned int numAnimations = pScene->mNumAnimations;
		for (unsigned int i = 0; i < numAnimations; ++i) {
			const aiAnimation* pAnimation = pScene->mAnimations[i];
			if (pAnimation != nullptr) {
				std::string name(pAnimation->mName.data);
				int64_t duration = pAnimation->mDuration * 1000.0 / pAnimation->mTicksPerSecond;
				auto pNodeAnimation = std::make_shared<NodeAnimation>(name, pRootNode);
				assert(pNodeAnimation);
				pNodeAnimation->setDuration(duration);
				if (!mAnimations.emplace(name, pNodeAnimation).second) {
					LOGE("duplicate animation name in mesh file MeshLoaderAssimp");
				}
				//how many nodes affected by this animation
				unsigned int num = pAnimation->mNumChannels;
				for (unsigned int j = 0; j < num; ++j) {
					aiNodeAnim* pNodeAnim = pAnimation->mChannels[j];
					if (pNodeAnim != nullptr) {
						std::string nodeName(pNodeAnim->mNodeName.data);
						pNodeAnimation->addNodeName(nodeName);
						auto numPos = pNodeAnim->mNumPositionKeys;
						auto* pPosKey = pNodeAnim->mPositionKeys;
						auto pPosKeys = std::make_unique<std::vector<NodeAnimation::KeyFrameVec3Time>>();
						for (unsigned int m = 0; m < numPos; ++m) {
							pPosKeys->emplace_back(glm::vec3(pPosKey[m].mValue.x, pPosKey[m].mValue.y, pPosKey[m].mValue.z),
								pPosKey[m].mTime * 1000.0 / pAnimation->mTicksPerSecond);
						}
						if (!std::is_sorted(pPosKeys->begin(), pPosKeys->end(),
							[](const NodeAnimation::KeyFrameVec3Time& key1, const NodeAnimation::KeyFrameVec3Time& key2)->bool {
								return key1.timeMs < key2.timeMs;
							})) {
							LOGE("the time of pos key frame in node animation is not sorted ");
						};
						pNodeAnimation->addPosKeyFrame(nodeName, pPosKeys);

						auto numScale = pNodeAnim->mNumScalingKeys;
						auto* pScaleKey = pNodeAnim->mScalingKeys;
						auto pScaleKeys = std::make_unique<std::vector<NodeAnimation::KeyFrameVec3Time>>();
						for (unsigned int m = 0; m < numScale; ++m) {
							pScaleKeys->emplace_back(glm::vec3(pScaleKey[m].mValue.x, pScaleKey[m].mValue.y, pScaleKey[m].mValue.z),
								pScaleKey[m].mTime * 1000.0 / pAnimation->mTicksPerSecond);
						}
						if (!std::is_sorted(pScaleKeys->begin(), pScaleKeys->end(),
							[](const NodeAnimation::KeyFrameVec3Time& key1, const NodeAnimation::KeyFrameVec3Time& key2)->bool {
								return key1.timeMs < key2.timeMs;
							})) {
							LOGE("the time of scale key frame in node animation is not sorted ");
						};
						pNodeAnimation->addScaleKeyFrame(nodeName, pScaleKeys);

						auto numRotate = pNodeAnim->mNumRotationKeys;
						auto* pRotateKey = pNodeAnim->mRotationKeys;
						auto pRotateKeys = std::make_unique<std::vector<NodeAnimation::KeyFrameQuatTime>>();
						for (unsigned int m = 0; m < numRotate; ++m) {
							pRotateKeys->emplace_back(glm::quat(pRotateKey[m].mValue.w,pRotateKey[m].mValue.x, pRotateKey[m].mValue.y, pRotateKey[m].mValue.z),
								pRotateKey[m].mTime * 1000.0 / pAnimation->mTicksPerSecond);
						}
						if (!std::is_sorted(pRotateKeys->begin(), pRotateKeys->end(),
							[](const NodeAnimation::KeyFrameQuatTime& key1, const NodeAnimation::KeyFrameQuatTime& key2)->bool {
								return key1.timeMs < key2.timeMs;
							})) {
							LOGE("the time of rotate key frame in node animation is not sorted ");
						};
						pNodeAnimation->addRotateKeyFrame(nodeName, pRotateKeys);
					}
				}
			}
		}
	}
}

void MeshLoaderAssimpImpl::recursive_parse(const struct aiScene* sc, const struct aiNode* nd,std::shared_ptr<Node>& pNode) {

	if (nd == nullptr) {
		return;
	}

	std::string nodeName(nd->mName.data);
	pNode->setName(nodeName);

	unsigned int n = 0;
	unsigned int t = 0;
	aiMatrix4x4 model = nd->mTransformation;//本节点的model矩阵
	if (!model.IsIdentity()) {
		pNode->setLocalMatrix(glm::mat4{ model.a1,model.b1,model.c1,model.d1,
			model.a2,model.b2,model.c2,model.d2 ,
			model.a3,model.b3,model.c3,model.d3 ,
			model.a4,model.b4,model.c4,model.d4 });
		//std::cout << "node transform is not identity" << std::endl;
	}

	for (n = 0; n < nd->mNumMeshes; ++n)//遍历本节点下的mesh
	{
		vector<float> mPos;//保存顶点坐标
		vector<float> mNormals;//保存normal
		vector<float> mTexcoords;//保存纹理坐标值
		vector<int> mBoneIds;//保存影响顶点的骨头id，一个顶点最多被4个骨头影响。
		vector<float> mBoneWeight;//保存骨头对顶点影响的权重，4个权重加起来要==1
		vector<glm::mat4> mBonesOffset;
		vector<unsigned int> mIndexes;//保存索引值
		unordered_map<std::string, int> mBonesNameIdMap;//通过name找到id，以便外面设置mBones的值
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
			int texIndex = 1;
			auto ret = aiReturn::aiReturn_FAILURE;
			while (ret == aiReturn::aiReturn_FAILURE) {
				ret = pMeshMaterial->GetTexture((aiTextureType)(texIndex++), 0, &textureFilePath);
				if (texIndex >= aiTextureType_UNKNOWN) {
					break;
				}
			}
			std::string pathStr(textureFilePath.C_Str());
			if (texIndex > 2 && texIndex < aiTextureType_UNKNOWN) {
				LOGE("mesh has no diffuse map but has %d map,filename is %s", texIndex, pathStr.c_str());
			}
			std::string_view materialName = Utils::getFileName(pathStr);
			int numOfBones = 0;
			unsigned int index = 0;
			for (t = 0; t < mesh->mNumFaces; ++t) {
				const struct aiFace* face = &mesh->mFaces[t];
				for (int i = 0; i < face->mNumIndices; i++) {
					int vertexIndex = face->mIndices[i];
					auto cit = indexMap.find(vertexIndex);
					if (cit == indexMap.end()) {
						if (mesh->mVertices != nullptr) {
							mPos.emplace_back(mesh->mVertices[vertexIndex].x);
							mPos.emplace_back(mesh->mVertices[vertexIndex].y);
							mPos.emplace_back(mesh->mVertices[vertexIndex].z);
						}
						if (mesh->mNormals != nullptr){
							mNormals.emplace_back(mesh->mNormals[vertexIndex].x);
							mNormals.emplace_back(mesh->mNormals[vertexIndex].y);
							mNormals.emplace_back(mesh->mNormals[vertexIndex].z);
						}
						else {
							LOGD("a mesh has no normals");
						}

						if (mesh->HasTextureCoords(0)) {
							mTexcoords.emplace_back(mesh->mTextureCoords[0][vertexIndex].x);
							mTexcoords.emplace_back(mesh->mTextureCoords[0][vertexIndex].y);
						}
						else {
							LOGD("a mesh has no texcoords");
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
				mBoneIds.assign(static_cast<size_t>(index) * 4, 0);
				mBoneWeight.assign(static_cast<size_t>(index) * 4,0.0f);
				boneCounts.assign(index,0);
				numOfBones = mesh->mNumBones;

				//a mesh can be controled by a lot of bones
				for (int i = 0; i < mesh->mNumBones; ++i) {
					aiBone* pBone = mesh->mBones[i];
					if (pBone != nullptr) {
						if (!mBonesNameIdMap.emplace(pBone->mName.data, i).second) {
							LOGE("duplicate bone name in mesh file MeshLoaderAssim");
						}
						if (pBone->mWeights != nullptr) {
							//how many vertexs can affected by a bone ? the mNumWeights
							for (int j = 0; j < pBone->mNumWeights; ++j) {
								auto& weights = pBone->mWeights[j];
								if (weights.mWeight > 0.0f) {
									auto it = indexMap.find(weights.mVertexId);
									if (it != indexMap.end()) {
										//a vertex can be controlled by as many as 4 bones
										//because the bone attribute of vertex in shader has only 4 component
										int boneCount = boneCounts[it->second];
										mBoneIds[static_cast<size_t>(4) * it->second + boneCount] = i;
										mBoneWeight[static_cast<size_t>(4) * it->second + boneCount] = weights.mWeight;
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
						auto& om = pBone->mOffsetMatrix;
						mBonesOffset.emplace_back(om.a1, om.b1, om.c1, om.d1,
							om.a2, om.b2, om.c2, om.d2,
							om.a3, om.b3, om.c3, om.d3,
							om.a4, om.b4, om.c4, om.d4);
					}
				}
			}

			MeshSP pMesh = make_shared<Mesh>(MeshType::DIY);
			if (pMesh->loadMesh(mPos, mTexcoords, mNormals, mIndexes)) {
				if (!mBoneIds.empty()) {
					for (const auto& pair : mAnimations) {
						for (const auto& nameId : mBonesNameIdMap) {
							if (pair.second->findBone(nameId.first)) {
								pMesh->addNodeAnimation(pair);
								break;
							}
						}
					}
					pMesh->loadBoneData(mBoneIds.data(), mBoneIds.size() * sizeof(int),
						mBoneWeight.data(), mBoneWeight.size() * sizeof(float));
					pMesh->initBoneInfo(std::move(mBonesOffset), std::move(mBonesNameIdMap));
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

	parseAnimationInfo(mpAiScene, pRootNode);

	recursive_parse(mpAiScene, pNode, pRootNode);

	return true;
}
