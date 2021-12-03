#include "MeshCubicSpline.h"
#include "Spline.h"
MeshCubicSpline::MeshCubicSpline():
	Mesh(MeshType::MESH_Cubic_Spline)
{

}

MeshCubicSpline::~MeshCubicSpline() {

}

void MeshCubicSpline::draw(int posloc, int texloc, int norloc, int colorloc,int tangentloc) {
	drawLineStrip(posloc);
}

void MeshCubicSpline::loadMesh(const std::vector<Vec3>& points,int num) {
	std::vector<Vec3> pos;
	std::vector<double> X;
	std::vector<double> Y;
	std::vector<double> Z;
	std::vector<double> T;
	double totalT = 0.0f;
	float t = 0;
	const Vec3* pPre = nullptr;
	T.emplace_back(0.0);
	for (auto& p : points) {
		X.emplace_back(p.x);
		Y.emplace_back(p.y);
		Z.emplace_back(p.z);
		if (pPre != nullptr) {
			auto distance = pPre->distance(p);
			totalT += distance;
			T.emplace_back(totalT);
		}
		pPre = &p;
	}
	tk::spline s1(T, X);
	tk::spline s2(T, Y);
	tk::spline s3(T, Z);
	double index = 0.0;
	for (int i = 0; i < num; ++i) {
		Vec3 p;
		index = (double)i * totalT / (double)(num - 1);
		if (index > totalT) {
			index = totalT;
		}
		p.x = s1(index);
		p.y = s2(index);
		p.z = s3(index);
		pos.emplace_back(p);
	}

	int numOfVertex = pos.size();
	bool b = createBufferObject((GLfloat*)pos.data(), sizeof(Vec3) * numOfVertex, numOfVertex, nullptr, 0);
	if (!b)
	{
		LOGD("error to load MeshCubicSpline\n");
	}
}

void MeshCubicSpline::loadMesh(const std::vector<Vec2>& points,int num) {
	std::vector<Vec2> pos;
	std::vector<double> X;
	std::vector<double> Y;
	std::vector<double> T;//����һ��ʱ�����T������������
	double totalT = 0.0f;
	float t = 0;
	const Vec2* pPre = nullptr;
	T.emplace_back(0.0);
	for (auto& p : points) {
		X.emplace_back(p.x);
		Y.emplace_back(p.y);
		if (pPre != nullptr) {
			auto distance = pPre->distance(p);
			totalT += distance;
			T.emplace_back(totalT);
		}
		pPre = &p;
	}
	tk::spline s1(T, X);
	tk::spline s2(T, Y);
	double index = 0.0;
	for (int i = 0; i < num; ++i) {
		Vec2 p;
		index = (double)i * totalT / (double)(num - 1);
		if (index > totalT) {
			index = totalT;
		}
		p.x = s1(index);
		p.y = s2(index);
		pos.emplace_back(p);
	}

	int numOfVertex = pos.size();
	bool b = createBufferObject((GLfloat*)pos.data(), sizeof(Vec2) * numOfVertex, numOfVertex, nullptr, 0);
	if (!b)
	{
		LOGD("error to load MeshCubicSpline\n");
	}
}