#ifndef _MESH_LOADER_H_
#define _MESH_LOADER_H_
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

#endif