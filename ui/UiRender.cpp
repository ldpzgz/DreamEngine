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

const string CharSizeKey("charSize");		//一个字占用的宽高
static const std::string gFontTextureName("fontsTexture");
static const string gfontfile("./material/simfang.ttf");
static const string gmyfontfile("./material/myfont.data");

FT_Library  FontInfo::glibrary;
FT_Face     FontInfo::gface;      /* handle to face object */
bool FontInfo::gIsFreetypeInit = false;



FontInfo::FontInfo(const shared_ptr<Texture>& pTex, const shared_ptr<Material>& pMaterial, const string& savePath_, const string& ttfPath, int charSize_) :
	pCharTexture(pTex),
	mpMaterial(pMaterial),
	savePath(savePath_),
	curTextureHeight(0),
	charSize(charSize_)
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

shared_ptr<FontInfo> FontInfo::loadFromFile(const string& savePath, const string& ttfPath, const string& materialPath) {

	auto pMaterial = make_shared<Material>();

	if (!pMaterial->parseMaterialFile(materialPath)) {
		LOGE("error to parse font material");
		return shared_ptr<FontInfo>();
	}

	auto pTex = Material::getTexture(gFontTextureName);
	if (!pTex) {
		LOGE("font text %s not found in material %s",gFontTextureName.c_str(), materialPath.c_str());
		pMaterial.reset();
		return shared_ptr<FontInfo>();
	}

	auto charSize = pMaterial->getKeyAsInt(CharSizeKey);

	if (charSize < 0) {
		LOGE("not found charSize key in material %s", materialPath.c_str());
		pMaterial.reset();
		return shared_ptr<FontInfo>();
	}

	shared_ptr<FontInfo> fontInfo = make_shared<FontInfo>(pTex, pMaterial,savePath, ttfPath, charSize);

	fontInfo->mpCharMesh = make_shared<Mesh>(MeshType::MESH_FONTS);

	fontInfo->mpCharMesh->setMaterial(pMaterial);

	ifstream inFile(savePath, ios::in);
	if (inFile.is_open()) {
		//把字符信息取出来
		int countOfChar = 0;
		inFile >> countOfChar >> fontInfo->textureWidth >> fontInfo->textureHeight >> fontInfo->curTextureWidth >> fontInfo->curTextureHeight;
		if (fontInfo->textureWidth != fontInfo->pCharTexture->getWidth() ||
			fontInfo->textureHeight != fontInfo->pCharTexture->getHeight()) {
			LOGE("charTexture widht or height is not equal width the one defined in fonts Material file %s", materialPath.c_str());
			fontInfo.reset();
			return fontInfo;
		}
		for (int i = 0; i < countOfChar; ++i) {
			CharInTexture info;
			UnicodeType code;
			inFile.read((char*)&code, sizeof(UnicodeType));
			inFile >> info.x >> info.y >> info.left >> info.top >> info.width >> info.width >> info.advX >> info.advX;

			fontInfo->fontsMap.insert(make_pair(code, info));
		}
		
		//将文件中保存的渲染好的部分字符的图像读取出来
		int imageSize = fontInfo->textureWidth*fontInfo->curTextureHeight;
		std::vector<char> imageData;
		imageData.resize(imageSize);
		inFile.read(imageData.data(), imageSize);
		//更新纹理，把已经渲染好的字符图像上传到纹理上去
		fontInfo->pCharTexture->update(0, 0, fontInfo->textureHeight, fontInfo->curTextureHeight, imageData.data());
	}

	return fontInfo;
}

void FontInfo::getTextRenderInfo(const string& text, std::vector<CharRenderInfo>& renderInfoArray, const Rect<int>& rect){
	
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
		size_t countOfChar = fontsMap.size();
		out << countOfChar << textureWidth << textureHeight << curTextureWidth << curTextureHeight;
		for_each(fontsMap.cbegin(), fontsMap.cend(), [&out](const FontsMapValueType& info) {
			out.write((const char*)&info.first, sizeof(UnicodeType));
			out<< info.second.x << info.second.y << info.second.left << info.second.top << info.second.width << info.second.width << info.second.advX << info.second.advX;
		});
	}
	
}

const CharInTexture& FontInfo::getCharInTexture(UnicodeType code) {
	auto it = fontsMap.find(code);
	if (it != fontsMap.end()) {
		return it->second;
	}
	else {
		//使用freetype渲染出来，然后更新到纹理上，记录字符在纹理上的位置信息。
		int error = 0;
		error = FT_Set_Pixel_Sizes(gface, charSize, charSize);
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
		CharInTexture info;
		//int bitmapWidth = slot->bitmap.width;
		//int bitmapHeight = slot->bitmap.rows;
		info.left = slot->bitmap_left;
		info.top = charSize - slot->bitmap_top;//奇怪
		info.width = slot->bitmap.width;
		info.height = slot->bitmap.rows;
		info.advX = slot->advance.x >> 6;
		info.advY = slot->advance.y >> 6;
		if (curTextureHeight == 0) {
			curTextureHeight = charSize;
		}

		if (curTextureWidth + info.width > textureWidth) {//当前行写不下要换一行了
			curTextureWidth = 0;
			if (curTextureHeight + charSize > textureHeight)
			{
				//当前纹理已经写满了
				throw -1;
			}
			curTextureHeight += charSize;
		}

		info.x = curTextureWidth;
		info.y = curTextureHeight - charSize;
		
		//把字符点阵数据更新到纹理上。
		//由于opengl纹理坐标的原点是在左下角，上下颠倒一下图像
		std::vector<unsigned char> tempBuf;
		tempBuf.resize(info.width*info.height);
		unsigned char* pStart = slot->bitmap.buffer;
		unsigned char* pEnd = tempBuf.data();
		for (int i = info.height -1; i >=0; --i) {
			memcpy(pEnd, pStart + (slot->bitmap.pitch*i), info.width);
			pEnd += info.width;
		}
		pCharTexture->update(info.x, info.y, info.width, info.height, tempBuf.data());

		curTextureWidth += info.width;

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

void UiRender::initUiRender(const string& savedPath, const string& ttfPath, const string& materialPath) {
	pFontInfo = FontInfo::loadFromFile(savedPath, ttfPath, materialPath);
}

void UiRender::drawTextView(TextView* tv) {
	if (tv != nullptr && pFontInfo) {
		int originCharSize = pFontInfo->charSize;
		int tvCharSize = tv->getCharSize();
		auto& tvRect = tv->getRect();
		const auto& text = tv->getText();
		auto slen = text.size();
		unsigned char* pStr = (unsigned char*)text.c_str();

		//float currentPosX = tvRect.x; //下一个要渲染的文字的位置x
		//float currentPosY = tvRect.y + tvRect.height / 2.0f + tvCharSize / 2.0f;//下一个要渲染的文字的位置x
		//currentPosY = mWindowHeight - currentPosY;//ui的坐标系跟OpenGL的坐标系不一样，需要转换一下
		//计算出要渲染的文字的纹理信息与位置信息，从(0,0)这个位置开始排版文字，等会再根据textview的属性，translate一下。
		float currentPosX = 0.0f; //下一个要渲染的文字的基线位置x
		float currentPosY = -tvCharSize - tv->getLineSpacingInc();//下一个要渲染的文字的基线位置y
		float yAdvance = currentPosY;//考虑到textview设置的额外行间距，默认的行间距是freetype渲染文字时的CharSize
		float xExtraAdvance = tv->getCharSpacingInc();//考虑到textview设置的额外字符间距
		float maxWidth = tvRect.width;//textView的宽度，字符渲染不能超出这个宽度
		float maxHeight = tvRect.height;//textView的高度，字符渲染不能超出这个宽度
		float currentWidth = 0.0f;//当前已经渲染出去的字符的宽度，这个不能超出maxWidth
		int fontTextureWidth = pFontInfo->pCharTexture->getWidth();//保存字体位图的纹理的宽度
		int fontTextureHeight = pFontInfo->pCharTexture->getHeight();//保存字体位图的纹理的高度
		int tvMaxLines = tv->getMaxLines(); //textview设置的显示行数
		int currentLines = 1;			//
		int totalWidth = 0;				//字符串占用的宽度
		int totalHeight = currentPosY; //字符串占用的高度
		std::vector<CharRenderInfo> charsRenderInfoArray;
		for(size_t i = 0; i<slen;){
			UnicodeType code;
			auto len = UtfConvert::utf(pStr, code);
			pStr += len;
			i += len;
			try {
				const auto& cinfo = pFontInfo->getCharInTexture(code);
				if (currentPosX + cinfo.width>maxWidth) {//当前要渲染的文字会超出textview的边界
					if (currentLines >= tvMaxLines) {
						if (currentPosX >= maxWidth) {
							//渲染完毕
							break;
						}
					}
					else {
						//要换行了,增加一行
						currentPosX = 0;
						currentPosY += yAdvance;
						++currentLines;
						totalWidth = tvRect.width;
						totalHeight = currentPosY;
					}
				}

				//可以计算纹理矩阵了，mesh中的纹理坐标是(0,0)到(1,1)
				CharRenderInfo rInfo;
				glm::scale(rInfo.texMatrix, glm::vec2((float)cinfo.width/ (float)fontTextureWidth,
					(float)cinfo.height / (float)fontTextureHeight));
				glm::translate(rInfo.texMatrix, glm::vec2((float)cinfo.x / (float)fontTextureWidth, 
					(float)cinfo.y / (float)fontTextureHeight));
				//计算文字的model矩阵，渲染ui的时候使用的是正交投影，适配窗口的宽高
				//注意，ui坐标系统原点在左上角，y轴向下，与传统保持一直，openglY轴向上的
				glm::scale(rInfo.matrix, glm::vec3(cinfo.width, cinfo.height, 1.0f));
				glm::translate(rInfo.matrix, glm::vec3(currentPosX+cinfo.left, currentPosY+cinfo.top-cinfo.height, 0.0f));

				currentPosX += (cinfo.advX + xExtraAdvance);//考虑到textview设置的额外字符间距
				if (totalWidth < tvRect.width) {
					totalWidth = currentPosX;
				}
				charsRenderInfoArray.emplace_back(std::forward<CharRenderInfo>(rInfo));
			}
			catch (int error) {
				LOGE("getCharInfo error %d", error);
			}

		}
		//经过上面的处理，文字已经排版好了，但是整个文本的左上角在坐标原点(0,0)
		//需要根据textview的对齐属性（左对齐，右对齐，顶对齐，底对齐，水平居中，垂直居中）,结合textView的位置，确定平移向量，
		
		auto textAlignment = tv->getAligment();
		glm::vec3 moveVec(0.0f, 0.0f, 0.0f);
		if (textAlignment & TextAlignment::AlignCenter) {
			if (totalWidth > tvRect.width) {
				moveVec.x = tvRect.x;
			}
			else {
				moveVec.x = tvRect.x + (float)(tvRect.width- totalWidth)/2.0f;
			}

			if (totalHeight > tvRect.height) {
				moveVec.y = mWindowHeight - tvRect.y;
			}
			else {
				moveVec.y = mWindowHeight - tvRect.y - (float)(tvRect.height - totalHeight) / 2.0f;
			}
			
		}
		else if (textAlignment & TextAlignment::AlignHCenter) {

		}
		else if (textAlignment & TextAlignment::AlignVCenter) {

		}

		//根据对齐属性，调整每一个的位置,并且渲染
		glEnable(GL_SCISSOR_TEST);
		glScissor(tvRect.x, mWindowHeight-tvRect.y, tvRect.width, tvRect.height);
		for (auto it = charsRenderInfoArray.begin(); it != charsRenderInfoArray.end(); ++it) {
			glm::translate(it->matrix, moveVec);
			//绘制
			pFontInfo->mpCharMesh->render(mProjMatrix * it->matrix, it->texMatrix);
		}
		glDisable(GL_SCISSOR_TEST);
		//处理完毕
	}
}

void UiRender::drawButton(Button* bt){

}

shared_ptr<UiNode> UiNode::loadFromFile(const string& filepath) {
	shared_ptr<UiNode> pUiNode;
	return pUiNode;
}

UiNode::UiNode():
	Node()
{

}
UiNode::~UiNode() {

}