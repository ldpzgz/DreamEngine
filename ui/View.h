#ifndef _VIEW_H_
#define _VIEW_H_
#include <string>
#include "../Rect.h"
#include "../Attachable.h"
#include <functional>
#include <list>
#include <memory>
#include <unordered_map>
#include <string>
#include "Background.h"
/*
c++11 中unicode相关
unicode编码，为世界上的每一个字符给定一个编号。
utf-8，utf-16，utf-32：如何将unicode编码存储到内存或文件里面

char 可以存储utf-8编码的字符串
wchar_t  C++98 中的方式
char16_t 存储utf-16编码的字符串
char32_t 存储utf-32编码的字符串

新增的字符串前缀：
u8(表示把后面的字符串转换为UTF-8编码，存储到变量里面,
u 转换为UTF-16编码，存储到变量里面,
U 转换为UTF-32编码，存储到变量里面,
const char* sTest = u8"你好";  //这个要注意，如果使用编译选项-finput-charset=utf-8指定代码文件编码为UTF-8，而实际上代码文件编码为GBK，会导致编译器转换出错
//如果使用-finput-charset=gbk，那么编译器在编译时会将GBK编码的“你好”转换为UTF-8编码，存储到sTest变量里面。正确输出E4 BD A0 E5 A5 BD
const char* sTest = u8"\u4F60\u597D"; //"你好"的 unicode编码码值分别是 0x4F60 和 0x597D


char16_t c = u'\u4f60'; // C++规定 \u后面跟着4个16进制数，这个数是一个unicode编码的值。
char32_t C = U'\U00004f60';// C++规定 \U后面跟8个16进制数，这个数是一个unicode编码的值。

宽字符，窄字符（多字节字符）
有的编码是变长的，例如 UTF-8、GB2312、GBK 等；如果一个字符使用了这种编码方式，就称为多字节字符，或者窄字符
使用固定长度编码一个字符 比如UTF-32、UTF-16 等；如果一个字符使用了这种编码方式，就称为宽字符。
*/

//综上所述，我们应该使用unicode编码，并使用utf-8表示和存储字符。

//准备把ui的逻辑和绘制分开


using namespace std;

enum MouseState : unsigned char {
	MouseNone = 0x00,
	MouseOver = 0x01,
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
	const Vec2i& getTranslateVector() {
		return mTranslateVector;
	};
	void translate(const Vec2i& move) {
		mTranslateVector += move;
	}
protected:
	Vec2i mTranslateVector;
};

//这个类及其子类都只能在堆中分配内存，用智能指针管理
class View : public Attachable,public std::enable_shared_from_this<View>{
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

	virtual bool mouseMove(int x, int y) { return false; };

	virtual bool mouseLButtonDown(int x, int y) { return false; };

	virtual bool mouseLButtonUp(int x, int y) { return false; };

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

	virtual void setBackgroundColor(const Color& c) {
		if (!mpBackground) {
			mpBackground = make_shared<Background>();
			mpBackground->mpShape = make_shared<Shape>();
		}
		
		if (mpBackground->mpShape) {
			mpBackground->mpShape->setSolidColor(c);
		}
		
	}

	Color& getBackgroundColor() {
		static Color temp{ 0.0f,0.0f,0.0f,0.0f };
		if (mpBackground && mpBackground->mpShape) {
			return mpBackground->mpShape->getSolidColor();
		}
		return temp;
	}

	void setBackgroundImg(shared_ptr<Texture>& pTex) {
		if (!mpBackground) {
			mpBackground = make_shared<Background>();
			mpBackground->mpShape = make_shared<Shape>();
		}
		
		mpBackground->mpTexture = pTex;
		
	}

	std::shared_ptr<Background>& getBackground() {
		return mpBackground;
	}

	void setBackgroundShape(const shared_ptr<Shape>& pShape) {
		if (!mpBackground) {
			mpBackground = make_shared<Background>();
		}

		mpBackground->mpShape = pShape;
	}

	void initBackground();

	Vec2i getTranslateVector(){
		auto pMoveLis = mpMoveListener.lock();
		if (pMoveLis) {
			return pMoveLis->getTranslateVector();
		}
		else {
			return Vec2i();
		}
	}

	void setMove(Vec2i v) {
		mMoveVector = v;
		for (auto& pChild : mChildren) {
			if (pChild) {
				pChild->setMove(v);
			}
		}
	}

	Vec2i& getMoveVector() {
		return mMoveVector;
	}

	virtual void afterGetWidthHeight() {

	}

	static shared_ptr<View> createView(const string& name, shared_ptr<View> parent);

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
	Vec2i mMoveVector;
	std::list<std::shared_ptr<View>> mChildren;
	weak_ptr<DirtyListener> mpDirtyListener;
	weak_ptr<MoveListener> mpMoveListener;
	bool mbIsDirty{ false };
	std::shared_ptr<Background> mpBackground;
	std::shared_ptr<void> mpBackgroundMesh;
	std::unique_ptr< std::unordered_map<std::string, std::shared_ptr<View>> > mpId2ViewMap;
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

	static unordered_map < string, std::function<void(const shared_ptr<View>&, const std::string&)>> gLayoutAttributeHandler;
	static unordered_map<string, int> gGravityKeyValue;
	

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
};

extern std::shared_ptr<View> gpViewNothing;

#endif