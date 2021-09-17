#include "FreetypeUtils.h"
#include "../Log.h"
using namespace std;
static const string gFontFile("./opengles3/material/simfang.ttf");

static 	FT_Library  glibrary;
static  FT_Face     gface;      /* handle to face object */
static  bool gIsFreetypeInit = false;
using namespace std;
bool initFreetype(const string& ttfPath) {
	int error = 0;
	if (!gIsFreetypeInit)
	{

		error = FT_Init_FreeType(&glibrary);
		if (error)
		{
			LOGE("error to init freetype\n");
			return false;
		}

		error = FT_New_Face(glibrary,
			ttfPath.c_str(),
			0,
			&gface);
		if (error == FT_Err_Unknown_File_Format)
		{
			LOGE("error to FT_New_Face FT_Err_Unknown_File_Format \n");
			FT_Done_FreeType(glibrary);
			return false;
		}
		else if (error)
		{
			LOGE("error to FT_New_Face other \n");
			FT_Done_FreeType(glibrary);
			return false;
		}

		error = FT_Select_Charmap(gface, FT_ENCODING_UNICODE);
		if (error)
		{
			LOGE("error to FT_Select_Charmap \n");
			return false;
		}
	}
	gIsFreetypeInit = true;
	return true;
}

void releaseFreetype()
{
	if (gIsFreetypeInit)
	{
		FT_Done_Face(gface);
		FT_Done_FreeType(glibrary);
	}
}

void getCharBitmap(UnicodeType code, int charPixelSize, CharInfo& info, std::vector<unsigned char>& tempBuf) {
	if (!gIsFreetypeInit) {
		initFreetype(gFontFile);
	}

	//使用freetype渲染出来，然后更新到纹理上，记录字符在纹理上的位置信息。
	int error = 0;
	error = FT_Set_Pixel_Sizes(gface, charPixelSize, charPixelSize);
	if (error)
	{
		LOGE("error to FT_Set_Pixel_Sizes \n");
		return;
	}
	FT_Select_Charmap(gface, FT_ENCODING_UNICODE);
	error = FT_Load_Char(gface, code, FT_LOAD_RENDER);
	if (error)
	{
		LOGE("error to FT_Load_Char \n");
		return;
	}
	
	FT_GlyphSlot slot = gface->glyph;
	/*auto metrics = slot->metrics;*/
	info.left = slot->bitmap_left;
	info.top = slot->bitmap_top;
	info.width = slot->bitmap.width;
	info.height = slot->bitmap.rows;
	info.advX = slot->advance.x >> 6;
	info.advY = slot->advance.y >> 6;

	tempBuf.resize(info.width * info.height);
	unsigned char* pStart = slot->bitmap.buffer;
	unsigned char* pEnd = tempBuf.data();
	//把字符点阵数据更新到纹理上。
	//由于opengl纹理坐标的原点是在左下角，上下颠倒一下图像
	for (int i = info.height - 1; i >= 0; --i) {
		memcpy(pEnd, pStart + (slot->bitmap.pitch * i), info.width);
		pEnd += info.width;
	}
}