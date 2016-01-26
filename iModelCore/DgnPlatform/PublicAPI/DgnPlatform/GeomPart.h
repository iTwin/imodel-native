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

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! A DgnGeometryPart stores geometry that can be shared between multiple elements.
//! Use the ElementGeometryBuilder to create the shared geometry.
//! @see DgnGeometryParts
//! @ingroup ElementGeometryGroup
// @bsiclass                                                BentleySystems
//=======================================================================================
struct DgnGeometryPart : RefCountedBase, ICodedObject
{
//__PUBLISH_SECTION_END__
    friend struct DgnGeometryParts;
    friend struct DgnImportContext;
    friend struct ElementGeometryBuilder;

//__PUBLISH_SECTION_START__
private:
    DgnDbR              m_db;
    DgnGeometryPartId   m_id;       //!< Id of this geometry part.  Invalid until DgnGeometryParts::InsertGeometryPart is called or part is read from the DgnDb.
    GeomStream          m_geometry; //!< Geometry of part
    ElementAlignedBox3d m_bbox;     //!< Bounding box of part geometry
    DgnCode             m_code;     //!< Uniquely identifies this part

    DgnGeometryPart(DgnDbR db, DgnCodeCR code) : m_db(db), m_code(code) { }

    void SetId(DgnGeometryPartId id) {m_id = id;}

    virtual DgnDbR _GetDgnDb() const override final { return m_db; }
    virtual bool _SupportsCodeAuthority(DgnAuthorityCR auth) const override final { return GeometryPartAuthority::GetGeometryPartAuthorityId() == auth.GetAuthorityId(); }
    virtual DgnCode _GenerateDefaultCode() const override final { return GeometryPartAuthority::CreateEmptyCode(); }
    virtual DgnCode const& _GetCode() const override final { return m_code; }
    virtual DgnDbStatus _SetCode(DgnCode const& code) override final { m_code = code; return DgnDbStatus::Success; }
    virtual DgnGeometryPartCP _ToGeometryPart() const override final { return this; }
protected:
    //! Only ElementGeometryBuilder should have write access to the GeomStream...
    GeomStreamR GetGeomStreamR() {return m_geometry;}
    void SetBoundingBox(ElementAlignedBox3dCR box) {m_bbox = box;}
public:
    //! Create a DgnGeometryPart
    //! @see DgnGeometryParts::InsertGeometryPart
    DGNPLATFORM_EXPORT static DgnGeometryPartPtr Create(DgnDbR db, DgnCode code=GeometryPartAuthority::CreateEmptyCode());

    //! Get the persistent Id of this DgnGeometryPart.
    //! @note Id will be invalid if not yet persisted.
    DgnGeometryPartId GetId() const {return m_id;}

    //! Get the geometry for this part (part local coordinates)
    GeomStreamCR GetGeomStream() const {return m_geometry;}

    //! Get the bounding box for this part (part local coordinates)
    ElementAlignedBox3dCR GetBoundingBox() const {return m_bbox;}

    //! Create a DgnCode suitable for assigning to a DgnGeometryPart
    static DgnCode CreateCode(Utf8StringCR nameSpace, Utf8StringCR name) { return GeometryPartAuthority::CreateGeometryPartCode(nameSpace, name); }
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
