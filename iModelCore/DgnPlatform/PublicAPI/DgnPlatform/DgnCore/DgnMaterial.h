/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/DgnMaterial.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDbTables.h"
#include "Render.h"

// JSon  Material Asset Keywords.
#define MATERIAL_ASSET_Rendering "RenderMaterial"

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! The DgnMaterials holds the materials defined for a DgnDb. Each material has a unique
//! combination of palette and material name, and an optional description and parent
//! material ID.
//! @see DgnDb::Materials
//=======================================================================================
struct DgnMaterials : DgnDbTable
{
private:
    friend struct DgnDb;
    explicit DgnMaterials(DgnDbR db) : DgnDbTable(db) {}

public:

    //=======================================================================================
    //! Holds a material's data in memory.
    //=======================================================================================
    struct Material
    {
    private:
        friend struct DgnMaterials;

        DgnMaterialId m_id;
        DgnMaterialId m_parentId;
        Utf8String    m_name;
        Utf8String    m_descr;
        Utf8String    m_palette;
        Utf8String    m_value;

    public:
        //! Constructs an empty, invalid Material
        Material() {}
        //! Constructs a Material for insertion into the materials table.
        //! @param[in]      name     The material's name. The combination of name and palette must be unique.
        //! @param[in]      palette  The name of the material's palette. The combination of name and palette must be unique.
        //! @param[in]      descr    Optional material description.
        //! @param[in]      parentId Optional ID of this material's parent material.
        Material(Utf8CP name, Utf8CP palette, Utf8CP descr=nullptr, DgnMaterialId parentId=DgnMaterialId()) : m_parentId(parentId), m_name(name), m_palette(palette), m_descr(descr) {}

        DgnMaterialId GetId() const {return m_id;}  //!< The ID of this material.
        DgnMaterialId GetParentId() const {return m_parentId;}  //!< The ID of this material's parent, or an invalid ID if no parent is defined.
        Utf8StringCR GetName() const {return m_name;}   //!< The name of this material.
        Utf8StringCR GetPalette() const {return m_palette;} //!< The name of this material's palette.
        Utf8StringCR GetValue() const {return m_value;} //!< JSON representation of this material.
        Utf8StringCR GetDescr() const {return m_descr;} //!< Description of this material.                                                                              
        void SetName(Utf8CP val) {m_name = val;} //!< Sets the name of this material.
        void SetPalette(Utf8CP val) {m_palette = val;} //!< Sets the name of this material's palette.
        void SetValue(Utf8CP val) {m_value = val;} //!< Sets the JSON representation of this material.
        void SetDescr(Utf8CP val) {m_descr= val;} //!< Sets the description of this material.
        void SetParentId(DgnMaterialId id) {m_parentId=id;} //!< Sets the parent material ID.
        bool IsValid() const {return m_id.IsValid();} //!< Test if the Material is valid.
        
        //! Get an asset of the material as a Json value.  (Rendering, physical etc.)
        //! @param[out] value  The Json value for the asset.
        //! @param[in]  keyword asset keyword -- "RenderMaterial", "Physical" etc.
        DGNPLATFORM_EXPORT BentleyStatus GetAsset(JsonValueR value, Utf8CP keyword) const; 

        //! Set an asset of material from a Json value.
        //! @param[in] value   The Json value for the asset.
        //! @param[in] keyword asset keyword -- "RenderMaterial", "Physical" etc.
        DGNPLATFORM_EXPORT void SetAsset(JsonValueCR value, Utf8CP keyword);

        //! Set the rendering material asset.
        void SetRenderingAsset(JsonValueCR value) {SetAsset(value, MATERIAL_ASSET_Rendering);}

        //! Get the rendering material asset.
        BentleyStatus GetRenderingAsset(JsonValueR value) {return GetAsset(value, MATERIAL_ASSET_Rendering);}
    };

    //! An iterator over the materials in a DgnDb
    struct Iterator : BeSQLite::DbTableIterator
    {
    public:
        explicit Iterator(DgnDbCR db) : DbTableIterator((BeSQLite::DbCR)db) {}

        //! An entry in the material table.
        struct Entry : DbTableIterator::Entry, std::iterator<std::input_iterator_tag, Entry const>
        {
        private:
            friend struct Iterator;
            Entry(BeSQLite::StatementP sql, bool isValid) : DbTableIterator::Entry(sql,isValid) {}
        public:
            DGNPLATFORM_EXPORT DgnMaterialId GetId() const; //!< The material ID.
            DGNPLATFORM_EXPORT DgnMaterialId GetParentId() const; //!< The parent material ID.
            DGNPLATFORM_EXPORT Utf8CP GetName() const; //!< The material name.
            DGNPLATFORM_EXPORT Utf8CP GetPalette() const; //!< The material palette name.
            DGNPLATFORM_EXPORT Utf8CP GetValue() const; //!< The JSON representation of the material.
            DGNPLATFORM_EXPORT Utf8CP GetDescr() const; //!< The material description.
            Entry const& operator*() const {return *this;}
        };

        typedef Entry const_iterator;
        typedef Entry iterator;
        DGNPLATFORM_EXPORT size_t QueryCount() const; //!< The number of entries in the material table.
        DGNPLATFORM_EXPORT Entry begin() const; //!< An iterator to the first entry in the table.
        Entry end() const {return Entry(nullptr, false);} //!< An iterator one beyond the last entry in the table.
    };

    //! Obtain an iterator over the materials in a DgnDb.
    Iterator MakeIterator() const {return Iterator(m_dgndb);}

    //! Insert a new material into the DgnDb. The combination of material+palette name must be unique.
    //! @param[in]      material    The new material
    //! @param[out]     result      If supplied, holds the result of the insert operation
    //! @return The DgnMaterialId of the newly created material, or an invalid ID if the material was not created.
    DGNPLATFORM_EXPORT DgnMaterialId Insert(Material& material, DgnDbStatus* result=nullptr);

    //! Remove a material from the DgnDb.
    //! @param[in] id the id of the material to remove.
    //! @return whether the delete statement succeeded. Note that this method will return BE_SQLITE_OK even if the materialId did not exist prior to this call.
    //! @note Deleting a material can result in an inconsistent database. There is no checking that the material to be removed is not in use somehow, and
    //! in general the answer to that question is nearly impossible to determine. It is very rarely possible to use this method unless you
    //! know for sure that the material is no longer necessary (for example, on a blank database). Otherwise, avoid using this method.
    DGNPLATFORM_EXPORT BeSQLite::DbResult Delete(DgnMaterialId id);

    //! Change the properties of the specified material. This method cannot be used to change the material or palette name.
    //! @param[in]      material The modified material.
    //! @return Success if the material was updated, or else an error code.
    DGNPLATFORM_EXPORT DgnDbStatus Update(Material const& material) const;

    //! Look up a material by ID.
    //! @param[in]      id The ID of the desired material
    //! @return The material with the specified ID, or an invalid material if no such ID exists.
    DGNPLATFORM_EXPORT Material Query(DgnMaterialId id) const;

    //! Look up the ID of the material with the specifed name and palette name.
    //! @param[in]      name    The material name
    //! @param[in]      palette The palette name
    //! @return The ID of the specified material, or an invalid ID if no such material exists.
    DGNPLATFORM_EXPORT DgnMaterialId QueryMaterialId(Utf8StringCR name, Utf8StringCR palette) const;
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
