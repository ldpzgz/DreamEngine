#pragma once

//#include <ft2build.h>
//#include FT_FREETYPE_H
//#include FT_GLYPH_H
//#include FT_OUTLINE_H 
#include <string>
#include <vector>
#include "CharInfo.h"


//bool getCharBitmap(UnicodeType code,int charPixelSize,CharInfo& info, std::vector<unsigned char>& tempBuf);
//void getCharOutline();

bool getCharBitmap2(UnicodeType code, int charPixelSize, CharInfo& info, std::vector<unsigned char>& tempBuf);
