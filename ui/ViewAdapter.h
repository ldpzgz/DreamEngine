#pragma once

#include"core/Rect.h"
#include <memory>
#include <functional>
class View;
class ViewAdapter
{
public:
	explicit ViewAdapter(std::shared_ptr<View>& pContainerView) :
		mpParentView(pContainerView)
	{

	}
	ViewAdapter() = default;
	virtual ~ViewAdapter() {

	}

	virtual std::shared_ptr<View> getView(int position) {
		return {};
	}

	virtual Rect<int>* getViewRect(int position) {
		return nullptr;
	}

	virtual int getVisibleItemCount() {
		return 0;
	}

	virtual int getFirstItemHideLength() {
		return 0;
	}

	virtual size_t getDataSize() {
		return 0;
	}

	//return value is moveable
	virtual bool getVisibleItems(bool fromFirstItem = true) {
		return false;
	}

	/*
	* 尝试移动moveDistance的距离。
	* 如果移动了，计算出listview里面的第一个和最后一个可见的item
	* 反回值表示是否有移动
	*/
	virtual bool tryToMove(int moveDistance, bool isVertical) {
		return false;
	}

	virtual void forVisibleItems(const std::function<bool(std::shared_ptr<View>&)>&) {

	}
protected:
	std::weak_ptr<View> mpParentView;
};
