#include"NodeListAdapter.h"
#include "../ListView.h"
#include "../../Node.h"
#include "../UiManager.h"
#include "../TextView.h"
#include "../../MeshFilledRect.h"
#include "../../Texture.h"
#include <algorithm>

constexpr std::string_view cNodeInfo{"nodeInfo"};
struct NodeAnyInfo {
	int indentation{ 0 };
	bool bUnwrap{ false };
	bool hasSetChild{ false };
};


void NodeListAdapter::setNode(std::shared_ptr<Node>& pNode) {
	if (pNode) {
		//设置节点进来的时候，只显示根节点
		mpRootNode = pNode;
		mpFirstVisibleNode = mpRootNode;
		NodeAnyInfo anyInfo;
		pNode->setAny(NodeAnyIndex::TreeNodeInfo, std::any(anyInfo));
		initItemView();
		getVisibleItems();
	}
}

void NodeListAdapter::initItemView() {
	if (!mpItemView) {
		auto pParent = mpParentView.lock();
		if (pParent) {
			auto& parentRect = pParent->getRect();
			mpItemView = UiManager::loadFromFile("./opengles3/resource/layout/lvItem.xml", parentRect.width, parentRect.height);
			if (mpItemView) {
				mpItemView->setParent(mpParentView);
				auto pWrapView = mpItemView->findViewById("wrapIcon");
				if (pWrapView) {
					pWrapView->setOnClickListener([this](View* pView) {
						if (pView) {
							auto pParent = pView->getParent().lock();
							if (pParent) {
								auto any = pParent->getAny(cNodeInfo);
								if (any.has_value()) {
									auto pNode = std::any_cast<std::shared_ptr<Node>>(any);
									if (pNode && pNode->hasChild()) {
										NodeAnyInfo anyInfo = std::any_cast<NodeAnyInfo>(pNode->getAny(NodeAnyIndex::TreeNodeInfo));
				
										if (!anyInfo.hasSetChild) {
											anyInfo.hasSetChild = true;
											auto& children = pNode->getChildren();
											for (auto& pChild : children) {
												if (pChild) {
													NodeAnyInfo info;
													info.indentation = anyInfo.indentation + 1;
													pChild->setAny(NodeAnyIndex::TreeNodeInfo, std::any(info));
												}
											}
										}
										anyInfo.bUnwrap = !anyInfo.bUnwrap;
										pNode->setAny(NodeAnyIndex::TreeNodeInfo, std::any(anyInfo));
										//更新treeView
										getVisibleItems();
									}
								}
							}
						}
						
						});
				}
				mpWrapTex = UiManager::getInstance().getTexture("ui/wrap.png");
				mpUnWrapTex = UiManager::getInstance().getTexture("ui/unwrap.png");
				mpNullTex = UiManager::getInstance().getTexture("nullTex");
				mpNodeTex = UiManager::getInstance().getTexture("ui/node.png");

				//mpIconView = mpItemView->findViewById("objectIcon");
				//mpTvView = std::dynamic_pointer_cast<TextView>(mpItemView->findViewById("tv"));
				mItemHeight = mpItemView->getRect().height;
				mVisibleItems.emplace_back(mpItemView);
			}
		}
	}
}

Node* NodeListAdapter::getViewInternal(int position, bool toFirst) {
	Node* pNode = nullptr;
	bool bOver = false;
	auto visitChildFunc = [](Node* pnn)->bool {
		auto any = pnn->getAny(NodeAnyIndex::TreeNodeInfo);
		if (any.has_value()) {
			try {
				if (std::any_cast<bool>(any)) {
					return true;
				}
			}
			catch (std::bad_any_cast e) {

			}
		}
		return false;
	};
	if (position >= 0) {
		std::shared_ptr<Node> pFirstNode;
		if (toFirst) {
			pFirstNode = mpFirstVisibleNode;
		}
		else {
			pFirstNode = mpLastVisibleNode;
		}
		//pNode = pFirstNode.get();
		while (pFirstNode && pNode == nullptr) {
			pFirstNode->visitNodeForward([&pNode, &position](Node* pn)->bool {
				if (position == 0) {
					pNode = pn;
					return true;
				}
				else {
					--position;
				}
				return false;
				}, visitChildFunc, bOver);
			//还没有找到
			if (pNode==nullptr) {
				auto curNode = pFirstNode;
				std::shared_ptr<Node> pNextNode;
				while (curNode) {
					//我下一个兄弟的index
					auto nextNodeIndex = curNode->getIdInParent() + 1;
					auto curParentNode = curNode->getParent().lock();
					if (curParentNode) {
						pNextNode = curParentNode->getChild(nextNodeIndex);
						if (pNextNode) {
							break;
						}
					}
					curNode = curParentNode;
				}
				pFirstNode = pNextNode;
			}
		}
	}
	else {
		std::shared_ptr<Node> curNode;
		if (toFirst) {
			curNode = mpFirstVisibleNode;
		}
		else {
			curNode = mpLastVisibleNode;
		}
		std::shared_ptr<Node> pPreNode;
		while (curNode && pNode==nullptr) {
			int preNodeIndex = curNode->getIdInParent() - 1;
			auto curParentNode = curNode->getParent().lock();
			if (curParentNode) {
				if (preNodeIndex >= 0) {
					pPreNode = curParentNode->getChild(preNodeIndex);
					if (pPreNode) {
						pPreNode->visitNodeBackward([&pNode, &position](Node* pn)->bool {
							++position;
							if (position == 0) {
								pNode = pn;
								return true;
							}
							return false;
							}, visitChildFunc, bOver);
						curNode = pPreNode;
					}
				}
				else {
					++position;
					if (position == 0) {
						pNode = curParentNode.get();
					}
					else {
						curNode = curParentNode;
					}
				}
			}
			else {
				break;
			}
		}
	}
	return pNode;
}

/*
* 找到对应的itemView，设置好他们的外观，
* 把他们的缩进信息写入的itemView上，以便后面draw到相应的位置
*/
std::shared_ptr<View> NodeListAdapter::getView(int position){
	if (position + 1 > mVisibleItems.size()) {
		return{};
	}
	auto& pItemView = mVisibleItems[position];
	if (!pItemView) {
		return {};
	}
	auto pNodeAny = pItemView->getAny(cNodeInfo);
	if (!pNodeAny.has_value()) {
		return {};
	}
	std::shared_ptr<Node> pNode;
	try {
		pNode = std::any_cast<std::shared_ptr<Node>>(pNodeAny);
	}
	catch (std::bad_any_cast e) {

	}
	if (pNode) {
		auto nodeInfoAny = pNode->getAny(NodeAnyIndex::TreeNodeInfo);
		auto nodeInfo = std::any_cast<NodeAnyInfo>(nodeInfoAny);
		auto pWrapView = pItemView->findViewById("wrapIcon");
		pItemView->setAny(ViewAnyIndex::TreeNodeIndentation, nodeInfo.indentation);
		if (pNode->hasChild()) {
			if (nodeInfo.bUnwrap) {
				pWrapView->setBackgroundImg(mpUnWrapTex);
			}
			else {
				pWrapView->setBackgroundImg(mpWrapTex);
			}	
		}
		else {
			pWrapView->setBackgroundImg(mpNullTex);
		}
		auto pIconView = pItemView->findViewById("objectIcon");
		if (pIconView) {
			pIconView->setBackgroundImg(mpNodeTex);
		}
		auto pTvView = std::dynamic_pointer_cast<TextView>(pItemView->findViewById("tv"));
		if (pTvView) {
			auto& nodeName = pNode->getName();
			if (nodeName.empty()) {
				pTvView->setText("x");
			}
			else {
				pTvView->setText(nodeName);
			}
		}
		return pItemView;
	}
	return {};
}

Rect<int>* NodeListAdapter::getViewRect(int position) {
	if (position >= 0) {
		if (mpItemView) {
			return &mpItemView->getRect();
		}
	}
	return nullptr;
}

size_t NodeListAdapter::getDataSize() {
	return mItemCount;
}

/*
* treeView里面的data有变化的时候，或者拖动后调用这个函数
* 计算出第一个可见的item
* 与最后一个可见的item
* 可以选择从firstVisibleNode开始计算，或者从lastVisibleNode
*/
bool NodeListAdapter::getVisibleItems(bool fromFirstItem) {
	mVisibleItemCount = 0;
	auto pParentView = mpParentView.lock();
	if (!pParentView || !mpItemView) {
		return false;//can't move
	}
	int parentHeight = pParentView->getRect().height;
	mVisibleItemTotalHeight = 0;
	auto visitFunc = [this, fromFirstItem,parentHeight](Node* pNode)->bool {
		if (pNode) {
			if (mVisibleItems.size() <= mVisibleItemCount) {
				auto pView = mpItemView->clone(mpParentView.lock());
				if (pView) {
					mVisibleItems.emplace_back(pView);
				}
				else {
					LOGE("NodeListAdapter::getVisibleItems clone itemview failed");
				}
			}
			mVisibleItems[mVisibleItemCount++]->setAny(cNodeInfo, std::any(pNode->shared_from_this()));
			mVisibleItemTotalHeight += mItemHeight;
			
			if (fromFirstItem) {
				if (mVisibleItemTotalHeight - mFirstItemHideLength >= parentHeight) {
					mpLastVisibleNode = pNode->shared_from_this();
					mLastItemHideLength = mVisibleItemTotalHeight - mFirstItemHideLength - parentHeight;
					return true;//返回true，中断遍历节点
				}
				return false;
			}
			else {
				if (mVisibleItemTotalHeight - mLastItemHideLength >= parentHeight) {
					mpFirstVisibleNode = pNode->shared_from_this();
					mFirstItemHideLength = mVisibleItemTotalHeight - mLastItemHideLength - parentHeight;
					return true;//返回true，中断遍历节点
				}
				return false;
			}
		}
		else {
			return true;
		}
		return false;
	};
	auto isVisitChildFunc = [](Node* pNode)->bool {
		if (pNode->hasChild()) {
			auto any = pNode->getAny(NodeAnyIndex::TreeNodeInfo);
			if (any.has_value()) {
				try {
					if (std::any_cast<NodeAnyInfo>(any).bUnwrap) {
						return true;
					}
				}
				catch (std::bad_any_cast e) {

				}
			}
		}
		return false;
	};
	bool bOver = false;

	if (fromFirstItem) {
		auto pCurNode = mpFirstVisibleNode;
		while (pCurNode && !bOver) {
			pCurNode->visitNodeForward(visitFunc, isVisitChildFunc, bOver);
			if (!bOver) {
				do {
					int id = pCurNode->getIdInParent();
					auto pParentNode = pCurNode->getParent().lock();
					if (pParentNode) {
						pCurNode = pParentNode->getChild(id + 1);
						if (!pCurNode) {
							pCurNode = pParentNode;
							continue;
						}
						break;
					}
					else {
						pCurNode = pParentNode;
						break;
					}
				} while (true);
			}
		}
	}
	else {
		auto pCurNode = mpLastVisibleNode;
		while (!bOver && pCurNode) {
			pCurNode->visitNodeBackward(visitFunc, isVisitChildFunc, bOver);
			if (!bOver) {
				//visit parent or pre sibling;
				int id = 0;
				do {
					id = pCurNode->getIdInParent();
					pCurNode = pCurNode->getParent().lock();
					if (!pCurNode) {
						bOver = true;
						break;
					}
					if (id == 0) {
						bOver = visitFunc(pCurNode.get());
					}
					else {
						pCurNode = pCurNode->getChild(id - 1);
						break;
					}
				} while (!bOver && id==0);
			}
		}
		std::reverse(mVisibleItems.begin(), mVisibleItems.begin()+mVisibleItemCount);
	}

	pParentView->setDirty(true);

	if (mVisibleItemTotalHeight - mFirstItemHideLength-mLastItemHideLength > parentHeight) {
		return true;
	}
	
	return false;
}

/*
* 尝试移动moveDistance的距离。
* 如果移动了，计算出listview里面的第一个和最后一个可见的item
* 反回值表示是否有移动
*/
bool NodeListAdapter::tryToMove(int moveDistance, bool isVertical) {
	//在这里处理上下左右拖动的逻辑
	//根据移动距离，当前第一个和最后一个可见item，
	//计算出最新的第一个和最后一个可见item
	auto pParentView = mpParentView.lock();
	if (!pParentView)
		return false;
	
	int parentHeight = pParentView->getRect().height;
	if (mVisibleItemTotalHeight < parentHeight) {
		return false;
	}

	int leftLength = moveDistance;
	int itemCounts = getVisibleItemCount();
	if (itemCounts > 0) {
		if (leftLength < 0) {
			//往上拉，或者往左拉，能拉动多少？
			//确定最后一个可见的item
			int nextIndex = 1;
			Rect<int>* pNextItemRect = nullptr;
			Node* pLastVisibleItem = nullptr;
			int realMovedLen = 0;
			int toMove = mLastItemHideLength;
			while (true) {
				int delta = toMove + realMovedLen + leftLength;
				if (delta >= 0) {
					//移动完成
					realMovedLen += (toMove - delta);
					mLastItemHideLength = delta;
					break;
				}
				else {
					//继续移动
					realMovedLen += toMove;
					pLastVisibleItem = getViewInternal(nextIndex, false);
					if (pLastVisibleItem == nullptr) {
						mLastItemHideLength = 0;
						break;
					}
					else {
						mpLastVisibleNode = pLastVisibleItem->shared_from_this();
						//获取最后一个item的下一个item的rect
						//目前的版本每个item的rect都是一样的
						pNextItemRect = getViewRect(0);
						toMove = pNextItemRect->height;
					}
				}
			}
			if (realMovedLen == 0) {
				//拉不动了
				return false;
			}
			getVisibleItems(false);
			return true;
		}
		else if (leftLength > 0) {
			//往下拉，或者往右拉，能拉多少？
			//确定第一个可见的item
			int preIndex = -1;
			Rect<int>* pPreItemRect = nullptr;
			Node* pPreVisibleItem = nullptr;
			int realMovedLen = 0;
			int toMove = mFirstItemHideLength;
			while (true) {
				int delta = toMove + realMovedLen - leftLength;
				if (delta >= 0) {
					//移动完成
					realMovedLen += (toMove - delta);
					mFirstItemHideLength = delta;
					break;
				}
				else {
					//继续移动
					realMovedLen += toMove;
					pPreVisibleItem = getViewInternal(preIndex);
					if (pPreVisibleItem == nullptr) {
						mFirstItemHideLength = 0;
						break;
					}
					else {
						if (!pPreVisibleItem->getAny(NodeAnyIndex::TreeNodeInfo).has_value()) {
							//到顶了，再往上的Node不是最初setNode()进来的那个node了
							mFirstItemHideLength = 0;
							break;
						}
						mpFirstVisibleNode = pPreVisibleItem->shared_from_this();
						//获取第一个item的前一个item的rect
						//目前的版本每个item的rect都是一样的
						pPreItemRect = getViewRect(0);
						toMove = pPreItemRect->height;
					}
				}
			}
			if (realMovedLen == 0) {
				//拉不动了
				return false;
			}
			getVisibleItems();
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
	return true;
}

void NodeListAdapter::forVisibleItems(const std::function<bool(std::shared_ptr<View>&)>& func) {
	for (auto& pView : mVisibleItems) {
		if (pView && func) {
			if (func(pView)) {
				break;
			}
		}
	}
}