#include "Sampler.h"
#include <vector>
#include "Log.h"
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

void Sampler::unbind(unsigned texUnit) {
	glBindSampler(texUnit - GL_TEXTURE0, 0);
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
std::vector<std::shared_ptr<Sampler>> gSamplers;
std::shared_ptr<Sampler> Sampler::getSampler(SamplerType type) {
//#ifdef USE_SAMPLER
	size_t pos = static_cast<size_t>(type);
	if (pos < gSamplers.size()) {
		return gSamplers[pos];
	}
	else {
		auto pSampler = std::make_shared<Sampler>();
		if (pSampler) {
			pSampler->genSampler();
		}
		if (type == SamplerType::NearNearEdgeEdge) {
			pSampler->setParam(GL_NEAREST, GL_NEAREST);
		}
		else if (type == SamplerType::LinearLinearEdgeEdge) {
			pSampler->setParam(GL_LINEAR,GL_LINEAR);
		}
		else if (type == SamplerType::LML_LinearEdgeEdge) {
			pSampler->setParam(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
		}
		else if (type == SamplerType::NearNearBorderBorder) {
			pSampler->setParam(GL_NEAREST, GL_NEAREST,
				GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
		}
		else if (type == SamplerType::LinearLinearBorderBorder) {
			pSampler->setParam(GL_LINEAR, GL_LINEAR,
				GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
		}
		else if (type == SamplerType::LinearLinearBorderBorderEdgeLe) {
			pSampler->setParam(GL_LINEAR, GL_LINEAR,
				GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER,
				GL_COMPARE_REF_TO_TEXTURE,GL_LEQUAL);
		}
		else if (type == SamplerType::NearNearRepeatRepeat) {
			pSampler->setParam(GL_NEAREST, GL_NEAREST,
				GL_REPEAT, GL_REPEAT, GL_REPEAT);
		}
		else if (type == SamplerType::LinearLinearRepeatRepeat) {
			pSampler->setParam(GL_LINEAR, GL_LINEAR,
				GL_REPEAT, GL_REPEAT, GL_REPEAT);
		}
		gSamplers.emplace_back(pSampler);
		return pSampler;
	}
//#else
//	auto pSampler = std::make_shared<Sampler>();
//	if (type == SamplerType::NearNearEdgeEdge) {
//		pSampler->setMinMag(GL_NEAREST, GL_NEAREST);
//	}
//	else if (type == SamplerType::LinearLinearEdgeEdge) {
//		pSampler->setParam(GL_LINEAR, GL_LINEAR);
//	}
//	else if (type == SamplerType::NearNearBorderBorder) {
//		pSampler->setParam(GL_NEAREST, GL_NEAREST,
//			GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
//	}
//	else if (type == SamplerType::LinearLinearBorderBorder) {
//		pSampler->setParam(GL_LINEAR, GL_LINEAR,
//			GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
//	}
//	else if (type == SamplerType::NearNearRepeatRepeat) {
//		pSampler->setParam(GL_NEAREST, GL_NEAREST,
//			GL_REPEAT, GL_REPEAT, GL_REPEAT);
//	}
//	else if (type == SamplerType::LinearLinearRepeatRepeat) {
//		pSampler->setParam(GL_LINEAR, GL_LINEAR,
//			GL_REPEAT, GL_REPEAT, GL_REPEAT);
//	}
//}
//	return pSampler;
//		
//#endif
}