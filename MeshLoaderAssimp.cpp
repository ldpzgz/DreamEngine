#include "MeshLoaderAssimp.h"
#include "Node.h"
#include "Log.h"
#include "Utils.h"
#include "Material.h"
#include "Mesh.h"
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

class MeshLoaderAssimpImpl {
public:
	bool loadFromFile(const std::string& filename, std::shared_ptr<Node<glm::mat4>>& pRootNode);
	void recursive_parse(const struct aiScene* sc, const struct aiNode* nd, std::shared_ptr<Node<glm::mat4>>& pRootNode);

	aiScene* mpAiScene{ nullptr };
	bool mbMergeAllMesh{ false };
	vector<float> mPos;//保存顶点坐标
	vector<float> mNormals;//保存normal
	vector<float> mTangents;//保存tangents，用于做凹凸贴图
	vector<float> mTexcoords;//保存纹理坐标值
	vector<unsigned int> mIndexes;//保存索引值
	unordered_map<unsigned int, unsigned int> mIndexMap;
	unsigned int mCurIndex{ 0 };
	int mMeshNum{ 0 }; //记录mesh的个数
	std::string mMaterialName;//当前mesh的材质名字
	std::string mFileName;//当前要加载的文件的名字，不包括文件的路径与后缀名。用于作为material的名字的前缀
	unordered_map<string, string> mContentNameMap;
};


void MeshLoaderAssimpImpl::recursive_parse(const struct aiScene* sc, const struct aiNode* nd,std::shared_ptr<Node<glm::mat4>>& pRootNode) {

	if (nd == nullptr) {
		return;
	}
	unsigned int n = 0;
	unsigned int t = 0;
	aiMatrix4x4 model = nd->mTransformation;//本节点的model矩阵
	if (!model.IsIdentity()) {
		std::cout << "node transform is not identity" << std::endl;
	}

	for (n = 0; n < nd->mNumMeshes; ++n)//遍历本节点下的mesh
	{
		std::shared_ptr<Material> pMeshMat;
		if (!mbMergeAllMesh) {
			mCurIndex = 0;
			mPos.clear();
			mNormals.clear();
			mTangents.clear();
			mIndexes.clear();
			mIndexMap.clear();
			mTexcoords.clear();
			mMaterialName.clear();
		}
		int indexTranslate = mCurIndex;
		const struct aiMesh* mesh = sc->mMeshes[nd->mMeshes[n]];
		auto pMeshMaterial = sc->mMaterials[mesh->mMaterialIndex];
		int texCount = 0;
		std::string diffuse;
		std::string specular;
		std::string normal;
		std::string mask;
		std::map<int, std::string> texMap;
		for (int i = 1; i <= 9; ++i) {
			aiString textureFilePath;
			auto texCount = pMeshMaterial->GetTextureCount((aiTextureType)i);
			if (texCount > 0) {
				pMeshMaterial->GetTexture((aiTextureType)i, 0, &textureFilePath);
				texMap.emplace(i, textureFilePath.C_Str());
				std::cout << i << ": " << textureFilePath.C_Str() << std::endl;
				++texCount;
			}
		}
		std::string matName = mFileName;
		std::string program;
		std::string sampler;
		string op;
		if (!texMap.empty()) {
			//1 diff 5 normal 8 mask
			if (texMap.find(1) != texMap.end()) {
				sampler = "\tsampler{\n\t\t";
				sampler += "s_texture=";
				sampler += texMap[1];
				sampler += "\n";
				int normalMapIndex = 0;
				if (texMap.find(5) != texMap.end()) {
					normalMapIndex = 5;
				}
				else if (texMap.find(9) != texMap.end()) {
					normalMapIndex = 9;
				}
				if (normalMapIndex != 0) {
					sampler += "\t\ts_normal=";
					sampler += texMap[normalMapIndex];
					sampler += "\n";
					if (texMap.find(8) != texMap.end()) {
						matName += "_dnlm";//diffuse normal light mask
						program = "\tprogram=posDiffNormalLightMask\n";
						sampler += "\t\tmaskTex=";
						sampler += texMap[8];
						sampler += "\n";
						op = "\top{\n";
						op += "\t\tblend=true,sa,1-sa,add\n";
						op += "\t}\n";
					}
					else {
						matName += "_dnl";
						program = "\tprogram=posDiffNormalLight\n";
					}
				}
				else if (texMap.find(8) != texMap.end()) {
					matName += "_dlm";
					program = "\tprogram=posDiffLightMask\n";
					sampler += "\t\tmaskTex=";
					sampler += texMap[8];
					sampler += "\n";
					op = "\top{\n";
					op += "\t\tblend=true,sa,1-sa,add\n";
					op += "\t}\n";
				}
				else {
					matName += "_dl";
					program = "\tprogram=posDiffLight\n";
				}
				sampler += "\n\t}\n";
			}
			else {
				matName += "_l";
				program = "\tprogram=posLight\n";
			}
		}
		else {
			//如果没有纹理，这个物体的材质名字就默认为
			matName += "_l";
			program = "\tprogram=posLight\n";
		}
		//写入material信息
		stringstream tt;
		tt << matName << "_" << mMeshNum;
		mMaterialName = tt.str();
		string matContents = program + sampler + op;
		auto it = mContentNameMap.find(matContents);
		if (it != mContentNameMap.end()) {
			mMaterialName = it->second;
			pMeshMat = Material::getMaterial(mMaterialName);
		}
		else {
			if (Material::parseMaterial(mMaterialName, matContents)) {
				pMeshMat = Material::getMaterial(mMaterialName);
			}
			else {
				LOGE("parse material failed when import mesh");
			}
			mContentNameMap.emplace(matContents, mMaterialName);
		}

		auto primitivType = mesh->mPrimitiveTypes;
		auto& aabb = mesh->mAABB;
		for (t = 0; t < mesh->mNumFaces; ++t) {
			auto face = &mesh->mFaces[t];
			auto drawType = face->mNumIndices;
			if (drawType != 3) {
				std::cout << "discard a mesh,the drawType is <<drawType " << ",not triangle" << std::endl;
				break;
			}
			/*switch (face->mNumIndices)
			{
			case 1: face_mode = GL_POINTS; break;
			case 2: face_mode = GL_LINES; break;
			case 3: face_mode = GL_TRIANGLES; break;
			default: face_mode = GL_POLYGON; break;
			}*/

			for (int i = 0; i < face->mNumIndices; i++)		// go through all vertices in face
			{
				auto vertexIndex = face->mIndices[i];	// get group index for current index
				auto tempIt = mIndexMap.find(vertexIndex + indexTranslate);
				if (tempIt != mIndexMap.end()) {
					mIndexes.emplace_back(tempIt->second);
				}
				else {
					auto pPos = &mesh->mVertices[vertexIndex];
					auto vec3Size = sizeof(*pPos);
					mPos.emplace_back(pPos->x);
					mPos.emplace_back(pPos->y);
					mPos.emplace_back(pPos->z);
					mIndexes.emplace_back(mCurIndex);
					mIndexMap.emplace(vertexIndex + indexTranslate, mCurIndex);
					++mCurIndex;

					if (mesh->mNormals != nullptr)
					{
						auto pNormal = &mesh->mNormals[vertexIndex];
						mNormals.emplace_back(pNormal->x);
						mNormals.emplace_back(pNormal->y);
						mNormals.emplace_back(pNormal->z);
					}

					if (mesh->mTangents != nullptr)
					{
						auto pTangent = &mesh->mTangents[vertexIndex];
						mTangents.emplace_back(pTangent->x);
						mTangents.emplace_back(pTangent->y);
						mTangents.emplace_back(pTangent->z);
					}

					if (mesh->HasTextureCoords(0))		//HasTextureCoords(texture_coordinates_set)
					{
						mTexcoords.emplace_back(mesh->mTextureCoords[0][vertexIndex].x);
						mTexcoords.emplace_back(mesh->mTextureCoords[0][vertexIndex].y);
					}
				}
			}
		}
		MeshP pMesh = make_shared<Mesh>(MeshType::MESH_DIY);
		if (pMesh->loadMesh(mPos, mTexcoords, mNormals, mIndexes)) {
			if (pMeshMat) {
				pMesh->setMaterial(pMeshMat);
				pRootNode->addAttachment(pMesh);
			}
			else {
				LOGE( "%s mesh material is null",__func__);
			}
		}
		else {
			LOGE("%s load mesh failed", __func__);
		}
		++mMeshNum;
	}
	
	// recursive all children
	for (n = 0; n < nd->mNumChildren; ++n)
	{
		SP_Node pChidNode = pRootNode->newAChild();
		recursive_parse(sc, nd->mChildren[n], pChidNode);
	}
}

bool MeshLoaderAssimp::loadFromFile(const std::string& path, std::shared_ptr<Node<glm::mat4>>& pRootNode) {
	MeshLoaderAssimpImpl impl;
	return impl.loadFromFile(path, pRootNode);
}

bool MeshLoaderAssimpImpl::loadFromFile(const std::string& filename, std::shared_ptr<Node<glm::mat4>>& pRootNode) {
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
	mFileName = Utils::getFileName(filename);
	

	auto pNode = mpAiScene->mRootNode;
	recursive_parse(mpAiScene, pNode, pRootNode);

	return true;
}
