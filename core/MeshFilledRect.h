#pragma once

#include "Mesh.h"
#include "Rect.h"
#include <glm/ext/vector_float3.hpp>                // vec3
#include <vector>
#include "Color.h"
/*
可填充的矩形mesh，内部生成了纹理坐标
可以绘制为填充的，或者是线圈
*/
class MeshFilledRect : public Mesh {
public:
	MeshFilledRect();

	~MeshFilledRect();
	
	virtual void loadMesh(float rightTopRadius, float leftTopRadius, float leftDownRadius,
		float rightDownRadius, int gradientAngle, float centerX, float centerY, float width, float height) {

	}

	virtual void loadMesh(float radius, int gradientAngle,float centerX, float centerY, float width, float height) {

	}
	virtual void loadMesh(float width, float height, int gradientAngle, float centerX, float centerY);

	//创建一份color数据，
	unsigned int createAColorData(float angle, const Color& startColor, const Color& endColor, const Color& centerColor);
	//设置为一个填充的还是一个边框
	void setFilled(bool b) {
		mbFilled = b;
		if (b) {
			mDrawType = DrawType::TriangleFan;
		}
		else {
			mDrawType = DrawType::LineStrip;
		}
		
	}
	void setColorVbo(unsigned int vbo);
	unsigned int getColorVbo() {
		return mColorVbo;
	}
protected:
	//void draw(int posloc = -1, int texloc = -1, int norloc = -1, int colorloc = -1, int tangentloc = -1) override;
	void drawLineStrip(int posloc) override;
	bool mbFilled{true};
	float mCenterX{ 0.0f };
	float mCenterY{ 0.0f };
	float mWidth{ 0.0f };
	float mHeight{ 0.0f };
	std::vector<glm::vec3> mPoints;
	std::vector<GLuint> mExtraColorVbos;
};
