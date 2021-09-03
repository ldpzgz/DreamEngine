#ifndef _MESH_FILLED_RECT_H_
#define _MESH_FILLED_RECT_H_
#include "Mesh.h"
#include "Rect.h"
/*
可填充的矩形mesh，内部生成了纹理坐标
可以绘制为填充的，或者是线圈
*/
class MeshFilledRect : public Mesh {
public:
	MeshFilledRect();

	~MeshFilledRect();
	/*
	功能	创建一个被[0,0,width,height]盒子包围的椭圆
	centerX	圆的中心点x的坐标
	centerY	圆的中心点y的坐标
	*/
	virtual void loadMesh(float rightTopRadius, float leftTopRadius, float leftDownRadius,
		float rightDownRadius, float centerX, float centerY, float width, float height) {

	}

	virtual void loadMesh(float radius, float centerX, float centerY, float width, float height) {

	}
	virtual void loadMesh(float width, float height, float centerX, float centerY);
	void setColorData(float angle, const Color& startColor, const Color& endColor, const Color& centerColor);
	//设置为一个填充的圆还是一个圆圈
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