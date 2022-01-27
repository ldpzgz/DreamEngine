#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "FreetypeUtils.h"
#define STB_TRUETYPE_IMPLEMENTATION
#include "../stb_truetype.h"
#include "../Log.h"
using namespace std;
static const string gFontFile("./opengles3/material/aliFont.ttf");

//static 	FT_Library  glibrary;
//static  FT_Face     gface;      /* handle to face object */
//static  bool gIsFreetypeInit = false;
//using namespace std;
//bool initFreetype(const string& ttfPath) {
//	int error = 0;
//	if (!gIsFreetypeInit)
//	{
//
//		error = FT_Init_FreeType(&glibrary);
//		if (error)
//		{
//			LOGE("error to init freetype\n");
//			return false;
//		}
//
//		error = FT_New_Face(glibrary,
//			ttfPath.c_str(),
//			0,
//			&gface);
//		if (error == FT_Err_Unknown_File_Format)
//		{
//			LOGE("error to FT_New_Face FT_Err_Unknown_File_Format \n");
//			FT_Done_FreeType(glibrary);
//			return false;
//		}
//		else if (error)
//		{
//			LOGE("error to FT_New_Face other \n");
//			FT_Done_FreeType(glibrary);
//			return false;
//		}
//
//		error = FT_Select_Charmap(gface, FT_ENCODING_UNICODE);
//		if (error)
//		{
//			LOGE("error to FT_Select_Charmap \n");
//			return false;
//		}
//	}
//	gIsFreetypeInit = true;
//	return true;
//}
//
//void releaseFreetype()
//{
//	if (gIsFreetypeInit)
//	{
//		FT_Done_Face(gface);
//		FT_Done_FreeType(glibrary);
//	}
//}
//
//bool getCharBitmap(UnicodeType code, int charPixelSize, CharInfo& info, std::vector<unsigned char>& tempBuf) {
//	if (!gIsFreetypeInit) {
//		initFreetype(gFontFile);
//	}
//
//	//使用freetype渲染出来，然后更新到纹理上，记录字符在纹理上的位置信息。
//	int error = 0;
//	error = FT_Set_Pixel_Sizes(gface, charPixelSize, charPixelSize);
//	if (error)
//	{
//		LOGE("error to FT_Set_Pixel_Sizes \n");
//		return false;
//	}
//	FT_Select_Charmap(gface, FT_ENCODING_UNICODE);
//	error = FT_Load_Char(gface, code, FT_LOAD_RENDER);
//	if (error)
//	{
//		LOGE("error to FT_Load_Char \n");
//		return false;
//	}
//	
//	FT_GlyphSlot slot = gface->glyph;
//	/*auto metrics = slot->metrics;*/
//	info.left = slot->bitmap_left;
//	info.top = slot->bitmap_top;
//	info.width = slot->bitmap.width;
//	info.height = slot->bitmap.rows;
//	info.advX = slot->advance.x >> 6;
//	info.advY = slot->advance.y >> 6;
//
//	tempBuf.resize(info.width * info.height);
//	unsigned char* pStart = slot->bitmap.buffer;
//	unsigned char* pEnd = tempBuf.data();
//	//把字符点阵数据更新到纹理上。
//	//由于opengl纹理坐标的原点是在左下角，上下颠倒一下图像
//	for (int i = info.height - 1; i >= 0; --i) {
//		memcpy(pEnd, pStart + (slot->bitmap.pitch * i), info.width);
//		pEnd += info.width;
//	}
//
//	return true;
//}

std::vector<unsigned char> gFContent;
stbtt_fontinfo sgfont;
float sgScale = 1.0f;
int sgAscent;
int sgDscent;
int sgBaseLine;
bool getCharBitmap2(UnicodeType code, int charPixelSize, CharInfo& info, std::vector<unsigned char>& tempBuf) {
	if (gFContent.empty()) {
		std::ifstream gfFile(gFontFile, std::ios::binary);
		gFContent.assign((std::istreambuf_iterator<char>(gfFile)), std::istreambuf_iterator<char>());
		std::cout << "font file size is " << gFContent.size() << std::endl;
		stbtt_InitFont(&sgfont, gFContent.data(), 0);
		sgScale = stbtt_ScaleForPixelHeight(&sgfont, charPixelSize);
		stbtt_GetFontVMetrics(&sgfont, &sgAscent, &sgDscent, 0);
		sgAscent = (int)(sgAscent * sgScale);
		sgDscent = (int)(sgDscent * sgScale);
	}

	int advance, lsb, x0, y0, x1, y1;
	stbtt_GetCodepointHMetrics(&sgfont, code, &advance, &lsb);
	stbtt_GetFontBoundingBox(&sgfont, &x0, &y0, &x1, &y1);
	//拿到文字的尺寸信息
	stbtt_GetCodepointBitmapBoxSubpixel(&sgfont, code, sgScale, sgScale, 0, 0, &x0, &y0, &x1, &y1);
	
	info.left = x0;
	info.top = -y0;
	info.width = x1 - x0;
	info.height = y1 - y0;
	info.base = -sgDscent;
	info.advX = advance * sgScale;//+sgScale * stbtt_GetCodepointKernAdvance(&font, text[ch], text[ch + 1])
	info.advY = 0;
	tempBuf.resize(info.width * info.height);
	std::vector<unsigned char> ttBuf;
	ttBuf.resize(tempBuf.size());
	stbtt_MakeCodepointBitmapSubpixel(&sgfont, ttBuf.data(), info.width, info.height, info.width,
		sgScale, sgScale, 0, 0, code);

	unsigned char* pStart = ttBuf.data();
	unsigned char* pEnd = tempBuf.data();
	//把字符点阵数据更新到纹理上。
	//由于opengl纹理坐标的原点是在左下角，上下颠倒一下图像
	for (int i = info.height - 1; i >= 0; --i) {
		memcpy(pEnd, pStart + (info.width * i), info.width);
		pEnd += info.width;
	}
	
	// note that this stomps the old data, so where character boxes overlap (e.g. 'lj') it's wrong
	// because this API is really for baking character bitmaps into textures. if you want to render
	// a sequence of characters, you really need to render each bitmap to a temp buffer, then
	// "alpha blend" that into the working buffer
	/*xpos += (advance * sgScale);
	if (text[ch + 1])
		xpos += sgScale * stbtt_GetCodepointKernAdvance(&font, text[ch], text[ch + 1]);
	++ch;*/
	
	return true;
}