#include "Button.h"
#include "UiRender.h"
#include "core/MeshFilledRect.h"
std::shared_ptr<View> Button::clone() {
	return std::make_shared<Button>(*this);
}
void Button::draw() {
	UiRender::getInstance()->drawTextView(this);
	View::draw();
}

//bool Button::mouseMove(int x, int y) {
//	if (mRect.isInside(x, y)) {
//		mMouseState = MouseState::MouseHover;
//		return true;
//	}
//	return false;
//}
//
//bool Button::mouseLButtonDown(int x, int y) {
//	if (mRect.isInside(x, y)) {
//		mMouseState = MouseState::MouseLButtonDown;
//		mbLButtonDown = true;
//		return true;
//	}
//	return false;
//}
//
//bool Button::mouseLButtonUp(int x, int y) {
//	if (mRect.isInside(x, y)) {
//		mMouseState = MouseState::MouseLButtonUp;
//		if (mbLButtonDown && mClickedListener) {
//			onClicked(this);
//		}
//		return true;
//	}
//	else {
//		mbLButtonDown = false;
//		mMouseState = MouseState::MouseNone;
//		return false;
//	}
//
//}