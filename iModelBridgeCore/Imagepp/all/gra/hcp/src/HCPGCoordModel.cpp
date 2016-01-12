//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hcp/src/HCPGCoordModel.cpp $
//:>
//:>  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#include <ImagePPInternal/hstdcpp.h>                // must be first for PreCompiledHeader Option


#include <Imagepp/all/h/HCPGCoordModel.h>
#include <Imagepp/all/h/HCPGCoordUtility.h>
#include <Imagepp/all/h/HGF2DLiteExtent.h>
#include <Imagepp/all/h/HGF2DDisplacement.h>
#include <Imagepp/all/h/HGF2DIdentity.h>
#include <ImagePP/all/h/HFCException.h>
#include <ImagePP/all/h/HCPGCoordLatLongModel.h>
#include <ImagePP/all/h/HVE2DShape.h>
#include <ImagePP/all/h/HVE2DPolygonOfSegments.h>
#include <ImagePP/all/h/HCPGCoordUtility.h>

#include <Geom/GeoPoint.h>

//-----------------------------------------------------------------------------
// Default Constructor
//-----------------------------------------------------------------------------
//HCPGCoordModel::HCPGCoordModel ()
//    :HGF2DTransfoModel ()
//    {
//    Prepare ();
//
//    HINVARIANTS;
//    }

/** -----------------------------------------------------------------------------
    Creates a GCoord reprojection model based on two provided
    GCoord projections. The units of the model are based on the units
    of the projections given.

    @param pi_SourceGEOCS A Valid GCoord projection representing the
                          projection of the direct channel of the model.

    @param pi_DestinationGEOCS A Valid GCoord projection representing the
                               projection of the inversechannel of the model.


    -----------------------------------------------------------------------------
*/
HCPGCoordModel::HCPGCoordModel(GeoCoordinates::BaseGCSCR pi_SourceGEOCS, GeoCoordinates::BaseGCSCR pi_DestinationGEOCS)
    :HGF2DTransfoModel (),
     m_pSrcGCS (&pi_SourceGEOCS),
     m_pDestGCS (&pi_DestinationGEOCS),
     m_domainComputed(false),
     m_domainDirect(NULL),
     m_domainInverse(NULL)
    {
    // The two projections must be valid
    HPRECONDITION (m_pSrcGCS->IsValid ());
    HPRECONDITION (m_pDestGCS->IsValid ());
    }

//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another object.
//-----------------------------------------------------------------------------
HCPGCoordModel::HCPGCoordModel (const HCPGCoordModel& pi_rObj)
    :HGF2DTransfoModel (pi_rObj),
     m_pSrcGCS(pi_rObj.m_pSrcGCS),
     m_pDestGCS(pi_rObj.m_pDestGCS),
     m_domainComputed(false),
     m_domainDirect(NULL),
     m_domainInverse(NULL)
    {
    Copy (pi_rObj);

    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HCPGCoordModel::~HCPGCoordModel ()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another object.
//-----------------------------------------------------------------------------
HCPGCoordModel& HCPGCoordModel::operator=(const HCPGCoordModel& pi_rObj)
    {
    HINVARIANTS;

    // Check that object to copy is not self
    if (this != &pi_rObj)
        {
        // Call ancestor operator=
        HGF2DTransfoModel::operator=(pi_rObj);
        Copy (pi_rObj);
        }

    // Return reference to self
    return (*this);
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HCPGCoordModel::ConvertDirect
(
    double* pio_pXInOut,
    double* pio_pYInOut
) const
    {
    HINVARIANTS;
    HPRECONDITION (pio_pXInOut != NULL);
    HPRECONDITION (pio_pYInOut != NULL);

    DPoint3d inCartesian;
    inCartesian.x = *pio_pXInOut;
    inCartesian.y = *pio_pYInOut;
    inCartesian.z = 0.0;

    StatusInt   stat1;
    StatusInt   stat2;
    StatusInt   stat3;

    GeoPoint inLatLong;
    stat1 = m_pSrcGCS->LatLongFromCartesian (inLatLong, inCartesian);

    GeoPoint outLatLong;
    stat2 = m_pSrcGCS->LatLongFromLatLong(outLatLong, inLatLong, *m_pDestGCS);

    DPoint3d outCartesian;
    stat3 = m_pDestGCS->CartesianFromLatLong(outCartesian, outLatLong);

    StatusInt status = SUCCESS;

    // Status returns hardest error found in the three error statuses
    // The hardest error is the first one encountered that is not a warning (value 1 [cs_CNVRT_USFL])
    if (SUCCESS != stat1)
        status = stat1;
    if ((SUCCESS != stat2) && ((SUCCESS == status) || (1 == status))) // If stat2 has error and status not already hard error
        {
        if (0 > stat2) // If stat2 is negative ... this is the one ...
            status = stat2;
        else  // Both are positive (status may be SUCCESS) we use the highest value which is either warning or error
            status = (stat2 > status ? stat2 : status);
        }
    if ((SUCCESS != stat3) && ((SUCCESS == status) || (1 == status))) // If stat3 has error and status not already hard error
        {
        if (0 > stat3) // If stat3 is negative ... this is the one ...
            status = stat3;
        else  // Both are positive (status may be SUCCESS) we use the highest value
            status = (stat3 > status ? stat3 : status);
        }

    *pio_pXInOut = outCartesian.x;
    *pio_pYInOut = outCartesian.y;

    return status;
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HCPGCoordModel::ConvertDirect
(
    double  pi_XIn,
    double  pi_YIn,
    double* po_pXOut,
    double* po_pYOut
) const
    {
    HINVARIANTS;
    HPRECONDITION (po_pXOut != NULL);
    HPRECONDITION (po_pYOut != NULL);

    *po_pXOut = pi_XIn;
    *po_pYOut = pi_YIn;

    return ConvertDirect (po_pXOut, po_pYOut);
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HCPGCoordModel::ConvertDirect
(
    double    pi_YIn,
    double    pi_XInStart,
    size_t    pi_NumLoc,
    double    pi_XInStep,
    double*   po_pXOut,
    double*   po_pYOut
) const
    {
    // Make sure recipient arrays are provided
    HPRECONDITION (po_pXOut != NULL);
    HPRECONDITION (po_pYOut != NULL);

    StatusInt status = SUCCESS;

    double    X;
    uint32_t  Index;
    double* pCurrentX = po_pXOut;
    double* pCurrentY = po_pYOut;

    for (Index = 0, X = pi_XInStart;
         Index < pi_NumLoc ; ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
        {
        StatusInt tempStatus = ConvertDirect (X, pi_YIn, pCurrentX, pCurrentY);

        if ((SUCCESS != tempStatus) && (SUCCESS == status))
            status = tempStatus;
        }

    return status;
    }


//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HCPGCoordModel::ConvertDirect
(
 size_t pi_NumLoc, 
 double* pio_aXInOut,
 double* pio_aYInOut
) const
    {
    // Make sure recipient arrays are provided
    HPRECONDITION (pio_aXInOut!=NULL);
    HPRECONDITION (pio_aYInOut!=NULL);

    StatusInt status = SUCCESS;

    for(uint32_t i = 0; i < pi_NumLoc; i++)
        {
        StatusInt tempStatus = ConvertDirect (pio_aXInOut[i], pio_aYInOut[i], pio_aXInOut + i, pio_aYInOut + i);

        // Return the first non-SUCCESS error yet continue the process as possible.
        if ((SUCCESS != tempStatus) && (SUCCESS == status))
            status = tempStatus;
        }

    return status;
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HCPGCoordModel::ConvertInverse
(
    double* pio_pXInOut,
    double* pio_pYInOut
) const
    {
    HINVARIANTS;
    HPRECONDITION (pio_pXInOut != NULL);
    HPRECONDITION (pio_pYInOut != NULL);

    DPoint3d inCartesian;
    inCartesian.x = *pio_pXInOut;
    inCartesian.y = *pio_pYInOut;
    inCartesian.z = 0.0;

    StatusInt   stat1;
    StatusInt   stat2;
    StatusInt   stat3;

    GeoPoint inLatLong;
    stat1 = m_pDestGCS->LatLongFromCartesian (inLatLong, inCartesian);

    GeoPoint outLatLong;
    stat2 = m_pDestGCS->LatLongFromLatLong(outLatLong, inLatLong, *m_pSrcGCS);

    DPoint3d outCartesian;
    stat3 = m_pSrcGCS->CartesianFromLatLong(outCartesian, outLatLong);

    StatusInt status = SUCCESS;

    // Status returns hardest error found in the three error statuses
    // The hardest error is the first one encountered that is not a warning (value 1 [cs_CNVRT_USFL])
    if (SUCCESS != stat1)
        status = stat1;
    if ((SUCCESS != stat2) && ((SUCCESS == status) || (1 == status))) // If stat2 has error and status not already hard error
        {
        if (0 > stat2) // If stat2 is negative ... this is the one ...
            status = stat2;
        else  // Both are positive (status may be SUCCESS) we use the highest value which is either warning or error
            status = (stat2 > status ? stat2 : status);
        }
    if ((SUCCESS != stat3) && ((SUCCESS == status) || (1 == status))) // If stat3 has error and status not already hard error
        {
        if (0 > stat3) // If stat3 is negative ... this is the one ...
            status = stat3;
        else  // Both are positive (status may be SUCCESS) we use the highest value
            status = (stat3 > status ? stat3 : status);
        }

    *pio_pXInOut = outCartesian.x;
    *pio_pYInOut = outCartesian.y;

    return status;
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HCPGCoordModel::ConvertInverse
(
    double pi_XIn,
    double pi_YIn,
    double* po_pXOut,
    double* po_pYOut
) const
    {
    HINVARIANTS;
    HPRECONDITION (po_pXOut != NULL);
    HPRECONDITION (po_pYOut != NULL);

    *po_pXOut = pi_XIn;
    *po_pYOut = pi_YIn;

    return ConvertInverse (po_pXOut, po_pYOut);
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HCPGCoordModel::ConvertInverse
(
    double    pi_YIn,
    double    pi_XInStart,
    size_t    pi_NumLoc,
    double    pi_XInStep,
    double*   po_pXOut,
    double*   po_pYOut
) const
    {
    // Make sure recipient arrays are provided
    HPRECONDITION (po_pXOut != NULL);
    HPRECONDITION (po_pYOut != NULL);

    StatusInt status = SUCCESS;
    double   X;
    uint32_t Index;
    double* pCurrentX = po_pXOut;
    double* pCurrentY = po_pYOut;

    for (Index = 0, X = pi_XInStart;
         Index < pi_NumLoc ; ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
        {
        StatusInt tempStatus = ConvertInverse (X, pi_YIn, pCurrentX, pCurrentY);

        if ((SUCCESS != tempStatus) && (SUCCESS == status))
            status = tempStatus;
        }

    return status;
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HCPGCoordModel::ConvertInverse
(
 size_t pi_NumLoc, 
 double* pio_aXInOut,
 double* pio_aYInOut
) const
    {
    // Make sure recipient arrays are provided
    HPRECONDITION (pio_aXInOut!=NULL);
    HPRECONDITION (pio_aYInOut!=NULL);

    StatusInt status = SUCCESS;


    for(uint32_t i = 0; i < pi_NumLoc; i++)
        {
        StatusInt tempStatus = ConvertInverse (pio_aXInOut[i], pio_aYInOut[i], pio_aXInOut + i, pio_aYInOut + i);

        // Return the first non-SUCCESS error yet continue the process as possible.
        if ((SUCCESS != tempStatus) && (SUCCESS == status))
            status = tempStatus;
        }

    return status;
    }

//-----------------------------------------------------------------------------
// PreservesLinearity
// Indicate if the transformation model preserves linearity
//-----------------------------------------------------------------------------
bool   HCPGCoordModel::PreservesLinearity () const
    {
    HINVARIANTS;

    return (false);
    }

//-----------------------------------------------------------------------------
// PreservesParallelism
// Indicate if the transformation model preserves parallelism
//-----------------------------------------------------------------------------
bool   HCPGCoordModel::PreservesParallelism () const
    {
    HINVARIANTS;

    return (false);
    }

//-----------------------------------------------------------------------------
// PreservesShape
// Indicate if the transformation model preserves the shape
//-----------------------------------------------------------------------------
bool   HCPGCoordModel::PreservesShape () const
    {
    HINVARIANTS;

    return (false);
    }

//-----------------------------------------------------------------------------
// PreservesDirection
// Indicate if the transformation model preserves directions
//-----------------------------------------------------------------------------
bool   HCPGCoordModel::PreservesDirection () const
    {
    HINVARIANTS;

    return (false);
    }

//-----------------------------------------------------------------------------
// CanBeRepresentedByAMatrix
// Indicates if the model can be represented by a transformation matrix
//-----------------------------------------------------------------------------
bool HCPGCoordModel::CanBeRepresentedByAMatrix () const
    {
    HINVARIANTS;

    return (false);
    }

//-----------------------------------------------------------------------------
//  IsIdentity
//  Returns true if the model contains no transformation
//-----------------------------------------------------------------------------
bool HCPGCoordModel::IsIdentity () const
    {
    HINVARIANTS;

    return (false);
    }

//-----------------------------------------------------------------------------
//  IsStetchable
//  Returns true if the model contains only scaling and translation
//-----------------------------------------------------------------------------
bool HCPGCoordModel::IsStretchable (double pi_AngleTolerance) const
    {
    HINVARIANTS;

    return (false);
    }

//-----------------------------------------------------------------------------
//  GetStetchParams
//  Returns the stretch parameters
//-----------------------------------------------------------------------------
void HCPGCoordModel::GetStretchParams
(
    double* po_pScaleFactorX,
    double* po_pScaleFactorY,
    HGF2DDisplacement* po_pDisplacement
) const
    {
    HINVARIANTS;

    HPRECONDITION (po_pScaleFactorX != NULL);
    HPRECONDITION (po_pScaleFactorY != NULL);
    HPRECONDITION (po_pDisplacement != NULL);

    // Do not use!
    HASSERT (0);

    HFCPtr<HGF2DTransfoModel> pModel;

    pModel->GetStretchParams (po_pScaleFactorX, po_pScaleFactorY, po_pDisplacement);
    }

//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the projective by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3> HCPGCoordModel::GetMatrix () const
    {
    HINVARIANTS;

    // Should not be called
    HASSERT (0);

    HFCMatrix<3, 3> Matrix;

    return (Matrix);
    }

//-----------------------------------------------------------------------------
// Reverse
// This method reverses the transformation model
//-----------------------------------------------------------------------------
void    HCPGCoordModel::Reverse ()
    {
    HINVARIANTS;

    std::swap(m_pSrcGCS, m_pDestGCS);
    
    // Invoque reversing of ancester
    // This call will in turn invoque Prepare()
    HGF2DTransfoModel::Reverse ();
    }


//-----------------------------------------------------------------------------
// ComposeInverseWithDirectOf
// Composes a new transformation model as a combination of self and given
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HCPGCoordModel::ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const
    {
    HINVARIANTS;

    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    // Check if it is identity
    if (pi_rModel.IsIdentity ())
        {
        // Model is identity ... return copy of self
        pResultModel = new HCPGCoordModel (*this);

        }
    else
        {
        // Model is not known ... ask other
        pResultModel = CallComposeOf (pi_rModel);
        }

    return (pResultModel);
    }

//-----------------------------------------------------------------------------
// Clone
// This method allocates a copy of self. The caller is responsible for
// the deletion of this object.
//-----------------------------------------------------------------------------
HGF2DTransfoModel* HCPGCoordModel::Clone () const
    {
    HINVARIANTS;

    // Allocate object as copy and return
    return (new HCPGCoordModel (*this));
    }

//-----------------------------------------------------------------------------
// ComposeYourself
// PRIVATE
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HCPGCoordModel::ComposeYourself (const HGF2DTransfoModel& pi_rModel) const
    {
    HINVARIANTS;

    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    // Type is not known ... build a complex
    // To do this we call the ancester ComposeYourself
    pResultModel = HGF2DTransfoModel::ComposeYourself (pi_rModel);

    return (pResultModel);
    }

//-----------------------------------------------------------------------------
//  Prepare
//  This methods prepares the conversion parameters from the basic
//  model attribute
//-----------------------------------------------------------------------------
void HCPGCoordModel::Prepare ()
    {
    // Obtain conversion ratio for direct X to inverse X units
    }

//-----------------------------------------------------------------------------
//  Copy
//  Copy method
//-----------------------------------------------------------------------------
void HCPGCoordModel::Copy (const HCPGCoordModel& pi_rObj)
    {
    // Copy master data
    m_pSrcGCS  = pi_rObj.m_pSrcGCS;
    m_pDestGCS = pi_rObj.m_pDestGCS;
    m_domainComputed   = false;
    m_domainDirect     = NULL;
    m_domainInverse    = NULL;

    }

//-----------------------------------------------------------------------------
// CreateSimplifiedModel
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HCPGCoordModel::CreateSimplifiedModel () const
    {
    HINVARIANTS;

    if (m_pSrcGCS->IsEquivalent(*m_pDestGCS))
        return new HGF2DIdentity();

    // If we get here, no simplification is possible.
    return 0;
    }

//-----------------------------------------------------------------------------
// GetSourceGEOCS
// Returns the source projection
//-----------------------------------------------------------------------------
GeoCoordinates::BaseGCSCR HCPGCoordModel::GetSourceGEOCS() const
    {
    return *m_pSrcGCS;
    }

//-----------------------------------------------------------------------------
// GetDestinationGEOCS
// Returns the destination projection
//-----------------------------------------------------------------------------
GeoCoordinates::BaseGCSCR HCPGCoordModel::GetDestinationGEOCS() const
    {
    return *m_pDestGCS;
    }

#ifdef HVERIFYCONTRACT
void               HCPGCoordModel::ValidateInvariants() const
    {
    HASSERT(m_pSrcGCS->IsValid());
    HASSERT(m_pDestGCS->IsValid());
    }
#endif




//-----------------------------------------------------------------------------
// PRIVATE
// ComputeDomain - Copmputes the domains applicable to the transformation model
//-----------------------------------------------------------------------------
StatusInt HCPGCoordModel::ComputeDomain () const
    {
    StatusInt status = SUCCESS;

    if (!m_domainComputed)
        {
        // Domain not computed ... we first obtain the geographic domain from both GCS
        bvector<GeoPoint> sourceGeoDomain;
        bvector<GeoPoint> destinationGeoDomain;

        HGF2DCoordCollection<double> sourceGeoDomain2;
        HGF2DCoordCollection<double> destinationGeoDomain2;

        m_pSrcGCS->GetMathematicalDomain(sourceGeoDomain);
        m_pDestGCS->GetMathematicalDomain(destinationGeoDomain);

        // Convert geo points to Image++ points
        for (int idx = 0 ; idx < sourceGeoDomain.size() ; idx++)
            sourceGeoDomain2.push_back(HGF2DCoord<double>(sourceGeoDomain[idx].longitude, sourceGeoDomain[idx].latitude));

        for (int idx = 0 ; idx < destinationGeoDomain.size() ; idx++)
            destinationGeoDomain2.push_back(HGF2DCoord<double>(destinationGeoDomain[idx].longitude, destinationGeoDomain[idx].latitude));

        
        // Create the three coordinate systems required for transformation
        HFCPtr<HGF2DCoordSys> latLongCoordinateSystem = new HGF2DCoordSys();
        HFCPtr<HGF2DTransfoModel> directLatLongTransfoModel = new HCPGCoordLatLongModel (*m_pSrcGCS);
        HFCPtr<HGF2DTransfoModel> inverseLatLongTransfoModel = new HCPGCoordLatLongModel (*m_pDestGCS);
        HFCPtr<HGF2DCoordSys> directCoordinateSystem = new HGF2DCoordSys(*directLatLongTransfoModel, latLongCoordinateSystem);
        HFCPtr<HGF2DCoordSys> inverseCoordinateSystem = new HGF2DCoordSys(*inverseLatLongTransfoModel, latLongCoordinateSystem);

        // Create shapes from these

        HFCPtr<HVE2DShape> sourceDomainShape = new HVE2DPolygonOfSegments(HGF2DPolygonOfSegments(sourceGeoDomain2), latLongCoordinateSystem);
        HFCPtr<HVE2DShape> destinationDomainShape = new HVE2DPolygonOfSegments(HGF2DPolygonOfSegments(destinationGeoDomain2), latLongCoordinateSystem);

        HFCPtr<HVE2DShape> resultDomainShape = sourceDomainShape->IntersectShape (*destinationDomainShape);

        // Obtain shapes in direct of inverse coordinate systems, drop and copy points
        HFCPtr<HVE2DShape> tempShapeDirect = static_cast<HVE2DShape*>(resultDomainShape->AllocateCopyInCoordSys (directCoordinateSystem));
        m_domainDirect = tempShapeDirect->GetLightShape();
        HFCPtr<HVE2DShape> tempShapeInverse = static_cast<HVE2DShape*>(resultDomainShape->AllocateCopyInCoordSys (inverseCoordinateSystem));
        m_domainInverse = tempShapeInverse->GetLightShape();

        m_domainComputed = true;
        }

    return status;
    }


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool HCPGCoordModel::HasDomain() const
    {
    return true;
    }


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
HFCPtr<HGF2DShape> HCPGCoordModel::GetDirectDomain() const
    {
    ComputeDomain();

    return m_domainDirect;
    }

//-----------------------------------------------------------------------------
// CreateSimplifiedModel
//-----------------------------------------------------------------------------
HFCPtr<HGF2DShape> HCPGCoordModel::GetInverseDomain () const
    {
    StatusInt status = SUCCESS;

    status = ComputeDomain();

    return m_domainInverse;
    }
