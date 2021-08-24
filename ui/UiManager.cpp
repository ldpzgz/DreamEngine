#include "UiManager.h"
#include "../Log.h"
#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>  //rapidxml::file
#include <rapidxml_print.hpp>  //rapidxml::print
using namespace std;

static const string gResourceStringFile("./opengles3/material/strings.xml");
static const string gResourceColorFile("./opengles3/material/colors.xml");

unique_ptr<UiManager> UiManager::gInstance = make_unique<UiManager>();
unordered_map<string, string> UiManager::gRStrings;
unordered_map<string, Color> UiManager::gRColors;
void UiManager::parseRColors(const string& path) {
	rapidxml::file<> fdoc(path.c_str());
	if (fdoc.size() > 0) {
		rapidxml::xml_document<> doc;// character type defaults to char
		doc.parse<0>(fdoc.data());// 0 means default parse flags
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
		LOGE("error to parse resource string file %s",path);
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
				if (attrName == "id") {
					//有id的控件，才保存起来，以便查找
					mpTree->mViews.emplace(attr->value(), pView);
				}
				else {
					auto it = View::gAttributeHandler.find(attrName);
					if (it != View::gAttributeHandler.end()) {
						it->second(pView, attr->value());
					}
					else {
						LOGD("there are no %s attributeHandler,please supplement", attrName.c_str());
					}
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
	rapidxml::file<> fdoc(filepath.c_str());
	if (fdoc.size() > 0) {
		rapidxml::xml_document<> doc;// character type defaults to char
		doc.parse<0>(fdoc.data());// 0 means default parse flags

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
		/*if (mpUiTree->mpRootView) {
			mpUiTree->mpRootView->setDirty(true);
		}*/
		UiRender::getInstance()->setTexture(mpUiTree->mpTexture);
	}
}

void UiManager::updateWidthHeight(float width, float height) {
	mWindowWidth = width;
	mWindowHeight = height;
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

bool UiManager::initUi(int w, int h) {
	//初始化uirender
	UiRender::getInstance()->initUiRender();
	//加载ui string和color配置
	parseRStrings(gResourceStringFile);
	parseRColors(gResourceColorFile);
	updateWidthHeight(w, h);
	return true;
}

void UiManager::draw() {
	if (mpUiTree) {
		//这个是将uitree绘制到纹理
		mpUiTree->draw();
		//这个是把纹理显示出来
		UiRender::getInstance()->drawUi();
	}
}