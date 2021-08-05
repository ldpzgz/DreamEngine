#include "UiRender.h"
#include "../Log.h"
#include <fstream>
#include <vector>
#include <algorithm>
#include <cuchar>
#include "../Fbo.h"
#include "../Utils.h"
#include "Ui.h"
using namespace std;

const int FontTexWidth = 1024;
const int FontTexHeight = 1024;
const int CharSize = 128;		//一个字占用的宽高
static const std::string gFontTextureName("fontsTexture");
static const string gfontfile("./materialsimfang.ttf");
static const string gmyfontfile("./material/myfont.data");

FT_Library  FontInfo::glibrary;
FT_Face     FontInfo::gface;      /* handle to face object */
bool FontInfo::gIsFreetypeInit = false;



FontInfo::FontInfo(int texWidth, int texHeight, const string& savePath_, const string& ttfPath):
	textureWidth(texWidth),
	textureHeight(texHeight),
	savePath(savePath_),
	curTextureHeight(0)
{
	initFreetype(ttfPath);
}

FontInfo::~FontInfo()
{
	releaseFreetype();
}

bool FontInfo::initFreetype(const string& ttfPath) {
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

shared_ptr<FontInfo> FontInfo::loadFromFile(const string& savePath, const string& ttfPath, int imageWidth, int imageHeight) {

	shared_ptr<FontInfo> fontInfo = make_shared<FontInfo>(imageWidth,imageHeight, savePath, ttfPath);
	
	ifstream inFile(savePath, ios::in);
	if (inFile.is_open()) {
		int countOfChar = 0;
		inFile >> countOfChar >> fontInfo->textureWidth >> fontInfo->textureHeight >> fontInfo->curTextureWidth >> fontInfo->curTextureHeight;
		for (int i = 0; i < countOfChar; ++i) {
			CharInfo info;
			inFile.read((char*)&info.code, 2);
			inFile >> info.x >> info.y >> info.left >> info.top >> info.width >> info.width >> info.advX >> info.advX;

			fontInfo->fontsMap.insert(make_pair(info.code, info));
		}
		//创建一张textureWidth*textureHeight的纹理
		fontInfo->initCharTexture();
		//将文件中保存的渲染好的部分字符的图像读取出来
		int imageSize = fontInfo->textureWidth*fontInfo->curTextureHeight;
		std::vector<char> imageData;
		imageData.resize(imageSize);
		inFile.read(imageData.data(), imageSize);
		//更新纹理，把已经渲染好的字符图像上传到纹理上去
		fontInfo->pCharTexture->update(0, 0, fontInfo->textureHeight, fontInfo->curTextureHeight, imageData.data());
	}
	else {
		fontInfo->initCharTexture();
	}

	return fontInfo;
}

void FontInfo::rendText(const string& text) {
	auto slen = text.size();
	unsigned char* pStr = (unsigned char*)text.c_str();
	int i = 0;
	while(i<slen){
		char32_t code;
		auto len = UtfConvert::utf(pStr, code);
		pStr += len;
		i += len;
		const auto& cinfo = getCharInfo(code);
	}

}

bool FontInfo::initCharTexture() {
	pCharTexture = Material::createTexture(gFontTextureName, textureWidth, textureHeight, nullptr, GL_LUMINANCE);
	return pCharTexture ? true: false;
}

void FontInfo::releaseFreetype()
{
	if (gIsFreetypeInit)
	{
		FT_Done_Face(gface);
		FT_Done_FreeType(glibrary);
	}
}

void FontInfo::saveToFile() {
	if (!fontsMap.empty()) {
		Fbo fbo;
		fbo.attachColorTexture(pCharTexture, 0);
		fbo.startRender();
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		std::vector<unsigned char> imageData;
		imageData.resize(textureWidth*curTextureHeight);
			glReadPixels(0, 0, textureWidth, curTextureHeight, GL_LUMINANCE, GL_UNSIGNED_BYTE, (void*)imageData.data());
		fbo.endRender();

		ofstream out(savePath, ios::out | ios::binary);
		int countOfChar = fontsMap.size();
		out << countOfChar << textureWidth << textureHeight << curTextureWidth << curTextureHeight;
		for_each(fontsMap.cbegin(), fontsMap.cend(), [&out](const FontsMapValueType& info) {
			out.write((const char*)&info.first, sizeof(char16_t));
			out<< info.second.x << info.second.y << info.second.left << info.second.top << info.second.width << info.second.width << info.second.advX << info.second.advX;
		});
	}
	
}

const CharInfo& FontInfo::getCharInfo(char32_t code) {
	auto it = fontsMap.find(code);
	if (it != fontsMap.end()) {
		return it->second;
	}
	else {
		//使用freetype渲染出来，然后更新到纹理上，记录字符在纹理上的位置信息。
		int error = 0;
		error = FT_Set_Pixel_Sizes(gface, CharSize, CharSize);
		if (error)
		{
			LOGE("error to FT_Set_Pixel_Sizes \n");
			throw error;
		}
		FT_Select_Charmap(gface, FT_ENCODING_UNICODE);
		error = FT_Load_Char(gface, code, FT_LOAD_RENDER);
		if (error)
		{
			LOGE("error to FT_Load_Char \n");
			throw error;
		}

		FT_GlyphSlot slot = gface->glyph;
		CharInfo info;
		info.code = code;
		info.left = slot->bitmap_left;
		info.top = CharSize - slot->bitmap_top;//奇怪
		info.width = slot->bitmap.width;
		info.height = slot->bitmap.rows;
		info.advX = slot->advance.x >> 6;
		info.advY = slot->advance.y >> 6;
		if (curTextureHeight == 0) {
			curTextureHeight = CharSize;
		}

		if (curTextureWidth + info.width > textureWidth) {//当前行写不下要换一行了
			curTextureWidth = 0;
			if (curTextureHeight + CharSize > textureHeight)
			{
				//当前纹理已经写满了
				throw -1;
			}
			curTextureHeight += CharSize;
		}

		info.x = curTextureWidth;
		info.y = curTextureHeight - CharSize;
		curTextureWidth += info.width;
		//把字符点阵数据更新到纹理上。
		//由于opengl纹理坐标的原点是在左下角，上下颠倒一下图像
		std::vector<unsigned char> tempBuf;
		tempBuf.resize(info.width*info.height);
		unsigned char* pStart = slot->bitmap.buffer;
		unsigned char* pEnd = tempBuf.data();
		for (int i = info.height-1; i >=0; --i) {
			memcpy(pEnd, pStart + (slot->bitmap.pitch*i), info.width);
			pEnd += info.width;
		}
		pCharTexture->update(info.x, info.y, info.width, info.height, tempBuf.data());
		//slot->bitmap.buffer, info.width*info.height);
		auto& ret = fontsMap.try_emplace(code, info);
		if (ret.second)
		{
			return ret.first->second;
			LOGD("success to insert map \n");
		}
		else {
			throw - 2;
		}
	}
}

unique_ptr<UiRender> UiRender::gInstance = make_unique<UiRender>();

void UiRender::initUiRender() {
	pFontInfo = FontInfo::loadFromFile(gmyfontfile, gfontfile, FontTexWidth, FontTexHeight);
}

void UiRender::drawTextView(TextView* tv) {
	if (tv != nullptr) {
		/*
		1 在哪里绘制，确定矩形区域
		2 拿到要绘制的文字，一个字一个字的绘制，需要写一个绘制单个字的shader，这个shader有参数可以调整之后绘制其他文字。
		3 textview的各种属性，左对齐，右对齐，居中等，行数
		*/
	}
}

void UiRender::drawButton(Button* bt){

}