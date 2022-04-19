#ifndef _MESH_LOADER_ASSIMP_H_
#define _MESH_LOADER_ASSIMP_H_
#include "MeshLoader.h"
#include <unordered_map>
#include <memory>
#include <string>
/*
* 利用assimp库进行模型加载
*/
class MeshLoaderAssimp : public MeshLoader {
public:
	MeshLoaderAssimp() = default;
	~MeshLoaderAssimp() {

	}
	bool loadFromFile(const std::string& path, std::shared_ptr<Node>& pRootNode) override;
};

#endif