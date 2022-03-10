#include "Texture.h"

/*
* 将hdr的texture，转换为cubemap
*/
TextureSP convertHdrToCubicmap(const std::shared_ptr<Texture>& pHdr);

/*
* 从hdr的cubemap生成diffuse irradiance map,默认纹理宽高都为128
*/
TextureSP genDiffuseIrrMap(const std::shared_ptr<Texture>& pCube);

/*
* 从hdr的cubemap生成specular filter map,默认纹理宽高都为128
* 根据粗糙度，生成6个等级的specular pre-filter map，粗糙度越高越清晰
* 越低越模糊，其结果保存在
* 一个开启了mipmap的cubemap里面。
*/
TextureSP genSpecularFilterMap(const std::shared_ptr<Texture>& pCube);

/*
* 生成一张brdflut图像，pbr里面会用到
*/
TextureSP genBrdfLut();

TextureSP genSSAO();