#pragma once

#include "Mesh.h"
/*
一段hermiter曲线，由两个顶点，以及两个顶点处的切向量，插值而成，
这段曲线会经过两个顶点，切向量由大小和方向，切向量的大小和方向共同控制曲线的形状。
如果要使得两段hermiter曲线g1连续，则连接点处的切向量方向要相同,如果是C1连续，大小也得相同。
*/
class MeshHermiter : public Mesh {
public:
	MeshHermiter();
	~MeshHermiter();
	
	//生成n段hermiter曲线，4个点生成一段曲线,num表示每段曲线多少个点
	void loadMesh(const std::vector<glm::vec3>& p,int num) override;
protected:
	void draw(int posloc = -1) override;
};
