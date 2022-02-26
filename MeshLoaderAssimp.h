#ifndef _MESH_LOADER_ASSIMP_H_
#define _MESH_LOADER_ASSIMP_H_
#include "MeshLoader.h"

class MeshLoaderAssimp : public MeshLoader {
public:
	MeshLoaderAssimp() = default;
	virtual ~MeshLoaderAssimp() {

	}
	bool loadFromFile(const std::string& path, std::shared_ptr<Node<glm::mat4>>& pRootNode) override;
};

#endif