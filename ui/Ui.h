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
class View : public Attachable{
public:
	virtual void draw() = 0;
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
	Rect<int> rect;
	Color textColor;
	int charSize;//���ִ�С��������Ϊ��λ
	int maxLine{ 1 };
	int mLineSpacingInc{0}; //�м������
	int mCharSpacingInc{ 0 };//�ַ��������
	unsigned int mAligment{ TextAlignment::AlignCenter}; //�ı����뷽ʽ
};

class Button : public View{
public:
	Button(const std::string& str, int x, int y, int w, int h):
		text(str), rect(x, y, w, h)
	{

	}
	Button(int x, int y, int w, int h) :
		rect(x, y, w, h)
	{

	}

	explicit Button(const Rect<int>& r) :
		rect(r)
	{

	}

	void draw() override;
private:
	std::string text;
	Rect<int> rect;
};



#endif