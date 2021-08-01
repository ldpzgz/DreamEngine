#ifndef _MESH_NURBS_H_
#define _MESH_NURBS_H_
#include "Mesh.h"

class MeshNurbs : public Mesh {
public:
	MeshNurbs();
	~MeshNurbs();
	/*
	P[P0...Pn]
	knots[t(0)..t(n+4)]
	w[w0......wn,wn+1,..w(n+4)];
	*/
	void loadMesh(const std::vector<float>& P, const std::vector<int>& knots,const std::vector<float>& w) override;
protected:
	void draw(int posloc = -1, int texloc = -1, int norloc = -1) override;
private:
	//计算nonuniform b-splines的per-control-point blend function,k是阶数，三阶就好了
	float Niku(int i, int k, float u, const std::vector<int>& knots);
};

#endif