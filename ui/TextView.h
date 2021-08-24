#ifndef _TEXT_VIEW_H_
#define _TEXT_VIEW_H_
#include "View.h"
using namespace std;

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
#endif