#ifndef _MESH_BEZIER_H_
#define _MESH_BEZIER_H_
#include "Mesh.h"
/*
n个控制点[P1,P2,....Pn]，通过控制点生成一段曲线，曲线会经过P1,Pn,逼近P2,P3...Pn-1。
顶点越多，阶数越高
*/
class MeshBezier : public Mesh {
public:
	MeshBezier();
	~MeshBezier();
	
	//生成n段hermiter曲线，4个点生成一段曲线。
	void loadMesh(const std::vector<float>& p) override;
protected:
	void draw(int posloc = -1, int texloc = -1, int norloc = -1, int colorloc = -1) override;
};

#endif