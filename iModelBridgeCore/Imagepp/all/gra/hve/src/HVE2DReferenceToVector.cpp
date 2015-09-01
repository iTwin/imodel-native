//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hve/src/HVE2DReferenceToVector.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Methods for class HVE2DReferenceToVector
//-----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>


#include <Imagepp/all/h/HVE2DReferenceToVector.h>
#include <Imagepp/all/h/HGF2DCoordSys.h>
#include <Imagepp/all/h/HGF2DTranslation.h>
#include <Imagepp/all/h/HGF2DSimilitude.h>
#include <Imagepp/all/h/HGF2DStretch.h>


HPM_REGISTER_CLASS(HVE2DReferenceToVector, HVE2DVector)


// HMG_BEGIN_DUPLEX_MESSAGE_MAP(HVE2DReferenceToVector, HVE2DVector, HMG_NO_NEED_COHERENCE_SECURITY)
// HMG_REGISTER_MESSAGE(HVE2DReferenceToVector, HGFGeometryChangedMsg, NotifyGeometryChanged)
// HMG_END_MESSAGE_MAP()


//-----------------------------------------------------------------------------
// Default constructor.
//-----------------------------------------------------------------------------
HVE2DReferenceToVector::HVE2DReferenceToVector()
    : HVE2DVector()
    {
    }


//-----------------------------------------------------------------------------
// Constructor from source only.
//-----------------------------------------------------------------------------
HVE2DReferenceToVector::HVE2DReferenceToVector(const HVE2DVector* pi_pSource)
    : HVE2DVector(pi_pSource->GetCoordSys()),
      m_pSource(pi_pSource)
    {
    // Link ourselves to the source vector, to receive notifications
//     LinkTo(m_pSource);
    }



//-----------------------------------------------------------------------------
// Constructor from source and coordinate system.
//-----------------------------------------------------------------------------
HVE2DReferenceToVector::HVE2DReferenceToVector(const HVE2DVector*   pi_pSource,
                                               const HFCPtr<HGF2DCoordSys>& pi_pCoordSys)
    : HVE2DVector(pi_pCoordSys),
      m_pSource(pi_pSource)
    {
    // Do we have a different coordinate system?
    if (pi_pCoordSys != pi_pSource->GetCoordSys())
        m_CoordSysChanged = true;
    else
        m_CoordSysChanged = false;

    // Link ourselves to the source vector, to receive notifications
//    LinkTo(m_pSource);
    }

//-----------------------------------------------------------------------------
// Copy constructor
//-----------------------------------------------------------------------------
HVE2DReferenceToVector::HVE2DReferenceToVector(const HVE2DReferenceToVector& pi_rObj)
    : HVE2DVector((HVE2DVector&)pi_rObj),
      m_pSource(pi_rObj.m_pSource)
    {
    HPRECONDITION(pi_rObj.m_pSource != 0);

    // Copy geometry status
    m_CoordSysChanged = pi_rObj.m_CoordSysChanged;

    // Link ourselves to the source vector, to receive notifications
//    LinkTo(m_pSource);
    }


//-----------------------------------------------------------------------------
// Destructor
//-----------------------------------------------------------------------------
HVE2DReferenceToVector::~HVE2DReferenceToVector()
    {
    if (m_pSource != 0)
        {
        // Unlink from the source vector
//        UnlinkFrom(m_pSource);
        }
    }


//-----------------------------------------------------------------------------
// operator=
// Assignment operation
//-----------------------------------------------------------------------------
HVE2DReferenceToVector& HVE2DReferenceToVector::operator=(const HVE2DReferenceToVector& pi_rObj)
    {
    HPRECONDITION(pi_rObj.m_pSource != 0);
    HPRECONDITION(m_pSource != 0);

    if (&pi_rObj != this)
        {
        // Unlink from our previous source
//        UnlinkFrom(m_pSource);

        // Copy the HVE2DVector portion
        HVE2DVector::operator=(pi_rObj);

        // Copy the other object's source pointer
        m_pSource = pi_rObj.m_pSource;

        // Copy geometry status
        m_CoordSysChanged = pi_rObj.m_CoordSysChanged;

        // Link ourselves to the new source vector, to receive notifications
//        LinkTo(m_pSource);
        }

    return(*this);
    }



//-----------------------------------------------------------------------------
// NotifyGeometryChanged
// Notification for geometry change
//-----------------------------------------------------------------------------
bool HVE2DReferenceToVector::NotifyGeometryChanged(HMGMessage& pi_rMessage)
    {
    // Retrieve pointer to sender object
    HFCPtr<HGF2DCoordSys> pSenderCoordSys = ((HVE2DVector*) pi_rMessage.GetSender())->GetCoordSys();

    if (m_CoordSysChanged)
        {
        // We have our own coordinate system. Change it to reflect
        // the source's change.

        // Set our new coordinate system. Use sender's CS as a reference,
        // keeping the transformation between our old CS and our old source's CS.
        SetCoordSys(new HGF2DCoordSys(*GetCoordSys()->GetTransfoModelTo(GetCoordSys()->GetReference()),
                                      pSenderCoordSys));
        }
    else
        {
        // Set our new coordinate system as the source's
        SetCoordSys(pSenderCoordSys);
        }

    // Stop message here. Will be propagated be SetCoordSys()
    return false;
    }


//-----------------------------------------------------------------------------
// SetCoordSysImplementation
// Set our coordinate system
//-----------------------------------------------------------------------------
void HVE2DReferenceToVector::SetCoordSysImplementation(const HFCPtr<HGF2DCoordSys>& pi_rCoordSys)
    {
    HPRECONDITION(m_pSource != 0);

    // Let our ancestor do the normal job
    HVE2DVector::SetCoordSysImplementation(pi_rCoordSys);

    if (GetCoordSys() == m_pSource->GetCoordSys())
        m_CoordSysChanged = false;
    else
        m_CoordSysChanged = true;
    }


//-----------------------------------------------------------------------------
// Move
// Move the reference
//-----------------------------------------------------------------------------
void HVE2DReferenceToVector::Move(const HGF2DDisplacement& pi_rDisplacement)
    {
    HPRECONDITION(m_pSource != 0);

    // Model with translation.
    HGF2DTranslation NewModel (pi_rDisplacement);

    if (m_CoordSysChanged)
        {
        // We have a private coordinate system. Simply change the
        // model between it and the source's coordinate system
        SetCoordSys(new HGF2DCoordSys(*NewModel.ComposeInverseWithDirectOf (
                                          *(GetCoordSys()->GetTransfoModelTo (
                                                m_pSource->GetCoordSys()))),
                                      m_pSource->GetCoordSys()));
        }
    else
        {
        // Create our coordinate system, using the new transformation.

        SetCoordSys(new HGF2DCoordSys(NewModel, m_pSource->GetCoordSys()));

        // Do not notify, since SetCoordSys() will do it for us...
        }
    }


//-----------------------------------------------------------------------------
// Rotate
// Rotate the reference around a point
//-----------------------------------------------------------------------------
void HVE2DReferenceToVector::Rotate(double               pi_Angle,
                                    const HGF2DLocation& pi_rOrigin)
    {
    HPRECONDITION(m_pSource != 0);

    // Model with translation.
    HGF2DSimilitude Rotation ();
    HGF2DLocation LogicalLocation(pi_rOrigin, GetCoordSys());
    Rotation.AddRotation(pi_Angle, LogicalLocation.GetX(), LogicalLocation.GetY());

    if (m_CoordSysChanged)
        {
        // We have a private coordinate system. Simply change the
        // model between it and the source's coordinate system

        SetCoordSys(new HGF2DCoordSys(*Rotation.ComposeInverseWithDirectOf (
                                          *(GetCoordSys()->GetTransfoModelTo (
                                                m_pSource->GetCoordSys()))),
                                      m_pSource->GetCoordSys()));
        }
    else
        {
        // Create our coordinate system, using the new transformation.

        SetCoordSys(new HGF2DCoordSys(Rotation, m_pSource->GetCoordSys()));

        // Do not notify, since SetCoordSys() will do it for us...
        }
    }


//-----------------------------------------------------------------------------
// Scale
// Scale the reference around a point
//-----------------------------------------------------------------------------
void HVE2DReferenceToVector::Scale(double pi_ScaleFactorX,
                                   double pi_ScaleFactorY,
                                   const HGF2DLocation& pi_rOrigin)
    {
    HPRECONDITION(m_pSource != 0);

    // Model with translation.
    HGF2DStretch Scale ();
    HGF2DLocation LogicalLocation(pi_rOrigin, GetCoordSys());
    Scale.AddAnisotropicScaling(pi_ScaleFactorX, pi_ScaleFactorY,
                                LogicalLocation.GetX(), LogicalLocation.GetY());

    if (m_CoordSysChanged)
        {
        // We have a private coordinate system. Simply change the
        // model between it and the source's coordinate system

        SetCoordSys(new HGF2DCoordSys(*Scale.ComposeInverseWithDirectOf (
                                          *(GetCoordSys()->GetTransfoModelTo (
                                                m_pSource->GetCoordSys()))),
                                      m_pSource->GetCoordSys()));
        }
    else
        {
        // Create our coordinate system, using the new transformation.

        SetCoordSys(new HGF2DCoordSys(Scale, m_pSource->GetCoordSys()));

        // Do not notify, since SetCoordSys() will do it for us...
        }
    }


//-----------------------------------------------------------------------------
// CalculateBearing
// Calculates and returns the bearing of reference to vector at specified point
//-----------------------------------------------------------------------------
HGFBearing HVE2DReferenceToVector::CalculateBearing(const HGF2DLocation& pi_rPoint,
                                                    HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on reference to vector
    HPRECONDITION(IsPointOn(pi_rPoint));

    // The bearing on reference to vector is the bearing on the vector source
    // in other coordinate system
    HGF2DLocation  Point(pi_rPoint, GetCoordSys());
    Point.SetCoordSys(m_pSource->GetCoordSys());

    // Return bearing
    return (m_pSource->CalculateBearing(pi_rPoint, pi_Direction));
    }

//-----------------------------------------------------------------------------
// CalculateAngularAcceleration
// Calculates and returns the angular acceleration of vector at specified point
//-----------------------------------------------------------------------------
double HVE2DReferenceToVector::CalculateAngularAcceleration(const HGF2DLocation& pi_rPoint,
                                                                            HVE2DVector::ArbitraryDirection pi_Direction) const
    {
    // The point must be located on reference to vector
    HPRECONDITION(IsPointOn(pi_rPoint));

    return (m_pSource->CalculateAngularAcceleration(pi_rPoint, pi_Direction));
    }




//-----------------------------------------------------------------------------
// CalculateClosestPoint
// Calculates and returns the closest point on vector from given point
//-----------------------------------------------------------------------------
HGF2DLocation HVE2DReferenceToVector::CalculateClosestPoint(const HGF2DLocation& pi_rLocation) const
    {

    HGF2DLocation Point = m_pSource->CalculateClosestPoint(pi_rLocation);

    Point.ChangeCoordSys(GetCoordSys());

    return(Point);
    }





//-----------------------------------------------------------------------------
// Crosses
// Indicates if the two reference to vector cross each other
//-----------------------------------------------------------------------------
bool HVE2DReferenceToVector::Crosses(const HVE2DVector& pi_rVector) const
    {
    bool   IsCrossing = false;

    return (IsCrossing);
    }



//-----------------------------------------------------------------------------
// IsPointOn
// Checks if the point is located on the reference to vector
//-----------------------------------------------------------------------------
bool   HVE2DReferenceToVector::IsPointOn(const HGF2DLocation& pi_rTestPoint,
                                          HVE2DVector::ExtremityProcessing pi_ExtremityProcessing) const
    {
    HGF2DLocation  Point(pi_rTestPoint, GetCoordSys());

    Point.SetCoordSys(m_pSource->GetCoordSys());

    return(m_pSource->IsPointOnSCS(Point));
    }

//-----------------------------------------------------------------------------
// IsPointOnSCS
// Checks if the point is located on the reference to vector
//-----------------------------------------------------------------------------
bool   HVE2DReferenceToVector::IsPointOnSCS(const HGF2DLocation& pi_rTestPoint,
                                             HVE2DVector::ExtremityProcessing pi_ExtremityProcessing) const
    {
    // The point must have the same coordinate system as vector
    HPRECONDITION(GetCoordSys() == pi_rTestPoint.GetCoordSys());

    // Make a duplicate
    HGF2DLocation  Point(pi_rTestPoint);

    Point.SetCoordSys(m_pSource->GetCoordSys());

    return(m_pSource->IsPointOnSCS(Point));
    }


//-----------------------------------------------------------------------------
// ObtainContiguousnessPoints
// Finds contiguousness points with vector
//-----------------------------------------------------------------------------
int HVE2DReferenceToVector::ObtainContiguousnessPoints(const HVE2DVector& pi_rVector,
                                                       HGF2DLocationCollection* po_pContiguousnessPoints) const
    {
    HPRECONDITION(po_pContiguousnessPoints);

    // Allocate a copy of vector in self coordinate system
    HVE2DVector* pVector = (HVE2DVector*)pi_rVector.AllocateCopyInCoordSys(GetCoordSys());

    // Move to source coordinate system
    pVector->SetCoordSys(m_pSource->GetCoordSys());

    HGF2DLocationCollection  ContiguousPoints;
    // Obtain contiguousness points
    int NumPoints = m_pSource->ObtainContiguousnessPoints(*pVector, &ContiguousPoints);

    // If any points were found
    if (NumPoints > 0)
        {
        // For every point
        HGF2DLocationCollection::iterator ItrPoints;
        for(ItrPoints = ContiguousPoints.begin() ; ItrPoints != ContiguousPoints.end() ; ++ItrPoints)
            {
            // Create a copy of point
            HGF2DLocation TempPoint(*ItrPoints);

            // Move to reference system
            TempPoint.SetCoordSys(GetCoordSys());

            // Add to contiguousness points
            po_pContiguousnessPoints->push_back(TempPoint);
            }

        }

    return(NumPoints);
    }



//-----------------------------------------------------------------------------
// ObtainContiguousnessPointsAt
// Finds contiguousness points with vector
//-----------------------------------------------------------------------------
void HVE2DReferenceToVector::ObtainContiguousnessPointsAt(const HVE2DVector& pi_rVector,
                                                          const HGF2DLocation& pi_rPoint,
                                                          HGF2DLocation* po_pFirstContiguousnessPoint,
                                                          HGF2DLocation* po_pSecondContiguousnessPoint) const
    {
    // The vectors must be contiguous at specified point
    HPRECONDITION(AreContiguousAt(pi_rVector, pi_rPoint));

    // Create back up of point
    HGF2DLocation Point(pi_rPoint);

    // Move it to source system
    Point.SetCoordSys(m_pSource->GetCoordSys());

    // Allocate a copy of vector in self coordinate system
    HVE2DVector* pVector = (HVE2DVector*)pi_rVector.AllocateCopyInCoordSys(GetCoordSys());

    // Move to source coordinate system
    pVector->SetCoordSys(m_pSource->GetCoordSys());

    // Obtain contiguousness points
    m_pSource->ObtainContiguousnessPointsAt(*pVector,
                                            Point,
                                            po_pFirstContiguousnessPoint,
                                            po_pSecondContiguousnessPoint);

    // Move points to current coordinate system
    po_pFirstContiguousnessPoint->SetCoordSys(GetCoordSys());
    po_pSecondContiguousnessPoint->SetCoordSys(GetCoordSys());
    }






//-----------------------------------------------------------------------------
// Intersect
// Finds intersection points with vector
//-----------------------------------------------------------------------------
int HVE2DReferenceToVector::Intersect(const HVE2DVector& pi_rVector,
                                      HGF2DLocationCollection* po_pCrossPoints) const
    {
    int32_t NumberOfNewPoints = 0;

    // Allocate a copy of vector in self coordinate system
    HVE2DVector* pVector = (HVE2DVector*)pi_rVector.AllocateCopyInCoordSys(GetCoordSys());

    // Move to source coordinate system
    pVector->SetCoordSys(m_pSource->GetCoordSys());

    // Declare container for new intersect points
    HGF2DLocationCollection  CrossPoints;

    // Obtain cross points
    int NumPoints = m_pSource->Intersect(*pVector,
                                         &CrossPoints);

    // If any points were found
    if (NumPoints > 0)
        {
        // For every point
        HGF2DLocationCollection::iterator ItrPoints;
        for(ItrPoints = CrossPoints.begin() ; ItrPoints != CrossPoints.end() ; ++ItrPoints)
            {
            // Create a copy of point
            HGF2DLocation TempPoint(*ItrPoints);

            // Move to reference system
            TempPoint.SetCoordSys(GetCoordSys());

            // Add to cross points
            po_pCrossPoints->push_back(TempPoint);
            }

        }

    return (NumPoints);
    }




//-----------------------------------------------------------------------------
// GetExtent
// Returns the extent of the reference to vector
//-----------------------------------------------------------------------------
HGF2DExtent HVE2DReferenceToVector::GetExtent() const
    {
    if (!m_ExtentUpToDate)
        {
        // Cast out of const
        HGF2DExtent*   pTheExtent = const_cast<HGF2DExtent*>(&m_Extent);

        // Force extent to empty and set appropriate coordinate system
        *pTheExtent = HGF2DExtent(GetCoordSys());

        *(const_cast<bool*>(&m_ExtentUpToDate)) = true;
        }

    return (m_Extent);
    }


//-----------------------------------------------------------------------------
// AllocateCopyInCoordSys
// Returns a dynamically allocated copy of the reference to vector in a different
// coordinate system
//-----------------------------------------------------------------------------
HVE2DVector* HVE2DReferenceToVector::AllocateCopyInCoordSys(const HFCPtr<HGF2DCoordSys>& pi_rpCoordSys) const
    {
    return(new HVE2DReferenceToVector(m_pSource, pi_rpCoordSys));
    }

//-----------------------------------------------------------------------------
// AreAdjacent
// Indicates if the vector is adjacent to the given
//-----------------------------------------------------------------------------
bool HVE2DReferenceToVector::AreAdjacent(const HVE2DVector& pi_rVector) const
    {
    // Allocate a copy of vector in self coordinate system
    HAutoPtr<HVE2DVector> pVector((HVE2DVector*)pi_rVector.AllocateCopyInCoordSys(GetCoordSys()));

    // Move to source coordinate system
    pVector->SetCoordSys(m_pSource->GetCoordSys());

    return(m_pSource->AreAdjacent(*pVector));
    }


//-----------------------------------------------------------------------------
// AreContiguous
// Indicates if the vector is contiguous to the given
//-----------------------------------------------------------------------------
bool HVE2DReferenceToVector::AreContiguous(const HVE2DVector& pi_rVector) const
    {
    // Allocate a copy of vector in self coordinate system
    HAutoPtr<HVE2DVector> pVector((HVE2DVector*)pi_rVector.AllocateCopyInCoordSys(GetCoordSys()));

    // Move to source coordinate system
    pVector->SetCoordSys(m_pSource->GetCoordSys());

    return(m_pSource->AreContiguous(*pVector));
    }


//-----------------------------------------------------------------------------
// AreContiguousAt
// Indicates if the vector is contiguous at the given point
//-----------------------------------------------------------------------------
bool HVE2DReferenceToVector::AreContiguousAt(const HVE2DVector& pi_rVector,
                                              const HGF2DLocation& pi_rPoint) const
    {

    // The given point must be located on both vectors
    HPRECONDITION(IsPointOn(pi_rPoint) && pi_rVector.IsPointOn(pi_rPoint));

    // Duplicate point
    HGF2DLocation Point(pi_rPoint, GetCoordSys());

    // Move to source system
    Point.SetCoordSys(m_pSource->GetCoordSys());

    // Allocate a copy of vector in self coordinate system
    HAutoPtr<HVE2DVector> pVector((HVE2DVector*)pi_rVector.AllocateCopyInCoordSys(GetCoordSys()));

    // Move to source coordinate system
    pVector->SetCoordSys(m_pSource->GetCoordSys());

    return(m_pSource->AreContiguousAt(*pVector, Point));
    }





//-----------------------------------------------------------------------------
// PrintState
// This method dumps the content of the object in the given output stream
// in text format
//-----------------------------------------------------------------------------
void HVE2DReferenceToVector::PrintState(ostream& po_rOutput) const
    {
#ifdef __HMR_PRINTSTATE
    po_rOutput << "Object is a HVE2DReferenceToVector" << endl;
    HDUMP0("Object is a HVE2DReferenceToVector\n");
    HVE2DVector::PrintState(po_rOutput);
    po_rOutput << "Begin component listing" << endl;
    HDUMP0("Begin component listing\n");

    m_pSource->PrintState(po_rOutput);

    HDUMP0("END OF COMPONENT LISTING\n");

#endif
    }




