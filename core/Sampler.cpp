#include "Sampler.h"
#include <vector>
#include "Log.h"
#include <unordered_map>

std::unordered_map<int, int> minMap{
	{GL_NEAREST,0},
	{GL_LINEAR,1},
	{GL_NEAREST_MIPMAP_NEAREST,2},
	{GL_LINEAR_MIPMAP_NEAREST,3},
	{GL_NEAREST_MIPMAP_LINEAR,4},
	{GL_LINEAR_MIPMAP_LINEAR,5},
};
std::unordered_map<int, int> magMap{
	{GL_NEAREST,0},
	{GL_LINEAR,1}
};

std::unordered_map<int, int> wrapMap{
	{GL_CLAMP_TO_EDGE,0},
	{GL_MIRRORED_REPEAT,1},
	{GL_REPEAT,2},
	{GL_CLAMP_TO_BORDER,3}
};

std::unordered_map<int, int> compareModeMap{
	{GL_NONE,0},
	{GL_COMPARE_REF_TO_TEXTURE,1}
};
std::unordered_map<int, int> compareFuncMap{
	{GL_LEQUAL,0},
	{GL_GEQUAL,1},
	{GL_LESS,2},
	{GL_GREATER,3},
	{GL_EQUAL,4},
	{GL_NOTEQUAL,5},
	{GL_ALWAYS,6},
	{GL_NEVER,7},
};

Sampler::~Sampler() {
	if (mId > 0) {
		glDeleteSamplers(1,&mId);
		mId = 0;
	}
}

void Sampler::setParam(int minFilter,
	int magFilter,
	int wrapS,
	int wrapT,
	int wrapR,
	int compareMode,
	int compareFunc
) {
	glSamplerParameteri(mId, GL_TEXTURE_MIN_FILTER, minFilter);
	glSamplerParameteri(mId, GL_TEXTURE_MAG_FILTER, magFilter);
	glSamplerParameteri(mId, GL_TEXTURE_WRAP_S, wrapS);
	glSamplerParameteri(mId, GL_TEXTURE_WRAP_T, wrapT);
	glSamplerParameteri(mId, GL_TEXTURE_WRAP_R, wrapR);
	glSamplerParameteri(mId, GL_TEXTURE_COMPARE_MODE, compareMode);
	glSamplerParameteri(mId, GL_TEXTURE_COMPARE_FUNC, compareFunc);
	if (wrapS == GL_CLAMP_TO_BORDER ||
		wrapT == GL_CLAMP_TO_BORDER ||
		wrapR == GL_CLAMP_TO_BORDER) {
		mbSetBorderColor = true;
	}
}

void Sampler::setMinMag(int minFilter, int magFilter) {
	glSamplerParameteri(mId, GL_TEXTURE_MIN_FILTER, minFilter);
	glSamplerParameteri(mId, GL_TEXTURE_MAG_FILTER, magFilter);
}

void Sampler::setWrap(int s, int t, int r) {
	glSamplerParameteri(mId, GL_TEXTURE_WRAP_S, s);
	glSamplerParameteri(mId, GL_TEXTURE_WRAP_T, t);
	glSamplerParameteri(mId, GL_TEXTURE_WRAP_R, r);
	if (s == GL_CLAMP_TO_BORDER ||
		t == GL_CLAMP_TO_BORDER ||
		r == GL_CLAMP_TO_BORDER) {
		mbSetBorderColor = true;
	}
}

void Sampler::setCompare(int mode, int func) {
	glSamplerParameteri(mId, GL_TEXTURE_COMPARE_MODE, mode);
	glSamplerParameteri(mId, GL_TEXTURE_COMPARE_FUNC, func);
}

void Sampler::bindTex(unsigned texUnit, float* borderColor) {
	glBindSampler(texUnit- GL_TEXTURE0, mId);
	if (mbSetBorderColor && borderColor != nullptr) {
		glSamplerParameterfv(mId, GL_TEXTURE_BORDER_COLOR, borderColor);
	}
}

//void Sampler::applyParams(unsigned int texId,float* borderColor,unsigned int target) {
//#ifndef USE_SAMPLER
//	if (texUnit > 0) {
//		glBindTexture(target, texId);
//		glTexParameteri(target, GL_TEXTURE_MIN_FILTER, mMinFilter);
//		glTexParameteri(target, GL_TEXTURE_MAG_FILTER, mMagFilter);
//		glTexParameteri(target, GL_TEXTURE_WRAP_S, mWrapParamS);
//		glTexParameteri(target, GL_TEXTURE_WRAP_T, mWrapParamT);
//		if (mTarget == GL_TEXTURE_CUBE_MAP) {
//			glTexParameteri(target, GL_TEXTURE_WRAP_R, mWrapParamR);
//		}
//		if (borderColor!=nullptr && (mTexParams.mWrapParamS == GL_CLAMP_TO_BORDER ||
//			mTexParams.mWrapParamT == GL_CLAMP_TO_BORDER ||
//			mTexParams.mWrapParamR == GL_CLAMP_TO_BORDER)) {
//			glTexParameterfv(target, GL_TEXTURE_BORDER_COLOR, borderColor);
//		}
//		if (mTexParams.mCompareMode != GL_NONE) {
//			glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, mCompareMode);
//			glTexParameteri(target, GL_TEXTURE_COMPARE_FUNC, mCompareFunc);
//		}
//	}
//#endif
//}

void Sampler::unBind(int texUnit) {
	glBindSampler(texUnit, 0);
}

void Sampler::genSampler() {
	if (mId == 0) {
		glGenSamplers(1, &mId);
	}
}
std::unordered_map<int,std::unique_ptr<Sampler>> gSamplers;

Sampler* Sampler::getSampler(int minFilter,
	int magFilter,
	int wrapS,
	int wrapT,
	int wrapR,
	int compareMode,
	int compareFunc) {
	int index = 0;
	auto minIt = minMap.find(minFilter);
	auto magIt = magMap.find(magFilter);
	auto wrapSIt = wrapMap.find(wrapS);
	auto wrapTIt = wrapMap.find(wrapT);
	auto wrapRIt = wrapMap.find(wrapR);
	auto modeIt = compareModeMap.find(compareMode);
	auto funcIt = compareFuncMap.find(compareFunc);
	if (minIt != minMap.end()) {
		index |= minIt->second;
	}
	else {
		index |= 1;
		minFilter = GL_LINEAR;
	}
	if (magIt != magMap.end()) {
		index |= magIt->second << 3;
	}
	else {
		index |= 1<<3;
		magFilter = GL_LINEAR;
	}

	if (wrapSIt != wrapMap.end()) {
		index |= wrapSIt->second << 4;
	}
	else {
		wrapS = GL_CLAMP_TO_EDGE;
	}
	if (wrapTIt != wrapMap.end()) {
		index |= wrapTIt->second << 6;
	}
	else {
		wrapT = GL_CLAMP_TO_EDGE;
	}
	if (wrapRIt != wrapMap.end()) {
		index |= wrapRIt->second << 8;
	}
	else {
		wrapR = GL_CLAMP_TO_EDGE;
	}
	
	if (modeIt != compareModeMap.end()) {
		index |= modeIt->second << 10;
	}
	else {
		compareMode = GL_NONE;
	}

	if (funcIt != compareFuncMap.end()) {
		index |= funcIt->second << 11;
	}
	else {
		compareFunc = GL_LEQUAL;
	}
	auto it = gSamplers.find(index);
	if ( it!= gSamplers.end()) {
		return it->second.get();
	}
	else {
		auto pSampler = std::make_unique<Sampler>();
		pSampler->genSampler();
		pSampler->setParam(minFilter, magFilter, wrapS, wrapT, wrapR, compareMode, compareFunc);
		Sampler* pS = pSampler.get();
		gSamplers.try_emplace(index, std::move(pSampler));
		return pS;
	}
}
//std::shared_ptr<Sampler> Sampler::getSampler(SamplerType type) {
////#ifdef USE_SAMPLER
//	size_t pos = static_cast<size_t>(type);
//	if (pos < gSamplers.size()) {
//		return gSamplers[pos];
//	}
//	else {
//		auto pSampler = std::make_shared<Sampler>();
//		if (pSampler) {
//			pSampler->genSampler();
//		}
//		if (type == SamplerType::NearNearEdgeEdgeEdge) {
//			pSampler->setParam(GL_NEAREST, GL_NEAREST);
//		}
//		else if (type == SamplerType::LinearLinearEdgeEdgeEdge) {
//			pSampler->setParam(GL_LINEAR,GL_LINEAR);
//		}
//		else if (type == SamplerType::LML_LinearEdgeEdgeEdge) {
//			pSampler->setParam(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
//		}
//		else if (type == SamplerType::NearNearBorderBorderBorder) {
//			pSampler->setParam(GL_NEAREST, GL_NEAREST,
//				GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
//		}
//		else if (type == SamplerType::LinearLinearBorderBorderBorder) {
//			pSampler->setParam(GL_LINEAR, GL_LINEAR,
//				GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
//		}
//		else if (type == SamplerType::LinearLinearBorderBorderEdgeLe) {
//			pSampler->setParam(GL_LINEAR, GL_LINEAR,
//				GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER,
//				GL_COMPARE_REF_TO_TEXTURE,GL_LEQUAL);
//		}
//		else if (type == SamplerType::NearNearRepeatRepeatRepeat) {
//			pSampler->setParam(GL_NEAREST, GL_NEAREST,
//				GL_REPEAT, GL_REPEAT, GL_REPEAT);
//		}
//		else if (type == SamplerType::LinearLinearRepeatRepeatRepeat) {
//			pSampler->setParam(GL_LINEAR, GL_LINEAR,
//				GL_REPEAT, GL_REPEAT, GL_REPEAT);
//		}
//		gSamplers.emplace_back(pSampler);
//		return pSampler;
//	}
//#else
//	auto pSampler = std::make_shared<Sampler>();
//	if (type == SamplerType::NearNearEdgeEdgeEdge) {
//		pSampler->setMinMag(GL_NEAREST, GL_NEAREST);
//	}
//	else if (type == SamplerType::LinearLinearEdgeEdgeEdge) {
//		pSampler->setParam(GL_LINEAR, GL_LINEAR);
//	}
//	else if (type == SamplerType::NearNearBorderBorderBorder) {
//		pSampler->setParam(GL_NEAREST, GL_NEAREST,
//			GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
//	}
//	else if (type == SamplerType::LinearLinearBorderBorderBorder) {
//		pSampler->setParam(GL_LINEAR, GL_LINEAR,
//			GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
//	}
//	else if (type == SamplerType::NearNearRepeatRepeatRepeat) {
//		pSampler->setParam(GL_NEAREST, GL_NEAREST,
//			GL_REPEAT, GL_REPEAT, GL_REPEAT);
//	}
//	else if (type == SamplerType::LinearLinearRepeatRepeatRepeat) {
//		pSampler->setParam(GL_LINEAR, GL_LINEAR,
//			GL_REPEAT, GL_REPEAT, GL_REPEAT);
//	}
//}
//	return pSampler;
//		
//#endif
//}