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
//! A DgnGeomPart stores geometry that can be shared between multiple elements.
//! Use the ElementGeometryBuilder to create the shared geometry.
//! @see DgnGeomParts
//! @ingroup ElementGeometryGroup
// @bsiclass                                                BentleySystems
//=======================================================================================
struct DgnGeomPart : RefCountedBase, ICodedObject
{
//__PUBLISH_SECTION_END__
    friend struct DgnGeomParts;
    friend struct DgnImportContext;
    friend struct ElementGeometryBuilder;

//__PUBLISH_SECTION_START__
private:
    DgnDbR              m_db;
    DgnGeomPartId       m_id;       //!< Id of this geometry part.  Invalid until DgnGeomParts::InsertGeomPart is called or part is read from the DgnDb.
    GeomStream          m_geometry; //!< Geometry of part
    ElementAlignedBox3d m_bbox;     //!< Bounding box of part geometry
    DgnCode             m_code;     //!< Uniquely identifies this part

    DgnGeomPart(DgnDbR db, DgnCodeCR code) : m_db(db), m_code(code) { }

    void SetId(DgnGeomPartId id) {m_id = id;}

    virtual DgnDbR _GetDgnDb() const override final { return m_db; }
    virtual bool _SupportsCodeAuthority(DgnAuthorityCR auth) const override final { return GeomPartAuthority::GetGeomPartAuthorityId() == auth.GetAuthorityId(); }
    virtual DgnCode _GenerateDefaultCode() const override final { return GeomPartAuthority::CreateEmptyCode(); }
    virtual DgnCode const& _GetCode() const override final { return m_code; }
    virtual DgnDbStatus _SetCode(DgnCode const& code) override final { m_code = code; return DgnDbStatus::Success; }
    virtual DgnGeomPartCP _ToGeomPart() const override final { return this; }
protected:
    //! Only ElementGeometryBuilder should have write access to the GeomStream...
    GeomStreamR GetGeomStreamR() {return m_geometry;}
    void SetBoundingBox(ElementAlignedBox3dCR box) {m_bbox = box;}
public:
    //! Create a DgnGeomPart
    //! @see DgnGeomParts::InsertGeomPart
    DGNPLATFORM_EXPORT static DgnGeomPartPtr Create(DgnDbR db, DgnCode code=GeomPartAuthority::CreateEmptyCode());

    //! Get the persistent Id of this DgnGeomPart.
    //! @note Id will be invalid if not yet persisted.
    DgnGeomPartId GetId() const {return m_id;}

    //! Get the geometry for this part (part local coordinates)
    GeomStreamCR GetGeomStream() const {return m_geometry;}

    //! Get the bounding box for this part (part local coordinates)
    ElementAlignedBox3dCR GetBoundingBox() const {return m_bbox;}

    //! Create a DgnCode suitable for assigning to a DgnGeomPart
    static DgnCode CreateCode(Utf8StringCR nameSpace, Utf8StringCR name) { return GeomPartAuthority::CreateGeomPartCode(nameSpace, name); }
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
