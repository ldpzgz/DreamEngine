#ifndef _MESH_LOADER_GLTF_H_
#define _MESH_LOADER_GLTF_H_
#include "MeshLoader.h"
class Node;
class MeshLoaderGltf : public MeshLoader {
public:
	MeshLoaderGltf() = default;
	~MeshLoaderGltf() = default;
	bool loadFromFile(const std::string& path, std::shared_ptr<Node>& pRootNode) override;
};

#endif