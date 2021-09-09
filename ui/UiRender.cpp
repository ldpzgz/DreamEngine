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
	if (pTex) {
		textureWidth = pTex->getWidth();
		textureHeight = pTex->getHeight();
	}
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

shared_ptr<FontInfo> FontInfo::loadFromFile(const string& savePath, const string& ttfPath, const string& materialName) {

	auto& pMaterial = Material::getMaterial(materialName);

	if (!pMaterial) {
		LOGE("error to parse font material");
		return shared_ptr<FontInfo>();
	}

	auto& pTex = Material::getTexture(gFontTextureName);
	if (!pTex) {
		LOGE("font texture %s not found in material %s",gFontTextureName.c_str(), materialName.c_str());
		pMaterial.reset();
		return shared_ptr<FontInfo>();
	}

	auto charSize = pMaterial->getKeyAsInt(CharSizeKey);

	if (charSize < 0) {
		LOGE("not found charSize key in material %s", materialName.c_str());
		pMaterial.reset();
		return shared_ptr<FontInfo>();
	}

	shared_ptr<FontInfo> fontInfo = make_shared<FontInfo>(pTex, pMaterial,savePath, ttfPath, charSize);

	fontInfo->mpCharMesh = make_shared<Mesh>(MeshType::MESH_FONTS);
	if (fontInfo->mpCharMesh) {
		fontInfo->mpCharMesh->loadMesh();
		fontInfo->mpCharMesh->setMaterial(pMaterial);
	}

	ifstream inFile(savePath, ios::in);
	if (inFile.is_open()) {
		//把字符信息取出来
		int countOfChar = 0;
		inFile >> countOfChar >> fontInfo->textureWidth >> fontInfo->textureHeight >> fontInfo->curTextureWidth >> fontInfo->curTextureHeight;
		if (fontInfo->textureWidth != fontInfo->pCharTexture->getWidth() ||
			fontInfo->textureHeight != fontInfo->pCharTexture->getHeight()) {
			LOGE("charTexture widht or height is not equal width the one defined in fonts Material file %s", materialName.c_str());
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
		inFile.close();
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
		fbo.enable();
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		std::vector<unsigned char> imageData;
		imageData.resize(textureWidth*curTextureHeight);
			glReadPixels(0, 0, textureWidth, curTextureHeight, GL_LUMINANCE, GL_UNSIGNED_BYTE, (void*)imageData.data());
		fbo.disable();

		ofstream out(savePath, ios::out | ios::binary);
		size_t countOfChar = fontsMap.size();
		out << countOfChar << textureWidth << textureHeight << curTextureWidth << curTextureHeight;
		for (const auto& info : fontsMap) {
			out.write((const char*)&info.first, sizeof(UnicodeType));
			out << info.second.x << info.second.y << info.second.left << info.second.top << info.second.width << info.second.width << info.second.advX << info.second.advX;
		}
		out.close();
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
		//FT_BBox bbox;
		//FT_Glyph glyph;
		//FT_Get_Glyph(gface->glyph, &glyph);                   //获取字形图像 的信息
		//FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_TRUNCATE, &bbox);
		FT_GlyphSlot slot = gface->glyph;
		
		CharInTexture info;
		/*auto metrics = slot->metrics;*/
		info.left = slot->bitmap_left;
		info.top = slot->bitmap_top;
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

void UiRender::initUiRender() {
	initTextView(gSavedFontFile, gFontFile, gFontMaterialName);
	mpLastMaterial = Material::getMaterial("posTextureMS");
	mpLastMesh = make_shared<Mesh>(MeshType::MESH_FONTS);
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
	mpFontInfo = FontInfo::loadFromFile(savedPath, ttfPath, materialName);
	if (mpFontInfo) {
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
			auto& pMaterial = Material::getMaterial("posTexture");
			if (pMaterial) {
				//pMaterial->changeTexture("s_texture", pTexture);//这个每次渲染前都需要调用
				pMesh->setMaterial(pMaterial);
			}
			else {
				LOGE("ERROR cannot found posTexture material");
			}
		}
		else if (pMesh && hasGradient) {
			if (gradientType == GradientType::Linear) {
				auto& pMaterial = Material::getMaterial("posColor");
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
			auto& pMaterial = Material::getMaterial("posUniformColor");
			if (pMaterial) {
				//pMesh->setUniformColor(solidColor);//这个每次渲染前都需要调用
				pMesh->setMaterial(pMaterial);
			}
			else {
				LOGE("ERROR cannot found posUniformColor material");
			}
		}

		if (pStrokeMesh) {
			auto& pMaterial = Material::getMaterial("posUniformColor");
			if (pMaterial) {
				//pStrokeMesh->setUniformColor(solidColor);//这个每次渲染前都需要调用
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
	float scaleFactor = (float)textSize / (float)mpFontInfo->charSize;
	std::vector<CharRenderInfo> charsRenderInfoArray;
	for (size_t i = 0; i<slen;) {
		UnicodeType code;
		auto len = UtfConvert::utf(pStr, code);
		pStr += len;
		i += len;
		try {
			const auto& cinfo = mpFontInfo->getCharInTexture(code);
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
		catch (int error) {
			LOGE("UiRender::calcTextViewWidthHeight getCharInTexture error %d", error);
		}
	}

	if (tv->mLayoutWidth == LayoutParam::WrapContent) {
		tvRect.width = totalWidth + tv->mLayoutMarginLeft + tv->mLayoutMarginRight;
	}
	if (tv->mLayoutHeight == LayoutParam::WrapContent) {
		tvRect.height = totalHeight + tv->mLayoutMarginTop + tv->mLayoutMarginBottom;
	}
}

void UiRender::drawTextView(TextView* tv) {
	if (tv != nullptr) {
		drawBackground(tv);
		const auto& text = tv->getText();
		if (text.empty()) {
			return;
		}
		if (!mpFontInfo) {
			LOGE("ERROR NO fontInfo");
			return;
		}
		int originCharSize = mpFontInfo->charSize;
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
		int fontTextureWidth = mpFontInfo->pCharTexture->getWidth();//保存字体位图的纹理的宽度
		int fontTextureHeight = mpFontInfo->pCharTexture->getHeight();//保存字体位图的纹理的高度
		int tvMaxLines = tv->getMaxLines(); //textview设置的显示行数
		int currentLines = 1;			//
		int totalWidth = 0;				//字符串占用的宽度
		int totalHeight = -currentPosY; //字符串占用的高度
		float scaleFactor = (float)textSize / (float)mpFontInfo->charSize;
		std::vector<CharRenderInfo> charsRenderInfoArray;
		for(size_t i = 0; i<slen;){
			UnicodeType code;
			auto len = UtfConvert::utf(pStr, code);
			pStr += len;
			i += len;
			try {
				const auto& cinfo = mpFontInfo->getCharInTexture(code);
				if (currentPosX + scaleFactor*cinfo.width>maxWidth) {//当前要渲染的文字会超出textview的边界
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
						//maxYAdv = 0;
						++currentLines;
						totalWidth = tvRect.width;
						totalHeight = -currentPosY;
					}
				}

				auto maxYAdvTemp = scaleFactor*(cinfo.height - cinfo.top);
				
				//可以计算纹理矩阵了，mesh中的纹理坐标是(0,0)到(1,1)
				CharRenderInfo rInfo;
				
				rInfo.texMatrix = glm::translate(rInfo.texMatrix, glm::vec3((float)cinfo.x / (float)fontTextureWidth,
					(float)cinfo.y / (float)fontTextureHeight,0.0f));
				rInfo.texMatrix = glm::scale(rInfo.texMatrix, glm::vec3((float)cinfo.width / (float)fontTextureWidth,
					(float)cinfo.height / (float)fontTextureHeight,1.0f));

				//计算文字的model矩阵，渲染ui的时候使用的是正交投影，适配窗口的宽高
				//注意，ui坐标系统原点在左上角，y轴向下，与传统保持一直，openglY轴向上的
				rInfo.matrix = glm::translate(rInfo.matrix, glm::vec3(currentPosX+ scaleFactor*cinfo.left, currentPosY+ lineSpace+5 + scaleFactor*(cinfo.top-cinfo.height), 0.0f));
				rInfo.matrix = glm::scale(rInfo.matrix, glm::vec3(scaleFactor*cinfo.width, scaleFactor*cinfo.height, 1.0f));

				currentPosX += (scaleFactor*cinfo.advX + xExtraAdvance);//考虑到textview设置的额外字符间距
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
		auto textGrivate = tv->getGravity();
		glm::vec3 moveVec(0.0f, 0.0f, 0.0f);
		if (textGrivate == LayoutParam::Center) {
			if (totalWidth >= tvRect.width) {
				moveVec.x = tvRect.x;
			}
			else {
				moveVec.x = tvRect.x + (float)(tvRect.width- totalWidth)/2.0f;
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

		//根据对齐属性，调整每一个的位置,并且渲染
		glEnable(GL_SCISSOR_TEST);
		glScissor(tvRect.x, mWindowHeight-tvRect.y- tvRect.height, tvRect.width, tvRect.height);
		float trx = 0.0f;
		for (auto it = charsRenderInfoArray.begin(); it != charsRenderInfoArray.end(); ++it) {
			glm::mat4 moveToScreenMatrix(1.0f);
			moveToScreenMatrix = glm::translate(moveToScreenMatrix, moveVec);
			//it->matrix = glm::translate(it->matrix, moveVec);
			//绘制
			if (mpFontInfo->mpMaterial) {
				mpFontInfo->mpMaterial->updateUniformColor(tv->getTextColor());
			}
			if (mpFontInfo->mpCharMesh) {
				mpFontInfo->mpCharMesh->render(mProjMatrix*moveToScreenMatrix*it->matrix, it->texMatrix);
			}
		}
		glDisable(GL_SCISSOR_TEST);
		//处理完毕
	}
}

void UiRender::drawBackground(View* v){
	//绘制view的shape
	if (v!=nullptr)
	{
		auto& pBack = v->getBackground();
		if (!pBack) {
			return;
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

			auto& rect = v->getRect();
			glm::mat4 model(1.0f);
			model = glm::translate(model, glm::vec3(rect.x + paddingLeft, 
				(mWindowHeight - rect.y - rect.height + paddingBottom), 0.0f));
			/*model = glm::scale(model, 
				glm::vec3(rect.width-(paddingLeft+ paddingRight), rect.height-(paddingTop+ paddingBottom), 1.0f));*/

			if (pBackMesh) {
				auto& pTexture = pBack->mpTexture;
				if (pTexture) {
					pBackMesh->setTexture(pTexture);
				}
				pBackMesh->setUniformColor(pShape->getSolidColor());
				pBackMesh->render(mProjMatrix * model);
			}
			
			if (pBackStrokeMesh) {
				pBackStrokeMesh->setUniformColor(pShape->getSrokeColor());
				pBackStrokeMesh->render(mProjMatrix * model);
			}
		}
	}
}

void UiRender::drawLinearLayout(LinearLayout* pll) {
	drawBackground(pll);
	auto& children = pll->getChildren();
	for (auto& pChild : children) {
		if (pChild) {
			pChild->draw();
		}
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