#ifndef _BACKGROUND_H_
#define _BACKGROUND_H_
#include "../Texture.h"
#include "../Mesh.h"
#include "Shape.h"
#include <memory>

/*
* 表示view的背景，
* shape表示一个形状，可以是矩形，圆角矩形，椭圆，用一个MeshFilledRect绘制出来
* shape可以有一个边框，这个边框也是用一个MeshFilledRect绘制出来
* shape的位置宽高如何确定：
*	x = rect.x + paddingLeft;
*	y = rect.y + paddingTop;
*	width -= (paddingLeft + paddingRight);  //width是view的宽度，padding是shape的属性
*	height -= (paddingTop+ paddingBottom);  //height是view的高度
* 
* mpMesh有三种填充模式：纹理、渐变色、纯色
*	1 如果mpTexture不为空，则mpMesh使用posDiff material来渲染
*	2 如果shape设置了渐变属性，则mpMesh使用posColor material来渲染
*	3 如果shape设置了纯色属性，则mpMesh使用posUniformColor material来渲染
*/
class Background {
public:
	/*
	* style有三种填充模式1 纹理，2渐变色，3纯色
	* 对于同一种填充模式的style，可以共享同一个shape
	*/
	struct BackgroundStyle {
		std::shared_ptr<Shape> mpShape;
		std::shared_ptr<Texture> mpTex;//如果不为空，就表示用纹理填充，优先级最高
		Color mStartColor;//如果不为空，就表示用渐变色填充，优先级次之
		Color mCenterColor;
		Color mEndColor;
		Color mSolidColor;//如果不为空就表示用纯色填充，最低优先级
		Color mBorderColor;
		unsigned int mColorVbo{0};
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
		void setMyStyle() {
			if (mpShape) {
				mpShape->setTexture(mpTex);
				mpShape->setGradientStartColor(mStartColor);
				mpShape->setGradientCenterColor(mCenterColor);
				mpShape->setGradientEndColor(mEndColor);
				mpShape->setSolidColor(mSolidColor);
				mpShape->setBorderColor(mBorderColor);
			}
		}
		void setShape(const std::shared_ptr<Shape>& pShape) {
			mpShape = pShape;
		}
		void setTexture(const std::shared_ptr<Texture>& pTex) {
			mpTex = pTex;
			if (mpShape) {
				mpShape->setTexture(mpTex);
			}
		}
		std::shared_ptr<Texture>& getTexture() {
			return mpTex;
		}
		void setStartColor(const Color& c) {
			mStartColor = c;
			if (mpShape) {
				mpShape->setGradientStartColor(c);
			}
		}
		void setCenterColor(const Color& c) {
			mCenterColor = c;
			if (mpShape) {
				mpShape->setGradientCenterColor(c);
			}
		}
		void setEndColor(const Color& c) {
			mEndColor = c;
			if (mpShape) {
				mpShape->setGradientEndColor(c);
			}
		}
		void setBorderColor(const Color& c) {
			mBorderColor = c;
			if (mpShape) {
				mpShape->setBorderColor(c);
			}
		}
		void setSolidColor(const Color& c) {
			mSolidColor = c;
			if (mpShape) {
				mpShape->setSolidColor(c);
			}
		}
	};

	Background() {
		mpNormalStyle = make_unique<BackgroundStyle>();
		mpNormalStyle->mpShape = std::make_shared<Shape>();
	}

	std::unique_ptr<BackgroundStyle>& getNormalStyle () {
		return mpNormalStyle;
	}

	std::unique_ptr<BackgroundStyle>& getPushedStyle() {
		return mpPushedStyle;
	}

	std::unique_ptr<BackgroundStyle>& getHoverStyle() {
		return mpHoverStyle;
	}

	std::unique_ptr<BackgroundStyle>& getDisabledStyle() {
		return mpDisabledStyle;
	}

	std::unique_ptr<BackgroundStyle> mpNormalStyle;
	std::unique_ptr<BackgroundStyle> mpPushedStyle;
	std::unique_ptr<BackgroundStyle> mpHoverStyle;
	std::unique_ptr<BackgroundStyle> mpDisabledStyle;
};

#endif