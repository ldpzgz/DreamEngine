#include "Button.h"
#include "UiRender.h"
#include "../MeshFilledRect.h"
std::shared_ptr<View> Button::clone() {
	return std::make_shared<Button>(*this);
}
void Button::draw() {
	UiRender::getInstance()->drawTextView(this);
	View::draw();
}