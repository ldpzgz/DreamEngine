#include "MeshCubicSpline.h"
#include "Spline.h"
#include "Log.h"
MeshCubicSpline::MeshCubicSpline():
	Mesh(MeshType::CubicSpline)
{

}

MeshCubicSpline::~MeshCubicSpline() {

}

void MeshCubicSpline::draw(int posloc) {
	drawLineStrip(posloc);
}

void MeshCubicSpline::loadMesh(const std::vector<glm::vec3>& points,int num) {
	std::vector<glm::vec3> pos;
	std::vector<double> X;
	std::vector<double> Y;
	std::vector<double> Z;
	std::vector<double> T;
	double totalT = 0.0f;
	float t = 0;
	const glm::vec3* pPre = nullptr;
	T.emplace_back(0.0);
	for (auto& p : points) {
		X.emplace_back(p.x);
		Y.emplace_back(p.y);
		Z.emplace_back(p.z);
		if (pPre != nullptr) {
			auto distance = glm::distance(*pPre,p);
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
		glm::vec3 p{0.0f,0.0f,0.0f};
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
	bool b = createBufferObject((GLfloat*)pos.data(), sizeof(glm::vec3) * numOfVertex, 
		numOfVertex, nullptr, 0);
	if (!b)
	{
		LOGD("error to load MeshCubicSpline\n");
	}
}

void MeshCubicSpline::loadMesh(const std::vector<glm::vec2>& points,int num) {
	std::vector<glm::vec2> pos;
	std::vector<double> X;
	std::vector<double> Y;
	std::vector<double> T;
	double totalT = 0.0f;
	float t = 0;
	const glm::vec2* pPre = nullptr;
	T.emplace_back(0.0);
	for (auto& p : points) {
		X.emplace_back(p.x);
		Y.emplace_back(p.y);
		if (pPre != nullptr) {
			auto distance = glm::distance(*pPre, p);
			totalT += distance;
			T.emplace_back(totalT);
		}
		pPre = &p;
	}
	tk::spline s1(T, X);
	tk::spline s2(T, Y);
	double index = 0.0;
	for (int i = 0; i < num; ++i) {
		glm::vec2 p(0.0f,0.0f);
		index = (double)i * totalT / (double)(num - 1);
		if (index > totalT) {
			index = totalT;
		}
		p.x = s1(index);
		p.y = s2(index);
		pos.emplace_back(p);
	}

	int numOfVertex = pos.size();
	bool b = createBufferObject((GLfloat*)pos.data(), sizeof(glm::vec2) * numOfVertex, 
		numOfVertex, nullptr, 0);
	if (!b)
	{
		LOGD("error to load MeshCubicSpline\n");
	}
}