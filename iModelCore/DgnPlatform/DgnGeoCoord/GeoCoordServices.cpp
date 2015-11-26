/*----------------------------------------------------------------------+
|
|   $Source: DgnGeoCoord/GeoCoordServices.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <Geom/GeomApi.h>
#include <DgnPlatform/IGeoCoordServices.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/GeoCoordErrors.h>
#include <csmap/cs_map.h>
#include "GeoCoordElement.h"
#include <GeoCoord/GCSLibrary.h>

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM
using namespace BentleyApi::GeoCoordinates;

namespace {

/*=================================================================================**//**
* This is the class that implements geocoordinate reference interface.
* @bsiclass                                                     Barry.Bentley   10/06
+===============+===============+===============+===============+===============+======*/
class   GeoCoordinateServices : public IGeoCoordinateServices
{
public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual WCharCP GetGCSName (WStringR gcsName, DgnGCSP sourceGCS) override
    {
    gcsName.clear();

    if (sourceGCS !=NULL && sourceGCS->IsValid())
        return sourceGCS->GetDisplayName (gcsName);

    return gcsName.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     04/2012
+---------------+---------------+---------------+---------------+---------------+------*/
virtual WCharCP GetDescription (WStringR gcsDescription, DgnGCSP sourceGCS) override
    {
    gcsDescription.clear();
    if (sourceGCS != NULL && sourceGCS->IsValid())
        gcsDescription.assign (sourceGCS->GetDescription ());

    return gcsDescription.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   04/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual GeoReferenceStatus  ReprojectUorPoints
(
DgnDbR    source,
DgnDbR    target,
DPoint3dP       outUors,
DPoint3dCP      inUors,
int             numPoints
) override
    {
    if ( (NULL == outUors) || (NULL == inUors) || (numPoints <= 0) )
        return GEOREF_STATUS_BadArg;

    DgnGCSP     refGCS;
    if (NULL == (refGCS = DgnGCS::FromProject (source)))
        return GEOREF_STATUS_AttachmentHasNoGCS;

    DgnGCSP     targetGCS;
    if (NULL == (targetGCS = DgnGCS::FromProject (target)))
        return GEOREF_STATUS_MasterHasNoGCS;

    return (GeoReferenceStatus) refGCS->ReprojectUors (outUors, NULL, NULL, inUors, numPoints, *targetGCS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   04/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual GeoReferenceStatus  ReprojectUorPoints
(
DgnGCSP         sourceGCS,
DgnGCSP         targetGCS,
DPoint3dP       outUors,
DPoint3dCP      inUors,
int             numPoints
) override
    {
    if ( (NULL == outUors) || (NULL == inUors) || (numPoints <= 0) )
        return GEOREF_STATUS_BadArg;

    if ( (NULL == sourceGCS) || (NULL == targetGCS) )
        return GEOREF_STATUS_BadArg;

    return (GeoReferenceStatus) sourceGCS->ReprojectUors (outUors, NULL, NULL, inUors, numPoints, *targetGCS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool        HasGCS (DgnDbR project) override
    {
    return (NULL != DgnGCS::FromProject (project));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual DgnGCS* GetGCSFromProject (DgnDbR project) override
    {
    return DgnGCS::FromProject (project);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool        RequiresReprojection (DgnGCSP sourceGCS, DgnGCSP targetGCS)
    {
    if ( (NULL == sourceGCS) || (NULL == targetGCS) )
        return false;

    if (!sourceGCS->IsValid() || !targetGCS->IsValid())
        return false;

    if (!sourceGCS->UnitsIdentical (*targetGCS))
        return true;

    return !sourceGCS->IsEquivalent (*targetGCS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     03/2011
+---------------+---------------+---------------+---------------+---------------+------*/
virtual bool        GetUnitDefinition (DgnGCSP sourceGCS, UnitDefinitionR unitDef, Dgn::StandardUnit&  standardUnitNumber) override
    {
    return (SUCCESS == sourceGCS->GetUnitDefinition(unitDef,standardUnitNumber));
    }

virtual  BentleyStatus UorsFromLatLong
(
DPoint3dR               outUors,
GeoPointCR              inLatLong,
DgnGCS&                 gcs
)
    {
    return gcs.UorsFromLatLong (outUors, inLatLong) == ReprojectStatus::REPROJECT_Success? BSISUCCESS: BSIERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Sam.Wilson      11/14
+---------------+---------------+---------------+---------------+---------------+------*/
virtual  BentleyStatus LatLongFromUors
(
GeoPointR               outLatLong,
DPoint3dCR              inUors,
DgnGCS&                 gcs
) override
    {
    return gcs.LatLongFromUors (outLatLong, inUors) == ReprojectStatus::REPROJECT_Success? BSISUCCESS: BSIERROR;
    }
        
};

}   // Ends anonymous namespace

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
IGeoCoordinateServicesP   DgnGeoCoordinationAdmin::_GetServices () const
    {
    if (NULL == m_gcrp)
        {
        CompleteInitialization();
        // if we don't have a data directory, can't provide services.
        if (!m_dataDirectory.empty())
            m_gcrp = new GeoCoordinateServices();
        }

    return m_gcrp;
    }
