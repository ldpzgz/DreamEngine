#include "MeshBezier.h"
#include "Log.h"

MeshBezier::MeshBezier():
	Mesh(MeshType::MESH_Bezier_Curves)
{

}

MeshBezier::~MeshBezier() {

}

void MeshBezier::draw(int posloc, int texloc, int norloc, int colorloc, int tangentloc) {
	drawLineStrip(posloc);
}

void MeshBezier::getBezierPoints(const std::vector<Vec3>& points, int num, std::vector<Vec3>& result) {
	int index = 0;
	auto n = points.size() - 1;
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
	float numf = num - 1;
	for (int i = 0; i < num; ++i) {
		Vec3 vec(0.0f, 0.0f, 0.0f);
		for (int k = 0; k <= n; ++k) {
			auto t = i / numf;
			vec += nk[k] * std::powf(t, k) * std::powf(1 - t, n - k) * points[k];
		}
		result.emplace_back(vec);
	}
}

void MeshBezier::loadMesh(const std::vector<Vec3>& points,int num) {
	std::vector<Vec3> pos;
	getBezierPoints(points, num, pos);
	int numOfVertex = pos.size();
	bool b = createBufferObject((GLfloat*)pos.data(), sizeof(Vec3)* numOfVertex, numOfVertex, nullptr, 0);
	if (!b)
	{
		LOGD("error to load Bezier Curves\n");
	}
}