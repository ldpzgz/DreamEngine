#include "MeshCRSpline.h"

MeshCRSpline::MeshCRSpline():
	Mesh(MeshType::MESH_Catmull_Rom_Splines)
{

}

MeshCRSpline::~MeshCRSpline() {

}

void MeshCRSpline::draw(int posloc, int texloc, int norloc, int colorloc) {
	drawLineStrip(posloc);
}

void MeshCRSpline::loadMesh(const std::vector<Vec3>& points,int num) {
	
	auto endIndex = points.size();
	std::vector<Vec3> pos;

	glm::mat4x4 Mh(
		0.0f, -1.0f, 2.0f, -1.0f,
		2.0f, 0.0f, -5.0f, 3.0f,
		0.0f, 1.0f, 4.0f, -3.0f,
		0.0f, 0.0f, -1.0f, 1.0f
	);
	Mh *= 0.5f;
	int index = 1;
	while (index <= endIndex - 3) {

		glm::mat3x4 G(
			points[index - 1].x, points[index].x, points[index + 1].x, points[index + 2].x,
			points[index - 1].y, points[index].y, points[index + 1].y, points[index + 2].y,
			points[index - 1].z, points[index].z, points[index + 1].z, points[index + 2].z
		);
		float numf = num - 1;
		for (int i = 0; i < num; ++i) {
			glm::vec4 t;
			t.x = 1.0f;
			t.y = (float)i / numf;
			t.z = t.y*t.y;
			t.w = t.z*t.y;

			auto v = t*Mh*G;
			pos.emplace_back(v.x, v.y, v.z);
		}
		++index;
	}
	int numOfVertex = pos.size();
	bool b = createBufferObject((float*)pos.data(), sizeof(Vec3)* numOfVertex, numOfVertex, nullptr, 0);
	if (!b)
	{
		LOGD("error to loadCRSplines\n");
	}
}