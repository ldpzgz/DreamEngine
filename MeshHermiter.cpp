#include "MeshHermiter.h"

MeshHermiter::MeshHermiter():
	Mesh(MeshType::MESH_Cubic_Hermiter_Curves)
{

}

MeshHermiter::~MeshHermiter() {

}

void MeshHermiter::draw(int posloc, int texloc, int norloc, int colorloc) {
	drawLineStrip(posloc);
}

void MeshHermiter::loadMesh(const std::vector<float>& points) {
	int index = 0;
	auto size = points.size();
	auto endIndex = size / 12;
	std::vector<float> pos;

	glm::mat4x4 Mh(
		1.0f, 0.0f, -3.0f, 2.0f,
		0.0f, 0.0f, 3.0f, -2.0f,
		0.0f, 1.0f, -2.0f, 1.0f,
		0.0f, 0.0f, -1.0f, 1.0f
	);

	while (index < endIndex) {

		glm::mat3x4 G(
			points[12 * index], points[12 * index + 3], points[12 * index + 6], points[12 * index + 9],
			points[12 * index + 1], points[12 * index + 4], points[12 * index + 7], points[12 * index + 10],
			points[12 * index + 2], points[12 * index + 5], points[12 * index + 8], points[12 * index + 11]
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
		LOGD("error to loadHermiterCurves\n");
	}
}