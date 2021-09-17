#ifndef _FREETYPE_UTILS_H_
#define _FREETYPE_UTILS_H_
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H 
#include<string>
#include<vector>
#include "CharInfo.h"


bool initFreetype(const std::string& ttfPath);
void releaseFreetype();

void getCharBitmap(UnicodeType code,int charPixelSize,CharInfo& info, std::vector<unsigned char>& tempBuf);
void getCharOutline();
#endif