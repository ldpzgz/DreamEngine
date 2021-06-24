#ifndef GLVIEW_H_
#define GLVIEW_H_

#include "../Demuxer.h"

#include "OpenGL.h"
#include "GraphicsMesh.h"
#include "GraphicsShader.h"
#include "GraphicsTexture.h"

#include "Button.h"
#include "UIManager.h"


class GLSurface : public OpenGL
{
public:
	GLSurface();
	~GLSurface();

	virtual void resize(int w,int h);
	virtual void preDraw(void);
	virtual void draw();
	virtual void postDraw(void);
	virtual bool postInit(int width,int height);

	virtual void onLButtonDown(int x,int y);
	virtual void onMouseMove(int x,int y);
	virtual void onLButtonUp(int x,int y);

    void onClick(View* view);
	void onSeekPosChanged(View* view,float newPos);
private:
	int mWindowWidth;
	int mWindowHeight;
	GraphicsMesh mMesh;
	GraphicsShader mShader;
	GraphicsTexture mTextrueY;
	GraphicsTexture mTextrueU;
	GraphicsTexture mTextrueV;
	Demuxer mDemuxer;

	Button* mpBt;
	SeekBar* mpSeekBar;
};

#endif
