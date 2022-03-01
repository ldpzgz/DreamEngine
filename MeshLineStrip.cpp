#include "MeshLineStrip.h"
#include "Log.h"
MeshLineStrip::MeshLineStrip():
	Mesh(MeshType::MESH_Line_strip,DrawType::LineStrip)
{

}

MeshLineStrip::~MeshLineStrip() {

}

void MeshLineStrip::loadMesh(const std::vector<Vec3>& points) {
	int numOfVertex = points.size();
	bool b = createBufferObject((GLfloat*)points.data(), sizeof(Vec3)* numOfVertex, numOfVertex,nullptr, 0);
	if (!b)
	{
		LOGD("error to load MeshLineStrip\n");
	}
}

void MeshLineStrip::loadMesh(const std::vector<Vec2>& points) {
	int numOfVertex = points.size();
	bool b = createBufferObject((GLfloat*)points.data(), sizeof(Vec2) * numOfVertex, numOfVertex,nullptr, 0);
	if (!b)
	{
		LOGD("error to load MeshLineStrip\n");
	}
}