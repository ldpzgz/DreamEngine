#ifndef _SAMPLER_H_
#define _SAMPLER_H_
#ifdef _GLES3
#include <GLES3/gl3.h>
#include <GLES3/gl31.h>
#include <GLES3/gl32.h>
#else
#include <glad/glad.h>
#endif
#include "Config.h"
#include <memory>
//#include <array>
enum class SamplerType {
	NearNearEdgeEdgeEdge,
	LinearLinearEdgeEdgeEdge,
	LML_LinearEdgeEdgeEdge,//linear_mipmap_linear
	NearNearBorderBorderBorder,
	LinearLinearBorderBorderBorder,
	LinearLinearBorderBorderEdgeLe,//compare mode is GL_COMPARE_REF_TO_TEXTURE,
	NearNearRepeatRepeatRepeat,
	LinearLinearRepeatRepeatRepeat,
	End
};
class Sampler {
public:
	Sampler()= default;
	~Sampler();
	void setParam(int minFilter,
		int magFilter,
		int wrapS = GL_CLAMP_TO_EDGE,
		int wrapT = GL_CLAMP_TO_EDGE,
		int wrapR = GL_CLAMP_TO_EDGE,
		int compareMode = GL_NONE,
		int compareFunc = GL_NEVER
	);
	void setMinMag(int minFilter,int magFilter);
	void setWrap(int r, int s, int t );
	void setCompare(int mode,int func);
	void genSampler();

	void bindTex(unsigned texUnit, float* borderColor);

	//void applyParams(unsigned int texUnit,float* borderColor,unsigned int target=GL_TEXTURE_2D);
	void unBind(int texUnit);

	//static std::shared_ptr<Sampler> getSampler(SamplerType type);

	static Sampler* getSampler(int minFilter,
		int magFilter,
		int wrapS = GL_CLAMP_TO_EDGE,
		int wrapT = GL_CLAMP_TO_EDGE,
		int wrapR = GL_CLAMP_TO_EDGE,
		int compareMode = GL_NONE,
		int compareFunc = GL_NEVER);
private:
	unsigned int mId{0};
	bool mbSetBorderColor{ false };
};

#endif