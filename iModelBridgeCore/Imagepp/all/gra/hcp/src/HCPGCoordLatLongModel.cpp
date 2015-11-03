/*--------------------------------------------------------------------------------------+
|
|     $Source: all/gra/hcp/src/HCPGCoordLatLongModel.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <ImagePPInternal/hstdcpp.h>                // must be first for PreCompiledHeader Option


#include <Imagepp/all/h/HCPGCoordLatLongModel.h>
#include <ImagePP/all/h/HVE2DShape.h>
#include <ImagePP/all/h/HVE2DPolygonOfSegments.h>
#include <ImagePP/all/h/HCPGCoordUtility.h>



/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
HCPGCoordLatLongModel::HCPGCoordLatLongModel(GeoCoordinates::BaseGCSCR pi_GEOCS)
 :HGF2DTransfoModel (),
 m_pBaseGCS (&pi_GEOCS),
 m_reversed(false)
    {
    // The two projections must be valid
    HPRECONDITION (m_pBaseGCS->IsValid ());
    }


//-----------------------------------------------------------------------------
// Copy constructor.  It duplicates another object.
//-----------------------------------------------------------------------------
HCPGCoordLatLongModel::HCPGCoordLatLongModel (const HCPGCoordLatLongModel& pi_rObj)
:HGF2DTransfoModel (pi_rObj),
 m_pBaseGCS(pi_rObj.m_pBaseGCS)
    {
    Copy (pi_rObj);

    HINVARIANTS;
    }

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
HCPGCoordLatLongModel::~HCPGCoordLatLongModel ()
    {
    }

//-----------------------------------------------------------------------------
// operator=
// Assignment operator.  It duplicates another object.
//-----------------------------------------------------------------------------
HCPGCoordLatLongModel& HCPGCoordLatLongModel::operator=(const HCPGCoordLatLongModel& pi_rObj)
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
StatusInt HCPGCoordLatLongModel::ConvertDirect
(
 double* pio_pXInOut,
 double* pio_pYInOut
) const
    {
    HINVARIANTS;

    // Variables must be provided
    HPRECONDITION (pio_pXInOut!=NULL);
    HPRECONDITION (pio_pYInOut!=NULL);


    double X = *pio_pXInOut;
    double Y = *pio_pYInOut;

    return ConvertDirectReversible (X, Y, pio_pXInOut, pio_pYInOut, m_reversed);
    }


//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HCPGCoordLatLongModel::ConvertDirect
(
 double  pi_XIn,
 double  pi_YIn,
 double* po_pXOut,
 double* po_pYOut
) const
    {
    HINVARIANTS;

    HPRECONDITION (po_pXOut!=NULL);
    HPRECONDITION (po_pYOut!=NULL);

    return ConvertDirectReversible (pi_XIn, pi_YIn, po_pXOut, po_pYOut, m_reversed);
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HCPGCoordLatLongModel::ConvertDirect
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
    HPRECONDITION (po_pXOut!=NULL);
    HPRECONDITION (po_pYOut!=NULL);

    StatusInt status = SUCCESS;

    double  X;
    size_t   Index;
    double* pCurrentX = po_pXOut;
    double* pCurrentY = po_pYOut;

    for (Index = 0, X = pi_XInStart;
        Index < pi_NumLoc ; ++Index, X+=pi_XInStep, ++pCurrentX, ++pCurrentY)
        {
        StatusInt tempStatus = ConvertDirect (X, pi_YIn, pCurrentX, pCurrentY);

        // Return the first non-SUCCESS error yet continue the process as possible.
        if ((SUCCESS != tempStatus) && (SUCCESS == status))
            status = tempStatus;
        }

    return status;
    }

//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HCPGCoordLatLongModel::ConvertDirect
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


    for(uint32_t i = 0; i < pi_NumLoc; ++i)
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
StatusInt HCPGCoordLatLongModel::ConvertInverse
(
 double* pio_pXInOut,
 double* pio_pYInOut
) const
    {
    HINVARIANTS;

    HPRECONDITION (pio_pXInOut!=NULL);
    HPRECONDITION (pio_pYInOut!=NULL);

    double X = *pio_pXInOut;
    double Y = *pio_pYInOut;
    return ConvertDirectReversible (X, Y, pio_pXInOut, pio_pYInOut, !m_reversed);
    }


//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HCPGCoordLatLongModel::ConvertInverse
(
 double pi_XIn,
 double pi_YIn,
 double* po_pXOut,
 double* po_pYOut
) const
    {
    HINVARIANTS;

    HPRECONDITION (po_pXOut!=NULL);
    HPRECONDITION (po_pYOut!=NULL);

    return ConvertDirectReversible (pi_XIn, pi_YIn, po_pXOut, po_pYOut, !m_reversed);
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
StatusInt HCPGCoordLatLongModel::ConvertInverse
(
 double    pi_YIn,
 double    pi_XInStart,
 size_t     pi_NumLoc,
 double    pi_XInStep,
 double*   po_pXOut,
 double*   po_pYOut
) const
    {
    // Make sure recipient arrays are provided
    HPRECONDITION (po_pXOut!=NULL);
    HPRECONDITION (po_pYOut!=NULL);

    StatusInt status = SUCCESS;

    double  X;
    size_t   Index;
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
StatusInt HCPGCoordLatLongModel::ConvertInverse
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


    for(uint32_t i = 0; i < pi_NumLoc; ++i)
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
bool   HCPGCoordLatLongModel::PreservesLinearity () const
    {
    HINVARIANTS;

    return (false);
    }

//-----------------------------------------------------------------------------
// PreservesParallelism
// Indicate if the transformation model preserves parallelism
//-----------------------------------------------------------------------------
bool   HCPGCoordLatLongModel::PreservesParallelism () const
    {
    HINVARIANTS;

    return (false);
    }

//-----------------------------------------------------------------------------
// PreservesShape
// Indicate if the transformation model preserves the shape
//-----------------------------------------------------------------------------
bool   HCPGCoordLatLongModel::PreservesShape () const
    {
    HINVARIANTS;

    return (false);
    }

//-----------------------------------------------------------------------------
// PreservesDirection
// Indicate if the transformation model preserves directions
//-----------------------------------------------------------------------------
bool   HCPGCoordLatLongModel::PreservesDirection () const
    {
    HINVARIANTS;

    return (false);
    }


//-----------------------------------------------------------------------------
// CanBeRepresentedByAMatrix
// Indicates if the model can be represented by a transformation matrix
//-----------------------------------------------------------------------------
bool HCPGCoordLatLongModel::CanBeRepresentedByAMatrix () const
    {
    HINVARIANTS;

    return (false);
    }


//-----------------------------------------------------------------------------
//  IsIdentity
//  Returns true if the model contains no transformation
//-----------------------------------------------------------------------------
bool HCPGCoordLatLongModel::IsIdentity () const
    {
    HINVARIANTS;

    return (false);
    }

//-----------------------------------------------------------------------------
//  IsStetchable
//  Returns true if the model contains only scaling and translation
//-----------------------------------------------------------------------------
bool HCPGCoordLatLongModel::IsStretchable (double pi_AngleTolerance) const
    {
    HINVARIANTS;

    return (false);
    }


//-----------------------------------------------------------------------------
//  GetStetchParams
//  Returns the stretch parameters
//-----------------------------------------------------------------------------
void HCPGCoordLatLongModel::GetStretchParams
(
 double* po_pScaleFactorX,
 double* po_pScaleFactorY,
 HGF2DDisplacement* po_pDisplacement
 ) const
    {
    HINVARIANTS;

    HPRECONDITION (po_pScaleFactorX!=NULL);
    HPRECONDITION (po_pScaleFactorY!=NULL);
    HPRECONDITION (po_pDisplacement!=NULL);

    // Do not use!
    BeAssert (0);

    HFCPtr<HGF2DTransfoModel> pModel;

    pModel->GetStretchParams (po_pScaleFactorX, po_pScaleFactorY, po_pDisplacement);
    }



//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the projective by matrix
//-----------------------------------------------------------------------------
HFCMatrix<3, 3> HCPGCoordLatLongModel::GetMatrix () const
    {
    HINVARIANTS;

    // Should not be called
    BeAssert (0);

    HFCMatrix<3, 3> Matrix;

    return (Matrix);
    }

//-----------------------------------------------------------------------------
// Reverse
// This method reverses the transformation model
//-----------------------------------------------------------------------------
void    HCPGCoordLatLongModel::Reverse ()
    {
    HINVARIANTS;

    m_reversed = !m_reversed;

    // Invoque reversing of ancester
    // This call will in turn invoque Prepare()
    HGF2DTransfoModel::Reverse ();
    }



//-----------------------------------------------------------------------------
// ComposeInverseWithDirectOf
// Composes a new transformation model as a combination of self and given
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HCPGCoordLatLongModel::ComposeInverseWithDirectOf (const HGF2DTransfoModel& pi_rModel) const
    {
    HINVARIANTS;

    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    // Check if it is identity
    if (pi_rModel.IsIdentity ())
        {
        // Model is identity ... return copy of self
        pResultModel = new HCPGCoordLatLongModel (*this);
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
HGF2DTransfoModel* HCPGCoordLatLongModel::Clone () const
    {
    HINVARIANTS;

    // Allocate object as copy and return
    return (new HCPGCoordLatLongModel (*this));
    }


//-----------------------------------------------------------------------------
// ComposeYourself
// PRIVATE
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel>  HCPGCoordLatLongModel::ComposeYourself (const HGF2DTransfoModel& pi_rModel) const
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
void HCPGCoordLatLongModel::Prepare ()
    {
    }


//-----------------------------------------------------------------------------
//  Copy
//  Copy method
//-----------------------------------------------------------------------------
void HCPGCoordLatLongModel::Copy (const HCPGCoordLatLongModel& pi_rObj)
    {
    // Copy master data
    m_pBaseGCS   = pi_rObj.m_pBaseGCS;
    m_reversed   = pi_rObj.m_reversed;
    }


//-----------------------------------------------------------------------------
// CreateSimplifiedModel
//-----------------------------------------------------------------------------
HFCPtr<HGF2DTransfoModel> HCPGCoordLatLongModel::CreateSimplifiedModel () const
    {
    HINVARIANTS;

    // If we get here, no simplification is possible.
    return 0;
    }







//-----------------------------------------------------------------------------
// Converter (direct)
//-----------------------------------------------------------------------------
StatusInt HCPGCoordLatLongModel::ConvertDirectReversible
(
 double  pi_XIn,
 double  pi_YIn,
 double* po_pXOut,
 double* po_pYOut,
 bool reverse

 ) const
    {
    HINVARIANTS;

    HPRECONDITION (po_pXOut!=NULL);
    HPRECONDITION (po_pYOut!=NULL);

    StatusInt status = SUCCESS;

    if (!reverse)
        {
        // Transform coordinates to source units
        DPoint3d  cartesianPt = {pi_XIn, pi_YIn, 0.0};

        GeoPoint  geoPt = {0,0,0};
        status = m_pBaseGCS->LatLongFromCartesian (geoPt, cartesianPt);
        
        *po_pXOut = geoPt.longitude;
        *po_pYOut = geoPt.latitude;
        }
    else
        {
        GeoPoint ptLatLong = {pi_XIn, pi_YIn, 0.0};
        DPoint3d cartesianPt = {0.0, 0.0, 0.0};

        status = m_pBaseGCS->CartesianFromLatLong (cartesianPt, ptLatLong);
        
        *po_pXOut = cartesianPt.x;
        *po_pYOut = cartesianPt.y;
        }

    return status;
    }


//-----------------------------------------------------------------------------
// PRIVATE
// ComputeDomain - Copmputes the domains applicable to the transformation model
//-----------------------------------------------------------------------------
StatusInt HCPGCoordLatLongModel::ComputeDomain () const
    {
    StatusInt status = SUCCESS;

    if (!m_domainComputed)
        {
        // Domain not computed ... we first obtain the geographic domain from GCS
        HGF2DCoordCollection<double> gcsGeoDomain;

        HCPGCoordUtility::GetGeoDomain(*m_pBaseGCS, gcsGeoDomain);

        // Create the three coordinate systems required for transformation
        HFCPtr<HGF2DCoordSys> latLongCoordinateSystem = new HGF2DCoordSys();
        HFCPtr<HGF2DCoordSys> directCoordinateSystem = new HGF2DCoordSys(*this, latLongCoordinateSystem);

        HFCPtr<HVE2DShape> latLongDomainShape = new HVE2DPolygonOfSegments(HGF2DPolygonOfSegments(gcsGeoDomain), latLongCoordinateSystem);

        // Obtain shapes in direct of inverse coordinate systems, drop and copy points
        if (m_reversed)
            {
            HFCPtr<HVE2DShape> tempShape = static_cast<HVE2DShape*>(latLongDomainShape->AllocateCopyInCoordSys(directCoordinateSystem));
            m_domainDirect = tempShape->GetLightShape();
            m_domainInverse = latLongDomainShape->GetLightShape();
            }
        else
            {
            HFCPtr<HVE2DShape> tempShape = static_cast<HVE2DShape*>(latLongDomainShape->AllocateCopyInCoordSys (directCoordinateSystem));
            m_domainInverse = tempShape->GetLightShape();
            m_domainDirect = latLongDomainShape->GetLightShape();
            }

        m_domainComputed = true;
        }

    return status;
    }


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
bool HCPGCoordLatLongModel::HasDomain() const
    {
    return true;
    }


//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
HFCPtr<HGF2DShape> HCPGCoordLatLongModel::GetDirectDomain() const
    {
    ComputeDomain();

    return m_domainDirect;
    }

//-----------------------------------------------------------------------------
// CreateSimplifiedModel
//-----------------------------------------------------------------------------
HFCPtr<HGF2DShape> HCPGCoordLatLongModel::GetInverseDomain () const
    {
    StatusInt status = SUCCESS;

    status = ComputeDomain();

    return m_domainInverse;
    }
