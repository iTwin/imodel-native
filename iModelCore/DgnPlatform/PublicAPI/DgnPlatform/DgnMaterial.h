/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnMaterial.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDb.h"
#include "DgnElement.h"
#include "ElementHandler.h"
#include "ECSqlStatementIterator.h"

#define DGN_CLASSNAME_MaterialElement "MaterialElement"

// JSon  Material Asset Keywords.
#define MATERIAL_ASSET_Rendering "RenderMaterial"

DGNPLATFORM_TYPEDEFS(DgnMaterial);
DGNPLATFORM_REF_COUNTED_PTR(DgnMaterial);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Defines a material. Material elements are stored in the dictionary model and identified
//! by a palette name and material name. Each material's name must be unique within its
//! palette, and each palette name unique within its DgnDb.
//! @bsistruct                                                  Paul.Connelly   09/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnMaterial : DictionaryElement
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_MaterialElement, DictionaryElement);
public:
    //! Holds the data which describe a material
    struct Data
    {
        Utf8String  m_value;
        Utf8String  m_descr;

        //! Construct material data with the specified value (as JSON) and description
        Data(Utf8StringCR value="", Utf8StringCR descr="") { Init(value, descr); }

        //! Initialize this material data with the specified value (as JSON) and description
        void Init(Utf8StringCR value="", Utf8StringCR descr="") { m_value = value; m_descr = descr; }

        uint32_t GetMemSize() const { return static_cast<uint32_t>(sizeof(*this) + m_value.length() + m_descr.length()); }
    };

    //! Parameters used to construct a DgnMaterial
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(DgnMaterial::T_Super::CreateParams);

        Data    m_data;

        //! Constructor from base class. Primarily for internal use.
        explicit CreateParams(DgnElement::CreateParams const& params, Utf8StringCR value="", Utf8StringCR descr="") : T_Super(params), m_data(value, descr) { }

        //! Constructs parameters for a material with the specified values. Primarily for internal use.
        CreateParams(DgnDbR db, DgnModelId modelId, DgnClassId classId, Code code, DgnElementId id = DgnElementId(),
                     DgnElementId parent = DgnElementId(), Utf8StringCR value="", Utf8StringCR descr="")
            : T_Super(db, modelId, classId, code, id, parent), m_data(value, descr) { }

        //! Constructs parameters for creating a material.
        //! @param[in]      db               The DgnDb in which the material will reside
        //! @param[in]      paletteName      The name of the material's palette. This becomes the namespace of the material's Code.
        //! @param[in]      materialName     The name of the material. This becomes the value of the material's Code.
        //! @param[in]      value            A JSON string describing the material data.
        //! @param[in]      parentMaterialId Optional ID of the parent material. If specified, this material inherits and can override the parent's material data.
        //! @param[in]      descr            Optional description of the material.
        //! @note The combination of palette and material name must be unique within the DgnDb.
        DGNPLATFORM_EXPORT CreateParams(DgnDbR db, Utf8StringCR paletteName, Utf8StringCR materialName, Utf8StringCR value="",
                    DgnMaterialId parentMaterialId=DgnMaterialId(), Utf8StringCR descr="");

        Utf8String GetPaletteName() const { return m_code.GetNameSpace(); } //!< Return the palette name
        Utf8String GetMaterialName() const { return m_code.GetValue(); } //!< Return the material name
    };

private:
    Data    m_data;

    DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement& stmt);
protected:
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ExtractSelectParams(BeSQLite::EC::ECSqlStatement& statement, ECSqlClassParams const& selectParams) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement& stmt) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement& stmt) override;
    DGNPLATFORM_EXPORT virtual void _CopyFrom(DgnElementCR source) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _SetParentId(DgnElementId parentId) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnChildImport(DgnElementCR child, DgnModelR destModel, DgnImportContext& importer) const override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _OnDelete() const override;

    virtual uint32_t _GetMemSize() const override { return T_Super::_GetMemSize() + m_data.GetMemSize(); }
    virtual Code _GenerateDefaultCode() override { return Code(); }
//__PUBLISH_SECTION_END__
public:
    static DgnMaterialId ImportMaterial(DgnMaterialId source, DgnImportContext& importer);
//__PUBLISH_SECTION_START__
public:
    //! Construct a new DgnMaterial with the specified parameters
    explicit DgnMaterial(CreateParams const& params) : T_Super(params), m_data(params.m_data) { }

    DgnMaterialId GetMaterialId() const { return DgnMaterialId(GetElementId().GetValue()); } //!< Returns the Id of this material.
    Utf8String GetPaletteName() const { return GetCode().GetNameSpace(); } //!< Returns the palette name
    Utf8String GetMaterialName() const { return GetCode().GetValue(); } //!< Returns the material name

    Utf8StringCR GetValue() const { return m_data.m_value; } //!< Returns the material data as a JSON string
    Utf8StringCR GetDescr() const { return m_data.m_descr; } //!< Returns the material description
    DgnMaterialId GetParentMaterialId() const { return DgnMaterialId(GetParentId().GetValueUnchecked()); } //!< Returns the ID of this material's parent material
    DgnMaterialCPtr GetParentMaterial() const { return GetParentId().IsValid() ? GetDgnDb().Elements().Get<DgnMaterial>(GetParentId()) : nullptr; } //!< Returns this material's parent material, if one exists.

    void SetValue(Utf8StringCR value) { m_data.m_value = value; } //!< Sets the material data as a JSON string
    void SetDescr(Utf8StringCR descr) { m_data.m_descr = descr; } //!< Sets the material description

    static ECN::ECClassId QueryECClassId(DgnDbR db) { return db.Schemas().GetECClassId(DGN_ECSCHEMA_NAME, DGN_CLASSNAME_MaterialElement); } //!< Returns the class ID used for material elements.
    static DgnClassId QueryDgnClassId(DgnDbR db) { return DgnClassId(QueryECClassId(db)); } //!< Returns the class ID used for material elements.
    static ECN::ECClassCP QueryECClass(DgnDbR db) { return db.Schemas().GetECClass(QueryECClassId(db)); } //!< Looks up the ECClass used for material elements.

    DgnMaterialCPtr Insert(DgnDbStatus* status = nullptr) { return GetDgnDb().Elements().Insert<DgnMaterial>(*this, status); } //!< Inserts this material into the DgnDb and returns the persistent material.
    DgnMaterialCPtr Update(DgnDbStatus* status = nullptr) { return GetDgnDb().Elements().Update<DgnMaterial>(*this, status); } //!< Updates this material in the DgnDb and returns the updated persistent material

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
    BentleyStatus GetRenderingAsset(JsonValueR value) const {return GetAsset(value, MATERIAL_ASSET_Rendering);}

    //! Creates a Code for a material. The palette name serves as the namespace, and the material name as the value.
    DGNPLATFORM_EXPORT static DgnElement::Code CreateMaterialCode(Utf8StringCR paletteName, Utf8StringCR materialName);

    //! Looks up the ID of the material with the specified code.
    DGNPLATFORM_EXPORT static DgnMaterialId QueryMaterialId(DgnElement::Code const& code, DgnDbR db);

    //! Looks up the ID of the material with the specified palette + material name.
    static DgnMaterialId QueryMaterialId(Utf8StringCR paletteName, Utf8StringCR materialName, DgnDbR db) { return QueryMaterialId(CreateMaterialCode(paletteName, materialName), db); }

    //! Looks up a material by ID.
    static DgnMaterialCPtr QueryMaterial(DgnMaterialId materialId, DgnDbR db) { return db.Elements().Get<DgnMaterial>(materialId); }

    //! An entry in a material iterator
    struct Entry : ECSqlStatementEntry
    {
        friend struct ECSqlStatementIterator<Entry>;
        friend struct DgnMaterial;
    private:
        Entry(BeSQLite::EC::ECSqlStatement* stmt=nullptr) : ECSqlStatementEntry(stmt) { }
    public:
        DgnMaterialId GetId() const { return m_statement->GetValueId<DgnMaterialId>(0); } //!< The material ID
        Utf8CP GetName() const { return m_statement->GetValueText(1); } //!< The material name
        Utf8CP GetPalette() const { return m_statement->GetValueText(2); } //!< The palette name
        DgnMaterialId GetParentId() const { return m_statement->GetValueId<DgnMaterialId>(3); } //!< The parent material ID
        Utf8CP GetDescr() const { return m_statement->GetValueText(4); } //!< The material description
    };

    //! An iterator over the materials within a DgnDb
    struct Iterator : ECSqlStatementIterator<Entry>
    {
        //! Options controlling material iteration
        struct Options
        {
        private:
            friend struct Iterator;

            bool m_ordered;
            bool m_byPalette;
            bool m_byParent;
            Utf8String m_palette;
            DgnMaterialId m_parent;

            Options(Utf8StringCP palette, DgnMaterialId const* parent, bool ordered) : m_ordered(ordered),
                m_byPalette(nullptr != palette), m_palette(m_byPalette ? *palette : Utf8String()),
                m_byParent(nullptr != parent), m_parent(m_byParent ? *parent : DgnMaterialId()) { }
        public:
            //! Default options: Includes all materials , unordered
            Options() : Options(nullptr, nullptr, false) { }
            //! Filter by palette name and parent material, optionally ordered by palette name and then material name
            Options(Utf8StringCR paletteName, DgnMaterialId parentId, bool ordered=false) : Options(&paletteName, &parentId, ordered) { }
            
            //! Unfiltered, ordered
            static Options Ordered() { return Options(nullptr, nullptr, true); }

            //! Filter by a specific palette name, optionally ordered by material name
            static Options ByPalette(Utf8StringCR paletteName, bool ordered = false) { return Options(&paletteName, nullptr, ordered); }

            //! Filter by a specific parent material, optionally ordered by palette name and then material name
            static Options ByParentId(DgnMaterialId parentId, bool ordered = false) { return Options(nullptr, &parentId, ordered); }
        };

        static Iterator Create(DgnDbR db, Options const& options); //!< @private
    };

    //! Create an iterator over the materials within a DgnDb
    //! @param[in]      db      The DgnDb in which to query
    //! @param[in]      options Options controlling which materials to include and in what order
    //! @return An iterator with the specified options
    DGNPLATFORM_EXPORT static Iterator MakeIterator(DgnDbR db, Iterator::Options options=Iterator::Options());
};

namespace dgn_ElementHandler
{
    //=======================================================================================
    //! The handler for material elements.
    //! @bsistruct                                                  Paul.Connelly   09/15
    //=======================================================================================
    struct Material : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_MaterialElement, DgnMaterial, Material, Element, DGNPLATFORM_EXPORT);
    protected:
        DGNPLATFORM_EXPORT virtual void _GetClassParams(ECSqlClassParams& params) override;
    };
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

