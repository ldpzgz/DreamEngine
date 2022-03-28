#pragma once
#include "Post.h"
class Mesh;
class Texture;
class Fbo;
class Material;
class PostToneMap:public Post {
public:
	PostToneMap();
	~PostToneMap();
	void initPost(int width, int height, std::shared_ptr<Mesh>& pMesh) override;
	void process(std::shared_ptr<Texture>& pInput, 
		std::shared_ptr<Texture>& pOutput) override;
private:
	std::unique_ptr<Fbo> mpFbo;
	std::shared_ptr<Material> mpMaterial;
};