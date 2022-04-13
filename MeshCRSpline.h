#ifndef _MESH_CR_SPLINE_H_
#define _MESH_CR_SPLINE_H_
#include "Mesh.h"

/*
这个是过n点，具有C1连续性质的曲线，在hermiter曲线原理的基础上，其几何矩阵由4个顶点组成，根据c1连续，推导出基矩阵，
给定点集[P0,P1,P2.....Pn],绘制出过点[P1,P2.....P(n-1)]的曲线
这种曲线，改变其中一个点的位置，只影响邻近的两个点之间的曲线。
*/
class MeshCRSpline : public Mesh {
public:
	MeshCRSpline();
	~MeshCRSpline();
	//num表示两个控制点之间插入的点的个数
	void loadMesh(const std::vector<glm::vec3>& p,int num) override;
protected:
	void draw(int posloc = -1) override;
};

#endif