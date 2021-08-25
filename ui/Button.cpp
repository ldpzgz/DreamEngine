#include "Button.h"
#include "UiRender.h"

void Button::draw() {
	drawBackground();
	//blend
	glEnable(GL_BLEND);
	glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ZERO, GL_ONE);
	glBlendEquation(GL_FUNC_ADD);
	UiRender::getInstance()->drawTextView(this);
	glDisable(GL_BLEND);
	View::draw();
}

bool Button::mouseMove(int x, int y) {
	if(mRect.isInside(x, y)){
		mMouseState |= MouseState::MouseOver;
		//如果鼠标移动过程中，鼠标鼠标左键是按下的状态，就显示按下的状态，否则显示鼠标移动的状态
		/*if ((mMouseState & MouseState::MouseLButtonDown) == 0) {
			Color c = getBackgroundColor();
			c.a = 0.8;
			View::setBackgroundColor(c);
			setDirty(true);
		}*/
		return true;
	}
	/*else {
		if (mMouseState & MouseState::MouseOver) {
			setBackgroundColor(mOriginBkColor);
			setDirty(true);
		}
	}*/
	return false;
}

bool Button::mouseLButtonDown(int x, int y) {
	if (mRect.isInside(x, y)) {
		mMouseState |= MouseState::MouseLButtonDown;
		mbLButtonDown = true;
		Color c = getBackgroundColor();
		c.a = 0.6;
		View::setBackgroundColor(c);
		setDirty(true);
		return true;
	}
	return false;
}

bool Button::mouseLButtonUp(int x, int y) {
	if (mRect.isInside(x, y)) {
		if (mbLButtonDown && mClickedListener) {
			onClicked(this);
		}
		mbLButtonDown = false;
		mMouseState = MouseState::MouseNone;
		setBackgroundColor(mOriginBkColor);
		setDirty(true);
		return true;
	}
	else {
		mbLButtonDown = false;
		mMouseState = MouseState::MouseNone;
		setBackgroundColor(mOriginBkColor);
		setDirty(true);
		return false;
	}
}

void Button::setBackgroundColor(const Color& c) {
	View::setBackgroundColor(c);
	mOriginBkColor = c;
}