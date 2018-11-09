//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HGF2DComplexShape.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    Default constructor for a complex shape. This constructor creates an empty
    complex shape that does not contain any components. The interpretation
    coordinate system is dynamically allocated.

    -----------------------------------------------------------------------------
*/
inline HGF2DComplexShape::HGF2DComplexShape()
    {
    }


/** -----------------------------------------------------------------------------
    Constructor for a complex shape. This constructor creates a complex shape that
    contains a duplicate or the given shape whatever its type. This given shape
    is the first component of the complex. The interpretation
    coordinate system is obtained from the given shape

    @param pi_rShape The first initial shape of the complex shape

    -----------------------------------------------------------------------------
*/
inline HGF2DComplexShape::HGF2DComplexShape(const HGF2DShape& pi_rShape)
    : HGF2DShape()
    {
    try
        {
        // Append shape as first shape of complex
        AddShape(pi_rShape);

        // Copy tolerance settings
        SetAutoToleranceActive(pi_rShape.IsAutoToleranceActive());
        SetTolerance(pi_rShape.GetTolerance());
        }
    catch(...)
        {
        MakeEmpty();
        throw;
        }
    }


/** -----------------------------------------------------------------------------
    Constructor for a complex shape. This constructor creates an empty
    complex shape that does not contain any component. The interpretation
    coordinate system is the one provided.

    @param pi_rObj  Reference to complex shape to copy

    Example:
    @code
    @end
    -----------------------------------------------------------------------------
*/
inline HGF2DComplexShape::HGF2DComplexShape(const HGF2DComplexShape& pi_rObj)
    : HGF2DShape(pi_rObj)
    {
    // Make a copy of the list of shapes
    HGF2DShape::ShapeList::const_iterator Itr = pi_rObj.m_ShapeList.begin();

    try
        {
        while (Itr != pi_rObj.m_ShapeList.end())
            {
            m_ShapeList.push_back(static_cast<HGF2DShape*>((*Itr)->Clone()));

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
inline HGF2DComplexShape::~HGF2DComplexShape()
    {
    MakeEmpty();
    }

/** -----------------------------------------------------------------------------
    This method adds a shape to the complex shape definition. The given
    shape may not be contained in the area already defined by the
    complex shape, nor be contiguous to any other internal shape.

    @param pi_rShape Constant reference to an HGF2DShape, specifying the new shape to add.

    Example:
    @code
    @end

    @see HGF2DShape
    @see AreContiguous()
    @see CalculateSpatialPositionOf()
    -----------------------------------------------------------------------------
*/
inline void HGF2DComplexShape::AddShape(const HGF2DShape& pi_rShape)
    {
    // The given shape must be out of current area defined
    HPRECONDITION(m_ShapeList.size() == 0 || CalculateSpatialPositionOf(pi_rShape) == HGF2DShape::S_OUT);

    // The given shape must not be contiguous to shape
    HPRECONDITION(m_ShapeList.size() == 0 || !AreContiguous(pi_rShape));

    // We add shape expressed in self coordinate system
    HGF2DShape* pNewShape = static_cast<HGF2DShape*>(pi_rShape.Clone());

    // Set tolerance setting of new shape
    pNewShape->SetAutoToleranceActive(IsAutoToleranceActive());

    // Add shape to list
    m_ShapeList.push_back(pNewShape);
    }


//-----------------------------------------------------------------------------
// IsComplex
// This method returns true since a complex shape is complex
//-----------------------------------------------------------------------------
inline bool   HGF2DComplexShape::IsComplex() const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// IsSimple
// This method returns false since a complex shape is not simple
//-----------------------------------------------------------------------------
inline bool   HGF2DComplexShape::IsSimple() const
    {
    return (false);
    }

//-----------------------------------------------------------------------------
// GetShapeList
// This method returns a cont reference to the internal shape list
//-----------------------------------------------------------------------------
inline const HGF2DShape::ShapeList& HGF2DComplexShape::GetShapeList() const
    {
    return (m_ShapeList);
    }

//-----------------------------------------------------------------------------
// HasHoles
// This method indicates if there are any holes in the shape.
// A complex shape may have holes, but it is not possible to iterate them
// ???? true or false that is the question
//-----------------------------------------------------------------------------
inline bool   HGF2DComplexShape::HasHoles() const
    {
    return (false);
    }


//-----------------------------------------------------------------------------
// GetHoleList
// The method returns a constant reference to the list of holes
// Since there are no such list of holes in complex shapes
// it is an error to call this method
//-----------------------------------------------------------------------------
inline const HGF2DShape::HoleList& HGF2DComplexShape::GetHoleList() const
    {
    HPRECONDITION(false);

    // This sequence is only for proper compilation.
    HGF2DShape::HoleList* pPtr=0;
    return (*pPtr);
    }

//-----------------------------------------------------------------------------
// Clone
// This method allocates dynamically a copy of the polygon
//-----------------------------------------------------------------------------
inline HGF2DVector* HGF2DComplexShape::Clone() const
    {
    return (new HGF2DComplexShape(*this));
    }


//-----------------------------------------------------------------------------
// IsEmpty
// This method indicates if the complex shape is empty
//-----------------------------------------------------------------------------
inline bool HGF2DComplexShape::IsEmpty() const
    {
    bool DoIsEmpty = true;

    // If any shape in list is not empty, then the complex shape is not either
    // Note that if there are no shapes in list, then also the complex shape is empty
    HGF2DShape::ShapeList::const_iterator Itr = m_ShapeList.begin();

    while (Itr != m_ShapeList.end() && DoIsEmpty)
        {
        DoIsEmpty= (*Itr)->IsEmpty();

        Itr++;
        }

    return (DoIsEmpty);
    }

//-----------------------------------------------------------------------------
// GetShapeType
// This method returns the shape type
//-----------------------------------------------------------------------------
inline HGF2DShapeTypeId HGF2DComplexShape::GetShapeType() const
    {
    return(HGF2DComplexShape::CLASS_ID);
    }


END_IMAGEPP_NAMESPACE




























