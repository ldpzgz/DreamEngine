#include "Ui.h"
#include "UiRender.h"

void TextView::draw() {
	UiRender::getInstance()->drawTextView(this);
}

void Button::draw() {
	UiRender::getInstance()->drawButton(this);
}

bool Button::mouseMove(int x, int y) {
	if(mRect.isInside(x, y)){
		mMouseState = MouseState::MouseOver;
		return true;
	}
	return false;
}

bool Button::mouseLButtonDown(int x, int y) {
	if (mRect.isInside(x, y)) {
		mMouseState = MouseState::MouseLButtonDown;
		mbLButtonDown = true;
		return true;
	}
	return false;
}

bool Button::mouseLButtonUp(int x, int y) {
	if (mRect.isInside(x, y)) {
		mMouseState = MouseState::MouseLButtonUp;
		if (mbLButtonDown && mClickedListener) {
			onClicked(this);
		}
		return true;
	}
	else {
		mbLButtonDown = false;
		mMouseState = MouseState::MouseNone;
		return false;
	}
	
}

int LinearLayout::getTotalWidthPercent() {
	if (!mChildren.empty()) {
		for (auto& child : mChildren) {
			if (child) {
				mTotalWidthPercent += child->mWidthPercent;
			}
		}
	}
	return mTotalWidthPercent;
}

int LinearLayout::getTotalHeightPercent() {
	if (!mChildren.empty()) {
		for (auto& child : mChildren) {
			if (child) {
				mTotalHeightPercent += child->mHeightPercent;
			}
		}
	}
	return mTotalHeightPercent;
}

void LinearLayout::calcChildPos() {
	int x = mRect.x;
	int y = mRect.y;
	int totalWidth = 0;
	int totalHeight = 0;
	
	for (auto& pChild : mChildren) {
		if (mOrientation == LayoutParam::Horizontal) {
			totalWidth += pChild->advanceX();
		}
		else if (mOrientation == LayoutParam::Vertical) {
			totalHeight += pChild->advanceY();
		}
	}

	if (mOrientation == LayoutParam::Horizontal) {
		if (mGravity == LayoutParam::LeftTop) {
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->calcX(x);
					pChild->calcY(y);
					x += pChild->advanceX();
				}
			}
		}
		else if (mGravity == LayoutParam::HCenter) {
			x = mRect.x + (mRect.width - totalWidth) / 2;
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->calcX(x);
					pChild->calcY(y);
					x += pChild->advanceX();
				}
			}
		}
		else if (mGravity == LayoutParam::VCenter) {
			y = mRect.y + mRect.height / 2;
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->calcX(x);
					pChild->setCenterY(y);
					x += pChild->advanceX();
				}
			}
		}
		else if (mGravity = LayoutParam::Center) {
			x = mRect.x + (mRect.width - totalWidth) / 2;
			y = mRect.y + mRect.height / 2;

			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->calcX(x);
					pChild->setCenterY(y);
					x += pChild->advanceX();
				}
			}
		}
	}
	if (mOrientation == LayoutParam::Vertical) {
		if (mGravity == LayoutParam::LeftTop) {
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->calcX(x);
					pChild->calcY(y);
					y += pChild->advanceY();
				}
			}
		}
		else if (mGravity == LayoutParam::HCenter) {
			x = mRect.x + mRect.width/ 2;
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->setCenterX(x);
					pChild->calcY(y);
					y += pChild->advanceY();
				}
			}
		}
		else if (mGravity == LayoutParam::VCenter) {
			y = mRect.y + (mRect.height-totalHeight) / 2;
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->calcX(x);
					pChild->calcY(y);
					y += pChild->advanceY();
				}
			}
		}
		else if (mGravity = LayoutParam::Center) {
			x = mRect.x + mRect.width / 2;
			y = mRect.y + (mRect.height - totalHeight) / 2;

			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->setCenterX(x);
					pChild->calcY(y);
					y += pChild->advanceY();
				}
			}
		}
	}
}

void LinearLayout::draw() {
	UiRender::getInstance()->drawLinearLayout(this);
}

void View::calcWidth(int width) {
	//如果mLayoutWidth==0，肯定是比例布局，由父view来确定尺寸
	if (mLayoutWidth == LayoutParam::MatchParent) {
		mRect.width = width - (mLayoutMarginLeft + mLayoutMarginRight);
	}
	else if (mLayoutWidth > 0) {
		mRect.width = mLayoutWidth;
	}
}

void View::calcHeight(int height) {
	//如果mLayoutHeight==0，肯定是比例布局，由父view来确定尺寸
	if (mLayoutHeight == LayoutParam::MatchParent) {
		mRect.height = height - (mLayoutMarginTop + mLayoutMarginBottom);
	}
	else if (mLayoutHeight > 0) {
		mRect.height = mLayoutHeight;
	}
}

//按比例布局，宽高由父view来确定
void View::setWidth(int width) {
	mRect.width = width - (mLayoutMarginLeft + mLayoutMarginRight);
}
//按比例布局，宽高由父view来确定
void View::setHeight(int height) {
	mRect.height = height - (mLayoutMarginTop + mLayoutMarginBottom);
}

//计算自身以及子view的位置尺寸
bool View::calcRect(const Rect<int>& parentRect) {
	//计算width，height
	//width，height是matchparent,或者固定宽度高度的情况，由下面这两个函数处理
	if (mWidthPercent == 0) {
		calcWidth(parentRect.width);
	}
	if (mHeightPercent == 0) {
		calcHeight(parentRect.height);
	}
	//计算子view的宽高
	if (!mChildren.empty()) {
		int percentWidth = 0;
		int percentHeight = 0;
		for (auto& child : mChildren) {
			if (child) {
				percentWidth += child->mWidthPercent;
				percentHeight += child->mHeightPercent;
			}
		}
		if (percentWidth > 0) {
			for (auto& child : mChildren) {
				if (child) {
					int childWidth = (int)((float)mRect.width*(float)child->mWidthPercent / (float)percentWidth);
					child->setWidth(childWidth);
				}
			}
		}
		else {
			calcWidth(mRect.width);
		}

		if (percentHeight > 0) {
			for (auto& child : mChildren) {
				if (child) {
					int childHeight = (int)((float)mRect.height*(float)child->mHeightPercent / (float)percentHeight);
					child->setHeight(childHeight);
				}
			}
		}
		else {
			calcHeight(mRect.height);
		}
	}
	return true;
}

void View::layoutWidthHandler(const shared_ptr<View>& pv, const std::string& value) {
	if (pv) {
		if (value == "wrap_content") {
			pv->mLayoutWidth = LayoutParam::WrapContent;
		}
		else if (value == "match_parent") {
			pv->mLayoutWidth = LayoutParam::MatchParent;
		}
		else {
			try {
				pv->mLayoutWidth = stoi(value);
			}
			catch (const logic_error& e) {
				LOGE("error to parse layoutWidth value %s", value.c_str());
			}
		}
	}
}

void View::layoutWidthPercentHandler(const shared_ptr<View>& pv, const std::string& value) {
	if (pv) {
		try {
			pv->mWidthPercent = stoi(value);
		}
		catch (const logic_error& e) {
			LOGE("error to parse layoutWidthPercent value %s", value.c_str());
		}
	}
}

void View::layoutHeightHandler(const shared_ptr<View>& pv, const std::string& value) {
	if (pv) {
		if (value == "wrap_content") {
			pv->mLayoutHeight = LayoutParam::WrapContent;
		}
		else if (value == "match_parent") {
			pv->mLayoutHeight = LayoutParam::MatchParent;
		}
		else {
			try {
				pv->mLayoutHeight = stoi(value);
			}
			catch (const logic_error& e) {
				LOGE("error to parse layoutHeight value %s", value.c_str());
			}
		}
	}
}

void View::layoutHeightPercentHandler(const shared_ptr<View>& pv, const std::string& value) {
	if (pv) {
		try {
			pv->mHeightPercent = stoi(value);
		}
		catch (const logic_error& e) {
			LOGE("error to parse layoutHeightPercent value %s", value.c_str());
		}
	}
}
void View::layoutMarginTopHandler(const shared_ptr<View>& pv, const std::string& value) {
	if (pv) {
		try {
			pv->mLayoutMarginTop = stoi(value);
		}
		catch (const logic_error& e) {
			LOGE("error to parse layoutMarginTop value %s", value.c_str());
		}
	}
}
void View::layoutMarginBottomHandler(const shared_ptr<View>& pv, const std::string& value) {
	if (pv) {
		try {
			pv->mLayoutMarginBottom = stoi(value);
		}
		catch (const logic_error& e) {
			LOGE("error to parse layoutMarginBottom value %s", value.c_str());
		}
	}
}
void View::layoutMarginLeftHandler(const shared_ptr<View>& pv, const std::string& value) {
	if (pv) {
		try {
			pv->mLayoutMarginLeft = stoi(value);
		}
		catch (const logic_error& e) {
			LOGE("error to parse layoutMarginLeft value %s", value.c_str());
		}
	}
}
void View::layoutMarginRightHandler(const shared_ptr<View>& pv, const std::string& value) {
	if (pv) {
		try {
			pv->mLayoutMarginRight = stoi(value);
		}
		catch (const logic_error& e) {
			LOGE("error to parse layoutMarginRight value %s", value.c_str());
		}
	}
}

void View::gravityHandler(const shared_ptr<View>& pv, const std::string& value) {
	if (pv) {
		if (value == "center") {
			pv->mGravity = LayoutParam::Center;
		}
		else if (value == "hCenter") {
			pv->mGravity = LayoutParam::HCenter;
		}
		else if (value == "vCenter") {
			pv->mGravity = LayoutParam::VCenter;
		}
	}
}

void LinearLayout::orientationHandler(const shared_ptr<View>& pv, const string& value) {
	auto pl = dynamic_pointer_cast<LinearLayout>(pv);
	if (pl) {
		if (value == "horizontal") {
			pl->mOrientation = LayoutParam::Horizontal;
		}
		else if (value == "vertical") {
			pl->mOrientation = LayoutParam::Vertical;
		}
		else {
			LOGE("can not recognize orientation value %s",value.c_str());
		}
	}
}

unordered_map < string, std::function<void(const shared_ptr<View>&, const std::string&)>> View::gAttributeHandler{
	{ "width",View::layoutWidthHandler },
	{ "height",View::layoutHeightHandler },
	{ "widthPercent",View::layoutWidthPercentHandler },
	{ "heightPercent",View::layoutHeightPercentHandler },
	{ "marginTop",View::layoutMarginTopHandler },
	{ "marginBottom",View::layoutMarginBottomHandler },
	{ "marginLeft",View::layoutMarginLeftHandler },
	{ "marginRight",View::layoutMarginRightHandler },
	{ "gravity",View::gravityHandler },
	{ "orientation",LinearLayout::orientationHandler },
};

shared_ptr<View> View::createView(const string& name, shared_ptr<View> parent) {
	shared_ptr<View> pView;
	if (name == "LinearLayout") {
		pView = make_shared<LinearLayout>(parent);
	}
	else if (name == "TextView") {
		pView = make_shared<TextView>(parent);
	}
	else if (name == "Button") {
		pView = make_shared<Button>(parent);
	}
	else {
		LOGE("can not create %s view",name.c_str());
	}
	return pView;
}