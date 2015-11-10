/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnTexture.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define PROP_Data   "Data"
#define PROP_Descr  "Descr"
#define PROP_Format "Format"
#define PROP_Width  "Width"
#define PROP_Height "Height"
#define PROP_Flags  "Flags"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
namespace dgn_ElementHandler
{
    HANDLER_DEFINE_MEMBERS(Texture);

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void Texture::_GetClassParams(ECSqlClassParams& params)
    {
    T_Super::_GetClassParams(params);
    params.Add(PROP_Data);
    params.Add(PROP_Descr);
    params.Add(PROP_Format);
    params.Add(PROP_Width);
    params.Add(PROP_Height);
    params.Add(PROP_Flags);
    }
}
END_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTexture::CreateParams::CreateParams(DgnDbR db, Utf8StringCR name, Data const& data, Utf8StringCR descr)
    : T_Super(db, QueryDgnClassId(db), CreateTextureCode(name)), m_data(data), m_descr(descr)
    {
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnTexture::BindParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    BeAssert(0 < m_data.GetSize());
    if (m_data.GetSize() <= 0)
        return DgnDbStatus::BadArg;

    if (ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex(PROP_Descr), m_descr.c_str(), IECSqlBinder::MakeCopy::No)
        || ECSqlStatus::Success != stmt.BindBinary(stmt.GetParameterIndex(PROP_Data), m_data.GetData(), static_cast<int>(m_data.GetSize()), IECSqlBinder::MakeCopy::No)
        || ECSqlStatus::Success != stmt.BindInt(stmt.GetParameterIndex(PROP_Format), static_cast<int>(m_data.GetFormat()))
        || ECSqlStatus::Success != stmt.BindInt(stmt.GetParameterIndex(PROP_Width), static_cast<int>(m_data.GetWidth()))
        || ECSqlStatus::Success != stmt.BindInt(stmt.GetParameterIndex(PROP_Height), static_cast<int>(m_data.GetHeight()))
        || ECSqlStatus::Success != stmt.BindInt(stmt.GetParameterIndex(PROP_Flags), static_cast<int>(m_data.GetFlags())))
        {
        return DgnDbStatus::BadArg;
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   08/15
+---------------+---------------+---------------+---------------+---------------+------*/
static DgnTexture::Format extractFormat(int value)
    {
    auto fmt = static_cast<DgnTexture::Format>(value);
    switch (fmt)
        {
        case DgnTexture::Format::JPEG:
        case DgnTexture::Format::RAW:
        case DgnTexture::Format::PNG:
        case DgnTexture::Format::TIFF:
            return fmt;
        }
    
    return DgnTexture::Format::Unknown;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnTexture::_ExtractSelectParams(BeSQLite::EC::ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ExtractSelectParams(stmt, params);
    if (DgnDbStatus::Success != status)
        return status;

    m_descr.AssignOrClear(stmt.GetValueText(params.GetSelectIndex(PROP_Descr)));

    auto dataIdx = params.GetSelectIndex(PROP_Data);
    int dataSize = 0;
    m_data.Clear();
    Byte const* data = static_cast<Byte const*>(stmt.GetValueBinary(dataIdx, &dataSize));
    BeAssert(dataSize > 0);
    m_data.SaveData(data, dataSize);

    m_data.m_format = extractFormat(stmt.GetValueInt(params.GetSelectIndex(PROP_Format)));
    m_data.m_width = static_cast<uint32_t>(stmt.GetValueInt(params.GetSelectIndex(PROP_Width)));
    m_data.m_height = static_cast<uint32_t>(stmt.GetValueInt(params.GetSelectIndex(PROP_Height)));
    m_data.m_flags = static_cast<DgnTexture::Flags>(stmt.GetValueInt(params.GetSelectIndex(PROP_Flags)));
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnTexture::_BindInsertParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindInsertParams(stmt);
    return DgnDbStatus::Success == status ? BindParams(stmt) : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnTexture::_BindUpdateParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    auto status = T_Super::_BindUpdateParams(stmt);
    return DgnDbStatus::Success == status ? BindParams(stmt) : status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnTexture::_CopyFrom(DgnElementCR src)
    {
    T_Super::_CopyFrom(src);
    auto tx = dynamic_cast<DgnTextureCP>(&src);
    BeAssert(nullptr != tx);
    if (nullptr != tx)
        {
        m_descr = tx->GetDescription();
        m_data = tx->GetTextureData();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnTexture::_OnDelete() const
    {
    return DgnDbStatus::DeletionProhibited; // purge only
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
Render::ImagePtr DgnTexture::ExtractImage() const
    {
    ImageUtilities::RgbImageInfo imageInfo;
    memset(&imageInfo, 0, sizeof (imageInfo));

    Render::ImagePtr image = new Render::Image(m_data.m_width, m_data.m_height);
    switch (m_data.GetFormat())
        {
        case DgnTexture::Format::RAW:
            image->GetByteStreamR() = std::move(m_data); // extracts the data
            break;

        case DgnTexture::Format::PNG:  
            ImageUtilities::ReadImageFromPngBuffer(image->GetByteStreamR(), imageInfo, m_data.GetData(), m_data.GetSize());
            break;

        case DgnTexture::Format::JPEG:
            {
            ImageUtilities::RgbImageInfo    jpegInfo;

            jpegInfo.width = m_data.GetWidth();
            jpegInfo.height = m_data.GetHeight();
            jpegInfo.hasAlpha = true;
            jpegInfo.isBGR = false;
            jpegInfo.isTopDown = true;
            
            ImageUtilities::ReadImageFromJpgBuffer(image->GetByteStreamR(), imageInfo, m_data.GetData(), m_data.GetSize(), jpegInfo);
            break;
            }
    
        default:
            BeAssert(false);
            return nullptr;
        }

    // This is tricky. We may have just transferred ownership of the image data to the the caller. That means that this
    // element is no longer in its "persistent" state. Drop if from the pool so if anyone else attempts to use it we will reload it.
    if (IsPersistent())
        GetDgnDb().Elements().DropFromPool(*this);

    return image;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureId DgnTexture::QueryTextureId(Code const& code, DgnDbR db)
    {
    return DgnTextureId(db.Elements().QueryElementIdByCode(code).GetValueUnchecked());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
#ifdef WIP_MERGE_YII
DgnTextureId DgnTextures::ImportTexture(DgnImportContext& context, DgnDbR sourceDb, DgnTextureId source)
    {
    Texture sourceTexture = sourceDb.Textures().Query(source);
    if (!sourceTexture.IsValid())
        {
        BeAssert(!source.IsValid() && "look up should fail only for an invalid Textureid");
        return DgnTextureId();
        }

    // If the destination Db already contains a Texture by this name, then remap to it. Don't create another copy.
    DgnTextureId destTextureId = context.GetDestinationDb().Textures().QueryTextureId (sourceTexture.GetName());
    if (destTextureId.IsValid())
        return destTextureId;

    //  Must copy and remap the source material.
    Texture destTexture(sourceTexture);

    Insert (destTexture);

    return context.AddTextureId(source, destTexture.GetId());
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureId DgnImportContext::RemapTextureId(DgnTextureId source)
    {
#ifdef WIP_MERGE_YII
    if (!IsBetweenDbs())
        return source;

    DgnTextureId dest = FindTextureId (source);
    if (dest.IsValid())
        return dest;

    return GetDestinationDb().Textures().ImportTexture(*this, GetSourceDb(), source);
#else
    return DgnTextureId();
#endif
    }
