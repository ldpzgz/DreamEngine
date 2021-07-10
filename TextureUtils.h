#ifndef _TEXTUREUTILS_H_
#define _TEXTUREUTILS_H_

#include <opencv2/opencv.hpp>
#include <iostream>
#include <memory>
#ifndef Texture
class Texture;
#endif
std::shared_ptr<Texture> loadImageFromFile(const std::string& path);
#endif
