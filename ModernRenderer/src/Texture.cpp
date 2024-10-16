#include "Texture.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void Texture::LoadTextureData()
{
    int width, height, channels;
    unsigned char* image = stbi_load(path.c_str(), &width, &height, &channels, 0);
    if (image)
    {
        // Process the loaded image data
        // ...

        // Free the image data
        stbi_image_free(image);
    }
    else
    {
       	// Handle error
    }
}