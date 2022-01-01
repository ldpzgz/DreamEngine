#ifndef _MESH_LOADER_H_
#define _MESH_LOADER_H_
#include "Node.h"
class MeshLoader {
public:
	MeshLoader() = default;
	virtual ~MeshLoader() {

	}
	virtual std::shared_ptr<Node<glm::mat4>> loadFromFile(const std::string path) = 0;
};

#endif