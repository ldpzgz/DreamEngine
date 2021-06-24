#ifndef _BUTTON_H_
#define _BUTTON_H_
#include "View.h"
#include "OnClickListener.h"
#include<memory>
class Button : public View
{
public:
	Button();
	Button(const char* name);
	~Button();

	bool onLButtonUp(float x,float y);

	template<typename T>
	void setOnClickListener(T* p, void (T::*tFunc)(View* view))
	{
		mpOnClickListener = std::make_shared<OnClickListenerImp<T>>(p,tFunc);
	}

private:
	std::shared_ptr<OnClickListener> mpOnClickListener;
};

#endif
