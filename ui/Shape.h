#pragma once

#include "core/Log.h"
#include "core/Color.h"
#include <string>
#include <functional>
#include <unordered_map>
#include <memory>
enum class ShapeType : int {
	Rectangle,
	RoundedRectangle,
	Oval,
};
enum class GradientType:int{
	None,
	Linear,
	Radial,//Radial，sweep这两个的效果没什么鸟用，不实现了
	Sweep
};

/*
* 表示一个形状，有三种矩形，圆角矩形，椭圆
* 形状可以填充：
*	设置了这三种情况的一种：solidColor.isZero() || pTexture || gradientType != GradientType::None
*	就需要填充，代表三种填充模式：纯色，纹理，渐变色。
	如果
* 可以有边框：
*	设置了：strokeColor，就会有一个边框，否则没有边框。
*/
class MeshFilledRect;
class Texture;
class Shape {
public:
	explicit Shape(ShapeType type) :
		mType(type)
	{

	}
	Shape() = default;
	~Shape() = default;
	void setType(const std::string& str) {
		if (str == "rectangle") {
			mType = ShapeType::Rectangle;
		}
		else if (str == "oval") {
			mType = ShapeType::Oval;
		}
		else if (str == "roundedRectangle") {
			mType = ShapeType::RoundedRectangle;
		}
		else {
			LOGE("error to parse shape type %s", str.c_str());
		}
	}
	void setType(ShapeType type) {
		mType = type;
	}
	ShapeType getType() {
		return mType;
	}
	void setCornerRadius(int radius) {
		mRadius = radius;
	}
	int getCornerRadius() {
		return mRadius;
	}
	void setCornerLeftTopRadius(int radius) {
		mLeftTopRadius = radius;
	}
	int getCornerLeftTopRadius() {
		return mLeftTopRadius;
	}

	void setCornerLeftBottomRadius(int radius) {
		mLeftBottomRadius = radius;
	}
	int getCornerLeftBottomRadius() {
		return mLeftBottomRadius;
	}

	void setCornerRightTopRadius(int radius) {
		mRightTopRadius = radius;
	}
	int getCornerRightTopRadius() {
		return mRightTopRadius;
	}

	void setCornerRightBottomRadius(int radius) {
		mRightBottomRadius = radius;
	}
	int getCornerRightBottomRadius() {
		return mRightBottomRadius;
	}

	void setOvalWidth(float width) {
		mOvalWidth = width;
	}
	float getOvalWidth() {
		return mOvalWidth;
	}

	void setOvalHeight(float height) {
		mOvalHeight = height;
	}
	float getOvalHeight() {
		return mOvalHeight;
	}

	void setSolidColor(const Color& color) {
		mSolidColor = color;
	}
	Color& getSolidColor() {
		return mSolidColor;
	}

	void setBorderWidth(int width) {
		mBorderWidth = width;
	}
	int getBorderWidth() {
		return mBorderWidth;
	}

	void setBorderColor(const Color& color) {
		mBorderColor = color;
	}
	Color& getBorderColor() {
		return mBorderColor;
	}

	void setSrokeDashWidth(int width) {
		mDashWidth = width;
	}
	int getSrokeDashWidth() {
		return mDashWidth;
	}

	void setSrokeDashGap(int gap) {
		mDashGap = gap;
	}
	int getSrokeDashGap() {
		return mDashGap;
	}

	void setGradientAngle(int angle) {
		mAngle = angle;
	}
	int getGradientAngle() {
		return mAngle;
	}

	void setGradientCenterX(float x) {
		mCenterX = x;
	}
	float getGradientCenterX() {
		return mCenterX;
	}

	void setGradientCenterY(float x) {
		mCenterX = x;
	}
	float getGradientCenterY() {
		return mCenterY;
	}

	void setGradientStartColor(const Color& color) {
		mStartColor = color;
	}
	Color& getGradientStartColor() {
		return mStartColor;
	}

	void setGradientEndColor(const Color& color) {
		mEndColor = color;
	}
	Color& getGradientEndColor() {
		return mEndColor;
	}

	void setGradientCenterColor(const Color& color) {
		mCenterColor = color;
	}
	Color& getGradientCenterColor() {
		return mCenterColor;
	}

	void setGradientRadius(float radius) {
		mGradientRadius = radius;
	}
	float getGradientRadius() {
		return mGradientRadius;
	}

	void setGradientType(const std::string& type) {
		if (type == "Linear") {
			mGradientType = GradientType::Linear;
		}
		else if (type == "Radial") {
			mGradientType = GradientType::Radial;
		}
		else if (type == "Sweep") {
			mGradientType = GradientType::Sweep;
		}
		else {
			LOGE("ERROR cannot recognize gradient type %s",type.c_str());
		}
			
	}
	GradientType getGradientType() {
		return mGradientType;
	}

	void setPadding(int pad) {
		mPadding = pad;
	}
	int getPadding() {
		return mPadding;
	}

	void setPaddingLeft(int left) {
		mPaddingLeft = left;
	}
	int getPaddingLeft() {
		return mPaddingLeft;
	}

	void setPaddingRight(int right) {
		mPaddingRight = right;
	}
	int getPaddingRight() {
		return mPaddingRight;
	}

	void setPaddingTop(int Top) {
		mPaddingTop = Top;
	}
	int getPaddingTop() {
		return mPaddingTop;
	}

	void setPaddingBottom(int Bottom) {
		mPaddingBottom = Bottom;
	}
	int getPaddingBottom() {
		return mPaddingBottom;
	}

	void setTexture(const std::shared_ptr<Texture>& pTex) {
		mpTexture = pTex;
	}

	std::shared_ptr<Texture>& getTexture() {
		return mpTexture;
	}

	static std::unordered_map<std::string, std::function<void(const std::shared_ptr<Shape>&, const std::string&)>> gShapeAttributeHandler;
public:
	ShapeType mType{ ShapeType::Rectangle };
	//corners的属性，单位是像素
	int mRadius{ 0 };
	int mLeftTopRadius{ 0 };
	int mLeftBottomRadius{ 0 };
	int mRightTopRadius{ 0 };
	int mRightBottomRadius{ 0 };
	//solid属性
	Color mSolidColor{ 0.0f,0.0f,0.0f,1.0f };
	//stroke属性
	int mBorderWidth{ 1 };
	Color mBorderColor{ 0.0f,0.0f,0.0f,1.0f };
	int mDashWidth{ 0 };//实线的长度
	int mDashGap{ 0 };//虚线的长度
	//padding属性,单位是像素
	int mPadding{ 0 };
	int mPaddingLeft{ 0 };
	int mPaddingTop{ 0 };
	int mPaddingRight{ 0 };
	int mPaddingBottom{ 0 };
	//gradient属性
	int mAngle{ 0 };//linear
	float mCenterX{ 0.5f };//取值范围[0.0f,1.0f]
	float mCenterY{ 0.5f };//只实现linear，centerx，y在有centerColor的情况下才使用
	Color mStartColor{ 0.0f,0.0f,0.0f,1.0f };
	Color mEndColor{ 0.0f,0.0f,0.0f,1.0f };
	Color mCenterColor{ 0.0f,0.0f,0.0f,1.0f };
	float mGradientRadius{ 0.0f };//raida
	GradientType mGradientType{ GradientType::None };

	//用于计算椭圆的宽高比
	float mOvalWidth{ 1.0f };
	float mOvalHeight{ 1.0f };

	std::shared_ptr<Texture> mpTexture;
private:
	static void typeHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
	static void cornerRadiusHandler(const std::shared_ptr<Shape>& shape,const std::string& value);
	static void cornerLeftTopRadiusHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
	static void cornerLeftBottomRadiusHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
	static void cornerRightTopRadiusHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
	static void cornerRightBottomRadiusHandler(const std::shared_ptr<Shape>& shape, const std::string& value);

	static void textureHandler(const std::shared_ptr<Shape>& shape, const std::string& value);

	static void gradientAngleHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
	static void gradientCenterXHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
	static void gradientCenterYHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
	static void gradientCenterColorHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
	static void gradientStartColorHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
	static void gradientEndColorHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
	static void gradientRadiusHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
	static void gradientTypeHandler(const std::shared_ptr<Shape>& shape, const std::string& value);

	static void paddingHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
	static void paddingLeftHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
	static void paddingRightHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
	static void paddingBottomHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
	static void paddingTopHandler(const std::shared_ptr<Shape>& shape, const std::string& value);

	static void sizeWidthHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
	static void sizeHeightHandler(const std::shared_ptr<Shape>& shape, const std::string& value);

	static void solidColorHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
	static void strokeWidthHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
	static void strokeColorHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
	//static void strokeDashWidthHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
	//static void strokeDashGapHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
};