#include "MeshBezier.h"

MeshBezier::MeshBezier():
	Mesh(MeshType::MESH_Bezier_Curves)
{

}

MeshBezier::~MeshBezier() {

}

void MeshBezier::draw(int posloc, int texloc, int norloc, int colorloc) {
	drawLineStrip(posloc);
}

void MeshBezier::loadMesh(const std::vector<float>& points) {
	int index = 0;
	auto n = points.size() / 3 - 1;
	std::vector<float> pos;
	std::vector<float> tk;
	std::vector < float> t_1k;
	std::vector<float> nk;
	auto ni = std::tgammaf(n + 1);//注意这个计算的是n的阶乘
								  //计数多项式系数中的有阶乘运算的部分
	for (int k = 0; k <= n; ++k) {
		if (k <= n / 2) {
			auto nki = ni / std::tgammaf(k + 1) / std::tgammaf(n - k + 1);
			nk.emplace_back(nki);
		}
		else {
			nk.push_back(nk[n - k]);
		}
	}
	constexpr float delta = 1.0f / 1000.0f;
	for (int i = 0; i < 1000; ++i) {
		glm::vec3 vec(0.0f, 0.0f, 0.0f);
		for (int k = 0; k <= n; ++k) {
			glm::vec3 Pn(points[3 * k], points[3 * k + 1], points[3 * k + 2]);
			auto t = i*delta;
			vec += nk[k] * std::powf(t, k) * std::powf(1 - t, n - k)*Pn;
		}
		pos.emplace_back(vec.x);
		pos.emplace_back(vec.y);
		pos.emplace_back(vec.z);
	}

	bool b = createBufferObject(pos.data(), sizeof(float)*pos.size(), nullptr, 0);
	if (!b)
	{
		LOGD("error to load Bezier Curves\n");
	}
}