/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnTexture.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDbTables.h"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE
//=======================================================================================
//! The DgnTextures holds the textures defined for a DgnDb. Texture data is stored as a
//! binary blob interpreted according to the specified texture format. Textures may be
//! optionally named, but names must be unique within the DgnDb.
//=======================================================================================
struct DgnTextures : DgnDbTable
{
private:
    friend struct DgnDb;
    explicit DgnTextures(DgnDbR db) : DgnDbTable(db) {}


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

    enum class Flags : uint32_t
    {
        None        = 0,    //!< No flags
    };

    //! Holds the raw texture data
    struct TextureData
    {
    private:
        friend struct DgnTextures;

        bvector<Byte>       m_data;
        uint32_t            m_width;
        uint32_t            m_height;
        Flags               m_flags;
        Format              m_format;
    public:
        //! Constructs an empty, invalid texture data
        TextureData() : m_width(0), m_height(0), m_flags(Flags::None), m_format(Format::Unknown) {}

        //! Constructor
        //! @param[in] format   The format of the raw texture data
        //! @param[in] data     The texture data encoded according to specified format. Must be non-null.
        //! @param[in] dataSize The number of byte in the texture data. Must be greater than 0
        //! @param[in] width    The width of the texture
        //! @param[in] height   The height of the texture
        //! @param[in] flags    Additional texture flags
        TextureData(Format format, ByteCP data, size_t dataSize, uint32_t width, uint32_t height, Flags flags = Flags::None)
            : m_data(data, data + dataSize), m_width(width), m_height(height), m_format(format), m_flags(flags)  {}

        Format GetFormat() const { return m_format; }//!< The format of the texture data
        uint32_t GetWidth() const { return m_width; } //!< The texture width
        uint32_t GetHeight() const { return m_height; }//!< The texture height
        bvector<Byte> const& GetData() const { return m_data; } //!< The raw texture data
        Flags GetFlags() const { return m_flags; }//!< Texture flags
        };

    //=======================================================================================
    //! Holds a texture's data in memory.
    //=======================================================================================
    struct Texture
    {
    private:
        friend struct DgnTextures;

        DgnTextureId        m_id;
        Utf8String          m_name;
        Utf8String          m_descr;
        TextureData         m_data;

    public:
        //! Constructs an empty, invalid texture
        Texture() {}
        //! Constructs a texture for insertion into the textures table
        //! @param[in]      data  The encoded texture data
        //! @param[in]      name  The optional name of this texture. Must be unique within the DgnDb.
        //! @param[in]      descr The optional texture description
        explicit Texture(TextureData const& data, Utf8CP name = nullptr, Utf8CP descr = nullptr) : m_data(data), m_name(name), m_descr(descr) {}

        bool IsValid() const { return m_id.IsValid(); } //!< Test whether this is a valid texture

        DgnTextureId GetId() const { return m_id; } //!< The ID of the texture
        Utf8StringCR GetName() const { return m_name; } //!< The optional texture name
        Utf8StringCR GetDescription() const { return m_descr; } //!< The optional texture description
        TextureData const& GetData() const { return m_data; } //!< The encoded texture data
        DGNPLATFORM_EXPORT BentleyStatus GetImage(bvector<Byte>& image) const; //!< The image data (RGBA).

        void SetName(Utf8CP name) { m_name = name; } //!< Set the texture name
        void SetDescription(Utf8CP descr) { m_descr = descr; } //!< Set the texture description
        void SetData(TextureData const& data) { m_data = data; } //!< Set the texture data
    };

    //! An iterator over the textures in a DgnDb
    struct Iterator : BeSQLite::DbTableIterator
    {
    public:
        explicit Iterator(DgnDbCR db) : DbTableIterator((BeSQLite::DbCR)db) {}

        //! An entry in the texture table
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct Iterator;
            Entry(BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql, isValid) {}
        public:
            DGNPLATFORM_EXPORT DgnTextureId GetId() const;          //!< The texture ID
            DGNPLATFORM_EXPORT Utf8CP       GetName() const;        //!< The texture name
            DGNPLATFORM_EXPORT Utf8CP       GetDescr() const;       //!< The texture description
            DGNPLATFORM_EXPORT Format       GetFormat() const;      //!< The texture format
            DGNPLATFORM_EXPORT Flags        GetFlags() const;       //!< The texture flags
            DGNPLATFORM_EXPORT uint32_t     GetWidth() const;       //!< The texture width
            DGNPLATFORM_EXPORT uint32_t     GetHeight() const;      //!< The texture height
            DGNPLATFORM_EXPORT size_t       GetDataSize() const;    //!< The number of bytes in the encoded texture data
            DGNPLATFORM_EXPORT ByteCP       GetDataBytes() const;   //!< The encoded texture data

            Entry const& operator*() const { return *this; }
        };

        typedef Entry const_iterator;
        typedef Entry iterator;

        DGNPLATFORM_EXPORT size_t QueryCount() const; //!< The number of entries in the texture table
        DGNPLATFORM_EXPORT Entry begin() const; //!< An iterator to the first entry in the table
        Entry end() const { return Entry(nullptr, false); }    //!< An iterator one beyond the last entry in the table
    };

    //! Obtain an iterator over the textures in a DgnDb.
    Iterator MakeIterator() const {return Iterator(m_dgndb);}

    //! Insert a new texture into the DgnDb. If the texture is named, its name must be unique.
    //! @param[in]      texture The new texture
    //! @param[out]     result  If supplied, holds the result of the insert operation
    //! @return The ID of the newly-created texture, or an invalid ID if insertion failed.
    DGNPLATFORM_EXPORT DgnTextureId Insert(Texture& texture, DgnDbStatus* result = nullptr);

    //! Change the properties of the specified texture. This method cannot be used to change the texture name
    //! @param[in]      texture The modified texture.
    //! @return Success if the texture was updated, or else an error code.
    DGNPLATFORM_EXPORT DgnDbStatus Update(Texture const& texture) const;

    //! Look up a texture by ID.
    //! @param[in]      id The ID of the desired texture
    //! @return The texture with the specified ID, or an invalid texture if no such texture exists.
    DGNPLATFORM_EXPORT Texture Query(DgnTextureId id) const;

    //! Remove a texture from the DgnDb.
    //! @param[in] id the id of the texture to remove.
    //! @return whether the delete statement succeeded. Note that this method will return BE_SQLITE_OK even if the textureId did not exist prior to this call.
    //! @note Deleting a texture can result in an inconsistent database. There is no checking that the texture to be removed is not in use somehow, and
    //! in general the answer to that question is nearly impossible to determine. It is very rarely possible to use this method unless you
    //! know for sure that the texture is no longer necessary (for example, on a blank database). Otherwise, avoid using this method.
    DGNPLATFORM_EXPORT BeSQLite::DbResult Delete(DgnTextureId id);

    //! Look up the ID of the texture with the specified name
    //! @param[in]      name The name of the desired texture
    //! @return The ID of the texture with the specified name, or an invalid ID if no such texture exists.
    DGNPLATFORM_EXPORT DgnTextureId QueryTextureId(Utf8StringCR name) const;

};

END_BENTLEY_DGNPLATFORM_NAMESPACE
