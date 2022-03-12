#include "MeshLineStrip.h"
#include "Log.h"
MeshLineStrip::MeshLineStrip():
	Mesh(MeshType::MESH_Line_strip,DrawType::LineStrip)
{

}

MeshLineStrip::~MeshLineStrip() {

}

void MeshLineStrip::loadMesh(const std::vector<glm::vec3>& points) {
	int numOfVertex = points.size();
	bool b = createBufferObject((GLfloat*)points.data(), sizeof(glm::vec3)* numOfVertex, numOfVertex,nullptr, 0);
	if (!b)
	{
		LOGD("error to load MeshLineStrip\n");
	}
}

void MeshLineStrip::loadMesh(const std::vector<glm::vec2>& points) {
	int numOfVertex = points.size();
	bool b = createBufferObject((GLfloat*)points.data(), sizeof(glm::vec2) * numOfVertex, numOfVertex,nullptr, 0);
	if (!b)
	{
		LOGD("error to load MeshLineStrip\n");
	}
}