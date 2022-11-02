/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatform/Render.h>
#include <DgnPlatform/PlatformLib.h>
#include "DgnDbWorker.h"
#include "IModelJsNative.h"

using namespace IModelJsNative;
USING_NAMESPACE_BENTLEY_RENDER;

//=======================================================================================
// @bsistruct
//=======================================================================================
struct TextureImageWorker : DgnDbWorker
{
private:
    DgnTextureId m_textureId;
    uint32_t m_maxSize;
    TexturePtr m_texture; // Is valid if texture found and no resizing needed, to avoid unnecessary copy of ImageSource.
    ImageSource m_resizedImage; // Is valid if texture found and needed resizing.
    uint32_t m_outWidth;
    uint32_t m_outHeight;
    ImageSource::Format m_outFormat;
    TextureTransparency m_outTransparency;

    TextureImageWorker(DgnDbR db, Napi::Env env, DgnTextureId textureId, uint32_t maxSize)
        : DgnDbWorker(db, env), m_textureId(textureId), m_maxSize(maxSize) {
         }

    void Execute() final;
    void OnOK() final;
public:
    static DgnDbWorkerPtr Create(DgnDbR db, Napi::CallbackInfo const& info);
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbWorkerPtr TextureImageWorker::Create(DgnDbR db, Napi::CallbackInfo const& info)
    {
    REQUIRE_ARGUMENT_ANY_OBJ(0, opts);

    DgnTextureId textureId;
    Napi::Value textureIdStr = opts.Get("name");
    if (textureIdStr.IsString())
        textureId = DgnTextureId(BeInt64Id::FromString(textureIdStr.ToString().Utf8Value().c_str()).GetValueUnchecked());

    if (!textureId.IsValid())
        THROW_JS_EXCEPTION("name property must be a valid Id64String");

    uint32_t maxSize = 0;
    Napi::Value maxTextureSizeNumber = opts.Get("maxTextureSize");
    if (!maxTextureSizeNumber.IsUndefined())
        {
        if (!maxTextureSizeNumber.IsNumber() || static_cast<int32_t>(maxTextureSizeNumber.ToNumber()) <= 0)
            THROW_JS_EXCEPTION("maxTextureSize property must be a positive number");

        maxSize = static_cast<uint32_t>(maxTextureSizeNumber.ToNumber());
        }

    return new TextureImageWorker(db, opts.Env(), textureId, maxSize);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TextureImageWorker::Execute()
    {
    auto system = T_HOST.Visualization().GetRenderSystem();
    if (nullptr == system)
        return;

    auto texture = system->_GetTexture(m_textureId, GetDb());
    if (texture.IsNull())
        return;

    auto imageSource = texture->GetImageSource();
    if (nullptr == imageSource)
        return;

    auto dimensions = texture->GetDimensions();
    m_outWidth = dimensions.width;
    m_outHeight = dimensions.height;
    m_outFormat = imageSource->GetFormat();
    m_outTransparency = texture->GetTransparency();
    if (0 == m_maxSize || (m_outWidth <= m_maxSize && m_outHeight <= m_maxSize))
        {
        m_texture = texture;
        return;
        }

    // Since we need to re-encode the image anyway, and we're dealing with a very large image, encode as JPEG if alpha channel is not needed.
    if (ImageSource::Format::Png == m_outFormat && TextureTransparency::Opaque == m_outTransparency)
        m_outFormat = ImageSource::Format::Jpeg;

    Image image(*imageSource, m_outFormat == ImageSource::Format::Png ? Image::Format::Rgba : Image::Format::Rgb);
    if (m_outWidth > m_outHeight) // xPrimary
        {
        double reduceScale = static_cast<double>(m_maxSize) / static_cast<double>(m_outWidth);
        m_outWidth = static_cast<uint32_t>(m_maxSize);
        m_outHeight = static_cast<uint32_t>(std::floor(m_outHeight * reduceScale));
        }
    else // yPrimary
        {
        double reduceScale = static_cast<double>(m_maxSize) / static_cast<double>(m_outHeight);
        m_outWidth = static_cast<uint32_t>(std::floor(m_outWidth * reduceScale));
        m_outHeight = static_cast<uint32_t>(m_maxSize);
        }

    auto scaledImage = Image::Scale(image, m_outWidth, m_outHeight);
    m_resizedImage = ImageSource(scaledImage, m_outFormat);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void TextureImageWorker::OnOK()
    {
    ImageSourceCP img = nullptr;
    if (m_resizedImage.IsValid())
        img = &m_resizedImage;
    else if (m_texture.IsValid())
        img = m_texture->GetImageSource();

    if (img)
        {
        auto const& bytes = img->GetByteStream();
        Napi::Uint8Array dataArray = Napi::Uint8Array::New(Env(), bytes.size());
        memcpy(dataArray.Data(), bytes.data(), bytes.size());
        Napi::Object texData = Napi::Object::New(Env());
        texData.Set("width", Napi::Number::New(Env(), (uint32_t) m_outWidth));
        texData.Set("height", Napi::Number::New(Env(), (uint32_t) m_outHeight));
        texData.Set("format", Napi::Number::New(Env(), (uint32_t) m_outFormat));
        texData.Set("transparency", Napi::Number::New(Env(), (uint32_t) m_outTransparency));
        texData.Set("bytes", dataArray);

        Promise().Resolve(texData);
        }
    else
        {
        Promise().Resolve(Env().Undefined());
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbWorkerPtr DgnDbWorker::CreateTextureImageWorker(DgnDbR db, Napi::CallbackInfo const& info)
    {
    return TextureImageWorker::Create(db, info);
    }

