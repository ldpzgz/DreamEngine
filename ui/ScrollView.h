#ifndef _SCROLL_VIEW_H_
#define _SCROLL_VIEW_H_
#include "LinearLayout.h"
/*
* 如果ScrollView有很多个子view，
* 继承MoveListener，让子view监听移动，不必遍历每个子view，一个一个move子view。
*/
class ScrollView :public LinearLayout,public MoveListener {
public:
	using LinearLayout::LinearLayout;//继承基类的构造函数
	ScrollView() = default;
	
	void draw() override;

	bool mouseMove(int x, int y) override;

	bool mouseLButtonDown(int x, int y) override;

	bool mouseLButtonUp(int x, int y) override;

	void afterGetWidthHeight() override;
private:
	unsigned int mMouseState{ MouseState::MouseNone };
	bool mbLButtonDown{ false };
	Vec2i mPrePos;
	Vec2i mChildTotalWidthHeight;
};
#endif