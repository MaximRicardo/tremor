#include "texture_raylib.hpp"
#include <raylib.h>

TextureRaylib::LoadImageRet TextureRaylib::load_image(std::string path)
{
    Image img = LoadImage(path.c_str());

    LoadImageRet ret;
    ret.width = img.width;
    ret.height = img.height;

    for (int y = 0; y < img.height; ++y) {
        for (int x = 0; x < img.width; ++x) {
            Color color = GetImageColor(img, x, y);
            ret.data.push_back(color.r);
            ret.data.push_back(color.g);
            ret.data.push_back(color.b);
        }
    }

    return ret;
}
