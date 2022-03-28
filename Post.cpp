#include "Post.h"
#include "Mesh.h"
#include "Texture.h"

Post::Post() = default;

Post::~Post() = default;

void Post::initPost(int width, int height, std::shared_ptr<Mesh>& pMesh) {
	mpMesh = pMesh;
}