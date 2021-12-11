#include "Button.h"
#include "UiRender.h"

void Button::draw() {
	UiRender::getInstance()->drawTextView(this);
	View::draw();
}