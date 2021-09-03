#ifndef _MESH_CIRCLE_H_
#define _MESH_CIRCLE_H_
#include "MeshFilledRect.h"
/*
椭圆mesh，内部生成了纹理坐标，不过纹理坐标是按圆来生成的，如果是椭圆，显示处理的图片会变形
可以绘制为填充的，或者是线圈
*/
class MeshCircle : public MeshFilledRect {
public:
	MeshCircle();

	~MeshCircle();
	/*
	功能	创建一个被[0,0,width,height]盒子包围的椭圆
	centerX	圆的中心点x的坐标
	centerY	圆的中心点y的坐标
	*/
	void loadMesh(float width, float height, float centerX, float centerY) override;
protected:
};

#endif