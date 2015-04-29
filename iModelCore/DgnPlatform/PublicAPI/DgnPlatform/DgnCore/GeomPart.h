/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/GeomPart.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__PUBLISH_SECTION_START__
#include <DgnPlatform/DgnPlatform.h>
#include <DgnPlatform/DgnCore/ElementGeometry.h>

BEGIN_BENTLEY_DGNPLATFORM_NAMESPACE

//=======================================================================================
//! Geometry parts represent shared geometry.
// @bsiclass                                                BentleySystems
//=======================================================================================
struct DgnGeomPart : RefCountedBase
{
//__PUBLISH_SECTION_END__
    friend struct DgnGeomParts;

//__PUBLISH_SECTION_START__
private:
    DgnGeomPartId               m_id;                   //!< Id of this geometry part.  Invalid until DgnGeomParts::InsertGeomPart is called or part is read from the DgnDb.
    Utf8String                  m_code;                 //!< Code of this geometry part for "named" look-ups. Code is optional.
    bvector<ElementGeometryPtr> m_geometry;             //!< Geometry of part

    explicit DgnGeomPart(Utf8CP code) {SetCode(code);}

    void SetId(DgnGeomPartId id) {m_id = id;}
    void SetCode(Utf8CP code) {m_code.AssignOrClear(code);}
    
public:
    DGNPLATFORM_EXPORT static DgnGeomPartPtr Create(Utf8CP code=NULL);

    //! Get the persistent Id of this DgnGeomPart.
    //! @note Id will be invalid if not yet persisted.
    DgnGeomPartId GetId() const {return m_id;}

    //! Get the "code" (typically programmatically-generated) for this DgnGeomPart.
    //! @note Code can be NULL in the case of "anonymous" parts that can only be looked up by Id.
    Utf8CP GetCode() const {return m_code.c_str();}

    //! Get the geometry for this part (part local coordinates)
    bvector<ElementGeometryPtr> const& GetGeometry() const {return m_geometry;}

    //! For adding geometry when initially creating this part.
    bvector<ElementGeometryPtr>& GetGeometryR() {return m_geometry;}
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
