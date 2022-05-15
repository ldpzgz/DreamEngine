#pragma once

#include "TextView.h"

class UiRender;
class Button : public TextView{
public:
	friend UiRender;
	explicit Button(shared_ptr<View> parent) :
		TextView(parent) {
		//button默认是白底黑字
		setBackgroundColor(Color(1.0f,1.0f,1.0f,1.0f));
		setTextColor(Color(0.0f, 0.0f, 0.0f, 1.0f));
	}

	Button(const Button&) = default;
	std::shared_ptr<View> clone() override;
	void draw() override;
};