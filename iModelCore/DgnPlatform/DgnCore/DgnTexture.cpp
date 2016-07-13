/*--------------------------------------------------------------------------------------+
|
|     $Source: DgnCore/DgnTexture.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
void Texture::_TEMPORARY_GetPropertyHandlingCustomAttributes(ECSqlClassParams::PropertyHandlingCustomAttributes& params) // *** WIP_AUTO_HANDLED_PROPERTIES
    {
    T_Super::_TEMPORARY_GetPropertyHandlingCustomAttributes(params);
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
DgnDbStatus DgnTexture::BindParams(BeSQLite::EC::ECSqlStatement& stmt)
    {
    BeAssert(0 < m_data.GetByteStream().GetSize());
    if (m_data.GetByteStream().GetSize() <= 0)
        return DgnDbStatus::BadArg;

    if (ECSqlStatus::Success != stmt.BindText(stmt.GetParameterIndex(PROP_Descr), m_descr.c_str(), IECSqlBinder::MakeCopy::No)
        || ECSqlStatus::Success != stmt.BindBinary(stmt.GetParameterIndex(PROP_Data), m_data.GetByteStream().GetData(), static_cast<int>(m_data.GetByteStream().GetSize()), IECSqlBinder::MakeCopy::No)
        || ECSqlStatus::Success != stmt.BindInt(stmt.GetParameterIndex(PROP_Format), static_cast<int>(m_data.GetFormat()))
        || ECSqlStatus::Success != stmt.BindInt(stmt.GetParameterIndex(PROP_Width), static_cast<int>(m_width))
        || ECSqlStatus::Success != stmt.BindInt(stmt.GetParameterIndex(PROP_Height), static_cast<int>(m_height))
        || ECSqlStatus::Success != stmt.BindInt(stmt.GetParameterIndex(PROP_Flags), static_cast<int>(m_flags)))
        {
        return DgnDbStatus::BadArg;
        }

    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnTexture::_ReadSelectParams(BeSQLite::EC::ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success != status)
        return status;

    m_descr.AssignOrClear(stmt.GetValueText(params.GetSelectIndex(PROP_Descr)));

    auto dataIdx = params.GetSelectIndex(PROP_Data);
    int dataSize = 0;
    m_data.GetByteStreamR().Clear();
    Byte const* data = static_cast<Byte const*>(stmt.GetValueBinary(dataIdx, &dataSize));
    BeAssert(dataSize > 0);
    m_data.GetByteStreamR().SaveData(data, dataSize);

    m_data.SetFormat(static_cast<ImageSource::Format>(stmt.GetValueInt(params.GetSelectIndex(PROP_Format))));
    m_width = static_cast<uint32_t>(stmt.GetValueInt(params.GetSelectIndex(PROP_Width)));
    m_height = static_cast<uint32_t>(stmt.GetValueInt(params.GetSelectIndex(PROP_Height)));
    m_flags = static_cast<DgnTexture::Flags>(stmt.GetValueInt(params.GetSelectIndex(PROP_Flags)));
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
        m_data = tx->GetImageSource();
        m_height = tx->GetHeight();
        m_width = tx->GetWidth();
        m_flags = tx->GetFlags();
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
DgnTextureId DgnTexture::QueryTextureId(DgnCode const& code, DgnDbR db)
    {
    return DgnTextureId(db.Elements().QueryElementIdByCode(code).GetValueUnchecked());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureId DgnTexture::ImportTexture(DgnImportContext& context, DgnTextureId source)
    {
    DgnTextureCPtr sourceTexture = DgnTexture::QueryTexture(source, context.GetSourceDb());
    if (!sourceTexture.IsValid())
        {
        BeAssert(!source.IsValid()); //look up should fail only for an invalid Textureid
        return DgnTextureId();
        }

    // If the destination Db already contains a Texture by this name, then remap to it. Don't create another copy.
    DgnTextureId destTextureId = DgnTexture::QueryTextureId(sourceTexture->GetCode(), context.GetDestinationDb());
    if (destTextureId.IsValid())
        return destTextureId;

    //  Must copy and remap the source material.
    auto destTextureElem = sourceTexture->Import(nullptr, context.GetDestinationDb().GetDictionaryModel(), context);
    if (!destTextureElem.IsValid())
        return DgnTextureId();
    
    DgnTextureCP destTexture = dynamic_cast<DgnTextureCP>(destTextureElem.get());
    if (nullptr == destTexture)
        {
        BeAssert(false);
        return DgnTextureId();
        }

    return destTexture->GetTextureId();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureId DgnImportContext::RemapTextureId(DgnTextureId source)
    {
    if (!IsBetweenDbs())
        return source;

    DgnTextureId dest = FindTextureId(source);
    return dest.IsValid() ? dest : DgnTexture::ImportTexture(*this, source);
    }
