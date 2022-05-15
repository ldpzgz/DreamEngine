#pragma once

#include <memory>
#include <string>
class Node;
class MeshLoader {
public:
	MeshLoader() = default;
	virtual ~MeshLoader() {

	}
	virtual bool loadFromFile(const std::string& path, std::shared_ptr<Node>& pRootNode) = 0;
};
