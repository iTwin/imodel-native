/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "Render.h"
#include "ElementHandler.h"

DGNPLATFORM_TYPEDEFS(DgnTexture);
DGNPLATFORM_REF_COUNTED_PTR(DgnTexture);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! DgnTexture is an element representing a named texture. The texture data is stored as a
//! binary blob interpreted according to the specified texture format.
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnTexture : DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_Texture, DefinitionElement);

public:
    //! Optional flags which can be applied to a texture
    enum class Flags : uint32_t
    {
        None = 0,    //!< No flags
    };

    //! Parameters used to construct a DgnTexture
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(DgnTexture::T_Super::CreateParams);

        Render::ImageSource m_data;
        uint32_t m_width=0;
        uint32_t m_height=0;
        Flags m_flags=Flags::None;
        Utf8String m_descr;

        //! Constructor from base class. Chiefly for internal use.
        explicit CreateParams(DgnElement::CreateParams const& params, Utf8String descr="") : T_Super(params), m_descr(descr) {}

        //! Constructs parameters for creating a texture
        //! @param[in] model The DefinitionModel in which the texture is to reside
        //! @param[in] name The name of the texture - must be unique within the DgnDb.
        //! @param[in] data The data describing the texture's appearance
        //! @param[in] descr An optional description of the texture
        //! @param[in] height the height of the image
        //! @param[in] width the width of the image
        //! @param[in] flags optional flags
        CreateParams(DefinitionModelR model, Utf8StringCR name, Render::ImageSourceCR data, uint32_t width, uint32_t height, Utf8StringCR descr="", Flags flags=Flags::None) :
                T_Super(model.GetDgnDb(), model.GetModelId(), QueryDgnClassId(model.GetDgnDb()), CreateCode(model, name)), m_data(data), m_width(width), m_height(height), m_flags(flags), m_descr(descr) {}
    };

private:
    uint32_t m_width=0;
    uint32_t m_height=0;
    Flags m_flags=Flags::None;
    Render::ImageSource m_data;
    Utf8String m_descr;

protected:
    DGNPLATFORM_EXPORT DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, ECSqlClassParams const& selectParams) override;
    DGNPLATFORM_EXPORT void _ToJson(JsonValueR out, JsonValueCR opts) const override;
    DGNPLATFORM_EXPORT void _FromJson(JsonValueR props) override;
    DGNPLATFORM_EXPORT void _BindWriteParams(BeSQLite::EC::ECSqlStatement&, ForInsert) override;
    DGNPLATFORM_EXPORT void _CopyFrom(DgnElementCR source, CopyFromOptions const&) override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnDelete() const override;
    DgnCode _GenerateDefaultCode() const override { return DgnCode::CreateEmpty(); }
    bool _SupportsCodeSpec(CodeSpecCR codeSpec) const override { return !codeSpec.IsNullCodeSpec(); }

public:
    BE_JSON_NAME(description)
    BE_JSON_NAME(data)
    BE_JSON_NAME(format)
    BE_JSON_NAME(width)
    BE_JSON_NAME(height)
    BE_JSON_NAME(flags)

    static DgnTextureId ImportTexture(DgnImportContext& context, DgnTextureId source);

    //! Construct a new DgnTexture with the specified parameters
    explicit DgnTexture(CreateParams const& params) : T_Super(params), m_data(params.m_data), m_descr(params.m_descr) {}

    DgnTextureId GetTextureId() const {return DgnTextureId(GetElementId().GetValue());} //!< The texture ID.
    Utf8String GetTextureName() const {return GetCode().GetValue().GetUtf8();} //!< The texture name. Note that unnamed textures are permitted, in which case the name will be empty.

    Render::ImageSourceCR GetImageSource() const {return m_data;} //!< The image source
    Render::ImageSourceCR GetImageSourceR() {return m_data;} //!< A writable reference to the image source
    Utf8StringCR GetDescription() const {return m_descr;} //!< The description of this texture
    void SetImageSource(Render::ImageSourceCR data, uint32_t width, uint32_t height) {m_data = data; m_width=width; m_height=height;} //!< Set the image source
    void SetDescription(Utf8StringCR descr) {m_descr = descr;} //!< Set the description
    uint32_t GetWidth() const {return m_width;}
    uint32_t GetHeight() const {return m_height;}
    Flags GetFlags() const {return m_flags;}

    static ECN::ECClassId QueryECClassId(DgnDbR db) {return db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_Texture);} //!< Return the class ID used for textures
    static DgnClassId QueryDgnClassId(DgnDbR db) {return DgnClassId(QueryECClassId(db));} //!< Return the class ID used for textures

    DgnTextureCPtr Insert(DgnDbStatus* status=nullptr) {return GetDgnDb().Elements().Insert<DgnTexture>(*this, status);} //!< Inserts the texture into the DgnDb and returns the persistent copy.
    DgnTextureCPtr Update(DgnDbStatus* status=nullptr) {return GetDgnDb().Elements().Update<DgnTexture>(*this, status);} //!< Updates the texture in the DgnDb and returns the persistent copy.

    //! Create a DgnCode for a texture given a name that is meant to be unique within the scope of the specified DefinitionModel
    //! Create a DgnCode for a texture.
    //! If name is non-empty, the name must be unique among all textures within the scope of the specified DefinitionModel
    //! If name is empty, the code identifies an anonymous texture
    static DgnCode CreateCode(DefinitionModelCR scope, Utf8StringCR name) {return name.empty() ? DgnCode::CreateEmpty() : CodeSpec::CreateCode(BIS_CODESPEC_Texture, scope, name);}

    //! Looks up the ID of a texture by DgnCode
    DGNPLATFORM_EXPORT static DgnTextureId QueryTextureId(DgnDbR db, DgnCodeCR code);
    //! Looks up the DgnTextureId of a texture by model and name
    static DgnTextureId QueryTextureId(DefinitionModelCR model, Utf8StringCR name) {return QueryTextureId(model.GetDgnDb(), CreateCode(model, name));}

    //! Looks up a texture by ID
    static DgnTextureCPtr Get(DgnDbR db, DgnTextureId textureId) {return db.Elements().Get<DgnTexture>(textureId);}
};

namespace dgn_ElementHandler
{
    //=======================================================================================
    //! The handler for named textures
    //! @bsistruct                                                  Paul.Connelly   10/15
    //=======================================================================================
    struct Texture : Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_Texture, DgnTexture, Texture, Definition, DGNPLATFORM_EXPORT);
        DGNPLATFORM_EXPORT void _RegisterPropertyAccessors(ECSqlClassInfo&, ECN::ClassLayoutCR) override;
    };
}

END_BENTLEY_DGNPLATFORM_NAMESPACE
