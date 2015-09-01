//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DHoledShape.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    Constructor for a holed shape. This constructor creates holed shape
    that defines the outter base shape but no holes.
    The interpretation coordinate system is the one of the given simple shape.
    The tolerance settings are also obtained from the given simple shape.

    @param pi_rSimpleShape Reference to a simple shape that will be used as base
                           shape for the holed shape.

    Example:
    @code
    @end

    -----------------------------------------------------------------------------
*/
inline HGF2DHoledShape::HGF2DHoledShape(const HGF2DSimpleShape& pi_rSimpleShape)
    : HGF2DShape(pi_rSimpleShape)
    {
    // Duplicate shape
    m_pBaseShape = static_cast<HGF2DSimpleShape*>(pi_rSimpleShape.Clone());

    // Set tolerance settings
    SetAutoToleranceActive(pi_rSimpleShape.IsAutoToleranceActive());
    SetTolerance(pi_rSimpleShape.GetTolerance());
    }

/** -----------------------------------------------------------------------------
    Copy Constructor for a holed shape.

    @param pi_rObj Holed shape to copy. All components are duplicated.

    Example:
    @code
    @end

    @see HGF2DCoordSys
    -----------------------------------------------------------------------------
*/
inline HGF2DHoledShape::HGF2DHoledShape(const HGF2DHoledShape& pi_rObj)
    : HGF2DShape(pi_rObj),
      m_pBaseShape(static_cast<HGF2DSimpleShape*>(pi_rObj.m_pBaseShape->Clone()))
    {
    // Duplicate list of holes
    HGF2DShape::HoleList::const_iterator  Itr = pi_rObj.m_HoleList.begin();

    try
        {
        while (Itr != pi_rObj.m_HoleList.end())
            {
            m_HoleList.push_back(static_cast<HGF2DSimpleShape*>((*Itr)->Clone()));

            Itr++;
            }
        }
    catch(...)
        {
        MakeEmpty();
        throw;
        }
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HGF2DHoledShape::~HGF2DHoledShape()
    {
    MakeEmpty();
    }

//-----------------------------------------------------------------------------
// IsComplex
// This method returns false since a holed shape may not be complex
//-----------------------------------------------------------------------------
inline bool   HGF2DHoledShape::IsComplex() const
    {
    return (false);
    }

//-----------------------------------------------------------------------------
// GetShapeList
// The method returns a constant reference to the list of shapes
// Since there are no such list of shapes in holed shapes
// it is an error to call this method
//-----------------------------------------------------------------------------
inline const HGF2DShape::ShapeList& HGF2DHoledShape::GetShapeList() const
    {
    HPRECONDITION(false);

    // This sequence is only for proper compilation.
    HGF2DShape::ShapeList* pPtr=0;
    return (*pPtr);
    }

//-----------------------------------------------------------------------------
// HasHoles
// This method indicates if there are any holes in the shape.
//-----------------------------------------------------------------------------
inline bool   HGF2DHoledShape::HasHoles() const
    {
    return (m_HoleList.size() > 0);
    }

//-----------------------------------------------------------------------------
// IsSimple
// This method indicates if the present shape is simple
//-----------------------------------------------------------------------------
inline bool   HGF2DHoledShape::IsSimple() const
    {
    return (false);
    }

//-----------------------------------------------------------------------------
// IsEmpty
// This method indicates if the present shape is empty
//-----------------------------------------------------------------------------
inline bool   HGF2DHoledShape::IsEmpty() const
    {
    return (m_pBaseShape->IsEmpty());
    }


//-----------------------------------------------------------------------------
// GetHoleList
// The method returns a constant reference to the list of holes
//-----------------------------------------------------------------------------
inline const HGF2DShape::HoleList& HGF2DHoledShape::GetHoleList() const
    {
    HPRECONDITION(m_HoleList.size() > 0);

    return (m_HoleList);
    }

/** -----------------------------------------------------------------------------
    This method returns a constant reference to the internal simple
    shape specifying the base shape of the holed shape.

    @return Constant reference to simple shape of the base shape.

    Example:
    @code
    @end

    @see HGF2DSimpleShape
    -----------------------------------------------------------------------------
*/
inline const HGF2DSimpleShape& HGF2DHoledShape::GetBaseShape() const
    {
    return(*m_pBaseShape);
    }

//-----------------------------------------------------------------------------
// Clone
// This method allocates dynamically a copy of the holed shape
//-----------------------------------------------------------------------------
inline HGF2DVector* HGF2DHoledShape::Clone() const
    {
    return (new HGF2DHoledShape(*this));
    }


//-----------------------------------------------------------------------------
// GetExtent
// This method returns the extent of the holed shape
//-----------------------------------------------------------------------------
inline HGF2DLiteExtent HGF2DHoledShape::GetExtent() const
    {
    return(m_pBaseShape->GetExtent());
    }


//-----------------------------------------------------------------------------
// GetShapeType
// This method returns the shape type
//-----------------------------------------------------------------------------
inline HGF2DShapeTypeId HGF2DHoledShape::GetShapeType() const
    {
    return(HGF2DHoledShape::CLASS_ID);
    }


END_IMAGEPP_NAMESPACE