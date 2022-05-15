#pragma once

#include "MeshLoader.h"
class Node;
class MeshLoaderGltf : public MeshLoader {
public:
	MeshLoaderGltf() = default;
	~MeshLoaderGltf() = default;
	bool loadFromFile(const std::string& path, std::shared_ptr<Node>& pRootNode) override;
};
