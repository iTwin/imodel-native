//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVE2DComplexLinear.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HVE2DComplexLinear.h>


HPM_REGISTER_CLASS(HVE2DComplexLinear, HVE2DLinear)


/** -----------------------------------------------------------------------------
    Copy constructor.  It duplicates another HVE2DComplexLinear object.

    The component linears are also duplicated

    @param pi_rObj Constant reference to complex linear to duplicate.

    -----------------------------------------------------------------------------
*/
HVE2DComplexLinear::HVE2DComplexLinear(const HVE2DComplexLinear& pi_rObj)
    : HVE2DLinear(pi_rObj),
      m_ExtentUpToDate(pi_rObj.m_ExtentUpToDate),
      m_Extent(pi_rObj.m_Extent),
      m_LengthUpToDate(pi_rObj.m_LengthUpToDate),
      m_Length(pi_rObj.m_Length)
    {
    // Copy complex linear components
    HVE2DComplexLinear::LinearList::const_iterator  MyIterator;

    try
        {
        // We create copies of component linear expressed in current coordinate system
        for (MyIterator = pi_rObj.m_LinearList.begin() ;
             MyIterator != pi_rObj.m_LinearList.end() ; MyIterator++)
            m_LinearList.push_back((HVE2DLinear*)(*MyIterator)->Clone());
        }
    catch(...)
        {
        MakeEmpty();
        throw;
        }
    }

//-----------------------------------------------------------------------------
// SetCoordSysImplementation
// Changes the interpretation coordinate system
//-----------------------------------------------------------------------------
void HVE2DComplexLinear::SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    {
    // We call ancester to update extremity points
    HVE2DLinear::SetCoordSysImplementation(pi_rpCoordSys);

    HVE2DComplexLinear::LinearList::iterator  MyIterator;

    // We set coordsys of each and every part of the complex
    for (MyIterator = m_LinearList.begin() ; MyIterator != m_LinearList.end() ; MyIterator++)
        (*MyIterator)->SetCoordSys(GetCoordSys());

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
    HVE2DComplexLinear    MyComplex(pMyWorld);
    MyComplex.InsertLinear(MySeg1);
    @end

    @see HVE2DLinear
    -----------------------------------------------------------------------------
*/
void HVE2DComplexLinear::InsertLinear(const HVE2DLinear& pi_rLinear)
    {
    // The given linear must link to present complex at start point if not empty
    HPRECONDITION(m_LinearList.empty() ||
                  (m_StartPoint.IsEqualTo(pi_rLinear.GetEndPoint(), GetTolerance())));

    // Check if start point is equal
    if (!m_LinearList.empty() && m_StartPoint != pi_rLinear.GetEndPoint())
        {
        // They are equal when tolerance applied, but slightly different
        // Adjust
        AdjustStartPointTo(pi_rLinear.GetEndPoint());
        }


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
    HGF2DLocation            MyThirdPoint(12, 13, pMyWorld);
    HGF2Dlocation            MyFourthPoint(10, 10, pMyWorld);
    HVE2DSegment             MySeg2(MyThirdPoint, MyFourthPoint);

    HVE2DComplexLinear    MyComplex(pMyWorld);
    MyComplex.InsertLinearPtrSCS(new HVE2DSegment(MySeg2));
    @end

    @see HVE2DLinear
    -----------------------------------------------------------------------------
*/
void HVE2DComplexLinear::InsertLinearPtrSCS(HVE2DLinear* pi_pLinear)
    {
    // The linear must share the same coordinate system as complex
    HPRECONDITION(GetCoordSys() == pi_pLinear->GetCoordSys());

    // The given linear must link to present complex at start point if not empty
    HPRECONDITION(m_LinearList.empty() ||
                  (m_StartPoint.IsEqualTo(pi_pLinear->GetEndPoint(), GetTolerance())));

    // Check if start point is equal
    if (!m_LinearList.empty() && m_StartPoint != pi_pLinear->GetEndPoint())
        {
        // They are equal when tolerance applied, but slightly different
        // Adjust
        AdjustStartPointTo(pi_pLinear->GetEndPoint());
        }

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
    HVE2DComplexLinear    MyComplex(pMyWorld);
    MyComplex.AppendLinear(MySeg1);
    @end

    @see HVE2DLinear
    @see InsertLinear
    -----------------------------------------------------------------------------
*/
void HVE2DComplexLinear::AppendLinear(const HVE2DLinear& pi_rLinear)
    {
    // The given linear must link to present complex at end point if not empty
    HPRECONDITION(m_LinearList.empty() ||
                  (m_EndPoint.IsEqualTo(pi_rLinear.GetStartPoint(), GetTolerance())));

    // Check if end point is equal
    if (!m_LinearList.empty() && m_EndPoint != pi_rLinear.GetStartPoint())
        {
        // They are equal when tolerance applied, but slightly different
        // Adjust
        AdjustEndPointTo(pi_rLinear.GetStartPoint());
        }

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
    HGF2DLocation       MyThirdPoint(15, 16, pMyWorld);
    HGF2Dlocation       MyFourthPoint(12, -3  pMyWorld);
    HVE2DSegment        MySeg2(MyThirdPoint, MyFourthPoint);

    HVE2DComplexLinear    MyComplex(pMyWorld);
    MyComplex.AppendLinearPtrSCS(new HVE2DSegment(MySeg2));
    @end

    @see HVE2DLinear
    @see AppendLinear
    -----------------------------------------------------------------------------
*/
void HVE2DComplexLinear::AppendLinearPtrSCS(HVE2DLinear* pi_pLinear)
    {
    // The linear must share the same coordinate system as complex
    HPRECONDITION(GetCoordSys() == pi_pLinear->GetCoordSys());

    // The given linear must link to present complex at end point if not empty
    HPRECONDITION(m_LinearList.empty() || (m_EndPoint == pi_pLinear->GetStartPoint()));

    // Check if end point is equal
    if (!m_LinearList.empty() && m_EndPoint != pi_pLinear->GetStartPoint())
        {
        // They are equal when tolerance applied, but slightly different
        // Adjust
        AdjustEndPointTo(pi_pLinear->GetStartPoint());
        }

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





//-----------------------------------------------------------------------------
// Move
// Moves the complex linear by a specified displacement
//-----------------------------------------------------------------------------
void HVE2DComplexLinear::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    HVE2DComplexLinear::LinearList::iterator  MyIterator;

    // We move each and every part of the complex by the displacement
    for (MyIterator = m_LinearList.begin() ; MyIterator != m_LinearList.end() ; MyIterator++)
        (*MyIterator)->Move(pi_rDisplacement);

    // We call ancester move to update extremity points
    HVE2DLinear::Move(pi_rDisplacement);

    // Indicate extent and length are no more up to date
    m_ExtentUpToDate = false;
    m_LengthUpToDate = false;

    // Update tolerance from extent if needed
    if (IsAutoToleranceActive())
        {
        double Tolerance = HGLOBAL_EPSILON;

        // Extract new extent ... this puts extent up to date
        HGF2DExtent NewExtent(GetExtent());

        if (NewExtent.IsDefined())
            {
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetXMin()));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetXMax()));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetYMin()));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetYMax()));
            }

        // Set tolerance
        SetTolerance(Tolerance);
        }
    }

//-----------------------------------------------------------------------------
// Scale
// Scales the complex linear by a specified factor
//-----------------------------------------------------------------------------
void HVE2DComplexLinear::Scale(double pi_ScaleFactor, const HGF2DLocation& pi_rScaleOrigin)
    {
    // The given scale factor may not be equal to 0.0
    HPRECONDITION(pi_ScaleFactor != 0.0);

    HVE2DComplexLinear::LinearList::iterator  MyIterator;

    // We scale each and every part of the complex by the specified parameters
    for (MyIterator = m_LinearList.begin() ; MyIterator != m_LinearList.end() ; MyIterator++)
        (*MyIterator)->Scale(pi_ScaleFactor, pi_rScaleOrigin);

    // We call ancester scale to update extremity points
    HVE2DLinear::Scale(pi_ScaleFactor, pi_rScaleOrigin);

    // Indicate extent and length are no more up to date
    m_ExtentUpToDate = false;
    m_LengthUpToDate = false;

    // Update tolerance from extent if needed
    if (IsAutoToleranceActive())
        {
        double Tolerance = HGLOBAL_EPSILON;

        // Extract new extent ... this puts extent up to date
        HGF2DExtent NewExtent(GetExtent());

        if (NewExtent.IsDefined())
            {
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetXMin()));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetXMax()));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetYMin()));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetYMax()));
            }

        // Set tolerance
        SetTolerance(Tolerance);
        }
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

    @param pi_rComplexLinear Reference to complex linear to copy linear components
                             and add to the present complex linear.

    Example:
    @code
    HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
    HGF2DLocation        MyFirstPoint(10, 10, pMyWorld);
    HGF2DLocation        MySecondPoint(15, 16, pMyWorld);
    HVE2DSegment        MySeg1(MyFirstPoint, MySecondPoint);
    ...
    HGF2DLocation          MyThirdPoint(15, 16, pMyWorld);
    HGF2Dlocation        MyFourthPoint(12, -3, pMyWorld);
    HVE2DSegment        MySeg2(MyThirdPoint, MyFourthPoint);

    HVE2DComplexLinear    MyComplex(pMyWorld);
    MyComplex.AppendLinear(MySeg1);
    MyComplex.AppendLinearInSameCoordSys(MySeg2);

    HVE2DComplexLinear    AnOtherComplex(pMyWorld);

    AnOtherComplex.InsertComplexLinear(MyComplex);
    @end

    @see HVE2DLinear
    @see InsertLinear
    -----------------------------------------------------------------------------
*/
void HVE2DComplexLinear::InsertComplexLinear(const HVE2DComplexLinear& pi_rComplexLinear)
    {
    // The given must not be empty
    HPRECONDITION(!pi_rComplexLinear.IsEmpty());

    // The given linear must link to present complex at start point if self not empty
    HPRECONDITION(m_LinearList.empty() ||
                  (m_StartPoint.IsEqualTo(pi_rComplexLinear.GetEndPoint(), GetTolerance())));

    // Check if start point is equal
    if (!m_LinearList.empty() && m_StartPoint != pi_rComplexLinear.GetEndPoint())
        {
        // They are equal when tolerance applied, but slightly different
        // Adjust
        AdjustStartPointTo(pi_rComplexLinear.GetEndPoint());
        }


    if (m_LinearList.empty())
        {
        // The self complex is originaly empty ... and given is not
        // We set end point appropriately
        m_EndPoint = pi_rComplexLinear.GetEndPoint();
        }

    // Update tolerance if needed
    if (IsAutoToleranceActive())
        {
        // Set tolerance to greatest tolerance
        SetTolerance(MAX(GetTolerance(), pi_rComplexLinear.GetTolerance()));
        }

    HVE2DComplexLinear::LinearList::const_reverse_iterator  MyIterator;

    // We create copies of component linear expressed in current coordinate system
    // Scanning is reversed because insertion
    for (MyIterator = pi_rComplexLinear.m_LinearList.rbegin() ;
         MyIterator != pi_rComplexLinear.m_LinearList.rend() ; MyIterator++)
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
    m_StartPoint = pi_rComplexLinear.GetStartPoint();

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

    @param pi_rComplexLinear Reference to complex linear to copy linear components
                             and add to the present complex linear.

    Example:
    @code
    HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
    HGF2DLocation        MyFirstPoint(10, 10,  pMyWorld);
    HGF2DLocation        MySecondPoint(15, 16, pMyWorld);
    HVE2DSegment        MySeg1(MyFirstPoint, MySecondPoint);
    ...
    HGF2DLocation          MyThirdPoint(15, 16, pMyWorld);
    HGF2Dlocation        MyFourthPoint(12, -3, pMyWorld);
    HVE2DSegment        MySeg2(MyThirdPoint, MyFourthPoint);

    HVE2DComplexLinear    MyComplex(pMyWorld);
    MyComplex.AppendLinear(MySeg1);
    MyComplex.AppendLinearInSameCoordSys(MySeg2);

    HVE2DComplexLinear    AnOtherComplex(pMyWorld);

    AnOtherComplex.AppendComplexLinear(MyComplex);
    @end

    @see HVE2DLinear
    @see AppendLinear
    -----------------------------------------------------------------------------
*/
void HVE2DComplexLinear::AppendComplexLinear(const HVE2DComplexLinear& pi_rComplexLinear)
    {
    // The given linear must link to present complex at end point if not empty
    HPRECONDITION(m_LinearList.empty() ||
                  (m_EndPoint.IsEqualTo(pi_rComplexLinear.GetStartPoint(), GetTolerance())));

    // Check if end point is equal
    if (!m_LinearList.empty() && m_EndPoint != pi_rComplexLinear.GetStartPoint())
        {
        // They are equal when tolerance applied, but slightly different
        // Adjust
        AdjustEndPointTo(pi_rComplexLinear.GetStartPoint());
        }


    if (m_LinearList.empty())
        {
        // The self complex is originaly empty ... and given is not
        // We set start point appropriately
        m_StartPoint = pi_rComplexLinear.GetStartPoint();
        }

    if (IsAutoToleranceActive())
        {
        // Set tolerance to greatest tolerance
        SetTolerance(MAX(GetTolerance(), pi_rComplexLinear.GetTolerance()));
        }

    HVE2DComplexLinear::LinearList::const_iterator  MyIterator;

    // We create copies of component linear expressed in current coordinate system
    for (MyIterator = pi_rComplexLinear.m_LinearList.begin() ;
         MyIterator != pi_rComplexLinear.m_LinearList.end() ; MyIterator++)
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
    m_EndPoint = pi_rComplexLinear.GetEndPoint();

    // Indicate extent and length are no more up to date
    m_ExtentUpToDate = false;
    m_LengthUpToDate = false;
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another line object.
//-----------------------------------------------------------------------------
HVE2DComplexLinear& HVE2DComplexLinear::operator=(const HVE2DComplexLinear& pi_rObj)
    {
    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        // Empty the current list
        MakeEmpty();

        // Call ancester copy
        HVE2DLinear::operator=(pi_rObj);

        // Copy the list of linear
        AppendComplexLinear(pi_rObj);
        }

    return (*this);
    }

//-----------------------------------------------------------------------------
// CalculateBearing
// Calculates and returns the bearing of complex linear at specified point
//-----------------------------------------------------------------------------
HGFBearing HVE2DComplexLinear::CalculateBearing(const HGF2DLocation& pi_rPoint,
                                                HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on complex linear
    HPRECONDITION(IsPointOn(pi_rPoint));

    // The bearing on complex linear is the bearing on the linear part of it
    // on which the point is located
    HVE2DComplexLinear::LinearList::const_iterator   MyIterator = m_LinearList.begin();

    // Loop till the linear on which the point is is found
//HChk AR ... Should we impose this condition??????
//    while (!(*MyIterator)->IsPointOn(pi_rPoint) || (*MyIterator)->IsNull())
    while (!(*MyIterator)->IsPointOn(pi_rPoint))
        MyIterator++;

    // Check if the point is on end point and BETA direction desired
    if ((pi_Direction == HVE2DVector::BETA) &&
        (pi_rPoint.IsEqualTo((*MyIterator)->GetEndPoint(), GetTolerance())))
        {
        // Since we want BETA direction of end point of current component,
        // We really want the BETA direction of next component start point instead
        HVE2DComplexLinear::LinearList::const_iterator OtherIterator = MyIterator;

        // Advance iterator one component
        OtherIterator++;

        // If the preceeding component was not the last one
        if (OtherIterator != m_LinearList.end())
            {
            // Make sure new current is not null
            while (OtherIterator != m_LinearList.end() && (*OtherIterator)->IsNull())
                OtherIterator++;

            if (OtherIterator != m_LinearList.end())
                {

                // Current iterator is now next one
                MyIterator = OtherIterator;
                }
            }
        }

    // Return bearing
    return ((*MyIterator)->CalculateBearing(pi_rPoint, pi_Direction));
    }

//-----------------------------------------------------------------------------
// CalculateAngularAcceleration
// Calculates and returns the angular acceleration of linear at specified point
//-----------------------------------------------------------------------------
double HVE2DComplexLinear::CalculateAngularAcceleration(
    const HGF2DLocation& pi_rPoint,
    HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on complex linear
    HPRECONDITION(IsPointOn(pi_rPoint));

    // The angular acceleration on complex linear is the acceleration on the linear part of it
    // on which the point is located
    HVE2DComplexLinear::LinearList::const_iterator   MyIterator = m_LinearList.begin();

    // Loop till the linear on which the point is is found
    while (!(*MyIterator)->IsPointOn(pi_rPoint))
        MyIterator++;

    // Check if the point is on end point and BETA direction desired
    if ((pi_Direction == HVE2DVector::BETA) &&
        (pi_rPoint.IsEqualTo((*MyIterator)->GetEndPoint(), GetTolerance())))
        {
        // Since we want BETA direction of end point of current component,
        // We really want the BETA direction of next component start point instead
        HVE2DComplexLinear::LinearList::const_iterator OtherIterator = MyIterator;

        // Advance iterator one component
        OtherIterator++;

        // If the preceeding component was not the last one
        if (OtherIterator != m_LinearList.end())
            {
            // Current iterator is now next one
            MyIterator = OtherIterator;
            }
        }

    // Return angular acceleration
    return ((*MyIterator)->CalculateAngularAcceleration(pi_rPoint, pi_Direction));
    }


//-----------------------------------------------------------------------------
// CalculateLength
// Calculates and returns the length of complex linear
//-----------------------------------------------------------------------------
double HVE2DComplexLinear::CalculateLength() const
    {
    if (!m_LengthUpToDate)
        {
        // Initialize distance to 0.0
        m_Length = 0.0;

        HVE2DComplexLinear::LinearList::const_iterator   MyIterator = m_LinearList.begin();

        // Loop and sum up the length of linears
        while (MyIterator != m_LinearList.end())
            {
            m_Length += (*MyIterator)->CalculateLength();

            MyIterator++;
            }

        (*(bool*)(&m_LengthUpToDate)) = true;
        }

    return (m_Length);
    }

//-----------------------------------------------------------------------------
// CalculateRayArea
// Calculates and returns the area of the ray extended from given point to
// the given linear
//-----------------------------------------------------------------------------
double HVE2DComplexLinear::CalculateRayArea(const HGF2DLocation& pi_rPoint) const
    {
    // Create recipient area in the X dimension units squared
    double         MyTotalArea = 0.0;

    HGF2DLocation   MyPoint(pi_rPoint);

    HVE2DComplexLinear::LinearList::const_iterator   MyIterator = m_LinearList.begin();

    // Loop and sum up the ray area of linears
    while (MyIterator != m_LinearList.end())
        {
        MyTotalArea += (*MyIterator)->CalculateRayArea(MyPoint);

        MyPoint = (*MyIterator)->GetStartPoint();

        MyIterator++;
        }

    return (MyTotalArea);
    }


//-----------------------------------------------------------------------------
// CalculateRelativePoint
// Calculates and returns the location based on the given relative position.
//-----------------------------------------------------------------------------
HGF2DLocation HVE2DComplexLinear::CalculateRelativePoint(double pi_RelativePos) const
    {
    // The relative position must be between 0.0 and 1.0
    HPRECONDITION((pi_RelativePos >= 0.0) && (pi_RelativePos <= 1.0));

    // The linear must not be NULL
//HChk AR ... Should we impose this condition??????
//    HPRECONDITION(CalculateLength() > 0.0);

    HGF2DLocation   ResultPoint(GetCoordSys());

    if (m_LinearList.size() > 0)
        {
        if (pi_RelativePos == 0.0)
            {
            // Start point is asked for
            ResultPoint = m_StartPoint;
            }
        else if (pi_RelativePos == 1.0)
            {
            // End point is asked for
            ResultPoint = m_EndPoint;
            }
        else
            {
            // Obtain the desired relative length
            double MyPosLength(CalculateLength() * pi_RelativePos);

            HVE2DComplexLinear::LinearList::const_iterator   MyIterator = m_LinearList.begin();

            // Loop till the linear on which the point is is found ...
            // It is found when the relative length is exceeded (pos length is negative)
            while ((MyIterator != m_LinearList.end()) &&
                   ((MyPosLength -= (*MyIterator)->CalculateLength()) > 0.0))
                MyIterator++;

            // Since the loop may leave with a EPSILON sized positive distance, we truncate
            if (MyIterator == m_LinearList.end())
                {
                // Backtrack
                MyIterator--;

                // Set positional length to 0.0
                MyPosLength = 0.0;

                }

            // The component linear is found ... we ask it to complete operation
            // with adjusted relative position
            ResultPoint = (*MyIterator)->CalculateRelativePoint(1.0 +
                                                                (MyPosLength /
                                                                 (*MyIterator)->CalculateLength()));
            }
        }

    return(ResultPoint);
    }


//-----------------------------------------------------------------------------
// CalculateClosestPoint
// Calculates and returns the closest point on linear from given point
//-----------------------------------------------------------------------------
HGF2DLocation HVE2DComplexLinear::CalculateClosestPoint(const HGF2DLocation& pi_rLocation) const
    {
    HGF2DLocation   ClosestPoint(GetCoordSys());
    bool            DistanceInitialized = false;
    double          TempDistance;
    HGF2DLocation   TempPoint(GetCoordSys());
    double          ShortestDistance=0.0;

    // Create an iterator
    HVE2DComplexLinear::LinearList::const_iterator   MyIterator = m_LinearList.begin();

    // Loop and ask to each part for the closest point
    while (MyIterator != m_LinearList.end())
        {
        // We ask current component linear for its closest point
        TempPoint = (*MyIterator)->CalculateClosestPoint(pi_rLocation);

        // Evaluate distance to given point
        TempDistance = (TempPoint-pi_rLocation).CalculateLength();

        // If either it is the first component evaluated, or the
        // distance is shortest than preceeding result

        if ((!DistanceInitialized) || (TempDistance < ShortestDistance))
            {
            // We save point
            ClosestPoint = TempPoint;

            // We save new shortest distance
            ShortestDistance = TempDistance;

            // Indicate that the shortest distance has already been evaluated
            DistanceInitialized = true;
            }

        // Advance to next component linear
        MyIterator++;
        }

    return(ClosestPoint);
    }


//-----------------------------------------------------------------------------
// CalculateRelativePosition
// Calculates and returns the relative position of given location on complex linear.
//-----------------------------------------------------------------------------
double HVE2DComplexLinear::CalculateRelativePosition(const HGF2DLocation& pi_rPointOnLinear) const
    {
    // The given point must be located on complex linear
    HPRECONDITION(IsPointOn(pi_rPointOnLinear));

    // The complex linear must not be null
    HPRECONDITION(CalculateLength() > 0.0);

    // Obtain the total length
    double  MyTotalLength(CalculateLength());

    // Declare a positional distance (in the same units as total length)
    double  MyPosLength = 0.0;

    HVE2DComplexLinear::LinearList::const_iterator   MyIterator = m_LinearList.begin();

    // Loop till the linear on which the point is located (ON) is found
    while (!(*MyIterator)->IsPointOn(pi_rPointOnLinear))
        {
        // Increment current positional length
        MyPosLength += (*MyIterator)->CalculateLength();

        // Advance to next component linear
        MyIterator++;
        }

    // Calculate length on found linear and add it to length on complex
    // This is of course the component total length multiplied by the relative position
    // of point on this component
    MyPosLength += (*MyIterator)->CalculateLength() *
                   (*MyIterator)->CalculateRelativePosition(pi_rPointOnLinear);

    // Return relative position (ratio of length)
    return(MyPosLength / MyTotalLength);
    }



//-----------------------------------------------------------------------------
// ShortenFrom
// Shortens the complex linear definition by specification of a new start relative point
//-----------------------------------------------------------------------------
void HVE2DComplexLinear::ShortenFrom(double pi_StartRelativePos)
    {
    // Both relative position must be between 0.0 and 1.0, and the
    // end relative position must be greater than the start relative
    HPRECONDITION((pi_StartRelativePos >= 0.0) && (pi_StartRelativePos <= 1.0));

    // Obtain the total length
    double MyTotalLength(CalculateLength());

    // Obtain length to new start point
    double MyStartLength(MyTotalLength * pi_StartRelativePos);

    // Declare increment length initialy set at 0.0 expressed un units of total length
    double MyPosLength(0.0);

    // Declare iterator to list of linears
    HVE2DComplexLinear::LinearList::iterator   MyIterator = m_LinearList.begin();

    // Loop till the linear on which the start length falls
    while ((MyIterator != m_LinearList.end()) &&
           ((MyPosLength += (*MyIterator)->CalculateLength()) < MyStartLength))
        {
        // Since this linear is located before the new start point we remove it
        m_LinearList.pop_front();

        // Go on to next linear
        MyIterator = m_LinearList.begin();
        }

    // At this time, the iterator points on the linear on which is the start point
    // We ask this linear to shorten itself

    // Obtain the linear component length
    double MyComponentLength((*MyIterator)->CalculateLength());

    // Only the start point falls on this linear ... shorten it one side only
    (*MyIterator)->ShortenFrom((MyStartLength - MyPosLength + MyComponentLength) /
                               MyComponentLength);

    // We update start point
    m_StartPoint = (*(m_LinearList.begin()))->GetStartPoint();

    // Indicate extent and length are no more up to date
    m_ExtentUpToDate = false;
    m_LengthUpToDate = false;

    // Update tolerance from extent if needed
    if (IsAutoToleranceActive())
        {
        double Tolerance = HGLOBAL_EPSILON;

        // Extract new extent ... this puts extent up to date
        HGF2DExtent NewExtent(GetExtent());

        if (NewExtent.IsDefined())
            {
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetXMin()));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetXMax()));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetYMin()));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetYMax()));
            }

        // Set tolerance
        SetTolerance(Tolerance);
        }


    }

//-----------------------------------------------------------------------------
// ShortenTo
// Shortens the complex linear definition by specification of a new end relative point
//-----------------------------------------------------------------------------
void HVE2DComplexLinear::ShortenTo(double pi_EndRelativePos)
    {
    // Both relative position must be between 0.0 and 1.0, and the
    // end relative position must be greater than the start relative
    HPRECONDITION((pi_EndRelativePos >= 0.0) && (pi_EndRelativePos <= 1.0));

    // Obtain the total length
    double MyTotalLength(CalculateLength());

    // Obtain length to new end point
    double MyEndLength(MyTotalLength * pi_EndRelativePos);

    // Declare increment length initialy set at 0.0 expressed un units of total length
    double MyPosLength(0.0);

    // Declare iterator to list of linears
    HVE2DComplexLinear::LinearList::iterator   MyIterator = m_LinearList.begin();

    // Loop till the linear on which the end length falls
    while ((MyIterator != m_LinearList.end()) &&
           ((MyPosLength += (*MyIterator)->CalculateLength()) < MyEndLength))
        MyIterator++;

    // At this time, the iterator points on the linear on which is the end point
    // We ask this linear to shorten itself

    // Obtain the linear component length
    double MyComponentLength((*MyIterator)->CalculateLength());

    (*MyIterator)->ShortenTo((MyEndLength - MyPosLength + MyComponentLength) /
                             MyComponentLength);

    // Now we remove all remaining linear of the complex linear which are further
    // than new end point
    MyIterator++;
    m_LinearList.erase(MyIterator, m_LinearList.end());

    // We update start and end points
    m_EndPoint = (*(m_LinearList.rbegin()))->GetEndPoint();

    // Indicate extent and length are no more up to date
    m_ExtentUpToDate = false;
    m_LengthUpToDate = false;

    // Update tolerance from extent if needed
    if (IsAutoToleranceActive())
        {
        double Tolerance = HGLOBAL_EPSILON;

        // Extract new extent ... this puts extent up to date
        HGF2DExtent NewExtent(GetExtent());

        if (NewExtent.IsDefined())
            {
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetXMin()));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetXMax()));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetYMin()));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetYMax()));
            }

        // Set tolerance
        SetTolerance(Tolerance);
        }
    }


//-----------------------------------------------------------------------------
// Shorten
// Shortens the complex linear definition by specification of relative positions to self.
//-----------------------------------------------------------------------------
void HVE2DComplexLinear::Shorten(double pi_StartRelativePos, double pi_EndRelativePos)
    {
    // Both relative position must be between 0.0 and 1.0, and the
    // end relative position must be greater than the start relative
    HPRECONDITION((pi_StartRelativePos >= 0.0) && (pi_EndRelativePos <= 1.0));
    HPRECONDITION(pi_StartRelativePos <= pi_EndRelativePos);

    // The linear must not be null length or empty
    HPRECONDITION(CalculateLength() > 0.0);

    // Obtain the total length
    double MyTotalLength(CalculateLength());

    // Obtain length to new start point
    double MyStartLength(MyTotalLength * pi_StartRelativePos);

    // Obtain length to new end point
    double MyEndLength(MyTotalLength * pi_EndRelativePos);

    // Declare increment length initialy set at 0.0 expressed un units of total length
    double MyPosLength(0.0);

    // Declare iterator to list of linears
    HVE2DComplexLinear::LinearList::iterator   MyIterator = m_LinearList.begin();

    // Loop till the linear on which the start length falls
    while ((MyIterator != m_LinearList.end()) &&
           ((MyPosLength += (*MyIterator)->CalculateLength()) < MyStartLength))
        {
        // Since this linear is located before the new start point we remove it
        m_LinearList.pop_front();

        // Go on to next linear
        MyIterator = m_LinearList.begin();
        }

    // At this time, the iterator points on the linear on which is the start point
    // We ask this linear to shorten itself

    // Obtain the linear component length
    double MyComponentLength((*MyIterator)->CalculateLength());


    // We first check if the end point also falls on this linear
    if (MyPosLength >= MyEndLength)
        {
        // Both start and end point fall on this linear

        // We shorten the current linear on both sides
        (*MyIterator)->Shorten((MyStartLength - MyPosLength + MyComponentLength) /
                               MyComponentLength,
                               (MyEndLength - MyPosLength + MyComponentLength) /
                               MyComponentLength);

        }
    else
        {
        // Only the start point falls on this linear ... shorten it one side only
        (*MyIterator)->ShortenFrom((MyStartLength - MyPosLength + MyComponentLength) /
                                   MyComponentLength);

        // From now we continue the shortening for the end side
        // Loop till the linear on which the end length falls
        MyIterator++;
        while ((MyIterator != m_LinearList.end()) &&
               ((MyPosLength += (*MyIterator)->CalculateLength()) < MyEndLength))
            MyIterator++;

        // At this time, the iterator points on the linear on which is the end point
        // Obtain the linear component length
        MyComponentLength = (*MyIterator)->CalculateLength();

        // We ask this linear to shorten itself
        (*MyIterator)->ShortenTo((MyEndLength - MyPosLength + MyComponentLength) /
                                 MyComponentLength);
        }

    // Now we remove all remaining linear of the complex linear which are further than new end point
    MyIterator++;
    m_LinearList.erase(MyIterator, m_LinearList.end());

    // We update start and end points
    m_StartPoint = (*(m_LinearList.begin()))->GetStartPoint();
    m_EndPoint = (*(m_LinearList.rbegin()))->GetEndPoint();

    // Indicate extent and length are no more up to date
    m_ExtentUpToDate = false;
    m_LengthUpToDate = false;

    // Update tolerance from extent if needed
    if (IsAutoToleranceActive())
        {
        double Tolerance = HGLOBAL_EPSILON;

        // Extract new extent ... this puts extent up to date
        HGF2DExtent NewExtent(GetExtent());

        if (NewExtent.IsDefined())
            {
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetXMin()));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetXMax()));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetYMin()));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetYMax()));
            }

        // Set tolerance
        SetTolerance(Tolerance);
        }
    }

//-----------------------------------------------------------------------------
// Reverse
// Reverses the direction followed by the linear. The start point becomes the
// end point, and the end point becomes the start point. The path followed in
// space by the line is not changed.
//-----------------------------------------------------------------------------
void HVE2DComplexLinear::Reverse()
    {
    // For every linear part of the self complex ...
    HVE2DComplexLinear::LinearList::iterator MySelfIterator = m_LinearList.begin();

    while (MySelfIterator != m_LinearList.end())
        {
        // Reverse the linear
        (*MySelfIterator)->Reverse();

        MySelfIterator++;
        }

    // Reverse the list
    m_LinearList.reverse();

    // Change start and end point
    HGF2DLocation DummyPoint(m_StartPoint);
    m_StartPoint = m_EndPoint;
    m_EndPoint = DummyPoint;
    }



//-----------------------------------------------------------------------------
// Crosses
// Indicates if the two complex linear cross each other
//-----------------------------------------------------------------------------
bool HVE2DComplexLinear::Crosses(const HVE2DVector& pi_rVector) const
    {
    bool   IsCrossing = false;

    // Check if given is a linear
    if (pi_rVector.GetMainVectorType() != HVE2DLinear::CLASS_ID)
        {
        // The given is not a linear ... we ask it to perform the operation
        IsCrossing = pi_rVector.Crosses(*this);
        }
    else
        {
        // Check their extent
        if (GetExtent().DoTheyOverlap(pi_rVector.GetExtent()))
            {
            // Check if they have a direction preserving relation
            // The following procedure only works if the transformation model between vectors
            // Preserves direction
            if ((GetCoordSys() == pi_rVector.GetCoordSys()) ||
                GetCoordSys()->HasDirectionPreservingRelationTo(pi_rVector.GetCoordSys()))
                {

                // For every linear part of the self complex ...
                HVE2DComplexLinear::LinearList::const_iterator MySelfIterator;
                MySelfIterator = m_LinearList.begin();

                while ((MySelfIterator != m_LinearList.end()) &&  !(IsCrossing))
                    {
                    // Check if current component crosses
                    IsCrossing = pi_rVector.Crosses(**MySelfIterator);

                    // Advance to next component
                    MySelfIterator++;
                    }

                // Check if it is crossing
                if (!IsCrossing)
                    {
                    // Since they do not cross, it is still yet possible that they do
                    // cross at connection points between components
                    MySelfIterator = m_LinearList.begin();

                    while ((MySelfIterator != m_LinearList.end()) && !(IsCrossing))
                        {
                        // Make a copy of current iterator
                        HVE2DComplexLinear::LinearList::const_iterator  OtherIterator;
                        OtherIterator = MySelfIterator;

                        // Advance the copy by one
                        OtherIterator++;

                        // Check if the current component is not the last one of complex
                        if (OtherIterator != m_LinearList.end())
                            {
                            // Check if there is an intersection at split point
                            IsCrossing = IntersectsAtSplitPoint(pi_rVector,
                                                                (*MySelfIterator)->GetEndPoint(),
                                                                (*OtherIterator)->GetEndPoint(),
                                                                true);
                            }

                        MySelfIterator++;
                        }
                    }
                }
            else
                {
                // The coordinate system do not have a direction preserving
                // relation

                // Allocate a copy of vector in another coordinate system
                HVE2DVector* pTempVector = pi_rVector.AllocateCopyInCoordSys(GetCoordSys());

                // Obtain answer
                IsCrossing = Crosses(*pTempVector);

                // Destroy temporary copy
                delete pTempVector;
                }
            }
        }

    return (IsCrossing);
    }


//-----------------------------------------------------------------------------
// AutoCrosses
// Indicates if the complex crosses itself
// For a complex to cross itself, either one of the part crosses itself, or
// one part crosses another
// INCOMPLETE ?????
//-----------------------------------------------------------------------------
bool HVE2DComplexLinear::AutoCrosses() const
    {
#if (1)
    bool   IsCrossing = false;

    // For every linear part of the self complex ...
    HVE2DComplexLinear::LinearList::const_iterator MySelfIterator = m_LinearList.begin();

    while ((MySelfIterator != m_LinearList.end()) &&  !(IsCrossing))
        {
        // Check if the linear crosses itself
        if (!(IsCrossing = (*MySelfIterator)->AutoCrosses()))
            {
            // The linear did not cross itself ... check with other parts
            // We only check the parts after itself
            HVE2DComplexLinear::LinearList::const_iterator MyOtherIterator = MySelfIterator;

            // Advance one element
            MyOtherIterator++;

            // For remaining parts ... check if they cross
            while ((MyOtherIterator != m_LinearList.end()) &&
                   !(IsCrossing = (*MySelfIterator)->Crosses((**MyOtherIterator))))
                ++MyOtherIterator;

            // Check if crossed
            if (!IsCrossing)
                {
                // It is possible that no component crosses itself, nor crosses
                // each other and still cross...
                // For example if the linear crosses at a junction point of a component

                MyOtherIterator = MySelfIterator;

                // Advance two components
                ++MyOtherIterator;

                if (MyOtherIterator != m_LinearList.end())
                    ++MyOtherIterator;

                HVE2DComplexLinear::LinearList::const_iterator SavedMyOtherIterator;

                bool PossibleInteraction = false;
                while ((MyOtherIterator != m_LinearList.end()) && !PossibleInteraction)
                    {
                    // Check if current component is linked to other component
                    if (PossibleInteraction = ((*MySelfIterator)->ConnectsTo(**MyOtherIterator)))
                        {
                        SavedMyOtherIterator = MyOtherIterator;
                        }
                    else if(PossibleInteraction = ((*MyOtherIterator)->ConnectsTo(**MySelfIterator)))
                        {
                        SavedMyOtherIterator = MySelfIterator;
                        }

                    ++MyOtherIterator;
                    }

                // Check if there is a possible interaction (connection between non
                // previous or next components
                if (PossibleInteraction)
                    {
                    // For the following processes we require partial linear
                    // which exclude the current component from
                    HVE2DComplexLinear PartialLinear1(GetCoordSys());
                    HVE2DComplexLinear PartialLinear2(GetCoordSys());
                    HVE2DComplexLinear::LinearList::const_iterator MyPartialIterator;
                    for (MyPartialIterator = m_LinearList.begin(); MyPartialIterator != SavedMyOtherIterator; ++MyPartialIterator)
                        PartialLinear1.AppendLinear(**MyPartialIterator);

                    for (++MyPartialIterator ; MyPartialIterator != m_LinearList.end(); ++MyPartialIterator)
                        PartialLinear2.AppendLinear(**MyPartialIterator);


                    // Check if component crosses with linears
                    IsCrossing = (*SavedMyOtherIterator)->Crosses(PartialLinear1) ||
                                 (*SavedMyOtherIterator)->Crosses(PartialLinear2);
                    }
                }
            }

        // Advance to next component
        MySelfIterator++;
        }

    return (IsCrossing);
#else
    return(Crosses(*this));
#endif

    }

#if (0)
//-----------------------------------------------------------------------------
// AutoIntersect
// Finds and returns all auto intersection points of linear
// INCOMPLETE ????
//-----------------------------------------------------------------------------
inline int HVE2DComplexLinear::AutoIntersect(HGF2DLocationCollection* po_pPoints) const
    {
    HPRECONDITION(po_pPoints);

    // For every linear part of the self complex ...
    HVE2DComplexLinear::LinearList::const_iterator MySelfIterator = m_LinearList.begin();

    while ((MySelfIterator != m_LinearList.end()))
        {
        // Check if the linear crosses itself
        (*MySelfIterator)->AutoIntersect(po_pPoints);

        // We only check the parts after itself
        HVE2DComplexLinear::LinearList::const_iterator MyOtherIterator (MySelfIterator);

        // Advance one element
        MyOtherIterator++;

        // For remaining parts ... find intersection points
        while (MyOtherIterator != m_LinearList.end())
            {
            (*MySelfIterator)->Intersect(**MyOtherIterator, po_pPoints);

            MyOtherIterator++;
            }

        // Advance to next component
        MySelfIterator++;
        }

    return(po_pPoints->size());
    }
#endif

//-----------------------------------------------------------------------------
// IsPointOn
// Checks if the point is located on the complex linear
//-----------------------------------------------------------------------------
bool HVE2DComplexLinear::IsPointOn(
    const HGF2DLocation& pi_rTestPoint,
    HVE2DVector::ExtremityProcessing pi_ExtremityProcessing,
    double pi_Tolerance) const
    {
    bool   IsOn = false;

    // Obtain tolerance
    double Tolerance = pi_Tolerance;
    if (Tolerance < 0.0)
        Tolerance = GetTolerance();

    // If the point has the same coordinate system...
    if (pi_rTestPoint.GetCoordSys() == GetCoordSys())
        IsOn = IsPointOnSCS(pi_rTestPoint, pi_ExtremityProcessing, Tolerance);
    else
        {
        // For every linear part of the complex ... check if point is on
        for (HVE2DComplexLinear::LinearList::const_iterator MyIterator = m_LinearList.begin();
             (MyIterator != m_LinearList.end() &&
              !(IsOn = (*MyIterator)->IsPointOn(pi_rTestPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance)));
             MyIterator++)
            ;

        // If ON and extremity processing indicates exclusion from extremities
        if (IsOn && (pi_ExtremityProcessing == HVE2DVector::EXCLUDE_EXTREMITIES))
            {
            IsOn = (!m_StartPoint.IsEqualTo(pi_rTestPoint, Tolerance) &&
                    !m_EndPoint.IsEqualTo(pi_rTestPoint, Tolerance));
            }
        }

    return (IsOn);
    }

//-----------------------------------------------------------------------------
// IsPointOnSCS
// Checks if the point is located on the complex linear
//-----------------------------------------------------------------------------
bool HVE2DComplexLinear::IsPointOnSCS(
    const HGF2DLocation& pi_rTestPoint,
    HVE2DVector::ExtremityProcessing pi_ExtremityProcessing,
    double pi_Tolerance) const
    {
    // The point must have the same coordinate system as linear
    HPRECONDITION(GetCoordSys() == pi_rTestPoint.GetCoordSys());

    bool   IsOn = false;

    // Obtain tolerance
    double Tolerance = pi_Tolerance;
    if (Tolerance < 0.0)
        Tolerance = GetTolerance();


    // For every linear part of the complex ... check if point is on
    for (HVE2DComplexLinear::LinearList::const_iterator MyIterator = m_LinearList.begin();
         (MyIterator != m_LinearList.end() &&
          !(IsOn = (*MyIterator)->IsPointOnSCS(pi_rTestPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance)));
         MyIterator++)
        ;

    // If ON and extremity processing indicates exclusion from extremities
    if (IsOn && (pi_ExtremityProcessing == HVE2DVector::EXCLUDE_EXTREMITIES))
        {
        IsOn = (!m_StartPoint.IsEqualToSCS(pi_rTestPoint, Tolerance) &&
                !m_EndPoint.IsEqualToSCS(pi_rTestPoint, Tolerance));
        }

    return (IsOn);
    }


//-----------------------------------------------------------------------------
// ObtainContiguousnessPoints
// Finds contiguousness points with vector
//-----------------------------------------------------------------------------
size_t HVE2DComplexLinear::ObtainContiguousnessPoints(
    const HVE2DVector& pi_rVector,
    HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    HPRECONDITION(po_pContiguousnessPoints != 0);

    // Save initial number of points
    size_t  InitialNumberOfPoints = po_pContiguousnessPoints->size();

    // Check if given is a linear
    if (pi_rVector.GetMainVectorType() != HVE2DLinear::CLASS_ID)
        {
        // The given vector is not a linear, we ask it to perfom the operation
        pi_rVector.ObtainContiguousnessPoints(*this, po_pContiguousnessPoints);

        // We check if the order is correct (must be in increasing ALPHA)
        // ??????????????
        }
    else
        {
        // For every linear part of the complex ... obtain contiguousness points
        for (HVE2DComplexLinear::LinearList::const_iterator MyLinearIterator = m_LinearList.begin();
             MyLinearIterator != m_LinearList.end();
             MyLinearIterator++)
            {
            // Check if this part is contiguous
            if ((*MyLinearIterator)->AreContiguous(pi_rVector))
                {
                HGF2DLocationCollection     TempPoints;

                // Obtain contiguousness points
                if ((*MyLinearIterator)->ObtainContiguousnessPoints(pi_rVector, &TempPoints) != 0)
                    {
                    // There is at least one pair of points
                    HGF2DLocationCollection::const_iterator MyPointIterator = TempPoints.begin();

                    // We check if first returned point is equal to last point obtained previously
                    if (po_pContiguousnessPoints->size() > InitialNumberOfPoints)
                        {
                        // We have previous points ... compare first of new one
                        // with last of previous
                        if ((*(po_pContiguousnessPoints->rbegin())).IsEqualTo(*MyPointIterator,
                                                                              GetTolerance()))
                            {
                            // These two are equal ... we remove previous last one
                            po_pContiguousnessPoints->pop_back();

                            // We advance to next new point
                            MyPointIterator++;
                            }
                        }

                    // Reserve required space
                    po_pContiguousnessPoints->reserve(TempPoints.size());

                    // We copy new points to returned list
                    while (MyPointIterator != TempPoints.end())
                        {
                        po_pContiguousnessPoints->push_back(*MyPointIterator);

                        MyPointIterator++;
                        }
                    }
                }
            }
        }

    return (po_pContiguousnessPoints->size() - InitialNumberOfPoints);
    }



//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsAt
// Finds contiguousness points with vector
//-----------------------------------------------------------------------------
void HVE2DComplexLinear::ObtainContiguousnessPointsAt(
    const HVE2DVector& pi_rVector,
    const HGF2DLocation& pi_rPoint,
    HGF2DLocation* po_pFirstContiguousnessPoint,
    HGF2DLocation* po_pSecondContiguousnessPoint) const
    {
    // The vectors must be contiguous at specified point
    HPRECONDITION(AreContiguousAt(pi_rVector, pi_rPoint));


    // Check if given is a vector
    if (pi_rVector.GetMainVectorType() != HVE2DLinear::CLASS_ID)
        {
        // The given vector is not a linear
        pi_rVector.ObtainContiguousnessPointsAt(*this,
                                                pi_rPoint,
                                                po_pFirstContiguousnessPoint,
                                                po_pSecondContiguousnessPoint);
        }
    else
        {
        bool   Found = false;

        // Obtain tolerance
        double Tolerance = MIN(GetTolerance(), pi_rVector.GetTolerance());

        // For every linear part of the complex ... obtain contiguousness points
        HVE2DComplexLinear::LinearList::const_iterator MyIterator = m_LinearList.begin();

        while(MyIterator != m_LinearList.end() && (Found == false))
            {
            // Check if point is on
            if ((*MyIterator)->IsPointOn(pi_rPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                {
                // Check if this part is contiguous at specified point
                if ((*MyIterator)->AreContiguousAt(pi_rVector, pi_rPoint))
                    {

                    // Obtain contiguousness points
                    (*MyIterator)->ObtainContiguousnessPointsAt(pi_rVector,
                                                                pi_rPoint,
                                                                po_pFirstContiguousnessPoint,
                                                                po_pSecondContiguousnessPoint);

                    Found = true;
                    }
                else
                    {
                    // We advance to next component
                    MyIterator++;
                    }
                }
            else
                MyIterator++;
            }

        // Since they are contiguous ... there should be some contiguousness points
        HASSERT(Found);

        // We have found a component which is contiguous at given point ...
        // The contiguousness may however extend past this component in both direction

        // We first back trak to obtain start of contiguousness region
        // Obtain a copy of iterator
        HVE2DComplexLinear::LinearList::const_iterator  MyOtherIterator = MyIterator;

        while (((*MyOtherIterator)->GetStartPoint().IsEqualTo(*po_pFirstContiguousnessPoint,
                                                              Tolerance) ||
                (*MyOtherIterator)->GetStartPoint().IsEqualTo(*po_pSecondContiguousnessPoint,
                                                              Tolerance)) &&
               (MyOtherIterator != m_LinearList.begin()))
            {
            // Rewind one component
            MyOtherIterator--;

            HGF2DLocation   AdditionalFirstPoint(GetCoordSys());
            HGF2DLocation   AdditionalSecondPoint(GetCoordSys());

            if (pi_rVector.IsPointOn((*MyOtherIterator)->GetEndPoint(), HVE2DVector::INCLUDE_EXTREMITIES, Tolerance) &&
                (*MyOtherIterator)->AreContiguousAt(pi_rVector,
                                                    (*MyOtherIterator)->GetEndPoint()))
                {
                // They are still contiguous ... obtain contiguousness points
                (*MyOtherIterator)->ObtainContiguousnessPointsAt(pi_rVector,
                                                                 (*MyOtherIterator)->GetEndPoint(),
                                                                 &AdditionalFirstPoint,
                                                                 &AdditionalSecondPoint);

                // Remove those identical to end point
                if (AdditionalFirstPoint.IsEqualTo((*MyOtherIterator)->GetEndPoint(),
                                                   Tolerance))
                    {
                    if (po_pFirstContiguousnessPoint->IsEqualTo((*MyOtherIterator)->GetEndPoint(), Tolerance))
                        *po_pFirstContiguousnessPoint = AdditionalSecondPoint;
                    else
                        *po_pSecondContiguousnessPoint = AdditionalSecondPoint;
                    }
                else
                    {
                    if (po_pFirstContiguousnessPoint->IsEqualTo((*MyOtherIterator)->GetEndPoint(),
                                                                Tolerance))
                        *po_pFirstContiguousnessPoint = AdditionalFirstPoint;
                    else
                        *po_pSecondContiguousnessPoint = AdditionalFirstPoint;
                    }
                }
            }

        // We forward track in the same fashion
        MyOtherIterator = MyIterator;

        MyOtherIterator++;

        while ((MyOtherIterator != m_LinearList.end()) &&
               ((*MyOtherIterator)->GetStartPoint().IsEqualTo(*po_pFirstContiguousnessPoint,
                                                              Tolerance) ||
                (*MyOtherIterator)->GetStartPoint().IsEqualTo(*po_pSecondContiguousnessPoint,
                                                              Tolerance)))
            {
            HGF2DLocation   AdditionalFirstPoint(GetCoordSys());
            HGF2DLocation   AdditionalSecondPoint(GetCoordSys());

            if (pi_rVector.IsPointOn((*MyOtherIterator)->GetStartPoint(), HVE2DVector::INCLUDE_EXTREMITIES, Tolerance) &&
                (*MyOtherIterator)->AreContiguousAt(pi_rVector,
                                                    (*MyOtherIterator)->GetStartPoint()))
                {
                // They are still contiguous ... obtain contiguousness points
                (*MyOtherIterator)->ObtainContiguousnessPointsAt(pi_rVector,
                                                                 (*MyOtherIterator)->GetStartPoint(),
                                                                 &AdditionalFirstPoint,
                                                                 &AdditionalSecondPoint);

                // Remove those identical to end point
                if (AdditionalFirstPoint.IsEqualTo((*MyOtherIterator)->GetStartPoint(),
                                                   Tolerance))
                    {
                    if (po_pFirstContiguousnessPoint->IsEqualTo((*MyOtherIterator)->GetStartPoint(),
                                                                Tolerance))
                        *po_pFirstContiguousnessPoint = AdditionalSecondPoint;
                    else
                        *po_pSecondContiguousnessPoint = AdditionalSecondPoint;
                    }
                else
                    {
                    if (po_pFirstContiguousnessPoint->IsEqualTo((*MyOtherIterator)->GetStartPoint(), Tolerance))
                        *po_pFirstContiguousnessPoint = AdditionalFirstPoint;
                    else
                        *po_pSecondContiguousnessPoint = AdditionalFirstPoint;
                    }
                }

            MyOtherIterator++;
            }
        }
    }



//-----------------------------------------------------------------------------
// AreContiguousAtAndGet
// Checks if contiguous and if yes finds contiguousness point with vector
//-----------------------------------------------------------------------------
bool HVE2DComplexLinear::AreContiguousAtAndGet(const HVE2DVector& pi_rVector,
                                                const HGF2DLocation& pi_rPoint,
                                                HGF2DLocation* po_pFirstContiguousnessPoint,
                                                HGF2DLocation* po_pSecondContiguousnessPoint) const
    {
    HPRECONDITION(po_pFirstContiguousnessPoint != 0);
    HPRECONDITION(po_pSecondContiguousnessPoint != 0);

    bool Answer = false;

    // Check if given is a vector
    if (pi_rVector.GetMainVectorType() != HVE2DLinear::CLASS_ID)
        {
        // The given vector is not a linear
        Answer = pi_rVector.AreContiguousAtAndGet(*this,
                                                  pi_rPoint,
                                                  po_pFirstContiguousnessPoint,
                                                  po_pSecondContiguousnessPoint);
        }
    else
        {
        bool   Found = false;

        // Obtain tolerance
        double Tolerance = MIN(GetTolerance(), pi_rVector.GetTolerance());

        // For every linear part of the complex ... obtain contiguousness points
        HVE2DComplexLinear::LinearList::const_iterator MyIterator = m_LinearList.begin();

        while(MyIterator != m_LinearList.end() && (Found == false))
            {
            // Check if point is on
            if ((*MyIterator)->IsPointOn(pi_rPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                {
                // Check if this part is contiguous at specified point
                Found = ((*MyIterator)->AreContiguousAtAndGet(pi_rVector,
                                                              pi_rPoint,
                                                              po_pFirstContiguousnessPoint,
                                                              po_pSecondContiguousnessPoint));

                if (!Found)
                    {
                    // We advance to next component
                    MyIterator++;
                    }
                }
            else
                MyIterator++;
            }

        if (Found)
            {

            // We have found a component which is contiguous at given point ...
            // The contiguousness may however extend past this component in both direction
            Answer = true;

            // We first back trak to obtain start of contiguousness region
            // Obtain a copy of iterator
            HVE2DComplexLinear::LinearList::const_iterator  MyOtherIterator = MyIterator;

            while (((*MyOtherIterator)->GetStartPoint().IsEqualTo(*po_pFirstContiguousnessPoint,
                                                                  Tolerance) ||
                    (*MyOtherIterator)->GetStartPoint().IsEqualTo(*po_pSecondContiguousnessPoint,
                                                                  Tolerance)) &&
                   (    MyOtherIterator != m_LinearList.begin()))
                {
                // Rewind one component
                MyOtherIterator--;

                HGF2DLocation   AdditionalFirstPoint(GetCoordSys());
                HGF2DLocation   AdditionalSecondPoint(GetCoordSys());

                if (pi_rVector.IsPointOn((*MyOtherIterator)->GetEndPoint(), HVE2DVector::INCLUDE_EXTREMITIES, Tolerance) &&
                    (*MyOtherIterator)->AreContiguousAtAndGet(pi_rVector,
                                                              (*MyOtherIterator)->GetEndPoint(),
                                                              &AdditionalFirstPoint,
                                                              &AdditionalSecondPoint))
                    {
                    // Remove those identical to end point
                    if (AdditionalFirstPoint.IsEqualTo((*MyOtherIterator)->GetEndPoint(),
                                                       Tolerance))
                        {
                        if (po_pFirstContiguousnessPoint->IsEqualTo((*MyOtherIterator)->GetEndPoint(),
                                                                    Tolerance))
                            *po_pFirstContiguousnessPoint = AdditionalSecondPoint;
                        else
                            *po_pSecondContiguousnessPoint = AdditionalSecondPoint;
                        }
                    else
                        {
                        if (po_pFirstContiguousnessPoint->IsEqualTo((*MyOtherIterator)->GetEndPoint(),
                                                                    Tolerance))
                            *po_pFirstContiguousnessPoint = AdditionalFirstPoint;
                        else
                            *po_pSecondContiguousnessPoint = AdditionalFirstPoint;
                        }
                    }
                }

            // We forward track in the same fashion
            MyOtherIterator = MyIterator;

            MyOtherIterator++;

            while ((MyOtherIterator != m_LinearList.end()) &&
                   ((*MyOtherIterator)->GetStartPoint().IsEqualTo(*po_pFirstContiguousnessPoint,
                                                                  Tolerance) ||
                    (*MyOtherIterator)->GetStartPoint().IsEqualTo(*po_pSecondContiguousnessPoint, \
                                                                  Tolerance)))
                {
                HGF2DLocation   AdditionalFirstPoint(GetCoordSys());
                HGF2DLocation   AdditionalSecondPoint(GetCoordSys());

                if (pi_rVector.IsPointOn((*MyOtherIterator)->GetStartPoint(), HVE2DVector::INCLUDE_EXTREMITIES, Tolerance) &&
                    (*MyOtherIterator)->AreContiguousAtAndGet(pi_rVector,
                                                              (*MyOtherIterator)->GetStartPoint(),
                                                              &AdditionalFirstPoint,
                                                              &AdditionalSecondPoint))
                    {
                    // They are still contiguous ...

                    // Remove those identical to end point
                    if (AdditionalFirstPoint.IsEqualTo((*MyOtherIterator)->GetStartPoint(),
                                                       Tolerance))
                        {
                        if (po_pFirstContiguousnessPoint->IsEqualTo((*MyOtherIterator)->GetStartPoint(),
                                                                    Tolerance))
                            *po_pFirstContiguousnessPoint = AdditionalSecondPoint;
                        else
                            *po_pSecondContiguousnessPoint = AdditionalSecondPoint;
                        }
                    else
                        {
                        if (po_pFirstContiguousnessPoint->IsEqualTo((*MyOtherIterator)->GetStartPoint(),
                                                                    Tolerance))
                            *po_pFirstContiguousnessPoint = AdditionalFirstPoint;
                        else
                            *po_pSecondContiguousnessPoint = AdditionalFirstPoint;
                        }
                    }

                MyOtherIterator++;
                }
            }
        }

    return Answer;
    }



//-----------------------------------------------------------------------------
// Intersect
// Finds intersection points with linear
//-----------------------------------------------------------------------------
size_t HVE2DComplexLinear::Intersect(const HVE2DVector& pi_rVector,
                                     HGF2DLocationCollection* po_pCrossPoints) const
    {
    size_t  NumberOfNewPoints = 0;

    // Check if given is a linear
    if (pi_rVector.GetMainVectorType() != HVE2DLinear::CLASS_ID)
        {
        // The given vector is not a linear ... we ask it to perform the operation
        NumberOfNewPoints = pi_rVector.Intersect(*this, po_pCrossPoints);
        }
    else
        {
        // Check that their extent overlap
        if (GetExtent().DoTheyOverlap(pi_rVector.GetExtent()))
            {
            // Check if they have a direction preserving relation
            // The following procedure only works if the transformation model between vectors
            // Preserves direction
            if ((GetCoordSys() == pi_rVector.GetCoordSys()) ||
                GetCoordSys()->HasDirectionPreservingRelationTo(pi_rVector.GetCoordSys()))
                {
                HVE2DComplexLinear::LinearList::const_iterator MyPreviousIterator;

                // For every linear part of the complex ... obtain intersection points
                for (HVE2DComplexLinear::LinearList::const_iterator MyIterator = m_LinearList.begin();
                     MyIterator != m_LinearList.end();
                     MyIterator++)
                    {
                    // Obtain intersection points
                    NumberOfNewPoints += (*MyIterator)->Intersect(pi_rVector, po_pCrossPoints);

                    // Make a copy of current iterator
                    HVE2DComplexLinear::LinearList::const_iterator  OtherIterator = MyIterator;

                    // Advance the copy by one
                    OtherIterator++;

                    // Check if the current component is not the last one of complex
                    if (OtherIterator != m_LinearList.end())
                        {
                        // Check if there is an intersection at split point
                        if (IntersectsAtSplitPoint(pi_rVector,
                                                   (*MyIterator)->GetEndPoint(),
                                                   (*OtherIterator)->GetEndPoint(),
                                                   true))
                            {
                            // There is ...
                            po_pCrossPoints->push_back((*MyIterator)->GetEndPoint());

                            NumberOfNewPoints++;
                            }
                        }
                    }
                }
            else
                {
                // The coordinate system do not have a direction preserving
                // relation

                // Allocate a copy of vector in another coordinate system
                HVE2DVector* pTempVector = pi_rVector.AllocateCopyInCoordSys(GetCoordSys());

                // Obtain answer
                NumberOfNewPoints = Intersect(*pTempVector, po_pCrossPoints);

                // Destroy temporary copy
                delete pTempVector;
                }
            }
        }

    return (NumberOfNewPoints);
    }




//-----------------------------------------------------------------------------
// GetExtent
// Returns the extent of the complex linear
//-----------------------------------------------------------------------------
HGF2DExtent HVE2DComplexLinear::GetExtent() const
    {
    if (!m_ExtentUpToDate)
        {
        // Cast out of const
        HGF2DExtent*    pTheExtent = (HGF2DExtent*)&m_Extent;

        // Force extent to empty and set appropriate coordinate system
        *pTheExtent = HGF2DExtent(GetCoordSys());

        // For every linear part of the complex ... merge extents
        for (HVE2DComplexLinear::LinearList::const_iterator MyIterator = m_LinearList.begin();
             (MyIterator != m_LinearList.end());
             MyIterator++)
            {
            pTheExtent->Add((*MyIterator)->GetExtent());
            }

        (*((bool*)&m_ExtentUpToDate)) = true;
        }

    return (m_Extent);
    }


/** -----------------------------------------------------------------------------
    Removes all components in trhe complex linear.

    Example:
    @code
    HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
    HGF2DLocation        MyFirstPoint(10, 10,  pMyWorld);
    HGF2DLocation        MySecondPoint(15, 16, pMyWorld);
    HVE2DSegment        MySeg1(MyFirstPoint, MySecondPoint);
    ...
    HGF2DLocation          MyThirdPoint(12, 13, pMyWorld);
    HGF2Dlocation        MyFourthPoint(10, 10, pMyWorld);
    HVE2DSegment        MySeg2(MyThirdPoint, MyFourthPoint);

    HVE2DComplexLinear    MyComplex(pMyWorld);
    MyComplex.MakeEmpty()
    @end

    @see IsEmpty()
    -----------------------------------------------------------------------------
*/
void HVE2DComplexLinear::MakeEmpty()
    {
    // Create an iterator
    HVE2DComplexLinear::LinearList::iterator   MyIterator = m_LinearList.begin();

    // Destroy all linears pointed by list nodes
    while (MyIterator != m_LinearList.end())
        {
        delete *MyIterator;
        MyIterator++;
        }

    // Clear list of nodes
    m_LinearList.clear();

    // Indicate extent and length are no more up to date
    m_ExtentUpToDate = false;
    m_LengthUpToDate = false;

    // Set tolerance back to default
    SetTolerance(HGLOBAL_EPSILON);
    }


//-----------------------------------------------------------------------------
// AllocateCopyInCoordSys
// Returns a dynamically allocated copy of the complex linear in a different
// coordinate system
//-----------------------------------------------------------------------------
HVE2DVector* HVE2DComplexLinear::AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {

    // Create new complex linear
    HVE2DComplexLinear* pNewComplex = new HVE2DComplexLinear(pi_rpCoordSys);

    // Create an iterator
    HVE2DComplexLinear::LinearList::const_iterator   MyIterator = m_LinearList.begin();

    // For every linear in complex
    while (MyIterator != m_LinearList.end())
        {
        // Allocate a copy of component in new coordinate system, and add it to new complex
        HVE2DLinear* pNewLinear = (HVE2DLinear*)((*MyIterator)->AllocateCopyInCoordSys(pi_rpCoordSys));

        // Check if new linear is null
        if (pNewLinear->IsNull())
            {
            // The component is null ...
            // Check if there any components yet
            if (pNewComplex->m_LinearList.size() > 0)
                {
                // There are already a linear in it ...
                // Adjust end point
                (*(pNewComplex->m_LinearList.rbegin()))->AdjustEndPointTo(pNewLinear->GetEndPoint());

                // delete new linear
                delete pNewLinear;
                }
            }
        else
            pNewComplex->m_LinearList.push_back(pNewLinear);

        // Advance to next component
        MyIterator++;
        }

    // Check if any components are set
    if (pNewComplex->m_LinearList.size() > 0)
        {
        // In case the first components were dropped because they were NULL
        // Check that start and end points are equal  (NOTE: do not use EPSILON in following)
        if ((*pNewComplex->m_LinearList.begin())->GetStartPoint() != (*pNewComplex->m_LinearList.rbegin())->GetEndPoint())
            {
            // The start and end point are not exactly equal ... we adjust
            (*pNewComplex->m_LinearList.rbegin())->AdjustEndPointTo((*pNewComplex->m_LinearList.begin())->GetStartPoint());
            }

        // Set start and end point
        pNewComplex->m_StartPoint = (*pNewComplex->m_LinearList.begin())->GetStartPoint();
        pNewComplex->m_EndPoint = (*pNewComplex->m_LinearList.rbegin())->GetEndPoint();

        }

    // Update tolerance from extent if needed
    if (IsAutoToleranceActive())
        {
        double Tolerance = HGLOBAL_EPSILON;

        // Extract new extent ... this puts extent up to date
        HGF2DExtent NewExtent(pNewComplex->GetExtent());

        if (NewExtent.IsDefined())
            {

            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetXMin()));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetXMax()));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetYMin()));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetYMax()));
            }

        // Set tolerance
        pNewComplex->SetTolerance(Tolerance);
        }

    // Copy Stroke Tolerance
    pNewComplex->SetStrokeTolerance(m_pStrokeTolerance);

    return (pNewComplex);
    }

//-----------------------------------------------------------------------------
// AreAdjacent
// Indicates if the linear is adjacent to the given
//-----------------------------------------------------------------------------
bool HVE2DComplexLinear::AreAdjacent(const HVE2DVector& pi_rVector) const
    {
    bool   DoAreAdjacent = false;

    // Create an iterator
    HVE2DComplexLinear::LinearList::const_iterator   MyIterator = m_LinearList.begin();

    // For every linear in complex
    while (MyIterator != m_LinearList.end() && !DoAreAdjacent)
        {
        // Check if component is adjacent to given vector
        DoAreAdjacent = (*MyIterator)->AreAdjacent(pi_rVector);

        // Advance to next component
        MyIterator++;
        }

    return(DoAreAdjacent);
    }


//-----------------------------------------------------------------------------
// AreContiguous
// Indicates if the linear is contiguous to the given
//-----------------------------------------------------------------------------
bool HVE2DComplexLinear::AreContiguous(const HVE2DVector& pi_rVector) const
    {
    bool   DoAreContiguous = false;

    if (!IsNull() && !pi_rVector.IsNull())
        {
        // Create an iterator
        HVE2DComplexLinear::LinearList::const_iterator   MyIterator = m_LinearList.begin();

        // For every linear in complex
        while (MyIterator != m_LinearList.end() && !DoAreContiguous)
            {
            // Check if component is contiguous to given vector
            DoAreContiguous = (*MyIterator)->AreContiguous(pi_rVector);

            // Advance to next component
            MyIterator++;
            }
        }

    return(DoAreContiguous);
    }


//-----------------------------------------------------------------------------
// AreContiguousAt
// Indicates if the linear is contiguous at the given point
//-----------------------------------------------------------------------------
bool HVE2DComplexLinear::AreContiguousAt(const HVE2DVector& pi_rVector,
                                          const HGF2DLocation& pi_rPoint) const
    {
    // The given point must be located on both vectors
    HPRECONDITION(IsPointOn(pi_rPoint) && pi_rVector.IsPointOn(pi_rPoint));

    bool   DoAreContiguous = false;

    if (!IsNull() && !pi_rVector.IsNull())
        {
        // Obtain tolerance
        double Tolerance = MIN(GetTolerance(), pi_rVector.GetTolerance());

        // Create an iterator
        HVE2DComplexLinear::LinearList::const_iterator   MyIterator = m_LinearList.begin();

        // For every linear in complex
        while (MyIterator != m_LinearList.end() && !DoAreContiguous)
            {
            if ((*MyIterator)->IsPointOn(pi_rPoint, HVE2DVector::INCLUDE_EXTREMITIES, Tolerance))
                {
                // Check if component is contiguous to given vector
                DoAreContiguous = (*MyIterator)->AreContiguousAt(pi_rVector, pi_rPoint);
                }

            // Advance to next component
            MyIterator++;
            }
        }

    return(DoAreContiguous);
    }



#if (0)
//-----------------------------------------------------------------------------
// Flirts
// Indicates if the linear is flirts the given
// INCOMPLETE ?????
//-----------------------------------------------------------------------------
bool HVE2DComplexLinear::Flirts(const HVE2DVector& pi_rVector) const
    {
    bool   DoAreFlirting = false;

    // A flirting linear may not cross
    if (!Crosses(pi_rVector))
        {
        // Create an iterator
        HVE2DComplexLinear::LinearList::const_iterator   MyIterator = m_LinearList.begin();

        // For every linear in complex
        while (MyIterator != m_LinearList.end() && !DoAreFlirting)
            {
            // Check if components flirt
            DoAreFlirting = (*MyIterator)->Flirts(pi_rVector);

            MyIterator++;
            }
        }

    return(DoAreFlirting);
    }
#endif


/** -----------------------------------------------------------------------------
    This method splits the complex linear components at all intersection
    points it encounter with given vector. The process of splitting consist
    in taking one component on which an intersection point falls, and making
    two component linears with it inside the complex linear.

    @param pi_rVector Constant reference to vector to obtain intersection points with.

    Example:
    @code
    HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
    HGF2DLocation        MyFirstPoint(10, 10,  pMyWorld);
    HGF2DLocation        MySecondPoint(15, 16, pMyWorld);
    HVE2DSegment        MySeg1(MyFirstPoint, MySecondPoint);
    ...
    HGF2DLocation          MyThirdPoint(12, 13, pMyWorld);
    HGF2Dlocation        MyFourthPoint(10, 10, pMyWorld);
    HVE2DSegment        MySeg2(MyThirdPoint, MyFourthPoint);

    HVE2DComplexLinear    MyComplex(pMyWorld);
    MyComplex.InsertLinear(MySeg1);
    MyComplex.InsertLinearInSameCoordSys(MySeg2

    HVE2DSegment    SplitSegment(HGF2DLocation(0.0, 0.0, pMyWorld),
                                HGF2DLocation(25.0, 25.0, pMyWorld));

    MyComplex.SplitAtAllIntersectionPoints(SplitSegment);

    @end

    @see SplitAtAllOnPoint()
    @see HVE2DVector
    -----------------------------------------------------------------------------
*/
void HVE2DComplexLinear::SplitAtAllIntersectionPoints(const HVE2DVector& pi_rVector)
    {
//    HPRECONDITION(!AutoCrosses());

    // Create list for receiving list of intersection points
    HGF2DLocationCollection MyListOfIntersectPoints;

    // We intersect self with the given linear
    Intersect(pi_rVector, &MyListOfIntersectPoints);

    // Split linear at all these points
    SplitAtAllOnPoints(MyListOfIntersectPoints);
    }

/** -----------------------------------------------------------------------------
    This method splits the complex linear components at given points which
    must all be located on the complex linear.
    The process of splitting consists in taking one component on which
    a point falls, and making two component linears with it inside the
    complex linear.

    @param pi_rPoints A constant reference to location collection containing
                      a series of points that must all be located upon the
                      complex linear and at which split points will be addded.

    Example:
    @code
    HFCPtr<HGF2DCoordSys>    pMyWorld(new HGF2DCoordSys());
    HGF2DLocation        MyFirstPoint(10, 10, pMyWorld);
    HGF2DLocation        MySecondPoint(15, 16, pMyWorld);
    HVE2DSegment        MySeg1(MyFirstPoint, MySecondPoint);
    ...
    HGF2DLocation          MyThirdPoint(12, 13, pMyWorld);
    HGF2Dlocation        MyFourthPoint(10, 10,  pMyWorld);
    HVE2DSegment        MySeg2(MyThirdPoint, MyFourthPoint);

    HVE2DComplexLinear    MyComplex(pMyWorld);
    MyComplex.InsertLinear(MySeg1);
    MyComplex.InsertLinearInSameCoordSys(MySeg2

    HVE2DSegment    SplitSegment(HGF2DLocation(0.0, 0.0, pMyWorld),
                                HGF2DLocation(25.0, 25.0, pMyWorld));

    HGF2DLocationCollection    SplitPoints;

    if (SplitSegment.Intersect(MyComplex, &SplitPoints) > 0)
    {
        MyComplex.SplitAtAllOnPoints(SplitPoints);
    }
    @end

    @see SplitAtAllIntersectionPoints()
    @see SplitAtAllOnPointsSCS()
    -----------------------------------------------------------------------------
*/
void HVE2DComplexLinear::SplitAtAllOnPoints(const HGF2DLocationCollection& pi_rPoints)
    {
//    HPRECONDITION(!AutoCrosses());

    // For every point in intersection list
    for (size_t Index = 0 ; Index < pi_rPoints.size() ; Index++)
        {

        HASSERT(IsPointOn(pi_rPoints[Index]));

        // Declare iterator to list of linears
        HVE2DComplexLinear::LinearList::iterator   MyIterator = m_LinearList.begin();

        // Loop till the linear on which the split length falls
        while ((MyIterator != m_LinearList.end()) && (!(*MyIterator)->IsPointOn(pi_rPoints[Index])))
            {
            // Go on to next linear
            MyIterator++;
            }

        // At this time, the iterator points on the linear on which is the split point
        // If this is not one of the extremity point ...
        // We ask this linear to shorten itself
        if ((!pi_rPoints[Index].IsEqualTo((*MyIterator)->GetStartPoint(), GetTolerance())) &&
            (!pi_rPoints[Index].IsEqualTo((*MyIterator)->GetEndPoint(), GetTolerance())))
            {
            // Make a copy of linear
            HVE2DLinear* pOtherLinear = (HVE2DLinear*)(*MyIterator)->Clone();

            // Shorten linear to split point
            (*MyIterator)->ShortenTo(pi_rPoints[Index]);

            // Shorten clone copy
            pOtherLinear->ShortenFrom(pi_rPoints[Index]);

            // Advance iterator
            MyIterator++;

            // Insert shortened copy
            m_LinearList.insert(MyIterator, pOtherLinear);
            }
        }
    }


/** -----------------------------------------------------------------------------
    This method splits the complex linear components at given points which
    must all be located on the complex linear.
    The process of splitting consists in taking one component on which
    a point falls, and making two component linears with it inside the
    complex linear.
    The method works exactly like SplitAtAllOnPoints() excpet that this version
    assumes and requires that all points part of the collection be expressed
    in the same interpretation coordinate system as the complex linear.

    @param pi_rPoints A constant reference to location collection containing
                      a series of points that must all be located upon the
                      complex linear and at which split points will be addded.

    @see SplitAtAllOnPoints()
    @see SplitAtAllIntersectionPoints()
    -----------------------------------------------------------------------------
*/
void HVE2DComplexLinear::SplitAtAllOnPointsSCS(const HGF2DLocationCollection& pi_rPoints)
    {
//    HPRECONDITION(!AutoCrosses());

    // For every point in intersection list
    for (size_t Index = 0 ; Index < pi_rPoints.size() ; Index++)
        {


        // Declare iterator to list of linears
        HVE2DComplexLinear::LinearList::iterator   MyIterator = m_LinearList.begin();

        // Loop till the linear on which the split length falls
        while ((MyIterator != m_LinearList.end()) && (!(*MyIterator)->IsPointOnSCS(pi_rPoints[Index])))
            {
            // Go on to next linear
            MyIterator++;
            }

        // At this time, the iterator points on the linear on which is the split point
        // If this is not one of the extremity point ...
        // We ask this linear to shorten itself

        if ((!pi_rPoints[Index].IsEqualTo((*MyIterator)->GetStartPoint(), GetTolerance())) &&
            (!pi_rPoints[Index].IsEqualTo((*MyIterator)->GetEndPoint(), GetTolerance())))
            {
            // Make a copy of linear
            HVE2DLinear* pOtherLinear = (HVE2DLinear*)(*MyIterator)->Clone();

            // Shorten linear to split point
            (*MyIterator)->ShortenTo(pi_rPoints[Index]);

            // Shorten clone copy
            pOtherLinear->ShortenFrom(pi_rPoints[Index]);

            // Advance iterator
            MyIterator++;

            // Insert shortened copy
            m_LinearList.insert(MyIterator, pOtherLinear);
            }
        }
    }


//-----------------------------------------------------------------------------
// AdjustStartPointTo
// Adjust the start point to an more exact point located less than
// an epsilon from previous point
//-----------------------------------------------------------------------------
void HVE2DComplexLinear::AdjustStartPointTo(const HGF2DLocation& pi_rPoint)
    {
    HPRECONDITION(m_StartPoint.IsEqualTo(pi_rPoint, GetTolerance()));

    // Adjust point
    m_StartPoint.Set(pi_rPoint);

    // Adjust point of first component
    (*(m_LinearList.begin()))->AdjustEndPointTo(pi_rPoint);

    // Indicate extent and length are no more up to date
    m_ExtentUpToDate = false;
    m_LengthUpToDate = false;

    // We do not adjust tolerance since modification is small

    }

//-----------------------------------------------------------------------------
// AdjustEndPointTo
// Adjust the end point to an more exact point located less than
// an epsilon from previous point
//-----------------------------------------------------------------------------
void HVE2DComplexLinear::AdjustEndPointTo(const HGF2DLocation& pi_rPoint)
    {
    HPRECONDITION(m_EndPoint.IsEqualTo(pi_rPoint, GetTolerance()));

    // Adjust point
    m_EndPoint.Set(pi_rPoint);

    // Adjust point of last component
    (*(m_LinearList.rbegin()))->AdjustEndPointTo(pi_rPoint);

    // Indicate extent and length are no more up to date
    m_ExtentUpToDate = false;
    m_LengthUpToDate = false;

    // We do not adjust tolerance since modification is small
    }



//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HVE2DComplexLinear::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    po_rOutput << "Object is a HVE2DComplexLinear" << endl;
    HDUMP0("Object is a HVE2DComplexLinear\n");
    HVE2DLinear::PrintState(po_rOutput);
    po_rOutput << "Begin component listing" << endl;
    HDUMP0("Begin component listing\n");

    HVE2DComplexLinear::LinearList::const_iterator  MyIterator;

    // We print the state of every component
    for (MyIterator = m_LinearList.begin() ;
         MyIterator != m_LinearList.end() ; MyIterator++)
        (*MyIterator)->PrintState(po_rOutput);

    HDUMP0("END OF COMPONENT LISTING\n");

#endif
    }


//-----------------------------------------------------------------------------
// Drop
// Returns the description of linear in the form of raw location
// segments
//-----------------------------------------------------------------------------
inline void HVE2DComplexLinear::Drop(HGF2DLocationCollection* po_pPoints,
                                     double                   pi_Tolerance,
                                     EndPointProcessing       pi_EndPointProcessing) const
    {
    HPRECONDITION(po_pPoints != 0);

    // NOTE : Tolerance is unused since drop is exact

    // Declkare iterator upon components
    HVE2DComplexLinear::LinearList::const_iterator  MyIterator;

    // We print the state of every component
    for (MyIterator = m_LinearList.begin() ; MyIterator != m_LinearList.end() ; MyIterator++)
        {
        // Drop this component
        (*MyIterator)->Drop(po_pPoints, pi_Tolerance, HVE2DLinear::EXCLUDE_END_POINT);
        }

    // Check if end point must be added
    if (pi_EndPointProcessing == HVE2DLinear::INCLUDE_END_POINT)
        {
        // Must be added ... add it
        po_pPoints->push_back(m_EndPoint);
        }
    }

//-----------------------------------------------------------------------------
// SetTolerance
// Sets the tolerance and in addition sets the tolerance of all components
//-----------------------------------------------------------------------------
void HVE2DComplexLinear::SetTolerance(double pi_Tolerance)
    {
    // The tolerance must be greater than 0.0
    HPRECONDITION(pi_Tolerance > 0.0);

    // Check if tolerance different from provided one (Exact compare)
    if (GetTolerance() != pi_Tolerance)
        {
        // Set tolerance of every component
        HVE2DComplexLinear::LinearList::iterator  MyIterator;

        // We set tolerance of each
        for (MyIterator = m_LinearList.begin() ; MyIterator != m_LinearList.end() ; MyIterator++)
            (*MyIterator)->SetTolerance(pi_Tolerance);

        // Call ancester
        HVE2DVector::SetTolerance(pi_Tolerance);
        }
    }

//-----------------------------------------------------------------------------
// SetTolerance
// Sets the stroke tolerance and in addition sets the stroke tolerance of all components
//-----------------------------------------------------------------------------
void HVE2DComplexLinear::SetStrokeTolerance(const HFCPtr<HGFTolerance> & pi_Tolerance)
    {
    // Set tolerance of every component
    HVE2DComplexLinear::LinearList::iterator  MyIterator;

    // We set tolerance of each
    for (MyIterator = m_LinearList.begin() ; MyIterator != m_LinearList.end() ; MyIterator++)
        (*MyIterator)->SetStrokeTolerance(pi_Tolerance);

    // Call ancester
    HVE2DVector::SetStrokeTolerance(pi_Tolerance);
    }
