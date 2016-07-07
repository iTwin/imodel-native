/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnTexture.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
        uint32_t  m_width=0;
        uint32_t  m_height=0;
        Flags     m_flags=Flags::None;
        Utf8String m_descr;

        //! Constructor from base class. Chiefly for internal use.
        explicit CreateParams(DgnElement::CreateParams const& params, Utf8String descr="") : T_Super(params), m_descr(descr) {}

        //! Constructs parameters for creating a texture
        //! @param[in] db The DgnDb in which the texture is to reside
        //! @param[in] name The name of the texture - must be unique within the DgnDb.
        //! @param[in] data The data describing the texture's appearance
        //! @param[in] descr An optional description of the texture
        //! @param[in] height the height of the image
        //! @param[in] width the width of the image
        //! @param[in] flags optional flags
        CreateParams(DgnDbR db, Utf8StringCR name, Render::ImageSourceCR data, uint32_t width, uint32_t height, Utf8StringCR descr="", Flags flags=Flags::None) : 
                T_Super(db, DgnModel::DictionaryId(), QueryDgnClassId(db), CreateTextureCode(name)), m_data(data), m_width(width), m_height(height), m_flags(flags), m_descr(descr) {}
    };

private:
    uint32_t m_width=0;
    uint32_t m_height=0;
    Flags m_flags=Flags::None;
    Render::ImageSource m_data;
    Utf8String m_descr;

    DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement& stmt);
protected:
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement& statement, ECSqlClassParams const& selectParams) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement& stmt) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& stmt) override;
    DGNPLATFORM_EXPORT virtual void _CopyFrom(DgnElementCR source) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnDelete() const override;
    virtual DgnCode _GenerateDefaultCode() const override { return DgnCode::CreateEmpty(); }
    virtual bool _SupportsCodeAuthority(DgnAuthorityCR auth) const override { return ResourceAuthority::IsResourceAuthority(auth); }
public:
    static DgnTextureId ImportTexture(DgnImportContext& context, DgnTextureId source);

    //! Construct a new DgnTexture with the specified parameters
    explicit DgnTexture(CreateParams const& params) : T_Super(params), m_data(params.m_data), m_descr(params.m_descr) {}

    DgnTextureId GetTextureId() const {return DgnTextureId(GetElementId().GetValue());} //!< The texture ID.
    Utf8String GetTextureName() const {return GetCode().GetValue();} //!< The texture name

    Render::ImageSourceCR GetImageSource() const {return m_data;} //!< The image source
    Render::ImageSourceCR GetImageSourceR() {return m_data;} //!< A writable reference to the image source
    Utf8StringCR GetDescription() const {return m_descr;} //!< The description of this texture
    void SetImageSource(Render::ImageSourceCR data, uint32_t width, uint32_t height) {m_data = data; m_width=width; m_height=height;} //!< Set the image source
    void SetDescription(Utf8StringCR descr) {m_descr = descr;} //!< Set the description
    uint32_t GetWidth() const {return m_width;}
    uint32_t GetHeight() const {return m_height;}
    Flags GetFlags() const {return m_flags;}

    static ECN::ECClassId QueryECClassId(DgnDbR db) {return db.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_Texture);} //!< Return the class ID used for textures
    static DgnClassId QueryDgnClassId(DgnDbR db) {return DgnClassId(QueryECClassId(db));} //!< Return the class ID used for textures

    DgnTextureCPtr Insert(DgnDbStatus* status=nullptr) {return GetDgnDb().Elements().Insert<DgnTexture>(*this, status);} //!< Inserts the texture into the DgnDb and returns the persistent copy.
    DgnTextureCPtr Update(DgnDbStatus* status=nullptr) {return GetDgnDb().Elements().Update<DgnTexture>(*this, status);} //!< Updates the texture in the DgnDb and returns the persistent copy.

    // Creates a DgnCode for a texture with the specified name.
    DGNPLATFORM_EXPORT static DgnCode CreateTextureCode(Utf8StringCR textureName);

    //! Looks up the ID of a texture by DgnCode
    DGNPLATFORM_EXPORT static DgnTextureId QueryTextureId(DgnCode const& code, DgnDbR db);

    //! Looks up the ID of a texture by name
    static DgnTextureId QueryTextureId(Utf8StringCR textureName, DgnDbR db) {return QueryTextureId(CreateTextureCode(textureName), db);}

    //! Looks up a texture by ID
    static DgnTextureCPtr QueryTexture(DgnTextureId textureId, DgnDbR db) {return db.Elements().Get<DgnTexture>(textureId);}
};

namespace dgn_ElementHandler
{
    //=======================================================================================
    //! The handler for named textures
    //! @bsistruct                                                  Paul.Connelly   10/15
    //=======================================================================================
    struct Texture : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_Texture, DgnTexture, Texture, Element, DGNPLATFORM_EXPORT);
    protected:
        DGNPLATFORM_EXPORT virtual void _GetClassParams(ECSqlClassParams& params) override;
    };
}

END_BENTLEY_DGNPLATFORM_NAMESPACE
