/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/CodeSpec.h>
#include <DgnPlatform/DgnModel.h>

BEGIN_BENTLEY_DGN_NAMESPACE

namespace dgn_ElementHandler {struct GeometryPart;};

//=======================================================================================
//! A DgnGeometryPart stores geometry that can be shared between multiple elements.
//! Use the GeometryBuilder to create the shared geometry.
//! @see DgnGeometryParts
//! @ingroup GROUP_Geometry
//=======================================================================================
struct EXPORT_VTABLE_ATTRIBUTE DgnGeometryPart : DefinitionElement
{
    DGNELEMENT_DECLARE_MEMBERS(BIS_CLASS_GeometryPart, DefinitionElement);
    friend struct dgn_ElementHandler::GeometryPart;

    friend struct DgnImportContext;
    friend struct GeometryBuilder;

private:
    GeometryStream              m_geometry; //!< Geometry of part
    ElementAlignedBox3d         m_bbox;     //!< Bounding box of part geometry
    mutable bool                m_multiChunkGeomStream = false;

    explicit DgnGeometryPart(CreateParams const& params) : T_Super(params) { }
    DgnDbStatus WriteGeometryStream();

    DGNPLATFORM_EXPORT DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement&, ECSqlClassParamsCR) override;
    DGNPLATFORM_EXPORT void _ToJson(BeJsValue out, BeJsConst opts) const override;
    DGNPLATFORM_EXPORT void _FromJson(BeJsConst props) override;
    DGNPLATFORM_EXPORT void _BindWriteParams(BeSQLite::EC::ECSqlStatement&, ForInsert) override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnInsert() override;
    DGNPLATFORM_EXPORT DgnDbStatus _InsertInDb() override;
    DGNPLATFORM_EXPORT DgnDbStatus _UpdateInDb() override;
    DGNPLATFORM_EXPORT DgnDbStatus _OnDelete() const override;
    DGNPLATFORM_EXPORT void _CopyFrom(DgnElementCR, CopyFromOptions const&) override;
    DGNPLATFORM_EXPORT void _RemapIds(DgnImportContext&) override;

protected:
    //! Only GeometryBuilder should have write access to the GeometryStream...
    GeometryStreamR GetGeometryStreamR() {return m_geometry;}
    void SetBoundingBox(ElementAlignedBox3dCR box) {m_bbox = box;}
    uint32_t _GetMemSize() const override {return T_Super::_GetMemSize() + (sizeof(*this) - sizeof(T_Super)) + m_geometry.GetAllocSize();}
    DgnGeometryPartCP _ToGeometryPart() const override final {return this;}

public:
    BE_PROP_NAME(BBoxLow)
    BE_PROP_NAME(BBoxHigh)
    BE_PROP_NAME(GeometryStream)
    BE_JSON_NAME(bbox)
    BE_JSON_NAME(geom)
    BE_JSON_NAME(geomBinary)
    BE_JSON_NAME(elementGeometryBuilderParams)

    //! Create a DgnGeometryPart
    //! @param[in] model Create the DgnGeometryPart in this model
    //! @param[in] name Optional. If name provided, create the DgnCode for the DgnGeometryPart using this name and the specified model as the uniqueness scope for the name
    DGNPLATFORM_EXPORT static DgnGeometryPartPtr Create(DefinitionModelR model, Utf8StringCR name="");
    //! @private
    //! @deprecated
    DGNPLATFORM_EXPORT static DgnGeometryPartPtr Create(DgnDbR db, DgnCodeCR code=DgnCode::CreateEmpty());

    //! Create a DgnCode for a DgnGeometryPart given a name that is meant to be unique within the scope of the specified model
    static DgnCode CreateCode(DefinitionModelCR scope, Utf8StringCR name) {return name.empty() ? DgnCode() : CodeSpec::CreateCode(BIS_CODESPEC_GeometryPart, scope, name);}

    //! Get the persistent Id of this DgnGeometryPart.
    //! @note Id will be invalid if not yet persisted.
    DgnGeometryPartId GetId() const {return DgnGeometryPartId(GetElementId().GetValue());}

    //! Get the geometry for this part (part local coordinates)
    GeometryStreamCR GetGeometryStream() const {return m_geometry;}

    //! Get the bounding box for this part (part local coordinates)
    ElementAlignedBox3dCR GetBoundingBox() const {return m_bbox;}

    //! Looks up a DgnGeometryPartId by its code.
    DGNPLATFORM_EXPORT static DgnGeometryPartId QueryGeometryPartId(DgnDbR db, DgnCodeCR code);
    //! Looks up a DgnGeometryPartId by its containing model and name
    DGNPLATFORM_EXPORT static DgnGeometryPartId QueryGeometryPartId(DefinitionModelR model, Utf8StringCR name) {return QueryGeometryPartId(model.GetDgnDb(), CreateCode(model, name));}

    //! Query the range of a DgnGeometryPart by ID.
    //! @param[out] range On successful return, holds the DgnGeometryPart's range
    //! @param[in] db The DgnDb to query
    //! @param[in] geomPartId The ID of the DgnGeometryPart to query
    //! @return SUCCESS if the range was retrieved, or else ERROR if e.g. no DgnGeometryPart exists with the specified ID
    DGNPLATFORM_EXPORT static BentleyStatus QueryGeometryPartRange(DRange3dR range, DgnDbR db, DgnGeometryPartId geomPartId);
};

namespace dgn_ElementHandler
{
    //! @private
    struct GeometryPart : Definition
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(BIS_CLASS_GeometryPart, DgnGeometryPart, GeometryPart, Definition, DGNPLATFORM_EXPORT);
        DGNPLATFORM_EXPORT void _RegisterPropertyAccessors(ECSqlClassInfo&, ECN::ClassLayoutCR) override;
    };
};

END_BENTLEY_DGN_NAMESPACE
