#include "UiRender.h"
#include "../Log.h"
#include <fstream>
#include <vector>
#include "../MeshRoundedRectangle.h"
#include "../MeshCircle.h"

using namespace std;

const string CharSizeKey("charSize");		//一个字占用的宽高
//static const string gUIRenderMaterialFile("./opengles3/material/uiDraw.material");
static const string gFontTextureName("fontsTexture");
static const string gFontFile("./opengles3/material/simfang.ttf");
static const string gSavedFontFile("./opengles3/material/myfont.data");
static const string gFontMaterialName("font");

FontManager::FontManager()
{
}

bool FontManager::initFontManager(const shared_ptr<Texture>& pTex, const shared_ptr<Material>& pMaterial, const string& savePath_, const string& ttfPath, int charSize_) {
	if (!pTex || !pMaterial) {
		LOGE("ERROR to initFontManager the texture or material is null");
		return false;
	}
	mpCharTexture = pTex;
	mpMaterial = pMaterial;
	mSavePath = savePath_;
	mCharSize = charSize_;
	
	if (pTex) {
		mTextureWidth = pTex->getWidth();
		mTextureHeight = pTex->getHeight();
	}

	mpCharMesh = make_shared<Mesh>(MeshType::MESH_Rectangle);
	if (mpCharMesh) {
		mpCharMesh->loadMesh();
		mpCharMesh->setMaterial(pMaterial);
	}

	/*mpTextGpuRender = make_shared<TextGpuRender>();
	if (mpTextGpuRender) {
		mpTextGpuRender->init();
		mpTextGpuRender->setRenderTexture(pTex);
	}*/

	ifstream inFile(mSavePath, ios::in);
	if (inFile.is_open()) {
		//把字符信息取出来
		int countOfChar = 0;
		inFile >> countOfChar >> mTextureWidth >> mTextureHeight >> mCurTextureWidth >> mCurTextureHeight;
		if (mTextureWidth != mpCharTexture->getWidth() || mTextureHeight != mpCharTexture->getHeight()) {
			LOGE("charTexture widht or height is not equal width the one defined in fonts Material file ");
			return false;
		}
		for (int i = 0; i < countOfChar; ++i) {
			CharInfo info;
			UnicodeType code;
			inFile.read((char*)&code, sizeof(UnicodeType));
			inFile >> info.x >> info.y >> info.left >> info.top >> info.width >> info.width >> info.advX >> info.advX;

			mFontsMap.insert(make_pair(code, info));
		}

		//将文件中保存的渲染好的部分字符的图像读取出来
		int imageSize = mTextureWidth * mCurTextureHeight;
		std::vector<char> imageData(imageSize,0);
		inFile.read(imageData.data(), imageSize);
		//更新纹理，把已经渲染好的字符图像上传到纹理上去
		mpCharTexture->update(0, 0, mTextureHeight, mCurTextureHeight, imageData.data());
		inFile.close();
	}
	return true;
}

FontManager::~FontManager()
{
}

shared_ptr<FontManager> FontManager::loadFromFile(const string& savePath, const string& ttfPath, const string& materialName) {

	auto& pMaterial = Material::getMaterial(materialName);

	if (!pMaterial) {
		LOGE("error to parse font material");
		return shared_ptr<FontManager>();
	}

	auto& pTex = Material::getTexture(gFontTextureName);
	if (!pTex) {
		LOGE("font texture %s not found in material %s",gFontTextureName.c_str(), materialName.c_str());
		pMaterial.reset();
		return shared_ptr<FontManager>();
	}

	auto charSize = pMaterial->getKeyAsInt(CharSizeKey);

	if (charSize < 0) {
		LOGE("not found charSize key in material %s", materialName.c_str());
		pMaterial.reset();
		return shared_ptr<FontManager>();
	}

	shared_ptr<FontManager> fontInfo = make_shared<FontManager>();
	fontInfo->initFontManager(pTex, pMaterial, savePath, ttfPath, charSize);
	return fontInfo;
}

void FontManager::saveToFile() {
	if (!mFontsMap.empty()) {
		Fbo fbo;
		fbo.attachColorTexture(mpCharTexture, 0);
		fbo.enable();
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		std::vector<unsigned char> imageData;
		imageData.resize((size_t)mTextureWidth * (size_t)mCurTextureHeight);
			glReadPixels(0, 0, mTextureWidth, mCurTextureHeight, GL_LUMINANCE, GL_UNSIGNED_BYTE, (void*)imageData.data());
		fbo.disable();

		ofstream out(mSavePath, ios::out | ios::binary);
		size_t countOfChar = mFontsMap.size();
		out << countOfChar << mTextureWidth << mTextureHeight << mCurTextureWidth << mCurTextureHeight;
		for (const auto& info : mFontsMap) {
			out.write((const char*)&info.first, sizeof(UnicodeType));
			out << info.second.x << info.second.y << info.second.left << info.second.top << info.second.width << info.second.width << info.second.advX << info.second.advX;
		}
		out.close();
	}	
}

//const CharInfo& FontManager::getCharIntexture2(UnicodeType code)
//{
//	auto it = mFontsMap.find(code);
//	if (it != mFontsMap.end()) {
//		return it->second;
//	}
//	else {
//		int error = 0;
//		error = FT_Set_Pixel_Sizes(gface, mCharSize, mCharSize);
//		if (error)
//		{
//			LOGE("error to FT_Set_Pixel_Sizes \n");
//			throw error;
//		}
//		FT_Select_Charmap(gface, FT_ENCODING_UNICODE);
//		auto glyphIndex = FT_Get_Char_Index(gface, code);
//		error = FT_Load_Glyph(gface, glyphIndex, FT_LOAD_DEFAULT);
//		if (gface->glyph->format == FT_GLYPH_FORMAT_OUTLINE) {
//			FT_Outline* pOutline = &gface->glyph->outline;
//			FT_Outline_Funcs callbacks;
//
//			callbacks.move_to = TextGpuRender::moveTo;
//			callbacks.line_to = TextGpuRender::lineTo;
//			callbacks.conic_to = TextGpuRender::conicTo;
//			callbacks.cubic_to = TextGpuRender::cubicTo;
//			callbacks.shift = 0;
//			callbacks.delta = 0;
//			printf("ExtractOutline \n");
//			FT_Error error = FT_Outline_Decompose(pOutline, &callbacks, pOutline);
//			if (error) {
//				printf("Couldn't extract the outline: FT_Outline_Decompose() failed");
//			}
//		}
//		else {
//			error = FT_Render_Glyph(gface->glyph, FT_RENDER_MODE_NORMAL);
//			if (error)
//			{
//				LOGE("error to FT_Load_Char \n");
//				throw error;
//			}
//			//FT_BBox bbox;
//			//FT_Glyph glyph;
//			//FT_Get_Glyph(gface->glyph, &glyph);                   //获取字形图像 的信息
//			//FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_TRUNCATE, &bbox);
//			FT_GlyphSlot slot = gface->glyph;
//
//			CharInfo info;
//			/*auto metrics = slot->metrics;*/
//			info.left = slot->bitmap_left;
//			info.top = slot->bitmap_top;
//			info.width = slot->bitmap.width;
//			info.height = slot->bitmap.rows;
//			info.advX = slot->advance.x >> 6;
//			info.advY = slot->advance.y >> 6;
//			if (mCurTextureHeight == 0) {
//				mCurTextureHeight = mCharSize;
//			}
//
//			if (mCurTextureWidth + info.width > mTextureWidth) {//当前行写不下要换一行了
//				mCurTextureWidth = 0;
//				if (mCurTextureHeight + mCharSize > mTextureHeight)
//				{
//					//当前纹理已经写满了
//					throw - 1;
//				}
//				mCurTextureHeight += mCharSize;
//			}
//
//			info.x = mCurTextureWidth;
//			info.y = mCurTextureHeight - mCharSize;
//
//			//把字符点阵数据更新到纹理上。
//			//由于opengl纹理坐标的原点是在左下角，上下颠倒一下图像
//			std::vector<unsigned char> tempBuf;
//			tempBuf.resize(info.width * info.height);
//			unsigned char* pStart = slot->bitmap.buffer;
//			unsigned char* pEnd = tempBuf.data();
//			for (int i = info.height - 1; i >= 0; --i) {
//				memcpy(pEnd, pStart + (slot->bitmap.pitch * i), info.width);
//				pEnd += info.width;
//			}
//			mpCharTexture->update(info.x, info.y, info.width, info.height, tempBuf.data());
//
//			mCurTextureWidth += info.width;
//
//			auto& ret = mFontsMap.try_emplace(code, info);
//			if (ret.second)
//			{
//				return ret.first->second;
//				LOGD("success to insert map \n");
//			}
//			else {
//				throw - 2;
//			}
//		}
//	}
//}

bool FontManager::getCharInTexture(UnicodeType code, CharInfo& info) {
	auto it = mFontsMap.find(code);
	if (it != mFontsMap.end()) {
		info = it->second;
		return true;
	}
	else {
		std::vector<unsigned char> tempBuf;

		if (getCharBitmap(code, mCharSize, info, tempBuf)) {
			if (mCurTextureHeight == 0) {
				mCurTextureHeight = mCharSize;
			}
			if (mCurTextureWidth + info.width > mTextureWidth) {//当前行写不下要换一行了
				mCurTextureWidth = 0;
				if (mCurTextureHeight + mCharSize > mTextureHeight)
				{
					//当前纹理已经写满了
					LOGE("ERROR the current char Texture is full");
					return false;
				}
				mCurTextureHeight += mCharSize;
			}
			info.x = mCurTextureWidth;
			info.y = mCurTextureHeight - mCharSize;

			mpCharTexture->update(info.x, info.y, info.width, info.height, tempBuf.data());

			mCurTextureWidth += info.width;

			auto& ret = mFontsMap.try_emplace(code, info);
			if (!ret.second)
			{
				LOGD("ERROR to insert code-charinfo pair \n");
			}
			return true;
		}
		else {
			LOGE("FreetypeUtils getCharBitmap error");
			return false;
		}
	}
}

unique_ptr<UiRender> UiRender::gInstance = make_unique<UiRender>();

void UiRender::initUiRender() {
	initTextView(gSavedFontFile, gFontFile, gFontMaterialName);
	mpLastMaterial = Material::getMaterial("posTextureMS");
	mpLastMesh = make_shared<Mesh>(MeshType::MESH_Rectangle);
	if (mpLastMesh) {
		mpLastMesh->loadMesh();
		mpLastMesh->setMaterial(mpLastMaterial);
	}
}

void UiRender::updateWidthHeight(float width, float height) {
	mWindowWidth = width;
	mWindowHeight = height;
	mProjMatrix = glm::ortho(0.0f, width, 0.0f, height,200.0f,-200.0f);
	mLastMeshModelMatrix = glm::identity<glm::mat4>();
	mLastMeshModelMatrix = glm::scale(mLastMeshModelMatrix, glm::vec3(width, height, 1.0f));
}

bool UiRender::initTextView(const string& savedPath, const string& ttfPath, const string& materialName) {
	mpFontManager = FontManager::loadFromFile(savedPath, ttfPath, materialName);
	if (mpFontManager) {
		return true;
	}
	else {
		return false;
	}
}

void UiRender::initBackground(View* pView) {
	if (pView) {
		auto& pBack = pView->getBackground();
		if (!pBack) {
			return;
		}
		if (!pBack->mpShape) {
			return;
		}
		auto& pShape = pBack->mpShape;
		auto& rect = pView->getRect();
		auto width = rect.width;
		auto height = rect.height;
		auto paddingLeft = pShape->getPaddingLeft();
		auto paddingRight = pShape->getPaddingRight();
		auto paddingTop = pShape->getPaddingTop();
		auto paddingBottom = pShape->getPaddingBottom();
		auto padding = pShape->getPadding();
		if (padding > 0) {
			paddingLeft = paddingRight = paddingTop = paddingBottom = padding;
		}
		width -= (paddingLeft + paddingRight);
		height -= (paddingTop+ paddingBottom);
		auto shapeType = pShape->getType();
		auto& solidColor = pShape->getSolidColor();
		GradientType gradientType = pShape->getGradientType();
		bool hasGradient = (gradientType != GradientType::None);
		auto cornerRadius = pShape->getCornerRadius();
		auto rtRadius = pShape->getCornerRightTopRadius();
		auto ltRadius = pShape->getCornerLeftTopRadius();
		auto lbRadius = pShape->getCornerLeftBottomRadius();
		auto rbRadius = pShape->getCornerRightBottomRadius();
		auto ovalWidth = pShape->getOvalWidth();
		auto ovalHeight = pShape->getOvalHeight();
		auto centerX = pShape->getGradientCenterX();
		auto centerY = pShape->getGradientCenterY();
		centerX *= width;
		centerY *= height;
		auto gradientAngle = pShape->getGradientAngle();
		auto& startColor = pShape->getGradientStartColor();
		auto& endColor = pShape->getGradientEndColor();
		auto& centerColor = pShape->getGradientCenterColor();
		auto strokeWidth = pShape->getSrokeWidth();
		auto& strokeColor = pShape->getSrokeColor();
		auto& pTexture = pBack->mpTexture;

		bool hasBackground = (!solidColor.isZero() || pTexture || gradientType != GradientType::None);
		shared_ptr<MeshFilledRect> pMesh;
		shared_ptr<MeshFilledRect> pStrokeMesh;
		if (shapeType == ShapeType::Rectangle) {
			if(hasBackground){
				pMesh = make_shared<MeshFilledRect>();
				pMesh->loadMesh(width, height, centerX, centerY);
				//pShape->setMesh(static_pointer_cast<void>(pMesh));
				pBack->mpMesh = pMesh;
			}
			if (!strokeColor.isZero()) {
				pStrokeMesh = make_shared<MeshFilledRect>();
				pStrokeMesh->loadMesh(width, height, centerX, centerY);
				pStrokeMesh->setFilled(false);
				pStrokeMesh->setLineWidth(strokeWidth);
				//pShape->setStrokeMesh(static_pointer_cast<void>(pStrokeMesh));
				pBack->mpStrokeMesh = pStrokeMesh;
			}
		}
		else if (shapeType == ShapeType::RoundedRectangle) {
			if (hasBackground) {
				pMesh = make_shared<MeshRoundedRectangle>();
				//pShape->setMesh(static_pointer_cast<void>(pMesh));
				pBack->mpMesh = pMesh;
				if (cornerRadius > 0) {
					//四个圆角是一样的半径
					pMesh->loadMesh(cornerRadius, centerX, centerY, width, height);
				}
				else {
					//四个圆角半径不一样
					pMesh->loadMesh(rtRadius, ltRadius, lbRadius, rbRadius, centerX, centerY, width, height);
				}
			}
			if (!strokeColor.isZero()) {
				pStrokeMesh = make_shared<MeshRoundedRectangle>();
				if (cornerRadius > 0) {
					//四个圆角是一样的半径
					pMesh->loadMesh(cornerRadius, width, height, centerX, centerY);
				}
				else {
					//四个圆角半径不一样
					pMesh->loadMesh(rtRadius, ltRadius, lbRadius, rbRadius, centerX, centerY, width, height);
				}
				pStrokeMesh->setFilled(false);
				pStrokeMesh->setLineWidth(strokeWidth);
				//pShape->setStrokeMesh(static_pointer_cast<void>(pStrokeMesh));
				pBack->mpStrokeMesh = pStrokeMesh;
			}
		}
		else if (shapeType == ShapeType::Oval) {
			if (hasBackground) {
				pMesh = make_shared<MeshCircle>();
				pMesh->loadMesh(width, height, centerX, centerY);
				//pShape->setMesh(static_pointer_cast<void>(pMesh));
				pBack->mpMesh = pMesh;
			}
			if (!strokeColor.isZero()) {
				pStrokeMesh = make_shared<MeshCircle>();
				pStrokeMesh->loadMesh(width, height, centerX, centerY);
				pStrokeMesh->setFilled(false);
				pStrokeMesh->setLineWidth(strokeWidth);
				//pShape->setStrokeMesh(static_pointer_cast<void>(pStrokeMesh));
				pBack->mpStrokeMesh = pStrokeMesh;
			}
		}

		if (pMesh && pTexture) {
			auto pMaterial = Material::clone("posTexture");
			if (pMaterial) {
				//pMaterial->setTextureForSampler("s_texture", pTexture);//这个每次渲染前都需要调用
				pMesh->setMaterial(pMaterial);
			}
			else {
				LOGE("ERROR cannot found posTexture material");
			}
		}
		else if (pMesh && hasGradient) {
			if (gradientType == GradientType::Linear) {
				auto pMaterial = Material::clone("posColor");
				if (pMaterial) {
					pMesh->setColorData(gradientAngle, startColor, endColor, centerColor);
					pMesh->setMaterial(pMaterial);
				}
			}
			else {
				LOGD("warning only support linear gradient type now");
			}
		}
		else if (pMesh && !solidColor.isZero()) {
			auto pMaterial = Material::clone("posUniformColor");
			if (pMaterial) {
				pMaterial->setUniformColor(solidColor);//这个每次渲染前都需要调用
				pMesh->setMaterial(pMaterial);
			}
			else {
				LOGE("ERROR cannot found posUniformColor material");
			}
		}

		if (pStrokeMesh) {
			auto pMaterial = Material::clone("posUniformColor");
			if (pMaterial) {
				pMaterial->setUniformColor(strokeColor);//这个每次渲染前都需要调用
				pStrokeMesh->setMaterial(pMaterial);
			}
			else {
				LOGE("ERROR cannot found posUniformColor material");
			}
		}
	}
}

//宽度或者高度为wrapContent的时候，计算TextView文本的宽度和高度以像素为单位
void UiRender::calcTextViewWidthHeight(TextView* tv) {
	if (tv->mLayoutWidth != LayoutParam::WrapContent && tv->mLayoutHeight != LayoutParam::WrapContent) {
		LOGE("ERROR no need to call UiRender::calcTextViewWidthHeight");
		return;
	}
	auto& tvRect = tv->getRect();
	const auto& text = tv->getText();
	unsigned char* pStr = (unsigned char*)text.c_str();
	int textSize = tv->getTextSize();//字的点阵大小，像素为单位
	auto slen = text.size();
	int width = 0;
	int height = 0;
	int currentPosX = 0.0f; //下一个要渲染的文字的基线位置x
	int lineSpace = tv->getLineSpacingInc();
	int currentPosY = -textSize - lineSpace;//下一个要渲染的文字的基线位置y
	int yAdvance = currentPosY;//考虑到textview设置的额外行间距，默认的行间距是freetype渲染文字时的CharSize
	int charSpace = tv->getCharSpacingInc();//考虑到textview设置的额外字符间距
	
	int maxWidth = tv->getMaxWidth();//textView的最大宽度，字符渲染不能超出这个宽度
	int maxHeight = tv->getMaxHeight();//textView的最大高度，字符渲染不能超出这个宽度
	if (tvRect.width > 0) {
		maxWidth = tvRect.width;//如果宽度已经指定了，更新最大宽度为指定宽度
	}
	if (tvRect.height > 0) {
		maxHeight = tvRect.height;//如果高度已经指定了，更新最大高度为指定高度
	}
	int tvMaxLines = tv->getMaxLines(); //textview设置的显示行数
	int currentLines = 1;			//
	int totalWidth = 0;				//记录字符串占用的总宽度
	int totalHeight = -currentPosY; //字符串占用的高度
	float scaleFactor = (float)textSize / (float)mpFontManager->mCharSize;
	std::vector<CharPosition> charsRenderInfoArray;
	for (size_t i = 0; i<slen;) {
		UnicodeType code;
		auto len = UtfConvert::utf(pStr, code);
		if (len == 0) {
			LOGE("ERROR in UiRender::calcTextViewWidthHeight convert utf8 to unicode,zhe input text is not utf8 encoded ");
			break;
		}
		pStr += len;
		i += len;
		
		CharInfo cinfo;
		if (!mpFontManager->getCharInTexture(code, cinfo)) {
			continue;
		}
		//判断宽度高度是否已经超出，是否要退出
		if (currentPosX + scaleFactor*cinfo.width>maxWidth) {
			//当前行满了，可能要换行或者结束
			if (currentLines >= tvMaxLines) {
				if (currentPosX >= maxWidth) {
					//已经达到最大行了，并且当前的最后一个字符已经达到或者超出了最大宽度，渲染完毕
					//允许最后一个字被截掉了一点。
					break;
				}
			}
			else {
				//要换行了,判断高度是否超出边界
				if (-currentPosY- yAdvance > maxHeight) {
					break;
				}
				currentPosX = 0;
				currentPosY += yAdvance;
				//maxYAdv = 0;
				++currentLines;
				totalWidth = maxWidth;
				totalHeight = -currentPosY;
			}
		}
		//没有超出边界，继续
		currentPosX += (scaleFactor*cinfo.advX + charSpace);//考虑到textview设置的额外字符间距
		if (totalWidth < maxWidth) {
			totalWidth = currentPosX - charSpace;
		}
	}

	if (tv->mLayoutWidth == LayoutParam::WrapContent) {
		tvRect.width = totalWidth + tv->mLayoutMarginLeft + tv->mLayoutMarginRight;
	}
	if (tv->mLayoutHeight == LayoutParam::WrapContent) {
		tvRect.height = totalHeight + tv->mLayoutMarginTop + tv->mLayoutMarginBottom;
	}
}

/*
* 根据textview的各种属性，计算出textview里面每个文字的大小位置
* 最终得到每个文字的方位矩阵，纹理矩阵。
* 这个函数只计算textview控件能够显示得出来的那些文字
*/
void UiRender::calcTextPosition(TextView* tv) {
	const auto& text = tv->getText();
	if (text.empty()) {
		return;
	}
	if (!mpFontManager) {
		LOGE("ERROR NO fontInfo");
		return;
	}
	int originCharSize = mpFontManager->mCharSize;
	int textSize = tv->getTextSize();//字的点阵大小，像素为单位
	auto& tvRect = tv->getRect();
	auto slen = text.size();
	unsigned char* pStr = (unsigned char*)text.c_str();

	//float currentPosX = tvRect.x; //下一个要渲染的文字的位置x
	//float currentPosY = tvRect.y + tvRect.height / 2.0f + tvCharSize / 2.0f;//下一个要渲染的文字的位置x
	//currentPosY = mWindowHeight - currentPosY;//ui的坐标系跟OpenGL的坐标系不一样，需要转换一下
	//计算出要渲染的文字的纹理信息与位置信息，从(0,0)这个位置开始排版文字，等会再根据textview的属性，translate一下。
	float currentPosX = 0.0f; //下一个要渲染的文字的基线位置x
	float lineSpace = tv->getLineSpacingInc();
	float currentPosY = -textSize - lineSpace;//下一个要渲染的文字的基线位置y
	float yAdvance = currentPosY;//考虑到textview设置的额外行间距，默认的行间距是freetype渲染文字时的CharSize
	float xExtraAdvance = tv->getCharSpacingInc();//考虑到textview设置的额外字符间距
	float maxWidth = tvRect.width;//textView的宽度，字符渲染不能超出这个宽度
	float maxHeight = tvRect.height;//textView的高度，字符渲染不能超出这个宽度
	float currentWidth = 0.0f;//当前已经渲染出去的字符的宽度，这个不能超出maxWidth
	int fontTextureWidth = mpFontManager->mpCharTexture->getWidth();//保存字体位图的纹理的宽度
	int fontTextureHeight = mpFontManager->mpCharTexture->getHeight();//保存字体位图的纹理的高度
	int tvMaxLines = tv->getMaxLines(); //textview设置的显示行数
	int currentLines = 1;			//
	int totalWidth = 0;				//字符串占用的宽度
	int totalHeight = -currentPosY; //字符串占用的高度
	float scaleFactor = (float)textSize / (float)mpFontManager->mCharSize;
	std::vector<CharPosition>& textPositions = tv->getCharPositionArray();
	textPositions.clear();

	/*
	* 拿到每个字，在存储字符的纹理里面的位置。
	* 计算出textview的方框内能容下的所有字符的宽度高度。
	*/
	for (size_t i = 0; i < slen;) {
		UnicodeType code;
		auto len = UtfConvert::utf(pStr, code);
		pStr += len;
		i += len;
		
		CharInfo cinfo;
		if (!mpFontManager->getCharInTexture(code, cinfo)) {
			continue;
		}
		if (currentPosX + scaleFactor * cinfo.width > maxWidth) {//当前要渲染的文字会超出textview的边界
			if (currentLines >= tvMaxLines) {
				if (currentPosX >= maxWidth) {
					//渲染完毕
					break;
				}
			}
			else {
				//要换行了,判断高度是否超出边界
				if (-currentPosY - yAdvance > maxHeight) {
					break;
				}
				//要换行了,增加一行
				currentPosX = 0;
				currentPosY += yAdvance;
				//maxYAdv = 0;
				++currentLines;
				totalWidth = tvRect.width;
				totalHeight = -currentPosY;
			}
		}

		auto maxYAdvTemp = scaleFactor * (cinfo.height - cinfo.top);

		//可以计算纹理矩阵了，mesh中的纹理坐标是(0,0)到(1,1)
		CharPosition rInfo;

		rInfo.texMatrix = glm::translate(rInfo.texMatrix, glm::vec3((float)cinfo.x / (float)fontTextureWidth,
			(float)cinfo.y / (float)fontTextureHeight, 0.0f));
		rInfo.texMatrix = glm::scale(rInfo.texMatrix, glm::vec3((float)cinfo.width / (float)fontTextureWidth,
			(float)cinfo.height / (float)fontTextureHeight, 1.0f));

		//计算文字的model矩阵，渲染ui的时候使用的是正交投影，适配窗口的宽高
		//注意，ui坐标系统原点在左上角，y轴向下，与传统保持一直，openglY轴向上的
		rInfo.matrix = glm::translate(rInfo.matrix, glm::vec3(currentPosX + scaleFactor * cinfo.left, currentPosY + lineSpace + 5 + scaleFactor * (cinfo.top - cinfo.height), 0.0f));
		rInfo.matrix = glm::scale(rInfo.matrix, glm::vec3(scaleFactor * cinfo.width, scaleFactor * cinfo.height, 1.0f));

		currentPosX += (scaleFactor * cinfo.advX + xExtraAdvance);//考虑到textview设置的额外字符间距
		if (totalWidth < tvRect.width) {
			totalWidth = currentPosX;
		}
		textPositions.emplace_back(rInfo);
		
	}
	//经过上面的处理，文字的大小位置已经计算好了，但是整个文本的左上角在坐标原点(0,0)
	//需要根据textview的对齐属性（左对齐，右对齐，顶对齐，底对齐，水平居中，垂直居中）,结合textView的位置，确定平移向量，
	auto textGrivate = tv->getGravity();
	glm::vec3 moveVec(0.0f, 0.0f, 0.0f);
	if (textGrivate == LayoutParam::Center) {
		if (totalWidth >= tvRect.width) {
			moveVec.x = tvRect.x;
		}
		else {
			moveVec.x = tvRect.x + (float)(tvRect.width - totalWidth) / 2.0f;
		}

		if (totalHeight >= tvRect.height) {
			moveVec.y = mWindowHeight - tvRect.y;
		}
		else {
			moveVec.y = mWindowHeight - tvRect.y - (float)(tvRect.height - totalHeight) / 2.0f;
		}

	}
	else if (textGrivate == LayoutParam::TopCenter) {
		if (totalWidth >= tvRect.width) {
			moveVec.x = tvRect.x;
		}
		else {
			moveVec.x = tvRect.x + (float)(tvRect.width - totalWidth) / 2.0f;
		}
		moveVec.y = mWindowHeight - tvRect.y;
	}
	else if (textGrivate == LayoutParam::LeftCenter) {
		moveVec.x = tvRect.x;

		if (totalHeight >= tvRect.height) {
			moveVec.y = mWindowHeight - tvRect.y;
		}
		else {
			moveVec.y = mWindowHeight - tvRect.y - (float)(tvRect.height - totalHeight) / 2.0f;
		}
	}
	else if (textGrivate == LayoutParam::BottomCenter) {
		if (totalWidth >= tvRect.width) {
			moveVec.x = tvRect.x;
		}
		else {
			moveVec.x = tvRect.x + (float)(tvRect.width - totalWidth) / 2.0f;
		}

		if (totalHeight >= tvRect.height) {
			moveVec.y = mWindowHeight - tvRect.y;
		}
		else {
			moveVec.y = mWindowHeight - tvRect.y - (float)(tvRect.height - totalHeight);
		}
	}
	else if (textGrivate == LayoutParam::RightCenter) {
		if (totalWidth >= tvRect.width) {
			moveVec.x = tvRect.x;
		}
		else {
			moveVec.x = tvRect.x + (float)(tvRect.width - totalWidth);
		}

		if (totalHeight >= tvRect.height) {
			moveVec.y = mWindowHeight - tvRect.y;
		}
		else {
			moveVec.y = mWindowHeight - tvRect.y - (float)(tvRect.height - totalHeight) / 2.0f;
		}
	}
	else if (textGrivate == LayoutParam::LeftTop) {
		moveVec.x = tvRect.x;
		moveVec.y = mWindowHeight - tvRect.y;
	}
	else if (textGrivate == LayoutParam::LeftBottom) {
		moveVec.x = tvRect.x;
		if (totalHeight >= tvRect.height) {
			moveVec.y = mWindowHeight - tvRect.y;
		}
		else {
			moveVec.y = mWindowHeight - tvRect.y - (float)(tvRect.height - totalHeight);
		}
	}
	else if (textGrivate == LayoutParam::RightBottom) {
		if (totalWidth >= tvRect.width) {
			moveVec.x = tvRect.x;
		}
		else {
			moveVec.x = tvRect.x + (float)(tvRect.width - totalWidth);
		}
		if (totalHeight >= tvRect.height) {
			moveVec.y = mWindowHeight - tvRect.y;
		}
		else {
			moveVec.y = mWindowHeight - tvRect.y - (float)(tvRect.height - totalHeight);
		}
	}
	else if (textGrivate == LayoutParam::RightTop) {
		if (totalWidth >= tvRect.width) {
			moveVec.x = tvRect.x;
		}
		else {
			moveVec.x = tvRect.x + (float)(tvRect.width - totalWidth);
		}
		moveVec.y = mWindowHeight - tvRect.y;
	}

	glm::mat4 moveToScreenMatrix(1.0f);
	moveToScreenMatrix = glm::translate(moveToScreenMatrix, moveVec);

	for (auto& info : textPositions) {
		info.matrix = moveToScreenMatrix * info.matrix;
	}
}
extern void checkglerror();
void UiRender::drawTextView(TextView* tv) {
	if (tv != nullptr) {
		drawBackground(tv);
		if (tv->getUpdateTextPosition()) {
			calcTextPosition(tv);
			tv->setUpdateTextPosition(false);
		}
		auto tvRect = tv->getRect();
		auto moveVec = tv->getTranslateVector();//view可能会被其他控件平移过，比如scrollview
		moveVec += tv->getMoveVector();
		tvRect.translate(moveVec);
		auto& textPositions = tv->getCharPositionArray();

		//根据对齐属性，调整每一个的位置,并且渲染
		GLboolean bScissorTest;
		glGetBooleanv(GL_SCISSOR_TEST, &bScissorTest);
		if (!bScissorTest) {
			glEnable(GL_SCISSOR_TEST);
			glScissor(tvRect.x, mWindowHeight-tvRect.y- tvRect.height, tvRect.width, tvRect.height);
		}
		float trx = 0.0f;
		for (auto& charPos : textPositions) {
			//绘制
			if (mpFontManager->mpMaterial) {
				mpFontManager->mpMaterial->setUniformColor(tv->getTextColor());
			}
			if (mpFontManager->mpCharMesh) {
				glm::mat4 tempMat(1.0f);
				tempMat = glm::translate(tempMat, glm::vec3(moveVec.x, -moveVec.y, 0.0f));
				tempMat = tempMat * charPos.matrix;
				mpFontManager->mpCharMesh->render(mProjMatrix * tempMat, charPos.texMatrix);
			}
		}
		if (!bScissorTest) {
			glDisable(GL_SCISSOR_TEST);
		}
		//处理完毕
	}
}

bool UiRender::drawBackground(View* v){
	//绘制view的shape
	if (v!=nullptr)
	{
		auto& pBack = v->getBackground();
		if (!pBack) {
			return false;
		}
		auto& pShape = pBack->mpShape;
		auto& pBackMesh = pBack->mpMesh;
		auto& pBackStrokeMesh = pBack->mpStrokeMesh;
		if (pShape) {
			auto paddingLeft = pShape->getPaddingLeft();
			auto paddingRight = pShape->getPaddingRight();
			auto paddingTop = pShape->getPaddingTop();
			auto paddingBottom = pShape->getPaddingBottom();
			auto padding = pShape->getPadding();
			if (padding > 0) {
				paddingLeft = paddingRight = paddingTop = paddingBottom = padding;
			}

			auto rect = v->getRect();
			auto moveVec = v->getTranslateVector();//view可能会被其他控件平移过，比如scrollview
			rect.translate(moveVec);
			rect.translate(v->getMoveVector());
			glm::mat4 model(1.0f);
			model = glm::translate(model, glm::vec3(rect.x + paddingLeft, 
				(mWindowHeight - rect.y - rect.height + paddingBottom), 0.0f));
			/*model = glm::scale(model, 
				glm::vec3(rect.width-(paddingLeft+ paddingRight), rect.height-(paddingTop+ paddingBottom), 1.0f));*/

			if (pBackMesh) {
				auto& pTexture = pBack->mpTexture;
				auto& pMat = pBackMesh->getMaterial();
				if (pMat) {
					pMat->setUniformColor(pShape->getSolidColor());
					if (pTexture) {
						pMat->setTextureForSampler("s_texture", pTexture);
					}
				}
				pBackMesh->render(mProjMatrix * model);
				
				return true;
			}
			
			if (pBackStrokeMesh) {
				auto& pMat = pBackStrokeMesh->getMaterial();
				if (pMat) {
					pMat->setUniformColor(pShape->getSrokeColor());
				}
				pBackStrokeMesh->render(mProjMatrix * model);
			}
		}
	}
	return true;
}

void UiRender::drawLinearLayout(LinearLayout* pll) {
	drawBackground(pll);
	auto& children = pll->getChildren();
	auto& moveVec = pll->getMoveVector();
	for (auto& pChild : children) {
		if (pChild) {
			pChild->setMove(moveVec);
			pChild->draw();
		}
	}
}

void UiRender::drawScrollView(ScrollView* psv) {
	drawBackground(psv);
	auto rect = psv->getRect();
	auto& children = psv->getChildren();
	bool hasDrawAChild = false;

	GLboolean bScissorTest;
	glGetBooleanv(GL_SCISSOR_TEST, &bScissorTest);
	Rect<int> preScissorBox;
	glGetIntegerv(GL_SCISSOR_BOX, (GLint*)&preScissorBox);
	glEnable(GL_SCISSOR_TEST);
	glScissor(rect.x, mWindowHeight - rect.y - rect.height, rect.width, rect.height);
	auto& moveVec = psv->getMoveVector();
	for (auto& pChild : children) {
		if (pChild) {
			pChild->setMove(moveVec);
			auto childRect = pChild->getRect();
			auto childMove = pChild->getTranslateVector();
			childRect.translate(childMove);
			if (childRect.intersect(rect)) {
				pChild->draw();
				hasDrawAChild = true;
			}
			else if (hasDrawAChild) {
				//后面的肯定不可见了
				break;
			}
		}
	}
	if (!bScissorTest) {
		glDisable(GL_SCISSOR_TEST);
	}
	else {
		glScissor(preScissorBox.x, preScissorBox.y, preScissorBox.width, preScissorBox.height);
	}
}

void UiRender::drawListView(ListView* plv) {
	drawBackground(plv);
	auto& lvRect = plv->getRect();

	GLboolean bScissorTest;
	glGetBooleanv(GL_SCISSOR_TEST, &bScissorTest);
	Rect<int> preScissorBox;
	glGetIntegerv(GL_SCISSOR_BOX, (GLint*) &preScissorBox);
	glEnable(GL_SCISSOR_TEST);
	glScissor(lvRect.x, mWindowHeight - lvRect.y - lvRect.height, lvRect.width, lvRect.height);
	
	auto& pAdapter = plv->getAdapter();
	if (pAdapter) {
		int firstItem;
		int firstItemHideLength;
		int lastItem;
		int lastItemHideLength;
		plv->getFirstVisibleItem(firstItem, firstItemHideLength);
		plv->getLastVisibleItem(lastItem, lastItemHideLength);
		bool isHorizontal = plv->isHorizontal();
		
		int moveLength = -firstItemHideLength;
		
		for (int i = firstItem; i <= lastItem; ++i) {
			auto& pView = pAdapter->getView(i);
			if (pView) {
				Vec2i tempMove(lvRect.x, lvRect.y);
				auto& rectInc = pView->getRect();
				if (isHorizontal) {
					tempMove.x += moveLength;
					moveLength += pView->advanceX();
				}
				else {
					tempMove.y += moveLength;
					moveLength += pView->advanceY();
				}
				pView->setMove(tempMove);
				pView->draw();
			}
		}
	}
	if (!bScissorTest) {
		glDisable(GL_SCISSOR_TEST);
	}
	else {
		glScissor(preScissorBox.x, preScissorBox.y, preScissorBox.width, preScissorBox.height);
	}
}

void UiRender::drawUi() {
	if (mpLastMesh) {
		//todo关闭深度测试，等等
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquation(GL_FUNC_ADD);
		mpLastMesh->render(mProjMatrix*mLastMeshModelMatrix);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
	}
}