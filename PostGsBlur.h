#pragma once

#include "Post.h"
#include <array>
class Mesh;
class Texture;
class Fbo;
class Material;
class PostGsBlur:public Post {
public:
	PostGsBlur();
	~PostGsBlur();
	void initPost(int width, int height, std::shared_ptr<Mesh>& pMesh) override;
	void process(std::shared_ptr<Texture>& pInput, 
		std::shared_ptr<Texture>& pOutput) override;
private:
	std::unique_ptr<Fbo> mpFbo;
	std::shared_ptr<Material> mpMaterial;
	std::array<float,5> mWeights;
	std::array<float,5> mOffset;
	bool isVertical{ false };
};