#include "Shader.h"
#include "Texture.h"
#include "material.h"
#include <fstream>
#include <set>
#include "Log.h"
#include "Utils.h"
#include "helper.h"
#include <algorithm>
#include <sstream>

using namespace std;

void Material::setDepthTest(bool b) {
	if (!mMyOpData) {
		mMyOpData = std::make_shared< OpData >();
	}
	mMyOpData->mbDepthTest = b;
}

void Material::setCullWhichFace(bool b, int whichFace) {
	if (!mMyOpData) {
		mMyOpData = std::make_shared< OpData >();
	}
	mMyOpData->mbCullFace = b;
	mMyOpData->mCullWhichFace = whichFace;
}

void Material::setBlend(bool b, unsigned int srcFactor, unsigned int destFactor,unsigned int blendOp,
	unsigned int srcFactorA, unsigned int destFactorA, unsigned int blendOpA) {
	if (!mMyOpData) {
		mMyOpData = std::make_unique< OpData >();
	}
	mMyOpData->mbBlendTest = b;
	mMyOpData->mBlendSrcFactor = srcFactor;
	mMyOpData->mBlendDstFactor = destFactor;
	mMyOpData->mBlendEquation = blendOp;
	mMyOpData->mBlendAlphaSrcFactor = srcFactorA;
	mMyOpData->mBlendAlphaDstFactor = destFactorA;
	mMyOpData->mBlendAlphaEquation = blendOpA;
}

void Material::setUniformColor(const Color& color) {
	mUniformColor = color;
}

void Material::setUniformColor(float r, float g, float b, float a) {
	mUniformColor = Color(r, g, b, a);
}

void Material::setAlbedoColor(float r, float g, float b) {
	mUniformColor = Color(r, g, b, 1.0f);
}

Material::Material()
{
}

Material::~Material() {
}

Material::Material(const Material& pMat) {
	mName = pMat.mName;
	mShader = pMat.mShader;
	mSamplerName2Texture = pMat.mSamplerName2Texture;
	mMyOpData = pMat.mMyOpData;
	mMetallical = pMat.mMetallical;
	mRoughness = pMat.mRoughness;
	mAo = pMat.mAo;
	mUniformColor = pMat.mUniformColor;
}

//深度测试，blend，cullface等操作
void Material::setMyRenderOperation() {
	if (mMyOpData) {
		if (!mOthersOpData) {
			mOthersOpData = std::make_shared<Material::OpData>();
		}
		if (mMyOpData->mbDepthTest >= 0) {
			GLboolean bDepthTest = true;
			glGetBooleanv(GL_DEPTH_TEST, &bDepthTest);
			mOthersOpData->mbDepthTest = bDepthTest;

			if (mMyOpData->mbDepthTest == 0) {
				glDisable(GL_DEPTH_TEST);
			}
			else {
				glEnable(GL_DEPTH_TEST);
			}
		}

		if (mMyOpData->mbBlendTest >= 0) {
			GLboolean bBlendTest = false;
			glGetBooleanv(GL_BLEND, &bBlendTest);
			mOthersOpData->mbBlendTest = bBlendTest;
			glGetIntegerv(GL_BLEND_SRC_RGB, &mOthersOpData->mBlendSrcFactor);
			glGetIntegerv(GL_BLEND_DST_RGB, &mOthersOpData->mBlendDstFactor);
			glGetIntegerv(GL_BLEND_EQUATION_RGB, &mOthersOpData->mBlendEquation);
			glGetIntegerv(GL_BLEND_SRC_ALPHA, &mOthersOpData->mBlendAlphaSrcFactor); 
			glGetIntegerv(GL_BLEND_DST_ALPHA, &mOthersOpData->mBlendAlphaDstFactor);
			glGetIntegerv(GL_BLEND_EQUATION_ALPHA, &mOthersOpData->mBlendAlphaEquation);
			if (mMyOpData->mbBlendTest == 0) {
				glDisable(GL_BLEND);
			}
			else {
				glEnable(GL_BLEND);
				glBlendFuncSeparate(mMyOpData->mBlendSrcFactor, mMyOpData->mBlendDstFactor,
					mMyOpData->mBlendAlphaSrcFactor, mMyOpData->mBlendAlphaDstFactor);
				glBlendEquationSeparate(mMyOpData->mBlendEquation,mMyOpData->mBlendAlphaEquation);
			}
		}
		
		if (mMyOpData->mbCullFace >= 0) {
			GLboolean preCullFace = false;
			int preCullFaceModel = GL_BACK;
			glGetBooleanv(GL_CULL_FACE, &preCullFace);
			mOthersOpData->mbCullFace = preCullFace;
			glGetIntegerv(GL_CULL_FACE_MODE, &mOthersOpData->mCullWhichFace);
			if (mMyOpData->mbCullFace == 0) {
				glDisable(GL_CULL_FACE);
			}
			else {
				glEnable(GL_CULL_FACE);
				glCullFace(mMyOpData->mCullWhichFace);
			}
		}
	}
}

//恢复之前的深度测试，blend，cullface等操作
void Material::restoreRenderOperation() {
	if (!mOthersOpData) {
		return;
	}
	if (mOthersOpData->mbDepthTest == 0) {
		glDisable(GL_DEPTH_TEST);
	}
	else if (mOthersOpData->mbDepthTest == 1) {
		glEnable(GL_DEPTH_TEST);
	}

	if (mOthersOpData->mbBlendTest == 0) {
		glDisable(GL_BLEND);
	}
	else if(mOthersOpData->mbBlendTest == 1) {
		glEnable(GL_BLEND);
		glBlendFuncSeparate(mOthersOpData->mBlendSrcFactor, mOthersOpData->mBlendDstFactor,
			mOthersOpData->mBlendAlphaSrcFactor, mOthersOpData->mBlendAlphaDstFactor);
		glBlendEquationSeparate(mOthersOpData->mBlendEquation, mOthersOpData->mBlendAlphaEquation);
	}

	if (mOthersOpData->mbCullFace== 0) {
		glDisable(GL_CULL_FACE);
	}
	else if(mOthersOpData->mbCullFace == 1) {
		glEnable(GL_CULL_FACE);
		glCullFace(mOthersOpData->mCullWhichFace);
	}
}

void Material::enable() {
	if (mShader) {
		mShader->setUniformColor(mUniformColor);
		mShader->setMetallic(mMetallical);
		mShader->setRoughness(mRoughness);
		mShader->setAo(mAo);
		mShader->enable();
		int texNum = 0;
		for (auto it = mSamplerName2Texture.begin(); it != mSamplerName2Texture.end(); it++) {
			if (it->second) {
				it->second->active(GL_TEXTURE0 + texNum);
				glUniform1i(it->first, texNum);
				++texNum;
			}
		}
	}
}

shared_ptr<Texture> Material::getTextureOfSampler(const string& samplerName) {
	if (mShader) {
		int samplerLoc = mShader->getUniformLoc(samplerName.c_str());
		if (samplerLoc != -1) {
			auto it = mSamplerName2Texture.find(samplerLoc);
			if (it != mSamplerName2Texture.end()) {
				return it->second;
			}
		}
	}
	return nullptr;
}

void Material::setTextureForSampler(int loc, const shared_ptr<Texture>& pTex) {
	if (loc != -1) {
		mSamplerName2Texture[loc] = pTex;
	}
}

void Material::setTextureForSampler(const char* samplerName, const shared_ptr<Texture>& pTex) {
	if (mShader) {
		int samplerLoc = mShader->getUniformLoc(samplerName);
		if (samplerLoc != -1) {
			//mShader->setTextureForSampler(samplerLoc,pTex);
			mSamplerName2Texture[samplerLoc] = pTex;
		}
		else {
			LOGD("no sampler %s,in material %s",samplerName,mName.c_str());
		}
	}
}

void Material::setUniform1fv(const std::string& name, int count, float* pdata) {
	if (mShader && !name.empty()) {
		mShader->setUniform1fv(name.c_str(), count, pdata);
	}
}