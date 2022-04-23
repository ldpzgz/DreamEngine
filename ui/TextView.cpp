#include "TextView.h"
#include "UiRender.h"
#include <stdexcept>
#include "../MeshFilledRect.h"
#include "../Log.h"

void TextView::draw() {
	UiRender::getInstance()->drawTextView(this);
	View::draw();
}

bool TextView::calcWidth(int parentWidth) {
	if (mLayoutWidth == LayoutParam::WrapContent)
	{
		UiRender::getInstance()->calcTextViewWidthHeight(this);
		return true;
	}
	else {
		return View::calcWidth(parentWidth);
	}
	
}
bool TextView::calcHeight(int parentHeight) {
	if (mLayoutHeight == LayoutParam::WrapContent)
	{
		UiRender::getInstance()->calcTextViewWidthHeight(this);
		return true;
	}
	else {
		return View::calcHeight(parentHeight);
	}
}

void TextView::textSizeHandler(const string& value) {
	try {
		auto size = stoi(value);
		setTextSize(size);
	}
	catch (const logic_error& e) {
		LOGE("error to parse textView prop textSize value %s,%s", value.c_str(),e.what());
	}
}

void TextView::maxWidthHandler(const std::string& value) {
	try {
		auto size = stoi(value);
		setMaxWidth(size);
	}
	catch (const logic_error& e) {
		LOGE("error to parse textView prop maxWidth value %s,%s", value.c_str(),e.what());
	}
}

void TextView::maxHeightHandler(const std::string& value) {
	try {
		auto size = stoi(value);
		setMaxHeight(size);
	}
	catch (const logic_error& e) {
		LOGE("error to parse textView prop maxHeight value %s,%s", value.c_str(),e.what());
	}
}

void TextView::maxLineHandler(const std::string& value) {
	try {
		auto size = stoi(value);
		setMaxLines(size);
	}
	catch (const logic_error& e) {
		LOGE("error to parse textView prop maxLine value %s,%s", value.c_str(),e.what());
	}
}

void TextView::charSpaceHandler(const std::string& value){
	try {
		auto size = stoi(value);
		setCharSpacingInc(size);
	}
	catch (const logic_error& e) {
		LOGE("error to parse textView prop charSpace value %s,%s", value.c_str(),e.what());
	}
}
void TextView::lineSpaceHandler(const std::string& value) {
	try {
		auto size = stoi(value);
		setLineSpacingInc(size);
	}
	catch (const logic_error& e) {
		LOGE("error to parse textView prop lineSpace value %s,%s", value.c_str(),e.what());
	}
}

void TextView::textColorHandler(const string& value) {
	if (!Color::parseColor(value, getTextColor())) {
		LOGE("error to parse TextView prop textColor who's value is %s",value.c_str());
	}
}

void TextView::textHandler(const string& value) {
	setText(value);
}

void TextView::calcTextPosition() {
	UiRender::getInstance()->calcTextPosition(this);
}