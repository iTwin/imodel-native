//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVE2DVectorGroup.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HVE2DVectorGroup.h>


HPM_REGISTER_CLASS(HVE2DVectorGroup, HVE2DVector)


//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another HVE2DVectorGroup object.
//-----------------------------------------------------------------------------
HVE2DVectorGroup::HVE2DVectorGroup(const HVE2DVectorGroup& pi_rObj)
    : HVE2DVector(pi_rObj),
      m_Extent(pi_rObj.m_Extent),
      m_ExtentUpToDate(pi_rObj.m_ExtentUpToDate)
    {
    // Declare iterator upon components
    HVE2DVectorGroup::VectorList::const_iterator  MyIterator;

    try
        {
        // We draw every component
        for (MyIterator = pi_rObj.m_VectorList.begin() ;
             MyIterator != pi_rObj.m_VectorList.end() ; ++MyIterator)
            {
            // Duplicate component
            m_VectorList.push_back((HVE2DVector*)(*MyIterator)->Clone());
            }
        }
    catch(...)
        {
        MakeEmpty();
        throw;
        }
    }


//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another vector object.
//-----------------------------------------------------------------------------
HVE2DVectorGroup& HVE2DVectorGroup::operator=(const HVE2DVectorGroup& pi_rObj)
    {
    // Check that object is not self
    if (this != &pi_rObj)
        {
        MakeEmpty();

        HVE2DVector::operator=(pi_rObj);

        // Declare iterator upon components
        HVE2DVectorGroup::VectorList::const_iterator  MyIterator;

        // We draw every component
        for (MyIterator = pi_rObj.m_VectorList.begin() ;
             MyIterator != pi_rObj.m_VectorList.end() ; ++MyIterator)
            {
            // Duplicate component
            m_VectorList.push_back((HVE2DVector*)(*MyIterator)->Clone());
            }
        }

    // Return reference to self
    return (*this);
    }


//-----------------------------------------------------------------------------
// SetCoordSysImplementation
// Changes the interpretation coordinate system
//-----------------------------------------------------------------------------
void HVE2DVectorGroup::SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys)
    {
    // We call ancester to update extremity points
    HVE2DVector::SetCoordSysImplementation(pi_rpCoordSys);

    // We set coordsys of each and every part of the group
    HVE2DVectorGroup::VectorList::iterator  MyIterator;
    for (MyIterator = m_VectorList.begin() ; MyIterator != m_VectorList.end() ; MyIterator++)
        (*MyIterator)->SetCoordSys(GetCoordSys());

    // Indicate extent and length are no more up to date
    m_ExtentUpToDate = false;
    }

//-----------------------------------------------------------------------------
// AddVector
// Adds a vector to the beginning of the group
//-----------------------------------------------------------------------------
void HVE2DVectorGroup::AddVector(HVE2DVector* pi_pVector)
    {
    // The given vector must not be located in group
//    HPRECONDITION(m_VectorList.find(pi_pVector) == m_VectorList.end());

    // Add vector to list
    m_VectorList.push_back(pi_pVector);

    // Indicate extent and length are no more up to date
    m_ExtentUpToDate = false;
    }


//-----------------------------------------------------------------------------
// Move
// Moves the group by a specified displacement
//-----------------------------------------------------------------------------
void HVE2DVectorGroup::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    HVE2DVectorGroup::VectorList::iterator  MyIterator;

    // We move each and every part of the group by the displacement
    for (MyIterator = m_VectorList.begin() ; MyIterator != m_VectorList.end() ; MyIterator++)
        (*MyIterator)->Move(pi_rDisplacement);

#if (0)
    // We call ancester move to update extremity points
    HVE2DVector::Move(pi_rDisplacement);
#endif

    // Indicate extent and length are no more up to date
    m_ExtentUpToDate = false;

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
// Scales the group by a specified factor
//-----------------------------------------------------------------------------
void HVE2DVectorGroup::Scale(double pi_ScaleFactor, const HGF2DLocation& pi_rScaleOrigin)
    {
    // The given scale factor may not be equal to 0.0
    HPRECONDITION(pi_ScaleFactor != 0.0);

    HVE2DVectorGroup::VectorList::iterator  MyIterator;

    // We scale each and every part of the group by the specified parameters
    for (MyIterator = m_VectorList.begin() ; MyIterator != m_VectorList.end() ; MyIterator++)
        (*MyIterator)->Scale(pi_ScaleFactor, pi_rScaleOrigin);

#if (0)
    // We call ancester scale
    HVE2DVector::Scale(pi_ScaleFactor, pi_rScaleOrigin);
#endif

    // Indicate extent and length are no more up to date
    m_ExtentUpToDate = false;

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
// CalculateBearing
// Calculates and returns the bearing of group at specified point
//-----------------------------------------------------------------------------
HGFBearing HVE2DVectorGroup::CalculateBearing(const HGF2DLocation& pi_rPoint,
                                              HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on group
    HPRECONDITION(IsPointOn(pi_rPoint));

    // The bearing on group is the bearing on the vector part of it
    // on which the point is located
    HVE2DVectorGroup::VectorList::const_iterator   MyIterator = m_VectorList.begin();

    // Loop till the vector on which the point is is found
    while (!(*MyIterator)->IsPointOn(pi_rPoint) || (*MyIterator)->IsNull())
        MyIterator++;

    // Return bearing
    return ((*MyIterator)->CalculateBearing(pi_rPoint, pi_Direction));
    }

//-----------------------------------------------------------------------------
// CalculateAngularAcceleration
// Calculates and returns the angular acceleration of vector at specified point
//-----------------------------------------------------------------------------
double HVE2DVectorGroup::CalculateAngularAcceleration(const HGF2DLocation& pi_rPoint,
                                                      HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on group
    HPRECONDITION(IsPointOn(pi_rPoint));

    // The angular acceleration on group is the acceleration on the vector part of it
    // on which the point is located
    HVE2DVectorGroup::VectorList::const_iterator   MyIterator = m_VectorList.begin();

    // Loop till the vector on which the point is is found
    while (!(*MyIterator)->IsPointOn(pi_rPoint))
        MyIterator++;

    // Return angular acceleration
    return ((*MyIterator)->CalculateAngularAcceleration(pi_rPoint, pi_Direction));
    }



//-----------------------------------------------------------------------------
// CalculateClosestPoint
// Calculates and returns the closest point on vector from given point
//-----------------------------------------------------------------------------
HGF2DLocation HVE2DVectorGroup::CalculateClosestPoint(const HGF2DLocation& pi_rLocation) const
    {
    HGF2DLocation   ClosestPoint(GetCoordSys());
    bool            DistanceInitialized = false;
    double          TempDistance;
    HGF2DLocation   TempPoint(GetCoordSys());
    double          ShortestDistance=0.0;

    // Create an iterator
    HVE2DVectorGroup::VectorList::const_iterator   MyIterator = m_VectorList.begin();

    // Loop and ask to each part for the closest point
    while (MyIterator != m_VectorList.end())
        {
        // We ask current component vector for its closest point
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

        // Advance to next component vector
        MyIterator++;
        }

    return(ClosestPoint);
    }


//-----------------------------------------------------------------------------
// Crosses
// Indicates if the two vector cross each other
//-----------------------------------------------------------------------------
bool HVE2DVectorGroup::Crosses(const HVE2DVector& pi_rVector) const
    {
    bool   IsCrossing = false;

    // Check their extent overlap
    if (GetExtent().DoTheyOverlap(pi_rVector.GetExtent()))
        {
        // For every vector part of the self group ...
        HVE2DVectorGroup::VectorList::const_iterator MySelfIterator = m_VectorList.begin();

        while ((MySelfIterator != m_VectorList.end()) &&  !(IsCrossing))
            {
            // Check if current component crosses
            IsCrossing = pi_rVector.Crosses(**MySelfIterator);

            // Advance to next component
            MySelfIterator++;
            }
        }


    return (IsCrossing);
    }


//-----------------------------------------------------------------------------
// IsPointOn
// Checks if the point is located on the group
//-----------------------------------------------------------------------------
bool   HVE2DVectorGroup::IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                    HVE2DVector::ExtremityProcessing pi_ExtremityProcessing,
                                    double pi_Tolerance) const
    {
    bool   IsOn = false;

    // If the point has the same coordinate system...
    if (pi_rTestPoint.GetCoordSys() == GetCoordSys())
        IsOn = IsPointOnSCS(pi_rTestPoint, pi_ExtremityProcessing, pi_Tolerance);
    else
        {
        // For every vector part of the group ... check if point is on
        for (HVE2DVectorGroup::VectorList::const_iterator MyIterator = m_VectorList.begin();
             (MyIterator != m_VectorList.end() && !(IsOn = (*MyIterator)->IsPointOn(pi_rTestPoint, pi_ExtremityProcessing, pi_Tolerance)));
             MyIterator++)
            ;
        }

    return (IsOn);
    }

//-----------------------------------------------------------------------------
// IsPointOnSCS
// Checks if the point is located on the group
//-----------------------------------------------------------------------------
bool   HVE2DVectorGroup::IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                       HVE2DVector::ExtremityProcessing pi_ExtremityProcessing,
                                       double pi_Tolerance) const
    {
    // The point must have the same coordinate system as vector
    HPRECONDITION(GetCoordSys() == pi_rTestPoint.GetCoordSys());

    bool   IsOn = false;

    // For every vector part of the group ... check if point is on
    for (HVE2DVectorGroup::VectorList::const_iterator MyIterator = m_VectorList.begin();
         (MyIterator != m_VectorList.end() && !(IsOn = (*MyIterator)->IsPointOnSCS(pi_rTestPoint, pi_ExtremityProcessing, pi_Tolerance)));
         MyIterator++)
        ;

    return (IsOn);
    }


//-----------------------------------------------------------------------------
// ObtainContiguousnessPoints
// Finds contiguousness points with vector
//-----------------------------------------------------------------------------
size_t HVE2DVectorGroup::ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                    HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    HPRECONDITION(po_pContiguousnessPoints != 0);

    // Save initial number of points
    size_t  InitialNumberOfPoints = po_pContiguousnessPoints->size();

    // For every vector part of the group ... obtain contiguousness points
    for (HVE2DVectorGroup::VectorList::const_iterator MyIterator = m_VectorList.begin();
         MyIterator != m_VectorList.end();
         MyIterator++)
        {
        // Check if this part is contiguous
        if ((*MyIterator)->AreContiguous(pi_rVector))
            {
            // Obtain contiguousness points
            (*MyIterator)->ObtainContiguousnessPoints(pi_rVector, po_pContiguousnessPoints);
            }
        }

    return (po_pContiguousnessPoints->size() - InitialNumberOfPoints);
    }



//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsAt
// Finds contiguousness points with vector
//-----------------------------------------------------------------------------
void HVE2DVectorGroup::ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                    const HGF2DLocation& pi_rPoint,
                                                    HGF2DLocation* po_pFirstContiguousnessPoint,
                                                    HGF2DLocation* po_pSecondContiguousnessPoint) const
    {
    // The vectors must be contiguous at specified point
    HPRECONDITION(AreContiguousAt(pi_rVector, pi_rPoint));

    bool   Found = false;

    // For every vector part of the group ... obtain contiguousness points
    HVE2DVectorGroup::VectorList::const_iterator MyIterator = m_VectorList.begin();

    while(MyIterator != m_VectorList.end() && (Found == false))
        {
        // Check if point is on
        if ((*MyIterator)->IsPointOn(pi_rPoint))
            {
            // Check if this part is contiguous at specified point
            if ((*MyIterator)->AreContiguousAt(pi_rVector, pi_rPoint))
                {

                // Obtain contiguousness points
                (*MyIterator)->ObtainContiguousnessPointsAt(pi_rVector, pi_rPoint, po_pFirstContiguousnessPoint, po_pSecondContiguousnessPoint);

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

    }

//-----------------------------------------------------------------------------
// Intersect
// Finds intersection points with vector
//-----------------------------------------------------------------------------
size_t HVE2DVectorGroup::Intersect(const HVE2DVector& pi_rVector,
                                   HGF2DLocationCollection* po_pCrossPoints) const
    {
    size_t  NumberOfNewPoints = 0;

    // Check that their extent overlap
    if (GetExtent().DoTheyOverlap(pi_rVector.GetExtent()))
        {
        // Check if they have a direction preserving relation
        // The following procedure only works if the transformation model between vectors
        // Preserves direction
        if ((GetCoordSys() == pi_rVector.GetCoordSys()) ||
            GetCoordSys()->HasDirectionPreservingRelationTo(pi_rVector.GetCoordSys()))
            {
            HVE2DVectorGroup::VectorList::const_iterator MyPreviousIterator;

            // For every vector part of the group ... obtain intersection points
            for (HVE2DVectorGroup::VectorList::const_iterator MyIterator = m_VectorList.begin();
                 MyIterator != m_VectorList.end();
                 MyIterator++)
                {
                // Obtain intersection points
                NumberOfNewPoints += (*MyIterator)->Intersect(pi_rVector, po_pCrossPoints);

                }
            }
        }

    return (NumberOfNewPoints);
    }

//-----------------------------------------------------------------------------
// GetExtent
// Returns the extent of the group
//-----------------------------------------------------------------------------
HGF2DExtent HVE2DVectorGroup::GetExtent() const
    {
    if (!m_ExtentUpToDate)
        {
        // Cast out of const
        HGF2DExtent*    pTheExtent = (HGF2DExtent*)&m_Extent;

        // Force extent to empty and set appropriate coordinate system
        *pTheExtent = HGF2DExtent(GetCoordSys());

        // For every vector part of the group ... merge extents
        for (HVE2DVectorGroup::VectorList::const_iterator MyIterator = m_VectorList.begin();
             (MyIterator != m_VectorList.end());
             MyIterator++)
            {
            pTheExtent->Add((*MyIterator)->GetExtent());
            }

        (*((bool*)&m_ExtentUpToDate)) = true;
        }

    return (m_Extent);
    }


//-----------------------------------------------------------------------------
// MakeEmpty
// Empties the vector
//-----------------------------------------------------------------------------
void HVE2DVectorGroup::MakeEmpty()
    {
    // Declare iterator upon components
    HVE2DVectorGroup::VectorList::const_iterator  MyIterator;

    // We draw every component
    for (MyIterator = m_VectorList.begin() ;
         MyIterator != m_VectorList.end() ; ++MyIterator)
        {
        // Destroy component
        delete (*MyIterator);
        }

    // Clear list of nodes
    m_VectorList.clear();

    // Indicate extent and length are no more up to date
    m_ExtentUpToDate = false;

    // Set tolerance back to default
    SetTolerance(HGLOBAL_EPSILON);
    }


//-----------------------------------------------------------------------------
// AllocateCopyInCoordSys
// Returns a dynamically allocated copy of the group in a different
// coordinate system
//-----------------------------------------------------------------------------
HVE2DVector* HVE2DVectorGroup::AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {

    // Create new group
    HVE2DVectorGroup* pNewGroup = new HVE2DVectorGroup(pi_rpCoordSys);

    // Create an iterator
    HVE2DVectorGroup::VectorList::const_iterator   MyIterator = m_VectorList.begin();

    // For every vector in group
    while (MyIterator != m_VectorList.end())
        {
        // Allocate a copy of component in new coordinate system, and add it to new group
        HVE2DVector* pNewVector = (HVE2DVector*)((*MyIterator)->AllocateCopyInCoordSys(pi_rpCoordSys));

        // Check if new vector is null
        if (!pNewVector->IsNull())
            pNewGroup->m_VectorList.push_back(pNewVector);

        // Advance to next component
        MyIterator++;
        }

    // Set tolerance
    if (IsAutoToleranceActive())
        {
        double Tolerance = HGLOBAL_EPSILON;

        // Extract new extent ... this puts extent up to date
        HGF2DExtent NewExtent(pNewGroup->GetExtent());

        if (NewExtent.IsDefined())
            {

            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetXMin()));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetXMax()));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetYMin()));
            Tolerance = MAX(Tolerance, HEPSILON_MULTIPLICATOR * fabs(NewExtent.GetYMax()));
            }

        // Set tolerance
        pNewGroup->SetTolerance(Tolerance);
        }

    pNewGroup->SetStrokeTolerance(m_pStrokeTolerance);

    return (pNewGroup);
    }

//-----------------------------------------------------------------------------
// AreAdjacent
// Indicates if the vector is adjacent to the given
//-----------------------------------------------------------------------------
bool HVE2DVectorGroup::AreAdjacent(const HVE2DVector& pi_rVector) const
    {
    bool   DoAreAdjacent = false;

    // Create an iterator
    HVE2DVectorGroup::VectorList::const_iterator   MyIterator = m_VectorList.begin();

    // For every vector in group
    while (MyIterator != m_VectorList.end() && !DoAreAdjacent)
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
// Indicates if the vector is contiguous to the given
//-----------------------------------------------------------------------------
bool HVE2DVectorGroup::AreContiguous(const HVE2DVector& pi_rVector) const
    {
    bool   DoAreContiguous = false;

    if (!IsNull() && !pi_rVector.IsNull())
        {
        // Create an iterator
        HVE2DVectorGroup::VectorList::const_iterator   MyIterator = m_VectorList.begin();

        // For every vector in group
        while (MyIterator != m_VectorList.end() && !DoAreContiguous)
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
// Indicates if the vector is contiguous at the given point
//-----------------------------------------------------------------------------
bool HVE2DVectorGroup::AreContiguousAt(const HVE2DVector& pi_rVector,
                                        const HGF2DLocation& pi_rPoint) const
    {
    // The given point must be located on both vectors
    HPRECONDITION(IsPointOn(pi_rPoint) && pi_rVector.IsPointOn(pi_rPoint));

    bool   DoAreContiguous = false;

    if (!IsNull() && !pi_rVector.IsNull())
        {
        // Create an iterator
        HVE2DVectorGroup::VectorList::const_iterator   MyIterator = m_VectorList.begin();

        // For every vector in group
        while (MyIterator != m_VectorList.end() && !DoAreContiguous)
            {
            if ((*MyIterator)->IsPointOn(pi_rPoint))
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



//-----------------------------------------------------------------------------
// IsAtAnExtremity
// Determines if the given point is located at an extremity
//-----------------------------------------------------------------------------
inline bool HVE2DVectorGroup::IsAtAnExtremity(const HGF2DLocation& pi_rLocation,
                                               double pi_Tolerance) const
    {
    bool   AtExtremity = false;

    if (!IsNull())
        {
        // Create an iterator
        HVE2DVectorGroup::VectorList::const_iterator   MyIterator = m_VectorList.begin();

        // For every vector in group until is on extremity
        while (MyIterator != m_VectorList.end() && !AtExtremity)
            {
            // Check if point is at extremity of component
            AtExtremity = (*MyIterator)->IsAtAnExtremity(pi_rLocation, pi_Tolerance);

            // Advance to next component
            ++MyIterator;
            }
        }

    return(AtExtremity);
    }



//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HVE2DVectorGroup::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    po_rOutput << "Object is a HVE2DVectorGroup" << endl;
    HDUMP0("Object is a HVE2DVectorGroup\n");
    HVE2DVector::PrintState(po_rOutput);
    po_rOutput << "Begin component listing" << endl;
    HDUMP0("Begin component listing\n");

    HVE2DVectorGroup::VectorList::const_iterator  MyIterator;

    // We print the state of every component
    for (MyIterator = m_VectorList.begin() ;
         MyIterator != m_VectorList.end() ; MyIterator++)
        (*MyIterator)->PrintState(po_rOutput);

    HDUMP0("END OF COMPONENT LISTING\n");

#endif
    }


//-----------------------------------------------------------------------------
// SetTolerance
// Sets the tolerance and in addition sets the tolerance of all components
//-----------------------------------------------------------------------------
void HVE2DVectorGroup::SetTolerance(double pi_Tolerance)
    {
    // The tolerance must be greater than 0.0
    HPRECONDITION(pi_Tolerance > 0.0);

    // Set tolerance of every component
    HVE2DVectorGroup::VectorList::iterator  MyIterator;

    // We set tolerance of each
    for (MyIterator = m_VectorList.begin() ; MyIterator != m_VectorList.end() ; MyIterator++)
        (*MyIterator)->SetTolerance(pi_Tolerance);

    // Call ancester
    HVE2DVector::SetTolerance(pi_Tolerance);
    }

//-----------------------------------------------------------------------------
// SetStrokeTolerance
// Sets the stroke tolerance and in addition sets the stroke tolerance of all components
//-----------------------------------------------------------------------------
void HVE2DVectorGroup::SetStrokeTolerance(const HFCPtr<HGFTolerance> & pi_Tolerance)
    {
    // Set tolerance of every component
    HVE2DVectorGroup::VectorList::iterator  MyIterator;

    // We set tolerance of each
    for (MyIterator = m_VectorList.begin() ; MyIterator != m_VectorList.end() ; MyIterator++)
        (*MyIterator)->SetStrokeTolerance(pi_Tolerance);

    // Call ancester
    HVE2DVector::SetStrokeTolerance(pi_Tolerance);
    }

//-----------------------------------------------------------------------------
// SetAutoToleranceActive
// Sets the auto tolerance active to the components
//-----------------------------------------------------------------------------
inline void HVE2DVectorGroup::SetAutoToleranceActive(bool pi_AutoToleranceActive)
    {
    // Set tolerance of every component
    HVE2DVectorGroup::VectorList::iterator  MyIterator;

    // We set tolerance of each
    for (MyIterator = m_VectorList.begin() ;
         MyIterator != m_VectorList.end() ; MyIterator++)
        (*MyIterator)->SetAutoToleranceActive(pi_AutoToleranceActive);

    // Call ancester
    HVE2DVector::SetAutoToleranceActive(pi_AutoToleranceActive);
    }

