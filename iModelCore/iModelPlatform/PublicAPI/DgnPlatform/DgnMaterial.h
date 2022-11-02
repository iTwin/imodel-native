/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include "DgnDb.h"
#include "DgnElement.h"
#include "Render.h"
#include "RenderMaterial.h"
#include "ECSqlStatementIterator.h"

DGNPLATFORM_TYPEDEFS(PhysicalMaterial);
DGNPLATFORM_TYPEDEFS(RenderMaterial);
DGNPLATFORM_REF_COUNTED_PTR(PhysicalMaterial);
DGNPLATFORM_REF_COUNTED_PTR(RenderMaterial);

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

namespace dgn_ElementHandler {struct PhysicalMaterial; struct RenderMaterial;}

//=======================================================================================
//! Base class for defining the physical properties of materials for analysis.
//! @note Differences in physical properties may not be relevant for rendering, so there are separate PhysicalMaterial and RenderMaterial classes.
//! @see RenderMaterial
//! @bsiclass
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE PhysicalMaterial : DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_PhysicalMaterial, DefinitionElement);
    friend struct dgn_ElementHandler::PhysicalMaterial;

protected:
    DgnCode _GenerateDefaultCode() const override {return DgnCode();}
    bool _SupportsCodeSpec(CodeSpecCR codeSpec) const override {return !codeSpec.IsNullCodeSpec();}

    explicit PhysicalMaterial(CreateParams const& params) : T_Super(params) {}

public:
    //! Creates a DgnCode for a PhysicalMaterial.
    static DgnCode CreateCode(DefinitionModelR scope, Utf8StringCR materialName) {return CodeSpec::CreateCode(BIS_CODESPEC_PhysicalMaterial, scope, materialName);}

    //! Construct a PhysicalMaterial with the specified parameters
    PhysicalMaterial(DefinitionModelR model, DgnClassId materialClassId, Utf8StringCR materialName)
        : T_Super(CreateParams(model.GetDgnDb(), model.GetModelId(), materialClassId, CreateCode(model, materialName))) {}
};

//=======================================================================================
//! Class for defining the rendering properties of materials for display.
//! @see PhysicalMaterial
//! @bsistruct
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
    BeJsConst GetMaterialAssets() const {return GetJsonProperties(json_materialAssets());}
    BeJsValue GetMaterialAssetsR() {return GetJsonPropertiesR(json_materialAssets());}

    DGNPLATFORM_EXPORT DgnDbStatus _SetParentId(DgnElementId parentId, DgnClassId parentRelClassId) override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnChildImport(DgnElementCR child, DgnModelR destModel, DgnImportContext& importer) const override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnDelete() const override;
    DGNPLATFORM_EXPORT void _RemapIds(DgnImportContext& importer) override;

    static Utf8String combineName(Utf8StringCR paletteName, Utf8StringCR materialName)
        {
        Utf8PrintfString combined("%s: %s", paletteName.c_str(), materialName.c_str());
        return std::move(combined);
        }

    DgnCode _GenerateDefaultCode() const override {return DgnCode();}
    bool _SupportsCodeSpec(CodeSpecCR codeSpec) const override {return !codeSpec.IsNullCodeSpec();}

    explicit RenderMaterial(CreateParams const& params) : T_Super(params) {}

public:
    //! @private
    static RenderMaterialId ImportMaterial(RenderMaterialId source, DgnImportContext& importer);

    //! Construct a RenderMaterial with the specified parameters
    //! @param[in] model The DefinitionModel in which the RenderMaterial will reside
    //! @param[in] paletteName The palette name which categorizes this RenderMaterial
    //! @param[in] materialName The name of the RenderMaterial. This becomes the value of the RenderMaterial's DgnCode.
    //! @param[in] parentMaterialId Optional ID of the parent RenderMaterial. If specified, this RenderMaterial inherits and can override the parent's material data.
    RenderMaterial(DefinitionModelR model, Utf8StringCR paletteName, Utf8StringCR materialName, RenderMaterialId parentMaterialId=RenderMaterialId())
        : T_Super(CreateParams(model.GetDgnDb(), model.GetModelId(), QueryClassId(model.GetDgnDb()), CreateCode(model, paletteName, materialName), nullptr, parentMaterialId))
        {
        SetPaletteName(paletteName);
        if (parentMaterialId.IsValid())
            m_parent.m_relClassId = GetDgnDb().Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_REL_RenderMaterialOwnsRenderMaterials);
        SetUserLabel(materialName.c_str());
        }

    RenderMaterialId GetMaterialId() const {return RenderMaterialId(GetElementId().GetValue());} //!< Returns the Id of this RenderMaterial.
    Utf8String GetMaterialName() const {return GetCode().GetValue().GetUtf8();} //!< Returns the RenderMaterial name

    Utf8String GetPaletteName() const {return GetPropertyValueString(prop_PaletteName());} //!< Returns the palette name which categorizes this RenderMaterial
    DgnDbStatus SetPaletteName(Utf8StringCR paletteName) {return SetPropertyValue(prop_PaletteName(), paletteName.c_str());} //!< Set the palette name which categorizes this RenderMaterial

    Utf8String GetDescription() const {return GetPropertyValueString(prop_Description());} //!< Get the description of this ColorBook
    DgnDbStatus SetDescription(Utf8StringCR description) {return SetPropertyValue(prop_Description(), description.c_str());} //!< Set the description for this ColorBook

    RenderMaterialId GetParentMaterialId() const {return RenderMaterialId(GetParentId().GetValueUnchecked());} //!< Returns the ID of this RenderMaterial's parent
    RenderMaterialCPtr GetParentMaterial() const {return GetParentId().IsValid() ? GetDgnDb().Elements().Get<RenderMaterial>(GetParentId()) : nullptr;} //!< Returns this RenderMaterial's parent, if one exists.

    static DgnClassId QueryClassId(DgnDbR db) {return db.Schemas().GetClassId(BIS_ECSCHEMA_NAME, BIS_CLASS_RenderMaterial);} //!< Returns the class ID used for RenderMaterial elements.
    static ECN::ECClassCP QueryECClass(DgnDbR db) {return db.Schemas().GetClass(QueryClassId(db));} //!< Looks up the ECClass used for RenderMaterial elements.

    RenderMaterialCPtr Insert(DgnDbStatus* status = nullptr) {return GetDgnDb().Elements().Insert<RenderMaterial>(*this, status);} //!< Inserts this RenderMaterial into the DgnDb and returns the persistent RenderMaterial.

    //! Get an asset of the RenderMaterial as a Json value.
    BeJsConst GetAsset(Utf8CP asset) const {return GetMaterialAssets()[asset];}
    BeJsValue GetAssetR(Utf8CP asset) {return GetMaterialAssetsR()[asset];}

    //! Set an asset of the RenderMaterial from a Json value.
    //! @param[in] name asset name
    //! @param[in] value The Json value for the asset.
    void SetAsset(Utf8CP name, BeJsConst value) {GetMaterialAssetsR()[name].From(value);}

    //! Get the rendering asset.
    RenderingAsset GetRenderingAsset() const {return RenderingAsset(GetAsset(json_renderMaterial()));}
    RenderingAsset GetRenderingAssetR() {return RenderingAsset(GetAssetR(json_renderMaterial()));}

    void SetRenderingAsset(BeJsConst val) {GetAssetR(json_renderMaterial()).From(val);}

    //! Creates a DgnCode for a RenderMaterial.
    static DgnCode CreateCode(DefinitionModelR scope, Utf8StringCR paletteName, Utf8StringCR materialName) {return CodeSpec::CreateCode(BIS_CODESPEC_RenderMaterial, scope, combineName(materialName, paletteName));}

    //! Looks up the ID of the material with the specified code.
    static RenderMaterialId QueryMaterialId(DgnDbR db, DgnCodeCR code) {return RenderMaterialId(db.Elements().QueryElementIdByCode(code).GetValueUnchecked());}
    //! Looks up the ID of the material with the specified palette + material name.
    static RenderMaterialId QueryMaterialId(DefinitionModelR model, Utf8StringCR paletteName, Utf8StringCR materialName) {return QueryMaterialId(model.GetDgnDb(), CreateCode(model, paletteName, materialName));}

    //! Looks up a RenderMaterial by ID.
    static RenderMaterialCPtr Get(DgnDbR db, RenderMaterialId materialId) {return db.Elements().Get<RenderMaterial>(materialId);}

    //! An entry in a RenderMaterial iterator
    struct Entry : ECSqlStatementEntry
    {
        friend struct ECSqlStatementIterator<Entry>;
        friend struct RenderMaterial;
    private:
        Entry(BeSQLite::EC::ECSqlStatement* stmt=nullptr) : ECSqlStatementEntry(stmt) {}
    public:
        RenderMaterialId GetId() const {return m_statement->GetValueId<RenderMaterialId>(0);} //!< The RenderMaterial ID
        Utf8CP GetName() const {return m_statement->GetValueText(1);} //!< The RenderMaterial name
        Utf8CP GetPalette() const {return m_statement->GetValueText(2);} //!< The palette name
        RenderMaterialId GetParentId() const {return m_statement->GetValueId<RenderMaterialId>(3);} //!< The parent RenderMaterial ID
        Utf8CP GetDescription() const {return m_statement->GetValueText(4);} //!< The RenderMaterial description
    };

    //! An iterator over the RenderMaterials within a DgnDb
    struct Iterator : ECSqlStatementIterator<Entry>
    {
        //! Options controlling RenderMaterial iteration
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
            //! Default options: Includes all RenderMaterials , unordered
            Options() : Options(nullptr, nullptr, false) {}
            //! Filter by palette name and parent RenderMaterial, optionally ordered by palette name and then RenderMaterial name
            Options(Utf8StringCR paletteName, RenderMaterialId parentId, bool ordered=false) : Options(&paletteName, &parentId, ordered) {}

            //! Unfiltered, ordered
            static Options Ordered() {return Options(nullptr, nullptr, true);}

            //! Filter by a specific palette name, optionally ordered by RenderMaterial name
            static Options ByPalette(Utf8StringCR paletteName, bool ordered = false) {return Options(&paletteName, nullptr, ordered);}

            //! Filter by a specific parent RenderMaterial, optionally ordered by palette name and then RenderMaterial name
            static Options ByParentId(RenderMaterialId parentId, bool ordered = false) {return Options(nullptr, &parentId, ordered);}
        };

        static Iterator Create(DgnDbR db, Options const& options); //!< @private
    };

    //! Create an iterator over the RenderMaterials within a DgnDb
    //! @param[in] db The DgnDb in which to query
    //! @param[in] options Options controlling which RenderMaterials to include and in what order
    //! @return An iterator with the specified options
    DGNPLATFORM_EXPORT static Iterator MakeIterator(DgnDbR db, Iterator::Options options=Iterator::Options());
};

namespace dgn_ElementHandler
{
    struct PhysicalMaterial : Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_PhysicalMaterial, Dgn::PhysicalMaterial, PhysicalMaterial, Definition, DGNPLATFORM_EXPORT);
    };

    struct RenderMaterial : Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_RenderMaterial, Dgn::RenderMaterial, RenderMaterial, Definition, DGNPLATFORM_EXPORT);
    };
}

END_BENTLEY_DGNPLATFORM_NAMESPACE
