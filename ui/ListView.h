#ifndef _LIST_VIEW_H_
#define _LIST_VIEW_H_
#include "View.h"
#include "ViewAdapter.h"
class ListView :public View{
public:
	using View::View;//�̳л���Ĺ��캯��
	
	void setAdapter(const std::shared_ptr<ViewAdapter>& pAdapter);

	std::shared_ptr<ViewAdapter>& getAdapter() {
		return mpAdapter;
	}

	void getFirstVisibleItem(int& firstItem,int& hideLength) {
		firstItem = mFirstVisibleItem;
		hideLength = mFirstItemHideLength;
	}

	void getLastVisibleItem(int& lastItem, int& hideLength) {
		lastItem = mLastVisibleItem;
		hideLength = mLastItemHideLength;
	}

	bool isHorizontal() {
		return mOrientation == LayoutParam::Horizontal;
	}

	void draw() override;

	bool mouseMove(int x, int y) override;

	bool mouseLButtonDown(int x, int y) override;

	bool mouseLButtonUp(int x, int y) override;

	void orientationHandler(const string& content) override;
private:
	/*
	* �����һ�������һ���ɼ���item
	*/
	void getVisibleItems();
	/*
	* �ƶ�listview�����item
	* �Ż�ֵ��ʾ�Ƿ�����ƶ���
	*/
	bool move(int moveDistance);
	/*
	* �����һ�������һ���ɼ���item
	*/
	//void getFirstAndLastVisibleItem();
	unsigned int mMouseState{ MouseState::MouseNone };
	bool mbLButtonDown{ false };
	Vec2i mPrePos;
	int mFirstVisibleItem{ 0 };
	int mFirstItemHideLength{ 0 };
	int mLastVisibleItem{ 0 };
	int mLastItemHideLength{ 0 };
	bool mbMovable{ false };
	int mVisibleItemTotalLength{ 0 };//���пɼ���item��������length��
	int mOrientation{ LayoutParam::Horizontal };
	std::shared_ptr<ViewAdapter> mpAdapter;
};

#endif