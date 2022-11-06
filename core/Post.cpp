#include "Post.h"
#include "Mesh.h"
#include "Texture.h"

Post::Post() = default;

Post::~Post() = default;

void Post::initPost(int width, int height, std::shared_ptr<Mesh>& pMesh) {
	mpMesh = pMesh;
}

std::shared_ptr<Texture> Post::doPost(std::shared_ptr<Post>& pPost,
	std::shared_ptr<Texture> inTex, 
	std::shared_ptr<Texture> outTex) {
	auto pCurPost = pPost;
	bool bProcessed = false;
	while (pCurPost) {
		if (pPost->processWrap(inTex, outTex)){
			std::swap(inTex, outTex);
			bProcessed = true;
		}
		pCurPost = pCurPost->mpNext;
	}
	return bProcessed ? outTex : inTex;
}