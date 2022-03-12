#include "MeshNurbs.h"
#include "Log.h"
MeshNurbs::MeshNurbs():
	Mesh(MeshType::MESH_NURBS)
{

}

MeshNurbs::~MeshNurbs() {

}

void MeshNurbs::draw(int posloc, int texloc, int norloc, int colorloc, int tangentloc) {
	drawLineStrip(posloc);
}

void MeshNurbs::loadMesh(const std::vector<glm::vec3>& P, const std::vector<int>& knots, const std::vector<float>& w) {
	std::vector<glm::vec3> pos;
	auto n = P.size() - 1;
	float u = 0.0f;
	float delta = (n - 2) / 1000.0f;
	for (; u < n - 2; u += delta) {
		int i = std::floor(u);

		float a = Niku(i, 3, u, knots);
		float b = Niku(i + 1, 3, u, knots);
		float c = Niku(i + 2, 3, u, knots);
		float d = Niku(i + 3, 3, u, knots);

		auto wu = a * w[i] + b * w[i + 1] + c * w[i + 2] + d * w[i + 3];
		auto vec = a * w[i] / wu * P[i] + b * w[i+1] / wu * P[i+1] + c * w[i+2] / wu * P[i+2] + d * w[i+3] / wu * P[i+3];
		pos.emplace_back(vec);
	}
	/*for (auto xyz : P) {
		pos.emplace_back(xyz);
	}*/
	int numOfVertex = pos.size();
	bool b = createBufferObject((float*)pos.data(), sizeof(glm::vec3)* numOfVertex, numOfVertex,nullptr, 0);
	if (!b)
	{
		LOGD("error to loadCRSplines\n");
	}
}

float MeshNurbs::Niku(int i, int k, float u, const std::vector<int>& knots) {
	float ti_2 = knots[i];
	float ti_1 = knots[i + 1];
	if (k == 0) {
		if (u >= ti_2 && u < ti_1) {
			return 1.0f;
		}
		else {
			return 0.0f;
		}
	}
	else {
		float tik_2 = knots[i + k];
		float tik_1 = knots[i + k + 1];
		float a = u - ti_2;
		float b = tik_1 - u;
		float x = 0.0f;
		float y = 0.0f;
		if (tik_2 - ti_2 > 0.00001) {
			x = Niku(i, k - 1, u, knots) / (tik_2 - ti_2);
		}
		if (tik_1 - ti_1 > 0.00001) {
			y = Niku(i + 1, k - 1, u, knots) / (tik_1 - ti_1);
		}
		return a*x + b*y;
	}

}