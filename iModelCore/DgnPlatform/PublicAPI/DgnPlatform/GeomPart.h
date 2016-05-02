/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomPart.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/ElementGeometry.h>
#include <DgnPlatform/DgnAuthority.h>

BEGIN_BENTLEY_DGN_NAMESPACE

namespace dgn_ElementHandler {struct GeometryPart;};

//=======================================================================================
//! A DgnGeometryPart stores geometry that can be shared between multiple elements.
//! Use the GeometryBuilder to create the shared geometry.
//! @see DgnGeometryParts
//! @ingroup GROUP_Geometry
//=======================================================================================
struct DgnGeometryPart : DictionaryElement
{
    DGNELEMENT_DECLARE_MEMBERS(DGN_CLASSNAME_GeometryPart, DictionaryElement);
    friend struct dgn_ElementHandler::GeometryPart;

//__PUBLISH_SECTION_END__
    friend struct DgnImportContext;
    friend struct GeometryBuilder;

//__PUBLISH_SECTION_START__
private:
    GeometryStream      m_geometry; //!< Geometry of part
    ElementAlignedBox3d m_bbox;     //!< Bounding box of part geometry
    mutable bool        m_multiChunkGeomStream = false;

    explicit DgnGeometryPart(CreateParams const& params) : T_Super(params) { }
    static void GetClassParams(Dgn::ECSqlClassParams& params);
    DgnDbStatus BindParams(BeSQLite::EC::ECSqlStatement& statement);
    DgnDbStatus WriteGeometryStream();

    virtual bool _SupportsCodeAuthority(DgnAuthorityCR auth) const override final { return GeometryPartAuthority::GetGeometryPartAuthorityId() == auth.GetAuthorityId(); }
    virtual DgnCode _GenerateDefaultCode() const override final { return GeometryPartAuthority::CreateEmptyCode(); }
    DGNPLATFORM_EXPORT virtual DgnDbStatus _ReadSelectParams(BeSQLite::EC::ECSqlStatement&, ECSqlClassParamsCR) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindInsertParams(BeSQLite::EC::ECSqlStatement&) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _BindUpdateParams(BeSQLite::EC::ECSqlStatement&) override;
    DGNPLATFORM_EXPORT virtual DgnDbStatus _InsertInDb();
    DGNPLATFORM_EXPORT virtual DgnDbStatus _UpdateInDb();

protected:
    //! Only GeometryBuilder should have write access to the GeometryStream...
    GeometryStreamR GetGeometryStreamR() {return m_geometry;}
    void SetBoundingBox(ElementAlignedBox3dCR box) {m_bbox = box;}

public:
    //! Create a DgnGeometryPart
    //! @see DgnGeometryParts::InsertGeometryPart
    DGNPLATFORM_EXPORT static DgnGeometryPartPtr Create(DgnDbR db, DgnCode code=GeometryPartAuthority::CreateEmptyCode());

    //! Get the persistent Id of this DgnGeometryPart.
    //! @note Id will be invalid if not yet persisted.
    DgnGeometryPartId GetId() const {return DgnGeometryPartId(GetElementId().GetValue());}

    //! Get the geometry for this part (part local coordinates)
    GeometryStreamCR GetGeometryStream() const {return m_geometry;}

    //! Get the bounding box for this part (part local coordinates)
    ElementAlignedBox3dCR GetBoundingBox() const {return m_bbox;}

    //! Create a DgnCode suitable for assigning to a DgnGeometryPart
    static DgnCode CreateCode(Utf8StringCR nameSpace, Utf8StringCR name) { return GeometryPartAuthority::CreateGeometryPartCode(nameSpace, name); }

    //! Looks up a DgnGeometryPartId by its code.
    DGNPLATFORM_EXPORT static DgnGeometryPartId QueryGeometryPartId(DgnCode const& code, DgnDbR db);

    //! Query the range of a DgnGeometryPart by ID.
    //! @param[out] range On successful return, holds the DgnGeometryPart's range
    //! @param[in] db The DgnDb to query
    //! @param[in] geomPartId The ID of the DgnGeometryPart to query
    //! @return SUCCESS if the range was retrieved, or else ERROR if e.g. no DgnGeometryPart exists with the specified ID
    DGNPLATFORM_EXPORT static BentleyStatus QueryGeometryPartRange(DRange3dR range, DgnDbR db, DgnGeometryPartId geomPartId);

    //! Insert the ElementGeomUsesParts relationship between an element and the geom parts it uses.
    //! @note Most apps will not need to call this directly.
    //! @private
    DGNPLATFORM_EXPORT static BentleyStatus InsertElementGeomUsesParts(DgnDbR db, DgnElementId elementId, DgnGeometryPartId geomPartId);
};

namespace dgn_ElementHandler
{
    //! @private
    struct GeometryPart : Element
    {
        ELEMENTHANDLER_DECLARE_MEMBERS(DGN_CLASSNAME_GeometryPart, DgnGeometryPart, GeometryPart, Element, DGNPLATFORM_EXPORT);
    protected:
        virtual void _GetClassParams(Dgn::ECSqlClassParams& params) override {T_Super::_GetClassParams(params); DgnGeometryPart::GetClassParams(params);}
    };
};

END_BENTLEY_DGN_NAMESPACE
