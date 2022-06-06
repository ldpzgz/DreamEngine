#include "MeshLoaderGltf.h"
#include "Node.h"
#include "Material.h"
#include "Resource.h"
#include "Texture.h"
#include "Sampler.h"
#include "Log.h"
#include "Vbo.h"
#include "Mesh.h"
#define CGLTF_IMPLEMENTATION
#include "cgltf.h"
#include <iostream>
#include <unordered_map>
#ifdef _GLES3
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>
#else
#include <glad/glad.h>
#endif
#include <unordered_map>
#include <string_view>
#include <vector>
#include "AnimationManager.h"
#include "animation/Skeleton.h"
#include "animation/SkeletonAnimation.h"
#include<glm/ext/quaternion_common.hpp>
#include<glm/ext/quaternion_transform.hpp>
#include <glm/gtx/quaternion.hpp>

struct AccRet {
	void* data{nullptr};
	int size{0};
	int offset{0};
	int stride{0};
	int count{0};
	int vboIndex{ -1 };
};

class MeshLoaderGltfImpl : public MeshLoader {
public:
	MeshLoaderGltfImpl() = default;
	~MeshLoaderGltfImpl() {

	}
	bool loadFromFile(const std::string& path, std::shared_ptr<Node>& pRootNode) override;
	
	void recursive_parse(const cgltf_node* pNode, std::shared_ptr<Node>& pMyNode);

	void parseBuffer(cgltf_data* data);//parse vbo
	void parseMaterial(cgltf_data* data);
	void parseMesh(cgltf_data* data);
	void parseNode(cgltf_data* data);
	void parseSkeleton(cgltf_data* data);
	void parseAnimationInfo(cgltf_data* pData);

	
	AccRet getAccessorData(cgltf_accessor* pAcc);
	std::string mCurrentParseFile;//for out debug info
	std::unordered_map<std::string, std::unique_ptr<MaterialInfo> > mMaterialsMap;//all material
	std::unordered_map<size_t, std::shared_ptr<Vbo> > mVboMap;//all vbo
	std::unordered_multimap<size_t, std::shared_ptr<Mesh> > mMeshMap;//all mesh
	std::unordered_map<size_t, std::shared_ptr<Node> > mNodeMap;//all node
	std::vector< std::shared_ptr<Node>> mRootNode;
	std::unordered_map < std::string, std::shared_ptr<Skeleton>> mSkeletonMap;//allSkeleton
};
bool MeshLoaderGltf::loadFromFile(const std::string& path, std::shared_ptr<Node>& pRootNode) {
	MeshLoaderGltfImpl impl;
	return impl.loadFromFile(path, pRootNode);
}

bool MeshLoaderGltfImpl::loadFromFile(const std::string& path, std::shared_ptr<Node>& pRootNode) {
	mCurrentParseFile = path;
	cgltf_options options;
	memset(&options, 0, sizeof(cgltf_options));
	cgltf_data* data = NULL;
	//分析gltf文件
	cgltf_result result = cgltf_parse_file(&options, path.c_str(), &data);

	if (result == cgltf_result_success) {
		//从bin文件加载数据
		result = cgltf_load_buffers(&options, data, path.c_str());
	}
	
	if (result == cgltf_result_success) {
		result = cgltf_validate(data);
		if (result == cgltf_result_success)
		{
			LOGD("load gltf Result: %d\n", result);
			LOGD("load gltf Type: %u\n", data->file_type);
			LOGD("load gltf Meshes count: %u\n", (unsigned)data->meshes_count);
			parseBuffer(data);
			parseMaterial(data);
			parseMesh(data);
			parseNode(data);
			parseSkeleton(data);
			parseAnimationInfo(data);
			cgltf_free(data);

			for (auto& pRoot : mRootNode) {
				pRootNode->addChild(pRoot);
				/*bool isSkeletonNode = false;
				for (auto& pske : skeletonMap) {
					if (pRoot == pske.second->getRootNode()) {
						isSkeletonNode = true;
					}
				}
				if (!isSkeletonNode) {
					pRootNode->addChild(pRoot);
				}*/
			}

			return true;
		}
	}
	cgltf_free(data);
	return false;
}
AccRet MeshLoaderGltfImpl::getAccessorData(cgltf_accessor* pAcc) {
	if (pAcc == nullptr) {
		return { nullptr,0,0,0,0};
	}
	auto pBv = pAcc->buffer_view;
	int offset = static_cast<int>(pAcc->offset);
	int stride = pAcc->stride;

	int count = static_cast<int>(pAcc->count);
	int component_type = pAcc->component_type;
	int dataType = pAcc->type;//scalar,vec2,vec3,vec4 mat2,mat3,mat4
	int componentSize = 0;
	int componentCount = 0;
	switch (component_type) {
	case cgltf_component_type_r_8:
	case cgltf_component_type_r_8u:
		componentSize = 1;
		break;
	case cgltf_component_type_r_16:
	case cgltf_component_type_r_16u:
		componentSize = 2;
		break;
	case cgltf_component_type_r_32u:
	case cgltf_component_type_r_32f:
		componentSize = 4;
		break;
	default:
		break;
	};

	switch (dataType) {
	case cgltf_type_scalar:
		componentCount = 1;
		break;
	case cgltf_type_vec2:
		componentCount = 2;
		break;
	case cgltf_type_vec3:
		componentCount = 3;
		break;
	case cgltf_type_vec4:
		componentCount = 4;
		break;
	case cgltf_type_mat2:
		componentCount = 4;
		break;
	case cgltf_type_mat3:
		componentCount = 9;
		break;
	case cgltf_type_mat4:
		componentCount = 16;
		break;
	default:
		break;
	};
	int size = componentSize* componentCount* count;
	
	void* pData = nullptr;
	if (pBv != nullptr) {
		//size = static_cast<int>(pBv->size);
		offset += pBv->offset;
		if (pBv->stride != 0) {
			stride = pBv->stride;
		}
		/*if (stride == componentSize * componentCount) {
			stride = 0;
		}*/
		
		if (pBv->data != nullptr) {
			pData = (void*)((char*)pBv->data + offset);
		}
		else if (pBv->buffer != nullptr) {
			pData = (void*)((char*)pBv->buffer->data + offset);
		}
	}
	return { pData,size,offset,stride,count};
}
void MeshLoaderGltfImpl::parseBuffer(cgltf_data* data) {
	auto pBuffers = data->buffers;
	auto bufferCounts = data->buffers_count;

	for (int i = 0; i < bufferCounts; ++i) {
		auto pVbo = std::make_shared<Vbo>();
		pVbo->initVbo(pBuffers->data, pBuffers->size);
		if (pBuffers->name != nullptr) {
			LOGD("buffer name in gltf is %s", pBuffers->name);
		}
		else {
			mVboMap.try_emplace(reinterpret_cast<size_t>(pBuffers), pVbo);
		}
		++pBuffers;
	}
}

void MeshLoaderGltfImpl::parseMaterial(cgltf_data* data) {
	auto pMaterial = data->materials;
	auto materialCount = data->materials_count;
	auto loadTextureFunc = [](cgltf_texture* pTex)->std::shared_ptr<Texture> {
		std::shared_ptr<Texture> pRet;
		auto pImage = pTex->image;
		auto pSampler = pTex->sampler;
		if (pImage != nullptr) {
			std::string name;
			if (pImage->name != nullptr) {
				name = pImage->name;
			}
			if (!name.empty()) {
				pRet = Resource::getInstance().getTexture(name);
			}
			if (!pRet) {
				std::string imageUri{ pImage->uri };
				if (!imageUri.empty()) {
					//for load file which path has spaces(such path will be encoded with base64 codec)
					auto  decodedSize = cgltf_decode_uri(imageUri.data());
					imageUri.resize(decodedSize);
					pRet = Resource::getInstance().getOrLoadTextureFromFile(imageUri, name);
					if (!pRet) {
						std::cout << "Error load texture from uri:" << imageUri << std::endl;
					}
				}
				else {
					std::cout << "Error image uri is empty,may be the texture data is in a buffer" << std::endl;
				}
			}
		}
		else {
			std::cout << "ERROR has texture but the image is null" << std::endl;
		}

		if (pRet && pSampler != nullptr) {
			LOGD("texture in gltf has define a sampler");
			auto pS = Sampler::getSampler(pSampler->min_filter,
				pSampler->mag_filter,
				pSampler->wrap_s,
				pSampler->wrap_t);
			pRet->setSampler(pS);
			//pSampler->extensions; 其他参数应该在这个扩展里面
		}
		return pRet;
	};
	for (int i = 0; i < materialCount; ++i) {
		auto pInfo = std::make_unique<MaterialInfo>();
		//the material name must not null,otherwise we cann't find material for mesh;
		if (pMaterial->name == nullptr) {
			LOGE("parse material in the gltf file %s,a material has no name",
				mCurrentParseFile.c_str());
			continue;
		}
		pInfo->name = pMaterial->name;
		//load texture from uri and set it sampler
		if (pMaterial->has_pbr_metallic_roughness) {
			pInfo->metallicRoughnessWorkFlow = true;
			auto& albedoTexView = pMaterial->pbr_metallic_roughness.base_color_texture;
			auto& meteRoughTexView = pMaterial->pbr_metallic_roughness.metallic_roughness_texture;
			if (albedoTexView.texture) {
				auto ptex = loadTextureFunc(albedoTexView.texture);
				if (ptex) {
					pInfo->albedoMap = ptex->getName();
				}
			}
			else {
				pInfo->albedoColor.setColor4(pMaterial->pbr_metallic_roughness.base_color_factor);
			}
			if (meteRoughTexView.texture) {
				auto ptex = loadTextureFunc(meteRoughTexView.texture);
				if (ptex) {
					pInfo->armMap = ptex->getName();
				}
			}
			pInfo->metallic = pMaterial->pbr_metallic_roughness.metallic_factor;
			pInfo->roughness = pMaterial->pbr_metallic_roughness.roughness_factor;
			
		}
		else if (pMaterial->has_pbr_specular_glossiness) {
			pInfo->metallicRoughnessWorkFlow = false;
			auto& albedoTexView = pMaterial->pbr_specular_glossiness.diffuse_texture;
			auto& specGlosTexView = pMaterial->pbr_specular_glossiness.specular_glossiness_texture;

			if (albedoTexView.texture) {
				auto pTex = loadTextureFunc(albedoTexView.texture);
				if (pTex) {
					pInfo->albedoMap = pTex->getName();
				}
			}
			else {
				pInfo->albedoColor.setColor4(pMaterial->pbr_specular_glossiness.diffuse_factor);
			}
			if (specGlosTexView.texture != nullptr) {
				auto pTex = loadTextureFunc(specGlosTexView.texture);
				if (pTex) {
					pInfo->specGlosMapPath = pTex->getName();
				}
			}
			else {
				pInfo->specularColor.setColor3(pMaterial->pbr_specular_glossiness.specular_factor);
				pInfo->roughness = 1.0f - pMaterial->pbr_specular_glossiness.glossiness_factor;
			}
		}
		auto pNormalTex = pMaterial->normal_texture.texture;
		if (pNormalTex != nullptr) {
			auto pTex = loadTextureFunc(pNormalTex);
			if (pTex) {
				pInfo->normalMap = pTex->getName();
			}
		}
		std::string op{"op{\n"};
		if (pMaterial->double_sided) {
			op += "\tcullFace=false\n";
		}
		if (pMaterial->alpha_mode == cgltf_alpha_mode_blend) {
			op += "\tblend=true,sa,1-sa\n";
		}
		op += "}\n";
		pInfo->opString = std::move(op);
		mMaterialsMap.try_emplace(pInfo->name, std::move(pInfo));
		++pMaterial;
		//clearCoat
		//emissive
		//transmission
		// sheen,:sheen color,sheen roughness
		// volume:
		//alpha_mode
	}
}

void MeshLoaderGltfImpl::parseMesh(cgltf_data* data) {
	auto pMesh = data->meshes;
	auto meshCount = data->meshes_count;
	for (int i = 0; i < meshCount; ++i) {
		bool hasSkeletonAnimation{ false };
		bool hasNormal{ false };
		bool hasColorAttribute{ false };
		LOGD("gltf mesh name:%s", pMesh->name );
		//primitive is real mesh,
		auto pPrimitive = pMesh->primitives;
		auto primitiveCount = pMesh->primitives_count;
		for (int j = 0; j < primitiveCount; ++j) {
			if (pPrimitive->has_draco_mesh_compression) {
				std::cout << "draco compression" << std::endl;
			}
			//new a mesh in here,and set material,set vbo for it.
			//indices data and vertex attribute data are all in a single vbo
			//if it not,then error.
			auto pMyMesh = std::make_shared<Mesh>(MeshType::DIY, static_cast<DrawType>(pPrimitive->type));
			mMeshMap.emplace(reinterpret_cast<size_t>(pMesh), pMyMesh);
			//pMyMesh->setDrawType();//point ,line triangle triangle_strip fan

			//index of data in vbo,such as offset,count ,stride,data type(int/float/vec3/vec2/vec4/mat3/mat4)
			auto pAccessor = pPrimitive->indices;

			if (pAccessor) {
				AccRet ret = getAccessorData(pAccessor);
				if (ret.count != 0) {
					auto it = mVboMap.find(reinterpret_cast<size_t>(pAccessor->buffer_view->buffer));
					if (it != mVboMap.end()) {
						ret.vboIndex = pMyMesh->pushVbo(it->second);
						pMyMesh->setIndexSizeOffset(ret.size, ret.offset, ret.stride, ret.count, ret.vboIndex);
					}
					else {
						LOGE(" gltf cannot find vbo for mesh");
					}
				}
				else {
					LOGE(" gltf cannot find vbo for mesh");
				}
			}
			else {
				LOGD("the mesh in gltf has no indecies,so call drawArray");
			}
			
			auto pAttr = pPrimitive->attributes;
			int attrCount = pPrimitive->attributes_count;
			for (int k = 0; k < attrCount; ++k) {
				//vertex ,normal,texcoord etc，
				auto type = pAttr->type;// vertex/normal/texcoord tec
				auto pAccessor = pAttr->data;
				AccRet ret = getAccessorData(pAccessor);
				if (ret.count != 0) {
					auto it = mVboMap.find(reinterpret_cast<size_t>(pAccessor->buffer_view->buffer));
					if (it != mVboMap.end()) {
						ret.vboIndex = pMyMesh->pushVbo(it->second);
					}
					else {
						LOGE(" gltf cannot find vbo for mesh");
					}
				}
				else {
					LOGE(" gltf cannot find vbo for mesh");
				}
				switch (type) {
				case cgltf_attribute_type_position:
					if (pAccessor->has_min && pAccessor->has_max) {
						auto& min = pAccessor->min;
						auto& max = pAccessor->max;
						pMyMesh->setAABB(min[0], max[0], min[1], max[1], min[2],max[2]);
					}
					pMyMesh->setPosSizeOffset(ret.size, ret.offset,ret.stride,ret.count, ret.vboIndex);
					break;
				case cgltf_attribute_type_normal:
					hasNormal = true;
					pMyMesh->setNorSizeOffset(ret.size, ret.offset, ret.stride, ret.vboIndex);
					break;
				case cgltf_attribute_type_texcoord:
					pMyMesh->setTexSizeOffset(ret.size, ret.offset, ret.stride, ret.vboIndex);
					break;
				case cgltf_attribute_type_color:
					hasColorAttribute = true;
					pMyMesh->setColorSizeOffset(ret.size, ret.offset, ret.stride, ret.vboIndex);
					break;
				case cgltf_attribute_type_joints:
					hasSkeletonAnimation = true;
					pMyMesh->setBoneIdSizeOffset(ret.size, ret.offset, ret.stride, ret.vboIndex);
					break;
				case cgltf_attribute_type_weights:
					pMyMesh->setBoneWeightSizeOffset(ret.size, ret.offset, ret.stride,ret.vboIndex);
					break;
				default:
					break;
				}
				++pAttr;
			}
			//set material
			auto pMat = pPrimitive->material;
			if (pMat != nullptr) {
				if (pMat->name == nullptr) {
					//give a default material
					MaterialInfo info;
					info.hasNormal = hasNormal;
					info.hasSkeletonAnimation = hasSkeletonAnimation;
					info.hasVertexColor = hasColorAttribute;
					auto pMaterial = Resource::getInstance().getMaterialDefferedGeoPass(info);
					pMyMesh->setMaterial(pMaterial);
				}
				else {
					auto pmat = Resource::getInstance().getMaterial(pMat->name);
					if (pmat) {
						pMyMesh->setMaterial(pmat);
					}
					else {
						auto it = mMaterialsMap.find(pMat->name);
						if (it != mMaterialsMap.end()) {
							auto& pMatInfo = it->second;
							if (pMatInfo) {
								pMatInfo->hasVertexColor = hasColorAttribute;
								pMatInfo->hasNormal = hasNormal;
								pMatInfo->hasSkeletonAnimation = hasSkeletonAnimation;
								auto pMyMat = Resource::getInstance().getMaterialDefferedGeoPass(*pMatInfo);
								if (pMyMat) {
									pMyMesh->setMaterial(pMyMat);
								}
							}
						}
						else {
							LOGE("gltf mesh not found the material %s", pMat->name);
						}
					}
				}
			}
			else {
				MaterialInfo info;
				info.hasNormal = hasNormal;
				info.hasSkeletonAnimation = hasSkeletonAnimation;
				info.hasVertexColor = hasColorAttribute;
				auto pMaterial = Resource::getInstance().getMaterialDefferedGeoPass(info);
				pMyMesh->setMaterial(pMaterial);
			}
			++pPrimitive;
		}
		++pMesh;
	}
}

void MeshLoaderGltfImpl::parseNode(cgltf_data* data) {
	cgltf_node* pNode = data->nodes;
	auto nodeCount = data->nodes_count;
	std::shared_ptr<Node> pRootNode;
	std::unordered_map<std::string_view, std::shared_ptr<Node>> namedNodes;
	//find all rootNode
	std::vector<cgltf_node*> rootNodes;
	for (int i = 0; i < nodeCount; ++i) {
		if (pNode->parent == nullptr) {
			rootNodes.emplace_back(pNode);
		}
		++pNode;
	}
	//visit all node
	for (auto pNode : rootNodes) {
		auto pMyNode = std::make_shared<Node>();
		mRootNode.emplace_back(pMyNode);
		recursive_parse(pNode, pMyNode);
	}
}

void MeshLoaderGltfImpl::recursive_parse(const cgltf_node* pNode, std::shared_ptr<Node>& pMyNode) {
	mNodeMap.try_emplace(reinterpret_cast<size_t>(pNode), pMyNode);
	if (pNode->name != nullptr) {
		pMyNode->setName(pNode->name);
		LOGD("recursive gltf node name %s", pNode->name);
	}
	
	if (pNode->mesh != nullptr) {
		auto key = reinterpret_cast<size_t>(pNode->mesh);
		auto count = mMeshMap.count(key);
		auto ret = mMeshMap.find(key);
		LOGD("node %s has mesh,count %lld", pNode->name, count);
		while (count-- > 0) {
			auto& pMesh = (ret++)->second;
			pMyNode->addRenderable(pMesh);
			/*if (pMySkeleton) {
				pMesh->setSkeleton(pMySkeleton);
			}*/
		}
	}
	if (pNode->camera != nullptr) {

	}
	if (pNode->light != nullptr) {

	}
	if (pNode->has_matrix) {
		glm::mat4 mat(pNode->matrix[0], pNode->matrix[1], pNode->matrix[2], pNode->matrix[3],
			pNode->matrix[4], pNode->matrix[5], pNode->matrix[6], pNode->matrix[7],
			pNode->matrix[8], pNode->matrix[9], pNode->matrix[10], pNode->matrix[11],
			pNode->matrix[12], pNode->matrix[13], pNode->matrix[14], pNode->matrix[15]);
		pMyNode->setLocalMatrix(mat, false);
	}
	else {
		glm::mat4 translate{ 1.0f };
		glm::mat4 rotate{ 1.0f };
		glm::mat4 scale{ 1.0f };
		if (pNode->has_translation) {
			translate = glm::translate(translate, glm::vec3(pNode->translation[0],
				pNode->translation[1], pNode->translation[2]));
		}
		if (pNode->has_rotation) {
			glm::quat q(pNode->rotation[0], pNode->rotation[1], pNode->rotation[2],
				pNode->rotation[3]);
			rotate = glm::toMat4(q);
		}
		if (pNode->has_scale) {
			scale = glm::scale(scale, glm::vec3(pNode->scale[0], pNode->scale[1], pNode->scale[2]));
		}
		pMyNode->setLocalMatrix(translate * rotate * scale);
	}
	if (pNode->children != nullptr) {
		for (int i = 0; i < pNode->children_count; ++i) {
			auto pChild = pNode->children[i];
			if (pChild != nullptr) {
				auto pMyChildNode = pMyNode->newAChild();
				recursive_parse(pChild, pMyChildNode);
			}
		}
	}
}

void MeshLoaderGltfImpl::parseSkeleton(cgltf_data* data) {
	cgltf_node* pNode = data->nodes;
	auto nodeCount = data->nodes_count;
	for (int i = 0; i < nodeCount; ++i) {
		std::shared_ptr<Skeleton> pMySkeleton;
		while (pNode->skin != nullptr) {
			LOGD("node %s has skin %s", pNode->name, pNode->skin->name);
			std::string skeletonName{ "default" };
			if (pNode->skin->name) {
				skeletonName = pNode->skin->name;
			}
			
			auto skeletonIt = mSkeletonMap.find(skeletonName);
			if (skeletonIt != mSkeletonMap.end()) {
				pMySkeleton = skeletonIt->second;
				break;
			}
			
			//get the offsetMatrix
			auto pAccessor = pNode->skin->inverse_bind_matrices;
			cgltf_node* pSkeleton = pNode->skin->skeleton;
			cgltf_node** pBones = pNode->skin->joints;
			int bonesCount = pNode->skin->joints_count;
			LOGD("find skeletion %s", pNode->skin->name);
			
			pMySkeleton = std::make_shared<Skeleton>(skeletonName);
			if (!mSkeletonMap.try_emplace(skeletonName, pMySkeleton).second) {
				LOGE("skeleton name conflict,%s", skeletonName.c_str());
				break;
			}
			if (pAccessor != nullptr) {
				auto ret = getAccessorData(pAccessor);
				if (ret.data != nullptr) {
					pMySkeleton->setOffsetMatrix(reinterpret_cast<glm::mat4*>(ret.data), ret.count);
				}
				else {
					LOGE("cannot get skeleton's offset matrix");
				}
			}
			if (pBones != nullptr) {
				auto& boneNodes = pMySkeleton->getBoneName2Index();
				for (int i = 0; i < bonesCount; ++i) {
					if (pBones[i] != nullptr) {
						std::string boneName;
						if (pBones[i]->name) {
							boneName = pBones[i]->name;
						}
						else {
							auto key = reinterpret_cast<size_t>(pBones[i]);
							boneName = std::to_string(key);
							auto it = mNodeMap.find(key);
							if (it != mNodeMap.end() && it->second) {
								it->second->setName(boneName);
							}
						}
						LOGD("bone name is %s", boneName.c_str());
						if (!boneNodes.try_emplace(boneName, i).second) {
							LOGE("parse gltf animation bones,the name of bone repeated");
						}
					}
				}
				//find skeleton's root node,if a skeleton has only one root node
				cgltf_node* pRootNode = nullptr;
				auto pBone = pBones[0];
				while (pBone != nullptr) {
					if (pBone->parent == nullptr) {
						pRootNode = pBone;
					}
					else {
						pBone = pBone->parent;
					}
					if (pRootNode != nullptr) {
						auto it = mNodeMap.find(reinterpret_cast<size_t>(pRootNode));
						if (it != mNodeMap.end()) {
							pMySkeleton->setRootNode(it->second);
							LOGD("find skeleton root node %s", pRootNode->name);
						}
						else {
							LOGE("cannot get root node for skeleton");
						}
						break;
					}
				}
				if (pRootNode == nullptr) {
					LOGE("cannot get root node for skeleton");
				}
			}
			break;
		}
		if (pMySkeleton && pNode->mesh != nullptr) {
			auto key = reinterpret_cast<size_t>(pNode->mesh);
			auto count = mMeshMap.count(key);
			auto ret = mMeshMap.find(key);
			LOGD("node %s has mesh,count %lld", pNode->name, count);
			while (count-- > 0) {
				auto& pMesh = (ret++)->second;
				pMesh->setSkeleton(pMySkeleton);
			}
		}
		++pNode;
	}
}

void MeshLoaderGltfImpl::parseAnimationInfo(cgltf_data* pData) {
	//one SkeletonAnimation has one skeleton
	auto pAnimation = pData->animations;
	int animatCount = pData->animations_count;
	for (int i = 0; i < animatCount; ++i) {
		if (pAnimation != nullptr) {
			std::string animationName("default");
			if (pAnimation->name != nullptr) {
			/*	LOGD("find animation with no name,so give a random name");
				animationName = "animation" + Utils::nowTime();
			}
			else {*/
				LOGD("find animation %s", pAnimation->name);
				animationName = pAnimation->name;
			}
			
			std::shared_ptr<Animation> pMyAnimation;
			AnimationType animatType{AnimationType::NodeAnimation};
			int64_t duration = 0;
			//a channel contains a set of keyframes of a node
			//a node may have four kinds of keyframe(rotate,translate,scale,weight)
			//a channel only contains one kind of keyframe
			auto pChannel = pAnimation->channels;
			int channelCount = pAnimation->channels_count;

			if (channelCount <= 0) {
				continue;
			}
			//find skeleton for this animation
			std::shared_ptr<Skeleton> pSkeleton;
			
			auto pTargetNode = pChannel->target_node;
			if (pTargetNode != nullptr) {
				std::string targetNodeName;
				if (pTargetNode->name != nullptr) {
					targetNodeName = pTargetNode->name;
				}
				else {
					targetNodeName = std::to_string(reinterpret_cast<size_t>(pTargetNode));
				}
				for (auto& ske : mSkeletonMap) {
					auto& nameIndexMap = ske.second->getBoneName2Index();
					auto it = nameIndexMap.find(targetNodeName);
					if (it != nameIndexMap.end()) {
						pSkeleton = ske.second;
						animatType = AnimationType::SkeletonAnimation;
						break;
					}
				}
					
				pMyAnimation = Animation::createAnimation(animatType, animationName);
				AnimationManager::getInstance().addAnimation(animationName, pMyAnimation);
				if (pSkeleton) {
					pMyAnimation->setAffectedSkeleton(pSkeleton);
					pSkeleton->addAnimation(pMyAnimation);
				}
				else {
					auto it = mNodeMap.find(reinterpret_cast<size_t>(pTargetNode));
					if (it != mNodeMap.end()) {
						pMyAnimation->setTargetNode(it->second);
						if (it->second) {
							it->second->setHasNodeAnimation(true);
							it->second->setAny(NodeAnyIndex::NodeAnimationMatrix, glm::mat4(1.0f));
						}
					}
				}	
			}
			for (int j = 0; j < channelCount; ++j) {
				auto pTargetNode = pChannel->target_node;
				if (pTargetNode != nullptr) {
					auto pSampler = pChannel->sampler;
					int dataType = pChannel->target_path;//translate,rotate,scale,weight
					if (pSampler) {
						//get keyframe info
						//time is float type in seconds;
						std::string targetNodeName;
						if (pTargetNode->name != nullptr) {
							targetNodeName = pTargetNode->name;
						}
						else {
							targetNodeName = std::to_string(reinterpret_cast<size_t>(pTargetNode));
						}
						auto retTime = getAccessorData(pSampler->input);
						auto retMat = getAccessorData(pSampler->output);
						if (dataType == cgltf_animation_path_type_translation) {
							pMyAnimation->setPosKeyFrame(targetNodeName,
								(float*)retTime.data, (glm::vec3*)retMat.data, 
								retTime.count, static_cast<InterpolationType>(pSampler->interpolation));
						}
						else if (dataType == cgltf_animation_path_type_rotation) {
							auto pRotate = std::make_unique<glm::quat[]>(retTime.count);
							glm::quat* pQuat = (glm::quat*)retMat.data;
							for (int i = 0; i < retTime.count; ++i) {
								pRotate[i].w = pQuat->z;
								pRotate[i].x = pQuat->w;
								pRotate[i].y = pQuat->x;
								pRotate[i].z = pQuat->y;
								++pQuat;
							}
							pMyAnimation->setRotateKeyFrame(targetNodeName,
								(float*)retTime.data, pRotate.get(),
								retTime.count, static_cast<InterpolationType>(pSampler->interpolation));
						}
						else if (dataType == cgltf_animation_path_type_scale) {
							pMyAnimation->setScaleKeyFrame(targetNodeName,
								(float*)retTime.data, (glm::vec3*)retMat.data, 
								retTime.count, static_cast<InterpolationType>(pSampler->interpolation));
						}
						else {
						}
						int64_t maxTime = ((float*)retTime.data)[retTime.count - 1]*1000;
						if (maxTime > duration) {
							duration = maxTime;
						}
					}
				}
				else {
					LOGE("node animation channel has no targetNode");
				}
				++pChannel;
			}
			if (pMyAnimation) {
				pMyAnimation->setDuration(duration);
			}
			++pAnimation;
		}
	}
}