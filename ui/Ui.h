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
class View : public Attachable{
public:
	virtual void draw() = 0;
	virtual void mouseMove(int x,int y) {};

	virtual void mouseLButtonDown(int x, int y) {};

	virtual void mouseLButtonUp(int x, int y) {};

	virtual void onClicked(View* pView) {
		if (mClickedListener) {
			mClickedListener(pView);
		}
	}

	void setOnClickListener(std::function<void(View*)> func) {
		mClickedListener = func;
	}

	std::function<void(View*)> mClickedListener;
	//与parent的相对位置信息
	unsigned int mLayoutWidth;
	unsigned int mLayoutHeight;
	unsigned int mLayoutMarginTop;
	unsigned int mLayoutMarginBottom;
	unsigned int mLayoutMarginLeft;
	unsigned int mLayoutMarginRight;
	unsigned int mLayoutGravite;
};

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

enum LayoutParam{
	Horizontal,
	Vertical,
	WrampContent, 
	MarchParent,
};

class TextView : public View {
public:
	TextView(const std::string& str, int x, int y, int w, int h) :
		text(str), rect(x, y, w, h)
	{

	}

	TextView(const std::string& str, const Rect<int>& r) :
		text(str), rect(r)
	{

	}

	TextView(const std::string& str, const Rect<int>& r,const Color& c) :
		text(str), rect(r), textColor(c)
	{

	}

	explicit TextView(const Rect<int>& r) :
		rect(r)
	{

	}

	TextView(int x, int y, int w, int h) :
		rect(x, y, w, h)
	{

	}

	void setRect(int x, int y, int w, int h) {
		rect.x = x;
		rect.y = y;
		rect.width = w;
		rect.height = h;
	}

	void setRect(const Rect<int>& r) {
		rect = r;
	}

	Rect<int>& getRect() {
		return rect;
	}

	void setTextColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		textColor.r = r; textColor.g = g; textColor.b = b; textColor.a = a;
	}

	void setTextColor(unsigned char r, unsigned char g, unsigned char b) {
		textColor.r = r; textColor.g = g; textColor.b = b;
	}

	void setTextColor(const Color& c) {
		textColor = c;
	}

	Color& getTextColor() {
		return textColor;
	}

	const std::string& getText() {
		return text;
	}

	void setText(const std::string& str) {
		text = str;
	}

	void setCharSize(int size) {
		charSize = size;
	}

	int getCharSize() {
		return charSize;
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

	void setAligment(unsigned int al) {
		mAligment = al;
	}

	unsigned int getAligment() {
		return mAligment;
	}

	void draw() override;
private:
	std::string text;
	Rect<int> rect{0,0,0,0};
	Color textColor{0.0f,0.0f,0.0f,1.0f};
	int charSize{ 32 };//文字大小，以像素为单位
	int maxLine{ 1 };
	int mLineSpacingInc{0}; //行间距增量
	int mCharSpacingInc{ 0 };//字符间距增量
	unsigned int mAligment{ TextAlignment::AlignCenter}; //文本对齐方式
};

class UiRender;
class Button : public View{
public:
	friend UiRender;
	Button(const std::string& str, int x, int y, int w, int h):
		mTv(str,x,y,w,h), mRect(x, y, w, h)
	{

	}
	Button(int x, int y, int w, int h) :
		mTv(x,y,w,h),
		mRect(x, y, w, h)
	{

	}

	explicit Button(const Rect<int>& r) :
		mTv(r),
		mRect(r)
	{

	}

	void setText(const std::string& t) {
		mTv.setText(t);
	}

	Rect<int>& getRect() {
		return mRect;
	}

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

	void setTextColor(const Color& c) {
		mTv.setTextColor(c);
	}

	void draw() override;
private:
	TextView mTv;
	Rect<int> mRect{0,0,0,0};
	Color mColor[4]{ {1.0f,1.0f,1.0f,1.0f},{ 1.0f,1.0f,1.0f,1.0f },{ 1.0f,1.0f,1.0f,1.0f },{ 1.0f,1.0f,1.0f,1.0f } };
	unsigned int mMouseState{ MouseState::MouseNone};
};

class LinearLayout :public View {
public:
	friend UiRender;
	LinearLayout(unsigned int layoutParam) {
		mLayoutParam = layoutParam;
	}

	void setLayoutParam(unsigned int layoutParam) {
		mLayoutParam = layoutParam;
	}

	Rect<int>& getRect() {
		return mRect;
	}

	void draw() override;
private:
	unsigned int mLayoutParam{ LayoutParam::Horizontal };
	Rect<int> mRect{ 0,0,0,0 };
	std::list<std::shared_ptr<View>> mChildren;
};



#endif