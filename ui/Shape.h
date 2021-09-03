#ifndef _SHAPE_H_
#define _SHAPE_H_
#include "../Rect.h"
#include "../Log.h"
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
	Radial,//Radial��sweep��������Ч��ûʲô���ã���ʵ����
	Sweep
};

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
	void setCornerRadius(float radius) {
		mRadius = radius;
	}
	float getCornerRadius() {
		return mRadius;
	}
	void setCornerLeftTopRadius(float radius) {
		mLeftTopRadius = radius;
	}
	float getCornerLeftTopRadius() {
		return mLeftTopRadius;
	}

	void setCornerLeftBottomRadius(float radius) {
		mLeftBottomRadius = radius;
	}
	float getCornerLeftBottomRadius() {
		return mLeftBottomRadius;
	}

	void setCornerRightTopRadius(float radius) {
		mRightTopRadius = radius;
	}
	float getCornerRightTopRadius() {
		return mRightTopRadius;
	}

	void setCornerRightBottomRadius(float radius) {
		mRightBottomRadius = radius;
	}
	float getCornerRightBottomRadius() {
		return mRightBottomRadius;
	}

	void setSizeWidth(float width) {
		mWidth = width;
	}
	float getSizeWidth() {
		return mWidth;
	}

	void setSizeHeight(float height) {
		mHeight = height;
	}
	float getSizeHeight() {
		return mHeight;
	}

	void setSolidColor(const Color& color) {
		mSolidColor = color;
	}
	Color& getSolidColor() {
		return mSolidColor;
	}

	void setSrokeWidth(int width) {
		mSrokeWidth = width;
	}
	int getSrokeWidth() {
		return mSrokeWidth;
	}

	void setSrokeColor(const Color& color) {
		mSrokeColor = color;
	}
	Color& getSrokeColor() {
		return mSrokeColor;
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

	void setGradientType(GradientType type) {
		mGradientType = type;
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
	std::shared_ptr<void>& getTexture() {
		return mpTexture;
	}
	void setTexture(std::shared_ptr<void>& pTexture) {
		mpTexture = pTexture;
	}
	std::shared_ptr<void>& getMesh() {
		return mpMesh;
	}
	void setMesh(std::shared_ptr<void>& pMesh) {
		mpMesh = pMesh;
	}
	void setStrokeMesh(std::shared_ptr<void>& pStrokeMesh) {
		mpStrokeMesh = pStrokeMesh;
	}
	std::shared_ptr<void>& getStrokeMesh() {
		return mpStrokeMesh;
	}

	void setInitialized() {
		mbInitialized = true;
	}

	bool getInitialized() {
		return mbInitialized;
	}

	static std::unordered_map<std::string, std::function<void(const std::shared_ptr<Shape>&, const std::string&)>> gShapeAttributeHandler;
public:
	ShapeType mType{ ShapeType::Rectangle };
	//corners�����ԣ�ȡֵ��Χ��[0��0.5]
	float mRadius{ 0.0f };
	float mLeftTopRadius{ 0.0f };
	float mLeftBottomRadius{ 0.0f };
	float mRightTopRadius{ 0.0f };
	float mRightBottomRadius{ 0.0f };
	//solid����
	Color mSolidColor{ 0.0f,0.0f,0.0f,1.0f };
	//stroke����
	int mSrokeWidth{ 0 };
	Color mSrokeColor{ 0.0f,0.0f,0.0f,1.0f };
	int mDashWidth{ 0 };//ʵ�ߵĳ���
	int mDashGap{ 0 };//���ߵĳ���
	//padding����,��λ������
	int mPadding{ 0 };
	int mPaddingLeft{ 0 };
	int mPaddingTop{ 0 };
	int mPaddingRight{ 0 };
	int mPaddingBottom{ 0 };
	//gradient����
	int mAngle{ 0 };//linear
	float mCenterX{ 0.5f };//ȡֵ��Χ[0.0f,1.0f]
	float mCenterY{ 0.5f };//ֻʵ��linear��centerx��y����centerColor������²�ʹ��
	Color mStartColor{ 0.0f,0.0f,0.0f,1.0f };
	Color mEndColor{ 0.0f,0.0f,0.0f,1.0f };
	Color mCenterColor{ 0.0f,0.0f,0.0f,1.0f };
	float mGradientRadius{ 0.0f };//raida
	GradientType mGradientType{ GradientType::None };

	//����shape���Ը���ͬ�Ŀؼ�ʹ�ã������������ȡֵ��Χ��[0,1.0f]
	float mWidth{ 1.0f };
	float mHeight{ 1.0f };
	
	bool mbInitialized{ false };

	std::shared_ptr<void> mpTexture;
	std::shared_ptr<void> mpMesh;//����shape��mesh
	std::shared_ptr<void> mpStrokeMesh;//���ر߿��mesh

private:
	static void cornerRadiusHandler(const std::shared_ptr<Shape>& shape,const std::string& value);
	static void cornerLeftTopRadiusHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
	static void cornerLeftBottomRadiusHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
	static void cornerRightTopRadiusHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
	static void cornerRightBottomRadiusHandler(const std::shared_ptr<Shape>& shape, const std::string& value);

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
	static void strokeDashWidthHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
	static void strokeDashGapHandler(const std::shared_ptr<Shape>& shape, const std::string& value);
};
#endif