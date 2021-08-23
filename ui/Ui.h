#ifndef _UI_H_
#define _UI_H_
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
#include <string>
#include "../Rect.h"
#include "../Attachable.h"
#include <functional>
#include <list>
#include <memory>
#include <unordered_map>
#include <string>
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

//这个类及其子类都只能在堆中分配内存，用智能指针管理
class View : public Attachable,public std::enable_shared_from_this<View>{
public:
	explicit View(shared_ptr<View> parent):mpParent(parent) {
	}

	virtual ~View() {};

	View(shared_ptr<View> parent,int x, int y, int w, int h) :
		mpParent(parent),
		mRect(x, y, w, h)
	{

	}

	View(int x, int y, int w, int h) :
		mRect(x,y,w,h)
	{

	}

	View(shared_ptr<View> parent, const Rect<int>& r) :
		mpParent(parent),
		mRect(r)
	{

	}

	View(const Rect<int>& r) :
		mRect(r)
	{

	}

	View() = default;

	virtual void draw() {
		setDirty(false);
	}

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

	virtual bool calcRect(const Rect<int>& parentRect);
	/*
	layout_width 为matchparent,或者固定尺寸的时候的时候调用这个函数计算宽度
	*/
	virtual void calcWidth(int width);
	virtual void calcHeight(int height);
	//宽高由父view来确定的时候，由父view来调用
	virtual void setWidth(int width);
	virtual void setHeight(int height);
	virtual int getTotalWidthPercent() {
		return 0;
	}
	virtual int getTotalHeightPercent() {
		return 0;
	}
	virtual void calcChildPos() {
		return;
	}
	void calcX(int x) {
		mRect.x = x + mLayoutMarginLeft;
	}
	void calcY(int y) {
		mRect.y = y + mLayoutMarginTop;
	}
	//x是矩形中心点的x
	void setCenterX(int x) {
		mRect.x = x - mRect.width/2;
	}
	//y是矩形中心点的y
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
	}

	void setDirty(bool b) {
		if (mbIsDirty == b) {
			return;
		}
		mbIsDirty = b;
		if (mbIsDirty) {
			auto pListener = mpDirtyListener.lock();
			if (pListener) {
				pListener->addDirtyView(shared_from_this());
			}
		}
	}

	bool getDirty() {
		return mbIsDirty;
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
	int mGravity{ LayoutParam::Center };			//控制view内部的元素或者子view如何居中对齐，水平居中，垂直居中，居中

	Rect<int> mRect{ 0,0,0,0 };

	std::list<std::shared_ptr<View>> mChildren;
	weak_ptr<DirtyListener> mpDirtyListener;
	bool mbIsDirty{ false };

	static void layoutWidthHandler(const shared_ptr<View>&, const std::string&);
	static void layoutWidthPercentHandler(const shared_ptr<View>&, const std::string&);
	static void layoutHeightHandler(const shared_ptr<View>&, const std::string&);
	static void layoutHeightPercentHandler(const shared_ptr<View>&, const std::string&);
	static void layoutMarginTopHandler(const shared_ptr<View>&, const std::string&);
	static void layoutMarginBottomHandler(const shared_ptr<View>&, const std::string&);
	static void layoutMarginLeftHandler(const shared_ptr<View>&, const std::string&);
	static void layoutMarginRightHandler(const shared_ptr<View>&, const std::string&);
	static void gravityHandler(const shared_ptr<View>&, const std::string&);

	static unordered_map < string, std::function<void(const shared_ptr<View>&, const std::string&)>> gAttributeHandler;
};

class TextView : public View {
public:
	using View::View;//继承基类的构造函数
	TextView(shared_ptr<View> parent,const std::string& str, int x, int y, int w, int h) :
		View(parent,x,y,w,h),
		mText(str)
	{

	}

	TextView(shared_ptr<View> parent, const std::string& str, const Rect<int>& r) :
		View(parent,r),
		mText(str)
	{

	}

	TextView(shared_ptr<View> parent,const std::string& str, const Rect<int>& r,const Color& c) :
		View(parent,r), mText(str), mTextColor(c)
	{

	}

	TextView() = default;

	void setTextColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		mTextColor[0] = r; mTextColor[1] = g; mTextColor[2] = b; mTextColor[3] = a;
	}

	void setTextColor(unsigned char r, unsigned char g, unsigned char b) {
		mTextColor[0] = r; mTextColor[1] = g; mTextColor[2] = b;
	}

	void setTextColor(const Color& c) {
		mTextColor = c;
	}

	Color& getTextColor() {
		return mTextColor;
	}

	const std::string& getText() {
		return mText;
	}

	void setText(const std::string& str) {
		mText = str;
	}

	void setTextSize(int size) {
		mTextSize = size;
	}

	int getTextSize() {
		return mTextSize;
	}

	void setMaxLines(int lines) {
		maxLine = lines;
	}

	int getMaxLines() {
		return maxLine;
	}

	void setCharSpacingInc(int inc) {
		if(inc>0)
			mCharSpacingInc = inc;
	}

	int getCharSpacingInc() {
		return mCharSpacingInc;
	}

	void setLineSpacingInc(int inc) {
		if (inc>0)
			mLineSpacingInc = inc;
	}

	int getLineSpacingInc() {
		return mLineSpacingInc;
	}

	/*void setAligment(unsigned int al) {
		mAligment = al;
	}

	unsigned int getAligment() {
		return mAligment;
	}*/

	void draw() override;

	static void textSizeHandler(const shared_ptr<View>&, const std::string&);
	static void textColorHandler(const shared_ptr<View>&, const std::string&);
	static void textHandler(const shared_ptr<View>&, const std::string&);
private:
	std::string mText;
	Color mTextColor{0.0f,0.0f,0.0f,1.0f};
	int mTextSize{ 32 };//文字大小，以像素为单位
	int maxLine{ 1 };
	int mLineSpacingInc{0}; //行间距增量
	int mCharSpacingInc{ 0 };//字符间距增量
	//unsigned int mAligment{ TextAlignment::AlignCenter}; //文本对齐方式
};

class UiRender;
class Button : public TextView{
public:
	friend UiRender;
	using TextView::TextView;//继承基类的构造函数

	/*
	设置button的背景颜色
	*/
	void setBackgroundColor(const Color& color) {
		for (auto& mc : mColor) {
			mc = color;
		}
	}
	/*
	设置button的背景颜色，从左到右渐变
	*/
	void setBackgroundColor(const Color& colorLeft, const Color& colorRight) {
		mColor[0] = colorLeft;
		mColor[1] = colorLeft;
		mColor[2] = colorRight;
		mColor[3] = colorRight;
	}

	void draw() override;

	bool mouseMove(int x, int y) override;

	bool mouseLButtonDown(int x, int y) override;

	bool mouseLButtonUp(int x, int y) override;
private:
	Color mColor[4]{ {1.0f,1.0f,1.0f,1.0f},{ 1.0f,1.0f,1.0f,1.0f },{ 1.0f,1.0f,1.0f,1.0f },{ 1.0f,1.0f,1.0f,1.0f } };
	unsigned int mMouseState{ MouseState::MouseNone};
	bool mbLButtonDown{ false };
};

class LinearLayout :public View {
public:
	friend UiRender;
	using View::View;//继承基类的构造函数
	LinearLayout(shared_ptr<View> parent, unsigned int layoutParam):
		View(parent),
		mLayoutParam(layoutParam)
	{

	}
	explicit LinearLayout(unsigned int layoutParam):
		mLayoutParam(layoutParam)
	{
	}

	void setLayoutParam(unsigned int layoutParam) {
		mLayoutParam = layoutParam;
	}

	void draw() override;

	int getTotalWidthPercent() override;

	int getTotalHeightPercent() override;

	void calcChildPos() override;

	static void orientationHandler(const shared_ptr<View>&, const string&);
private:
	unsigned int mLayoutParam{ LayoutParam::Horizontal };
	int mOrientation{ 0 };
	int mTotalWidthPercent{ 0 };
	int mTotalHeightPercent{ 0 };
};
#endif