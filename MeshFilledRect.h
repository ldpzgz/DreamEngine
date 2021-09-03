#ifndef _MESH_FILLED_RECT_H_
#define _MESH_FILLED_RECT_H_
#include "Mesh.h"
#include "Rect.h"
/*
�����ľ���mesh���ڲ���������������
���Ի���Ϊ���ģ���������Ȧ
*/
class MeshFilledRect : public Mesh {
public:
	MeshFilledRect();

	~MeshFilledRect();
	/*
	����	����һ����[0,0,width,height]���Ӱ�Χ����Բ
	centerX	Բ�����ĵ�x������
	centerY	Բ�����ĵ�y������
	*/
	virtual void loadMesh(float rightTopRadius, float leftTopRadius, float leftDownRadius,
		float rightDownRadius, float centerX, float centerY, float width, float height) {

	}

	virtual void loadMesh(float radius, float centerX, float centerY, float width, float height) {

	}
	virtual void loadMesh(float width, float height, float centerX, float centerY);
	void setColorData(float angle, const Color& startColor, const Color& endColor, const Color& centerColor);
	//����Ϊһ������Բ����һ��ԲȦ
	void setFilled(bool b) {
		mbFilled = b;
	}
protected:
	void draw(int posloc = -1, int texloc = -1, int norloc = -1, int colorloc = -1) override;
	void drawLineStrip(int posloc) override;
	bool mbFilled{true};
	float mCenterX{ 0.0f };
	float mCenterY{ 0.0f };
	float mWidth{ 0.0f };
	float mHeight{ 0.0f };
	vector<Vec3> mPoints;
};

#endif