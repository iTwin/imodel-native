/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>

#define PROP_Data           "Data"
#define PROP_Description    "Description"
#define PROP_Format         "Format"
#define PROP_Width          "Width"
#define PROP_Height         "Height"
#define PROP_Flags          "Flags"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
namespace dgn_ElementHandler
{
    HANDLER_DEFINE_MEMBERS(Texture);
}
END_BENTLEY_DGNPLATFORM_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnTexture::_BindWriteParams(ECSqlStatement& stmt, ForInsert forInsert)
    {
    T_Super::_BindWriteParams(stmt, forInsert);

    BeAssert(0 < m_data.GetByteStream().GetSize());
    stmt.BindText(stmt.GetParameterIndex(PROP_Description), m_descr.c_str(), IECSqlBinder::MakeCopy::No);
    stmt.BindBlob(stmt.GetParameterIndex(PROP_Data), m_data.GetByteStream().GetData(), static_cast<int>(m_data.GetByteStream().GetSize()), IECSqlBinder::MakeCopy::No);
    stmt.BindInt(stmt.GetParameterIndex(PROP_Format), static_cast<int>(m_data.GetFormat()));
    stmt.BindInt(stmt.GetParameterIndex(PROP_Width), static_cast<int>(m_width));
    stmt.BindInt(stmt.GetParameterIndex(PROP_Height), static_cast<int>(m_height));
    stmt.BindInt(stmt.GetParameterIndex(PROP_Flags), static_cast<int>(m_flags));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
DgnDbStatus DgnTexture::_ReadSelectParams(BeSQLite::EC::ECSqlStatement& stmt, ECSqlClassParams const& params)
    {
    auto status = T_Super::_ReadSelectParams(stmt, params);
    if (DgnDbStatus::Success != status)
        return status;

    m_descr.AssignOrClear(stmt.GetValueText(params.GetSelectIndex(PROP_Description)));

    auto dataIdx = params.GetSelectIndex(PROP_Data);
    int dataSize = 0;
    m_data.GetByteStreamR().Clear();
    Byte const* data = static_cast<Byte const*>(stmt.GetValueBlob(dataIdx, &dataSize));
    BeAssert(dataSize > 0);
    m_data.GetByteStreamR().SaveData(data, dataSize);

    m_data.SetFormat(static_cast<ImageSource::Format>(stmt.GetValueInt(params.GetSelectIndex(PROP_Format))));
    m_width = static_cast<uint32_t>(stmt.GetValueInt(params.GetSelectIndex(PROP_Width)));
    m_height = static_cast<uint32_t>(stmt.GetValueInt(params.GetSelectIndex(PROP_Height)));
    m_flags = static_cast<DgnTexture::Flags>(stmt.GetValueInt(params.GetSelectIndex(PROP_Flags)));
    return DgnDbStatus::Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnTexture::_ToJson(JsonValueR out, JsonValueCR opts) const
    {
    T_Super::_ToJson(out, opts);
    if (!m_descr.empty())
        out[json_description()] = m_descr;

    out[json_data()] = m_data.GetByteStream().ToBase64();
    out[json_format()] = (int) m_data.GetFormat();
    out[json_width()] = m_width;
    out[json_height()] = m_height;
    out[json_flags()] = (int) m_flags;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   08/17
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnTexture::_FromJson(JsonValueR props)
    {
    T_Super::_FromJson(props);

    if (props.isMember(json_description()))
        m_descr = props[json_description()].asString();

    if (props.isMember(json_data()))
        m_data.GetByteStreamR().FromBase64(props[json_data()].asString());

    if (props.isMember(json_format()))
        m_data.SetFormat((ImageSource::Format)props[json_format()].asUInt());

    if (props.isMember(json_width()))
        m_width = props[json_width()].asUInt();

    if (props.isMember(json_height()))
        m_height = props[json_height()].asUInt();

    if (props.isMember(json_height()))
        m_flags = (Flags) props[json_flags()].asUInt();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   10/15
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnTexture::_CopyFrom(DgnElementCR src, CopyFromOptions const& opts)
    {
    T_Super::_CopyFrom(src, opts);
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
DgnTextureId DgnTexture::QueryTextureId(DgnDbR db, DgnCodeCR code)
    {
    return DgnTextureId(db.Elements().QueryElementIdByCode(code).GetValueUnchecked());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     10/2015
+---------------+---------------+---------------+---------------+---------------+------*/
DgnTextureId DgnTexture::ImportTexture(DgnImportContext& context, DgnTextureId source)
    {
    DgnTextureCPtr sourceTexture = DgnTexture::Get(context.GetSourceDb(), source);
    if (!sourceTexture.IsValid())
        {
        BeAssert(!source.IsValid()); //look up should fail only for an invalid Textureid
        return DgnTextureId();
        }

    // If the destination Db already contains a Texture by this name, then remap to it. Don't create another copy.
    DgnTextureId destTextureId = DgnTexture::QueryTextureId(context.GetDestinationDb(), sourceTexture->GetCode());
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
DgnTextureId DgnImportContext::_RemapTextureId(DgnTextureId source)
    {
    if (!IsBetweenDbs())
        return source;

    DgnTextureId dest = FindTextureId(source);
    return dest.IsValid() ? dest : DgnTexture::ImportTexture(*this, source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Sam.Wilson                      02/17
+---------------+---------------+---------------+---------------+---------------+------*/
void dgn_ElementHandler::Texture::_RegisterPropertyAccessors(ECSqlClassInfo& params, ECN::ClassLayoutCR layout)
    {
    T_Super::_RegisterPropertyAccessors(params, layout);

    params.RegisterPropertyAccessors(layout, PROP_Description,
        [] (ECValueR value, DgnElementCR elIn)
            {
            auto const& el = (DgnTexture const&) elIn;
            value.SetUtf8CP(el.GetDescription().c_str());
            return DgnDbStatus::Success;
            },
        [] (DgnElementR elIn, ECValueCR value)
            {
            if (!value.IsString())
                return DgnDbStatus::BadArg;
            auto& el = (DgnTexture&) elIn;
            el.SetDescription(value.ToString());
            return DgnDbStatus::Success;
            });

#define NOT_AVAILABLE_VIA_PROPERTY_API(PROPNAME)\
    params.RegisterPropertyAccessors(layout, PROPNAME,\
        [] (ECValueR value, DgnElementCR elIn)\
            {\
            BeAssert(false && "TBD"); return DgnDbStatus::BadRequest;\
            },\
        [] (DgnElementR elIn, ECValueCR value)\
            {\
            BeAssert(false && "TBD"); return DgnDbStatus::BadRequest;\
            });

    NOT_AVAILABLE_VIA_PROPERTY_API(PROP_Data)                      // *** WIP_TEXTURE
    NOT_AVAILABLE_VIA_PROPERTY_API(PROP_Format)                    // *** WIP_TEXTURE
    NOT_AVAILABLE_VIA_PROPERTY_API(PROP_Width)                     // *** WIP_TEXTURE
    NOT_AVAILABLE_VIA_PROPERTY_API(PROP_Height)                    // *** WIP_TEXTURE
    NOT_AVAILABLE_VIA_PROPERTY_API(PROP_Flags)                     // *** WIP_TEXTURE

#undef NOT_AVAILABLE_VIA_PROPERTY_API
    }