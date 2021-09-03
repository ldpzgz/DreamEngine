#ifndef _MESH_CIRCLE_H_
#define _MESH_CIRCLE_H_
#include "MeshFilledRect.h"
/*
��Բmesh���ڲ��������������꣬�������������ǰ�Բ�����ɵģ��������Բ����ʾ�����ͼƬ�����
���Ի���Ϊ���ģ���������Ȧ
*/
class MeshCircle : public MeshFilledRect {
public:
	MeshCircle();

	~MeshCircle();
	/*
	����	����һ����[0,0,width,height]���Ӱ�Χ����Բ
	centerX	Բ�����ĵ�x������
	centerY	Բ�����ĵ�y������
	*/
	void loadMesh(float width, float height, float centerX, float centerY) override;
protected:
};

#endif