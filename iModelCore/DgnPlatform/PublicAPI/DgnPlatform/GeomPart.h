/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/GeomPart.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/ElementGeometry.h>

BEGIN_BENTLEY_DGN_NAMESPACE

//=======================================================================================
//! A DgnGeomPart stores geometry that can be shared between multiple elements.
//! Use the GeometryBuilder to create the shared geometry.
//! @see DgnGeomParts
//! @ingroup GeometricPrimitiveGroup
// @bsiclass                                                BentleySystems
//=======================================================================================
struct DgnGeomPart : RefCountedBase
{
//__PUBLISH_SECTION_END__
    friend struct DgnGeomParts;
    friend struct DgnImportContext;
    friend struct GeometryBuilder;

//__PUBLISH_SECTION_START__
private:
    DgnGeomPartId       m_id;       //!< Id of this geometry part.  Invalid until DgnGeomParts::InsertGeomPart is called or part is read from the DgnDb.
    Utf8String          m_code;     //!< Code of this geometry part for "named" look-ups. Code is optional.
    GeometryStream  m_geometry; //!< Geometry of part
    ElementAlignedBox3d m_bbox;     //!< Bounding box of part geometry

    explicit DgnGeomPart(Utf8CP code) {SetCode(code);}

    void SetId(DgnGeomPartId id) {m_id = id;}
    void SetCode(Utf8CP code) {m_code.AssignOrClear(code);}
    
protected:

    //! Only GeometryBuilder should have write access to the GeometryStream...
    GeometryStreamR GetGeometryStreamR() {return m_geometry;}
    void SetBoundingBox(ElementAlignedBox3dCR box) {m_bbox = box;}
public:
    //! Create a DgnGeomPart
    //! @see DgnGeomParts::InsertGeomPart
    DGNPLATFORM_EXPORT static DgnGeomPartPtr Create(Utf8CP code=NULL);

    //! Get the persistent Id of this DgnGeomPart.
    //! @note Id will be invalid if not yet persisted.
    DgnGeomPartId GetId() const {return m_id;}

    //! Get the "code" (typically programmatically-generated) for this DgnGeomPart.
    //! @note Code can be NULL in the case of "anonymous" parts that can only be looked up by Id.
    Utf8CP GetCode() const {return m_code.c_str();}

    //! Get the geometry for this part (part local coordinates)
    GeometryStreamCR GetGeometryStream() const {return m_geometry;}

    //! Get the bounding box for this part (part local coordinates)
    ElementAlignedBox3dCR GetBoundingBox() const {return m_bbox;}
};

END_BENTLEY_DGN_NAMESPACE
