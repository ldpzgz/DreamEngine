#ifndef _MESH_HERMITER_H_
#define _MESH_HERMITER_H_
#include "Mesh.h"
/*
һ��hermiter���ߣ����������㣬�Լ��������㴦������������ֵ���ɣ�
������߻ᾭ���������㣬�������ɴ�С�ͷ����������Ĵ�С�ͷ���ͬ�������ߵ���״��
���Ҫʹ������hermiter����g1�����������ӵ㴦������������Ҫ��ͬ,�����C1��������СҲ����ͬ��
*/
class MeshHermiter : public Mesh {
public:
	MeshHermiter();
	~MeshHermiter();
	
	//����n��hermiter���ߣ�4��������һ�����ߡ�
	void loadMesh(const std::vector<float>& p) override;
protected:
	void draw(int posloc = -1, int texloc = -1, int norloc = -1, int colorloc = -1) override;
};

#endif