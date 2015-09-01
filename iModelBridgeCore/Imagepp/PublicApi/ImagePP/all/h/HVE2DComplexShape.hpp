//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DComplexShape.hpp $
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
inline HVE2DComplexShape::HVE2DComplexShape()
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
inline HVE2DComplexShape::HVE2DComplexShape(const HVE2DShape& pi_rShape)
    : HVE2DShape(pi_rShape.GetCoordSys())
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

inline HVE2DComplexShape::HVE2DComplexShape(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DShape(pi_rpCoordSys)
    {
    }


/** -----------------------------------------------------------------------------
    Constructor for a complex shape. This constructor creates an empty
    complex shape that does not contain any component. The interpretation
    coordinate system is the one provided.

    @param pi_rpCoordSys Reference to smart pointer to interpretation coordinate system.
                         This pointer may not be null.

    Example:
    @code
    @end

    @see HGF2DCoordSys
    -----------------------------------------------------------------------------
*/
inline HVE2DComplexShape::HVE2DComplexShape(const HVE2DComplexShape& pi_rObj)
    : HVE2DShape(pi_rObj)
    {
    // Make a copy of the list of shapes
    HVE2DShape::ShapeList::const_iterator Itr = pi_rObj.m_ShapeList.begin();

    try
        {
        while (Itr != pi_rObj.m_ShapeList.end())
            {
            m_ShapeList.push_back((HVE2DShape*)(*Itr)->Clone());

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
inline HVE2DComplexShape::~HVE2DComplexShape()
    {
    MakeEmpty();
    }

/** -----------------------------------------------------------------------------
    This method adds a shape to the complex shape definition. The given
    shape may not be contained in the area already defined by the
    complex shape, nor be contiguous to any other internal shape.

    @param pi_rShape Constant reference to an HVE2DShape, specifying the new shape to add.

    Example:
    @code
    @end

    @see HVE2DShape
    @see AreContiguous()
    @see CalculateSpatialPositionOf()
    -----------------------------------------------------------------------------
*/
inline void HVE2DComplexShape::AddShape(const HVE2DShape& pi_rShape)
    {
    // The given shape must be out of current area defined
    HPRECONDITION(CalculateSpatialPositionOf(pi_rShape) == HVE2DShape::S_OUT);

    // The given shape must not be contiguous to shape
    HPRECONDITION(!AreContiguous(pi_rShape));

    // We add shape expressed in self coordinate system
    HVE2DShape* pNewShape = (HVE2DShape*)pi_rShape.AllocateCopyInCoordSys(GetCoordSys());

    // Set tolerance setting of new shape
    pNewShape->SetAutoToleranceActive(IsAutoToleranceActive());

    // Add shape to list
    m_ShapeList.push_back(pNewShape);
    }


//-----------------------------------------------------------------------------
// IsComplex
// This method returns true since a complex shape is complex
//-----------------------------------------------------------------------------
inline bool   HVE2DComplexShape::IsComplex() const
    {
    return (true);
    }

//-----------------------------------------------------------------------------
// IsSimple
// This method returns false since a complex shape is not simple
//-----------------------------------------------------------------------------
inline bool   HVE2DComplexShape::IsSimple() const
    {
    return (false);
    }

//-----------------------------------------------------------------------------
// GetShapeList
// This method returns a cont reference to the internal shape list
//-----------------------------------------------------------------------------
inline const HVE2DShape::ShapeList& HVE2DComplexShape::GetShapeList() const
    {
    return (m_ShapeList);
    }

//-----------------------------------------------------------------------------
// HasHoles
// This method indicates if there are any holes in the shape.
// A complex shape may have holes, but it is not possible to iterate them
// ???? true or false that is the question
//-----------------------------------------------------------------------------
inline bool   HVE2DComplexShape::HasHoles() const
    {
    return (false);
    }


//-----------------------------------------------------------------------------
// GetHoleList
// The method returns a constant reference to the list of holes
// Since there are no such list of holes in complex shapes
// it is an error to call this method
//-----------------------------------------------------------------------------
inline const HVE2DShape::HoleList& HVE2DComplexShape::GetHoleList() const
    {
    HPRECONDITION(false);

    // This sequence is only for proper compilation.
    HVE2DShape::HoleList* pPtr=0;
    return (*pPtr);
    }

//-----------------------------------------------------------------------------
// Clone
// This method allocates dynamically a copy of the polygon
//-----------------------------------------------------------------------------
inline HPMPersistentObject* HVE2DComplexShape::Clone() const
    {
    return (new HVE2DComplexShape(*this));
    }


//-----------------------------------------------------------------------------
// IsEmpty
// This method indicates if the complex shape is empty
//-----------------------------------------------------------------------------
inline bool HVE2DComplexShape::IsEmpty() const
    {
    bool DoIsEmpty = true;

    // If any shape in list is not empty, then the complex shape is not either
    // Note that if there are no shapes in list, then also the complex shape is empty
    HVE2DShape::ShapeList::const_iterator Itr = m_ShapeList.begin();

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
inline HVE2DShapeTypeId HVE2DComplexShape::GetShapeType() const
    {
    return(HVE2DComplexShape::CLASS_ID);
    }

END_IMAGEPP_NAMESPACE

