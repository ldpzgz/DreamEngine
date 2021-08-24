#include "TextView.h"
#include "UiRender.h"

void TextView::draw() {
	UiRender::getInstance()->drawTextView(this);
	View::draw();
}

void TextView::textSizeHandler(const shared_ptr<View>& pv, const string& value) {
	auto ptv = dynamic_pointer_cast<TextView>(pv);
	if (ptv) {
		try {
			auto size = stoi(value);
			ptv->setTextSize(size);
		}
		catch (const logic_error& e) {
			LOGE("error to parse textSize value %s", value.c_str());
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
