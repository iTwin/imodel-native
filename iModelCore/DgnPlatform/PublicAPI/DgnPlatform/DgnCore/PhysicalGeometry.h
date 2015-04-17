/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/DgnPlatform/DgnCore/PhysicalGeometry.h $
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
//! Physical geometry parts are likely shared across PhysicalGeomAspects.
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

//=======================================================================================
//! A physical geometry part that has a unique placement and potentially shared physical geometry parts.
// @bsiclass                                                BentleySystems
//=======================================================================================
struct PlacedGeomPart
{
private:
    DPoint3d m_origin;
    YawPitchRollAngles m_angles;
    DgnGeomPartPtr m_partPtr;

public:
    //! Construct an invalid PlacedGeomPart to be filled in later
    PlacedGeomPart() {m_origin.Zero();}
 
    //! Construct a PlacedGeomPart by loading a DgnGeomPart and specifying a placement
    DGNPLATFORM_EXPORT PlacedGeomPart(DgnDbR db, DgnGeomPartId geomPartId, DPoint3dCR origin, YawPitchRollAnglesCR angles);

    DPoint3d GetOrigin() const {return m_origin;}
    void SetOrigin(DPoint3dCR origin) {m_origin = origin;}

    YawPitchRollAngles GetAngles() const {return m_angles;}
    void SetAngles(YawPitchRollAnglesCR angles) {m_angles = angles;}

    //! Convert placement for this part to a Transform
    Transform GetGeomPartToGeomAspectTransform() const {return m_angles.ToTransform(m_origin);}

    DgnGeomPartPtr GetPartPtr() const {return m_partPtr;}
    DgnGeomPartPtr& GetPartPtr() {return m_partPtr;}
    void SetPartPtr(DgnGeomPartPtr const& partPtr) {m_partPtr = partPtr;}

    bool IsValid() {return m_partPtr.IsValid();}
};

//=======================================================================================
// @bsiclass                                                BentleySystems
//=======================================================================================
struct PhysicalGeometry : RefCountedBase 
{
//__PUBLISH_SECTION_END__
    friend struct DgnGeomAspects;

//__PUBLISH_SECTION_START__
private:
    bvector<PlacedGeomPart> m_parts;

    PhysicalGeometry() {}

public:
    DGNPLATFORM_EXPORT static PhysicalGeometryPtr Create();

    void AddPart(PlacedGeomPartCR part) {m_parts.push_back(part);}
    DGNPLATFORM_EXPORT ElementAlignedBox3d CalculateBoundingBox() const;

    typedef bvector<PlacedGeomPart>::const_iterator const_iterator;
    typedef bvector<PlacedGeomPart>::iterator iterator;
    const_iterator begin() const {return m_parts.begin();}
    const_iterator end() const {return m_parts.end();}
    iterator begin() {return m_parts.begin();}
    iterator end() {return m_parts.end();}
};

END_BENTLEY_DGNPLATFORM_NAMESPACE
