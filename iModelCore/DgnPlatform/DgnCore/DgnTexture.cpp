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
    //
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnTexture::BindParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    auto const& bytes = m_data.GetBytes();
    BeAssert(0 < bytes.size());
    if (bytes.size() <= 0)
        return DgnDbStatus::BadArg;

    if (ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex(PROP_Descr), m_descr.c_str(), IECSqlBinder::MakeCopy::No)
        || ECSqlStatus::Success != stmt.BindBinary(stmt.GetParameterIndex(PROP_Data), &bytes[0], static_cast<int>(bytes.size()), IECSqlBinder::MakeCopy::No)
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
        default:
            return DgnTexture::Format::Unknown;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnTexture::_ExtractSelectParams(BeSQLite::EC::ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ExtractSelectParams(stmt, params);
    if (DgnDbStatus::Success == status)
        {
        m_descr.AssignOrClear(stmt.GetValueText(params.GetSelectIndex(PROP_Descr)));

        auto dataIdx = params.GetSelectIndex(PROP_Data);
        int dataSize = 0;
        m_data.m_bytes.clear();
        Byte const* data = static_cast<Byte const*>(stmt.GetValueBinary(dataIdx, &dataSize));
        BeAssert(dataSize > 0);
        m_data.m_bytes.insert(m_data.m_bytes.begin(), data, data + dataSize);

        m_data.m_format = extractFormat(stmt.GetValueInt(params.GetSelectIndex(PROP_Format)));
        m_data.m_width = static_cast<uint32_t>(stmt.GetValueInt(params.GetSelectIndex(PROP_Width)));
        m_data.m_height = static_cast<uint32_t>(stmt.GetValueInt(params.GetSelectIndex(PROP_Height)));
        m_data.m_flags = static_cast<DgnTexture::Flags>(stmt.GetValueInt(params.GetSelectIndex(PROP_Flags)));
        }

    return status;
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
        m_data = tx->GetData();
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
BentleyStatus DgnTexture::GetImage(bvector<Byte>& image) const
    {
    Data const& textureData = GetData();
    ImageUtilities::RgbImageInfo imageInfo;
    BentleyStatus status = ERROR;

    memset(&imageInfo, 0, sizeof (imageInfo));
    switch (textureData.GetFormat())
        {
        case DgnTexture::Format::RAW:
            image = textureData.GetBytes();
            return SUCCESS;

        case DgnTexture::Format::PNG:  
            status = ImageUtilities::ReadImageFromPngBuffer(image, imageInfo, &textureData.GetBytes().front(), textureData.GetBytes().size());
            break;

        case DgnTexture::Format::JPEG:
            {
            ImageUtilities::RgbImageInfo    jpegInfo;

            jpegInfo.width = textureData.GetWidth();
            jpegInfo.height = textureData.GetHeight();
            jpegInfo.hasAlpha = true;
            jpegInfo.isBGR = false;
            jpegInfo.isTopDown = true;
            
            status = ImageUtilities::ReadImageFromJpgBuffer(image, imageInfo, &textureData.GetBytes().front(), textureData.GetBytes().size(), jpegInfo);
            break;
            }
    
        default:
            BeAssert(false);
            return ERROR;
        }

    if (SUCCESS != status ||
        imageInfo.width != textureData.GetWidth() ||
        imageInfo.height != textureData.GetHeight() ||
        !imageInfo.hasAlpha)
        {
        BeAssert(false);
        return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureId DgnTexture::QueryTextureId(Code const& code, DgnDbR db)
    {
    return DgnTextureId(db.Elements().QueryElementIdByCode(code).GetValueUnchecked());
    }

