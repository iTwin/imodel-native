//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: STM/ReprojectionModel.cpp $
//:>
//:>  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------

#include "ScalableMeshPCH.h" //precompile header, always first
#include "ImagePPHeaders.h"
#include "ReprojectionModel.h"
#include <ImagePP/all/h/HGF2DDisplacement.h>
#include <ImagePP/all/h/HGF2DIdentity.h>
#include <ImagePP/all/h/HCPGCoordUtility.h>
#ifndef VANCOUVER_API
#include <ImagePP/all/h/HCPGCoordLatLongModel.h>
#endif
#include <ImagePP/all/h/HVE2DShape.h>
#include <ImagePP/all/h/HVE2DPolygonOfSegments.h>

#include <GeoCoord\BaseGeoCoord.h>

USING_NAMESPACE_IMAGEPP

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2015
//----------------------------------------------------------------------------------------
ReprojectionModel::ReprojectionModel(GeoCoordinates::BaseGCSCR source, GeoCoordinates::BaseGCSCR destination)
:HGF2DTransfoModel(),
#ifndef VANCOUVER_API
m_pSrcGCS(&source),
m_pDestGCS(&destination),
#else
m_pSrcGCS(const_cast<GeoCoordinates::BaseGCSP>(&source)),
m_pDestGCS(const_cast<GeoCoordinates::BaseGCSP>(&destination)),
#endif
m_domainComputed(false),
m_domainDirect(NULL),
m_domainInverse(NULL),
m_isReverse(false)
    {
    HPRECONDITION(m_pSrcGCS->IsValid());
    HPRECONDITION(m_pDestGCS->IsValid());

    // This is to patch the fact that imagepp will always return a raster transfo model in meters but GCS assumed to receive
    // data in their corresponding units.
    m_srcUnitsFromMeters = m_pSrcGCS->UnitsFromMeters();
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2015
//----------------------------------------------------------------------------------------
ReprojectionModel::ReprojectionModel(const ReprojectionModel& pi_rObj)
    :HGF2DTransfoModel(pi_rObj),
    m_pSrcGCS(pi_rObj.m_pSrcGCS),
    m_pDestGCS(pi_rObj.m_pDestGCS),
    m_domainComputed(false),
    m_domainDirect(NULL),
    m_domainInverse(NULL),
    m_isReverse(pi_rObj.m_isReverse),
    m_srcUnitsFromMeters(pi_rObj.m_srcUnitsFromMeters)
    {}

//-----------------------------------------------------------------------------
// The destructor.
//-----------------------------------------------------------------------------
ReprojectionModel::~ReprojectionModel()
    {}

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2015
//----------------------------------------------------------------------------------------
StatusInt ReprojectionModel::Reproject(double& pio_XInOut, double& pio_YInOut, bool inverse) const
    {
    if (inverse)
        {
        DPoint3d inCartesian;
        inCartesian.x = pio_XInOut;
        inCartesian.y = pio_YInOut;
        inCartesian.z = 0.0;

        GeoPoint inLatLong;
        ReprojectStatus status = m_pDestGCS->LatLongFromCartesian(inLatLong, inCartesian);
        if (!(REPROJECT_Success == status || REPROJECT_CSMAPERR_OutOfUsefulRange == status))
            return status;

        GeoPoint outLatLong;
        status = m_pDestGCS->LatLongFromLatLong(outLatLong, inLatLong, *m_pSrcGCS);
        if (!(REPROJECT_Success == status || REPROJECT_CSMAPERR_OutOfUsefulRange == status))
            return status;

        DPoint3d outCartesian;
        status = m_pSrcGCS->CartesianFromLatLong(outCartesian, outLatLong);
        if (!(REPROJECT_Success == status || REPROJECT_CSMAPERR_OutOfUsefulRange == status))
            return status;

        pio_XInOut = outCartesian.x / m_srcUnitsFromMeters;
        pio_YInOut = outCartesian.y / m_srcUnitsFromMeters;
        }
    else
        {
        DPoint3d inCartesian;
        inCartesian.x = pio_XInOut * m_srcUnitsFromMeters;
        inCartesian.y = pio_YInOut * m_srcUnitsFromMeters;
        inCartesian.z = 0.0;

        GeoPoint inLatLong;
        ReprojectStatus status = m_pSrcGCS->LatLongFromCartesian(inLatLong, inCartesian);
        if (!(REPROJECT_Success == status || REPROJECT_CSMAPERR_OutOfUsefulRange == status))
            return status;

        GeoPoint outLatLong;
        status = m_pSrcGCS->LatLongFromLatLong(outLatLong, inLatLong, *m_pDestGCS);
        if (!(REPROJECT_Success == status || REPROJECT_CSMAPERR_OutOfUsefulRange == status))
            return status;

        DPoint3d outCartesian;
        status = m_pDestGCS->CartesianFromLatLong(outCartesian, outLatLong);
        if (!(REPROJECT_Success == status || REPROJECT_CSMAPERR_OutOfUsefulRange == status))
            return status;

        pio_XInOut = outCartesian.x;
        pio_YInOut = outCartesian.y;
        }

    return SUCCESS;
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2015
//----------------------------------------------------------------------------------------
#ifndef VANCOUVER_API
StatusInt ReprojectionModel::_ConvertDirect(double* pio_pXInOut, double* pio_pYInOut) const
#else
void ReprojectionModel::ConvertDirect(double* pio_pXInOut, double* pio_pYInOut) const
#endif
    {
#ifndef VANCOUVER_API
    return
#endif
	Reproject(*pio_pXInOut, *pio_pYInOut, m_isReverse);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2015
//----------------------------------------------------------------------------------------
#ifndef VANCOUVER_API
StatusInt ReprojectionModel::_ConvertDirect(double pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const
#else
void ReprojectionModel::ConvertDirect(double pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const
#endif
    {
    HPRECONDITION(po_pXOut != NULL);
    HPRECONDITION(po_pYOut != NULL);

    *po_pXOut = pi_XIn;
    *po_pYOut = pi_YIn;

#ifndef VANCOUVER_API
    return 
#endif
	Reproject(*po_pXOut, *po_pYOut, m_isReverse);
    }

//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2015
//----------------------------------------------------------------------------------------
#ifndef VANCOUVER_API
StatusInt ReprojectionModel::_ConvertDirect(double pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep, double* po_pXOut, double* po_pYOut) const
#else
void ReprojectionModel::ConvertDirect(double pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep, double* po_pXOut, double* po_pYOut) const
#endif
    {
    // Make sure recipient arrays are provided
    HPRECONDITION(po_pXOut != NULL);
    HPRECONDITION(po_pYOut != NULL);

#ifndef VANCOUVER_API
    StatusInt status = SUCCESS;
#endif

    double    X;
    uint32_t  Index;
    double* pCurrentX = po_pXOut;
    double* pCurrentY = po_pYOut;

    for (Index = 0, X = pi_XInStart; Index < pi_NumLoc; ++Index, X += pi_XInStep, ++pCurrentX, ++pCurrentY)
        {
#ifndef VANCOUVER_API
        StatusInt tempStatus = ConvertDirect(X, pi_YIn, pCurrentX, pCurrentY);

        // Return the first non-SUCCESS error and continue the process
        if ((SUCCESS != tempStatus) && (SUCCESS == status))
            status = tempStatus;
#else
	ConvertDirect(X, pi_YIn, pCurrentX, pCurrentY);
#endif
        }
#ifndef VANCOUVER_API
    return status;
#endif
    }


//----------------------------------------------------------------------------------------
// @bsimethod                                                   Mathieu.Marchand  10/2015
//----------------------------------------------------------------------------------------
#ifndef VANCOUVER_API
StatusInt ReprojectionModel::_ConvertDirect(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const
#else
void ReprojectionModel::ConvertDirect(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const
#endif
    {
    // Make sure recipient arrays are provided
    HPRECONDITION(pio_aXInOut != NULL);
    HPRECONDITION(pio_aYInOut != NULL);

#ifndef VANCOUVER_API
    StatusInt status = SUCCESS;
#endif

    for (uint32_t i = 0; i < pi_NumLoc; i++)
        {
#ifndef VANCOUVER_API
        StatusInt tempStatus = ConvertDirect(pio_aXInOut[i], pio_aYInOut[i], pio_aXInOut + i, pio_aYInOut + i);

        // Return the first non-SUCCESS error and continue the process
        if ((SUCCESS != tempStatus) && (SUCCESS == status))
            status = tempStatus;
#else
	ConvertDirect(pio_aXInOut[i], pio_aYInOut[i], pio_aXInOut + i, pio_aYInOut + i);
#endif
        }
#ifndef VANCOUVER_API
    return status;
#endif
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
#ifndef VANCOUVER_API
StatusInt ReprojectionModel::_ConvertInverse(double* pio_pXInOut, double* pio_pYInOut) const
#else
void ReprojectionModel::ConvertInverse(double* pio_pXInOut, double* pio_pYInOut) const
#endif
    {
#ifndef VANCOUVER_API
    return 
#endif
    Reproject(*pio_pXInOut, *pio_pYInOut, !m_isReverse);
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
#ifndef VANCOUVER_API
StatusInt ReprojectionModel::_ConvertInverse(double pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const
#else
void ReprojectionModel::ConvertInverse(double pi_XIn, double pi_YIn, double* po_pXOut, double* po_pYOut) const
#endif
    {
    HPRECONDITION(po_pXOut != NULL);
    HPRECONDITION(po_pYOut != NULL);

    *po_pXOut = pi_XIn;
    *po_pYOut = pi_YIn;

#ifndef VANCOUVER_API
    return 
#endif	
	Reproject(*po_pXOut, *po_pYOut, !m_isReverse);

    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
#ifndef VANCOUVER_API
StatusInt ReprojectionModel::_ConvertInverse(double pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep, double* po_pXOut, double* po_pYOut) const
#else
void ReprojectionModel::ConvertInverse(double pi_YIn, double pi_XInStart, size_t pi_NumLoc, double pi_XInStep, double* po_pXOut, double* po_pYOut) const
#endif
    {
    // Make sure recipient arrays are provided
    HPRECONDITION(po_pXOut != NULL);
    HPRECONDITION(po_pYOut != NULL);

#ifndef VANCOUVER_API
    StatusInt status = SUCCESS;
#endif
    double   X;
    uint32_t Index;
    double* pCurrentX = po_pXOut;
    double* pCurrentY = po_pYOut;

    for (Index = 0, X = pi_XInStart;
         Index < pi_NumLoc; ++Index, X += pi_XInStep, ++pCurrentX, ++pCurrentY)
		 {
#ifndef VANCOUVER_API        

        StatusInt tempStatus = ConvertInverse(X, pi_YIn, pCurrentX, pCurrentY);

        if ((SUCCESS != tempStatus) && (SUCCESS == status))
            status = tempStatus;
#else
	 ConvertInverse(X, pi_YIn, pCurrentX, pCurrentY);
#endif
        }

#ifndef VANCOUVER_API
    return status;
#endif
    }

//-----------------------------------------------------------------------------
// Converter (inverse)
//-----------------------------------------------------------------------------
#ifndef VANCOUVER_API
StatusInt ReprojectionModel::_ConvertInverse(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const
#else
void ReprojectionModel::ConvertInverse(size_t pi_NumLoc, double* pio_aXInOut, double* pio_aYInOut) const
#endif
    {
    // Make sure recipient arrays are provided
    HPRECONDITION(pio_aXInOut != NULL);
    HPRECONDITION(pio_aYInOut != NULL);

#ifndef VANCOUVER_API
    StatusInt status = SUCCESS;
#endif

    for (uint32_t i = 0; i < pi_NumLoc; i++)
        {
#ifndef VANCOUVER_API
        StatusInt tempStatus = ConvertInverse(pio_aXInOut[i], pio_aYInOut[i], pio_aXInOut + i, pio_aYInOut + i);

        // Return the first non-SUCCESS error yet continue the process as possible.
        if ((SUCCESS != tempStatus) && (SUCCESS == status))
            status = tempStatus;
#else
	ConvertInverse(pio_aXInOut[i], pio_aYInOut[i], pio_aXInOut + i, pio_aYInOut + i);
#endif
        }
#ifndef VANCOUVER_API
    return status;
#endif
    }

//-----------------------------------------------------------------------------
//  GetStetchParams
//  Returns the stretch parameters
//-----------------------------------------------------------------------------
#ifndef VANCOUVER_API
void ReprojectionModel::_GetStretchParams(double* po_pScaleFactorX, double* po_pScaleFactorY, HGF2DDisplacement* po_pDisplacement) const
#else
void ReprojectionModel::GetStretchParams(double* po_pScaleFactorX, double* po_pScaleFactorY, HGF2DDisplacement* po_pDisplacement) const
#endif
    {
    HPRECONDITION(po_pScaleFactorX != NULL);
    HPRECONDITION(po_pScaleFactorY != NULL);
    HPRECONDITION(po_pDisplacement != NULL);

    // Do not use!
    BeAssert(0);

    // Return stretch params for neutral model
    *po_pScaleFactorX = 1.0;
    *po_pScaleFactorY = 1.0;
    po_pDisplacement->SetDeltaX(0.0);
    po_pDisplacement->SetDeltaY(0.0);
    }

//-----------------------------------------------------------------------------
//  GetMatrix
//  Gets the components of the projective by matrix
//-----------------------------------------------------------------------------
#ifndef VANCOUVER_API
HFCMatrix<3, 3> ReprojectionModel::_GetMatrix() const
#else
HFCMatrix<3, 3> ReprojectionModel::GetMatrix() const
#endif
    {

    // Should not be called
    BeAssert(0);

    HFCMatrix<3, 3> Matrix;

    return (Matrix);
    }

//-----------------------------------------------------------------------------
// Reverse
// This method reverses the transformation model
//-----------------------------------------------------------------------------
#ifndef VANCOUVER_API
void    ReprojectionModel::_Reverse()
#else
void    ReprojectionModel::Reverse()
#endif
    {
    m_isReverse = !m_isReverse;

    // Invoke reversing of ancestor
    // This call will in turn invoque Prepare()
#ifndef VANCOUVER_API
    HGF2DTransfoModel::_Reverse();
#else
	HGF2DTransfoModel::Reverse();
#endif
    }


//-----------------------------------------------------------------------------
// ComposeInverseWithDirectOf
// Composes a new transformation model as a combination of self and given
//-----------------------------------------------------------------------------
#ifndef VANCOUVER_API
HFCPtr<HGF2DTransfoModel>  ReprojectionModel::_ComposeInverseWithDirectOf(const HGF2DTransfoModel& pi_rModel) const
#else
HFCPtr<HGF2DTransfoModel>  ReprojectionModel::ComposeInverseWithDirectOf(const HGF2DTransfoModel& pi_rModel) const
#endif
    {
    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    // Check if it is identity
    if (pi_rModel.IsIdentity())
        {
        // Model is identity ... return copy of self
        pResultModel = new ReprojectionModel(*this);

        }
    else
        {
        // Model is not known ... ask other
        pResultModel = CallComposeOf(pi_rModel);
        }

    return (pResultModel);
    }

//-----------------------------------------------------------------------------
// Clone
// This method allocates a copy of self. The caller is responsible for
// the deletion of this object.
//-----------------------------------------------------------------------------
#ifndef VANCOUVER_API
HGF2DTransfoModel* ReprojectionModel::_Clone() const
#else
HGF2DTransfoModel* ReprojectionModel::Clone() const
#endif
    {
    // Allocate object as copy and return
    return new ReprojectionModel(*this);
    }

//-----------------------------------------------------------------------------
// ComposeYourself
// PRIVATE
//-----------------------------------------------------------------------------
#ifndef VANCOUVER_API
HFCPtr<HGF2DTransfoModel>  ReprojectionModel::_ComposeYourself(const HGF2DTransfoModel& pi_rModel) const
#else
HFCPtr<HGF2DTransfoModel>  ReprojectionModel::ComposeYourself(const HGF2DTransfoModel& pi_rModel) const
#endif
    {
    // Recipient
    HFCPtr<HGF2DTransfoModel> pResultModel;

    // Type is not known ... build a complex
    // To do this we call the ancestor ComposeYourself

#ifndef VANCOUVER_API
    pResultModel = HGF2DTransfoModel::_ComposeYourself(pi_rModel);
#else
    pResultModel = HGF2DTransfoModel::ComposeYourself(pi_rModel);
#endif
    
    return (pResultModel);
    }

//-----------------------------------------------------------------------------
// CreateSimplifiedModel
//-----------------------------------------------------------------------------
#ifndef VANCOUVER_API
HFCPtr<HGF2DTransfoModel> ReprojectionModel::_CreateSimplifiedModel() const
#else
HFCPtr<HGF2DTransfoModel> ReprojectionModel::CreateSimplifiedModel() const
#endif
    {
    if (m_pSrcGCS->IsEquivalent(*m_pDestGCS))
        return new HGF2DIdentity();

    // If we get here, no simplification is possible.
    return 0;
    }

//-----------------------------------------------------------------------------
// GetSourceGEOCS
// Returns the source projection
//-----------------------------------------------------------------------------
GeoCoordinates::BaseGCSCR ReprojectionModel::GetSourceGCS() const
    {
    return *m_pSrcGCS;
    }

//-----------------------------------------------------------------------------
// GetDestinationGEOCS
// Returns the destination projection
//-----------------------------------------------------------------------------
GeoCoordinates::BaseGCSCR ReprojectionModel::GetDestinationGCS() const
    {
    return *m_pDestGCS;
    }

//-----------------------------------------------------------------------------
// PRIVATE
// ComputeDomain - Copmputes the domains applicable to the transformation model
//-----------------------------------------------------------------------------
StatusInt ReprojectionModel::ComputeDomain() const
    {
    StatusInt status = SUCCESS;

#ifndef VANCOUVER_API
    if (!m_domainComputed)
        {
        // Domain not computed ... we first obtain the geographic domain from both GCS
        bvector<GeoPoint> sourceGeoDomain;
        bvector<GeoPoint> destinationGeoDomain;

        HGF2DCoordCollection<double> sourceGeoDomain2;
        HGF2DCoordCollection<double> destinationGeoDomain2;

        m_pSrcGCS->GetMathematicalDomain(sourceGeoDomain);
        m_pDestGCS->GetMathematicalDomain(destinationGeoDomain);

        for (int idx = 0; idx < sourceGeoDomain.size(); idx++)
            sourceGeoDomain2.push_back(HGF2DCoord<double>(sourceGeoDomain[idx].longitude, sourceGeoDomain[idx].latitude));

        for (int idx = 0; idx < destinationGeoDomain.size(); idx++)
            destinationGeoDomain2.push_back(HGF2DCoord<double>(destinationGeoDomain[idx].longitude, destinationGeoDomain[idx].latitude));

        // Create the three coordinate systems required for transformation
        HFCPtr<HGF2DCoordSys> latLongCoordinateSystem = new HGF2DCoordSys();
        HFCPtr<HGF2DTransfoModel> directLatLongTransfoModel = new HCPGCoordLatLongModel(*m_pSrcGCS);
        HFCPtr<HGF2DTransfoModel> inverseLatLongTransfoModel = new HCPGCoordLatLongModel(*m_pDestGCS);
        HFCPtr<HGF2DCoordSys> directCoordinateSystem = new HGF2DCoordSys(*directLatLongTransfoModel, latLongCoordinateSystem);
        HFCPtr<HGF2DCoordSys> inverseCoordinateSystem = new HGF2DCoordSys(*inverseLatLongTransfoModel, latLongCoordinateSystem);

        // Create shapes from these

        HFCPtr<HVE2DShape> sourceDomainShape = new HVE2DPolygonOfSegments(HGF2DPolygonOfSegments(sourceGeoDomain2), latLongCoordinateSystem);
        HFCPtr<HVE2DShape> destinationDomainShape = new HVE2DPolygonOfSegments(HGF2DPolygonOfSegments(destinationGeoDomain2), latLongCoordinateSystem);

        HFCPtr<HVE2DShape> resultDomainShape = sourceDomainShape->IntersectShape(*destinationDomainShape);

        // Obtain shapes in direct of inverse coordinate systems, drop and copy points
        HFCPtr<HVE2DShape> tempShapeDirect = static_cast<HVE2DShape*>(resultDomainShape->AllocateCopyInCoordSys(directCoordinateSystem));
        m_domainDirect = tempShapeDirect->GetLightShape();
        HFCPtr<HVE2DShape> tempShapeInverse = static_cast<HVE2DShape*>(resultDomainShape->AllocateCopyInCoordSys(inverseCoordinateSystem));
        m_domainInverse = tempShapeInverse->GetLightShape();

        m_domainComputed = true;
        }
		
#endif
    return status;
    }

//-----------------------------------------------------------------------------
// 
//-----------------------------------------------------------------------------
#ifndef VANCOUVER_API
HFCPtr<HGF2DShape> ReprojectionModel::_GetDirectDomain() const
#else
HFCPtr<HGF2DShape> ReprojectionModel::GetDirectDomain() const
#endif
    {
    ComputeDomain();

    return m_domainDirect;
    }

//-----------------------------------------------------------------------------
// CreateSimplifiedModel
//-----------------------------------------------------------------------------
#ifndef VANCOUVER_API
HFCPtr<HGF2DShape> ReprojectionModel::_GetInverseDomain() const
#else
HFCPtr<HGF2DShape> ReprojectionModel::GetInverseDomain() const
#endif
    {
    ComputeDomain();

    return m_domainInverse;
    }
