#ifndef _MESH_NURBS_H_
#define _MESH_NURBS_H_
#include "Mesh.h"
/*
* 1 先说uniform B样条：
* b 样条具有local control，以及C2 连续的特征，
* {P0,P1,...Pn}n+1个控制点，可以组成n-2段cubic曲线{Q1，Q2....Qn-2},cubic曲线Qi由4个控制点加权合成
* Qi(t)=B0(t)*P(i+0-1) + B1(t)*P(i+1-1) + B2(t)*P(i+2-1) + B3(t)*P(i+3-1)，t属于[0,1]
* 这个方程可以写为这种通用的形式：Qi(t) = G(bs)*Mbs*[1,t,t2,t3]; G(bs)=[Pi-1,Pi,Pi+1,Pi+2]是几何矩阵，已知,Mbs就是要求的4x4的基本矩阵。
* 只要指定Bk(t)这几个权重系数就可以算出Qi这条曲线了，就是根据曲线是C2连续这个特征，
* 以及B0(t)+B1(t)+B2(t)+B3(t)=1这个特征，列出16个方程，求解得到Mbs。
* 这n-2段曲线连接的点，{Q2(0),Q3(0),....Qn-2(0)}叫做knots，n+1个控制点，有n-3个knots。
* 可以将B样条的每一段Qi(t),都写为所有控制点{P0,P1,...Pn}的加权和。
*/

class MeshNurbs : public Mesh {
public:
	MeshNurbs();
	~MeshNurbs();
	/*
	P[P0...Pn]
	knots[t(0)..t(n+4)]
	w[w0......wn,wn+1,..w(n+4)];
	*/
	void loadMesh(const std::vector<Vec3>& P, const std::vector<int>& knots,const std::vector<float>& w) override;
protected:
	void draw(int posloc = -1, int texloc = -1, int norloc = -1,int colorloc = -1, int tangentloc=-1) override;
private:
	//计算nonuniform b-splines的per-control-point blend function,k是阶数，三阶就好了
	float Niku(int i, int k, float u, const std::vector<int>& knots);
};

#endif