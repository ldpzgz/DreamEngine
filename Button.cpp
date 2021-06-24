#include "Button.h"

Button::Button()
{

}

Button::Button(const char* name):View(name)
{
	mViewType = VIEW_TYPE_BUTTON;
}
Button::~Button()
{
	
}

bool Button::onLButtonUp(float x,float y)
{
	if(mState==VIEW_STATE_PUSHED && hitTest(x,y) && mpOnClickListener)
	{
		mpOnClickListener->onClick(this);
	}
	return View::onLButtonUp(x,y);
}