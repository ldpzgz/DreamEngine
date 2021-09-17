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

	//ʹ��freetype��Ⱦ������Ȼ����µ������ϣ���¼�ַ��������ϵ�λ����Ϣ��
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
	//���ַ��������ݸ��µ������ϡ�
	//����opengl���������ԭ���������½ǣ����µߵ�һ��ͼ��
	for (int i = info.height - 1; i >= 0; --i) {
		memcpy(pEnd, pStart + (slot->bitmap.pitch * i), info.width);
		pEnd += info.width;
	}
}