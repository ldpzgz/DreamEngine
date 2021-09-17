#ifndef _MESH_LINE_STRIP_H_
#define _MESH_LINE_STRIP_H_
#include "Mesh.h"
#include "Rect.h"

class MeshLineStrip : public Mesh {
public:
	MeshLineStrip();
	~MeshLineStrip();
	
	void loadMesh(const std::vector<Vec3>& p) override;
	void loadMesh(const std::vector<Vec2>& p) override;
};

#endif