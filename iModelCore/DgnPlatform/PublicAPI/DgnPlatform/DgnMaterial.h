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

DGNPLATFORM_TYPEDEFS(RenderMaterial);
DGNPLATFORM_REF_COUNTED_PTR(RenderMaterial);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_ElementHandler
{
    struct RenderMaterial;
}

//=======================================================================================
//! @bsistruct                                                  Paul.Connelly   09/15
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE RenderMaterial : DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_RenderMaterial, DefinitionElement);
    friend struct dgn_ElementHandler::RenderMaterial;

public:
    BE_PROP_NAME(PaletteName);
    BE_PROP_NAME(Description);
    BE_JSON_NAME(materialAssets);
    BE_JSON_NAME(renderMaterial);

protected:
    JsonValueCR GetMaterialAssets() const {return m_jsonProperties[json_materialAssets()];}
    JsonValueR GetMaterialAssetsR() {return m_jsonProperties[json_materialAssets()];}

    DGNPLATFORM_EXPORT DgnDbStatus _SetParentId(DgnElementId parentId, DgnClassId parentRelClassId) override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnChildImport(DgnElementCR child, DgnModelR destModel, DgnImportContext& importer) const override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnDelete() const override;
    DGNPLATFORM_EXPORT void _RemapIds(DgnImportContext& importer) override;

    DgnCode _GenerateDefaultCode() const override {return DgnCode();}
    bool _SupportsCodeSpec(CodeSpecCR codeSpec) const override {return !codeSpec.IsNullCodeSpec();}
    
    explicit RenderMaterial(CreateParams const& params) : T_Super(params) {}

public:
    //! @private
    static RenderMaterialId ImportMaterial(RenderMaterialId source, DgnImportContext& importer);

    //! Construct a new RenderMaterial with the specified parameters
    //! Constructs parameters for creating a material.
    //! @param[in] model The DefinitionModel in which the material will reside
    //! @param[in] paletteName The palette name which categorizes this material
    //! @param[in] materialName The name of the material. This becomes the value of the material's DgnCode.
    //! @param[in] parentMaterialId Optional ID of the parent material. If specified, this material inherits and can override the parent's material data.
    RenderMaterial(DefinitionModelR model, Utf8StringCR paletteName, Utf8StringCR materialName, RenderMaterialId parentMaterialId=RenderMaterialId())
        : T_Super(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, materialName), nullptr, parentMaterialId)) 
        {
        SetPaletteName(paletteName);
        if (parentMaterialId.IsValid())
            m_parentRelClassId = GetDgnDb().Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_RenderMaterialOwnsChildMaterials);
        }

    RenderMaterialId GetMaterialId() const {return RenderMaterialId(GetElementId().GetValue());} //!< Returns the Id of this material.
    Utf8String GetMaterialName() const {return GetCode().GetValue();} //!< Returns the material name

    Utf8String GetPaletteName() const {return GetPropertyValueString(prop_PaletteName());} //!< Returns the palette name which categorizes this material
    DgnDbStatus SetPaletteName(Utf8StringCR paletteName) {return SetPropertyValue(prop_PaletteName(), paletteName.c_str());} //!< Set the palette name which categorizes this material

    Utf8String GetDescription() const {return GetPropertyValueString(prop_Description());} //!< Get the description of this ColorBook
    DgnDbStatus SetDescription(Utf8StringCR description) {return SetPropertyValue(prop_Description(), description.c_str());} //!< Set the description for this ColorBook

    RenderMaterialId GetParentMaterialId() const {return RenderMaterialId(GetParentId().GetValueUnchecked());} //!< Returns the ID of this material's parent material
    RenderMaterialCPtr GetParentMaterial() const {return GetParentId().IsValid() ? GetDgnDb().Elements().Get<RenderMaterial>(GetParentId()) : nullptr;} //!< Returns this material's parent material, if one exists.

    static DgnClassId QueryClassId(DgnDbR db) {return db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_RenderMaterial);} //!< Returns the class ID used for material elements.
    static ECN::ECClassCP QueryECClass(DgnDbR db) {return db.Schemas().GetClass(QueryClassId(db));} //!< Looks up the ECClass used for material elements.

    RenderMaterialCPtr Insert(DgnDbStatus* status = nullptr) {return GetDgnDb().Elements().Insert<RenderMaterial>(*this, status);} //!< Inserts this material into the DgnDb and returns the persistent material.
    RenderMaterialCPtr Update(DgnDbStatus* status = nullptr) {return GetDgnDb().Elements().Update<RenderMaterial>(*this, status);} //!< Updates this material in the DgnDb and returns the updated persistent material

    //! Get an asset of the material as a Json value.
    JsonValueCR GetAsset(Utf8CP asset) const {return GetMaterialAssets()[asset];}
    JsonValueR GetAssetR(Utf8CP asset) {return GetMaterialAssetsR()[asset];}

    //! Set an asset of material from a Json value.
    //! @param[in] name asset name -- "RenderMaterial", "Physical" etc.
    //! @param[in] value The Json value for the asset.
    void SetAsset(Utf8CP name, JsonValueCR value) {GetMaterialAssetsR()[name] = value;}

    //! Get the rendering asset.
    RenderingAssetCR GetRenderingAsset() const {return (RenderingAssetCR) GetAsset(json_renderMaterial());}
    RenderingAssetR GetRenderingAssetR() {return (RenderingAssetR) GetAsset(json_renderMaterial());}

    void SetRenderingAsset(JsonValueCR val) {GetAssetR(json_renderMaterial()) = val;}

    //! Creates a DgnCode for a material.
    static DgnCode CreateCode(DefinitionModelR scope, Utf8StringCR materialName) {return CodeSpec::CreateCode(BIS_CODESPEC_RenderMaterial, scope, materialName);}

    //! Looks up the ID of the material with the specified code.
    static RenderMaterialId QueryMaterialId(DgnDbR db, DgnCodeCR code) {return RenderMaterialId(db.Elements().QueryElementIdByCode(code).GetValueUnchecked());}
    //! Looks up the ID of the material with the specified palette + material name.
    static RenderMaterialId QueryMaterialId(DefinitionModelR model, Utf8StringCR materialName) {return QueryMaterialId(model.GetDgnDb(), CreateCode(model, materialName));}

    //! Looks up a material by ID.
    static RenderMaterialCPtr Get(DgnDbR db, RenderMaterialId materialId) {return db.Elements().Get<RenderMaterial>(materialId);}

    //! An entry in a material iterator
    struct Entry : ECSqlStatementEntry
    {
        friend struct ECSqlStatementIterator<Entry>;
        friend struct RenderMaterial;
    private:
        Entry(BeSQLite::EC::ECSqlStatement* stmt=nullptr) : ECSqlStatementEntry(stmt) {}
    public:
        RenderMaterialId GetId() const {return m_statement->GetValueId<RenderMaterialId>(0);} //!< The material ID
        Utf8CP GetName() const {return m_statement->GetValueText(1);} //!< The material name
        Utf8CP GetPalette() const {return m_statement->GetValueText(2);} //!< The palette name
        RenderMaterialId GetParentId() const {return m_statement->GetValueId<RenderMaterialId>(3);} //!< The parent material ID
        Utf8CP GetDescription() const {return m_statement->GetValueText(4);} //!< The material description
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
            RenderMaterialId m_parent;

            Options(Utf8StringCP palette, RenderMaterialId const* parent, bool ordered) : m_ordered(ordered),
                m_byPalette(nullptr != palette), m_palette(m_byPalette ? *palette : Utf8String()),
                m_byParent(nullptr != parent), m_parent(m_byParent ? *parent : RenderMaterialId()) {}
        public:
            //! Default options: Includes all materials , unordered
            Options() : Options(nullptr, nullptr, false) {}
            //! Filter by palette name and parent material, optionally ordered by palette name and then material name
            Options(Utf8StringCR paletteName, RenderMaterialId parentId, bool ordered=false) : Options(&paletteName, &parentId, ordered) {}
            
            //! Unfiltered, ordered
            static Options Ordered() {return Options(nullptr, nullptr, true);}

            //! Filter by a specific palette name, optionally ordered by material name
            static Options ByPalette(Utf8StringCR paletteName, bool ordered = false) {return Options(&paletteName, nullptr, ordered);}

            //! Filter by a specific parent material, optionally ordered by palette name and then material name
            static Options ByParentId(RenderMaterialId parentId, bool ordered = false) {return Options(nullptr, &parentId, ordered);}
        };

        static Iterator Create(DgnDbR db, Options const& options); //!< @private
    };

    //! Create an iterator over the materials within a DgnDb
    //! @param[in] db The DgnDb in which to query
    //! @param[in] options Options controlling which materials to include and in what order
    //! @return An iterator with the specified options
    DGNPLATFORM_EXPORT static Iterator MakeIterator(DgnDbR db, Iterator::Options options=Iterator::Options());
};

namespace dgn_ElementHandler
{
    //=======================================================================================
    //! The handler for material elements.
    //! @bsistruct                                                  Paul.Connelly   09/15
    //=======================================================================================
    struct RenderMaterial : Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_RenderMaterial, Dgn::RenderMaterial, RenderMaterial, Definition, DGNPLATFORM_EXPORT);
    };
}

END_BENTLEY_DGNPLATFORM_NAMESPACE
