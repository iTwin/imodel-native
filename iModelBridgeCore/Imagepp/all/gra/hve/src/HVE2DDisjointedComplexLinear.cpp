//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVE2DDisjointedComplexLinear.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HVE2DDisjointedComplexLinear.h>


HPM_REGISTER_CLASS(HVE2DDisjointedComplexLinear, HVE2DComplexLinear)


/** -----------------------------------------------------------------------------
    Copy constructor.  It duplicates another HVE2DDisjointedComplexLinear object.

    The component linears are also duplicated

    @param pi_rObj Constant reference to complex linear to duplicate.

    -----------------------------------------------------------------------------
*/
HVE2DDisjointedComplexLinear::HVE2DDisjointedComplexLinear(const HVE2DDisjointedComplexLinear& pi_rObj)
    : HVE2DComplexLinear(pi_rObj)
    {
    }

/** -----------------------------------------------------------------------------
    This method inserts a linear at the start of the complex linear. If the
    complex linear is originally empty the given linear is added in the
    complex unconditionally. The linear start point then become the complex
    linear start point and the given linear end point become the complex linear
    end point. If the complex linear is not originally empty, then the end point
    of the linear provided must be equal to the start point of the complex linear.
    The start point of this given linear then becomes the start point
    of the linear. The method generates a clone of the provided linear.

    @param pi_rLinear Reference to linear to add a copy of to the complex linear.

    The second one keeps the given linear, which cannot be modified nor destroyed
    by the caller; the linear is given unconditionally to the complex which
    becomes the proprietor. The second method requires furthermore, that
    the given linear be expressed in the same coordinate as the complex linear.
    This last method is provided for performance reasons and should only be
    used in the case the coordinate systems are known to be the same.

    Example:
    @code
    HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
    HGF2DLocation        MyFirstPoint(10, 10, pMyWorld);
    HGF2DLocation        MySecondPoint(15, 16, pMyWorld);
    HVE2DSegment        MySeg1(MyFirstPoint, MySecondPoint);
    ...
    HVE2DDisjointedComplexLinear    MyComplex(pMyWorld);
    MyComplex.InsertLinear(MySeg1);
    @end

    @see HVE2DLinear
    -----------------------------------------------------------------------------
*/
void HVE2DDisjointedComplexLinear::InsertLinear(const HVE2DLinear& pi_rLinear)
    {
    // Check if self is empty
    if (m_LinearList.empty())
        {
        // Since empty we must also set the end point
        m_EndPoint.Set(pi_rLinear.GetEndPoint());
        }

    // Update tolerance if needed
    if (IsAutoToleranceActive())
        {
        // Set tolerance to greatest tolerance of self and new linear
        SetTolerance(MAX(GetTolerance(), pi_rLinear.GetTolerance()));
        }

    // Create a copy of the given linear in the complex coordinate system
    HVE2DLinear* pTempLinear = (HVE2DLinear*)pi_rLinear.AllocateCopyInCoordSys(GetCoordSys());

    // Set its tolerance to the global complex tolerance and desactive automaic tolerance
    pTempLinear->SetAutoToleranceActive(false);
    pTempLinear->SetTolerance(GetTolerance());

    // Add copy to complex
    m_LinearList.push_front(pTempLinear);

    // Update start point
    m_StartPoint.Set(pi_rLinear.GetStartPoint());

    // Indicate extent and length are no more up to date
    m_ExtentUpToDate = false;
    m_LengthUpToDate = false;
    }

/** -----------------------------------------------------------------------------
    This method inserts a linear at the start of the complex linear. If the
    complex linear is originally empty the given linear is added in the
    complex unconditionally. The linear start point then become the complex
    linear start point and the given linear end point become the complex linear
    end point. If the complex linear is not originally empty, then the end point
    of the linear provided must be equal to the start point of the complex linear.
    The start point of this given linear then becomes the start point
    of the linear.
    The method keeps the given linear, which cannot be modified nor destroyed
    by the caller; the linear is given unconditionally to the complex which
    becomes the proprietor. The second method requires furthermore, that
    the given linear be expressed in the same coordinate as the complex linear.
    This last method is provided for performance reasons and should only be
    used in the case the coordinate systems are known to be the same.

    @param pi_pLinear Pointer to linear which must have been dynamically allocated (with new or Clone())
                      and that must be expressed in the same coordinate system as the complex linear.

    Example:
    @code
    HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
    HGF2DLocation          MyThirdPoint(12, 13, pMyWorld);
    HGF2Dlocation        MyFourthPoint(10, 10,  pMyWorld);
    HVE2DSegment        MySeg2(MyThirdPoint, MyFourthPoint);

    HVE2DDisjointedComplexLinear    MyComplex(pMyWorld);
    MyComplex.InsertLinearPtrSCS(new HVE2DSegment(MySeg2));
    @end

    @see HVE2DLinear
    -----------------------------------------------------------------------------
*/
void HVE2DDisjointedComplexLinear::InsertLinearPtrSCS(HVE2DLinear* pi_pLinear)
    {
    // The linear must share the same coordinate system as complex
    HPRECONDITION(GetCoordSys() == pi_pLinear->GetCoordSys());

    // The given linear must link to present complex at start point if not empty
    HPRECONDITION(m_LinearList.empty() ||
                  (m_StartPoint.IsEqualTo(pi_pLinear->GetEndPoint(), GetTolerance())));

    // Check if self is empty
    if (m_LinearList.empty())
        {
        // Since empty we must also set the end point
        m_EndPoint.SetX(pi_pLinear->GetEndPoint().GetX());
        m_EndPoint.SetY(pi_pLinear->GetEndPoint().GetY());
        }

    // Update tolerance if needed
    if (IsAutoToleranceActive())
        {
        // Set tolerance to greatest tolerance of self and new linear
        SetTolerance(MAX(GetTolerance(), pi_pLinear->GetTolerance()));
        }

    // Set its tolerance to the global complex tolerance and desactive automaic tolerance
    pi_pLinear->SetAutoToleranceActive(false);
    pi_pLinear->SetTolerance(GetTolerance());

    //The given linear is inserted
    m_LinearList.push_front(pi_pLinear);

    // Update start point
    m_StartPoint.SetX(pi_pLinear->GetStartPoint().GetX());
    m_StartPoint.SetY(pi_pLinear->GetStartPoint().GetY());

    // Indicate extent and length are no more up to date
    m_ExtentUpToDate = false;
    m_LengthUpToDate = false;
    }

/** -----------------------------------------------------------------------------
    These methods append a linear at the end of the complex linear. If the
    complex linear is originally empty, the given linear is added in the
    complex unconditionally. The linear start point then become the complex
    linear start point and the given linear end point become the complex
    linear end point. If the complex linear is not originally empty, then the
    start point of the linear provided must be equal to the end point of the
    complex linear. The end point of this given linear then becomes the end
    point of the linear. The first method generates a clone of the provided.

    @param pi_rLinear Reference to linear to add a copy of to the complex linear.

    Example:
    @code
    HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
    HGF2DLocation        MyFirstPoint(10, 10, pMyWorld);
    HGF2DLocation        MySecondPoint(15, 16, pMyWorld);
    HVE2DSegment        MySeg1(MyFirstPoint, MySecondPoint);
    ...
    HVE2DDisjointedComplexLinear    MyComplex(pMyWorld);
    MyComplex.AppendLinear(MySeg1);
    @end

    @see HVE2DLinear
    @see InsertLinear
    -----------------------------------------------------------------------------
*/
void HVE2DDisjointedComplexLinear::AppendLinear(const HVE2DLinear& pi_rLinear)
    {
    // Check if self is empty
    if (m_LinearList.empty())
        {
        // Since empty we must also set the start point
        m_StartPoint.Set(pi_rLinear.GetStartPoint());
        }

    // Update tolerance if needed
    if (IsAutoToleranceActive())
        {
        // Set tolerance to greatest tolerance of self and new linear
        SetTolerance(MAX(GetTolerance(), pi_rLinear.GetTolerance()));
        }

    // Create a copy of the given linear in the complex coordinate system
    HVE2DLinear* pTempLinear = (HVE2DLinear*)pi_rLinear.AllocateCopyInCoordSys(GetCoordSys());

    // Set its tolerance to the global complex tolerance and desactive automaic tolerance
    pTempLinear->SetAutoToleranceActive(false);
    pTempLinear->SetTolerance(GetTolerance());

    // Add copy to complex
    m_LinearList.push_back(pTempLinear);

    // Update end point
    m_EndPoint.Set(pi_rLinear.GetEndPoint());

    // Indicate extent and length are no more up to date
    m_ExtentUpToDate = false;
    m_LengthUpToDate = false;
    }

/** -----------------------------------------------------------------------------
    This method appends a linear at the end of the complex linear. If the
    complex linear is originally empty, the given linear is added in the
    complex unconditionally. The linear start point then become the complex
    linear start point and the given linear end point become the complex
    linear end point. If the complex linear is not originally empty, then
    the start point of the linear provided must be equal to the end point
    of the complex linear. The end point of this given linear then becomes the
    end point of the linear. The method keeps the given linear, which cannot be modified nor
    destroyed by the caller; the linear is given unconditionally to the complex
    which becomes the proprietor. The second method requires furthermore, that
    the given linear be expressed in the same coordinate as the complex linear.
    This last method is provided for performance reasons and should only be used
    in the case the coordinate systems are known to be the same.

    @param pi_pLinear Pointer to linear which must have been dynamically
                      allocated (with new or Clone()) and that must be
                      expressed in the same coordinate system as the complex linear.

    Example:
    @code
    HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
    ...
    HGF2DLocation      MyThirdPoint(15, 16, pMyWorld);
    HGF2Dlocation        MyFourthPoint(12, -3, pMyWorld);
    HVE2DSegment        MySeg2(MyThirdPoint, MyFourthPoint);

    HVE2DDisjointedComplexLinear    MyComplex(pMyWorld);
    MyComplex.AppendLinearPtrSCS(new HVE2DSegment(MySeg2));
    @end

    @see HVE2DLinear
    @see AppendLinear
    -----------------------------------------------------------------------------
*/
void HVE2DDisjointedComplexLinear::AppendLinearPtrSCS(HVE2DLinear* pi_pLinear)
    {
    // Check if self is empty
    if (m_LinearList.empty())
        {
        // Since empty we must also set the start point
        m_StartPoint.SetX(pi_pLinear->GetStartPoint().GetX());
        m_StartPoint.SetY(pi_pLinear->GetStartPoint().GetY());
        }

    // Update tolerance if needed
    if (IsAutoToleranceActive())
        {
        // Set tolerance to greatest tolerance of self and new linear
        SetTolerance(MAX(GetTolerance(), pi_pLinear->GetTolerance()));
        }

    // Set its tolerance to the global complex tolerance and desactive automaic tolerance
    pi_pLinear->SetAutoToleranceActive(false);
    pi_pLinear->SetTolerance(GetTolerance());

    // Append the linear at end of complex
    m_LinearList.push_back(pi_pLinear);

    // Update end point
    m_EndPoint.SetX(pi_pLinear->GetEndPoint().GetX());
    m_EndPoint.SetY(pi_pLinear->GetEndPoint().GetY());

    // Indicate extent and length are no more up to date
    m_ExtentUpToDate = false;
    m_LengthUpToDate = false;
    }

/** -----------------------------------------------------------------------------
    This methods inserts copies of the components of the given complex linear
    at the start of the self complex linear. If the self complex linear is
    originaly empty, clones of the component linears are added in the
    complex unconditionally. The given complex linear start point then become the
    self complex linear start point, and the given complex linear end point become
    the self complex linear end point. If the complex linear is not originally empty,
    then the end point of the complex linear provided must be equal to the start
    point of the self complex linear. The start point of this given linear then
    becomes the start point of the linear. This method is to be used when a complex
    linear is to be inserted to another complex linear, thus preventing the creation
    of an unnessassary level for linear representation that would result from
    using InsertLinear().

    @param pi_rDisComplexLinear Reference to complex linear to copy linear components
                             and add to the present complex linear.

    Example:
    @code
    HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
    HGF2DLocation        MyFirstPoint(10, 10, pMyWorld);
    HGF2DLocation        MySecondPoint(15, 16,  pMyWorld);
    HVE2DSegment        MySeg1(MyFirstPoint, MySecondPoint);
    ...
    HGF2DLocation          MyThirdPoint(15, 16, pMyWorld);
    HGF2Dlocation        MyFourthPoint(12, -3,  pMyWorld);
    HVE2DSegment        MySeg2(MyThirdPoint, MyFourthPoint);

    HVE2DDisjointedComplexLinear    MyComplex(pMyWorld);
    MyComplex.AppendLinear(MySeg1);
    MyComplex.AppendLinearInSameCoordSys(MySeg2);

    HVE2DDisjointedComplexLinear    AnOtherComplex(pMyWorld);

    AnOtherComplex.InsertComplexLinear(MyComplex);
    @end

    @see HVE2DLinear
    @see InsertLinear
    -----------------------------------------------------------------------------
*/
void HVE2DDisjointedComplexLinear::InsertComplexLinear(const HVE2DDisjointedComplexLinear& pi_rDisComplexLinear)
    {
    // The given must not be empty
    HPRECONDITION(!pi_rDisComplexLinear.IsEmpty());

    if (m_LinearList.empty())
        {
        // The self complex is originaly empty ... and given is not
        // We set end point appropriately
        m_EndPoint = pi_rDisComplexLinear.GetEndPoint();
        }

    // Update tolerance if needed
    if (IsAutoToleranceActive())
        {
        // Set tolerance to greatest tolerance
        SetTolerance(MAX(GetTolerance(), pi_rDisComplexLinear.GetTolerance()));
        }

    HVE2DDisjointedComplexLinear::LinearList::const_reverse_iterator  MyIterator;

    // We create copies of component linear expressed in current coordinate system
    // Scanning is reversed because insertion
    for (MyIterator = pi_rDisComplexLinear.m_LinearList.rbegin() ;
         MyIterator != pi_rDisComplexLinear.m_LinearList.rend() ; MyIterator++)
        {
        // Create copy in appropriate coordinate system
        HVE2DLinear* pTempLinear = (HVE2DLinear*)(*MyIterator)->AllocateCopyInCoordSys(
                                       GetCoordSys());

        // Set tolerance of component and desactivate automatic tolerance
        pTempLinear->SetAutoToleranceActive(false);
        pTempLinear->SetTolerance(GetTolerance());

        // Add component
        m_LinearList.push_front(pTempLinear);
        }

    // Update start point
    m_StartPoint = pi_rDisComplexLinear.GetStartPoint();

    // Indicate extent and length are no more up to date
    m_ExtentUpToDate = false;
    m_LengthUpToDate = false;
    }

/** -----------------------------------------------------------------------------
    This methods append copies of the components of the given complex linear
    at the end of the self complex linear. If the self complex linear is
    originaly empty, a clone of the component linear is added in the complex
    unconditionally. The given complex linear start point then become the self
    complex linear start point, and the given complex linear end point become
    the self complex linear end point. If the complex linear is not originally
    empty, then the start point of the complex linear provided must be equal
    to the end point of the self complex linear. The end point of this given
    linear then becomes the end point of the linear. This method is to be
    used when a complex linear is to be added to another complex linear, thus
    preventing the creation of an unnessassary level for linear representation
    that would result from using AppendLinear().

    @param pi_rDisComplexLinear Reference to complex linear to copy linear components
                             and add to the present complex linear.

    Example:
    @code
    HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
    HGF2DLocation        MyFirstPoint(10, 10, pMyWorld);
    HGF2DLocation        MySecondPoint(15, 16,  pMyWorld);
    HVE2DSegment        MySeg1(MyFirstPoint, MySecondPoint);
    ...
    HGF2DLocation          MyThirdPoint(15, 16, pMyWorld);
    HGF2Dlocation        MyFourthPoint(12, -3,  pMyWorld);
    HVE2DSegment        MySeg2(MyThirdPoint, MyFourthPoint);

    HVE2DDisjointedComplexLinear    MyComplex(pMyWorld);
    MyComplex.AppendLinear(MySeg1);
    MyComplex.AppendLinearInSameCoordSys(MySeg2);

    HVE2DDisjointedComplexLinear    AnOtherComplex(pMyWorld);

    AnOtherComplex.AppendComplexLinear(MyComplex);
    @end

    @see HVE2DLinear
    @see AppendLinear
    -----------------------------------------------------------------------------
*/
void HVE2DDisjointedComplexLinear::AppendComplexLinear(const HVE2DDisjointedComplexLinear& pi_rDisComplexLinear)
    {
    if (m_LinearList.empty())
        {
        // The self complex is originaly empty ... and given is not
        // We set start point appropriately
        m_StartPoint = pi_rDisComplexLinear.GetStartPoint();
        }

    if (IsAutoToleranceActive())
        {
        // Set tolerance to greatest tolerance
        SetTolerance(MAX(GetTolerance(), pi_rDisComplexLinear.GetTolerance()));
        }

    HVE2DComplexLinear::LinearList::const_iterator  MyIterator;

    // We create copies of component linear expressed in current coordinate system
    for (MyIterator = pi_rDisComplexLinear.m_LinearList.begin() ;
         MyIterator != pi_rDisComplexLinear.m_LinearList.end() ; MyIterator++)
        {
        // Create copy in appropriate coordinate system
        HVE2DLinear* pTempLinear = (HVE2DLinear*)(*MyIterator)->AllocateCopyInCoordSys(
                                       GetCoordSys());

        // Set tolerance of component and desactivate automatic tolerance
        pTempLinear->SetAutoToleranceActive(false);
        pTempLinear->SetTolerance(GetTolerance());

        // Add component
        m_LinearList.push_back(pTempLinear);
        }

    // Update end point
    m_EndPoint = pi_rDisComplexLinear.GetEndPoint();

    // Indicate extent and length are no more up to date
    m_ExtentUpToDate = false;
    m_LengthUpToDate = false;
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another line object.
//-----------------------------------------------------------------------------
HVE2DDisjointedComplexLinear& HVE2DDisjointedComplexLinear::operator=(const HVE2DDisjointedComplexLinear& pi_rObj)
    {
    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        // Empty the current list
        MakeEmpty();

        // Call ancester copy
        HVE2DComplexLinear::operator=(pi_rObj);

        // Copy the list of linear
        AppendComplexLinear(pi_rObj);
        }

    return (*this);
    }

//-----------------------------------------------------------------------------
// Drop
// Returns the description of linear in the form of raw location
// segments
//-----------------------------------------------------------------------------
inline void HVE2DDisjointedComplexLinear::Drop(HGF2DLocationCollection* po_pPoints,
                                               double                   pi_Tolerance,
                                               EndPointProcessing       pi_EndPointProcessing) const
    {
    //Since the linears are disjointed, returning only the points would lead to information lost.
    HASSERT(0);
    }