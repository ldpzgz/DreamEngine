#ifndef _VIEW_H_
#define _VIEW_H_
#include <string>
#include "../Rect.h"
#include "../Color.h"
#include <functional>
#include <list>
#include <memory>
#include <unordered_map>
#include <string>
#include <glm/vec2.hpp>           // vec2


using namespace std;

class MeshFilledRect;
class Background;
class Texture;
class Shape;

enum MouseState : unsigned char {
	MouseNone = 0x00,
	MouseHover = 0x01,
	MouseLButtonDown = 0x02,
	MouseLButtonUp = 0x04,
	MouseRButtonDown = 0x08,
	MouseRButtonUp = 0x10,
};

enum TextAlignment : unsigned char { //这里不适合使用强类型枚举
	AlignLeft = 0x01,
	AlignRight = 0x02,
	AlignTop = 0x04,
	AlignBottom = 0x08,
	AlignHCenter = 0x10,
	AlignVCenter = 0x20,
	AlignCenter = 0x40
};

enum LayoutParam {
	WrapContent = -2,
	MatchParent = -1,
	Horizontal,
	Vertical,

	LeftTop,
	TopCenter,
	RightTop,
	LeftCenter,
	Center,
	RightCenter,
	LeftBottom,
	BottomCenter,
	RightBottom,
};

class View;
class DirtyListener {
public:
	virtual void addDirtyView(const shared_ptr<View>& pView) = 0;
};

//如果父view要移动子view，而子view可能非常多的情况下，
//可以继承这个类，让子view监听移动。子view共享同样的移动量。
//例如ScrollView
class MoveListener {
public:
	MoveListener() = default;
	virtual ~MoveListener() {

	}
	const glm::ivec2& getTranslateVector() {
		return mTranslateVector;
	};
	void translate(const glm::ivec2& move) {
		mTranslateVector += move;
	}
protected:
	glm::ivec2 mTranslateVector{0,0};
};

//这个类及其子类都只能在堆中分配内存，用智能指针管理
class View : public std::enable_shared_from_this<View>{
public:
	explicit View(shared_ptr<View> parent):mpParent(parent) {
	}

	virtual ~View() {};

	View() = default;

	virtual void draw() {
		setDirty(false);
	}

	virtual void drawBackground();

	weak_ptr<View> getParent() {
		return mpParent;
	}

	void setParent(const shared_ptr<View>& parent) {
		mpParent = parent;
	}

	list<shared_ptr<View>>& getChildren() {
		return mChildren;
	}

	void addChild(const shared_ptr<View>& pChild) {
		mChildren.emplace_back(pChild);
	}

	//virtual bool isMouseInside(int x, int y) { return false; }

	virtual bool mouseMove(int x, int y, bool notInside);

	virtual bool mouseLButtonDown(int x, int y,bool notInside);

	virtual bool mouseLButtonUp(int x, int y,bool notInside);

	virtual void onClicked(View* pView) {
		if (mClickedListener) {
			mClickedListener(pView);
		}
	}

	void setOnClickListener(std::function<void(View*)> func) {
		mClickedListener = func;
	}

	std::string getId() {
		return mId;
	}

	void setId(const std::string& id) {
		mId = id;
	}

	void setRect(int x, int y, int w, int h) {
		mRect.x = x;
		mRect.y = y;
		mRect.width = w;
		mRect.height = h;
	}

	void setRect(const Rect<int>& r) {
		mRect = r;
	}

	Rect<int>& getRect() {
		return mRect;
	}

	int getGravity() {
		return mGravity;
	}

	void setGravity(int g) {
		mGravity = g;
	}

	virtual bool calcRect(int parentWidth, int parentHeight);

	shared_ptr<View>& findViewById(const std::string& id);
	//将有Id的控件搜集起来，以便查找
	void getId2View(std::unique_ptr< std::unordered_map< std::string, std::shared_ptr<View> > >& pId2ViewMap);
protected:
	virtual bool calcWidthHeight(int parentWidth, int parentHeight);
	virtual bool calcPos();
	/*
	width为matchparent,或者固定尺寸的时候的时候调用这个函数计算宽度
	*/
	virtual bool calcWidth(int parentWidth);
	/*
	height为matchparent,或者固定尺寸的时候的时候调用这个函数计算宽度
	*/
	virtual bool calcHeight(int parentHeight);
	//百分比布局的时候，宽高由父view来确定，由父view来调用设置宽高
	virtual void setWidth(int width);
	//百分比布局的时候，宽高由父view来确定，由父view来调用设置宽高
	virtual void setHeight(int height);

	virtual void getWidthAccordChildren() {
	}
	virtual void getHeightAccordChildren() {
	}
	virtual int getChildrenTotalWidthPercent() {
		return 0;
	}
	virtual int getChildrenTotalHeightPercent() {
		return 0;
	}
	virtual void calcChildPos() {
		return;
	}
public:	 
	void alignLeftX(int x) {
		mRect.x = x + mLayoutMarginLeft;
	}
	
	void alignRightX(int x) {
		mRect.x = x - mLayoutMarginRight - mRect.width;
	}
	
	void alignTopY(int y) {
		mRect.y = y + mLayoutMarginTop;
	}

	void alignBottomY(int y) {
		mRect.y = y - ( mRect.height + mLayoutMarginBottom );
	}
	//x是该控件的中心点的x坐标，再根据它自身的宽度可以算出左上角x坐标
	void setCenterX(int x) {
		mRect.x = x - mRect.width/2;
	}
	//y是该控件的中心点的y坐标，再根据它自身的宽度可以算出左上角y坐标
	void setCenterY(int y) {
		mRect.y = y - mRect.height/2;
	}
	int advanceX() {
		return mRect.width + mLayoutMarginLeft + mLayoutMarginRight;
	}
	int advanceY() {
		return mRect.height + mLayoutMarginTop + mLayoutMarginBottom;
	}

	void setDirtyListener(const shared_ptr<DirtyListener>& pDirtyListener) {
		mpDirtyListener = pDirtyListener;
		for (auto& pChild : mChildren) {
			if (pChild) {
				pChild->setDirtyListener(pDirtyListener);
			}
		}
	}

	void setMoveListener(const shared_ptr<MoveListener>& pMoveListener) {
		mpMoveListener = pMoveListener;
		for (auto& pchild : mChildren) {
			pchild->setMoveListener(pMoveListener);
		}
	}

	void setDirty(bool b) {
		if (!mpDirtyListener.lock() || mbIsDirty == b) {
			return;
		}
		mbIsDirty = b;
		if (mbIsDirty) {
			auto pListener = mpDirtyListener.lock();
			if (pListener) {
				pListener->addDirtyView(shared_from_this());
			}
			
			if (!mpBackground) {
				auto pParent = mpParent.lock();
				if (pParent) {
					pParent->setDirty(true);
				}
			}
		}
	}

	bool getDirty() {
		return mbIsDirty;
	}

	void setBackground(const std::shared_ptr<Background>& bk) {
		mpBackground = bk;
	}

	void setBackgroundImg(shared_ptr<Texture>& pTex);

	std::shared_ptr<Background>& getBackground() {
		return mpBackground;
	}

	void setBackgroundShape(const shared_ptr<Shape>& pShape);

	void initBackground();

	glm::ivec2 getTranslateVector(){
		auto pMoveLis = mpMoveListener.lock();
		if (pMoveLis) {
			return pMoveLis->getTranslateVector();
		}
		else {
			return glm::ivec2(0,0);
		}
	}

	void setMove(glm::ivec2 v) {
		mMoveVector = v;
		for (auto& pChild : mChildren) {
			if (pChild) {
				pChild->setMove(v);
			}
		}
	}

	glm::ivec2& getMoveVector() {
		return mMoveVector;
	}

	bool getEnabled() {
		return mbEnabled;
	}
	void setEnable(bool b) {
		mbEnabled = b;
	}

	unsigned int getMouseStatus() {
		return mMouseState;
	}

	int& getWidthPercent() {
		return mWidthPercent;
	}

	int& getHeightPercent() {
		return mHeightPercent;
	}

	int& getLayoutHeight() {
		return mLayoutHeight;
	}

	int& getLayoutWidth() {
		return mLayoutWidth;
	}

	int& getLayoutMarginBottom() {
		return mLayoutMarginBottom;
	}
	int& getLayoutMarginLeft() {
		return mLayoutMarginLeft;
	}
	int& getLayoutMarginRight() {
		return mLayoutMarginRight;
	}
	int& getLayoutMarginTop() {
		return mLayoutMarginTop;
	}

	virtual void orientationHandler(const string& content) {

	}

	virtual void textSizeHandler(const std::string& content) {

	}
	virtual void textColorHandler(const std::string& content) {

	}
	virtual void textHandler(const std::string& content) {

	}
	virtual void maxWidthHandler(const std::string& content) {

	}
	virtual void maxHeightHandler(const std::string& content) {

	}
	virtual void maxLineHandler(const std::string& content) {

	}
	virtual void charSpaceHandler(const std::string& content) {

	}
	virtual void lineSpaceHandler(const std::string& content) {

	}

	virtual void afterGetWidthHeight() {

	}

	virtual void setBackgroundColor(const Color& c);

	static shared_ptr<View> createView(const string& name, shared_ptr<View> parent);

	static void idHandler(const shared_ptr<View>&, const std::string&);
	static void layoutWidthHandler(const shared_ptr<View>&, const std::string&);
	static void layoutWidthPercentHandler(const shared_ptr<View>&, const std::string&);
	static void layoutHeightHandler(const shared_ptr<View>&, const std::string&);
	static void layoutHeightPercentHandler(const shared_ptr<View>&, const std::string&);
	static void layoutMarginTopHandler(const shared_ptr<View>&, const std::string&);
	static void layoutMarginBottomHandler(const shared_ptr<View>&, const std::string&);
	static void layoutMarginLeftHandler(const shared_ptr<View>&, const std::string&);
	static void layoutMarginRightHandler(const shared_ptr<View>&, const std::string&);
	static void gravityHandler(const shared_ptr<View>&, const std::string&);
	static void backgroundHandler(const shared_ptr<View>&, const std::string&);

	static void orientationHandler_s(const shared_ptr<View>&, const string&);
	static void textSizeHandler_s(const shared_ptr<View>&, const std::string&);
	static void textColorHandler_s(const shared_ptr<View>&, const std::string&);
	static void textHandler_s(const shared_ptr<View>&, const std::string&);
	static void maxWidthHandler_s(const shared_ptr<View>&, const std::string&);
	static void maxHeightHandler_s(const shared_ptr<View>&, const std::string&);
	static void maxLineHandler_s(const shared_ptr<View>&, const std::string&);
	static void charSpaceHandler_s(const shared_ptr<View>&, const std::string&);
	static void lineSpaceHandler_s(const shared_ptr<View>&, const std::string&);
protected:
	bool mbIsDirty{ false };
	bool mbEnabled{ true };
	unsigned int mMouseState{ MouseState::MouseNone };
	std::string mId;
	std::function<void(View*)> mClickedListener;
	weak_ptr<void> mpUserData;
	weak_ptr<View> mpParent;

	//与parent的相对位置信息
	int mLayoutWidth{ 0 };		//match_parent,具体的宽度，以像素为单位
	int mLayoutHeight{ 0 };		//match_parent,具体的宽度，以像素为单位
	int mLayoutMarginTop{ 0 };
	int mLayoutMarginBottom{ 0 };
	int mLayoutMarginLeft{ 0 };
	int mLayoutMarginRight{ 0 };
	int mWidthPercent{ 0 };		//宽度百分比
	int mHeightPercent{ 0 };	//高度百分比
	int mGravity{ LayoutParam::Center };//控制view内部的元素或者子view如何居中对齐，水平居中，垂直居中，居中
	Rect<int> mRect{ 0,0,0,0 };
	glm::ivec2 mMoveVector{ 0,0 };
	std::list<std::shared_ptr<View>> mChildren;
	weak_ptr<DirtyListener> mpDirtyListener;
	weak_ptr<MoveListener> mpMoveListener;
	std::shared_ptr<Background> mpBackground;
	std::unique_ptr< std::unordered_map<std::string, std::shared_ptr<View>> > mpId2ViewMap;
public:
	static unordered_map < string, std::function<void(const shared_ptr<View>&, const std::string&)>> gLayoutAttributeHandler;
	static unordered_map<string, int> gGravityKeyValue;
};

extern std::shared_ptr<View> gpViewNothing;

#endif