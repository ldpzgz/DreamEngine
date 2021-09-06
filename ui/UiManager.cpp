#include "UiManager.h"
#include "../Log.h"
#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>  //rapidxml::file
#include <rapidxml_print.hpp>  //rapidxml::print
#include <filesystem>
#include "../Utils.h"
using namespace std::filesystem;
using namespace std;

static const string gStringFile("./opengles3/material/strings.xml");
static const string gColorFile("./opengles3/material/colors.xml");
static const string gShapePath("./opengles3/material/shape");
static const string gUiImagePath("./opengles3/material/uiImage");
static const string gUiLayoutPath("./opengles3/material/layout");

unique_ptr<UiManager> UiManager::gInstance = make_unique<UiManager>();
unordered_map<string, string> UiManager::gRStrings;
unordered_map<string, Color> UiManager::gRColors;
unordered_map<string, std::shared_ptr<Shape>> UiManager::gRShapes;

void UiManager::loadAllShape() {
	//遍历UIimage目录
	path shapePath(gShapePath);
	if (!exists(shapePath)) {
		LOGE("ERROR the ui shape path %s is not exist", gShapePath.c_str());
	}

	if (is_directory(shapePath)) {
		//是目录
		directory_iterator list(shapePath);
		//directory_entry 是一个文件夹里的某一项，可以是path，也可以是文件
		for (auto& it : list) {
			auto filePath = it.path();
			if (is_regular_file(filePath)) {
				//是文件
				auto filePathString = filePath.string();
				parseRShape(filePathString);
			}
		}
	}
}

void UiManager::loadAllUiImage() {
	//遍历UIimage目录
	path imagePath(gUiImagePath);
	if (!exists(imagePath)) {
		LOGE("ERROR the ui image path %s is not exist", gUiImagePath.c_str());
	}

	if (is_directory(imagePath) ){
		//是目录
		directory_iterator list(imagePath);
		//directory_entry 是一个文件夹里的某一项，可以是path，也可以是文件
		for (auto& it : list) {
			auto filePath = it.path();
			if (is_regular_file(filePath)) {
				//是文件
				auto filePathString = filePath.string();
				Material::loadTextureFromFile(filePathString);
			}
		}
	}
}

void UiManager::parseRShape(const string& path) {
	shared_ptr<rapidxml::file<>> pfdoc;
	try {
		pfdoc = make_shared<rapidxml::file<>>(path.c_str());
	}
	catch (std::exception e) {
		LOGE("error to parseRShape %s file,error %s", path.c_str(), e.what());
		return;
	}
	if (pfdoc && pfdoc->size() > 0) {
		string shapeName = Utils::getFileName(path);
		rapidxml::xml_document<> doc;// character type defaults to char
		doc.parse<0>(pfdoc->data());// 0 means default parse flags
		auto pResNode = doc.first_node("shape");
		if (pResNode != nullptr) {
			auto shape = std::make_shared<Shape>();
			auto attribute = pResNode->first_attribute("type");
			if (attribute != nullptr && shape) {
				shape->setType(attribute->value());
			}
			pResNode = pResNode->first_node();
			while (pResNode != nullptr) {
				auto attribute = pResNode->first_attribute();
				while (attribute != nullptr) {
					string key = attribute->name();
					string value = attribute->value();
					auto it = Shape::gShapeAttributeHandler.find(key);
					if (it != Shape::gShapeAttributeHandler.end()) {
						it->second(shape, value);
					}
					else {
						LOGE("cannot recognize shape attribute %s",key.c_str());
					}
					attribute = attribute->next_attribute();
				}
				pResNode = pResNode->next_sibling();
			}
			if (gRShapes.emplace(shapeName, shape).second == false) {
				LOGE("ERROR to emplace shape %s ,already exist", shapeName.c_str());
				shape.reset();
			}
		}
	}
}

void UiManager::parseRColors(const string& path) {
	shared_ptr<rapidxml::file<>> pfdoc;
	try {
		pfdoc = make_shared<rapidxml::file<>>(path.c_str());
	}
	catch (std::exception e) {
		LOGE("error to parseRStrings %s file,error %s", path.c_str(), e.what());
		return;
	}
	if (pfdoc && pfdoc->size() > 0) {
		rapidxml::xml_document<> doc;// character type defaults to char
		doc.parse<0>(pfdoc->data());// 0 means default parse flags
		auto pResNode = doc.first_node("resources");
		if (pResNode != nullptr) {
			auto pColorNode = pResNode->first_node("color");
			while (pColorNode != nullptr) {
				auto attribute = pColorNode->first_attribute("name");
				if (attribute != nullptr) {
					string key = attribute->value();
					string value = pColorNode->value();//value is like #ffffffaa
					if (!key.empty() && !value.empty()) {
						Color color;
						if (Color::parseColor(value, color)) {
							auto ret = gRColors.try_emplace(key, color);
							if (!ret.second) {
								LOGD("there already has color who's name is %s in gRColors", key.c_str());
							}
						}
						else {
							LOGE("parse color %s in colors.xml", key.c_str());
						}
					}
				}
				pColorNode = pColorNode->next_sibling();
			}
		}
	}
	else {
		LOGE("error to parse resource string file %s",path.c_str());
	}
}

void UiManager::parseRStrings(const string& path) {
	shared_ptr<rapidxml::file<>> pfdoc;
	try {
		pfdoc = make_shared<rapidxml::file<>>(path.c_str());
	}
	catch (std::exception e) {
		LOGE("error to parseRStrings %s file,error %s", path.c_str(),e.what());
		return;
	}
	
	if (pfdoc && pfdoc->size() > 0) {
		rapidxml::xml_document<> doc;// character type defaults to char
		doc.parse<0>(pfdoc->data());// 0 means default parse flags
		auto pResNode = doc.first_node("resources");
		if (pResNode != nullptr) {
			auto pStringNode = pResNode->first_node("string");
			while (pStringNode != nullptr) {
				auto attribute = pStringNode->first_attribute("name");
				if (attribute != nullptr) {
					string key = attribute->value();
					string value = pStringNode->value();//value is like #ffffffaa
					if (!key.empty() && !value.empty()) {
						auto ret = gRStrings.try_emplace(key, value);
						if (!ret.second) {
							LOGE("error to store string resource %s",key.c_str());
						}
					}
				}
				pStringNode = pStringNode->next_sibling();
			}
		}
	}
}

void parseView(const shared_ptr<View>& parent, rapidxml::xml_node<char>* pnode, shared_ptr<UiTree>& mpTree) {
	shared_ptr<View> pView;
	if (pnode != nullptr) {
		string viewName = pnode->name();
		pView = View::createView(viewName, nullptr);
		
		if (pView) {
			pView->setDirtyListener(mpTree);
			if (parent) {
				pView->setParent(parent);
				parent->addChild(pView);
			}
			else{
				mpTree->mpRootView = pView;
			}

			auto attr = pnode->first_attribute();
			while (attr != nullptr) {
				string attrName = attr->name();
				
				auto it = View::gLayoutAttributeHandler.find(attrName);
				if (it != View::gLayoutAttributeHandler.end()) {
					it->second(pView, attr->value());
					if (attrName == "id") {
						//有id的控件，才保存起来，以便查找
						mpTree->mViews.emplace(attr->value(), pView);
					}
				}
				else {
					LOGD("there are no %s attributeHandler,please supplement", attrName.c_str());
				}
				
				attr = attr->next_attribute();
			}
		}
		auto child = pnode->first_node();
		auto sibling = pnode->next_sibling();
		if (child != nullptr) {
			parseView(pView, child, mpTree);
		}
		if (sibling != nullptr) {
			parseView(parent, sibling, mpTree);
		}
	}
}

//加载ui的布局文件
shared_ptr<UiTree> UiManager::loadFromFile(const string& filepath) {
	shared_ptr<UiTree> mpTree = make_shared<UiTree>();
	//读取xml
	shared_ptr<rapidxml::file<>> pfdoc;
	try {
		pfdoc = make_shared<rapidxml::file<>>(filepath.c_str());
	}
	catch (std::exception e) {
		LOGE("error to loadFromFile %s file,error %s", filepath.c_str(), e.what());
		return mpTree;
	}
	if (pfdoc->size() > 0) {
		rapidxml::xml_document<> doc;// character type defaults to char
		doc.parse<0>(pfdoc->data());// 0 means default parse flags

		auto root = doc.first_node();
		parseView(shared_ptr<View>(), root, mpTree);
	}
	return mpTree;
}

UiManager::UiManager() {

}

UiManager::~UiManager() {

}

void UiManager::setUiTree(const shared_ptr<UiTree>& tree) {
	mpUiTree = tree;
	if (mpUiTree) {
		//计算出view的位置尺寸
		mpUiTree->updateWidthHeight(mWindowWidth, mWindowHeight);
		mpUiTree->calcViewsRect(mWindowWidth, mWindowHeight);
	}
}

void UiManager::updateWidthHeight(float width, float height) {
	mWindowWidth = width;
	mWindowHeight = height;
	/*mFboForCopy.detachColorTexture();
	if (!mpTexture) {
		mpTexture = make_shared<Texture>();
	}
	mpTexture->unload();
	mpTexture->load(width, height, nullptr, GL_RGBA);
	mFboForCopy.attachColorTexture(mpTexture, 0);
	UiRender::getInstance()->setTexture(mpTexture);*/
	UiRender::getInstance()->updateWidthHeight(width, height);
	if (mpUiTree) {
		mpUiTree->updateWidthHeight(width, height);
	}
}

void UiManager::mouseMove(int x, int y) {
	if (mpUiTree) {
		if (mpUiTree->mpRootView) {
			mpUiTree->mpRootView->mouseMove( x, y);
			auto children = mpUiTree->mpRootView->getChildren();
			for (auto& child : children) {
				if (child) {
					child->mouseMove( x, y);
				}
			}
		}
	}
}

void UiManager::mouseLButtonDown(int x, int y) {
	if (mpUiTree) {
		if (mpUiTree->mpRootView) {
			mpUiTree->mpRootView->mouseLButtonDown( x, y);
			auto children = mpUiTree->mpRootView->getChildren();
			for (auto& child : children) {
				if (child) {
					child->mouseLButtonDown( x, y);
				}
			}
		}
	}
}

void UiManager::mouseLButtonUp(int x, int y) {
	if (mpUiTree) {
		if (mpUiTree->mpRootView) {
			mpUiTree->mpRootView->mouseLButtonUp( x, y);
			auto children = mpUiTree->mpRootView->getChildren();
			for (auto& child : children) {
				if (child) {
					child->mouseLButtonUp( x, y);
				}
			}
		}
	}
}

bool Color::parseColor(const std::string& value, Color& color)
{
	const string temp = "@color/";
	auto index1 = value.find(temp);
	auto size = value.size();
	if ( index1 == 0) {
		auto colorName = value.substr(temp.size());
		color = UiManager::getColor(colorName);
		return true;
	}
	else if (value.empty() || size > 9 || value[0] != '#') {
		return false;
	}
	int index = 0;
	int relIndex = 1 + 2 * index;
	bool hasError = false;
	while (index < 4 && relIndex < size) {
		auto Rstr = value.substr(relIndex, 2);
		try {
			color[index] = (float)std::stoi(Rstr, nullptr, 16) / 255.0f;
		}
		catch (.../*const logic_error& e*/) {
			hasError = true;
			break;
		}
		++index;
		relIndex = 1 + 2 * index;
	}
	if (!hasError) {
		if (size == 7) {
			color.a = 1.0f;
		}
		else if (size == 5) {
			color.b = 0.0f;
			color.a = 1.0f;
		}
		else if (size == 3) {
			color.g = 0.0f;
			color.b = 0.0f;
			color.a = 1.0f;
		}
	}
	return !hasError;
}

Color& UiManager::getColor(const std::string& name) {
	static Color temp;
	auto it = gRColors.find(name);
	if (it != gRColors.end()) {
		return it->second;
	}
	else {
		LOGE("cannot find %s color in color resource", name.c_str());
		return temp;
	}
}

std::shared_ptr<Shape>& UiManager::getShape(const std::string& name) {
	static std::shared_ptr<Shape> temp;
	auto it = gRShapes.find(name);
	if (it != gRShapes.end()) {
		return it->second;
	}
	else {
		LOGE("cannot find %s shape in shape resource", name.c_str());
		return temp;
	}
}

std::shared_ptr<Texture>& UiManager::getTexture(const std::string& name) {
	return Material::getTexture(name);
}

std::string& UiManager::getString(const std::string& name) {
	static std::string temp;
	auto it = gRStrings.find(name);
	if (it != gRStrings.end()) {
		return it->second;
	}
	else {
		LOGE("cannot find %s string in string resource",name.c_str());
		return temp;
	}
}

bool UiManager::initUi(int w, int h) {
	//加载ui string和color配置
	parseRStrings(gStringFile);
	parseRColors(gColorFile);
	//加载ui中需要用到的图片
	loadAllUiImage();
	loadAllShape();
	//初始化uirender
	UiRender::getInstance()->initUiRender();

	updateWidthHeight(w, h);
	return true;
}

void UiManager::draw() {
	if (mpUiTree) {
		//这个是将uitree绘制到纹理
		if (mpUiTree->draw()) {
			//把uitree渲染的结果拷贝到mpTexture里面，mpTexture已经设置给了uiRender
			//Fbo::blitFbo(mpUiTree->mFboForRender, mFboForCopy);
		}
		//这个是把纹理显示出来
		UiRender::getInstance()->drawUi();
	}
}