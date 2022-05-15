#pragma once

#include<memory>
#include "../Color.h"
class Shape;
class Texture;
class MeshFilledRect;

enum class FillType {
	Fill_Solid,
	Fill_Gradient,
	Fill_Texture,
};
/*
	* style有三种填充模式1 纹理，2渐变色，3纯色
	* 对于同一种填充模式的style，可以共享同一个shape
	*/
class BackgroundStyle {
public:
	std::shared_ptr<Shape> mpShape;
	std::shared_ptr<Texture> mpTex;//如果不为空，就表示用纹理填充，优先级最高
	Color mStartColor;//如果不为空，就表示用渐变色填充，优先级次之
	Color mCenterColor;
	Color mEndColor;
	Color mSolidColor;//如果不为空就表示用纯色填充，最低优先级
	Color mBorderColor;
	unsigned int mColorVbo{ 0 };
	FillType mFillType{ FillType::Fill_Solid };
	std::shared_ptr<MeshFilledRect> mpBkMesh;
	std::shared_ptr<MeshFilledRect> mpBkBorderMesh;

	void setFillType(FillType type) {
		mFillType = type;
	}

	auto getFillType() {
		return mFillType;
	}

	void setColorVbo(unsigned int vbo) {
		mColorVbo = vbo;
	}
	unsigned int getColorVbo() {
		return mColorVbo;
	}
	Color& getStartColor() {
		return mStartColor;
	}
	Color& getCenterColor() {
		return mCenterColor;
	}
	Color& getEndColor() {
		return mEndColor;
	}
	Color& getSolidColor() {
		return mSolidColor;
	}
	Color& getBorderColor() {
		return mBorderColor;
	}
	std::shared_ptr<Shape>& getShape() {
		return mpShape;
	}
	void setMyStyle();

	void setShape(const std::shared_ptr<Shape>& pShape) {
		mpShape = pShape;
	}
	
	std::shared_ptr<Texture>& getTexture() {
		return mpTex;
	}
	
	void setTexture(const std::shared_ptr<Texture>& pTex);

	void setStartColor(const Color& c);

	void setCenterColor(const Color& c);

	void setEndColor(const Color& c);

	void setBorderColor(const Color& c);

	void setSolidColor(const Color& c);

	MeshFilledRect* getBkMesh();

	MeshFilledRect* getBkBorderMesh();

	void setBkMesh(std::shared_ptr<MeshFilledRect>& pMesh);
	void setBkBorderMesh(std::shared_ptr<MeshFilledRect>& pMesh);
};