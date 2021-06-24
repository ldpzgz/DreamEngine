#include "GLSurface.h"


GLbyte vShaderStrLine[] = "attribute vec4 vPosition;    \n"
	"void main()                  \n"
	"{                            \n"
	"   gl_Position = vPosition;  \n"
	"}                            \n";

GLbyte fShaderStrLine[] = "precision mediump float;\n"
	"void main()                                  \n"
	"{                                            \n"
	"  //gl_FragColor = texture2D(sTex,vTex);			\n"

	"  gl_FragColor = vec4(1.0f,0.0f,0.0f,1.0f);			\n"
	"}                                            \n";

GLbyte vShaderStr[] = "attribute vec4 vPosition;    \n"
	"attribute vec2 texPos;    \n"
	"varying vec2 vTex;    \n"
	"varying vec4 vPos;    \n"
	"void main()                  \n"
	"{                            \n"
	"   gl_Position = vPosition;  \n"
	"   vTex = texPos;  \n"
	"   vPos = gl_Position;  \n"
	"}                            \n";

GLbyte fShaderStr[] = "precision mediump float;\n"
	"uniform sampler2D sTex;    \n"
	"uniform sampler2D sTex1;    \n"
	"varying vec2 vTex;    \n"
	"varying vec4 vPos;    \n"
	"void main()                                  \n"
	"{                                            \n"
	"	if(vPos.y>=0.5f)                    \n"
	"{\n"
	"  gl_FragColor = texture2D(sTex,vTex);       \n"
	"}\n"
	"else\n"
	"{\n"
	"  gl_FragColor = texture2D(sTex1,vTex);       \n"
	"}\n"
	"  //gl_FragColor = texture2D(sTex,vTex);			\n"

	"  //gl_FragColor = vec4(1.0f,0.0f,0.0f,1.0f);			\n"
	"} \n";  

GLbyte YuvVshader[] = 
	"attribute vec4 vPosition;    \n"  
	"attribute vec2 texPos;   \n"  
	"varying vec2 vTex;     \n"
	"varying vec4 vpos;"
	"void main()                  \n"  
	"{                            \n"  
	"   gl_Position = vPosition;  \n"  
	"   vTex = texPos;  \n"
	" vpos = vPosition;\n"
	"}                            \n"; 
GLbyte YuvPShader[] =
	"precision mediump float;\n"
	"varying vec2 vTex;\n"  
	"varying vec4 vpos;\n"
	"uniform sampler2D SamplerY;\n"  
	"uniform sampler2D SamplerU;\n"  
	"uniform sampler2D SamplerV;\n"  
	"void main(void)\n"  
	"{\n"  
	" vec3 yuv;\n"  
	" vec3 rgb;\n"  
	"yuv.x = texture2D(SamplerY, vTex).r-0.5;\n"  
	"yuv.y = texture2D(SamplerU, vTex).r-0.5;\n"  
	"yuv.z = texture2D(SamplerV, vTex).r;\n"  
	"rgb = mat3( 1,   1,   1,\n"  
	"0,       -0.39465,  2.03211,\n"  
	"1.13983,   -0.58060,  0) * yuv;\n"
	"rgb.r = yuv.z+1.4075*yuv.y;\n"
	"rgb.g = yuv.z-0.3455*yuv.x-0.7169*yuv.y;\n"
	"rgb.b = yuv.z+1.779*yuv.x;\n"
	" rgb.r = (rgb.r<0.0)? 0.0 : (rgb.r>1.0 ? 1.0 : rgb.r); \n"
	" rgb.g = (rgb.g<0.0)? 0.0 : (rgb.g>1.0 ? 1.0 : rgb.g); \n"
	" rgb.b = (rgb.b<0.0)? 0.0 : (rgb.b>1.0 ? 1.0 : rgb.b); \n"
	"gl_FragColor = vec4(rgb,1.0);\n" 
	"}\n";

//GLbyte YuvPShader[] =
//	"varying vec2 vTex;\n"  
//	"uniform sampler2D SamplerY;\n"  
//	"uniform sampler2D SamplerU;\n"  
//	"uniform sampler2D SamplerV;\n"  
//	"void main(void)\n"  
//	"{\n"  
//	" vec4 color; \n"
//	" float y = texture2D(SamplerY, vTex).r; \n"
//	" float u = texture2D(SamplerU, vTex).r; \n"  
//	" float v = texture2D(SamplerV, vTex).r; \n"  
//	" color.r = y + 1.402*(v-0.5); \n" 
//	" color.g = y-0.34414*(u-0.5)-0.71414*(v-0.5); \n"
//	" color.b = y+1.772*(u-0.5); \n"   
//	" color.a = 1.0; \n"  
//	" color.r = (color.r<0.0)? 0.0 : (color.r>1.0 ? 1.0 : color.r); \n"
//	" color.g = (color.g<0.0)? 0.0 : (color.g>1.0 ? 1.0 : color.g); \n"
//	" color.b = (color.b<0.0)? 0.0 : (color.b>1.0 ? 1.0 : color.b); \n"
//	" gl_FragColor = vec4(color.r,color.g,color.b,1.0);\n"  
//	"}\n";

GLSurface::GLSurface()
{

}
GLSurface::~GLSurface()
{

}

void GLSurface::resize(int w,int h)
{
	OpenGL::resize(w,h);
	UIManager::getInstance()->setWindowWidthHeight(w,h);
}
void GLSurface::preDraw(void)
{
	OpenGL::preDraw();

	VideoDataInfo vdi = mDemuxer.getVideoData();
	PcmDataInfo pdi = mDemuxer.getAudioData(0);
	if(pdi.pData!=0)
	{
		delete[] pdi.pData;
	}
	if(vdi.pData[0]!=0)
	{
/*		unsigned char* y = vdi.pData[0];
		unsigned char* u = vdi.pData[1];
		unsigned char* v = vdi.pData[2];
		unsigned char* rgb = new unsigned char[vdi.width*vdi.height*3];
		int width = vdi.width;
		int height = vdi.height;
		for(int i=0; i<height; i++)
		{
			for(int j=0; j<width; j++)
			{
				int yy = y[i*width+j];
				int uu = u[(i/2)*(width/2)+j/2];
				int vv = v[(i/2)*(width/2)+j/2];
				
				int r = yy+1.4075*(vv-128);
				int g = yy - 0.3455*(uu-128) - 0.7169*(vv-128);
				int b = yy + 1.779*(uu-128);

				r = (r<0)?0:(r>255?255:r);
				g = (g<0)?0:(g>255?255:g);
				b = (b<0)?0:(b>255?255:b);

				rgb[i*width*3 + 3*j] = r;
				rgb[i*width*3 + 3*j+1] = g;
				rgb[i*width*3 + 3*j+2] = b;
			}
		}*/

		mTextrueY.update(0,0,vdi.width,vdi.height,vdi.pData[0]);
		mTextrueU.update(0,0,vdi.width/2,vdi.height/2,vdi.pData[1]);
		mTextrueV.update(0,0,vdi.width/2,vdi.height/2,vdi.pData[2]);
/*
		mTextrueY.update(0,0,vdi.width,vdi.height,rgb);

		delete[] rgb;*/

		delete[] vdi.pData[0];
	}
}
void GLSurface::draw()
{
	OpenGL::draw();
	mShader.useMe();
	int posloc = mShader.getAttributeLoc("vPosition");
	int texloc = mShader.getAttributeLoc("texPos");
	//int texUni;
	mTextrueY.active(0);
	int texUni = mShader.getUniformLoc("SamplerY");
	glUniform1i(texUni,0);
	
	mTextrueU.active(1);
	texUni = mShader.getUniformLoc("SamplerU");
	glUniform1i(texUni,1);

	mTextrueV.active(2);
	texUni = mShader.getUniformLoc("SamplerV");
	glUniform1i(texUni,2);

	int widthloc = mShader.getUniformLoc("imgwidth");
	glUniform1f(widthloc,512);
	int heightloc = mShader.getUniformLoc("imgheight");
	glUniform1f(heightloc,288);

	mMesh.drawMesh(posloc,texloc,-1);
	
	UIManager::getInstance()->drawUI();
}
void GLSurface::postDraw(void)
{
	OpenGL::postDraw();
}
bool GLSurface::postInit(int width,int height)
{
	float t[] = {-1.0f,1.0f,0.0f,
		-1.0f,-1.0f,0.0f,
		1.0f,-1.0f,0.0f,
	1.0f,1.0f,0.0f};


	float tex[] = {
		0.0f,0.0f,
		0.0f,1.0f,1.0f,1.0f,
		1.0f,0.0f
	};
	unsigned short int ti[] = {0,1,2,0,2,3};
	mMesh.loadMesh(t,sizeof(t),ti,sizeof(ti),tex,sizeof(tex));
	mShader.initShaderFromFile("shader/vsShader.txt","shader/psShader.txt");

	UIManager::getInstance()->setWindowWidthHeight(width,height);
	mpBt = UIManager::getInstance()->createButton("bt",0);
	mpBt->setRect(100,100,100,100);

	mpBt->setOnClickListener(std::make_shared<OnClickListenerImp<GLSurface>>(this,&GLSurface::onClick));

	mpSeekBar = UIManager::getInstance()->createSeekBar("sb",0);
	mpSeekBar->setRect(50,50,100,10);
	mpSeekBar->setOnPosChangedListener(std::make_shared<OnSeekBarPosChangeListenerImp<GLSurface>>(this,&GLSurface::onSeekPosChanged));
	//mTextrue.load("a.png");
	//mTextrue1.load("b.png");

	mDemuxer.openFile("haha.flv");
	//mDemuxer.openFile("f:/fnb.RMVB");

	VideoParam vp = mDemuxer.getVideoParam();
	//vp.pix_fmt = AV_PIX_FMT_RGB24;
	//mDemuxer.setVideoDstParam(vp);
	
	mTextrueY.load(vp.width,vp.height,0,GL_LUMINANCE);
	mTextrueU.load(vp.width/2,vp.height/2,0,GL_LUMINANCE);
	mTextrueV.load(vp.width/2,vp.height/2,0,GL_LUMINANCE);
	
	mDemuxer.start();

	return OpenGL::postInit(width,height);
}

void GLSurface::onSeekPosChanged(View* view,float newPos)
{
	SeekBar* seekBar = dynamic_cast<SeekBar*>(view);
	if(seekBar)
	{

	}
}

void GLSurface::onClick(View* view)
{
	Button* bt = dynamic_cast<Button*>(view);
	if(bt)
	{
		
	}
}

void GLSurface::onLButtonDown(int x,int y)
{
	UIManager::getInstance()->onLButtonDown(x,y);
}
void GLSurface::onMouseMove(int x,int y)
{
	UIManager::getInstance()->onMouseMove(x,y);
}
void GLSurface::onLButtonUp(int x,int y)
{
	UIManager::getInstance()->onLButtonUp(x,y);
}