#include "MeshHermiter.h"
#include "Log.h"
MeshHermiter::MeshHermiter():
	Mesh(MeshType::CubicHermiterCurves)
{

}

MeshHermiter::~MeshHermiter() {

}

void MeshHermiter::draw(int posloc, int texloc, int norloc, int colorloc,int tangentloc) {
	drawLineStrip(posloc);
}

void MeshHermiter::loadMesh(const std::vector<glm::vec3>& points,int num) {
	auto size = points.size();
	auto endIndex = size / 4;
	std::vector<glm::vec3> pos;

	glm::mat4x4 Mh(
		1.0f, 0.0f, -3.0f, 2.0f,
		0.0f, 0.0f, 3.0f, -2.0f,
		0.0f, 1.0f, -2.0f, 1.0f,
		0.0f, 0.0f, -1.0f, 1.0f
	);
	float numf = num - 1;
	size_t index = 0;
	while (index < endIndex) {

		glm::mat3x4 G(
			points[4 * index].x, points[4 * index + 1].x, points[4 * index + 2].x, points[4 * index + 3].x,
			points[4 * index].y, points[4 * index + 1].y, points[4 * index + 2].y, points[4 * index + 3].y,
			points[4 * index].z, points[4 * index + 1].z, points[4 * index + 2].z, points[4 * index + 3].z
		);

		for (int i = 0; i < num; ++i) {
			glm::vec4 t;
			t.x = 1.0f;
			t.y = i / numf;
			t.z = t.y*t.y;
			t.w = t.z*t.y;

			auto v = t*Mh*G;
			pos.emplace_back(v.x,v.y,v.z);
		}
		++index;
	}
	int numOfVertex = pos.size();
	bool b = createBufferObject((float*)pos.data(), sizeof(glm::vec3)* numOfVertex, numOfVertex,nullptr, 0);
	if (!b)
	{
		LOGD("error to loadHermiterCurves\n");
	}
}