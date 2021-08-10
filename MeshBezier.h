#ifndef _MESH_BEZIER_H_
#define _MESH_BEZIER_H_
#include "Mesh.h"
/*
n�����Ƶ�[P1,P2,....Pn]��ͨ�����Ƶ�����һ�����ߣ����߻ᾭ��P1,Pn,�ƽ�P2,P3...Pn-1��
����Խ�࣬����Խ��
*/
class MeshBezier : public Mesh {
public:
	MeshBezier();
	~MeshBezier();
	
	//����n��hermiter���ߣ�4��������һ�����ߡ�
	void loadMesh(const std::vector<float>& p) override;
protected:
	void draw(int posloc = -1, int texloc = -1, int norloc = -1, int colorloc = -1) override;
};

#endif