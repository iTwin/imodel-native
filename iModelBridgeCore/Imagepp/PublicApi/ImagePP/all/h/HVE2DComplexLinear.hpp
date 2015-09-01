//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/HVE2DComplexLinear.hpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

BEGIN_IMAGEPP_NAMESPACE
/** -----------------------------------------------------------------------------
    Default constructor for a complex linear. This constructor creates an empty
    complex linear that does not contain any linear components. The interpretation
    coordinate system is dynamically allocated.

    -----------------------------------------------------------------------------
*/
inline HVE2DComplexLinear::HVE2DComplexLinear()
    : HVE2DLinear(),
      m_ExtentUpToDate(false),
      m_LengthUpToDate(false)
    {
    }

/** -----------------------------------------------------------------------------
    Default constructor for a complex linear. This constructor creates an empty
    complex linear that does not contain any linear components. The interpretation
    coordinate system is the one provided.

    @param pi_rpCoordSys Reference to smart pointer to interpretation coordinate system.
                         This pointer may not be null.

    Example:
    @code
    HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
    HVE2DComplexLinear        MyComplex1(pMyWorld);
    @end

    @see HGF2DCoordSys
    -----------------------------------------------------------------------------
*/
inline HVE2DComplexLinear::HVE2DComplexLinear(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    : HVE2DLinear(pi_rpCoordSys),
      m_ExtentUpToDate(false),
      m_Extent(pi_rpCoordSys),
      m_LengthUpToDate(false)
    {
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
inline HVE2DComplexLinear::~HVE2DComplexLinear()
    {
    MakeEmpty();
    }

/** -----------------------------------------------------------------------------
    This method returns true if there are no components in the complex linear.

    @return This method returns true if there are no components in the complex linear.

    @see MakeEmpty()
    -----------------------------------------------------------------------------
*/
inline bool HVE2DComplexLinear::IsEmpty() const
    {
    return (m_LinearList.empty());
    }

//-----------------------------------------------------------------------------
// IsNull
// Indicates if the shape is null (no length)
//-----------------------------------------------------------------------------
inline bool HVE2DComplexLinear::IsNull() const
    {
    return (m_LinearList.empty());
    }

//-----------------------------------------------------------------------------
// IsABasicLinear
// Indicates if the complex linear is a basic linear
//-----------------------------------------------------------------------------
inline bool HVE2DComplexLinear::IsABasicLinear() const
    {
    return (false);
    }

//-----------------------------------------------------------------------------
// IsComplex
// Indicates if the present linear is complex
//-----------------------------------------------------------------------------
inline bool HVE2DComplexLinear::IsComplex() const
    {
    return (true);
    }

/** -----------------------------------------------------------------------------
    Returns a const reference to the internal list of linear components of the
    complex linear.

    @return A reference to the internal list of components in the complex linear.


    -----------------------------------------------------------------------------
*/
inline const HVE2DComplexLinear::LinearList& HVE2DComplexLinear::GetLinearList() const
    {
    return (m_LinearList);
    }


/** -----------------------------------------------------------------------------
    Returns the number of linear components in the complex linear.

    @return The number of components in the complex linear.

    -----------------------------------------------------------------------------
*/
inline size_t HVE2DComplexLinear::GetNumberOfLinears() const
    {
    return(m_LinearList.size());
    }

/** -----------------------------------------------------------------------------
    Returns a const reference to the internal component linear specified
    by the index. The index number must be located between 0 and the number of
    components minus one.

    @param pi_Index The index to desired linear. This number must be located between 0
                    and the number of components minus one.

    @return A reference to the internal component refered to.

    -----------------------------------------------------------------------------
*/
inline const HVE2DLinear& HVE2DComplexLinear::GetLinear(size_t pi_Index) const
    {
    // The given index must be valid (between 0 and size-1)
    HPRECONDITION((m_LinearList.size() > pi_Index) && (pi_Index >= 0));

    HVE2DComplexLinear::LinearList::const_iterator Itr = m_LinearList.begin();

    for (size_t Index = 0 ; Index < pi_Index ; Index++)
        Itr++;

    return (**Itr);
    }


//-----------------------------------------------------------------------------
// ShortenTo
// Shortens the complex linear definition by specification of a new end point
// In the case of a complex linear this is identical to setting a new end point with
// SetEndPoint(), except that it is required that new point be located ON
// complex linear
//-----------------------------------------------------------------------------
inline void HVE2DComplexLinear::ShortenTo(const HGF2DLocation& pi_rNewEndPoint)
    {
    // The given point must be located on complex linear
    HPRECONDITION(IsPointOn(pi_rNewEndPoint));

    // Call other function to perform the processing
    ShortenTo(CalculateRelativePosition(pi_rNewEndPoint));
    }

//-----------------------------------------------------------------------------
// ShortenFrom
// Shortens the complex linear definition by specification of a new start point
// In the case of a complex linear this is identical to setting a new end point with
// SetEndPoint(), except that it is required that new point be located ON
// complex linear
//-----------------------------------------------------------------------------
inline void HVE2DComplexLinear::ShortenFrom(const HGF2DLocation& pi_rNewStartPoint)
    {
    // The given point must be located on complex linear
    HPRECONDITION(IsPointOn(pi_rNewStartPoint));

    // Call other function to perform the processing
    ShortenFrom(CalculateRelativePosition(pi_rNewStartPoint));
    }

//-----------------------------------------------------------------------------
// Shorten
// Shortens the complex linear definition by specification of a new start and end points
//-----------------------------------------------------------------------------
inline void HVE2DComplexLinear::Shorten(const HGF2DLocation& pi_rStartPoint,
                                        const HGF2DLocation& pi_rEndPoint)
    {
    // The given points must be located on complex linear
    HPRECONDITION(IsPointOn(pi_rStartPoint) && IsPointOn(pi_rEndPoint));

    // Call other function to perform the processing
    Shorten(CalculateRelativePosition(pi_rStartPoint), CalculateRelativePosition(pi_rEndPoint));
    }

//-----------------------------------------------------------------------------
// Clone
// Returns a dynamically allocated copy of the complex linear
//-----------------------------------------------------------------------------
inline HPMPersistentObject* HVE2DComplexLinear::Clone() const
    {
    return (new HVE2DComplexLinear(*this));
    }

END_IMAGEPP_NAMESPACE
