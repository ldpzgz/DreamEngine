#ifndef _CHARINFO_H_
#define _CHARINFO_H_

using UnicodeType = char32_t;
//一个字符的位图信息，描述了该字符在纹理里面的位置，宽高等信息
class CharInfo
{
public:
	int x{ 0 };
	int y{ 0 };		//(x,y)表示字符在纹理中的起始位置
	int width{ 0 };	//字符的宽度
	int height{ 0 };	//字符的高度,x,y,width,height取值范围都是[0,1]
	int left{ 0 };		//left
	int top{ 0 };		//top,相对于基线的高度，单位是像素，用于实际渲染
	int base{ 0 };
	int advX{ 0 }; //表示下一个字符与本字符的水平距离，单位是像素
	int advY{ 0 }; //表示下一个字符与本字符的垂直距离，单位是像素
};

#endif
