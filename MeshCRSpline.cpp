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

void MeshCRSpline::loadMesh(const std::vector<float>& points) {
	int index = 1;
	auto size = points.size();
	auto endIndex = size / 3;
	std::vector<float> pos;

	glm::mat4x4 Mh(
		0.0f, -1.0f, 2.0f, -1.0f,
		2.0f, 0.0f, -5.0f, 3.0f,
		0.0f, 1.0f, 4.0f, -3.0f,
		0.0f, 0.0f, -1.0f, 1.0f
	);
	Mh *= 0.5f;

	while (index <= endIndex - 3) {

		glm::mat3x4 G(
			points[3 * (index - 1)], points[3 * index], points[3 * (index + 1)], points[3 * (index + 2)],
			points[3 * (index - 1) + 1], points[3 * index + 1], points[3 * (index + 1) + 1], points[3 * (index + 2) + 1],
			points[3 * (index - 1) + 2], points[3 * index + 2], points[3 * (index + 1) + 2], points[3 * (index + 2) + 2]
		);

		for (int i = 0; i < 50; ++i) {
			glm::vec4 t;
			t.x = 1.0f;
			t.y = i*0.02f;
			t.z = t.y*t.y;
			t.w = t.z*t.y;

			auto v = t*Mh*G;
			pos.emplace_back(v.x);
			pos.emplace_back(v.y);
			pos.emplace_back(v.z);
		}
		++index;
	}

	bool b = createBufferObject(pos.data(), sizeof(float)*pos.size(), nullptr, 0);
	if (!b)
	{
		LOGD("error to loadCRSplines\n");
	}
}