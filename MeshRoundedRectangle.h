#ifndef _MESH_ROUNDED_RECTANGLE_H_
#define _MESH_ROUNDED_RECTANGLE_H_
#include "MeshFilledRect.h"
/*

*/
class MeshRoundedRectangle : public MeshFilledRect {
public:
	MeshRoundedRectangle();
	~MeshRoundedRectangle();

	void loadMesh(float rightTopRadius, float leftTopRadius, float leftDownRadius,
		float rightDownRadius, int gradientAngle,float centerX, float centerY, float width, float height) override;

	void loadMesh(float radius, int gradientAngle, float centerX, float centerY, float width, float height) override;
protected:
};
#endif