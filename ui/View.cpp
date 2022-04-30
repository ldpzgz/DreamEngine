#include "View.h"
#include "TextView.h"
#include "Button.h"
#include "LinearLayout.h"
#include "ScrollView.h"
#include "ListView.h"
#include "TreeView.h"

#include "UiRender.h"
#include "UiManager.h"
#include "Background.h"
#include "BackgroundStyle.h"
#include "Shape.h"
#include "../Texture.h"
#include "../Log.h"
#include "../MeshFilledRect.h"
#include <string_view>
#include <stdexcept>

using namespace std;

std::vector<std::string_view> gViewAnyIndex{
	"treeNodeIndentation"
};

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

//下面这2个成员没有clone，父子关系
//weak_ptr<View> mpParent;
//std::list<std::shared_ptr<View>> mChildren;
//std::unordered_map<std::string, std::shared_ptr<View>> mpId2ViewMap

View::View(const View& v):
	mbIsDirty(v.mbIsDirty),
	mbEnabled(v.mbEnabled),
	mMouseState(v.mMouseState),
	mId(v.mId),
	mClickedListener(v.mClickedListener),
	mLayoutWidth(v.mLayoutWidth),
	mLayoutHeight(v.mLayoutHeight),
	mLayoutMarginTop(v.mLayoutMarginTop),
	mLayoutMarginBottom(v.mLayoutMarginBottom),
	mLayoutMarginLeft(v.mLayoutMarginLeft),
	mLayoutMarginRight(v.mLayoutMarginRight),
	mWidthPercent(v.mWidthPercent),
	mHeightPercent(v.mHeightPercent),
	mGravity(v.mGravity),
	mRect(v.mRect),
	mMoveVector(v.mMoveVector),
	mpDirtyListener(v.mpDirtyListener),
	mpMoveListener(v.mpMoveListener),
	mpBackground(v.mpBackground),
	mAnyMap(v.mAnyMap)
{
	/*if (v.mpId2ViewMap) {
		mpId2ViewMap = std::make_unique<std::unordered_map<std::string, std::shared_ptr<View>>>(*v.mpId2ViewMap);
	}*/
}

void View::setAny(ViewAnyIndex index, const std::any& any) {
	auto& i = gViewAnyIndex[static_cast<size_t>(index)];
	mAnyMap[i] = any;
}

std::any View::getAny(ViewAnyIndex index) {
	auto& i = gViewAnyIndex[static_cast<size_t>(index)];
	return getAny(i);
}

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
			int childWidth = (int)((float)myWidth * (float)pChild->getWidthPercent() / (float)totalWPercent);
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
}

void View::idHandler(const shared_ptr<View>& pv, const std::string& value) {
	if (pv && !value.empty()) {
		pv->setId(value);
	}
}

void View::layoutWidthHandler(const shared_ptr<View>& pv, const std::string& value) {
	if (pv) {
		if (value == "wrapContent"sv) {
			pv->mLayoutWidth = LayoutParam::WrapContent;
		}
		else if (value == "matchParent"sv) {
			pv->mLayoutWidth = LayoutParam::MatchParent;
		}
		else {
			try {
				pv->mLayoutWidth = stoi(value);
			}
			catch (const logic_error& e) {
				LOGE("error to parse layoutWidth value %s,error:%s", value.c_str(),e.what());
			}
		}
	}
}

void View::layoutWidthPercentHandler(const shared_ptr<View>& pv, const std::string& value) {
	if (pv) {
		try {
			pv->getWidthPercent() = stoi(value);
		}
		catch (const logic_error& e) {
			LOGE("error to parse layoutWidthPercent value %s,error:%s", value.c_str(),e.what());
		}
	}
}

void View::layoutHeightHandler(const shared_ptr<View>& pv, const std::string& value) {
	if (pv) {
		if (value == "wrapContent"sv) {
			pv->mLayoutHeight = LayoutParam::WrapContent;
		}
		else if (value == "matchParent"sv) {
			pv->mLayoutHeight = LayoutParam::MatchParent;
		}
		else {
			try {
				pv->mLayoutHeight = stoi(value);
			}
			catch (const logic_error& e) {
				LOGE("error to parse layoutHeight value %s,error:%s", value.c_str(),e.what());
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
			LOGE("error to parse layoutHeightPercent value %s,error:%s", value.c_str(),e.what());
		}
	}
}

void View::layoutMarginTopHandler(const shared_ptr<View>& pv, const std::string& value) {
	if (pv) {
		try {
			pv->mLayoutMarginTop = stoi(value);
		}
		catch (const logic_error& e) {
			LOGE("error to parse layoutMarginTop value %s,error:%s", value.c_str(),e.what());
		}
	}
}
void View::layoutMarginBottomHandler(const shared_ptr<View>& pv, const std::string& value) {
	if (pv) {
		try {
			pv->mLayoutMarginBottom = stoi(value);
		}
		catch (const logic_error& e) {
			LOGE("error to parse layoutMarginBottom value %s,error:%s", value.c_str(),e.what());
		}
	}
}
void View::layoutMarginLeftHandler(const shared_ptr<View>& pv, const std::string& value) {
	if (pv) {
		try {
			pv->mLayoutMarginLeft = stoi(value);
		}
		catch (const logic_error& e) {
			LOGE("error to parse layoutMarginLeft value %s,error:%s", value.c_str(),e.what());
		}
	}
}
void View::layoutMarginRightHandler(const shared_ptr<View>& pv, const std::string& value) {
	if (pv) {
		try {
			pv->mLayoutMarginRight = stoi(value);
		}
		catch (const logic_error& e) {
			LOGE("error to parse layoutMarginRight value %s,error:%s", value.c_str(),e.what());
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
			string_view colorPrefix{ "@color/" };
			string_view imagePrefix{ "@image/" };
			string_view shapePrefix{ "@shape/" };
			string_view backgroundPrefix{ "@bk/" };
			if (value[0] == '#' || value.find(colorPrefix)==0) {
				Color c;
				if (!Color::parseColor(value, c)) {
					LOGE("error to parse backgoundColor who's value is %s in layout file", value.c_str());
				}
				else {
					pv->setBackgroundColor(c);
				}
			}
			else if (value.find(imagePrefix) == 0) {
				string imageName = value.substr(imagePrefix.size());
				auto pTex = UiManager::getTexture(imageName);
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
			else if (value.find(backgroundPrefix) == 0) {
				string bkName = value.substr(backgroundPrefix.size());
				auto& pBack = UiManager::getBackground(bkName);
				if (pBack) {
					pv->setBackground(pBack);
					//UiRender::getInstance()->initBackground(pShape);
				}
				else {
					LOGE("ERROR not found %s background in layout file", bkName.c_str());
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

void View::setBackgroundShape(const shared_ptr<Shape>& pShape) {
	if (!mpBackground) {
		mpBackground = make_shared<Background>();
	}
	auto& pStyle = mpBackground->getNormalStyle();
	pStyle->setShape(pShape);
	auto& pTex = pShape->getTexture();
	if (pTex) {
		pStyle->setTexture(pShape->getTexture());
		pStyle->setFillType(FillType::Fill_Texture);
	}
	else if (pShape->getGradientType() == GradientType::Linear) {
		pStyle->setStartColor(pShape->getGradientStartColor());
		pStyle->setCenterColor(pShape->getGradientCenterColor());
		pStyle->setEndColor(pShape->getGradientEndColor());
		pStyle->setFillType(FillType::Fill_Gradient);
	}
	else {
		pStyle->setSolidColor(pShape->getSolidColor());
		pStyle->setFillType(FillType::Fill_Solid);
	}
	
	pStyle->setBorderColor(pShape->getBorderColor());
	//setDirty(true);
}

void View::setBackgroundImg(shared_ptr<Texture>& pTex) {
	if (!mpBackground) {
		mpBackground = make_shared<Background>();
	}
	auto& pStyle = mpBackground->getNormalStyle();
	pStyle->setTexture(pTex);
	pStyle->setFillType(FillType::Fill_Texture);
	//setDirty(true);
}

void View::setBackgroundColor(const Color& c) {
	if (!mpBackground) {
		mpBackground = make_shared<Background>();
	}
	auto& pStyle = mpBackground->getNormalStyle();
	if (pStyle) {
		pStyle->setSolidColor(c);
		pStyle->setFillType(FillType::Fill_Solid);
	}
	//setDirty(true);
}

bool View::mouseMove(int x, int y, bool notInside) {
	auto tempRect = mRect;
	tempRect.x += mMoveVector.x;
	tempRect.y += mMoveVector.y;
	if (!notInside && tempRect.isInside(x, y)) {
		for (auto& pChild : mChildren) {
			if (pChild->mouseMove(x, y, false)) {
				break;
			}
		}
		if ((mMouseState & MouseState::MouseHover) == 0) {
			mMouseState |= MouseState::MouseHover;
			if (mpBackground && mpBackground->getHoverStyle()) {
				setDirty(true);
			}
		}
		return true;
	}
	else {
		if (mMouseState & MouseState::MouseHover) {
			for (auto& pChild : mChildren) {
				pChild->mouseMove(x, y, true);
			}
			mMouseState &= ~MouseState::MouseHover;
			if (mpBackground && mpBackground->getHoverStyle()) {
				setDirty(true);
			}
		}
	}
	return false;
};

bool View::mouseLButtonDown(int x, int y, bool notInside) {
	auto tempRect = mRect;
	tempRect.x += mMoveVector.x;
	tempRect.y += mMoveVector.y;
	if (!notInside && tempRect.isInside(x, y)) {
		for (auto& pChild : mChildren) {
			if (pChild->mouseLButtonDown(x, y, false)) {
				break;
			}
		}
		mMouseState |= MouseState::MouseLButtonDown;
		if (mpBackground && mpBackground->getPushedStyle()) {
			setDirty(true);
		}
		return true;
	}
	//else {
	//	for (auto& pChild : mChildren) {
	//		pChild->mouseLButtonDown(x, y, true);
	//	}
	//}
	return false;
};

bool View::mouseLButtonUp(int x, int y, bool notInside) {
	auto tempRect = mRect;
	tempRect.x += mMoveVector.x;
	tempRect.y += mMoveVector.y;
	if (!notInside && tempRect.isInside(x, y)) {
		for (auto& pChild : mChildren) {
			if (pChild->mouseLButtonUp(x, y, false)) {
				break;
			}
		}
		if ((mMouseState & MouseState::MouseLButtonDown) && mClickedListener) {
			onClicked(this);
		}
		mMouseState &= ~MouseState::MouseLButtonDown;
		if (mpBackground && mpBackground->getPushedStyle()) {
			setDirty(true);
		}
		return true;
	}
	else {
		if (mMouseState != MouseState::MouseNone) {
			for (auto& pChild : mChildren) {
				pChild->mouseLButtonUp(x, y, true);
			}
			if (mpBackground && mpBackground->getPushedStyle()) {
				setDirty(true);
			}
		}
		mMouseState = MouseState::MouseNone;
		return false;
	}
};

shared_ptr<View> View::findViewById(const std::string& id) {
	if (mpId2ViewMap && !id.empty()) {
		auto it = mpId2ViewMap->find(id);
		if (it != mpId2ViewMap->end()) {
			return it->second;
		}
		else {
			return {};
		}
	}
	else {
		return {};
	}
}

std::shared_ptr<View> View::clone(const std::shared_ptr<View>& parent) {
	auto pClone = clone();
	if (pClone) {
		pClone->setParent(parent);
		std::unique_ptr< std::unordered_map< std::string, std::shared_ptr<View> > > p;
		pClone->getId2View(p);
	}
	return pClone;
}

void View::getId2View(std::unique_ptr< std::unordered_map< std::string, std::shared_ptr<View> > >& pId2ViewMap) {
	if (pId2ViewMap) {
		if (!mId.empty()) {
			pId2ViewMap->emplace(mId, shared_from_this());
		}
		for (auto& pchild : mChildren) {
			if (pchild) {
				pchild->getId2View(pId2ViewMap);
			}
		}
	}
	else {
		mpId2ViewMap = make_unique< std::unordered_map<std::string, std::shared_ptr<View>> >();
		if (!mId.empty()) {
			mpId2ViewMap->emplace(mId, shared_from_this());
		}
		for (auto& pchild : mChildren) {
			if (pchild) {
				pchild->getId2View(mpId2ViewMap);
			}
		}
	}
}


shared_ptr<View> View::createView(const string& name, shared_ptr<View> parent) {
	shared_ptr<View> pView;
	if (name == "LinearLayout"sv) {
		pView = make_shared<LinearLayout>(parent);
	}
	else if (name == "TextView"sv) {
		pView = make_shared<TextView>(parent);
	}
	else if (name == "Button"sv) {
		pView = make_shared<Button>(parent);
	}
	else if (name == "ScrollView"sv) {
		pView = make_shared<ScrollView>(parent);
	}
	else if (name == "ListView"sv) {
		pView = make_shared<ListView>(parent);
	}
	else if (name == "TreeView"sv) {
		pView = make_shared<TreeView>(parent);
	}
	else {
		LOGE("can not create %s view",name.c_str());
	}
	return pView;
}