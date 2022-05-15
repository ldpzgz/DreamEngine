#pragma once

#include "Mesh.h"
#include "Rect.h"

class MeshLineStrip : public Mesh {
public:
	MeshLineStrip();
	~MeshLineStrip();
	
	void loadMesh(const std::vector<glm::vec3>& p) override;
	void loadMesh(const std::vector<glm::vec2>& p) override;
};
