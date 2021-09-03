#include "View.h"
#include "TextView.h"
#include "Button.h"
#include "LinearLayout.h"
#include "../Log.h"
#include "UiRender.h"
#include "UiManager.h"

unordered_map<string, int> View::gGravityKeyValue{
	{ "center" ,LayoutParam::Center },
	{ "topCenter" ,LayoutParam::TopCenter },
	{ "leftCenter" ,LayoutParam::LeftCenter },
	{ "bottomCenter" ,LayoutParam::BottomCenter },
	{ "rightCenter" ,LayoutParam::RightCenter },
	{ "leftTop" ,LayoutParam::LeftTop },
	{ "leftBottom" ,LayoutParam::LeftBottom },
	{ "rightBottom" ,LayoutParam::RightBottom },
	{ "rightTop" ,LayoutParam::RightTop }
};

unordered_map < string, std::function<void(const shared_ptr<View>&, const std::string&)>> View::gLayoutAttributeHandler{
	{ "id",View::idHandler },
	{ "width",View::layoutWidthHandler },
	{ "height",View::layoutHeightHandler },
	{ "widthPercent",View::layoutWidthPercentHandler },
	{ "heightPercent",View::layoutHeightPercentHandler },
	{ "marginTop",View::layoutMarginTopHandler },
	{ "marginBottom",View::layoutMarginBottomHandler },
	{ "marginLeft",View::layoutMarginLeftHandler },
	{ "marginRight",View::layoutMarginRightHandler },
	{ "gravity",View::gravityHandler },
	{ "background",View::backgroundHandler },
	{ "textSize",TextView::textSizeHandler },
	{ "textColor",TextView::textColorHandler },
	{ "text",TextView::textHandler },
	{ "maxWidth",TextView::maxWidthHandler },
	{ "maxHeight",TextView::maxHeightHandler },
	{ "maxLine",TextView::maxLineHandler },
	{ "charSpace",TextView::charSpaceHandler },
	{ "lineSpace",TextView::lineSpaceHandler },
	{ "orientation",LinearLayout::orientationHandler },
};

void View::drawBackground() {
	UiRender::getInstance()->drawBackground(this);
}

bool View::calcWidth(int parentWidth) {
	//如果mWidthPercent!=0，是比例布局，已经由父view计算出来了
	if (mWidthPercent != 0) {
		return true;
	}

	if (mLayoutWidth == LayoutParam::WrapContent)
	{
		return false;
	}
	else if (mLayoutWidth == LayoutParam::MatchParent) {
		if (parentWidth > 0) {
			mRect.width = parentWidth - (mLayoutMarginLeft + mLayoutMarginRight);
			return true;
		}
		else {
			return false;
		}
	}
	else if (mLayoutWidth > 0) {
		mRect.width = mLayoutWidth;
		return true;
	}
	return false;
}

bool View::calcHeight(int parentHeight) {
	//如果mHeightPercent != 0，是比例布局，已经由父view计算出来了
	if (mHeightPercent != 0) {
		return true;
	}

	if (mLayoutHeight == LayoutParam::WrapContent)
	{
		return false;
	}
	if (mLayoutHeight == LayoutParam::MatchParent) {
		if (parentHeight > 0) {
			mRect.height = parentHeight - (mLayoutMarginTop + mLayoutMarginBottom);
			return true;
		}
		else {
			return false;
		}
		
	}
	else if (mLayoutHeight > 0) {
		mRect.height = mLayoutHeight;
		return true;
	}
	return false;
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
	//计算parentWidth，parentHeight
	//parentWidth，parentHeight是matchparent,或者固定宽度高度的情况，由下面这两个函数处理
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

void View::idHandler(const shared_ptr<View>& pv, const std::string& value) {
	if (pv && !value.empty()) {
		pv->setId(value);
	}
}

void View::layoutWidthHandler(const shared_ptr<View>& pv, const std::string& value) {
	if (pv) {
		if (value == "wrapContent") {
			pv->mLayoutWidth = LayoutParam::WrapContent;
		}
		else if (value == "matchParent") {
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
		if (value == "wrapContent") {
			pv->mLayoutHeight = LayoutParam::WrapContent;
		}
		else if (value == "matchParent") {
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
		auto it = View::gGravityKeyValue.find(value);
		if (it != View::gGravityKeyValue.end()) {
			pv->mGravity = it->second;
		}
		else {
			LOGD("there are no gravity attribute %s,please check the layout file", value.c_str());
		}
	}
}

void View::backgroundHandler(const shared_ptr<View>& pv, const std::string& value) {
	if (pv) {
		if (!value.empty())
		{
			const string colorPrefix = "@color/";
			const string imagePrefix = "@image/";
			const string shapePrefix = "@shape/";
			if (value[0] == '#' || value.find(colorPrefix)==0) {
				Color c;
				if (!Color::parseColor(value, c)) {
					LOGE("error to parse backgoundColor who's value is %s in layout file", value.c_str());
				}
				else {
					pv->setBackgroundColor(c);
					auto& pShape = pv->getBackgroundShape();
					if (pShape) {
						UiRender::getInstance()->initShape(pShape);
					}
				}
			}
			else if (value.find(imagePrefix) == 0) {
				string imageName = value.substr(imagePrefix.size());
				auto& pTex = UiManager::getTexture(imageName);
				if (pTex) {
					pv->setBackgroundImg(static_pointer_cast<void>(pTex));
					auto& pShape = pv->getBackgroundShape();
					if (pShape) {
						UiRender::getInstance()->initShape(pShape);
					}
				}
				else {
					LOGE("ERROR not found %s image in layout file",imageName.c_str());
				}
			}
			else if (value.find(shapePrefix) == 0) {
				string shapeName = value.substr(shapePrefix.size());
				auto& pShape = UiManager::getShape(shapeName);
				if (pShape) {
					pv->setBackgroundShape(pShape);
					UiRender::getInstance()->initShape(pShape);
				}
				else {
					LOGE("ERROR not found %s shape in layout file", shapeName.c_str());
				}
			}
			else {
				LOGE("error to parse backgound value %s in layout file", value.c_str());
			}
		}
		else {
			LOGE("error to parse layout the backgound value is null");
		}
		
	}
}

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