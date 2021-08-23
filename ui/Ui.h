#ifndef _UI_H_
#define _UI_H_
/*
c++11 ��unicode���
unicode���룬Ϊ�����ϵ�ÿһ���ַ�����һ����š�
utf-8��utf-16��utf-32����ν�unicode����洢���ڴ���ļ�����

char ���Դ洢utf-8������ַ���
wchar_t  C++98 �еķ�ʽ
char16_t �洢utf-16������ַ���
char32_t �洢utf-32������ַ���

�������ַ���ǰ׺��
u8(��ʾ�Ѻ�����ַ���ת��ΪUTF-8���룬�洢����������,
u ת��ΪUTF-16���룬�洢����������,
U ת��ΪUTF-32���룬�洢����������,
const char* sTest = u8"���";  //���Ҫע�⣬���ʹ�ñ���ѡ��-finput-charset=utf-8ָ�������ļ�����ΪUTF-8����ʵ���ϴ����ļ�����ΪGBK���ᵼ�±�����ת������
//���ʹ��-finput-charset=gbk����ô�������ڱ���ʱ�ὫGBK����ġ���á�ת��ΪUTF-8���룬�洢��sTest�������档��ȷ���E4 BD A0 E5 A5 BD
const char* sTest = u8"\u4F60\u597D"; //"���"�� unicode������ֵ�ֱ��� 0x4F60 �� 0x597D


char16_t c = u'\u4f60'; // C++�涨 \u�������4��16���������������һ��unicode�����ֵ��
char32_t C = U'\U00004f60';// C++�涨 \U�����8��16���������������һ��unicode�����ֵ��

���ַ���խ�ַ������ֽ��ַ���
�еı����Ǳ䳤�ģ����� UTF-8��GB2312��GBK �ȣ����һ���ַ�ʹ�������ֱ��뷽ʽ���ͳ�Ϊ���ֽ��ַ�������խ�ַ�
ʹ�ù̶����ȱ���һ���ַ� ����UTF-32��UTF-16 �ȣ����һ���ַ�ʹ�������ֱ��뷽ʽ���ͳ�Ϊ���ַ���
*/

//��������������Ӧ��ʹ��unicode���룬��ʹ��utf-8��ʾ�ʹ洢�ַ���

//׼����ui���߼��ͻ��Ʒֿ�
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

enum TextAlignment : unsigned char { //���ﲻ�ʺ�ʹ��ǿ����ö��
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

//����༰�����඼ֻ���ڶ��з����ڴ棬������ָ�����
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
	layout_width Ϊmatchparent,���߹̶��ߴ��ʱ���ʱ������������������
	*/
	virtual void calcWidth(int width);
	virtual void calcHeight(int height);
	//����ɸ�view��ȷ����ʱ���ɸ�view������
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
	//x�Ǿ������ĵ��x
	void setCenterX(int x) {
		mRect.x = x - mRect.width/2;
	}
	//y�Ǿ������ĵ��y
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
	
	//��parent�����λ����Ϣ
	int mLayoutWidth{ 0 };		//match_parent,����Ŀ�ȣ�������Ϊ��λ
	int mLayoutHeight{ 0 };		//match_parent,����Ŀ�ȣ�������Ϊ��λ
	int mLayoutMarginTop{ 0 };
	int mLayoutMarginBottom{ 0 };
	int mLayoutMarginLeft{ 0 };
	int mLayoutMarginRight{ 0 };
	int mWidthPercent{ 0 };		//��Ȱٷֱ�
	int mHeightPercent{ 0 };	//�߶Ȱٷֱ�
	int mGravity{ LayoutParam::Center };			//����view�ڲ���Ԫ�ػ�����view��ξ��ж��룬ˮƽ���У���ֱ���У�����

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
	using View::View;//�̳л���Ĺ��캯��
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
	int mTextSize{ 32 };//���ִ�С��������Ϊ��λ
	int maxLine{ 1 };
	int mLineSpacingInc{0}; //�м������
	int mCharSpacingInc{ 0 };//�ַ��������
	//unsigned int mAligment{ TextAlignment::AlignCenter}; //�ı����뷽ʽ
};

class UiRender;
class Button : public TextView{
public:
	friend UiRender;
	using TextView::TextView;//�̳л���Ĺ��캯��

	/*
	����button�ı�����ɫ
	*/
	void setBackgroundColor(const Color& color) {
		for (auto& mc : mColor) {
			mc = color;
		}
	}
	/*
	����button�ı�����ɫ�������ҽ���
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
	using View::View;//�̳л���Ĺ��캯��
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