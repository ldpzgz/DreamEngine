#include "MeshBezier.h"
#include "Log.h"
#include <cmath>

MeshBezier::MeshBezier():
	Mesh(MeshType::BezierCurves)
{

}

MeshBezier::~MeshBezier() {

}

void MeshBezier::draw(int posloc) {
	drawLineStrip(posloc);
}

void MeshBezier::getBezierPoints(const std::vector<glm::vec3>& points, int num, std::vector<glm::vec3>& result) {
	int index = 0;
	auto n = points.size() - 1;
	std::vector<float> tk;
	std::vector < float> t_1k;
	std::vector<float> nk;
	auto ni = std::tgammaf(n + 1);
	for (int k = 0; k <= n; ++k) {
		if (k <= n / 2) {
			auto nki = ni / std::tgammaf(k + 1) / std::tgammaf(n - k + 1);
			nk.emplace_back(nki);
		}
		else {
			nk.push_back(nk[n - k]);
		}
	}
	float numf = num - 1;
	for (int i = 0; i < num; ++i) {
		glm::vec3 vec(0.0f, 0.0f, 0.0f);
		for (int k = 0; k <= n; ++k) {
			auto t = i / numf;
			vec += nk[k] * (float)std::pow(t, k) * (float)std::pow(1 - t, n - k) * points[k];
		}
		result.emplace_back(vec);
	}
}

void MeshBezier::loadMesh(const std::vector<glm::vec3>& points,int num) {
	std::vector<glm::vec3> pos;
	getBezierPoints(points, num, pos);
	int numOfVertex = pos.size();
	bool b = createBufferObject((GLfloat*)pos.data(), sizeof(glm::vec3)* numOfVertex, numOfVertex, nullptr, 0);
	if (!b)
	{
		LOGD("error to load Bezier Curves\n");
	}
}