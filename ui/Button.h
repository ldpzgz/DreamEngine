#ifndef _BUTTON_H_
#define _BUTTON_H_
#include "TextView.h"

class UiRender;
class Button : public TextView{
public:
	friend UiRender;
	using TextView::TextView;//继承基类的构造函数

	/*
	设置button的背景颜色
	*/
	void setBackgroundColor(const Color& color) {
		for (auto& mc : mColor) {
			mc = color;
		}
	}
	/*
	设置button的背景颜色，从左到右渐变
	*/
	void setBackgroundColor(const Color& colorLeft, const Color& colorRight) {
		mColor[0] = colorLeft;
		mColor[1] = colorLeft;
		mColor[2] = colorRight;
		mColor[3] = colorRight;
	}

	void draw() override;

	bool mouseMove(int x, int y) override;

	bool mouseLButtonDown(int x, int y) override;

	bool mouseLButtonUp(int x, int y) override;
private:
	Color mColor[4]{ {1.0f,1.0f,1.0f,1.0f},{ 1.0f,1.0f,1.0f,1.0f },{ 1.0f,1.0f,1.0f,1.0f },{ 1.0f,1.0f,1.0f,1.0f } };
	unsigned int mMouseState{ MouseState::MouseNone};
	bool mbLButtonDown{ false };
};

#endif