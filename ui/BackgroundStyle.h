#pragma once

#include<memory>
#include "core/Color.h"
class Shape;
class Texture;
class MeshFilledRect;

enum class FillType {
	Fill_Solid,
	Fill_Gradient,
	Fill_Texture,
};
/*
	* style���������ģʽ1 ������2����ɫ��3��ɫ
	* ����ͬһ�����ģʽ��style�����Թ���ͬһ��shape
	*/
class BackgroundStyle {
public:
	std::shared_ptr<Shape> mpShape;
	std::shared_ptr<Texture> mpTex;//�����Ϊ�գ��ͱ�ʾ��������䣬���ȼ����
	Color mStartColor;//�����Ϊ�գ��ͱ�ʾ�ý���ɫ��䣬���ȼ���֮
	Color mCenterColor;
	Color mEndColor;
	Color mSolidColor;//�����Ϊ�վͱ�ʾ�ô�ɫ��䣬������ȼ�
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