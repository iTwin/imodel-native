//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVEShape.hpp $
//:>
//:>  $Copyright: (c) 2011 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Inline methods for class HVEShape
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Assignment operator.  It duplicates another shape object.
//-----------------------------------------------------------------------------
inline HVEShape& HVEShape::operator=(const HVEShape& pi_rObj)
    {
    // INVARIANT
    HPRECONDITION(m_pShape->GetCoordSys() == GetCoordSys());

    if (this != &pi_rObj)
        {
        // Invoque ancester
        HGFGraphicObject::operator=(pi_rObj);

        if (m_pShape)
            delete m_pShape;

        m_pShape = (HVE2DShape*)pi_rObj.m_pShape->Clone();
        }

    HVESHAPE_SYNCH_DEBUG_CODE

    return *this;
    }

//-----------------------------------------------------------------------------
// Equality evaluation operator.  Two shapes are considered equal if they
// have the same ... pointer...
//-----------------------------------------------------------------------------
inline bool HVEShape::operator==(const HVEShape& pi_rObj) const
    {
    // INVARIANT
    HPRECONDITION(m_pShape->GetCoordSys() == GetCoordSys());

    return (this == &pi_rObj);
    }


//-----------------------------------------------------------------------------
// Make the shape become empty...
//-----------------------------------------------------------------------------
inline void HVEShape::MakeEmpty()
    {
    // INVARIANT
    HPRECONDITION(m_pShape->GetCoordSys() == GetCoordSys());

    m_pShape->MakeEmpty();

    HVESHAPE_SYNCH_DEBUG_CODE
    }


//-----------------------------------------------------------------------------
// Change the coordinate system. Update shape's vertices for new coord. sys.
//-----------------------------------------------------------------------------
inline void HVEShape::ChangeCoordSys(const HFCPtr<HGF2DCoordSys>& pi_pCoordSys)
    {
    // INVARIANT
    HPRECONDITION(m_pShape->GetCoordSys() == GetCoordSys());

    if (pi_pCoordSys != m_pShape->GetCoordSys())
        {
        HVE2DShape* pTmp = (HVE2DShape*)m_pShape->AllocateCopyInCoordSys (pi_pCoordSys);
        delete m_pShape;

        m_pShape = pTmp;

        // Set ancester graphic object coord sys
        HGFGraphicObject::SetCoordSys(pi_pCoordSys);
        }

    HVESHAPE_SYNCH_DEBUG_CODE
    }


//-----------------------------------------------------------------------------
// Change the coordinate system. Update shape's vertices for new coord. sys.
//-----------------------------------------------------------------------------
inline bool HVEShape::IsPointIn (const HGF2DLocation& pi_rPoint) const
    {
    // INVARIANT
    HPRECONDITION(m_pShape->GetCoordSys() == GetCoordSys());

    return (m_pShape->IsPointIn(pi_rPoint));
    }

//-----------------------------------------------------------------------------
// Change the coordinate system. Update shape's vertices for new coord. sys.
//-----------------------------------------------------------------------------
inline bool HVEShape::IsPointOn (const HGF2DLocation& pi_rPoint) const
    {
    // INVARIANT
    HPRECONDITION(m_pShape->GetCoordSys() == GetCoordSys());

    return (m_pShape->IsPointOn(pi_rPoint));
    }


//-----------------------------------------------------------------------------
// Tells if the shape is empty or not.
//-----------------------------------------------------------------------------
inline bool HVEShape::IsEmpty () const
    {
    // INVARIANT
    HPRECONDITION(m_pShape->GetCoordSys() == GetCoordSys());

    return (m_pShape->IsEmpty());
    }


//-----------------------------------------------------------------------------
// Get the extent of the shape
//-----------------------------------------------------------------------------
inline HGF2DExtent HVEShape::GetExtent() const
    {
    // INVARIANT
    HPRECONDITION(m_pShape->GetCoordSys() == GetCoordSys());

    return (m_pShape->GetExtent());
    }


//-----------------------------------------------------------------------------
// Shape intersection.
//-----------------------------------------------------------------------------
inline void HVEShape::Intersect(const HVEShape& pi_rObj)
    {
    // INVARIANT
    HPRECONDITION(m_pShape->GetCoordSys() == GetCoordSys());

    HVE2DShape* pShape = m_pShape->IntersectShape (*pi_rObj.m_pShape);

    delete m_pShape;
    m_pShape = pShape;

    HVESHAPE_SYNCH_DEBUG_CODE
    }


//-----------------------------------------------------------------------------
// Shape union
//-----------------------------------------------------------------------------
inline void HVEShape::Unify(const HVEShape& pi_rObj)
    {
    // INVARIANT
    HPRECONDITION(m_pShape->GetCoordSys() == GetCoordSys());

    HVE2DShape* pShape = m_pShape->UnifyShape (*pi_rObj.m_pShape);

    delete m_pShape;
    m_pShape = pShape;

    HVESHAPE_SYNCH_DEBUG_CODE
    }


//-----------------------------------------------------------------------------
// Shape difference.
//-----------------------------------------------------------------------------
inline void HVEShape::Differentiate(const HVEShape& pi_rObj)
    {
    // INVARIANT
    HPRECONDITION(m_pShape->GetCoordSys() == GetCoordSys());

    HVE2DShape* pShape = m_pShape->DifferentiateShape (*pi_rObj.m_pShape);

    delete m_pShape;
    m_pShape = pShape;

    HVESHAPE_SYNCH_DEBUG_CODE
    }



//-----------------------------------------------------------------------------
// GetShapePtr
// Provides access to the full 2d shape functionality
// RARE use
//-----------------------------------------------------------------------------
inline const HVE2DShape*  HVEShape::GetShapePtr() const
    {
    // INVARIANT
    HPRECONDITION(m_pShape->GetCoordSys() == GetCoordSys());

    return (m_pShape);
    }


//-----------------------------------------------------------------------------
// GenerateScanLines
// Scanlines generation
//-----------------------------------------------------------------------------
inline void HVEShape::GenerateScanLines(HGFScanLines& pio_rScanLines) const
    {
    // INVARIANT
    HPRECONDITION(m_pShape->GetCoordSys() == GetCoordSys());

    m_pShape->Rasterize(pio_rScanLines);
    }


/** ---------------------------------------------------------------------------
    Test if the two shapes match. For now, we do an exact match, testing
    that the other shape is "ON" the current one. We could eventually
    introduce an approximate match using Hausdorff or Frechet distance
    values.
    ---------------------------------------------------------------------------
*/
inline bool HVEShape::Matches(const HVEShape& pi_rObj) const
    {
    // INVARIANT
    HPRECONDITION(m_pShape->GetCoordSys() == GetCoordSys());

    HPRECONDITION(m_pShape != 0);
    HPRECONDITION(pi_rObj.m_pShape != 0);

    return (m_pShape->CalculateSpatialPositionOf(*pi_rObj.m_pShape) ==
            HVE2DShape::S_ON);
    }
