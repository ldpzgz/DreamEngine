#include "TextView.h"
#include "UiRender.h"

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

void TextView::textSizeHandler(const shared_ptr<View>& pv, const string& value) {
	auto ptv = dynamic_pointer_cast<TextView>(pv);
	if (ptv) {
		try {
			auto size = stoi(value);
			ptv->setTextSize(size);
		}
		catch (const logic_error& e) {
			LOGE("error to parse textView prop textSize value %s", value.c_str());
		}
	}
}

void TextView::maxWidthHandler(const shared_ptr<View>& pv, const std::string& value) {
	auto ptv = dynamic_pointer_cast<TextView>(pv);
	if (ptv) {
		try {
			auto size = stoi(value);
			ptv->setMaxWidth(size);
		}
		catch (const logic_error& e) {
			LOGE("error to parse textView prop maxWidth value %s", value.c_str());
		}
	}
}

void TextView::maxHeightHandler(const shared_ptr<View>& pv, const std::string& value) {
	auto ptv = dynamic_pointer_cast<TextView>(pv);
	if (ptv) {
		try {
			auto size = stoi(value);
			ptv->setMaxHeight(size);
		}
		catch (const logic_error& e) {
			LOGE("error to parse textView prop maxHeight value %s", value.c_str());
		}
	}
}

void TextView::maxLineHandler(const shared_ptr<View>& pv, const std::string& value) {
	auto ptv = dynamic_pointer_cast<TextView>(pv);
	if (ptv) {
		try {
			auto size = stoi(value);
			ptv->setMaxLines(size);
		}
		catch (const logic_error& e) {
			LOGE("error to parse textView prop maxLine value %s", value.c_str());
		}
	}
}

void TextView::charSpaceHandler(const shared_ptr<View>& pv, const std::string& value){
	auto ptv = dynamic_pointer_cast<TextView>(pv);
	if (ptv) {
		try {
			auto size = stoi(value);
			ptv->setCharSpacingInc(size);
		}
		catch (const logic_error& e) {
			LOGE("error to parse textView prop charSpace value %s", value.c_str());
		}
	}
}
void TextView::lineSpaceHandler(const shared_ptr<View>& pv, const std::string& value) {
	auto ptv = dynamic_pointer_cast<TextView>(pv);
	if (ptv) {
		try {
			auto size = stoi(value);
			ptv->setLineSpacingInc(size);
		}
		catch (const logic_error& e) {
			LOGE("error to parse textView prop lineSpace value %s", value.c_str());
		}
	}
}

void TextView::textColorHandler(const shared_ptr<View>& pv, const string& value) {
	auto ptv = dynamic_pointer_cast<TextView>(pv);
	if (ptv) {
		if (!Color::parseColor(value, ptv->getTextColor())) {
			LOGE("error to parse TextView prop textColor who's value is %s",value.c_str());
		}
	}
}

void TextView::textHandler(const shared_ptr<View>& pv, const string& value) {
	auto ptv = dynamic_pointer_cast<TextView>(pv);
	if (ptv) {
		ptv->setText(value);
	}
}

void TextView::calcTextPosition() {
	UiRender::getInstance()->calcTextPosition(this);
}