#include "GLWindow.h"
#include "../Utils.h"
#include <string>

GLWindow::GLWindow() :
	mMesh(MESH_Rectangle),
	mpTexY(0),
	mpTexU(0),
	mpTexV(0),
	mpTexRGB(0),
	mpReadPixelData(0),
	mpSink(0)
	//mAudioRecorder(44100,2)
{
}
GLWindow::~GLWindow()
{
	if (mpReadPixelData != 0)
	{
		delete[] mpReadPixelData;
		mpReadPixelData = 0;
	}
}

void GLWindow::onSize(int w, int h)
{
	OpenGL::resize(w, h);
}
bool GLWindow::init(int w, int h, char* name, int timerInterval)
{
	bool bret = Window::init(w, h, name, timerInterval);
	if (!bret)
	{

	}
	bret = OpenGL::initGL((HWND)mId, mWidth, mHeight);

	mpReadPixelData = new unsigned char[m_Width*m_Height * 4];

	return bret;
}
void GLWindow::onTimer(int elapseTime)
{
	OpenGL::drawScene();
}

void GLWindow::onDestroy()
{
	Window::onDestroy();
	//mAudioRecorder.stop();
	mDemutex.stop();
	//mRecorder.getVideoSink()->setOver();
	//mRecorder.getAudioSink(0)->setOver();
	mRecorder.stop();
}

void GLWindow::onButtonClick(View* p)
{
#ifdef _WIN32
	::DestroyWindow((HWND)mId);
	//PostMessage((HWND)mId,WM_DESTROY,0,0);
#endif

}

void GLWindow::preDraw(void)
{
}
void GLWindow::draw()
{
	int posLoc = -1;
	int texLoc = -1;
	VideoDataInfo vdi = mDemutex.getVideoData();

	mFbo3.startRender();
	mShader.useMe();
	posLoc = mShader.getAttributeLoc("position");
	texLoc = mShader.getAttributeLoc("texpos");
	mShader.setUniform1f("texelWidthOffset", 1.0f / 640.0f);
	mShader.setUniform1f("texelHeightOffset", 1.0f / 480.0f);

	if (mpTexRGB != 0)
	{
		mpTexRGB->active(0);
		if (vdi.pData[0] != 0)
		{
			mpTexRGB->update(vdi.pData[0]);
		}
		mShader.setUniform1i("inputImageTexture", 0);
		mMesh.draw(posLoc, texLoc);
		vdi.free();
	}
	mFbo3.endRender();

	mFbo.startRender();
	//glViewport(0, 0, m_Width, m_Height);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();							
	//gluPerspective(45.0f,(GLfloat)m_Width/(GLfloat)m_Height,1.0f,1000000.0f);							
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	/*glOrtho(-m_Width / 2.0, m_Width / 2.0, -m_Height / 2.0, m_Height / 2.0, -10.0, 1000.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glColor3f(0.0f, 1.0f, 0.0f);
	glBegin(GL_TRIANGLES);
	glVertex3f(100.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 100.0f, 0.0f);
	glVertex3f(-100.0f, 0.0f, 0.0f);
	glEnd();*/
	mShaderGrayScale.useMe();
	posLoc = mShaderGrayScale.getAttributeLoc("position");
	texLoc = mShaderGrayScale.getAttributeLoc("texpos");

	//mShader.setUniform1f("imgwidth", 640);
	//mShader.setUniform1f("imgheight", 480);
	if (mpTexY != 0)
	{
		mpTexY->active(0);
		if (vdi.pData[0] != 0)
		{
			mpTexY->update(vdi.pData[0]);
		}
		mShader.setUniform1i("SamplerY", 0);
	}

	if (mpTexU != 0)
	{
		mpTexU->active(1);
		if (vdi.pData[1] != 0)
		{
			mpTexU->update(vdi.pData[1]);
		}
		mShader.setUniform1i("SamplerU", 1);
	}

	if (mpTexV != 0)
	{
		mpTexV->active(2);
		if (vdi.pData[2] != 0)
		{
			mpTexV->update(vdi.pData[2]);
		}
		mShader.setUniform1i("SamplerV", 2);
	}
	if (mpTexRGB != 0)
	{
		mpTexRGB->active(0);
		/*if (vdi.pData[0] != 0)
		{
		mpTexRGB->update(vdi.pData[0]);
		}*/
		//mShader.setUniform1i("inputImageTexture", 0);
		//mShader.setUniform1f("texelWidthOffset", 1.0f/640.0f);
		//mShader.setUniform1f("texelHeightOffset", 0.0f);
		//mShader.setUniform1f("distanceNormalizationFactor", 2.0f);

		mShaderGrayScale.setUniform1i("inputImageTexture", 0);
	}
	mMesh.draw(posLoc, texLoc);
	//vdi.free();
	mFbo.endRender();


	mFbo2.startRender();
	//glViewport(0, 0, m_Width, m_Height);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mShaderGsBlur.useMe();
	posLoc = mShaderGsBlur.getAttributeLoc("position");
	texLoc = mShaderGsBlur.getAttributeLoc("inputTextureCoordinate");
	mShaderGsBlur.setUniform1f("texelWidthOffset", 1.0f / 640.0f);
	mShaderGsBlur.setUniform1f("texelHeightOffset", 1.0f / 480.0f);
	mpFboTex->active(0);
	mShaderGsBlur.setUniform1i("inputImageTexture", 0);
	mMesh.draw(posLoc, texLoc);
	mFbo2.endRender();

	//mpFboTex2
	mFbo.startRender();
	//glViewport(0, 0, m_Width, m_Height);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mShaderSobel.useMe();
	posLoc = mShaderSobel.getAttributeLoc("position");
	texLoc = mShaderSobel.getAttributeLoc("inputTextureCoordinate");
	mShaderSobel.setUniform1f("texelWidth", 1.0f / 640.0f);
	mShaderSobel.setUniform1f("texelHeight", 1.0f / 480.0f);
	mpFboTex2->active(0);
	mShaderSobel.setUniform1i("inputImageTexture", 0);
	mMesh.draw(posLoc, texLoc);
	mFbo.endRender();

	mFbo2.startRender();
	//glViewport(0, 0, m_Width, m_Height);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	mShaderDirNonmaxSuppres.useMe();
	posLoc = mShaderDirNonmaxSuppres.getAttributeLoc("position");
	texLoc = mShaderDirNonmaxSuppres.getAttributeLoc("inputTextureCoordinate");
	mShaderDirNonmaxSuppres.setUniform1f("texelWidth", 1.0f / 640.0f);
	mShaderDirNonmaxSuppres.setUniform1f("texelHeight", 1.0f / 480.0f);
	mShaderDirNonmaxSuppres.setUniform1f("upperThreshold", 0.5f);
	mShaderDirNonmaxSuppres.setUniform1f("lowerThreshold", 0.1f);
	mpFboTex->active(0);
	mShaderDirNonmaxSuppres.setUniform1i("inputImageTexture", 0);
	mMesh.draw(posLoc, texLoc);
	mFbo2.endRender();

	mFbo.startRender();
	mShaderWeakPixelInclusion.useMe();
	posLoc = mShaderWeakPixelInclusion.getAttributeLoc("position");
	texLoc = mShaderWeakPixelInclusion.getAttributeLoc("inputTextureCoordinate");
	mShaderWeakPixelInclusion.setUniform1f("texelWidth", 1.0f / 640.0f);
	mShaderWeakPixelInclusion.setUniform1f("texelHeight", 1.0f / 480.0f);
	mpFboTex2->active(0);
	mShaderWeakPixelInclusion.setUniform1i("inputImageTexture", 0);
	mMesh.draw(posLoc, texLoc);
	mFbo.endRender();

	mFbo2.startRender();
	mShaderButify.useMe();
	posLoc = mShaderButify.getAttributeLoc("position");
	texLoc = mShaderButify.getAttributeLoc("inputTextureCoordinate");
	mShaderButify.setUniform1f("smoothDegree", 0.5f);
	mpFboTex3->active(0);
	mShaderButify.setUniform1i("inputImageTexture", 0);
	mpFboTex->active(1);
	mShaderButify.setUniform1i("inputImageTexture2", 1);
	mpTexRGB->active(2);
	mShaderButify.setUniform1i("inputImageTexture3", 2);
	mMesh.draw(posLoc, texLoc);
	mFbo2.endRender();

	mFbo.startRender();
	mShaderForRecord.useMe();
	posLoc = mShaderForRecord.getAttributeLoc("vPosition");
	texLoc = mShaderForRecord.getAttributeLoc("texPos");
	mpFboTex2->active(0);
	mShaderForRecord.setUniform1i("SamplerRGB", 0);
	mMesh.draw(posLoc, texLoc);

	if (mpSink != 0)
	{
		//format:GL_ALPHA, GL_RGB, and GL_RGBA.
		//type: GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_4_4_4_4, or GL_UNSIGNED_SHORT_5_5_5_1
		//only one pair of format/type is always accept:GL_RGBA/GL_UNSIGNED_BYTE
		//
		int format;
		int type;
		glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &format);
		glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &type);
		glPixelStorei(GL_PACK_ALIGNMENT, 1);

		glReadPixels(0, 0, m_Width, m_Height, GL_RGBA, GL_UNSIGNED_BYTE, (void*)mpReadPixelData);
		checkglerror();
		mpSink->write(mpReadPixelData, m_Width*m_Height * 4, m_Width, m_Height, PIX_RGBA, 1);
	}

	mFbo.endRender();

	glViewport(0, 0, m_Width, m_Height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mShaderForShow.useMe();
	posLoc = mShaderForShow.getAttributeLoc("vPosition");
	texLoc = mShaderForShow.getAttributeLoc("texPos");
	mpFboTex2->active(0);
	mShaderForShow.setUniform1i("SamplerRGB", 0);
	mMesh.draw(posLoc, texLoc);

	UIManager::getInstance()->drawUI();

	/*mFbo.disable();
	glViewport(0, 0, m_Width, m_Height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	if (mpFboTex != 0)
	{
	mpFboTex->active(0);
	mShader.setUniform1i("SamplerRGB", 0);
	}
	mMesh.draw(posLoc, texLoc);*/
}
void GLWindow::postDraw(void)
{
}

bool GLWindow::postInit(int width, int height)
{
	av_register_all();
	VideoParam vp;
	//mDemutex.openFile("haha.flv",false,true);
	mDemutex.openDevice("vfwcap", 0, false, true);//"dshow", "video=WebCam SCB-0340N"
												  /*AudioParam param;
												  param.samplerate = 44100;
												  param.channel = 2;
												  param.sampleformat = AV_SAMPLE_FMT_S16;
												  mDemutex.setAudioDstParam(param);
												  *///demutex.setDstParam(1,param);
	vp = mDemutex.getSrcVideoParam();
	//vp.width = 640;
	//vp.height = 480;
	vp.pix_fmt = AV_PIX_FMT_RGB24;
	mDemutex.setDstVideoParam(vp);
	mDemutex.start();

	mDemutexFile.openFile("122.mp4", true, false);
	mDemutexFile.setIsPushInQueue(false, 0);
	mDemutexFile.setIsPushInQueueForMix(false);
	mDemutexFile.setMixAudio(false);
	mDemutexFile.setIsReadAudioTrack(false, 0);

	//mDemutexAudio.openDevice("dshow", UnicodeToUTF8(L"audio=麦克风 (USB Audio Device)").c_str(), true, false);
	//AudioParam ap = mDemutexAudio.getSrcAudioParam(0);
	//mDemutexAudio.setIsPushInQueue(false, 0);
	//mDemutexAudio.setIsPushInQueueForMix(false);
	//mDemutexAudio.setMixAudio(false);

	VideoParam vparam;
	VideoParam vparamDst;
	vparam.width = 640;
	vparam.height = 480;
	vparam.pix_fmt = AV_PIX_FMT_RGBA;
	vparamDst = vparam;
	vparamDst.pix_fmt = AV_PIX_FMT_YUV420P;
	mRecorder.setPath("recordGl.mkv");
	mRecorder.addVideoStream(vparam, vparamDst);
	Sink* psink = mRecorder.getVideoSink();
	mpSink = psink;

	/*AudioParam aparam;
	aparam.bitrate = 128000;
	aparam.samplerate = 44100;
	aparam.channel = 2;
	aparam.numsample = 1152;
	aparam.framesize = 4608;
	aparam.sampleformat = AV_SAMPLE_FMT_S16P;
	mRecorder.addAudioStream(aparam,aparam);*/

	mRecorder.addAudioStream(mDemutexFile.getAudioStream(0));
	AudioSink* psink2 = mRecorder.getAudioSink(0);
	psink2->initSink(MUXER_BUFFER_SIZE, true, true, true);
	

	mDemutexFile.setAudioPktSink(psink2, 0);
	mDemutexFile.start();

	mRecorder.start();
	//mAudioRecorder.addDataSink(psink2);
	//mAudioRecorder.start();

	/*mDemutexAudio.setAudioSink(psink2, 0);
	mDemutexAudio.start();*/
	



	if (vp.pix_fmt == AV_PIX_FMT_YUV420P)
	{
		mShader.initShaderFromFile("./shader/vsShader.txt", "./shader/psShader.txt");
		mpTexY = GraphicsImageManager::getInstance()->createImage("videoTexY", vp.width, vp.height, GL_LUMINANCE);
		mpTexU = GraphicsImageManager::getInstance()->createImage("videoTexU", vp.width / 2, vp.height / 2, GL_LUMINANCE);
		mpTexV = GraphicsImageManager::getInstance()->createImage("videoTexV", vp.width / 2, vp.height / 2, GL_LUMINANCE);
	}
	else if (vp.pix_fmt == AV_PIX_FMT_BGR24)
	{
		mShader.initShaderFromFile("./shader/vsRgbShader.txt", "./shader/psRgbShader.txt");
		mpTexRGB = GraphicsImageManager::getInstance()->createImage("videoTexRGB", vp.width, vp.height, GL_BGR);
		mpFboTex = GraphicsImageManager::getInstance()->createImage("fboTex", vp.width, vp.height, GL_BGR);
	}
	else if (vp.pix_fmt == AV_PIX_FMT_RGB24)
	{
		mShader.initShaderFromFile("./shader/vsBilateralFilter.txt", "./shader/psBilateralFilter.txt");
		mShaderGrayScale.initShaderFromFile("./shader/vsGrayScale.txt", "./shader/psGrayScale.txt");
		mShaderGsBlur.initShader();
		mShaderSobel.initShaderFromFile("./shader/vs33Sampler.txt", "./shader/psDirectionalSobelEdgeDetection.txt");
		mShaderDirNonmaxSuppres.initShaderFromFile("./shader/vsDirectionalNonmaximumSuppression.txt",
			"./shader/psDirectionalNonmaximumSuppression.txt");
		mShaderWeakPixelInclusion.initShaderFromFile("./shader/vs33Sampler.txt",
			"./shader/psWeakPixelInclusion.txt");
		mShaderButify.initShaderFromFile("./shader/vsButify.txt", "./shader/psButify.txt");

		mShaderForRecord.initShaderFromFile("./shader/vsRgbShader.txt", "./shader/psRgbShader.txt");
		mShaderForShow.initShaderFromFile("./shader/vsRgbShader2.txt", "./shader/psRgbShader.txt");
		mpTexRGB = GraphicsImageManager::getInstance()->createImage("videoTexRGB", vp.width, vp.height, GL_RGB);
		mpFboTex = GraphicsImageManager::getInstance()->createImage("fboTex", vp.width, vp.height, GL_RGB);
		mpFboTex2 = GraphicsImageManager::getInstance()->createImage("fboTex2", vp.width, vp.height, GL_RGB);
		mpFboTex3 = GraphicsImageManager::getInstance()->createImage("fboTex3", vp.width, vp.height, GL_RGB);
	}

	if (mpFboTex != 0)
	{
		mFbo.attachColorTexture(mpFboTex->getTexture());
	}

	if (mpFboTex2 != 0)
	{
		mFbo2.attachColorTexture(mpFboTex2->getTexture());
	}

	if (mpFboTex3 != 0)
	{
		mFbo3.attachColorTexture(mpFboTex3->getTexture());
	}
	/*GLfloat pos[] = {-1.0f,1.0f,0.0f,
	-1.0f,-1.0f,0.0f,
	1.0f,-1.0f,0.0f,
	1.0f,1.0f,0.0f };
	GLushort indexes[] = {0,1,2,0,2,3};
	GLfloat tex[] = {0.0f,1.0f,0.0f,0.0f,1.0f,0.0f,1.0f,1.0f};*/

	mMesh.loadMesh();


	UIManager::getInstance()->initUI("simkai.ttf", 640, 480);

	mpBt = UIManager::getInstance()->createButton("button1", 0);
	mpBt->setRect(100, 100, 100, 100);
	mpBt->setOnClickListener(this, &GLWindow::onButtonClick);
	return true;
}

void GLWindow::postSwapBuffer()
{
	/*if (mpSink != 0)
	{
	//format:GL_ALPHA, GL_RGB, and GL_RGBA.
	//type: GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_4_4_4_4, or GL_UNSIGNED_SHORT_5_5_5_1
	//only one pair of format/type is always accept:GL_RGBA/GL_UNSIGNED_BYTE
	//
	int format;
	int type;
	glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &format);
	glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &type);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);

	glReadPixels(0, 0, m_Width, m_Height, GL_RGBA, GL_UNSIGNED_BYTE, (void*)mpReadPixelData);
	checkglerror();
	mpSink->write(mpReadPixelData, m_Width*m_Height * 4, m_Width, m_Height, PIX_RGBA, 1);
	}*/
}

void GLWindow::onLButtonDown(int x, int y)
{
	UIManager::getInstance()->onLButtonDown(x, y);
}
void GLWindow::onMouseMove(int x, int y)
{
	UIManager::getInstance()->onMouseMove(x, y);
}
void GLWindow::onLButtonUp(int x, int y)
{
	UIManager::getInstance()->onLButtonUp(x, y);
}