#include "MeshCircle.h"

MeshCircle::MeshCircle():
	MeshFilledRect()
{
	mMeshType = MeshType::MESH_Circle;
}

MeshCircle::~MeshCircle() {

}

void MeshCircle::loadMesh(float width, float height, float centerX, float centerY) {
	mCenterX = centerX;
	mCenterY = centerY;
	mWidth = width;
	mHeight = height;
	int numOfVertex = 1026;
	GLfloat* pos = new GLfloat[numOfVertex * 3];
	GLfloat* tex = new GLfloat[numOfVertex * 2];
	if (pos)
	{
		float delta = 2.0f*3.1415926535897932f / 1024.0f;
		pos[0] = mCenterX;
		tex[0] = mCenterX / mWidth;
		pos[1] = mCenterY;
		tex[1] = mCenterY / mHeight;
		pos[2] = 0.0f;
		mPoints.emplace_back(pos[0], pos[1], pos[2]);
		for (int i = 1; i < numOfVertex; ++i)
		{
			pos[i * 3] = tex[i * 2] = 0.5f*cos((i - 1)*delta) + 0.5f;
			pos[i * 3 + 1] = tex[i * 2 + 1] = 0.5f*sin((i - 1)*delta) + 0.5f;
			pos[i * 3 + 2] = 0;
			pos[i * 3] *= width;
			pos[i * 3 + 1] *= height;
			mPoints.emplace_back(pos[i * 3], pos[i * 3 + 1], pos[i * 3 + 2]);
		}
		bool b = createBufferObject(pos, numOfVertex * 3 * sizeof(GLfloat), 
			numOfVertex,
			nullptr,0,
			tex,sizeof(GLfloat)* numOfVertex *2);
		if (!b)
		{
			LOGD("error to load circle mesh data\n");
		}
		
	}
	delete[] pos;
	delete[] tex;
}