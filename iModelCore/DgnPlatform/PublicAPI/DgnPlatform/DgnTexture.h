/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnTexture.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
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
struct EXPORT_VTABLE_ATTRIBUTE DgnTexture : DictionaryElement
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_Texture, DictionaryElement);
public:
    //! Supported texture formats. A texture's binary data is interpreted according to its specified format.
    enum class Format
    {
        JPEG        = 0,    //!< JPEG
        RAW         = 1,    //!< Raw RGBA bitmap
        PNG         = 2,    //!< PNG
        TIFF        = 3,    //!< TIFF
        Unknown     = 0xff  //!< Unrecognized texture format.
    };

    //! Optional flags which can be applied to a texture
    enum class Flags : uint32_t
    {
        None        = 0,    //!< No flags
    };

    //! Holds the raw texture data in memory
    struct TextureData : ByteStream
    {
    private:
        friend struct DgnTexture;
        uint32_t        m_width;
        uint32_t        m_height;
        Flags           m_flags;
        Format          m_format;

    public:
        //! Constructs an empty, invalid texture data
        TextureData() : m_flags(Flags::None), m_format(Format::Unknown) {}

        //! Constructor
        //! @param[in] format   The format of the raw texture data
        //! @param[in] data     The texture data encoded according to specified format. Must be non-null.
        //! @param[in] dataSize The number of byte in the texture data. Must be greater than 0
        //! @param[in] width    The width of the texture
        //! @param[in] height   The height of the texture
        //! @param[in] flags    Additional texture flags
        TextureData(Format format, Byte const* data, uint32_t dataSize, uint32_t width, uint32_t height, Flags flags = Flags::None)
            : ByteStream(data, dataSize), m_width(width), m_height(height), m_format(format), m_flags(flags)  {}
        Format GetFormat() const { return m_format; }//!< The format of the texture data
        uint32_t GetWidth() const { return m_width; } //!< The texture width
        uint32_t GetHeight() const { return m_height; }//!< The texture height
        Flags GetFlags() const { return m_flags; }//!< Texture flags
        };

    //! Parameters used to construct a DgnTexture
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(DgnTexture::T_Super::CreateParams);

        TextureData m_data;
        Utf8String  m_descr;

        //! Constructor from base class. Chiefly for internal use.
        explicit CreateParams(DgnElement::CreateParams const& params, TextureData const& data = TextureData(), Utf8String descr="") : T_Super(params), m_data(data), m_descr(descr) { }

        //! Constructs parameters for creating a texture
        //! @param[in]      db    The DgnDb in which the texture is to reside
        //! @param[in]      name  The name of the texture - must be unique within the DgnDb.
        //! @param[in]      data  The data describing the texture's appearance
        //! @param[in]      descr An optional description of the texture
        DGNPLATFORM_EXPORT CreateParams(DgnDbR db, Utf8StringCR name, TextureData const& data, Utf8StringCR descr="");
    };

private:
    mutable TextureData m_data;
    Utf8String  m_descr;

    DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement& stmt);
protected:
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ExtractSelectParams(BeSQLite::EC::ECSqlStatement& statement, ECSqlClassParams const& selectParams) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement& stmt) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& stmt) override;
    DGNPLATFORM_EXPORT virtual void _CopyFrom(DgnElementCR source) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnDelete() const override;
    DGNPLATFORM_EXPORT virtual Code _GenerateDefaultCode() override;

public:
    //! Construct a new DgnTexture with the specified parameters
    explicit DgnTexture(CreateParams const& params) : T_Super(params), m_data(params.m_data), m_descr(params.m_descr) { }

    DgnTextureId GetTextureId() const { return DgnTextureId(GetElementId().GetValue()); } //!< The texture ID.
    Utf8String GetTextureName() const { return GetCode().GetValue(); } //!< The texture name

    TextureData const& GetTextureData() const { return m_data; } //!< The texture data
    Utf8StringCR GetDescription() const { return m_descr; } //!< The description of this texture
    TextureData& GetTextureDataR() { return m_data; } //!< A writable reference to the texture data
    void SetTextureData(TextureData const& data) { m_data = data; } //!< Set the texture data
    void SetDescription(Utf8StringCR descr) { m_descr = descr; } //!< Set the description

    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_Texture); } //!< Return the class ID used for textures
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); } //!< Return the class ID used for textures

    DgnTextureCPtr Insert(DgnDbStatus* status=nullptr) { return GetDgnDb().Elements().Insert<DgnTexture>(*this, status); } //!< Inserts the texture into the DgnDb and returns the persistent copy.
    DgnTextureCPtr Update(DgnDbStatus* status=nullptr) { return GetDgnDb().Elements().Update<DgnTexture>(*this, status); } //!< Updates the texture in the DgnDb and returns the persistent copy.

    // Creates a Code for a texture with the specified name.
    DGNPLATFORM_EXPORT static Code CreateTextureCode(Utf8StringCR textureName);

    //! Looks up the ID of a texture by Code
    DGNPLATFORM_EXPORT static DgnTextureId QueryTextureId(Code const& code, DgnDbR db);

    //! Looks up the ID of a texture by name
    static DgnTextureId QueryTextureId(Utf8StringCR textureName, DgnDbR db) { return QueryTextureId(CreateTextureCode(textureName), db); }

    //! Looks up a texture by ID
    static DgnTextureCPtr QueryTexture(DgnTextureId textureId, DgnDbR db) { return db.Elements().Get<DgnTexture>(textureId); }

    DGNPLATFORM_EXPORT Render::ImagePtr ExtractImage() const; //!< The image data 
};

namespace dgn_ElementHandler
{
    //=======================================================================================
    //! The handler for named textures
    //! @bsistruct                                                  Paul.Connelly   10/15
    //=======================================================================================
    struct Texture : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_Texture, DgnTexture, Texture, Element, DGNPLATFORM_EXPORT);
    protected:
        DGNPLATFORM_EXPORT virtual void _GetClassParams(ECSqlClassParams& params) override;
    };
}

END_BENTLEY_DGNPLATFORM_NAMESPACE
