#ifndef _TEXT_VIEW_H_
#define _TEXT_VIEW_H_
#include "View.h"
#include <limits>
#include "CharPosition.h"
using namespace std;

class TextView : public View {
public:
	using View::View;//继承基类的构造函数
	TextView() = default;

	void setTextColor(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		Color temp(r,g,b,a);
		if (mTextColor != temp) {
			setDirty(true);
		}
		mTextColor[0] = r; mTextColor[1] = g; mTextColor[2] = b; mTextColor[3] = a;
	}

	void setTextColor(unsigned char r, unsigned char g, unsigned char b) {
		Color temp(r, g, b);
		if (mTextColor != temp) {
			setDirty(true);
		}
		mTextColor = temp;
	}

	void setTextColor(const Color& c) {
		if (mTextColor != c) {
			setDirty(true);
		}
		mTextColor = c;
	}

	Color& getTextColor() {
		return mTextColor;
	}

	const std::string& getText() {
		return mText;
	}

	void setText(const std::string& str) {
		if (mText != str) {
			setDirty(true);
			calcTextPosition();
		}
		mText = str;
	}

	void setTextSize(int size) {
		mTextSize = size;
	}

	void setMaxWidth(int max) {
		mMaxWidth = max;
	}

	int getMaxWidth() {
		return mMaxWidth;
	}

	void setMaxHeight(int max) {
		mMaxHeight = max;
	}

	int getMaxHeight() {
		return mMaxHeight;
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
	//计算出文本所在的屏幕位置，以及每个字符对应的纹理坐标(所有字符存在一张纹理上面)
	void calcTextPosition();

	std::vector<CharPosition>& getCharPositionArray() {
		return mTextPositions;
	}

	void setUpdateTextPosition(bool b) {
		mbUpdateTextPositioin = b;
	}

	bool getUpdateTextPosition() {
		return mbUpdateTextPositioin;
	}

	void draw() override;
	bool calcWidth(int parentHeight) override;
	bool calcHeight(int parentHeight) override;

	static void textSizeHandler(const shared_ptr<View>&, const std::string&);
	static void textColorHandler(const shared_ptr<View>&, const std::string&);
	static void textHandler(const shared_ptr<View>&, const std::string&);
	static void maxWidthHandler(const shared_ptr<View>&, const std::string&);
	static void maxHeightHandler(const shared_ptr<View>&, const std::string&);
	static void maxLineHandler(const shared_ptr<View>&, const std::string&);
	static void charSpaceHandler(const shared_ptr<View>&, const std::string&);
	static void lineSpaceHandler(const shared_ptr<View>&, const std::string&);
private:
	std::string mText;
	Color mTextColor{0.0f,0.0f,0.0f,1.0f};
	int mTextSize{ 32 };//文字大小，以像素为单位
	int maxLine{ 1000000000 };
	int mMaxWidth{ 100000000 };//默认是一个很大的数std::numeric_limits<int>::max()
	int mMaxHeight{ 100000000 };//默认是一个很大的数std::numeric_limits<int>::max()
	int mLineSpacingInc{ 0 }; //行间距增量
	int mCharSpacingInc{ 0 };//字符间距增量

	std::vector<CharPosition> mTextPositions;
	bool mbUpdateTextPositioin{ true };
	//unsigned int mAligment{ TextAlignment::AlignCenter}; //文本对齐方式
};
#endif