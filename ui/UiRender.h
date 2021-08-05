#ifndef _UI_RENDER_H_
#define _UI_RENDER_H_
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>
#include <locale.h>
#include <iostream>
#include <string>
#include <map>
#include <memory>
#include "../material.h"
using namespace std;

//void initUi();

class CharInfo
{
public:
	char16_t code;
	int x;
	int y;		//x,y表示字符在纹理中的起始位置
	int left;		//left
	int top;		//top,相对于基线的高度，用于实际渲染
	int width;	//字符的宽度
	int height; //字符的高度
	int advX; //表示下一个字符与本字符的水平距离
	int advY; //表示下一个字符与本字符的垂直距离
};

class FontInfo
{
public:
	FontInfo(int texWidth, int texHeight, const string& filePath, const string& ttfPath);
	~FontInfo();
	/*
	功能：从filePath加载预渲染好的文字信息，后面渲染好的文字，会保存到这个文件里面
	filePath	自定义的用于保存文字信息的文件的路径
	ttfPath		用于渲染文字的ttf等字体文件的路径
	imageWidth	运行时，用于保存文字位图的opengl纹理的宽度
	imageHeight	运行时，用于保存文字位图的opengl纹理的高度
	*/
	static shared_ptr<FontInfo> loadFromFile(const string& filePath, const string& ttfPath, int imageWidth, int imageHeight);

	/*
	text 为utf-8编码的字符
	*/
	void rendText(const string& text);

	void saveToFile();

	int textureWidth{ 0 };
	int textureHeight{ 0 };
	string savePath;
	shared_ptr<Texture> pCharTexture;
	int curTextureHeight{ 0 }; //pCharTexture纹理里面当前已经被占用的高度
	int curTextureWidth{ 0 };  //pCharTexture纹理里面当前行的宽度

	map<char32_t, CharInfo> fontsMap;

	using FontsMapValueType = map<char32_t, CharInfo>::value_type;

	static 	FT_Library  glibrary;
	static  FT_Face     gface;      /* handle to face object */
	static  bool gIsFreetypeInit;
	
private:
	
	const CharInfo& getCharInfo(char32_t code);
	/*
	初始化用于保存文字位图信息的opengl纹理
	*/
	bool initCharTexture();
	static bool initFreetype(const string& ttfPath);
	static void releaseFreetype();
};

class TextView;
class Button;
class UiRender {
public:
	static unique_ptr<UiRender>& getInstance() {
		return gInstance;
	}

	void initUiRender();
	void drawTextView(TextView* tv);
	void drawButton(Button* tv);
private:
	shared_ptr<FontInfo> pFontInfo;
	static unique_ptr<UiRender> gInstance;
};

#endif