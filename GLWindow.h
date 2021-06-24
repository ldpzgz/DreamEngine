#ifndef _GLWINDOW_H_
#define _GLWINDOW_H_

#include "../Window.h"
#include "OpenGL.h"

#include "../Demuxer.h"
#include "GraphicsMesh.h"
#include "GraphicsTexture.h"
#include "GraphicsShader.h"
#include "GraphicsFbo.h"
#include "Muxer.h"
#include "ShaderGaussianBlur.h"

#include "UIManager.h"
#include "../WinAudioRecorder.h"

//继承GLWindow的函数，重载postInit，draw函数。
class GLWindow : public Window,public OpenGL
{
public:
	GLWindow();
	~GLWindow();


	//opengl virtual func
	//初始化资源
	bool virtual postInit(int width, int height);
	void virtual preDraw(void);
	//实现绘制场景
	void virtual draw();
	void virtual postDraw(void);

	void virtual postSwapBuffer();


	//这个函数内部会调用postInit函数
	bool init(int w, int h, char* name, int timerInterval = 0);
	void onSize(int w,int h);
	void onLButtonDown(int x,int y);
	void onMouseMove(int x,int y);
	void onLButtonUp(int x,int y);

	//这个函数内部会调用preDraw，draw，postDraw等绘制函数。
	void onTimer(int elapseTime);

	void onDestroy();

	void onButtonClick(View* p);
private:
	Demuxer mDemutex;
	//Demuxer mDemutexAudio;
	Demuxer mDemutexFile;
	GraphicsMesh mMesh;
	GraphicsImage* mpTexY;
	GraphicsImage* mpTexU;
	GraphicsImage* mpTexV;
	GraphicsImage* mpTexRGB;
	GraphicsImage* mpFboTex;
	GraphicsImage* mpFboTex2;
	GraphicsImage* mpFboTex3;
	GraphicsShader mShader;
	GraphicsShader mShaderGrayScale;
	ShaderGSBlur mShaderGsBlur;
	GraphicsShader mShaderSobel;
	GraphicsShader mShaderDirNonmaxSuppres;
	GraphicsShader mShaderWeakPixelInclusion;
	GraphicsShader mShaderButify;
	GraphicsShader mShaderForRecord;
	GraphicsShader mShaderForShow;
	GraphicsFbo mFbo;
	GraphicsFbo mFbo2;
	GraphicsFbo mFbo3;
	Muxer mRecorder;

	Sink* mpSink;
	unsigned char* mpReadPixelData;

	Button* mpBt;
	//WinAudioRecorder mAudioRecorder;
};

#endif
