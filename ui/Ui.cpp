#include "Ui.h"
#include "UiRender.h"

void TextView::draw() {
	UiRender::getInstance()->drawTextView(this);
}

void Button::draw() {
	UiRender::getInstance()->drawButton(this);
}