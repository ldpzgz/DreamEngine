#include "View.h"
#include "TextView.h"
#include "Button.h"
#include "LinearLayout.h"
#include "ScrollView.h"
#include "../Log.h"
#include "UiRender.h"
#include "UiManager.h"

std::shared_ptr<View> gpViewNothing;

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
	{ "textSize",View::textSizeHandler_s },
	{ "textColor",View::textColorHandler_s },
	{ "text",View::textHandler_s },
	{ "maxWidth",View::maxWidthHandler_s },
	{ "maxHeight",View::maxHeightHandler_s },
	{ "maxLine",View::maxLineHandler_s },
	{ "charSpace",View::charSpaceHandler_s },
	{ "lineSpace",View::lineSpaceHandler_s },
	{ "orientation",View::orientationHandler_s },
};

void View::drawBackground() {
	UiRender::getInstance()->drawBackground(this);
}

void View::initBackground() {
	UiRender::getInstance()->initBackground(this);
}

//只计算MatchParent与固定宽度情况下的宽度
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
//只计算MatchParent与固定高度情况下的高度
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

bool View::calcPos() {
	calcChildPos();
	for (auto& pChild : mChildren) {
		if (pChild) {
			pChild->calcPos();
		}
	}
	return true;
}

bool View::calcWidthHeight(int parentWidth, int parentHeight) {
	//计算pView的宽度高度，以及pView的子view的宽度高度（如果子view是按百分比布局的)
	/*
	* view的宽度和高度有三种设置方式：matchParent、固定值、wrapContent
	从顶向下计算各个view的宽度和高度，不能相互依赖
	1 父view是wrapContent，需要根据子view的尺寸来计算出父view的尺寸
	2 子view是percentWidth，或者matchParent，需要根据父view的尺寸来计算子view的尺寸
	3 不能出现父子相互依赖的情况。
	*/
	//matchParent,或者固定尺寸，可计算出来，并返回true
	//如果是wrapContent，返回false
	bool hasGetWidth = calcWidth(parentWidth);
	bool hasGetHeight = calcHeight(parentHeight);

	int myWidth = mRect.width;
	int myHeight = mRect.height;

	//如果子view是按百分比布局的,帮它们算出宽度
	auto totalWPercent = getChildrenTotalWidthPercent();
	auto totalHPercent = getChildrenTotalHeightPercent();

	//计算子view的宽度，高度
	for (auto& pChild : mChildren) {
		if (totalWPercent > 0) {
			if (!hasGetWidth) {
				//父与子之间不能相互依赖，父是wrapContent，子是百分比布局，这样是不行的
				LOGE("ERROR to calc child's width");
				return false;
			}
			int childWidth = (int)((float)myWidth * (float)pChild->mWidthPercent / (float)totalWPercent);
			pChild->setWidth(childWidth);
		}
		if (totalHPercent > 0) {
			if (!hasGetHeight) {
				//父与子之间不能相互依赖，父是wrapContent，子是百分比布局，这样是不行的
				LOGE("ERROR to calc child's height");
				return false;
			}
			int childHeight = (int)((float)myHeight * (float)pChild->mHeightPercent / (float)totalHPercent);
			pChild->setHeight(childHeight);
		}
		//计算子view的尺寸
		pChild->calcWidthHeight(myWidth, myHeight);
	}

	//wrapContent的情况下，根据子view计算出尺寸
	if (!hasGetWidth) {
		getWidthAccordChildren();
	}
	if (!hasGetHeight) {
		getHeightAccordChildren();
	}
	afterGetWidthHeight();
	//确定了宽高之后，就可以初始化承载背景的shape了
	initBackground();
	return true;
}

//计算自身以及子view的位置尺寸
bool View::calcRect(int parentWidth, int parentHeight) {

	calcWidthHeight(parentWidth, parentHeight);
	calcPos();
	return true;

	////计算parentWidth，parentHeight
	////parentWidth，parentHeight是matchparent,或者固定宽度高度的情况，由下面这两个函数处理
	//if (mWidthPercent == 0) {
	//	calcWidth(parentRect.width);
	//}
	//if (mHeightPercent == 0) {
	//	calcHeight(parentRect.height);
	//}
	////计算子view的宽高
	//if (!mChildren.empty()) {
	//	int percentWidth = 0;
	//	int percentHeight = 0;
	//	for (auto& child : mChildren) {
	//		if (child) {
	//			percentWidth += child->mWidthPercent;
	//			percentHeight += child->mHeightPercent;
	//		}
	//	}
	//	//如果子view是百分比布局，根据子view的百分比、父view的宽度计算出每个ziview的宽度
	//	if (percentWidth > 0) {
	//		for (auto& child : mChildren) {
	//			if (child) {
	//				int childWidth = (int)((float)mRect.width*(float)child->mWidthPercent / (float)percentWidth);
	//				child->setWidth(childWidth);
	//			}
	//		}
	//	}
	//	else {
	//		calcWidth(mRect.width);
	//	}
	//	//同上
	//	if (percentHeight > 0) {
	//		for (auto& child : mChildren) {
	//			if (child) {
	//				int childHeight = (int)((float)mRect.height*(float)child->mHeightPercent / (float)percentHeight);
	//				child->setHeight(childHeight);
	//			}
	//		}
	//	}
	//	else {
	//		calcHeight(mRect.height);
	//	}
	//}
	//return true;
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
					/*auto& pShape = pv->getBackgroundShape();
					if (pShape) {
						UiRender::getInstance()->initBackground(pShape);
					}*/
				}
			}
			else if (value.find(imagePrefix) == 0) {
				string imageName = value.substr(imagePrefix.size());
				auto& pTex = UiManager::getTexture(imageName);
				if (pTex) {
					pv->setBackgroundImg(pTex);
					/*auto& pShape = pv->getBackgroundShape();
					if (pShape) {
						UiRender::getInstance()->initBackground(pShape);
					}*/
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
					//UiRender::getInstance()->initBackground(pShape);
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

void View::orientationHandler_s(const shared_ptr<View>& pView, const string& content) {
	if (pView) {
		pView->orientationHandler(content);
	}
}

void View::textSizeHandler_s(const shared_ptr<View>& pView, const std::string& content) {
	if (pView) {
		pView->textSizeHandler(content);
	}
}
void View::textColorHandler_s(const shared_ptr<View>& pView, const std::string& content) {
	if (pView) {
		pView->textColorHandler(content);
	}
}
void View::textHandler_s(const shared_ptr<View>& pView, const std::string& content) {
	if (pView) {
		pView->textHandler(content);
	}
}
void View::maxWidthHandler_s(const shared_ptr<View>& pView, const std::string& content) {
	if (pView) {
		pView->maxWidthHandler(content);
	}
}
void View::maxHeightHandler_s(const shared_ptr<View>& pView, const std::string& content) {
	if (pView) {
		pView->maxHeightHandler(content);
	}
}
void View::maxLineHandler_s(const shared_ptr<View>& pView, const std::string& content) {
	if (pView) {
		pView->maxLineHandler(content);
	}
}
void View::charSpaceHandler_s(const shared_ptr<View>& pView, const std::string& content) {
	if (pView) {
		pView->charSpaceHandler(content);
	}
}
void View::lineSpaceHandler_s(const shared_ptr<View>& pView, const std::string& content) {
	if (pView) {
		pView->lineSpaceHandler(content);
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
	else if (name == "ScrollView") {
		pView = make_shared<ScrollView>(parent);
	}
	else {
		LOGE("can not create %s view",name.c_str());
	}
	return pView;
}