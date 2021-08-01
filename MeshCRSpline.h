#ifndef _MESH_CR_SPLINE_H_
#define _MESH_CR_SPLINE_H_
#include "Mesh.h"

/*
����ǹ�n�㣬����C1�������ʵ����ߣ���hermiter����ԭ��Ļ����ϣ��伸�ξ�����4��������ɣ�����c1�������Ƶ���������
�����㼯[P0,P1,P2.....Pn],���Ƴ�����[P1,P2.....P(n-1)]������
�������ߣ��ı�����һ�����λ�ã�ֻӰ���ڽ���������֮������ߡ�
*/
class MeshCRSpline : public Mesh {
public:
	MeshCRSpline();
	~MeshCRSpline();
	
	void loadMesh(const std::vector<float>& p) override;
protected:
	void draw(int posloc = -1, int texloc = -1, int norloc = -1) override;
};

#endif