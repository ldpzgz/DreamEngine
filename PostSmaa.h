#pragma once
#include "Post.h"
class Texture;
class Material;
class Mesh;
class Fbo;
class PostSmaa :public Post {
public:
	PostSmaa();
	~PostSmaa();
	void process(std::shared_ptr<Texture>& pInput,
		std::shared_ptr<Texture>& pOutput) override;
	void initPost(int width, int height, std::shared_ptr<Mesh>& pMesh) override;
private:
	std::shared_ptr<Texture> mpAreaTex;
	std::shared_ptr<Texture> mpSearchTex;
	std::shared_ptr<Texture> mpEdgeTex;//rg
	std::shared_ptr<Texture> mpWeightTex;//rgba
	std::shared_ptr<Material> mpEdgeMaterial;
	std::shared_ptr<Material> mpWeightsMaterial;
	std::shared_ptr<Material> mpBlendMaterial;

	std::unique_ptr<Fbo> mpFboDetectEdge;
	std::unique_ptr<Fbo> mpFboWeights;
	std::unique_ptr<Fbo> mpFboBlend;
};