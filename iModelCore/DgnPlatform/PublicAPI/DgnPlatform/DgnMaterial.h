/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnMaterial.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__PUBLISH_SECTION_START__

#include "DgnDb.h"
#include "DgnElement.h"
#include "Render.h"
#include "RenderMaterial.h"
#include "ECSqlStatementIterator.h"

// JSon  Material Asset Keywords.
#define MATERIAL_ASSET_Rendering "RenderMaterial"

DGNPLATFORM_TYPEDEFS(DgnMaterial);
DGNPLATFORM_REF_COUNTED_PTR(DgnMaterial);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_ElementHandler
{
    struct Material;
}
    
//=======================================================================================
//! Defines a material. Material elements are stored in the dictionary model and identified
//! by a palette name and material name. Each material's name must be unique within its
//! palette, and each palette name unique within its DgnDb.
//! @bsistruct                                                  Paul.Connelly   09/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnMaterial : DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_MaterialElement, DefinitionElement);
    friend struct dgn_ElementHandler::Material;

public:

    //! Parameters used to construct a DgnMaterial
    struct CreateParams : T_Super::CreateParams
    {
        DEFINE_T_SUPER(DgnMaterial::T_Super::CreateParams);

        //! Constructor from base class. Primarily for internal use.
        explicit CreateParams(DgnElement::CreateParams const& params) : T_Super(params) {}

        //! Constructs parameters for creating a material.
        //! @param[in]      db               The DgnDb in which the material will reside
        //! @param[in]      paletteName      The name of the material's palette. This becomes the namespace of the material's DgnCode.
        //! @param[in]      materialName     The name of the material. This becomes the value of the material's DgnCode.
        //! @param[in]      parentMaterialId Optional ID of the parent material. If specified, this material inherits and can override the parent's material data.
        //! @note The combination of palette and material name must be unique within the DgnDb.
        DGNPLATFORM_EXPORT CreateParams(DgnDbR db, Utf8StringCR paletteName, Utf8StringCR materialName, DgnMaterialId parentMaterialId=DgnMaterialId());

        Utf8String GetPaletteName() const {return m_code.GetScope();} //!< Return the palette name
        Utf8String GetMaterialName() const {return m_code.GetValue();} //!< Return the material name
    };

protected:
    static Utf8CP constexpr str_Assets() {return "Assets";}
    JsonValueCR GetAssets() const {return m_jsonProperties[str_Assets()];}
    JsonValueR GetAssetsR() {return m_jsonProperties[str_Assets()];}

    DGNPLATFORM_EXPORT DgnDbStatus _SetParentId(DgnElementId parentId, DgnClassId parentRelClassId) override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnChildImport(DgnElementCR child, DgnModelR destModel, DgnImportContext& importer) const override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnDelete() const override;
    DGNPLATFORM_EXPORT void _RemapIds(DgnImportContext& importer) override;

    DgnCode _GenerateDefaultCode() const override {return DgnCode();}
    bool _SupportsCodeSpec(CodeSpecCR codeSpec) const override {return !codeSpec.IsNullCodeSpec();}
    
public:
    static DgnMaterialId ImportMaterial(DgnMaterialId source, DgnImportContext& importer);

    //! Construct a new DgnMaterial with the specified parameters
    explicit DgnMaterial(CreateParams const& params) : T_Super(params) {}

    DgnMaterialId GetMaterialId() const {return DgnMaterialId(GetElementId().GetValue());} //!< Returns the Id of this material.
    Utf8String GetPaletteName() const {return GetCode().GetScope();} //!< Returns the palette name
    Utf8String GetMaterialName() const {return GetCode().GetValue();} //!< Returns the material name

    DgnMaterialId GetParentMaterialId() const {return DgnMaterialId(GetParentId().GetValueUnchecked());} //!< Returns the ID of this material's parent material
    DgnMaterialCPtr GetParentMaterial() const {return GetParentId().IsValid() ? GetDgnDb().Elements().Get<DgnMaterial>(GetParentId()) : nullptr;} //!< Returns this material's parent material, if one exists.

    static ECN::ECClassId QueryECClassId(DgnDbR db) {return db.Schemas().GetECClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_MaterialElement);} //!< Returns the class ID used for material elements.
    static DgnClassId QueryDgnClassId(DgnDbR db) {return DgnClassId(QueryECClassId(db));} //!< Returns the class ID used for material elements.
    static ECN::ECClassCP QueryECClass(DgnDbR db) {return db.Schemas().GetECClass(QueryECClassId(db));} //!< Looks up the ECClass used for material elements.

    DgnMaterialCPtr Insert(DgnDbStatus* status = nullptr) {return GetDgnDb().Elements().Insert<DgnMaterial>(*this, status);} //!< Inserts this material into the DgnDb and returns the persistent material.
    DgnMaterialCPtr Update(DgnDbStatus* status = nullptr) {return GetDgnDb().Elements().Update<DgnMaterial>(*this, status);} //!< Updates this material in the DgnDb and returns the updated persistent material

    //! Get an asset of the material as a Json value.
    JsonValueCR GetAsset(Utf8CP asset) const {return GetAssets()[asset];}
    JsonValueR GetAssetR(Utf8CP asset) {return GetAssetsR()[asset];}

    //! Set an asset of material from a Json value.
    //! @param[in] name asset name -- "RenderMaterial", "Physical" etc.
    //! @param[in] value The Json value for the asset.
    void SetAsset(Utf8CP name, JsonValueCR value) {GetAssetsR()[name] = value;}

    //! Get the rendering asset.
    RenderingAssetCR GetRenderingAsset() const {return (RenderingAssetCR) GetAsset(MATERIAL_ASSET_Rendering);}
    RenderingAssetR GetRenderingAssetR() {return (RenderingAssetR) GetAsset(MATERIAL_ASSET_Rendering);}

    void SetRenderingAsset(JsonValueCR val) {GetAssetR(MATERIAL_ASSET_Rendering) = val;}

    //! Creates a DgnCode for a material. The palette name serves as the namespace, and the material name as the value.
    static DgnCode CreateCode(DgnDbR db, Utf8StringCR paletteName, Utf8StringCR materialName) {return CodeSpec::CreateCode(db, BIS_CODESPEC_MaterialElement, materialName, paletteName);}

    //! Looks up the ID of the material with the specified code.
    DGNPLATFORM_EXPORT static DgnMaterialId QueryMaterialId(DgnDbR db, DgnCodeCR code);

    //! Looks up the ID of the material with the specified palette + material name.
    static DgnMaterialId QueryMaterialId(DgnDbR db, Utf8StringCR paletteName, Utf8StringCR materialName) {return QueryMaterialId(db, CreateCode(db, paletteName, materialName));}

    //! Looks up a material by ID.
    static DgnMaterialCPtr Get(DgnDbR db, DgnMaterialId materialId) {return db.Elements().Get<DgnMaterial>(materialId);}

    //! An entry in a material iterator
    struct Entry : ECSqlStatementEntry
    {
        friend struct ECSqlStatementIterator<Entry>;
        friend struct DgnMaterial;
    private:
        Entry(BeSQLite::EC::ECSqlStatement* stmt=nullptr) : ECSqlStatementEntry(stmt) {}
    public:
        DgnMaterialId GetId() const {return m_statement->GetValueId<DgnMaterialId>(0);} //!< The material ID
        Utf8CP GetName() const {return m_statement->GetValueText(1);} //!< The material name
        Utf8CP GetPalette() const {return m_statement->GetValueText(2);} //!< The palette name
        DgnMaterialId GetParentId() const {return m_statement->GetValueId<DgnMaterialId>(3);} //!< The parent material ID
        Utf8CP GetDescr() const {return m_statement->GetValueText(4);} //!< The material description
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
                m_byParent(nullptr != parent), m_parent(m_byParent ? *parent : DgnMaterialId()) {}
        public:
            //! Default options: Includes all materials , unordered
            Options() : Options(nullptr, nullptr, false) {}
            //! Filter by palette name and parent material, optionally ordered by palette name and then material name
            Options(Utf8StringCR paletteName, DgnMaterialId parentId, bool ordered=false) : Options(&paletteName, &parentId, ordered) {}
            
            //! Unfiltered, ordered
            static Options Ordered() {return Options(nullptr, nullptr, true);}

            //! Filter by a specific palette name, optionally ordered by material name
            static Options ByPalette(Utf8StringCR paletteName, bool ordered = false) {return Options(&paletteName, nullptr, ordered);}

            //! Filter by a specific parent material, optionally ordered by palette name and then material name
            static Options ByParentId(DgnMaterialId parentId, bool ordered = false) {return Options(nullptr, &parentId, ordered);}
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
    struct Material : Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_MaterialElement, DgnMaterial, Material, Definition, DGNPLATFORM_EXPORT);
    };
}

END_BENTLEY_DGNPLATFORM_NAMESPACE

