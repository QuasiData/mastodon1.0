#include "asset_loader.h"

#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STBI_WRITE_NO_STDIO

#pragma warning( push )
#pragma warning( disable : 4018 )
#pragma warning( disable : 4267 )
#include "tiny_gltf/tiny_gltf.h"
#pragma warning( pop )