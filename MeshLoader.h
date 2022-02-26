#ifndef _MESH_LOADER_H_
#define _MESH_LOADER_H_
#include <memory>
#include <string>
#include "Node.h"
class MeshLoader {
public:
	MeshLoader() = default;
	virtual ~MeshLoader() {

	}
	virtual bool loadFromFile(const std::string& path, std::shared_ptr<Node<glm::mat4>>& pRootNode) = 0;
};

#endif