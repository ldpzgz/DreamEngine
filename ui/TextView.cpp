#include "TextView.h"
#include "UiRender.h"
#include <stdexcept>
#include "core/MeshFilledRect.h"
#include "core/Log.h"

std::shared_ptr<View> TextView::clone() {
	return std::make_shared<TextView>(*this);
}

void TextView::draw() {
	UiRender::getInstance()->drawTextView(this);
	View::draw();
}

void TextView::setText(const std::string& str) {
	if (mText != str) {
		mText = str;
		setDirty(true);
		setUpdateTextPosition(true);
		/*
		* 如果这个textview是wrapContent的，那么重新设置了text，它的宽高
		* 都会发生变化，这就导致了背景mesh也会发生变化，
		* 如果父view的宽高是wrapcontent的，那么父view的尺寸也会发生变化
		*/
		if (mLayoutWidth == LayoutParam::WrapContent || 
			mLayoutHeight == LayoutParam::WrapContent) {
			auto pParent = mpParent.lock();
			int parentWidth = 0;
			int parentHeight = 0;
			bool traceToTopView{ false };
			if (!pParent) {
				traceToTopView = true;
				pParent = shared_from_this();
			}
			while (pParent) {
				if (pParent->getLayoutHeight() == LayoutParam::WrapContent ||
					pParent->getLayoutWidth() == LayoutParam::WrapContent) {
					auto pp = pParent->getParent().lock();
					if (pp) {
						pParent = pp;
					}
					else {
						traceToTopView = true;
						break;
					}
				}
				else {
					auto ppp = pParent->getParent().lock();
					if (ppp) {
						auto& rect = ppp->getRect();
						parentWidth = rect.width;
						parentHeight = rect.height;
						break;
					}
					else {
						traceToTopView = true;
						break;
					}
				}
			}
			if (traceToTopView) {
				auto anyW = pParent->getAny("parentWidth");
				auto anyH = pParent->getAny("parentHeight");
				if (anyW.has_value() && anyH.has_value()) {
					try {
						parentWidth = std::any_cast<int>(anyW);
						parentHeight = std::any_cast<int>(anyH);
					}
					catch (std::bad_any_cast e) {
						LOGE("TextView::setText any cast error");
						return;
					}
				}
				else {
					LOGE("TextView::setText the top parent has no parent wh");
					return;
				}
			}
			pParent->calcRect(parentWidth, parentHeight);
		}
	}
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