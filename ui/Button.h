#ifndef _BUTTON_H_
#define _BUTTON_H_
#include "TextView.h"

class UiRender;
class Button : public TextView{
public:
	friend UiRender;
	explicit Button(shared_ptr<View> parent) :
		TextView(parent) {
		//button默认是白底黑字
		setBackgroundColor(Color(1.0f,1.0f,1.0f,1.0f));
		setTextColor(Color(0.0f, 0.0f, 0.0f, 1.0f));
	}

	void draw() override;

	bool mouseMove(int x, int y) override;

	bool mouseLButtonDown(int x, int y) override;

	bool mouseLButtonUp(int x, int y) override;

	void setBackgroundColor(const Color& c) override;
private:
	unsigned int mMouseState{ MouseState::MouseNone};
	bool mbLButtonDown{ false };
	Color mOriginBkColor;
};

#endif