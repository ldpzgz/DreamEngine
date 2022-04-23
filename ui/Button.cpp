#include "Button.h"
#include "UiRender.h"
#include "../MeshFilledRect.h"

void Button::draw() {
	UiRender::getInstance()->drawTextView(this);
	View::draw();
}