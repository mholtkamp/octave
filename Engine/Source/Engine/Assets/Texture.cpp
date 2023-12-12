#include "Assets/Texture.h"
#include "Renderer.h"
#include "Log.h"
#include "AssetManager.h"
#include "Engine.h"

#include <malloc.h>

#if EDITOR
#include <stb_image.h>
#include <stb_image_write.h>
#endif

using namespace std;

#define RGBA8_SIZE 4

#include "Graphics/Graphics.h"
#include "Graphics/GraphicsTypes.h"

static const char* sPixelFormatEnumStrings[] =
{
    "LA4",
    "RGB565",
    "RGBA8",
    "CMPR",
    "RGBA5551"
};
static_assert(
    uint32_t(PixelFormat::LA4) == 0 &&
    uint32_t(PixelFormat::RGB565) == 1 &&
    uint32_t(PixelFormat::RGBA8) == 2 &&
    uint32_t(PixelFormat::CMPR) == 3 &&
    uint32_t(PixelFormat::RGBA5551) == 4,
    "Need to update texture asset format string table");

const char* gFilterEnumStrings[] =
{
    "Nearest",
    "Linear"
};
static_assert(uint32_t(FilterType::Count) == 2, "Need to update filter type enum string table");

const char* gWrapEnumStrings[] =
{
    "Clamp",
    "Repeat",
    "Mirror"
};
static_assert(uint32_t(WrapMode::Count) == 3, "Need to update wrap mode enum string table");

FORCE_LINK_DEF(Texture);
DEFINE_ASSET(Texture);

bool Texture::HandlePropChange(Datum* datum, uint32_t index, const void* newValue)
{
    Property* prop = static_cast<Property*>(datum);
    OCT_ASSERT(prop != nullptr);
    Texture* texture = static_cast<Texture*>(prop->mOwner);
    bool success = false;

    if (prop->mName == "Mipmapped")
    {
        texture->mMipmapped = *(bool*)newValue;
        success = true;
    }
    else if (prop->mName == "Filter Type")
    {
        texture->mFilterType = *(FilterType*)newValue;
        success = true;
    }
    else if (prop->mName == "Wrap Mode")
    {
        texture->mWrapMode = *(WrapMode*)newValue;
        success = true;
    }

    if (success)
    {
#if EDITOR
        // Need to recreate the texture resource to show changes.
        GFX_DestroyTextureResource(texture);
        GFX_CreateTextureResource(texture, texture->mPixels);

        // Textures aren't normally meant to change at runtime so...
        // Kinda hacky, but refresh descriptors on all materials?
        AssetManager::Get()->DirtyAllMaterials();
#endif
    }

    return success;
}

bool UseCookedTextures(Platform platform)
{
    bool cook = false;

    if (platform == Platform::GameCube ||
        platform == Platform::Wii ||
        platform == Platform::N3DS)
    {
        cook = true;
    }

    return cook;
}

void CookTexture(Texture* texture, Platform platform, const std::vector<uint8_t>& srcPixels, std::vector<uint8_t>& outData)
{
#if EDITOR
    // (1) Save a temporary PNG in the Intermediate directory.
    std::string tempDir = GetEngineState()->mProjectDirectory + "Intermediate";
    const char* tempPng = "Temp.png";
    const char* tempOut = "Temp.tex";

    std::string pngPath = tempDir + "/" + tempPng;
    std::string outPath = tempDir + "/" + tempOut;
    if (!DoesDirExist(tempDir.c_str()))
    {
        CreateDir(tempDir.c_str());
    }

    // Check if texture is fully opaque
    bool opaque = true;
    for (uint32_t i = 0; i < srcPixels.size(); i += 4)
    {
        if (srcPixels[i + 3] != 0xff)
        {
            opaque = false;
            break;
        }
    }

    const uint32_t comps = 4;
    stbi_flip_vertically_on_write(platform == Platform::N3DS);
    stbi_write_png(pngPath.c_str(), texture->GetWidth(), texture->GetHeight(), comps, srcPixels.data(), texture->GetWidth() * 4);

    // (2) Exec platform-specific texture converter with relevant args, and output to another temp file in Intermediate.
    std::string cookCmd = "";

    switch (platform)
    {
    case Platform::GameCube:
    case Platform::Wii:
    {
#if PLATFORM_WINDOWS
        cookCmd += "gxtexconv.exe";
#else
        cookCmd += "$DEVKITPRO/tools/bin/gxtexconv";
#endif

        cookCmd += " -i ";
        cookCmd += pngPath.c_str();
        cookCmd += " -o ";
        cookCmd += outPath.c_str();
        cookCmd += " colfmt=";

        PixelFormat format = texture->GetFormat();

        // Alpha doesn't seem to be working with CMPR textures with gxtexconv, but I think
        // the CMPR does support 1 bit alpha. So I'm not sure what the problem is, but for now we can use a slightly
        // more compressed format for these.
        if (format == PixelFormat::CMPR && !opaque)
        {
            format = PixelFormat::RGBA5551;
        }

        switch (format)
        {
        case PixelFormat::LA4: cookCmd += "2"; break;
        case PixelFormat::RGB565: cookCmd += "4"; break;
        case PixelFormat::RGBA5551: cookCmd += "5"; break;
        case PixelFormat::CMPR: cookCmd += "14"; break;
        case PixelFormat::RGBA8: // Fallthrough to default
        default: cookCmd += "6"; break;
        }

        if (texture->IsMipmapped())
        {
            //int32_t maxLod = int32_t(texture->GetMipLevels()) - 1;
            int32_t maxLod = static_cast<int32_t>(floor(log2(std::min(texture->GetWidth(), texture->GetHeight()))) + 1) - 3;

            maxLod = glm::max(maxLod, 0);
            cookCmd += " mipmap=yes ";
            cookCmd += "minlod=0 ";
            cookCmd += "maxLod=";
            cookCmd += to_string(maxLod);
            cookCmd += " ";
        }
        else
        {
            cookCmd += " mipmap=no ";
        }

        cookCmd += "width=" + std::to_string(texture->GetWidth());
        cookCmd += " height=" + std::to_string(texture->GetHeight()) + " ";

        break;
    }
    case Platform::N3DS:
    {
#if PLATFORM_WINDOWS
        // Need to add devkitpro/tools/bin to your path. Sorry, couldn't access the directory in commandline using the env variable
        // since "/opt" doesn't exist. Not sure if I can replace "/opt/" with "C:/" ? ... Oh but it could be installed on a different drive...
        cookCmd += "tex3ds.exe";
#else
        cookCmd += "$DEVKITPRO/tools/bin/tex3ds";
#endif

        cookCmd += " -o ";
        cookCmd += outPath.c_str();
        cookCmd += " -f ";

        switch (texture->GetFormat())
        {
        case PixelFormat::LA4: cookCmd += "la4"; break;
        case PixelFormat::RGB565: cookCmd += "rgb565"; break;
        case PixelFormat::RGBA5551: cookCmd += "rgba5551"; break;
        case PixelFormat::CMPR: opaque ? (cookCmd += "etc1") : (cookCmd += "etc1a4"); break;
        case PixelFormat::RGBA8: // Fallthrough to default
        default: cookCmd += "rgba8"; break;
        }

        if (texture->IsMipmapped())
        {
            cookCmd += " -m triangle ";
        }
        else
        {
            cookCmd += " ";
        }

        cookCmd += pngPath.c_str();
        
        break;
    }

    default: OCT_ASSERT(0); break;
    }

    SYS_Exec(cookCmd.c_str());

    // (3) Use a stream to read the converted file, and copy all the data into outData
    Stream stream;
    stream.ReadFile(outPath.c_str(), false);
    outData.resize(stream.GetSize());
    memcpy(outData.data(), stream.GetData(), stream.GetSize());
#endif
}


Texture::Texture() :
    mWidth(0),
    mHeight(0),
    mMipLevels(1),
    mLayers(1),
    mFormat(PixelFormat::RGBA8),
    mFilterType(FilterType::Linear),
    mWrapMode(WrapMode::Repeat),
    mMipmapped(true),
    mRenderTarget(false)
{
    mType = Texture::GetStaticType();
}

Texture::~Texture()
{
    Destroy();
}

TextureResource* Texture::GetResource()
{
    return &mResource;
}

void Texture::LoadStream(Stream& stream, Platform platform)
{
    Asset::LoadStream(stream, platform);

    mWidth = stream.ReadUint32();
    mHeight = stream.ReadUint32();
    mMipLevels = stream.ReadUint32();
    mLayers = stream.ReadUint32();
    mFormat = (PixelFormat)stream.ReadUint32();
    mFilterType = (FilterType)stream.ReadUint32();
    mWrapMode = (WrapMode)stream.ReadUint32();

    mMipmapped = stream.ReadBool();
    mRenderTarget = stream.ReadBool();

    if (UseCookedTextures(platform))
    {
        uint32_t cookedDataSize = stream.ReadUint32();
        mPixels.resize(cookedDataSize);
        stream.ReadBytes(mPixels.data(), cookedDataSize);
    }
    else
    {
        int32_t size = (mWidth * mHeight * RGBA8_SIZE);
        mPixels.resize(size);

        for (int32_t i = 0; i < size; ++i)
        {
            mPixels[i] = stream.ReadUint8();
        }
    }
}

void Texture::SaveStream(Stream& stream, Platform platform)
{
    Asset::SaveStream(stream, platform);

#if EDITOR
    // For now, only allow saving in editor where mPixels is valid.
    // In the future, copy texture to buffer.
    stream.WriteUint32(mWidth);
    stream.WriteUint32(mHeight);
    stream.WriteUint32(mMipmapped ? mMipLevels : 1);
    stream.WriteUint32(mLayers);
    stream.WriteUint32(uint32_t(mFormat));
    stream.WriteUint32(uint32_t(mFilterType));
    stream.WriteUint32(uint32_t(mWrapMode));

    stream.WriteBool(mMipmapped);
    stream.WriteBool(mRenderTarget);

    if (UseCookedTextures(platform))
    {
        std::vector<uint8_t> cookedData;
        CookTexture(this, platform, mPixels, cookedData);
        uint32_t cookedDataSize = (uint32_t)cookedData.size();
        stream.WriteUint32(cookedDataSize);
        stream.WriteBytes(cookedData.data(), cookedDataSize);
    }
    else
    {
        // If not using an custom formats, just write out the raw RGBA8 pixels, uncompressed.
        OCT_ASSERT(mPixels.size() == (mWidth * mHeight * RGBA8_SIZE));
        for (int32_t i = 0; i < int32_t(mPixels.size()); ++i)
        {
            stream.WriteUint8(mPixels[i]);
        }
    }
#endif
}

void Texture::Create()
{
    Asset::Create();

    GFX_CreateTextureResource(this, mPixels);

#if !EDITOR
    // This pixel data is transferred to the GPU resource in GFX_CreateTextureResource(), so now 
    // we can clear the mPixels vector and shrink it so to free memory.
    // Keep copy of pixels when in editor so they can be saved without reading from the texture.
    mPixels.clear();
    mPixels.shrink_to_fit();
#endif
}

void Texture::Destroy()
{
    Asset::Destroy();

    GFX_DestroyTextureResource(this);
}

void Texture::Import(const std::string& path, ImportOptions* options)
{
    Asset::Import(path, options);

#if EDITOR
    int32_t texWidth;
    int32_t texHeight;
    int32_t texChannels;
    PixelFormat format = PixelFormat::RGBA8;

    stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

    uint32_t imageSize = texWidth * texHeight * 4;

    if (pixels == nullptr)
    {
        LogError("Failed to load texture image");
        OCT_ASSERT(0);
    }

    mPixels.resize(imageSize);
    memcpy(mPixels.data(), pixels, imageSize);


    mWidth = texWidth;
    mHeight = texHeight;
    mFormat = format;
    mRenderTarget = false;
    mMipmapped = true;
    mMipLevels = mMipmapped ? static_cast<int32_t>(floor(log2(std::max(mWidth, mHeight))) + 1) : 1;

    if (options != nullptr)
    {
        if (options->HasOption("mipmapped"))
        {
            mMipmapped = options->GetOptionValue("mipmapped");

            if (!mMipmapped)
            {
                mMipLevels = 1;
            }
        }
    }

    Create();

    stbi_image_free(pixels);
#endif
}

void Texture::GatherProperties(std::vector<Property>& outProps)
{
    Asset::GatherProperties(outProps);

    outProps.push_back(Property(DatumType::Bool, "Mipmapped", this, &mMipmapped));
    outProps.push_back(Property(DatumType::Integer, "Format", this, &mFormat, 1, Texture::HandlePropChange, 0, 5, sPixelFormatEnumStrings));
    outProps.push_back(Property(DatumType::Integer, "Filter Type", this, &mFilterType, 1, Texture::HandlePropChange, 0, int32_t(FilterType::Count), gFilterEnumStrings));
    outProps.push_back(Property(DatumType::Integer, "Wrap Mode", this, &mWrapMode, 1, Texture::HandlePropChange, 0, int32_t(WrapMode::Count), gWrapEnumStrings));
}

glm::vec4 Texture::GetTypeColor()
{
    return glm::vec4(1.0f, 0.3f, 0.3f, 1.0f);
}

const char* Texture::GetTypeName()
{
    return "Texture";
}

const char* Texture::GetTypeImportExt()
{
    return ".png";
}

void Texture::Init(uint32_t width, uint32_t height, uint8_t* data)
{
    OCT_ASSERT(width > 0);
    OCT_ASSERT(height > 0);
    OCT_ASSERT(data != nullptr);

    mWidth = width;
    mHeight = height;
    
    uint32_t imageSize = width * height * 4;
    mPixels.resize(imageSize);
    memcpy(mPixels.data(), data, imageSize);
}

void Texture::SetMipmapped(bool mipmapped)
{
    mMipmapped = mipmapped;
    mMipLevels = mMipmapped ? static_cast<int32_t>(floor(log2(std::max(mWidth, mHeight))) + 1) : 1;
}

bool Texture::IsMipmapped() const
{
    return mMipmapped;
}

bool Texture::IsRenderTarget() const
{
    return mRenderTarget;
}

uint32_t Texture::GetWidth() const
{
    return mWidth;
}

uint32_t Texture::GetHeight() const
{
    return mHeight;
}

uint32_t Texture::GetMipLevels() const
{
    return mMipLevels;
}

uint32_t Texture::GetLayers() const
{
    return mLayers;
}

PixelFormat Texture::GetFormat() const
{
    return mFormat;
}

FilterType Texture::GetFilterType() const
{
    return mFilterType;
}

WrapMode Texture::GetWrapMode() const
{
    return mWrapMode;
}

// These Set***() calls need to be called before Create().
void Texture::SetFormat(PixelFormat format)
{
    mFormat = format;
}

void Texture::SetFilterType(FilterType filterType)
{
    mFilterType = filterType;
}

void Texture::SetWrapMode(WrapMode wrapMode)
{
    mWrapMode = wrapMode;
}

