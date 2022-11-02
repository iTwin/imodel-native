/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <DgnPlatformInternal.h>
#include <Geom/GeomApi.h>
#include <DgnPlatform/DgnGeoCoord.h>
#include <DgnPlatform/GeoCoordErrors.h>
#include <csmap/cs_map.h>
#include "GeoCoordElement.h"

// tolerance is from old geocoord code
#define GEOCOORD_CLOSE_TOLERANCE (1e-5)
#define GEOCOORD66_SIZE (sizeof(GeoCoordType66) + 80*sizeof(double))
#define GEOCOORD66_VerticalDatumValid (0x8117)
#if defined (TEST_TYPE66_CREATE)
static GeoCoordType66*     s_oldType66;
static ProjectionParams*   s_oldProjectionParams;
#endif

#define     MSGEOCOORD_SIGNATURE                182     // 0x00b6
#define     MSAPPINFO_LEVEL             20

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGN

using namespace BentleyApi::GeoCoordinates;

#define RUNONCE_CHECK(var) {if (var) return; var=true;}
#define DIM(a) ((sizeof(a)/sizeof((a)[0])))

#define GEOCOORD_ACS_EXTENDERID     0x04151956
#define GEOCOORD_MGRS_EXTENDERID    0x03271957

BEGIN_UNNAMED_NAMESPACE


/*=================================================================================**//**
* Formatter for reading/writing the Type 66 element.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct  CoordinateSystemDgnFormatter
{
friend DgnGCS;

private:

// instance members
void                                   *m_structConversionRules;
CSDatum*                                m_datum;

// static members.
static  CoordinateSystemDgnFormatter   *s_instance;

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
private:
CoordinateSystemDgnFormatter
(
)
    {
    m_structConversionRules = NULL;
    m_datum                 = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
public:
bool            IsKeyNameCoordinateSystem
(
int     projType
)
    {
    return ( (COORDSYS_KEYNM == projType) || (COORDSYS_SPCSL == projType) || (COORDSYS_StatePlane == projType) );
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
public:
static  CoordinateSystemDgnFormatter *GetInstance()
    {
    if (NULL == s_instance)
        s_instance = new CoordinateSystemDgnFormatter();

    return s_instance;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       UpgradeType66Data
(
GeoCoordType66      &extracted,
ProjectionParams    &projectionParams
)
    {
    // This stuff taken directly from the old geocoordinate code.
    if (extracted.minor_version < GCOORD_MINOR_VERSION_460)
        {
        if (COORDSYS_UTM == extracted.coordsys)
            {
            Proj_Utmzn       utmzn;
            memset (&utmzn, 0, sizeof (utmzn));

            if (!sscanf (extracted.zone_knm, "%d", &utmzn.zoneNo))
                {
                utmzn.zoneNo = 0;
                return GeoCoordError_InvalidUTMZone;
                }

            extracted.coordsys = extracted.projType = COORDSYS_UTMZN;

            // the old code would have always ended up with hemisphere OBE_HEMISPHERE_NORTH, but this looks like what it was trying to do.
            utmzn.hemisphere = (0 == BeStringUtilities::Stricmp(extracted.grp_knm, "UTMS")) ? OBE_HEMISPHERE_SOUTH : OBE_HEMISPHERE_NORTH;
            utmzn.gcDom     = projectionParams.trmer.gcDom;
            utmzn.paper_scl = projectionParams.trmer.paper_scl;
            utmzn.quad      = projectionParams.trmer.quad;
            CSMap::CS_stncp(utmzn.unit_nm, projectionParams.trmer.unit_nm, DIM (utmzn.unit_nm));
            CSMap::CS_stncp(utmzn.dat_knm, projectionParams.trmer.dat_knm, DIM (utmzn.dat_knm));
            CSMap::CS_stncp(utmzn.ell_knm, projectionParams.trmer.ell_knm, DIM (utmzn.ell_knm));

            /* Copy the whole thing into the projectionParams union. */
            projectionParams.utmzn = utmzn;
            }
        else if (COORDSYS_STERO == extracted.coordsys)
            {
            if (fabs(projectionParams.stero.org_lat - 90.0) < GEOCOORD_CLOSE_TOLERANCE)
                {
                /* Format is identical */
                extracted.prj_code  = cs_PRJCOD_PSTRO;
                extracted.projType  =  COORDSYS_PSTRO;
                strncpy(extracted.prj_knm, CS_PSTRO, _countof(extracted.prj_knm));
                }
            else
                {
                extracted.prj_code  = cs_PRJCOD_SSTRO;
                extracted.projType  =  COORDSYS_STERO;
                strncpy(extracted.prj_knm, CS_STERO, _countof(extracted.prj_knm));
                }
            }
        }

    /* Some additions were made to unity coordsys params for 5.5.1.42 */
    if (extracted.minor_version < GCOORD_MINOR_VERSION_420)
        {
        if (COORDSYS_UNITY == extracted.projType)
            {
            projectionParams.unity.quad = 1; /* newthink: good (oldthink: bad) */
            // BJB COMMENT: as to newthink/oldthink if it was DB think, both bad.

            /* These fields added to match cs structure */
            projectionParams.unity.user_min = projectionParams.unity.domEN.min_x;
            projectionParams.unity.user_max = projectionParams.unity.domEN.max_x;

            /* These fields were added, but old ones may not be complete */
            projectionParams.unity.domLL.min_lng = projectionParams.unity.domEN.min_x;
            projectionParams.unity.domLL.min_lat = projectionParams.unity.domEN.min_y;
            projectionParams.unity.domLL.max_lng = projectionParams.unity.domEN.max_x;
            projectionParams.unity.domLL.max_lat = projectionParams.unity.domEN.max_y;

            // I have no idea what this "logic" was supposed to accomplish, but it was in the old geocoord stuff.
            if ( (fabs(projectionParams.unity.domLL.min_lng) > GEOCOORD_CLOSE_TOLERANCE) ||
                 (fabs(projectionParams.unity.domLL.max_lng) > GEOCOORD_CLOSE_TOLERANCE) )
                {
                if ( (fabs(projectionParams.unity.domLL.min_lat) > GEOCOORD_CLOSE_TOLERANCE) &&
                     (fabs(projectionParams.unity.domLL.max_lat) > GEOCOORD_CLOSE_TOLERANCE) )
                    {
                    projectionParams.unity.domLL.min_lat = -90;
                    projectionParams.unity.domLL.max_lat =  90;
                    }
                }
            else if ( (fabs(projectionParams.unity.domLL.min_lat) > GEOCOORD_CLOSE_TOLERANCE) ||
                      (fabs(projectionParams.unity.domLL.max_lat) > GEOCOORD_CLOSE_TOLERANCE) )
                {
                if ( (fabs(projectionParams.unity.domLL.min_lng) > GEOCOORD_CLOSE_TOLERANCE) &&
                     (fabs(projectionParams.unity.domLL.max_lng) > GEOCOORD_CLOSE_TOLERANCE) )
                    {
                    projectionParams.unity.domLL.min_lng = -270;
                    projectionParams.unity.domLL.max_lng =  270;
                    }
                }

            /* These are no longer expected to be used */
            projectionParams.unity.domEN.min_x = 0;
            projectionParams.unity.domEN.min_y = 0;
            projectionParams.unity.domEN.max_x = 0;
            projectionParams.unity.domEN.max_y = 0;
            }
        }

    // changes for 08000500
    if (extracted.minor_version < GCOORD_MINOR_VERSION_4000)
        {
        if (COORDSYS_StatePlane == extracted.coordsys)
            extracted.coordsys = COORDSYS_SPCSL;
        else if (COORDSYS_UTM == extracted.coordsys)
            extracted.coordsys = COORDSYS_UTMZN;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
public:
StatusInt       Extract
(
GeoCoordType66      &extracted,
ProjectionParams    &projectionParams,
LocalTransformerP   &localTransformer,
const short         *type66AppData
)
    {
    // start by setting whole structure to 0's.
    memset((void*)&extracted, 0, sizeof (extracted));
    memset((void*)&projectionParams, 0, sizeof (projectionParams));
    localTransformer = NULL;

    StatusInt   status = SUCCESS;

    // this used to use the resource conversion logic, but it turns out that the structure was set up
    // correctly, grouping the doubles and filling in the holes so just requires a memcpy.
    memcpy(&extracted, &type66AppData[1], sizeof (extracted));

    // check the version and size. In the old GeoCoord code, the type 66 was accepted unless its version was newer than GCOORD_CURRENT_VERSION, so do that here, too.
    if (extracted.version > GCOORD_08119_VERSION)
        return GeoCoordError_BadType66Version;

    // The ProjectionParams union was also set up correctly to allow a simple memcpy for every type of projection.
    memcpy (&projectionParams, &type66AppData[429], sizeof (projectionParams));

#if defined (NOTNOW)
    if (IsKeyNameCoordinateSystem(extracted.coordsys))
        {
        // when there's a "Key Name" coordinate system, extracted.coordsys will differ from extracted.projType.
        // The information stored in the union is presumably appropriate to extracted.projType, but we should be
        // able to look up the coordinate system data from the coordinate system dictionary. Thus we have redundant
        // information about the coordinate system. I guess we really should use the data stored in the design file,
        // since presumably the coordinates that are in there were placed assuming that information. I think the best
        // thing to do is probably to look up the coordinate system from the key name, and compare to what's in the
        // design file, giving the user the option of either reprojecting or
        status = SUCCESS;
        }
#endif

    // upgrade old format only.
    if ( (GCOORD_COMPATIBLE_08117_VERSION == extracted.version) && (extracted.minor_version < GCOORD_COMPATIBLE_08117_MINOR_VERSION) )
        UpgradeType66Data(extracted, projectionParams);

    if (GCOORD_08119_VERSION == extracted.version)
        localTransformer = LocalTransformer::CreateLocalTransformer((LocalTransformType)extracted.localTransformType, extracted.transformParams);
    else
        extracted.dtmOrElpsdFromUserLib = 0;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
private:
void            CommonParamsToCSDef
(
CSDefinition        &csDef,
GeoCoordType66      &extracted
)
    {
    csDef.protect = static_cast<short>(extracted.protect); // NEEDSWORK - is cast correct?  Should protect be a short?

    CSMap::CS_stncp(csDef.group,   extracted.grp_knm, DIM(csDef.group));
    CSMap::CS_stncp(csDef.prj_knm, extracted.prj_knm, DIM(csDef.prj_knm));
    CSMap::CS_stncp(csDef.source,  extracted.source, DIM(csDef.source));
    CSMap::CS_stncp(csDef.desc_nm, extracted.desc_nm, DIM(csDef.desc_nm));

    // I think this will be ignored by cs_map (unless we do cs_csloc, which we don't). I put into the structure
    //   so that it will be retained and then saved back to the type 66, as that what I found in all existing type 66's.
    CSMap::CS_stncp(csDef.key_nm,  extracted.cs_knm, DIM(csDef.key_nm));


    csDef.epsgNbr = (uint16_t)extracted.epsgNbr;

#if 0
    // truncate desc_nm at comma. (from old version, claims it was stripping unit name).
    char*   pComma;
    if (NULL  != (pComma = strrchr(csDef.desc_nm, ',')))
        *pComma = 0;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            DomainToCsDef
(
CSDefinition        &csDef,
GCDomain            &domain
)
    {
    csDef.ll_min[0] = domain.domLL.min_lng;
    csDef.ll_min[1] = domain.domLL.min_lat;
    csDef.ll_max[0] = domain.domLL.max_lng;
    csDef.ll_max[1] = domain.domLL.max_lat;

    csDef.xy_min[0] = domain.domEN.min_x;
    csDef.xy_min[1] = domain.domEN.min_y;
    csDef.xy_max[0] = domain.domEN.max_x;
    csDef.xy_max[1] = domain.domEN.max_y;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            AlbersToCsDef                   // COORDSYS_ALBER
(
CSDefinition        &csDef,
Proj_alber          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_ALBER, DIM(csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.lat_1;
    csDef.prj_prm2  = params.lat_2;
    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            AzimuthalEquidistantElevatedToCsDef     // COORDSYS_AZEDE
(
CSDefinition        &csDef,
Proj_azede          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_AZEDE, DIM(csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.az;
    csDef.prj_prm2  = params.ellElev;
    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            AzimuthalEqualAreaToCsDef       // COORDYS_AZMEA
(
CSDefinition        &csDef,
Proj_azmea          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_AZMEA, DIM(csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.az;
    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            AzimuthalEquidistantToCsDef     // COORDSYS_AZMED
(
CSDefinition        &csDef,
Proj_azmed          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_AZMED, DIM(csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.az;
    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            BonneToCsDef                    // COORDSYS_BONNE
(
CSDefinition        &csDef,
Proj_bonne          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_BONNE, DIM(csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            BipolarObliqueConformalConicToCsDef             // COORDSYS_BPCNC
(
CSDefinition        &csDef,
Proj_bpcnc          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_BPCNC, DIM(csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.lng_1;
    csDef.prj_prm2  = params.lat_1;
    csDef.prj_prm3  = params.lng_2;
    csDef.prj_prm4  = params.lat_2;
    csDef.prj_prm5  = params.pole_dd;
    csDef.prj_prm6  = params.sp_1;
    csDef.prj_prm7  = params.sp_2;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            CassiniToCsDef                  // COORDSYS_CSINI
(
CSDefinition        &csDef,
Proj_csini          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_CSINI, DIM(csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            EquidistantConicToCsDef         // COORDSYS_EDCNC
(
CSDefinition        &csDef,
Proj_edcnc          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_EDCNC, DIM(csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.lat_1;
    csDef.prj_prm2  = params.lat_2;
    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            EquidistantCylindricalToCsDef   // COORDSYS_EDCYL, COORDSYS_EDCYLE
(
CSDefinition        &csDef,
Proj_edcyl          &params,
CharCP            csMapCoordName
)
    {
    CSMap::CS_stncp(csDef.prj_knm, csMapCoordName, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.std_lat;
    csDef.org_lat   = params.org_lat;
    csDef.org_lng   = params.org_lng;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            PlateCarreeToCsDef   // COORDSYS_PCARREE
(
CSDefinition        &csDef,
Proj_pcarree         &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_PCARREE, DIM(csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.org_lat   = params.org_lat;
    csDef.org_lng   = params.org_lng;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            Eckert4ToCsDef                  // COORDSYS_EKRT4
(
CSDefinition        &csDef,
Proj_ekrt4          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_EKRT4, DIM(csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.org_lng;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            Eckert6ToCsDef                  // COORDSYS_EKRT6
(
CSDefinition        &csDef,
Proj_ekrt6          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_EKRT6, DIM(csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.org_lng;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            TransverseMercatorToCsDef       // COORDSYS_GAUSK
(
CSDefinition        &csDef,
Proj_trmer          &params,
CharCP            csMapCoordName
)
    {
    CSMap::CS_stncp(csDef.prj_knm, csMapCoordName, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.scl_red   = params.scl_red;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            TransverseMercatorMinnesotaToCsDef              // COORDSYS_TMMIN
(
CSDefinition        &csDef,
Proj_tmmin          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_TMMIN, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.org_lng;
    csDef.prj_prm2  = params.ellElev;
    csDef.org_lat   = params.org_lat;
    csDef.scl_red   = params.scl_red;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            TransverseMercatorWisconsinToCsDef              // COORDSYS_TMWIS
(
CSDefinition        &csDef,
Proj_tmwis          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_TMWIS, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.org_lng;
    csDef.prj_prm2  = params.geoidSep;
    csDef.prj_prm3  = params.geoidElev;
    csDef.org_lat   = params.org_lat;
    csDef.scl_red   = params.scl_red;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            TransverseMercatorAffinePostProcessToCsDef      // COORDSYS_TRMAF
(
CSDefinition        &csDef,
Proj_trmaf          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_TRMAF, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.org_lng;
    // NOTE: The CSMap documentation has them ordered A0,B0,A1,A2,B1,B2, and the code looks like that is exactly what they expect.
    //       However, the (stupid) old code labelled them A0,A1,A2,B0,B1,B2, and stored and restored them as if they were in that
    //       order. The member name of the structure of course doesn't really matter as long as they are stored and restored in
    //       consistent order, so I stuck with the (stupid) old code member names and order.
    csDef.prj_prm2  = params.affineA0;
    csDef.prj_prm3  = params.affineA1;
    csDef.prj_prm4  = params.affineA2;
    csDef.prj_prm5  = params.affineB0;
    csDef.prj_prm6  = params.affineB1;
    csDef.prj_prm7  = params.affineB2;
    csDef.org_lat   = params.org_lat;
    csDef.scl_red   = params.scl_red;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            UniversalTransverseMercatorZoneToCsDef          // COORDSYS_UTMZN
(
CSDefinition        &csDef,
Proj_utmzn          &params,
CharCP            csMapCoordName
)
    {
    CSMap::CS_stncp(csDef.prj_knm, csMapCoordName, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.zoneNo;
    csDef.prj_prm2  = (OBE_HEMISPHERE_NORTH == params.hemisphere ? 1 : -1); // Any positive is North, any negative is south (0 is interpreted as North)

    // Due to a small mixup (see TR# 261800) sometimes the hemisphere of the UTM defined coordinate system is incorrectyl interpreted
    // Since the code was fixed this only applies to coordinate system stored for the very first released version
    // For compatibility of coordinate stored using this version we extract the CS definition from the dictionary if it exists and make sure the
    // hemisphere definition fit. If not then likely the UTM definition was incorrectly stored and we corect it internally (not re-stored)
    if (strlen(csDef.key_nm) > 0)
        {
        try
            {
            BaseGCSPtr tempGCS = BaseGCS::CreateGCS(csDef.key_nm);
            if (tempGCS->IsValid())
                csDef.prj_prm2  = tempGCS->GetHemisphere();
            }
        catch (...)
            {
            // Keyname based does not exist in dictionary or definition is not UTM or any other error condition... we keep the hemisphere as it is
            }
        }
    csDef.map_scl   = params.paper_scl;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            GnomicToCsDef                   // COORDSYS_GNOMC
(
CSDefinition        &csDef,
Proj_gnomc          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_GNOMC, DIM(csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            ZonesToCsDef
(
double              *dest,
ZONE                *zone,
int                 numZones
)
    {
    // set up the zones. NOTE: This did not appear to be supported in the old geocoordinate stuff.
    // I am going by the comments in the ZONE typedef, probably added by TI Doug Bilinski.
    for (int iZone=0; iZone < numZones; iZone++, zone++)
        {
        // the first boundary is encoded with the applicable hemispheres. See CS_zones documentation in CSMap.
        *dest       = zone->west_bndry;
        *(dest+1)   = zone->cntrl_mer;
        *(dest+2)   = zone->east_bndry;
        if (zone->north_south > 0.0)
            *dest += 1000;
        else if (zone->north_south < 0.0)
            *dest += 2000;
        dest += 3;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            HomolsineToCsDef                // COORDSYS_HMLSN
(
CSDefinition        &csDef,
Proj_hmlsn          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_HMLSN, DIM(csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    ZonesToCsDef(&csDef.prj_prm1, params.zone, params.nZones);
    csDef.org_lng   = params.org_lng;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            ObliqueMercator1PointToCsDef    // COORDSYS_HOM1U, COORDSYS_OBLQ1, COORDSYS_RSKEW, COORDSYS_RSKWC, COORDSYS_RSKWO
(
CSDefinition        &csDef,
Proj_Oblq1          &params,
CharCP              csMapCoordName
)
    {
    CSMap::CS_stncp(csDef.prj_knm, csMapCoordName, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.lng_c;
    csDef.prj_prm2  = params.lat_c;
    csDef.prj_prm3  = params.az;
    csDef.scl_red   = params.scl_red;   // CSMap documentation does not mention using it.

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            MinnesotaDOTObliqueMercatorToCsDef    // COORDSYS_MNDOTOBL
(
CSDefinition        &csDef,
Proj_Mndotobl       &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_MNDOTOBL, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.lng_c;
    csDef.prj_prm2  = params.lat_c;
    csDef.prj_prm3  = params.az;
    csDef.prj_prm4  = params.ellElev;
    csDef.scl_red   = params.scl_red;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            ObliqueMercator2PointToCsDef    // COORDSYS_HOM2U, COORDSYS_OBLQ2
(
CSDefinition        &csDef,
Proj_Oblq2          &params,
CharCP              csMapCoordName
)
    {
    CSMap::CS_stncp(csDef.prj_knm, csMapCoordName, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.lng_1;
    csDef.prj_prm2  = params.lat_1;
    csDef.prj_prm3  = params.lng_2;
    csDef.prj_prm4  = params.lat_2;
    csDef.org_lat   = params.org_lat;
    csDef.scl_red   = params.scl_red;   // CSMap documentation does not mention using it.

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            HungarianEOVToCsDef             // COORDSYS_HUEOV
(
CSDefinition        &csDef,
Proj_Hueov          &params
)
    {
    // No CSMap documentation, looking at old geocoord stuff.
    CSMap::CS_stncp(csDef.prj_knm, CS_HUEOV, DIM(csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.prj_prm1  = params.std_lat;
    csDef.scl_red   = params.scl_red;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            KrovakToCsDef                   // COORDSYS_KRVKG,COORDSYS_KRVKP, COORDSYS_KRVK, COORDSYS_KRVMO and COORDSYS_KRVK95
(
CSDefinition        &csDef,
Proj_Krvak          &params,
CharCP         csMapCoordName
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord code.
    CSMap::CS_stncp(csDef.prj_knm, csMapCoordName, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.pole_lng;
    csDef.prj_prm2  = params.pole_lat;
    csDef.prj_prm3  = params.std_lat;
    csDef.scl_red   = params.scl_red;
    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;

    if (params.apply95)
        CSMap::CS_stncp(csDef.prj_knm, CS_KRVK95, DIM (csDef.prj_knm));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            LambertTangentialToCsDef        // COORDSYS_LM1SP, COORDSYS_LMTAN
(
CSDefinition        &csDef,
Proj_Lmtan          &params,
CharCP            csMapCoordName
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord cold.
    CSMap::CS_stncp(csDef.prj_knm, csMapCoordName, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.scl_red   = params.scl_red;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            LambertConformalConicToCsDef    // COORDSYS_LMBRT, COORDSYS_LMBLG
(
CSDefinition        &csDef,
Proj_Lmbrt          &params,
CharCP         csMapCoordName
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord cold.
    CSMap::CS_stncp(csDef.prj_knm, csMapCoordName, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.lat_1;
    csDef.prj_prm2  = params.lat_2;
    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            LambertConformalConicMinnesotaToCsDef           // COORDSYS_LMMIN
(
CSDefinition        &csDef,
Proj_Lmmin          &params
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord code.
    CSMap::CS_stncp(csDef.prj_knm, CS_LMMIN, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.lat_1;
    csDef.prj_prm2  = params.lat_2;
    csDef.prj_prm3  = params.ellElev;
    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            LambertConformalConicMichiganToCsDef           // COORDSYS_LMMICH
(
CSDefinition        &csDef,
Proj_Lmmich         &params
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord code.
    CSMap::CS_stncp(csDef.prj_knm, CS_LMMICH, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.lat_1;
    csDef.prj_prm2  = params.lat_2;
    csDef.prj_prm3  = params.ell_scale;
    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            LambertConformalConicWisconsinToCsDef           // COORDSYS_LMWIS
(
CSDefinition        &csDef,
Proj_Lmwis          &params
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord code.
    CSMap::CS_stncp(csDef.prj_knm, CS_LMWIS, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.lat_1;
    csDef.prj_prm2  = params.lat_2;
    csDef.prj_prm3  = params.geoidSep;
    csDef.prj_prm4  = params.geoidElev;
    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            LambertAffinePostProcessToCsDef                 // COORDSYS_LMBRTAF
(
CSDefinition        &csDef,
Proj_lmbrtaf        &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_LMBRTAF, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.lat_1;
    csDef.prj_prm2  = params.lat_2;
    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;

    // this seemingly random order is what cs_map uses.
    csDef.prj_prm3  = params.affineA0;
    csDef.prj_prm4  = params.affineB0;
    csDef.prj_prm5  = params.affineA1;
    csDef.prj_prm6  = params.affineA2;
    csDef.prj_prm7  = params.affineB1;
    csDef.prj_prm8  = params.affineB2;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            MillerToCsDef                   // COORDSYS_MILLR
(
CSDefinition        &csDef,
Proj_millr          &params
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord cold.
    CSMap::CS_stncp(csDef.prj_knm, CS_MILLR, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.org_lng;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            ModifiedPolyconicToCsDef        // COORDSYS_MODPC
(
CSDefinition        &csDef,
Proj_modpc          &params
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord cold.
    CSMap::CS_stncp(csDef.prj_knm, CS_MODPC, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.org_lng;
    csDef.prj_prm2  = params.lng_1;
    csDef.prj_prm3  = params.lat_1;
    csDef.prj_prm4  = params.lat_2;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            MollweideToCsDef                // COORDSYS_MOLWD
(
CSDefinition        &csDef,
Proj_molwd          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_MOLWD, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    ZonesToCsDef(&csDef.prj_prm1, params.zone, params.nZones);
    csDef.org_lng   = params.org_lng;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            MercatorToCsDef                 // COORDSYS_MRCAT
(
CSDefinition        &csDef,
Proj_mrcat          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_MRCAT, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.org_lng;
    csDef.prj_prm2  = params.std_lat;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            MercatorScaleReductionToCsDef   // COORDSYS_MRCSR
(
CSDefinition        &csDef,
Proj_mrcsr          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_MRCSR, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.org_lng;
    csDef.scl_red   = params.scl_red;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            PopularVisualizationMercatorToCsDef                 // COORDSYS_MRCATPV
(
CSDefinition        &csDef,
Proj_mrcatpv        &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_MRCATPV, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.org_lng;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            ModifiedStereographicToCsDef    // COORDSYS_MSTRO
(
CSDefinition        &csDef,
Proj_mstro          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_MSTRO, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.scl_red   = params.scl_red;

    // copy the imaginary numbers to prj_prm1 - prj_prm24
    memcpy(&csDef.prj_prm1, params.ABary, 24*sizeof(double));

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            NormalAspectAuthalicCylindricalToCsDef          // COORDSYS_NACYL
(
CSDefinition        &csDef,
Proj_nacyl          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_NACYL, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.prj_prm1  = params.std_lat;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            NonEarthToCsDef                 // COORDSYS_NERTH
(
CSDefinition        &csDef,
Proj_nerth          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_NERTH, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            NonEarthScaleRotationTranslationToCsDef         // COORDSYS_NESRT
(
CSDefinition        &csDef,
Proj_nesrt          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_NESRT, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.scale;
    csDef.prj_prm2  = params.rotateDeg;
    csDef.prj_prm3  = params.x_origin;
    csDef.prj_prm4  = params.y_origin;

    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            NewZealandToCsDef               // COORDSYS_NZLND
(
CSDefinition        &csDef,
Proj_nzlnd          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_NZLND, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.scl_red   = 1.0;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            OrthographicToCsDef             // COORDSYS_ORTHO
(
CSDefinition        &csDef,
Proj_ortho          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_ORTHO, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            StereographicToCsDef            // COORDSYS_OSTRO, COORDSYS_PSTRO, COORDSYS_STERO
(
CSDefinition        &csDef,
Proj_stero          &params,
CharCP            csMapCoordName
)
    {
    CSMap::CS_stncp(csDef.prj_knm, csMapCoordName, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.scl_red   = params.scl_red;
    csDef.prj_prm1  = params.az;            // not always used, but this method is used by multiple coordinate systems.

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            Ost97ToCsDef            // COORDSYS_OST97
(
CSDefinition        &csDef,
Proj_ost97          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_OST97, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);
    csDef.map_scl   = params.paper_scl;
    csDef.quad      = (short) params.quad;

    // In this projection, everything is hardcoded yet upon creation of the CSMAP structure (csloc1) it requires at the very least
    // the unit name and either the datum name or ellipsoid name. Since none is usually stored we set these to the
    // hard coded values. Take note also that previous version of GCS system (GeoGraphics/Map) did store this information so for
    // backward compatibility we should probably even though not useful (see Ost97FromCsDef)
    if (strlen(csDef.dat_knm) == 0)
        CSMap::CS_stncp(csDef.dat_knm, "WGS84", DIM (csDef.dat_knm));
    if (strlen(csDef.elp_knm) == 0)
        CSMap::CS_stncp(csDef.elp_knm, "WGS84", DIM (csDef.elp_knm));
    if (strlen(csDef.unit) == 0)
        CSMap::CS_stncp(csDef.unit, "METER", DIM (csDef.unit));
    if (0 == csDef.map_scl)
        csDef.map_scl = 1.0;
    if (0 == csDef.quad)
        csDef.quad = 1;

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            Ost02ToCsDef            // COORDSYS_OST02
(
CSDefinition        &csDef,
Proj_ost97          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_OST02, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);
    csDef.map_scl   = params.paper_scl;
    csDef.quad      = (short) params.quad;

    // In this projection, everything is hardcoded yet upon creation of the CSMAP structure (csloc1) it requires at the very least
    // the unit name and either the datum name or ellipsoid name. Since none is usually stored we set these to the
    // hard coded values. Take note also that previous version of GCS system (GeoGraphics/Map) did store this information so for
    // backward compatibility we should probably even though not useful(see Ost02FromCsDef)
    if (strlen(csDef.dat_knm) == 0)
        CSMap::CS_stncp(csDef.dat_knm, "WGS84", DIM (csDef.dat_knm));
    if (strlen(csDef.elp_knm) == 0)
        CSMap::CS_stncp(csDef.elp_knm, "WGS84", DIM (csDef.elp_knm));
    if (strlen(csDef.unit) == 0)
        CSMap::CS_stncp(csDef.unit, "METER", DIM (csDef.unit));
    if (0 == csDef.map_scl)
        csDef.map_scl = 1.0;
    if (0 == csDef.quad)
        csDef.quad = 1;

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            Ost15ToCsDef            // COORDSYS_OST15
(
CSDefinition        &csDef,
Proj_ost97          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_OST15, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);
    csDef.map_scl   = params.paper_scl;
    csDef.quad      = (short) params.quad;

    // In this projection, everything is hardcoded yet upon creation of the CSMAP structure (csloc1) it requires at the very least
    // the unit name and either the datum name or ellipsoid name. Since none is usually stored we set these to the
    // hard coded values. Take note also that previous version of GCS system (GeoGraphics/Map) did store this information so for
    // backward compatibility we should probably even though not useful(see Ost15FromCsDef)
    if (strlen(csDef.dat_knm) == 0)
        CSMap::CS_stncp(csDef.dat_knm, "WGS84", DIM (csDef.dat_knm));
    if (strlen(csDef.elp_knm) == 0)
        CSMap::CS_stncp(csDef.elp_knm, "WGS84", DIM (csDef.elp_knm));
    if (strlen(csDef.unit) == 0)
        CSMap::CS_stncp(csDef.unit, "METER", DIM (csDef.unit));
    if (0 == csDef.map_scl)
        csDef.map_scl = 1.0;
    if (0 == csDef.quad)
        csDef.quad = 1;

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            AmericanPolyconicToCsDef        // COORDSYS_PLYCN
(
CSDefinition        &csDef,
Proj_plycn          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_PLYCN, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.org_lng;
    csDef.org_lat   = params.org_lat;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            PolarStereographicStandardLatToCsDef            // COORDSYS_PSTSL:
(
CSDefinition        &csDef,
Proj_pstsl          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_PSTSL, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.prj_prm1  = params.std_lat;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            RobinsonToCsDef                 // COORDSYS_ROBIN
(
CSDefinition        &csDef,
Proj_robin          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_ROBIN, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            SinusoidalToCsDef               // COORDSYS_SINUS
(
CSDefinition        &csDef,
Proj_sinus          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_SINUS, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    ZonesToCsDef(&csDef.prj_prm1, params.zone, params.nZones);
    csDef.org_lng   = params.org_lng;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            SwissToCsDef                    // COORDSYS_SWISS
(
CSDefinition        &csDef,
Proj_swiss          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_SWISS, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            Danish34ToCsDef                  // COORDSYS_SYS34, COORDSYS_S3499, COORDSYS_S3401
(
CSDefinition        &csDef,
Proj_sys34          &params,
CharCP            csMapCoordName
)
    {
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.prj_knm, csMapCoordName, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.prj_prm1  = params.zoneNo;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            TransverseAuthalicCylindricalToCsDef            // COORDSYS_TACYL
(
CSDefinition        &csDef,
Proj_tacyl          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_TACYL, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.scl_red   = params.scl_red;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            VanDerGrintenToCsDef            // COORDSYS_VDGRN
(
CSDefinition        &csDef,
Proj_vdgrn          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_VDGRN, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            WinkelTripelToCsDef             // COORDSYS_WINKT
(
CSDefinition        &csDef,
Proj_winkt          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_WINKT, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef(csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.prj_prm1  = params.std_lat;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            GeographicToCsDef               // COORDSYS_Geographic
(
CSDefinition        &csDef,
Proj_unity          &params
)
    {
    CSMap::CS_stncp(csDef.prj_knm, CS_UNITY, DIM (csDef.prj_knm));
    CSMap::CS_stncp(csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp(csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp(csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));

    csDef.ll_min[0] = params.domLL.min_lng;
    csDef.ll_min[1] = params.domLL.min_lat;
    csDef.ll_max[0] = params.domLL.max_lng;
    csDef.ll_max[1] = params.domLL.max_lat;

    csDef.xy_min[0] = params.domEN.min_x;
    csDef.xy_min[1] = params.domEN.min_y;
    csDef.xy_max[0] = params.domEN.max_x;
    csDef.xy_max[1] = params.domEN.max_y;

    csDef.org_lng   = params.gwo_lng;
    csDef.org_lat   = params.gwo_lat;   // CS Map document makes no mention of it - from old code.
    csDef.prj_prm1  = params.user_min;
    csDef.prj_prm2  = params.user_max;

    csDef.map_scl   = params.paper_scl;
    csDef.quad      = (short) params.quad;

#if defined (NEEDSWORK_UNITY)
    // in the old code - need to figure out what it did.
    if (0 != BeStringUtilties::Stricmp(pCsDef->unit, STRING_DEGREE))
        {
        /* CSMap structures carry range in units of the coordinate system.  Convert it if necessary */
        status = ucrdConvert_distanceDegreesToAngular(&pCsDef->prj_prm1, pCsDef->prj_prm1, pCsDef->unit);
        status = ucrdConvert_distanceDegreesToAngular(&pCsDef->prj_prm2, pCsDef->prj_prm2, pCsDef->unit);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProjectionParamsToCSDef
(
CSDefinition        &csDef,
double              &paperScale,
int32_t             projectionType,
ProjectionParams    &params
)
    {
    switch (projectionType)
        {
        case COORDSYS_ALBER:    // Albers equal area projection
            AlbersToCsDef(csDef, params.alber);
            break;

        case COORDSYS_AZEDE:    // Azimuthal Equidistant Projection with Elevated Ellipsoid.
            AzimuthalEquidistantElevatedToCsDef(csDef, params.azede);
            break;

        case COORDSYS_AZMEA:    // Azimuthal Equal Area Projection.
            AzimuthalEqualAreaToCsDef(csDef, params.azmea);
            break;

        case COORDSYS_AZMED:    // Azimuthal Equidistant Projection.
            AzimuthalEquidistantToCsDef(csDef, params.azmed);
            break;

        case COORDSYS_BONNE:    // Bonne Projection
            BonneToCsDef(csDef, params.bonne);
            break;

        case COORDSYS_BPCNC:    // Bipolar Oblique Conic Projection
            BipolarObliqueConformalConicToCsDef(csDef, params.bpcnc);
            break;

        case COORDSYS_CSINI:    // Cassini Cylindrical Projection
            CassiniToCsDef(csDef, params.csini);
            break;

        case COORDSYS_EDCNC:    // Equidistant Conic Projection
            EquidistantConicToCsDef(csDef, params.edcnc);
            break;

        case COORDSYS_EDCYL:    // Equidistant Cylindrical Projection
            EquidistantCylindricalToCsDef(csDef, params.edcyl, CS_EDCYL);
            break;

        case COORDSYS_EKRT4:    // Eckert IV Projection
            Eckert4ToCsDef(csDef, params.ekrt4);
            break;

        case COORDSYS_EKRT6:    // Eckert VI Projection
            Eckert6ToCsDef(csDef, params.ekrt6);
            break;

        case COORDSYS_GAUSK:    // Gauss Kruger Projection (Transvers Mercator variant)
            TransverseMercatorToCsDef(csDef, params.trmer, CS_GAUSK);
            break;

        case COORDSYS_GNOMC:    // Gnomic Projection
            GnomicToCsDef(csDef, params.gnomc);
            break;

        case COORDSYS_HMLSN:    // Homolsine Projection
            HomolsineToCsDef(csDef, params.hmlsn);
            break;

        case COORDSYS_HOM1U:    // Hotine Oblique Mercator 1 Point Unrectified Projection (CSMap says rarely used).
        case COORDSYS_OBLQ1:    // CSMap calls it HOM1XY - Alask Variation, Rectified
            ObliqueMercator1PointToCsDef(csDef, params.oblq1, (COORDSYS_HOM1U == projectionType) ? CS_HOM1U : CS_OBLQ1);
            break;

        case COORDSYS_HOM2U:    // Hotine Oblique Mercator 1 Point Unrectified Projection (CSMap says rarely used).
        case COORDSYS_OBLQ2:
            ObliqueMercator2PointToCsDef(csDef, params.oblq2, (COORDSYS_HOM1U == projectionType) ? CS_HOM2U : CS_OBLQ2);
            break;

        case COORDSYS_HUEOV:    // Hungarian EOV Projection.
            HungarianEOVToCsDef(csDef, params.hueov);
            break;

        case COORDSYS_KRVKG:    // Krovak Oblique Conformal Conic
            KrovakToCsDef(csDef, params.krvkg, CS_KRVKG);
            break;
        case COORDSYS_KRVKP:
            KrovakToCsDef(csDef, params.krvkg, CS_KRVKP);
            break;
        case COORDSYS_KRVKR:
            KrovakToCsDef(csDef, params.krvkg, CS_KRVKR);
            break;

        case COORDSYS_LM1SP:    // Lambert Tangential Projection
        case COORDSYS_LMTAN:
            LambertTangentialToCsDef(csDef, params.lmtan, (COORDSYS_LM1SP == projectionType) ? CS_LM1SP : CS_LMTAN);
            break;

        case COORDSYS_LMBRT:    // Lambert Conformal Conic Projection
        case COORDSYS_LMBLG:    // Belgium variation
            LambertConformalConicToCsDef(csDef, params.lmbrt, (COORDSYS_LMBRT == projectionType) ? CS_LMBRT : CS_LMBLG);;
            break;

        case COORDSYS_LMMIN:    // Lambert Conformal Conic Projection, Minnesota variation
            LambertConformalConicMinnesotaToCsDef(csDef, params.lmmin);
            break;

        case COORDSYS_LMWIS:    // Lambert Conformal Conic Projection, Minnesota variation
            LambertConformalConicWisconsinToCsDef(csDef, params.lmwis);
            break;

        case COORDSYS_MILLR:    // Miller Projection
            MillerToCsDef(csDef, params.millr);
            break;

        case COORDSYS_MODPC:    // Modified Polyconic Projection
            ModifiedPolyconicToCsDef(csDef, params.modpc);
            break;

        case COORDSYS_MOLWD:    // Mollweide Projection
            MollweideToCsDef(csDef, params.molwd);
            break;

        case COORDSYS_MRCAT:    // Mercator Projection
            MercatorToCsDef(csDef, params.mrcat);
            break;

        case COORDSYS_MRCSR:    // Mercator Projection with Scale Reduction
            MercatorScaleReductionToCsDef(csDef, params.mrcsr);
            break;

        case COORDSYS_MSTRO:    // Modified Stereographic Projection
            ModifiedStereographicToCsDef(csDef, params.mstro);
            break;

        case COORDSYS_NACYL:    // Normal Aspect Authalic Cylindrical Projection
            NormalAspectAuthalicCylindricalToCsDef(csDef, params.nacyl);
            break;

        case COORDSYS_NERTH:    // Non-earth Projection
            NonEarthToCsDef(csDef, params.nerth);
            break;

        case COORDSYS_NESRT:    // Non-earth with scale, rotation, and translation
            NonEarthScaleRotationTranslationToCsDef(csDef, params.nesrt);
            break;

        case COORDSYS_NZLND:    // New Zealand Projection
            NewZealandToCsDef(csDef, params.nzlnd);
            break;

        case COORDSYS_ORTHO:    // Orthographic Projection
            OrthographicToCsDef(csDef, params.ortho);
            break;

        case COORDSYS_OSTRO:    // Oblique Stereographic Projection
            StereographicToCsDef(csDef, params.ostro, CS_OSTRO);
            break;

        case COORDSYS_PSTRO:    // Polar Sterographic Projection
            StereographicToCsDef(csDef, params.ostro, CS_PSTRO);
            break;

        case COORDSYS_OST97:    // Ordinance Survey Grid Transformation of 1997
            // This is a variation of Transverse Mercator (see csmap documentation) but there are no parameters - everything is hard coded.
            // yet for backward compatibility and due to a csmap issue we must still store as much as possible (datum, unit, ellipsoid...)
            Ost97ToCsDef(csDef, params.ost97);
            break;

        case COORDSYS_OST02:    // Ordinance Survey Grid Transformation of 2002
            // This is a variation of Transverse Mercator (see csmap documentation) but there are no parameters - everything is hard coded.
            // yet for backward compatibility and due to a csmap issue we must still store as much as possible (datum, unit, ellipsoid...)
            Ost02ToCsDef(csDef, params.ost97);
            break;

        case COORDSYS_OST15:    // Ordinance Survey Grid Transformation of 2015
            // This is a variation of Transverse Mercator (see csmap documentation) but there are no parameters - everything is hard coded.
            // yet for backward compatibility and due to a csmap issue we must still store as much as possible (datum, unit, ellipsoid...)
            Ost15ToCsDef(csDef, params.ost97);
            break;

        case COORDSYS_PLYCN:    // American Polyconic Projection
            AmericanPolyconicToCsDef(csDef, params.plycn);
            break;

        case COORDSYS_PSTSL:    // Polar Stereographic with Standard Latitude
            PolarStereographicStandardLatToCsDef(csDef, params.pstsl);
            break;

        case COORDSYS_ROBIN:    // Robinson Projection
            RobinsonToCsDef(csDef, params.robin);
            break;

        case COORDSYS_RSKEW:    // Rectified Skew Orthomorphic
            ObliqueMercator1PointToCsDef(csDef, params.oblq1, CS_RSKEW);
            break;

        case COORDSYS_RSKWC:    // Rectified Skew Orthomorphic Centered
            ObliqueMercator1PointToCsDef(csDef, params.oblq1, CS_RSKWC);
            break;

        case COORDSYS_RSKWO:    // Rectified Skew Orthomorphic Origin
            ObliqueMercator1PointToCsDef(csDef, params.oblq1, CS_RSKWO);
            break;

        case COORDSYS_SINUS:    // Sinusoidal Projection
            SinusoidalToCsDef(csDef, params.sinus);
            break;

        case COORDSYS_SOTRM:    // Transverse Mercator South Oriented Projection
            TransverseMercatorToCsDef(csDef, params.trmer, CS_SOTRM);
            break;

        case COORDSYS_STERO:    // Stereographic Projection
            StereographicToCsDef(csDef, params.stero, CS_STERO);
            break;


        case COORDSYS_SWISS:    // Oblique CylindricalSwiss Projection
            SwissToCsDef(csDef, params.swiss);
            break;

        case COORDSYS_SYS34:    // Danish System 34
        case COORDSYS_S3499:    // Danish System 34 with 1999 Polynomial.
        case COORDSYS_S3401:    // Danish System 34 with 2001 Polynomial.
            Danish34ToCsDef(csDef, params.sys34, (COORDSYS_SYS34 == projectionType) ? CS_SYS34 : ((COORDSYS_S3499 == projectionType) ? CS_S3499 : CS_S3401));
            break;

        case COORDSYS_TACYL:    // Transverse Authalic Cylindrical Projection
            TransverseAuthalicCylindricalToCsDef(csDef, params.tacyl);
            break;

        case COORDSYS_TMMIN:    // Transverse Mercator, Minnesota
            TransverseMercatorMinnesotaToCsDef(csDef, params.tmmin);
            break;

        case COORDSYS_TMWIS:    // Transverse Mercator, Wisconsin County
            TransverseMercatorWisconsinToCsDef(csDef, params.tmwis);
            break;

        case COORDSYS_TRMAF:    // Transverse Mercator with Affine Post Process
            TransverseMercatorAffinePostProcessToCsDef(csDef, params.trmaf);
            break;

        case COORDSYS_TRMER:    // Transverse Mercator Projection
            TransverseMercatorToCsDef(csDef, params.trmer, CS_TRMER);
            break;

        case COORDSYS_TRMRS:    // Transverse Mercator Projection
            TransverseMercatorToCsDef(csDef, params.trmrs, CS_TRMRS);
            break;

        case COORDSYS_TMKRG:    // Transverse Mercator Projection
            TransverseMercatorToCsDef(csDef, params.tmkrg, CS_TMKRG);
            break;

        case COORDSYS_UTMZN:    // Universal Tranverse Mercator Zone Projections
            UniversalTransverseMercatorZoneToCsDef(csDef, params.utmzn, CS_UTMZN);
            break;

        case COORDSYS_Geographic:   // Unity (Lat/Long) pseudo-projection.
            GeographicToCsDef(csDef, params.unity);
            break;

        case COORDSYS_VDGRN:        // Van der Grinten Projection
            VanDerGrintenToCsDef(csDef, params.vdgrn);
            break;

        case COORDSYS_WINKT:        // Winkel-Tripel Projection
            WinkelTripelToCsDef(csDef, params.winkt);
            break;

        case COORDSYS_LMBRTAF:      // Lambert with Affine Post Processor
            LambertAffinePostProcessToCsDef(csDef, params.lmbrtaf);
            break;

        // TOTAL_SPECIAL
        case COORDSYS_UTMZNBF:     // UTM Zone, using TOTal BF calculation
            UniversalTransverseMercatorZoneToCsDef(csDef, params.utmzn, CS_UTMZNBF);
            break;

        case COORDSYS_TRMERBF:     // Transverse Mercator, using TOTAL BF calculation
            TransverseMercatorToCsDef(csDef, params.trmer, CS_TRMERBF);
            break;

        case COORDSYS_EDCYLE:     // Equidistant Cylindrical Ellipsoid or Spherical form
            EquidistantCylindricalToCsDef(csDef, params.edcyl, CS_EDCYLE);
            break;

        case COORDSYS_PCARREE:     // Simple Cylindrical / Plate Carree
            PlateCarreeToCsDef(csDef, params.pcarree);
            break;

        case COORDSYS_MRCATPV:    // Popular Visualization Pseudo Mercator Projection
            PopularVisualizationMercatorToCsDef(csDef, params.mrcatpv);
            break;

       case COORDSYS_MNDOTOBL:    // Minnesota DOT Oblique Mercator
            MinnesotaDOTObliqueMercatorToCsDef(csDef, params.mndotobl);
            break;

        case COORDSYS_KRVK95:    // Krovak Oblique Conformal Conic 1995
            KrovakToCsDef(csDef, params.krvkg, CS_KRVK95);
            break;

       case COORDSYS_LMMICH:    // Lambert Conformal Conic Projection, Michigan variation
            LambertConformalConicMichiganToCsDef(csDef, params.lmmich);
            break;

        case COORDSYS_KRVMO:    // Krovak Modified Oblique Conformal Conic
            KrovakToCsDef(csDef, params.krvkg, CS_KRVMO);
            break;

        default:
            return ERROR;
        }

    // this was the tolerance used in the old code.
    if (fabs(csDef.map_scl) <= GEOCOORD_CLOSE_TOLERANCE)
        csDef.map_scl = 1.0;

    // we do not want to send map_scl to CSMap. We'll handle it internally. We set it to 1.0 in csDef, and we'll store it as a member of DgnGCS.
    paperScale      = csDef.map_scl;
    csDef.map_scl   = 1.0;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
public:
StatusInt       ConvertToCSDef
(
CSDefinition        &csDef,
double              &paperScale,
GeoCoordType66      &extracted,
ProjectionParams    &projectionParams
)
    {
    // convert the type 66 and projectionParams to a CSDefinition/CSDatum pair that can be made into CSParameters.
    memset((void*)&csDef, 0, sizeof(csDef));

    // get the strings from the type 66.
    CommonParamsToCSDef(csDef, extracted);
    ProjectionParamsToCSDef(csDef, paperScale, extracted.projType, projectionParams);

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
private:
void            CommonParamsFromCsDef
(
GeoCoordType66      &type66,
const CSDefinition  &csDef
)
    {
    type66.protect = csDef.protect;

    CSMap::CS_stncp(type66.grp_knm, csDef.group,   DIM(type66.grp_knm));
    CSMap::CS_stncp(type66.prj_knm, csDef.prj_knm, DIM(type66.prj_knm));
    CSMap::CS_stncp(type66.source,  csDef.source,  DIM(type66.source));
    CSMap::CS_stncp(type66.desc_nm, csDef.desc_nm, DIM(type66.desc_nm));
    type66.epsgNbr = csDef.epsgNbr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            DomainFromCsDef
(
GCDomain            &domain,
const CSDefinition  &csDef
)
    {
    domain.domLL.min_lng = csDef.ll_min[0];
    domain.domLL.min_lat = csDef.ll_min[1];
    domain.domLL.max_lng = csDef.ll_max[0];
    domain.domLL.max_lat = csDef.ll_max[1];

    domain.domEN.min_x   = csDef.xy_min[0];
    domain.domEN.min_y   = csDef.xy_min[1];
    domain.domEN.max_x   = csDef.xy_max[0];
    domain.domEN.max_y   = csDef.xy_max[1];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            SaveEllipsoidName(CharP ellipsoidName, const CSDefinition& csDef, size_t charCount)
    {
    if ( (0 != csDef.elp_knm[0]) || (NULL == m_datum) )
        CSMap::CS_stncp(ellipsoidName, csDef.elp_knm, (int)charCount);
    else
        CSMap::CS_stncp(ellipsoidName, m_datum->ell_knm, (int)charCount);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            AlbersFromCsDef
(
Proj_alber          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));

    DomainFromCsDef(params.gcDom, csDef);

    params.lat_1        = csDef.prj_prm1;
    params.lat_2        = csDef.prj_prm2;
    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            AzimuthalEquidistantElevatedFromCsDef
(
Proj_azede          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.az           = csDef.prj_prm1;
    params.ellElev      = csDef.prj_prm2;
    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            AzimuthalEqualAreaFromCsDef
(
Proj_azmea          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.az           = csDef.prj_prm1;
    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            AzimuthalEquidistantFromCsDef
(
Proj_azmed          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.az           = csDef.prj_prm1;
    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            BonneFromCsDef
(
Proj_bonne          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            BipolarObliqueConformalConicFromCsDef
(
Proj_bpcnc          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.lng_1        = csDef.prj_prm1;
    params.lat_1        = csDef.prj_prm2;
    params.lng_2        = csDef.prj_prm3;
    params.lat_2        = csDef.prj_prm4;
    params.pole_dd      = csDef.prj_prm5;
    params.sp_1         = csDef.prj_prm6;
    params.sp_2         = csDef.prj_prm7;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            CassiniFromCsDef
(
Proj_csini          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.prj_prm1;
    params.org_lat      = csDef.org_lat;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            EquidistantConicFromCsDef
(
Proj_edcnc          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.lat_1        = csDef.prj_prm1;
    params.lat_2        = csDef.prj_prm2;
    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            EquidistantCylindricalFromCsDef
(
Proj_edcyl          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.std_lat      = csDef.prj_prm1;
    params.org_lat      = csDef.org_lat;
    params.org_lng      = csDef.org_lng;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            Eckert4FromCsDef
(
Proj_ekrt4          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.prj_prm1;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            Eckert6FromCsDef
(
Proj_ekrt6          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.prj_prm1;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            TransverseMercatorFromCsDef
(
Proj_trmer          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    SaveEllipsoidName(params.ell_knm, csDef, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.prj_prm1;
    params.org_lat      = csDef.org_lat;
    params.scl_red      = csDef.scl_red;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            TransverseMercatorMinnesotaFromCsDef
(
Proj_tmmin          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.prj_prm1;
    params.ellElev      = csDef.prj_prm2;
    params.org_lat      = csDef.org_lat;
    params.scl_red      = csDef.scl_red;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            TransverseMercatorWisconsinFromCsDef
(
Proj_tmwis          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.prj_prm1;
    params.geoidSep     = csDef.prj_prm2;
    params.geoidElev    = csDef.prj_prm3;
    params.org_lat      = csDef.org_lat;
    params.scl_red      = csDef.scl_red;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            TransverseMercatorAffinePostProcessFromCsDef
(
Proj_trmaf          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.prj_prm1;
    // NOTE: The CSMap documentation has them ordered A0,B0,A1,A2,B1,B2, and the code looks like that is exactly what they expect.
    //       However, the (stupid) old code labelled them A0,A1,A2,B0,B1,B2, and stored and restored them as if they were in that
    //       order. The member name of the structure of course doesn't really matter as long as they are stored and restored in
    //       consistent order, so I stuck with the (stupid) old code member names and order.
    params.affineA0     = csDef.prj_prm2;
    params.affineA1     = csDef.prj_prm3;
    params.affineA2     = csDef.prj_prm4;
    params.affineB0     = csDef.prj_prm5;
    params.affineB1     = csDef.prj_prm6;
    params.affineB2     = csDef.prj_prm7;
    params.org_lat      = csDef.org_lat;
    params.scl_red      = csDef.scl_red;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            UniversalTransverseMercatorZoneFromCsDef
(
Proj_utmzn          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.zoneNo       =  (int32_t) csDef.prj_prm1;
    params.hemisphere   =  (int32_t) (0 > csDef.prj_prm2 ?  OBE_HEMISPHERE_SOUTH : OBE_HEMISPHERE_NORTH);


    params.paper_scl    =  csDef.map_scl;
    params.quad         =  csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            GnomicFromCsDef
(
Proj_gnomc          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            ZonesFromCsDef
(
ZONE                *zone,
int32_t             *numZones,
const double        *src
)
    {
    // set up the zones. NOTE: This did not appear to be supported in the old geocoordinate stuff.
    // I am going by the comments in the ZONE typedef, probably added by TI Doug Bilinski.
    for (*numZones = 0; *numZones < NUM_ZONE; ++*numZones, ++zone, src+=3)
        {
        // zero in the first and third positions indicates no more zones.
        if ( (0.0 == *src) && (0.0 == *(src+2)) )
            break;

        if (*src > 3000.0)
            {
            zone->north_south = 0.0;
            zone->west_bndry = *src - 3000.0;
            }
        else if (*src > 2000.0)
            {
            zone->north_south = -1.0;
            zone->west_bndry = *src - 2000.0;
            }
        else if (*src > 1000.0)
            {
            zone->north_south = 1.0;
            zone->west_bndry = *src - 1000.0;
            }
        else
            {
            zone->north_south = 0.0;
            zone->west_bndry = *src;
            }

        zone->cntrl_mer  = *(src+1);
        zone->east_bndry = *(src+2);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            HomolsineFromCsDef
(
Proj_hmlsn          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    ZonesFromCsDef(params.zone, &params.nZones, &csDef.prj_prm1);
    params.org_lng      = csDef.org_lng;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            ObliqueMercator1PointFromCsDef
(
Proj_Oblq1          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.lng_c        = csDef.prj_prm1;
    params.lat_c        = csDef.prj_prm2;
    params.az           = csDef.prj_prm3;
    params.scl_red      = csDef.scl_red;    // CSMap documentation does not mention using it.

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            ObliqueMercator2PointFromCsDef
(
Proj_Oblq2          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.lng_1        = csDef.prj_prm1;
    params.lat_1        = csDef.prj_prm2;
    params.lng_2        = csDef.prj_prm3;
    params.lat_2        = csDef.prj_prm4;
    params.org_lat      = csDef.org_lat;
    params.scl_red      = csDef.scl_red; // CSMap documentation does not mention using it.

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            MinnesotaDOTObliqueMercatorFromCsDef
(
Proj_Mndotobl       &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.lng_c        = csDef.prj_prm1;
    params.lat_c        = csDef.prj_prm2;
    params.az           = csDef.prj_prm3;
    params.ellElev      = csDef.prj_prm4;
    params.scl_red      = csDef.scl_red;    // CSMap documentation does not mention using it.

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            HungarianEOVFromCsDef
(
Proj_Hueov          &params,
const CSDefinition  &csDef
)
    {
    // No CSMap documentation, looking at old geocoord stuff.
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;
    params.std_lat      = csDef.prj_prm1;
    params.scl_red      = csDef.scl_red;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            KrovakFromCsDef
(
Proj_Krvak          &params,
const CSDefinition  &csDef
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord cold.
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.pole_lng  = csDef.prj_prm1;
    params.pole_lat  = csDef.prj_prm2;
    params.std_lat   = csDef.prj_prm3;
    params.scl_red   = csDef.scl_red;
    params.org_lng   = csDef.org_lng;
    params.org_lat   = csDef.org_lat;

    params.paper_scl = csDef.map_scl;
    params.x_off     = csDef.x_off;
    params.y_off     = csDef.y_off;
    params.quad      = csDef.quad;

    if (0 == strcmp(csDef.prj_knm, CS_KRVK95))
        params.apply95 = 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            LambertTangentialFromCsDef
(
Proj_Lmtan          &params,
const CSDefinition  &csDef
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord cold.
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;
    params.scl_red      = csDef.scl_red;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            LambertConformalConicFromCsDef
(
Proj_Lmbrt          &params,
const CSDefinition  &csDef
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord cold.
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.lat_1        = csDef.prj_prm1;
    params.lat_2        = csDef.prj_prm2;
    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            LambertConformalConicMinnesotaFromCsDef
(
Proj_Lmmin          &params,
const CSDefinition  &csDef
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord code.
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.lat_1        = csDef.prj_prm1;
    params.lat_2        = csDef.prj_prm2;
    params.ellElev      = csDef.prj_prm3;
    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            LambertConformalConicWisconsinFromCsDef
(
Proj_Lmwis          &params,
const CSDefinition  &csDef
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord code.
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.lat_1        = csDef.prj_prm1;
    params.lat_2        = csDef.prj_prm2;
    params.geoidSep     = csDef.prj_prm3;
    params.geoidElev    = csDef.prj_prm4;
    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            LambertConformalConicMichiganFromCsDef
(
Proj_Lmmich         &params,
const CSDefinition  &csDef
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord code.
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.lat_1        = csDef.prj_prm1;
    params.lat_2        = csDef.prj_prm2;
    params.ell_scale    = csDef.prj_prm3;
    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            MillerFromCsDef
(
Proj_millr          &params,
const CSDefinition  &csDef
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord cold.
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.prj_prm1;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            ModifiedPolyconicFromCsDef
(
Proj_modpc          &params,
const CSDefinition  &csDef
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord cold.
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.prj_prm1;
    params.lng_1        = csDef.prj_prm2;
    params.lat_1        = csDef.prj_prm3;
    params.lat_2        = csDef.prj_prm4;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            MollweideFromCsDef
(
Proj_molwd          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    ZonesFromCsDef(params.zone, &params.nZones, &csDef.prj_prm1);
    params.org_lng      = csDef.org_lng;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            MercatorFromCsDef
(
Proj_mrcat          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.prj_prm1;
    params.std_lat      = csDef.prj_prm2;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            MercatorScaleReductionFromCsDef
(
Proj_mrcsr          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.prj_prm1;
    params.scl_red      = csDef.scl_red;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            ModifiedStereographicFromCsDef
(
Proj_mstro          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;
    params.scl_red      = csDef.scl_red;

    // copy the imaginary numbers to prj_prm1 - prj_prm24
    memcpy(params.ABary, &csDef.prj_prm1, 24*sizeof(double));

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            NormalAspectAuthalicCylindricalFromCsDef
(
Proj_nacyl          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.org_lng;
    params.std_lat      = csDef.prj_prm1;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            NonEarthFromCsDef
(
Proj_nerth          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            NonEarthScaleRotationTranslationFromCsDef
(
Proj_nesrt          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.scale        = csDef.prj_prm1;
    params.rotateDeg    = csDef.prj_prm2;
    params.x_origin     = csDef.prj_prm3;
    params.y_origin     = csDef.prj_prm4;

    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            NewZealandFromCsDef
(
Proj_nzlnd          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng       = csDef.org_lng;
    params.org_lat       = csDef.org_lat;

    params.paper_scl     = csDef.map_scl;
    params.x_off         = csDef.x_off;
    params.y_off         = csDef.y_off;
    params.quad          = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            OrthographicFromCsDef           // COORDSYS_ORTHO
(
Proj_ortho          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng       = csDef.org_lng;
    params.org_lat       = csDef.org_lat;

    params.paper_scl     = csDef.map_scl;
    params.x_off         = csDef.x_off;
    params.y_off         = csDef.y_off;
    params.quad          = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            StereographicFromCsDef
(
Proj_stero          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;
    params.scl_red      = csDef.scl_red;
    params.az           = csDef.prj_prm1;   // not always used, but this method is used by multiple coordinate systems.

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            Ost97FromCsDef
(
Proj_ost97          &params,
const CSDefinition  &csDef
)
    {
    // All parameters are hard-coded ... we only save as this information since it is required for
    // recreation of the GCS upon reload (CSMAP issue) and to reload the DGN into Map XM or GeoGraphics

    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.paper_scl    = csDef.map_scl;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            Ost02FromCsDef
(
Proj_ost97          &params,
const CSDefinition  &csDef
)
    {
    // All parameters are hard-coded ... we only save as this information since it is required for
    // recreation of the GCS upon reload (CSMAP issue) and to reload the DGN into Map XM or GeoGraphics

    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.paper_scl    = csDef.map_scl;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            Ost15FromCsDef
(
Proj_ost97          &params,
const CSDefinition  &csDef
)
    {
    // All parameters are hard-coded ... we only save as this information since it is required for
    // recreation of the GCS upon reload (CSMAP issue) and to reload the DGN into Map XM or GeoGraphics

    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.paper_scl    = csDef.map_scl;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            AmericanPolyconicFromCsDef
(
Proj_plycn          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.prj_prm1;
    params.org_lat      = csDef.org_lat;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            PolarStereographicStandardLatFromCsDef
(
Proj_pstsl          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;
    params.std_lat      = csDef.prj_prm1;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            RobinsonFromCsDef
(
Proj_robin          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.org_lng;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            SinusoidalFromCsDef
(
Proj_sinus          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    ZonesFromCsDef(params.zone, &params.nZones, &csDef.prj_prm1);
    params.org_lng      = csDef.org_lng;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            SwissFromCsDef
(
Proj_swiss          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng       = csDef.org_lng;
    params.org_lat       = csDef.org_lat;

    params.paper_scl     = csDef.map_scl;
    params.x_off         = csDef.x_off;
    params.y_off         = csDef.y_off;
    params.quad          = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            Danish34FromCsDef
(
Proj_sys34          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.zoneNo       = (int32_t) csDef.prj_prm1;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            TransverseAuthalicCylindricalFromCsDef
(
Proj_tacyl          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;
    params.scl_red      = csDef.scl_red;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            VanDerGrintenFromCsDef
(
Proj_vdgrn          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.org_lng;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            WinkelTripelFromCsDef
(
Proj_winkt          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.org_lng;
    params.std_lat      = csDef.prj_prm1;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            LambertAffinePostProcessFromCsDef
(
Proj_Lmbrtaf        &params,
const CSDefinition  &csDef
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord cold.
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.lat_1        = csDef.prj_prm1;
    params.lat_2        = csDef.prj_prm2;
    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;

    // this seemingly random order is what cs_map uses.
    params.affineA0     = csDef.prj_prm3;
    params.affineB0     = csDef.prj_prm4;
    params.affineA1     = csDef.prj_prm5;
    params.affineA2     = csDef.prj_prm6;
    params.affineB1     = csDef.prj_prm7;
    params.affineB2     = csDef.prj_prm8;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            PlateCarreeFromCsDef
(
Proj_pcarree        &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lat      = csDef.org_lat;
    params.org_lng      = csDef.org_lng;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            PopularVisualizationMercatorFromCsDef
(
Proj_mrcatpv        &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef(params.gcDom, csDef);

    params.org_lng      = csDef.prj_prm1;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }




/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            GeographicFromCsDef
(
Proj_unity          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp(params.unit_nm, csDef.unit, DIM(params.unit_nm));
    CSMap::CS_stncp(params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp(params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));

    params.domLL.min_lng = csDef.ll_min[0];
    params.domLL.min_lat = csDef.ll_min[1];
    params.domLL.max_lng = csDef.ll_max[0];
    params.domLL.max_lat = csDef.ll_max[1];

    params.domEN.min_x   = csDef.xy_min[0];
    params.domEN.min_y   = csDef.xy_min[1];
    params.domEN.max_x   = csDef.xy_max[0];
    params.domEN.max_y   = csDef.xy_max[1];

    params.gwo_lng      = csDef.org_lng;
    params.gwo_lat      = csDef.org_lat;         // not mentioned as used in the cs_map documentation.

    params.user_min     = csDef.prj_prm1;
    params.user_max     = csDef.prj_prm2;

    params.paper_scl    = csDef.map_scl;
    params.quad         = csDef.quad;

#if defined (NEEDSWORK_UNITY)
    // in the old code - need to figure out what it did.
    if (0 != BeStringUtilties::Stricmp(pCsDef->unit, STRING_DEGREE))
        {
        /* CSMap structures carry range in units of the coordinate system.  Convert it if necessary */
        status = ucrdConvert_distanceDegreesToAngular(&pCsDef->prj_prm1, pCsDef->prj_prm1, pCsDef->unit);
        status = ucrdConvert_distanceDegreesToAngular(&pCsDef->prj_prm2, pCsDef->prj_prm2, pCsDef->unit);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       FormatProjectionParams
(
int32_t             projectionType,
ProjectionParams    &params,
const CSDefinition  &csDef
)
    {
    // make sure it starts out clear.
    memset(&params, 0, sizeof(params));

    switch (projectionType)
        {
        case COORDSYS_ALBER:    // Albers equal area projection
            AlbersFromCsDef(params.alber, csDef);
            break;

        case COORDSYS_AZEDE:    // Azimuthal Equidistant Projection with Elevated Ellipsoid.
            AzimuthalEquidistantElevatedFromCsDef(params.azede, csDef);
            break;

        case COORDSYS_AZMEA:    // Azimuthal Equal Area Projection.
            AzimuthalEqualAreaFromCsDef(params.azmea, csDef);
            break;

        case COORDSYS_AZMED:    // Azimuthal Equidistant Projection.
            AzimuthalEquidistantFromCsDef(params.azmed, csDef);
            break;

        case COORDSYS_BONNE:    // Bonne Projection
            BonneFromCsDef(params.bonne, csDef);
            break;

        case COORDSYS_BPCNC:    // Bipolar Oblique Conic Projection
            BipolarObliqueConformalConicFromCsDef(params.bpcnc, csDef);
            break;

        case COORDSYS_CSINI:    // Cassini Cylindrical Projection
            CassiniFromCsDef(params.csini, csDef);
            break;

        case COORDSYS_EDCNC:    // Equidistant Conic Projection
            EquidistantConicFromCsDef(params.edcnc, csDef);
            break;

        case COORDSYS_EDCYL:    // Equidistant Cylindrical Projection
            EquidistantCylindricalFromCsDef(params.edcyl, csDef);
            break;

        case COORDSYS_EKRT4:    // Eckert IV Projection
            Eckert4FromCsDef(params.ekrt4, csDef);
            break;

        case COORDSYS_EKRT6:    // Eckert VI Projection
            Eckert6FromCsDef(params.ekrt6, csDef);
            break;

        case COORDSYS_GAUSK:    // Gauss Kruger Projection (Transvers Mercator variant)
            TransverseMercatorFromCsDef(params.trmer, csDef);
            break;

        case COORDSYS_GNOMC:    // Gnomic Projection
            GnomicFromCsDef(params.gnomc, csDef);
            break;

        case COORDSYS_HMLSN:    // Homolsine Projection
            HomolsineFromCsDef(params.hmlsn, csDef);
            break;

        case COORDSYS_HOM1U:    // Hotine Oblique Mercator 1 Point Unrectified Projection (CSMap says rarely used).
        case COORDSYS_OBLQ1:    // CSMap calls it HOM1XY - Alask Variation, Rectified
            ObliqueMercator1PointFromCsDef(params.oblq1, csDef);
            break;

        case COORDSYS_HOM2U:    // Hotine Oblique Mercator 1 Point Unrectified Projection (CSMap says rarely used).
        case COORDSYS_OBLQ2:
            ObliqueMercator2PointFromCsDef(params.oblq2, csDef);
            break;

        case COORDSYS_HUEOV:    // Hungarian EOV Projection.
            HungarianEOVFromCsDef(params.hueov, csDef);
            break;

        case COORDSYS_KRVKG:    // Krovak Oblique Conformal Conic
            KrovakFromCsDef(params.krvkg, csDef);
            break;
        case COORDSYS_KRVKP:
            KrovakFromCsDef(params.krvkg, csDef);
            break;
        case COORDSYS_KRVKR:
            KrovakFromCsDef(params.krvkg, csDef);
            break;

        case COORDSYS_LM1SP:    // Lambert Tangential Projection
        case COORDSYS_LMTAN:
            LambertTangentialFromCsDef(params.lmtan, csDef);
            break;

        case COORDSYS_LMBRT:    // Lambert Conformal Conic Projection
        case COORDSYS_LMBLG:    // Belgium variation
            LambertConformalConicFromCsDef(params.lmbrt, csDef);;
            break;

        case COORDSYS_LMMIN:    // Lambert Conformal Conic Projection, Minnesota variation
            LambertConformalConicMinnesotaFromCsDef(params.lmmin, csDef);
            break;

        case COORDSYS_LMWIS:    // Lambert Conformal Conic Projection, Minnesota variation
            LambertConformalConicWisconsinFromCsDef(params.lmwis, csDef);
            break;

        case COORDSYS_MILLR:    // Miller Projection
            MillerFromCsDef(params.millr, csDef);
            break;

        case COORDSYS_MODPC:    // Modified Polyconic Projection
            ModifiedPolyconicFromCsDef(params.modpc, csDef);
            break;

        case COORDSYS_MOLWD:    // Mollweide Projection
            MollweideFromCsDef(params.molwd, csDef);
            break;

        case COORDSYS_MRCAT:    // Mercator Projection
            MercatorFromCsDef(params.mrcat, csDef);
            break;

        case COORDSYS_MRCSR:    // Mercator Projection with Scale Reduction
            MercatorScaleReductionFromCsDef(params.mrcsr, csDef);
            break;

        case COORDSYS_MSTRO:    // Modified Stereographic Projection
            ModifiedStereographicFromCsDef(params.mstro, csDef);
            break;

        case COORDSYS_NACYL:    // Normal Aspect Authalic Cylindrical Projection
            NormalAspectAuthalicCylindricalFromCsDef(params.nacyl, csDef);
            break;

        case COORDSYS_NERTH:    // Non-earth Projection
            NonEarthFromCsDef(params.nerth, csDef);
            break;

        case COORDSYS_NESRT:    // Non-earth with scale, rotation, and translation
            NonEarthScaleRotationTranslationFromCsDef(params.nesrt, csDef);
            break;

        case COORDSYS_NZLND:    // New Zealand Projection
            NewZealandFromCsDef(params.nzlnd, csDef);
            break;

        case COORDSYS_ORTHO:    // Orthographic Projection
            OrthographicFromCsDef(params.ortho, csDef);
            break;

        case COORDSYS_OSTRO:    // Oblique Stereographic Projection
            StereographicFromCsDef(params.ostro, csDef);
            break;

        case COORDSYS_PSTRO:    // Polar Sterographic Projection
            StereographicFromCsDef(params.ostro, csDef);
            break;

        case COORDSYS_OST97:    // Ordinance Survey Grid Transformation of 1997
            Ost97FromCsDef(params.ost97, csDef);
            break;

        case COORDSYS_OST02:    // Ordinance Survey Grid Transformation of 2002
            // This is a variation of Transverse Mercator (see csmap documentation) but there are no parameters - everything is hard coded.
            Ost02FromCsDef(params.ost97, csDef);
            break;

        case COORDSYS_OST15:    // Ordinance Survey Grid Transformation of 2015
            // This is a variation of Transverse Mercator (see csmap documentation) but there are no parameters - everything is hard coded.
            Ost15FromCsDef(params.ost97, csDef);
            break;

        case COORDSYS_PLYCN:    // American Polyconic Projection
            AmericanPolyconicFromCsDef(params.plycn, csDef);
            break;

        case COORDSYS_PSTSL:    // Polar Stereographic with Standard Latitude
            PolarStereographicStandardLatFromCsDef(params.pstsl, csDef);
            break;

        case COORDSYS_ROBIN:    // Robinson Projection
            RobinsonFromCsDef(params.robin, csDef);
            break;

        case COORDSYS_RSKEW:    // Rectified Skew Orthomorphic
            ObliqueMercator1PointFromCsDef(params.oblq1, csDef);
            break;

        case COORDSYS_RSKWC:    // Rectified Skew Orthomorphic Centered
            ObliqueMercator1PointFromCsDef(params.oblq1, csDef);
            break;

        case COORDSYS_RSKWO:    // Rectified Skew Orthomorphic Origin
            ObliqueMercator1PointFromCsDef(params.oblq1, csDef);
            break;

        case COORDSYS_SINUS:    // Sinusoidal Projection
            SinusoidalFromCsDef(params.sinus, csDef);
            break;

        case COORDSYS_SOTRM:    // Transverse Mercator South Oriented Projection
            TransverseMercatorFromCsDef(params.trmer, csDef);
            break;

        case COORDSYS_STERO:    // Stereographic Projection
            StereographicFromCsDef(params.stero, csDef);
            break;

        case COORDSYS_SWISS:    // Oblique CylindricalSwiss Projection
            SwissFromCsDef(params.swiss, csDef);
            break;

        case COORDSYS_SYS34:    // Danish System 34
        case COORDSYS_S3499:    // Danish System 34 with 1999 Polynomial.
        case COORDSYS_S3401:    // Danish System 34 with 2001 Polynomial.
            Danish34FromCsDef(params.sys34, csDef);
            break;

        case COORDSYS_TACYL:    // Transverse Authalic Cylindrical Projection
            TransverseAuthalicCylindricalFromCsDef(params.tacyl, csDef);
            break;

        case COORDSYS_TMMIN:    // Transverse Mercator, Minnesota
            TransverseMercatorMinnesotaFromCsDef(params.tmmin, csDef);
            break;

        case COORDSYS_TMWIS:    // Transverse Mercator, Wisconsin County
            TransverseMercatorWisconsinFromCsDef(params.tmwis, csDef);
            break;

        case COORDSYS_TRMAF:    // Transverse Mercator with Affine Post Process
            TransverseMercatorAffinePostProcessFromCsDef(params.trmaf, csDef);
            break;

        case COORDSYS_TRMER:    // Transverse Mercator Projection
            TransverseMercatorFromCsDef(params.trmer, csDef);
            break;

        case COORDSYS_TRMRS:    // Transverse Mercator Projection Snyder
            TransverseMercatorFromCsDef (params.trmrs, csDef);
            break;

        case COORDSYS_TMKRG:    // Transverse Mercator Projection Kruger
            TransverseMercatorFromCsDef(params.tmkrg, csDef);
            break;

        case COORDSYS_UTMZN:    // Universal Tranverse Mercator Zone Projections
            UniversalTransverseMercatorZoneFromCsDef(params.utmzn, csDef);
            break;

        case COORDSYS_Geographic:   // Unity (Lat/Long) pseudo-projection.
            GeographicFromCsDef(params.unity, csDef);
            break;

        case COORDSYS_VDGRN:        // Van der Grinten Projection
            VanDerGrintenFromCsDef(params.vdgrn, csDef);
            break;

        case COORDSYS_WINKT:        // Winkel-Tripel Projection
            WinkelTripelFromCsDef(params.winkt, csDef);
            break;

        case COORDSYS_LMBRTAF:        // Winkel-Tripel Projection
            LambertAffinePostProcessFromCsDef(params.lmbrtaf, csDef);
            break;

        // TOTAL_SPECIAL
        case COORDSYS_UTMZNBF:
            UniversalTransverseMercatorZoneFromCsDef(params.utmzn, csDef);
            break;

        case COORDSYS_TRMERBF:
            TransverseMercatorFromCsDef(params.trmer, csDef);
            break;

        case COORDSYS_EDCYLE:    // Equidistant Cylindrical Ellipsoidal Projection
            EquidistantCylindricalFromCsDef(params.edcyl, csDef);
            break;

        case COORDSYS_PCARREE:    // Simple Cylindrical / Plate Carree
            PlateCarreeFromCsDef(params.pcarree, csDef);
            break;

        case COORDSYS_MRCATPV:    // Popular Visualization Pseudo Mercator
            PopularVisualizationMercatorFromCsDef(params.mrcatpv, csDef);
            break;

        case COORDSYS_MNDOTOBL:
            MinnesotaDOTObliqueMercatorFromCsDef(params.mndotobl, csDef);
            break;

        case COORDSYS_KRVK95:    // Krovak Oblique Conformal Conic 1995
            KrovakFromCsDef(params.krvkg, csDef);
            break;

        case COORDSYS_LMMICH:
            LambertConformalConicMichiganFromCsDef(params.lmmich, csDef);
            break;

        case COORDSYS_KRVMO:    // Krovak Modified Oblique Conformal Conic
            KrovakFromCsDef(params.krvkg, csDef);
            break;

        default:
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
private:
void                FormatType66Struct
(
GeoCoordType66      &type66,
CSParameters        &csParams,
int32_t             coordSysId,
DgnDbR              cache,
bool                datumOrEllipsoidFromUserLib,
VertDatumCode       verticalDatum,
LocalTransformerP          localTransformer,
const GridFileDefinition&  gridFileDefinition
)
    {
    // convert the type 66 and projectionParams to a CSDefinition/CSDatum pair that can be made into CSParameters.
    memset((void*)&type66, 0, sizeof(type66));

    // must match previous geocoord code
    type66.gcoord66_size        = GEOCOORD66_SIZE;
    type66.version              = (NULL == localTransformer) && !datumOrEllipsoidFromUserLib ? GCOORD_COMPATIBLE_08117_VERSION : GCOORD_08119_VERSION;
    type66.e_rad                = csParams.datum.e_rad;
    type66.p_rad                = csParams.datum.p_rad;
    type66.delta_X              = csParams.datum.delta_X;
    type66.delta_Y              = csParams.datum.delta_Y;
    type66.delta_Z              = csParams.datum.delta_Z;
    type66.rot_X                = csParams.datum.rot_X;
    type66.rot_Y                = csParams.datum.rot_Y;
    type66.rot_Z                = csParams.datum.rot_Z;
    type66.bwscale              = csParams.datum.bwscale;
    if (datumOrEllipsoidFromUserLib && ConvertType_GENGRID == csParams.datum.to84_via &&
        gridFileDefinition.GetFormat() != GridFileFormat::FORMAT_NONE)
        {
        type66.gridFileFormat = (uint16_t)gridFileDefinition.GetFormat();
        type66.gridFileDirection = (uint16_t)gridFileDefinition.GetDirection();
        CSMap::CS_stncp(type66.gridFileName,     Utf8String(gridFileDefinition.GetFileName()).c_str(),  DIM (type66.gridFileName));
        }
    *((DPoint3d*)&type66.deprec_globorg) = cache.GeoLocation().GetGlobalOrigin();

    // this ridiculous unit stuff is only needed in the case where this file is saved to V7. I don't think the V8 geocoord stuff used it.
#ifdef WIP_DGN_GEOCOORD
    double  subPerMast          = dgnModel_getSubPerMaster(cache);
    double  uorPerSub           = dgnModel_getUorPerSub(cache);
    type66.deprec_subpermast    = DataConvert::RoundDoubleToULong(subPerMast);
    type66.deprec_uorpersub     = DataConvert::RoundDoubleToULong(uorPerSub);
#else
    type66.deprec_subpermast = type66.deprec_uorpersub = 1;
#endif

    DgnProjectionTypes  projectionType = BaseGCS::DgnProjectionTypeFromCSDefName (csParams.csdef.prj_knm);

    type66.protect              = csParams.csdef.protect;
    type66.projType             = projectionType;
    type66.family               = 0;    // not used
    type66.coordsys             = (0 != coordSysId) ? coordSysId : type66.projType;

#ifdef WIP_DGN_GEOCOORD
    // this stuff is hopefully never used, just ancient stupidity.
    WChar masterUnitLabel[MAX_StandardUnit::NAME_LENGTH];
    WChar subUnitLabel[MAX_StandardUnit::NAME_LENGTH];
    dgnModel_GetMasterUnitsLabel(cache, masterUnitLabel);
    dgnModel_GetSubUnitsLabel(cache, subUnitLabel);
    type66.deprec_mastname[0] = (char) (masterUnitLabel[0] && 0xFF);
    type66.deprec_mastname[1] = (char) (masterUnitLabel[1] && 0xFF);
    type66.deprec_subname[0]  = (char) (subUnitLabel[0] && 0xFF);
    type66.deprec_subname[1]  = (char) (subUnitLabel[1] && 0xFF);
    type66.deprec_dgnUnitNm[UNTSZ];
#else
    type66.deprec_mastname[0] = 'm';
    type66.deprec_mastname[1] = '\0';
    type66.deprec_subname[0] = 'm';
    type66.deprec_subname[1] = 'm';
#endif

    // used by some projections.
    CSMap::CS_stncp(type66.cs_knm,     csParams.csdef.key_nm,  DIM (type66.cs_knm));
    CSMap::CS_stncp(type66.grp_knm,    csParams.csdef.group,   DIM (type66.grp_knm));
    CSMap::CS_stncp(type66.prj_knm,    csParams.csdef.prj_knm, DIM (type66.prj_knm));
    CSMap::CS_stncp(type66.dat_nm,     csParams.datum.dt_name, DIM (type66.dat_nm));
    CSMap::CS_stncp(type66.ell_nm,     csParams.datum.el_name, DIM (type66.ell_nm));
    CSMap::CS_stncp(type66.desc_nm,    csParams.csdef.desc_nm, DIM (type66.desc_nm));
    CSMap::CS_stncp(type66.source,     csParams.csdef.source,  DIM (type66.source));

    type66.epsgNbr = (short)csParams.csdef.epsgNbr;

    if (0 != type66.projType)
        {
        Utf8String projName;
        BaseGeoCoordResource::GetLocalizedProjectionName(projName, projectionType);
        strncpy(type66.prj_nm, projName.c_str(), _countof(type66.prj_nm));
        }

    // so far, in every case I've seen that zone_knm has been set, it's been the same as cs_knm. I think it was used differently in the dim(witted) past.
    if (IsKeyNameCoordinateSystem(type66.coordsys))
        CSMap::CS_stncp(type66.zone_knm,   csParams.csdef.key_nm, DIM (type66.zone_knm));

    type66.to84_via                 = csParams.datum.to84_via;
    type66.minor_version            = (GCOORD_COMPATIBLE_08117_VERSION == type66.version) ? GCOORD_COMPATIBLE_08117_MINOR_VERSION : GCOORD_08119_MINOR_VERSION;
    type66.attributes               = 0;    // couldn't find anything useful in old code.
    type66.prj_code                 = csParams.prj_code;

    type66.dtmOrElpsdFromUserLib    = datumOrEllipsoidFromUserLib ? SOURCELIB_USER : 0;

    // set up the VerticalDatum. It takes up what was formerly part of the "filler" so we set a signature word
    //  indicating that it is valid. That's because we're not sure that the "filler" was properly initialized to zero in previous version.
    type66.verticalDatum        = (uint16_t) verticalDatum;
    type66.verticalDatumValid   = GEOCOORD66_VerticalDatumValid;

    if (NULL != localTransformer)
        localTransformer->SaveParameters(type66.localTransformType, type66.transformParams);
    }

#if defined (TEST_TYPE66_CREATE)
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
private:
void            CompareType66
(
GeoCoordType66      &oldType66,
GeoCoordType66      &newType66
)
    {
    if (oldType66.gcoord66_size != newType66.gcoord66_size)
        printf("Error in gcoord_size, old is %d, new is %d\n", oldType66.gcoord66_size, newType66.gcoord66_size);

    if (oldType66.version != newType66.version)
        printf("Error in version, old is %d, new is %d\n", oldType66.version, newType66.version);

    if (oldType66.e_rad != newType66.e_rad)
        printf("Error in e_rad, old is %lf, new is %lf\n", oldType66.e_rad, newType66.e_rad);

    if (oldType66.p_rad != newType66.p_rad)
        printf("Error in p_rad, old is %lf, new is %lf\n", oldType66.p_rad, newType66.p_rad);

    if (oldType66.delta_X != newType66.delta_X)
        printf("Error in delta_X, old is %lf, new is %lf\n", oldType66.delta_X, newType66.delta_X);
    if (oldType66.delta_Y != newType66.delta_Y)
        printf("Error in delta_Y, old is %lf, new is %lf\n", oldType66.delta_Y, newType66.delta_Y);
    if (oldType66.delta_Z != newType66.delta_Z)
        printf("Error in delta_Z, old is %lf, new is %lf\n", oldType66.delta_Z, newType66.delta_Z);

    if (oldType66.rot_X != newType66.rot_X)
        printf("Error in rot_X, old is %lf, new is %lf\n", oldType66.rot_X, newType66.rot_X);
    if (oldType66.rot_Y != newType66.rot_Y)
        printf("Error in rot_Y, old is %lf, new is %lf\n", oldType66.rot_Y, newType66.rot_Y);
    if (oldType66.rot_Z != newType66.rot_Z)
        printf("Error in rot_Z, old is %lf, new is %lf\n", oldType66.rot_Z, newType66.rot_Z);

    if (oldType66.bwscale != newType66.bwscale)
        printf("Error in bwscale, old is %lf, new is %lf\n", oldType66.bwscale, newType66.bwscale);

    if (oldType66.deprec_globorg.x != newType66.deprec_globorg.x)
        printf("Error in deprec_globorg.x, old is %lf, new is %lf\n", oldType66.deprec_globorg.x, newType66.deprec_globorg.x);
    if (oldType66.deprec_globorg.y != newType66.deprec_globorg.y)
        printf("Error in deprec_globorg.y, old is %lf, new is %lf\n", oldType66.deprec_globorg.y, newType66.deprec_globorg.y);
    if (oldType66.deprec_globorg.z != newType66.deprec_globorg.z)
        printf("Error in deprec_globorg.z, old is %lf, new is %lf\n", oldType66.deprec_globorg.z, newType66.deprec_globorg.z);

#if defined (DONT_CHECK)
    // most (but not all) of the type 66's I checked had deprec_mastunits set to 0. Certainly it is useless.
    if (oldType66.deprec_mastunits != newType66.deprec_mastunits)
        printf("Error in deprec_mastunits, old is %d, new is %d\n", oldType66.deprec_mastunits, newType66.deprec_mastunits);
#endif
    if (oldType66.deprec_subpermast != newType66.deprec_subpermast)
        printf("Error in deprec_subpermast, old is %d, new is %d\n", oldType66.deprec_subpermast, newType66.deprec_subpermast);
    if (oldType66.deprec_uorpersub != newType66.deprec_uorpersub)
        printf("Error in deprec_uorpersub, old is %d, new is %d\n", oldType66.deprec_uorpersub, newType66.deprec_uorpersub);

    if (oldType66.protect != newType66.protect)
        printf("Error in protect, old is %d, new is %d\n", oldType66.protect, newType66.protect);

    if (oldType66.projType != newType66.projType)
        printf("Error in projType, old is %d, new is %d\n", oldType66.projType, newType66.projType);

    if (oldType66.family != newType66.family)
        printf("Error in family, old is %d, new is %d\n", oldType66.family, newType66.family);

    if (oldType66.coordsys != newType66.coordsys)
        printf("Error in coordsys, old is %d, new is %d\n", oldType66.coordsys, newType66.coordsys);

    if (oldType66.deprec_mastname[0] != newType66.deprec_mastname[0])
        printf("Error in deprec_mastname[0], old is %c, new is %c\n", oldType66.deprec_mastname[0], newType66.deprec_mastname[0]);
    if (oldType66.deprec_mastname[1] != newType66.deprec_mastname[1])
        printf("Error in deprec_mastname[1], old is %c, new is %c\n", oldType66.deprec_mastname[1], newType66.deprec_mastname[1]);
    if (oldType66.deprec_subname[0] != newType66.deprec_subname[0])
        printf("Error in deprec_subname[0], old is %c, new is %c\n", oldType66.deprec_subname[0], newType66.deprec_subname[0]);
    if (oldType66.deprec_subname[1] != newType66.deprec_subname[1])
        printf("Error in deprec_subname[1], old is %c, new is %c\n", oldType66.deprec_subname[1], newType66.deprec_subname[1]);

    if (0 != strcmp(oldType66.cs_knm, newType66.cs_knm))
        printf("Error in cs_knm, old is '%s', new is '%s'\n", oldType66.cs_knm, newType66.cs_knm);
    if (0 != strcmp(oldType66.grp_knm, newType66.grp_knm))
        printf("Error in grp_knm, old is '%s', new is '%s'\n", oldType66.grp_knm, newType66.grp_knm);
    if (0 != strcmp(oldType66.zone_knm, newType66.zone_knm))
        printf("Error in zone_knm, old is '%s', new is '%s'\n", oldType66.zone_knm, newType66.zone_knm);
    if (0 != strcmp(oldType66.prj_knm, newType66.prj_knm))
        printf("Error in prj_knm, old is '%s', new is '%s'\n", oldType66.prj_knm, newType66.prj_knm);

    // the old prj_nm's often have " Projection" at the end, as in "Universal Tranverse Mercator Projection", but I think that was later shortened, so don't count as error.
    if (0 != strncmp(oldType66.prj_nm, newType66.prj_nm, strlen(newType66.prj_nm)))
        printf("Error in prj_nm, old is '%s', new is '%s'\n", oldType66.prj_nm, newType66.prj_nm);
    if (0 != strcmp(oldType66.dat_nm, newType66.dat_nm))
        printf("Error in dat_nm, old is '%s', new is '%s'\n", oldType66.dat_nm, newType66.dat_nm);
    if (0 != strcmp(oldType66.ell_nm, newType66.ell_nm))
        printf("Error in ell_nm, old is '%s', new is '%s'\n", oldType66.ell_nm, newType66.ell_nm);
    if (0 != strcmp(oldType66.desc_nm, newType66.desc_nm))
        printf("Error in desc_nm, old is '%s', new is '%s'\n", oldType66.desc_nm, newType66.desc_nm);
    if (0 != strcmp(oldType66.source, newType66.source))
        printf("Error in source, old is '%s', new is '%s'\n", oldType66.source, newType66.source);

    if (oldType66.to84_via != newType66.to84_via)
        printf("Error in to84_via, old is %d, new is %d\n", oldType66.to84_via, newType66.to84_via);

    if (oldType66.minor_version != newType66.minor_version)
        printf("Error in minor_version, old is %d, new is %d\n", oldType66.minor_version, newType66.minor_version);
    if (oldType66.attributes != newType66.attributes)
        printf("Error in attributes, old is %d, new is %d\n", oldType66.attributes, newType66.attributes);
    if (oldType66.prj_code != newType66.prj_code)
        printf("Error in prj_code, old is %d, new is %d\n", oldType66.prj_code, newType66.prj_code);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
private:
void            CompareProjectionParams
(
ProjectionParams    &oldProjectionParams,
ProjectionParams    &newProjectionParams
)
    {

    }


#endif
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
public:
StatusInt           FormatGeoCoordType66
(
short*              type66AppData,          // <= filled in with data. Must be at least 768 words.
uint32_t&           type66AppDataBytes,     // <= set to sizeof of type66AppData in bytes
CSParameters        &csParams,
int32_t             coordSysId,
DgnDbR              project,
bool                primary,
double              paperScaleFromType66,
bool                datumOrEllipsoidFromUserLib,
VertDatumCode       verticalDatum,
LocalTransformerP          localTransformer,
const GridFileDefinition&  gridFileDefinition
)
    {
    GeoCoordType66      type66;
    ProjectionParams    projectionParams;

    FormatType66Struct(type66, csParams, coordSysId, project, datumOrEllipsoidFromUserLib, verticalDatum, localTransformer, gridFileDefinition);

    // set map_scl only for the purpose of saving it to the type 66. We don't have CSMap handling it.
    csParams.csdef.map_scl = paperScaleFromType66;

    // if either the datum or the ellipsoid is from the user lib, we need to make sure the ellipsoid name is saved in the struct, even if there is a Datum.
    //  That's because the user library might not be present on the computer of the person opening this design file.
    //  So, we set the m_datum member so that we can extract the datum name from there. It isn't set in csParams.csdef unless there is no datum.
    if (datumOrEllipsoidFromUserLib)
        m_datum = &csParams.datum;
    else
        m_datum = NULL;
    FormatProjectionParams(type66.projType, projectionParams, csParams.csdef);
    // make sure we're not holding a stale datum.
    m_datum = NULL;

    // put map_scl back to 1.0;
    csParams.csdef.map_scl = 1.0;

#if defined (TEST_TYPE66_CREATE)
    if (NULL != s_oldType66)
        CompareType66(*s_oldType66, type66);
    if (NULL != s_oldProjectionParams)
        CompareProjectionParams(*s_oldProjectionParams, projectionParams);
#endif

    // set up the type 66 appData. Note this is the same format used by DgnV8
    type66AppData[0] = primary ? GCOORD_66_MAS : GCOORD_66_ALT;

    // this used to use the resource conversion logic, but it turns out that the structure was set up
    // correctly, grouping the doubles and filling in the holes so just requires a memcpy.
    memcpy(&type66AppData[1], &type66, sizeof (GeoCoordType66));

    // The ProjectionParams union was also set up correctly to allow a simple memcpy for every type of projection
    memcpy(type66AppData+429, &projectionParams, sizeof(ProjectionParams));

    //  The first short is the primary/alt flag
    //  The rest is the GeoCoordType66
    type66AppDataBytes = sizeof(short) + GEOCOORD66_SIZE;

    return SUCCESS;
    }

};

END_UNNAMED_NAMESPACE

CoordinateSystemDgnFormatter*   CoordinateSystemDgnFormatter::s_instance;

/*=================================================================================**//**
* GeoCoordinateSystem Native class
* @bsiclass
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS::DgnGCS () : BaseGCS()
    {
    m_uorsPerBaseUnit               = 1.0;
    m_paperScaleFromType66          = 1.0;
    m_globalOrigin.Zero();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS::DgnGCS
(
Utf8CP         coordinateSystemName
) : BaseGCS (coordinateSystemName)
    {
    m_uorsPerBaseUnit               = 1.0;
    m_paperScaleFromType66          = 1.0;
    m_globalOrigin.Zero();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS::DgnGCS
(
Utf8CP     coordinateSystemName,
DgnDbR     modelRef
) : BaseGCS (coordinateSystemName)
    {
    InitCacheParameters(modelRef, 1.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS::DgnGCS
(
CSParameters&   csParameters,
double          paperScale,
int32_t         coordSysId,
DgnDbR          cache,
bool                    datumOrEllipsoidFromUserLibrary,
CSGeodeticTransformDef const* geodeticTransform
) : BaseGCS (csParameters, coordSysId, geodeticTransform)
    {
    // This is the constructor called when we extract a GCS from the type 66. That's a case where we don't have m_sourceLibrary.
    // However, whether the datum or ellipsoid is in a user library is stored in the type 66.
    // If we don't have the user library at runtime, we don't have a way of telling whether
    //   the datum or ellipsoid came from there, so we need to keep what the type 66 tells us.
    InitCacheParameters(cache, paperScale);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS::DgnGCS
(
BaseGCSCP       baseGCS,
DgnDbR     modelRef
) : BaseGCS (*baseGCS)
    {
    InitCacheParameters(modelRef, 1.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS::DgnGCS (DgnDbR  modelRef) : BaseGCS ()
    {
    InitCacheParameters(modelRef, 1.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS::DgnGCS (DgnGCSCR sourceGcs) : BaseGCS(sourceGcs)
    {
    m_paperScaleFromType66 = sourceGcs.m_paperScaleFromType66;
    m_uorsPerBaseUnit      = sourceGcs.m_uorsPerBaseUnit;
    m_globalOrigin         = sourceGcs.m_globalOrigin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnGCS::InitCacheParameters
(
DgnDbR    project,
double      paperScale
)
    {
    m_paperScaleFromType66          = paperScale;

#ifdef WIP_DGN_GEOCOORD
    double      uorPerStorage = dgnModel_getUorPerStorage(project);

    UnitInfo    storageUnitInfo;
    if (SUCCESS != dgnModel_getStorageUnit(project, &storageUnitInfo))
        m_uorsPerBaseUnit = 100000.0;

    else
        {
        // this intentionally ignores the unit base.
        // Thus for a Lat/Long based coordinate system, if the design file storage units are Dgn::UnitBase::Meter, a meter is treated as a degree.
        // Similarly, for a projected coordinate system, if the design file storage units are Dgn::UnitBase::Degree, a degree is treated as a meter.
        m_uorsPerBaseUnit = (uorPerStorage * storageUnitInfo.numerator) / (storageUnitInfo.denominator * m_paperScaleFromType66);
        dgnModel_getGlobalOrigin(project, &m_globalOrigin);
        }
#else
    m_uorsPerBaseUnit = 1; // in DgnDb, all coordinates are stored in meters
    m_globalOrigin = project.GeoLocation().GetGlobalOrigin();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DgnGCS::GetUserDatumBasedOnGridFile()
{
    return (HasCustomDatum() && (GetDatumConvertMethod() == ConvertType_GENGRID));
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnGCS::SetGlobalOriginAndUnitScaling (DPoint3d& globalOrigin, double uorsPerBaseUnit)
    {
    // this method should only be called from the DgnV8Converter. For all BIM files, the base unit is meters, and uorsPerBaseUnit is 1.0
    m_globalOrigin    = globalOrigin;
    m_uorsPerBaseUnit = uorsPerBaseUnit;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS::~DgnGCS ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* Public Factory Methods
+---------------+---------------+---------------+---------------+---------------+------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCSPtr       DgnGCS::CreateGCS
(
Utf8CP             coordinateSystemName,
DgnDbR             modelRef
)
    {
    return new DgnGCS  (coordinateSystemName, modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCSPtr       DgnGCS::CreateGCS (DgnDbR  modelRef)
    {
    return new DgnGCS  (modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCSPtr       DgnGCS::CreateGCS
(
BaseGCSCP               baseGCS,
DgnDbR             modelRef
)
    {
    return new DgnGCS  (baseGCS, modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCSPtr DgnGCS::CreateGCS(DgnGCSCR dgnGcs)
    {
    return new DgnGCS(dgnGcs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnGCS::CartesianFromUors
(
DPoint3dR               outCartesian,       // <= cartesian, units of coordinate system
DPoint3dCR              inUors              // => UORS
) const
    {
    outCartesian.DifferenceOf(inUors, m_globalOrigin);
    outCartesian.Scale(m_csParameters->csdef.scale / m_uorsPerBaseUnit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnGCS::UorsFromCartesian
(
DPoint3dR               outUors,            // <= UORS
DPoint3dCR              inCartesian         // => cartesian, units of coordinate system
) const
    {
    outUors = inCartesian;
    outUors.Scale(m_uorsPerBaseUnit / m_csParameters->csdef.scale);
    outUors.SumOf(m_globalOrigin,outUors);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnGCS::CartesianFromUors2D
(
DPoint2dR               outCartesian,       // <= cartesian, units of coordinate system
DPoint2dCR              inUors              // => UORS
) const
    {
    double  scale = m_csParameters->csdef.scale / m_uorsPerBaseUnit;
    outCartesian.x = scale * (inUors.x - m_globalOrigin.x);
    outCartesian.y = scale * (inUors.y - m_globalOrigin.y);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnGCS::UorsFromCartesian2D
(
DPoint2dR               outUors,            // <= UORS
DPoint2dCR              inCartesian         // => cartesian, units of coordinate system
) const
    {
    double scale = m_uorsPerBaseUnit / m_csParameters->csdef.scale;
    outUors.x = inCartesian.x * scale + m_globalOrigin.x;
    outUors.y = inCartesian.y * scale + m_globalOrigin.y;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus DgnGCS::LatLongFromUors
(
GeoPointR               outLatLong,         // <= latitude longitude
DPoint3dCR              inUors              // => cartesian, in UORS
) const
    {
    DPoint3d    cartesian;
    CartesianFromUors(cartesian, inUors);
    return LatLongFromCartesian(outLatLong, cartesian);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus DgnGCS::LatLongFromUorsXYZ
(
GeoPointR               outLatLong,         // <= latitude longitude
DPoint3dCR              inUors              // => cartesian, in UORS
) const
    {
    DPoint3d    point;
    CartesianFromUors(point, inUors);
    return LatLongFromXYZ(outLatLong, point);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus DgnGCS::UorsFromLatLong
(
DPoint3dR               outUors,            // <= cartesian, in UORS
GeoPointCR              inLatLong           // => latitude longitude
) const
    {
    DPoint3d cartesian;
    ReprojectStatus status = CartesianFromLatLong(cartesian, inLatLong);
    UorsFromCartesian(outUors, cartesian);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus         DgnGCS::ReprojectUors
(
    DPoint3dP               outUorsDest,        // <= cartesian, in UORS of destination DgnGCS 's model.
    GeoPointP               outLatLongDest,     // <= (optional) lat long in destination Geocoordinate system.
    GeoPointP               outLatLongSrc,      // <= (optional) lat long in this Geocoordinate system.
    DPoint3dCP              inUors,             // => cartesian coordinates in this Geocoordinate system modelRef UORS.
    int                     numPoints,          // => number of points.
    DgnGCSCR                destMstnGCS         // => destination coordinate system
) const
    {
    return ReprojectUors(outUorsDest, outLatLongDest, outLatLongSrc, inUors, numPoints, GeoCoordInterpretation::Cartesian, destMstnGCS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus         DgnGCS::ReprojectUors
(
DPoint3dP               outUorsDest,        // <= cartesian, in UORS of destination DgnGCS 's model.
GeoPointP               outLatLongDest,     // <= (optional) lat long in destination Geocoordinate system.
GeoPointP               outLatLongSrc,      // <= (optional) lat long in this Geocoordinate system.
DPoint3dCP              inUors,             // => cartesian coordinates in this Geocoordinate system modelRef UORS.
int                     numPoints,          // => number of points.
GeoCoordInterpretation  interpretation,     // => the interpretation of the input points.
DgnGCSCR                destMstnGCS         // => destination coordinate system
) const
    {
    ReprojectStatus status = REPROJECT_Success;

    if ( (NULL == outUorsDest) || (NULL == inUors) )
        return REPROJECT_BadArgument;

    int iPoint;
    for (iPoint=0; iPoint < numPoints; iPoint++, inUors++, outUorsDest++)
        {
        ReprojectStatus   stat1 = REPROJECT_Success;
        ReprojectStatus   stat2;
        ReprojectStatus   stat3;

        // if we get a disconnect, set all outputs to DISCONNECT also, and move on to the next point.
        if (DISCONNECT == inUors->x)
            {
            *outUorsDest = *inUors;
            if (NULL != outLatLongSrc)
                {
                outLatLongSrc->latitude = outLatLongSrc->longitude = outLatLongSrc->elevation = DISCONNECT;
                outLatLongSrc++;
                }
            if (NULL != outLatLongDest)
                {
                outLatLongDest->latitude = outLatLongDest->longitude = outLatLongDest->elevation = DISCONNECT;
                outLatLongDest++;
                }
            continue;
            }

        // convert the input LatLong to meters.
        GeoPoint    srcLatLong = GeoPoint();
        switch (interpretation)
             {
            case GeoCoordInterpretation::Cartesian:
                {
                stat1 = LatLongFromUors(srcLatLong, *inUors);
                break;
                }
            case GeoCoordInterpretation::XYZ:
                {
                stat1 = LatLongFromUorsXYZ(srcLatLong, *inUors);
                break;
                }
            default:
                {
                assert(!"Unknown geocoordinate interpretion for reprojection");
                }
            };

        // does caller want the latlong in source coordinates?
        if (NULL != outLatLongSrc)
            *outLatLongSrc++ = srcLatLong;

        // convert srcLatLong to destLatLong
        GeoPoint    destLatLong;
        stat2 = LatLongFromLatLong(destLatLong, srcLatLong, destMstnGCS);

        // For datum conversion error 2 is in fact a strong warning instead of a hard math error ... we change to a warning
        if (REPROJECT_CSMAPERR_OutOfMathematicalDomain == stat2)
            stat2 = REPROJECT_CSMAPERR_OutOfUsefulRange;

        // does the caller want the destination LatLong?
        if (NULL != outLatLongDest)
            *outLatLongDest++ = destLatLong;

        // convert dstLatLong to destination cartesian.
        stat3 = destMstnGCS.UorsFromLatLong(*outUorsDest, destLatLong);

        if (REPROJECT_Success == status)
            {
            // Status returns hardest error found in the three error statuses
            // The hardest error is the first one encountered that is not a warning (value 1 [cs_CNVRT_USFL])
            if (REPROJECT_Success != stat1)
                status = stat1;
            if ((REPROJECT_Success != stat2) && ((REPROJECT_Success == status) || (cs_CNVRT_USFL == status))) // If stat2 has error and status not already hard error
                {
                if (0 > stat2) // If stat2 is negative ... this is the one ...
                    status = stat2;
                else  // Both are positive (status may be REPROJECT_Success) we use the highest value which is either warning or error
                    status = (stat2 > status ? stat2 : status);
                }
            if ((REPROJECT_Success != stat3) && ((REPROJECT_Success == status) || (cs_CNVRT_USFL == status))) // If stat3 has error and status not already hard error
                {
                if (0 > stat3) // If stat3 is negative ... this is the one ...
                    status = stat3;
                else  // Both are positive (status may be REPROJECT_Success) we use the highest value
                    status = (stat3 > status ? stat3 : status);
                }
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus       DgnGCS::LatLongFromUors2D
(
GeoPoint2dR             outLatLong,         // <= latitude longitude
DPoint2dCR              inUors              // => cartesian, in UORS
) const
    {
    DPoint2d    cartesian;
    CartesianFromUors2D (cartesian, inUors);
    return LatLongFromCartesian2D (outLatLong, cartesian);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus       DgnGCS::UorsFromLatLong2D
(
DPoint2dR               outUors,            // <= UORS
GeoPoint2dCR            inLatLong           // => latitude longitude
) const
    {

    DPoint2d cartesian;
    ReprojectStatus status = CartesianFromLatLong2D (cartesian, inLatLong);
    UorsFromCartesian2D (outUors, cartesian);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus       DgnGCS::ReprojectUors2D
(
DPoint2dP               outUorsDest,        // <= cartesian, in UORS of destination DgnGCS 's model.
GeoPoint2dP             outLatLongDest,     // <= (optional) lat long in destination Geocoordinate system.
GeoPoint2dP             outLatLongSrc,      // <= (optional) lat long in this Geocoordinate system.
DPoint2dCP              inUors,             // => cartesian coordinates in this Geocoordinate system modelRef UORS.
int                     numPoints,          // => number of points.
DgnGCSCR                destMstnGCS         // => destination coordinate system
) const
    {
    ReprojectStatus   status = REPROJECT_Success;

    if ( (NULL == outUorsDest) || (NULL == inUors) )
        return REPROJECT_BadArgument;

    int iPoint;
    for (iPoint=0; iPoint < numPoints; iPoint++, inUors++, outUorsDest++)
        {
        ReprojectStatus   stat1 = REPROJECT_Success;
        ReprojectStatus   stat2;
        ReprojectStatus   stat3;

        // if we get a disconnect, set all outputs to DISCONNECT also, and move on to the next point.
        if (DISCONNECT == inUors->x)
            {
            *outUorsDest = *inUors;
            if (NULL != outLatLongSrc)
                {
                outLatLongSrc->latitude = outLatLongSrc->longitude = DISCONNECT;
                outLatLongSrc++;
                }
            if (NULL != outLatLongDest)
                {
                outLatLongDest->latitude = outLatLongDest->longitude = DISCONNECT;
                outLatLongDest++;
                }
            continue;
            }

        // convert the input LatLong to meters.
        GeoPoint2d    srcLatLong;
        stat1 = LatLongFromUors2D (srcLatLong, *inUors);

        // does caller want the latlong in source coordinates?
        if (NULL != outLatLongSrc)
            *outLatLongSrc++ = srcLatLong;

        // convert srcLatLong to destLatLong
        GeoPoint2d    destLatLong;
        stat2 = LatLongFromLatLong2D (destLatLong, srcLatLong, destMstnGCS);

        // does the caller want the destination LatLong?
        if (NULL != outLatLongDest)
            *outLatLongDest++ = destLatLong;

        // convert dstLatLong to destination cartesian.
        stat3 = destMstnGCS.UorsFromLatLong2D (*outUorsDest, destLatLong);

        if (REPROJECT_Success == status)
            {
            // Status returns hardest error found in the three error statuses
            // The hardest error is the first one encountered that is not a warning (value 1 [cs_CNVRT_USFL])
            if (REPROJECT_Success != stat1)
                status = stat1;
            if ((REPROJECT_Success != stat2) && ((REPROJECT_Success == status) || (cs_CNVRT_USFL == status))) // If stat2 has error and status not already hard error
                {
                if (0 > stat2) // If stat2 is negative ... this is the one ...
                    status = stat2;
                else  // Both are positive (status may be REPROJECT_Success) we use the highest value which is either warning or error
                    status = (stat2 > status ? stat2 : status);
                }
            if ((REPROJECT_Success != stat3) && ((REPROJECT_Success == status) || (cs_CNVRT_USFL == status))) // If stat3 has error and status not already hard error
                {
                if (0 > stat3) // If stat3 is negative ... this is the one ...
                    status = stat3;
                else  // Both are positive (status may be REPROJECT_Success) we use the highest value
                    status = (stat3 > status ? stat3 : status);
                }
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* BaseGCSCartesianFromUors - Converts from the UORs of a DgnGCS to
* the Cartesian of the target BaseGCS.
* @return
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus DgnGCS::BaseGCSCartesianFromUors(DPoint3dR outCartesian, DPoint3dCR inUors, GeoCoordinates::BaseGCSCR targetGCS) const
{
    ReprojectStatus status = REPROJECT_Success;

    if (!IsValid())
        return (ReprojectStatus)GEOCOORDERR_InvalidCoordSys;

    if (!targetGCS.IsValid())
        return (ReprojectStatus)GEOCOORDERR_InvalidCoordSys;


    ReprojectStatus   stat1;
    ReprojectStatus   stat2;
    ReprojectStatus   stat3;

    GeoPoint inLatLong;
    stat1 = LatLongFromUors(inLatLong, inUors);

    GeoPoint outLatLong;
    stat2 = LatLongFromLatLong(outLatLong, inLatLong, targetGCS);

    stat3 = targetGCS.CartesianFromLatLong(outCartesian, outLatLong);

    if (REPROJECT_Success == status)
    {
        // Status returns hardest error found in the three error statuses
        // The hardest error is the first one encountered that is not a warning (value 1 [REPROJECT_CSMAPERR_OutOfUsefulRange])
        if (REPROJECT_Success != stat1)
            status = stat1;
        if ((REPROJECT_Success != stat2) && ((REPROJECT_Success == status) || (REPROJECT_CSMAPERR_OutOfUsefulRange == status) || (REPROJECT_CSMAPERR_VerticalDatumConversionError == status))) // If stat2 has error and status not already hard error
        {
            if (0 > stat2) // If stat2 is negative ... this is the one ...
                status = stat2;
            else  // Both are positive (status may be REPROJECT_Success) we use the highest value which is either warning or error
                status = (stat2 > status ? stat2 : status);
        }
        if ((REPROJECT_Success != stat3) && ((REPROJECT_Success == status) || (REPROJECT_CSMAPERR_OutOfUsefulRange == status) || (REPROJECT_CSMAPERR_VerticalDatumConversionError == status))) // If stat3 has error and status not already hard error
        {
            if (0 > stat3) // If stat3 is negative ... this is the one ...
                status = stat3;
            else  // Both are positive (status may be SUCCESS) we use the highest value
                status = (stat3 > status ? stat3 : status);
        }
    }

    return status;
}
/*---------------------------------------------------------------------------------**//**
* Derived from DgnGCS::GetLocalTransform and BaseGCS::GetLinearTransform
* @bsimethod
* @bsimethod
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus       DgnGCS::GetLinearTransformToBaseGCS
(
    TransformP                outTransform,       // <= the transform computed over specified area.
    DRange3dCR                extent,             // => the extent in source GCS coordinate to use to find the transform.
    GeoCoordinates::BaseGCSCR targetGCS,          // => target Base GCS
    double*                   maxError,           // => If provided receives the max error observed over the extent
    double*                   meanError           // => If provided receives the mean error observed over the extent
) const
{

    // Given the library is NOT initialized ...
    if (!IsLibraryInitialized())
    {
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        return (ReprojectStatus)m_csError;
    }

    if (!IsValid())
        return (ReprojectStatus)GEOCOORDERR_InvalidCoordSys;

    if (!targetGCS.IsValid())
        return (ReprojectStatus)GEOCOORDERR_InvalidCoordSys;

    if (extent.IsEmpty() || extent.IsNull())
        return REPROJECT_BadArgument;

    // If input and output GCS are identical this gets rid of computational errors and returns immediately
    if (IsEqual(targetGCS))
    {
        outTransform->InitIdentity();

        if (maxError != nullptr)
            *maxError = 0.0;

        if (meanError != nullptr)
            *meanError = 0.0;

        return REPROJECT_Success;
    }

    const double linearTolerance = 0.01;
    double tolerance = linearTolerance;

    if (!IsProjected())
        tolerance = 0.0000001; // Tolerance for longitude/latitude GCS in degrees

    // Extent must be large enough so we can effectively compute coordinate differences
    // since the geocoord engine will stop calculations when 0.001 per iteration is reached.
    if (extent.XLength() < tolerance || extent.YLength() < tolerance || extent.ZLength() < linearTolerance) // Z allways uses linear tolerance
        return REPROJECT_BadArgument;

    Transform   frameA, frameB, frameAInverse;
    DPoint3d    points[4];

    // We select point somewhat at the center of the region, not too much at the extreme periphery
    double offsetX = (extent.high.x - extent.low.x) / 2.0;
    double offsetY = (extent.high.y - extent.low.y) / 2.0;
    double offsetZ = (extent.high.z - extent.low.z) / 2.0;

    DPoint3d elementOrigin;

    elementOrigin.Init(((extent.low.x + extent.high.x) / 2.0) - offsetX / 2.0,
        ((extent.low.y + extent.high.y) / 2.0) - offsetY / 2.0,
        ((extent.low.z + extent.high.z) / 2.0) - offsetZ / 2.0);

    points[0] = elementOrigin;
    points[1].Init(elementOrigin.x + offsetX, elementOrigin.y, elementOrigin.z);
    points[2].Init(elementOrigin.x, elementOrigin.y + offsetY, elementOrigin.z);
    points[3].Init(elementOrigin.x, elementOrigin.y, elementOrigin.z + offsetZ);

    DPoint3d transformedPoints[4];

    // Transform the points
    ReprojectStatus status = REPROJECT_Success;
    for (size_t i = 0; i < 4; i++)
    {
        ReprojectStatus currentStatus = BaseGCSCartesianFromUors(transformedPoints[i], points[i], targetGCS);

        if (REPROJECT_Success == status) // No warning previously occured ...
            status = currentStatus;
        else if ((REPROJECT_Success != currentStatus) && (status != REPROJECT_CSMAPERR_VerticalDatumConversionError))
            status = currentStatus; // We keep vertical datum error which is a little 'harder' than out of user domain

        // Check for hard error and stop if there is one
        if ((REPROJECT_Success != status) && (REPROJECT_CSMAPERR_OutOfUsefulRange != currentStatus) && (REPROJECT_CSMAPERR_VerticalDatumConversionError != currentStatus))
            return status; // Hard error ... we exit method immediately
    }

    frameA.InitFrom4Points(points[0], points[1], points[2], points[3]);
    frameAInverse.InverseOf(frameA);

    frameB.InitFrom4Points(transformedPoints[0], transformedPoints[1], transformedPoints[2], transformedPoints[3]);

    outTransform->InitProduct(frameB, frameAInverse);

    if ((meanError != nullptr) || (maxError != nullptr))
    {
        ReprojectStatus analysisStatus = REPROJECT_Success;

        if (maxError != nullptr)
            *maxError = 0.0;

        double totalError = 0.0;
        double numberOfSamples = 0;
        // Error analysis is requested ...
        double xStep = (extent.high.x - extent.low.x) / 4.0;
        for (double currentX = extent.low.x; currentX <= extent.high.x + (xStep * 0.00000001); currentX += xStep) // The epsilon insures we process the high value
        {
            double yStep = (extent.high.y - extent.low.y) / 4.0;
            for (double currentY = extent.low.y; currentY <= extent.high.y + (yStep * 0.00000001); currentY += yStep) // The epsilon insures we process the high value
            {
                DPoint3d inCartesian;
                DPoint3d outCartesianGCS;
                inCartesian.Init(currentX, currentY, elementOrigin.z);

                // Transform normally
                analysisStatus = BaseGCSCartesianFromUors(outCartesianGCS, inCartesian, targetGCS);

                if ((REPROJECT_Success == analysisStatus) || (REPROJECT_CSMAPERR_OutOfUsefulRange == status) || (REPROJECT_CSMAPERR_VerticalDatumConversionError == status))
                {
                    // Not a hard error ... we use.

                    // Transform using produced transform
                    DPoint3d outCartesianTRF;
                    outTransform->Multiply(outCartesianTRF, inCartesian);

                    // Compute difference and cumulate
                    double diffX = outCartesianGCS.x - outCartesianTRF.x;
                    double diffY = outCartesianGCS.y - outCartesianTRF.y;
                    double diffZ = outCartesianGCS.z - outCartesianTRF.z;
                    double currentError = sqrt(diffX * diffX + diffY * diffY + diffZ * diffZ);

                    if ((maxError != nullptr) && (*maxError < currentError))
                        *maxError = currentError;

                    totalError += currentError;
                    numberOfSamples++;
                }
            }
        }

        // Compute mean error from total error
        if ((meanError != nullptr) && (numberOfSamples >= 1))
            *meanError = totalError / numberOfSamples;
    }

    return status;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus       DgnGCS::GetLocalTransform
(
TransformP              outTransform,       // <= the transform effective at the point elementOrigin in source coordinates
DPoint3dCR              elementOrigin,      // => the point to use to find the transform.
DPoint3dCP              extent,             // => the extent to use to find the rotation and scale, NULL to use a default range.
bool                    doRotate,           // => whether to apply a rotation.
bool                    doScale,            // => whehter to apply a scale based on the coordinate system distortion.
DgnGCSCR                destMstnGCS         // => destination coordinate system
) const
    {
    return GetLocalTransform(outTransform, elementOrigin, extent, doRotate, doScale, GeoCoordInterpretation::Cartesian, destMstnGCS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus       DgnGCS::GetLocalTransform
(
TransformP              outTransform,       // <= the transform effective at the point elementOrigin in source coordinates
DPoint3dCR              elementOrigin,      // => the point to use to find the transform.
DPoint3dCP              extent,             // => the extent to use to find the rotation and scale, NULL to use a default range.
bool                    doRotate,           // => whether to apply a rotation.
bool                    doScale,            // => whether to apply a scale based on the coordinate system distortion.
GeoCoordInterpretation  interpretation,     // => the interpretation of the input points.
DgnGCSCR                destMstnGCS         // => destination coordinate system
) const
    {

    static bool sDoPerpendicularAxes = false;   // If true, the destination frame is squared up.
    static double sReferenceFrameSize = 10.0;   // We map points in a 10 (meter, mu) frame to define coordinate systems.
                                                // In highly curved mappings this size affects skewing
    static bool sAllowScale = true;             // false suppresses scale arg.
    static bool sAllowRotate = true;            // false suppresses rotation arg.

    Transform   frameA, frameB, frameAInverse;
    DPoint3d    points[4];
    DPoint3d    localExtent;

    if (!sAllowScale)
        doScale = false;
    if (!sAllowRotate)
        doRotate = false;

    // if no extent is specified, use a 10 meter extent.
    if (NULL == extent)
        {
        // If it's a LL coordinate system, we don't want 10 degrees, we want about 10 meters, and there's about 111,000 meters per degree at the equator.
        if ( (NULL != m_csParameters) && (0 == strcmp(m_csParameters->csdef.unit, "Degree")) )
            localExtent.x = localExtent.y = localExtent.z = m_csParameters->csdef.unit_scl / 10000;
        else
            localExtent.x = localExtent.y = localExtent.z = (m_uorsPerBaseUnit * sReferenceFrameSize);

        extent = &localExtent;
        }

    points[0] = elementOrigin;
    points[1].Init(elementOrigin.x + extent->x, elementOrigin.y, elementOrigin.z);
    points[2].Init(elementOrigin.x, elementOrigin.y + extent->y, elementOrigin.z);
    points[3].Init(elementOrigin.x, elementOrigin.y, elementOrigin.z + extent->z);

    DPoint3d transformedPoints[4];
    ReprojectStatus status = ReprojectUors(transformedPoints, NULL, NULL, points, 4, interpretation, destMstnGCS);

    frameA.InitFrom4Points(points[0], points[1], points[2], points[3]);
    frameAInverse.InverseOf(frameA);

    frameB.InitFrom4Points(transformedPoints[0], transformedPoints[1], transformedPoints[2], transformedPoints[3]);
    RotMatrix axesA, axesB;
    axesA.InitFrom(frameA);
    axesB.InitFrom(frameB);

    // frameB is the direct image of the points.   Modify it per requests ....
    if (doScale)
        {
        if (doRotate)
            {
            // Use frameB unchanged.
            }
        else
            {
            // Get the ratio of x axis lengths.  Apply this to frameA axes, install in frameB.
            // uor scaling effects are already present in the vectors we inspect.
            DVec3d xVectorA, xVectorB;
            axesA.GetColumn(xVectorA, 0);
            axesB.GetColumn(xVectorB, 0);
            double scale = xVectorB.Magnitude() / xVectorA.Magnitude();
            axesB.ScaleColumns(axesA, scale, scale, scale);
            frameB.SetMatrix(axesB);
            }
        }
    else
        {
        // This is the proper length for x if no scaling is applied ....
        double ax = extent->x * destMstnGCS.m_uorsPerBaseUnit / m_uorsPerBaseUnit;
        if (doRotate)
            {
            // Accept the output directions but resize back to sReferenceFrameSize ...
            DVec3d xAxis, yAxis, zAxis;
            axesB.GetColumns(xAxis, yAxis, zAxis);
            double f = ax / xAxis.Magnitude();
            xAxis.Scale(f);
            yAxis.Scale(f);
            zAxis.Scale(f);
            axesB.InitFromColumnVectors(xAxis, yAxis, zAxis);
            frameB.SetMatrix(axesB);
            }
        else
            {
            // No change except uor effects ...
            axesB.InitFromScaleFactors(ax, ax, ax);
            frameB.SetMatrix(axesB);
            }
        }

    // optionally force output frame to be perpendicular
    if (sDoPerpendicularAxes)
        {
        RotMatrix unitB;
        axesB.InitFrom(frameB);
        unitB.SquareAndNormalizeColumns(axesB, 0, 1);
        DVec3d xVectorB;
        axesB.GetColumn(xVectorB, 0);
        double b = xVectorB.Magnitude();
        axesB.ScaleColumns(unitB, b, b, b);
        }


    outTransform->InitProduct(frameB, frameAInverse);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCSP         DgnGCS::FromGeoCoordType66AppData
(
short const*    type66AppData,
DgnDbR          cache
)
    {
    // extract the parameters from the type 66.
    CoordinateSystemDgnFormatter*   csf = CoordinateSystemDgnFormatter::GetInstance();
    GeoCoordType66                  extracted;
    ProjectionParams                projectionParams;
    CSParameters*                   csParameters;
    LocalTransformerP               localTransformer;
    GridFileDefinition              gridFileDefinition("", GridFileFormat::FORMAT_NONE, GridFileDirection::DIRECTION_NONE);

    // first, extract the type 66 into its two components.
    StatusInt   status;
    if (SUCCESS != (status = csf->Extract(extracted, projectionParams, localTransformer, &type66AppData[0])))
        {
        if (GeoCoordError_BadType66Version != status)
            assert(false);
        return NULL;
        }

    double  paperScale = 1.0;

#if defined (NOTNOW)
    if (csf->IsKeyNameCoordinateSystem(extracted.coordsys))
        {
        // when there's a "Key Name" coordinate system, extracted.coordsys will differ from extracted.projType.
        // The information stored in the union is presumably appropriate to extracted.projType, but we should be
        // able to look up the coordinate system data from the coordinate system dictionary. Thus we have redundant
        // information about the coordinate system. I guess we really should use the data stored in the design file,
        // since presumably the coordinates that are in there were placed assuming that information. I think the best
        // thing to do is probably to look up the coordinate system from the key name, and compare to what's in the
        // design file, giving the user the option of either reprojecting or
        }
    else
#endif
        {
        // if not a keyname, then we have the parameters in the type 66, and we populate the CsDef, CsDatum or CsEllipsoid structure from there.
        CSDefinition                    csProjectionDefinition;

        if (SUCCESS != csf->ConvertToCSDef(csProjectionDefinition, paperScale, extracted, projectionParams))
            {
            assert(false);
            return NULL;
            }

        if (SOURCELIB_USER == extracted.dtmOrElpsdFromUserLib)
            {
            CSDatumDef*         datumDefP = NULL;
            CSDatumDef          datumDef;
            CSEllipsoidDef      ellipsoidDef;

            // the datum or ellipsoid is from a user library. When that is the case, we use the GCS datum or ellipsoid information stored in the Type 66 rather than looking it up.
            if (0 != csProjectionDefinition.dat_knm[0])
                {
                // there's a datum.
                WGS84ConvertCode    convertType = (WGS84ConvertCode) extracted.to84_via;
                UNUSED_VARIABLE(convertType);
                assert( (ConvertType_MOLO == convertType) || (ConvertType_3PARM == convertType) || (ConvertType_GEOCTR == convertType) || (ConvertType_BURS == convertType) ||
                         (ConvertType_7PARM == convertType) || (ConvertType_6PARM == convertType) || (ConvertType_4PARM == convertType) || (ConvertType_GENGRID == convertType));

                memset(&datumDef, 0, sizeof(datumDef));
                datumDefP           = &datumDef;
                CSMap::CS_stncp(datumDef.key_nm, csProjectionDefinition.dat_knm, sizeof (datumDef.key_nm));
                CSMap::CS_stncp(datumDef.name, extracted.dat_nm, sizeof (datumDef.name));
                CSMap::CS_stncp(datumDef.ell_knm, csProjectionDefinition.elp_knm, sizeof (datumDef.ell_knm));
                datumDef.to84_via   = static_cast<short>(extracted.to84_via);   // NEEDSWORK - is cast correct?  Should to84_via be a short?
                datumDef.delta_X    = extracted.delta_X;
                datumDef.delta_Y    = extracted.delta_Y;
                datumDef.delta_Z    = extracted.delta_Z;
                datumDef.rot_X      = extracted.rot_X;
                datumDef.rot_Y      = extracted.rot_Y;
                datumDef.rot_Z      = extracted.rot_Z;
                datumDef.bwscale    = extracted.bwscale;
                gridFileDefinition.SetFormat((GridFileFormat)extracted.gridFileFormat);
                gridFileDefinition.SetDirection((GridFileDirection)extracted.gridFileDirection);
                gridFileDefinition.SetFileName(extracted.gridFileName);
                }
            else if (0 == csProjectionDefinition.elp_knm[0])
                {
                assert(false);
                return NULL;
                }

            // we must always have the ellipsoid info.
            memset(&ellipsoidDef, 0, sizeof(ellipsoidDef));
            CSMap::CS_stncp(ellipsoidDef.key_nm, csProjectionDefinition.elp_knm, sizeof (ellipsoidDef.key_nm));
            CSMap::CS_stncp(ellipsoidDef.name, extracted.ell_nm, sizeof (ellipsoidDef.name));
            ellipsoidDef.e_rad  = extracted.e_rad;
            ellipsoidDef.p_rad  = extracted.p_rad;

            // fill in flattening and eccentricity.
            Ellipsoid::CalculateParameters(ellipsoidDef.flat, ellipsoidDef.ecent, ellipsoidDef.e_rad, ellipsoidDef.p_rad);
            if (NULL == (csParameters = CSMap::CScsloc2(&csProjectionDefinition, datumDefP, &ellipsoidDef)))
                {
                assert(false);
                return NULL;
                }
            }

        else if (NULL == (csParameters = CSMap::CScsloc1(&csProjectionDefinition)))
            {
//          char    errorMsg[512];
//          CSMap::CS_errmsg (errorMsg, DIM(errorMsg));
//          printf ("ERROR: %s trying to create from saved type 66 parameters\n", errorMsg);
//          BeAssert(false);
            return NULL;
            }
        }

    DgnGCSP mstnGCS = nullptr;
    if (SOURCELIB_USER == extracted.dtmOrElpsdFromUserLib && ConvertType_GENGRID == (WGS84ConvertCode) extracted.to84_via && gridFileDefinition.GetFormat() != GridFileFormat::FORMAT_NONE)
        {
        GeodeticTransform* transform = GeodeticTransform::CreateGeodeticTransform(gridFileDefinition);
        mstnGCS = new DgnGCS (*csParameters, paperScale, extracted.coordsys, cache, SOURCELIB_USER == extracted.dtmOrElpsdFromUserLib, transform->GetCSGeodeticTransformDef());
        if (nullptr != transform)
            transform->Destroy();
        }
    else
        {
        // from the parameters, construct a new DgnGCS .
        mstnGCS = new DgnGCS (*csParameters, paperScale, extracted.coordsys, cache, SOURCELIB_USER == extracted.dtmOrElpsdFromUserLib);
        }

    // if the VerticalDatumValid is set correctly, extract VerticalDatum code.
    if (GEOCOORD66_VerticalDatumValid == extracted.verticalDatumValid)
        mstnGCS->SetVerticalDatumCode((VertDatumCode) extracted.verticalDatum);

    // set the local transformer.
    mstnGCS->SetLocalTransformer(localTransformer);

#if defined (TEST_TYPE66_CREATE)
    short type66AppData[2000];
    uint32_t type66AppDataBytes;
    s_oldType66             = &extracted;
    s_oldProjectionParams   = &projectionParams;
    mstnGCS->CreateGeoCoordType66(type66AppData, type66AppDataBytes, cache, true);
    s_oldType66             = NULL;
    s_oldProjectionParams   = NULL;
    const uint16_t *t1, *t2;
    int      count;
    for (t1 = (uint16_t*)&type66AppData[429], t2 = (uint16_t*)&test66.applicationElm.appData[429], count=429; count < (GEOCOORD66_SIZE / sizeof(short)); count++, t1++, t2++)
        {
        if (*t1 != *t2)
            printf("difference found at offset %d, existing is 0x%04x, new is 0x%04x\n", count, *t1, *t2);

        }
#endif
    return mstnGCS;
    }

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct      NotFoundAppData: BeSQLite::Db::AppData
{
static BeSQLite::Db::AppData::Key const& GetKey()
    {
    static BeSQLite::Db::AppData::Key s_key;
    return s_key;
    }
};

/*=================================================================================**//**
* @bsiclass
+===============+===============+===============+===============+===============+======*/
struct      DgnGCSAppData: BeSQLite::Db::AppData
{
    DgnGCSPtr m_gcs;

static BeSQLite::Db::AppData::Key const& GetKey()
    {
    static BeSQLite::Db::AppData::Key s_key;
    return s_key;
    }

    DgnGCSAppData(DgnGCS& gcs) : m_gcs(&gcs) {;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCSP         DgnGCS::FromProject(DgnDbR project)
    {
    // See if we already have it cached
    auto saved = (DgnGCSAppData*) project.FindAppData(DgnGCSAppData::GetKey()).get();
    if (NULL != saved)
        return saved->m_gcs.get();

    if (NULL != project.FindAppData(NotFoundAppData::GetKey()).get())
        return NULL;

    // Make sure GCS is initialized
    T_HOST.GetGeoCoordinationAdmin()._GetServices();

    uint32_t propSize;
    if (project.QueryPropertySize(propSize, DgnProjectProperty::DgnGCS()) != BeSQLite::BE_SQLITE_ROW)
        {
        project.AddAppData(NotFoundAppData::GetKey(), new NotFoundAppData());
        return NULL;
        }

    ScopedArray<Byte> buffer(propSize);
    project.QueryProperty(buffer.GetData(), propSize, DgnProjectProperty::DgnGCS());

    auto gcs = FromGeoCoordType66AppData((short const*)buffer.GetData(), project);
    if (NULL == gcs)
        {
        project.AddAppData(NotFoundAppData::GetKey(), new NotFoundAppData());
        return NULL;
        }

        // *** NEEDS WORK: Global origin is not saved, right? I have to get it from the project, don't I?
    gcs->m_globalOrigin = project.GeoLocation().GetGlobalOrigin();

    project.AddAppData(DgnGCSAppData::GetKey(), new DgnGCSAppData(*gcs));
    return gcs;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DgnGCS::CreateGeoCoordType66
(
short*              type66AppData,      // <= filled in with data. Must be at least 2000 words.
uint32_t&           type66AppDataBytes, // <= set to sizeof of type66AppData in bytes
DgnDbR              cache,
bool                primary
) const
    {
    CoordinateSystemDgnFormatter*   csf = CoordinateSystemDgnFormatter::GetInstance();

    VertDatumCode storedVerticalDatum = m_verticalDatum;

    // For historical reason and backward compatibility we store Ellipsoid vertical datum for non-NAD27 and non-NAD83 GCS
    // as value 0 (vdcFromDatum) even though the value is ambiguous for NAD27 and NAD83.
    // Likewise we store value 0 (vdcFromDatum) for NABD88 for a NAD83 GCS
    // Likewise we store value 0 (vdcFromDatum) for NGVD29 for a NAD27 GCS
    if (!IsNAD27() && !IsNAD83() && storedVerticalDatum == vdcEllipsoid)
        storedVerticalDatum = vdcFromDatum;
    if (IsNAD83() && storedVerticalDatum == vdcNAVD88)
        storedVerticalDatum = vdcFromDatum;
    if (IsNAD27() && storedVerticalDatum == vdcNGVD29)
        storedVerticalDatum = vdcFromDatum;

    GridFileDefinition gridFileDefinition("", GridFileFormat::FORMAT_NONE, GridFileDirection::DIRECTION_NONE);

    // Custom datums must not have more than one transform defined
    if (GetDatumCode() == Datum::CUSTOM_DATUM_CODE)
        {
        DatumCP theDatum = GetDatum();
        if (theDatum != nullptr)
            {
            GeodeticTransformPathCP thePath = theDatum->GetGeodeticTransformPathToWGS84();
            if ((thePath != nullptr) && thePath->GetGeodeticTransformCount() > 1)
                return ERROR;
            }
        }
    GetDatumGridFile(gridFileDefinition); // Will return a grid file definition only if appropriate. We ignore error

    bool storeDatumDef = HasCustomDatum();

    // Make sure that if there are a grid file to store the path is not too long.
    if (storeDatumDef && ConvertType_GENGRID == m_csParameters->datum.to84_via &&
        gridFileDefinition.GetFormat() != GridFileFormat::FORMAT_NONE)
        {
        GeoCoordType66      type66;
        if (gridFileDefinition.GetFileName().size() > (DIM(type66.gridFileName) - 1))
            return ERROR;
        }

    return csf->FormatGeoCoordType66(type66AppData, type66AppDataBytes, *m_csParameters, m_coordSysId, cache, primary, m_paperScaleFromType66, storeDatumDef, storedVerticalDatum, m_localTransformer.get(), gridFileDefinition);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DgnGCS::Store(DgnDbR project)
    {
    if (!IsValid())
        return GEOCOORDERR_InvalidCoordSys;

    // make sure the scale and global origin is set right.
    InitCacheParameters(project, m_paperScaleFromType66);

    short type66AppData[2000];
    uint32_t type66AppDataBytes;
    auto status = CreateGeoCoordType66(type66AppData, type66AppDataBytes, project, true);
    if (SUCCESS != status)
        return status;

    status = project.SaveProperty(DgnProjectProperty::DgnGCS(), type66AppData, type66AppDataBytes) == BeSQLite::BE_SQLITE_OK? SUCCESS: ERROR;

    // we have stored a new GCS to the BIM file. Make sure the next time we try to read it, we don't get the GCS that is stored in the DgnAppData.
    if (SUCCESS == status)
        {
        project.DropAppData (DgnGCSAppData::GetKey());
        project.DropAppData (NotFoundAppData::GetKey());
        project.GeoLocation().NewGCSStored ();
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP        DgnGCS::GetProjectionName
(
Utf8StringR    outputBuffer
) const
    {
    DgnProjectionTypes projectionType = BaseGCS::DgnProjectionTypeFromCSDefName (m_csParameters->csdef.prj_knm);

    return BaseGeoCoordResource::GetLocalizedProjectionName(outputBuffer, projectionType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP         DgnGCS::GetDisplayName(Utf8StringR outputBuffer) const
    {
    if ( (NULL != m_csParameters) && (0 != *m_csParameters->csdef.key_nm) )
        {
        outputBuffer = m_csParameters->csdef.key_nm;
        }
    else
        {
        GetProjectionName(outputBuffer);
        }

    if (m_localTransformer.IsValid())
        {
        Utf8String transformerDescription;
        m_localTransformer->GetDescription(transformerDescription);

        if (!transformerDescription.empty())
            {
            outputBuffer.append(" ");
            outputBuffer.append(transformerDescription);
            }
        }

    return outputBuffer.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DgnGCS::GetUnitDefinition
(
UnitDefinitionR unitDef,
StandardUnit&   standardUnitNumber
) const
    {
    standardUnitNumber = StandardUnit::None;

    int     csUnitCode;
    if (-1 == (csUnitCode = GetUnitCode()))
        {
        // should never happen.
        assert(false);
        unitDef.Init(Dgn::UnitBase::Meter, Dgn::UnitSystem::Metric, 1.0, 1.0, "Meters");
        return ERROR;
        }

    // convert from CS units to our UnitInfo.
    cs_Unittab_ const* csUnits = CSMap::GetCSUnitInfo(csUnitCode);

    Dgn::UnitBase base = (cs_UTYP_LEN == csUnits->type) ? Dgn::UnitBase::Meter : Dgn::UnitBase::Degree;

    // CS_Map has only a numerator, no denominator.
    unitDef.Init(base, Dgn::UnitSystem::Undefined, 1.0, csUnits->factor, "");

    standardUnitNumber = unitDef.IsStandardUnit();
    if ( (StandardUnit::None == standardUnitNumber) || (StandardUnit::Custom == standardUnitNumber) )
        {
        Dgn::UnitSystem system;

        if (cs_USYS_Metric == csUnits->system)
            system = Dgn::UnitSystem::Metric;
        else if (cs_USYS_English == csUnits->system)
            system = Dgn::UnitSystem::English;
        else
            system = Dgn::UnitSystem::Undefined;

        unitDef.Init(base, system, 1.0, csUnits->factor, csUnits->name);
        }
    else
        {
        // get the definition if we have one.
        unitDef = UnitDefinition::GetStandardUnit((StandardUnit) standardUnitNumber);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DgnGCS::UnitsIdentical(DgnGCSCR other) const
    {
    if (m_uorsPerBaseUnit != other.m_uorsPerBaseUnit)
        return false;

    return m_globalOrigin.IsEqual(other.m_globalOrigin, 0.000001);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
static bool         unitsSame
(
CSUnitInfo const *csUnitInfo,
UnitDefinitionCR  unitDef
)
    {
    // match the "base".
    if ( (cs_UTYP_LEN == csUnitInfo->type) && (Dgn::UnitBase::Meter != unitDef.GetBase()) )
        return false;

    if ( (cs_UTYP_ANG == csUnitInfo->type) && (Dgn::UnitBase::Degree != unitDef.GetBase()) )
        return false;

    // if it's not LEN or ANG, can't match.
    if ( (cs_UTYP_LEN != csUnitInfo->type) &&  (cs_UTYP_ANG != csUnitInfo->type) )
        return false;

    // adapted from units.c
    double  prod1 = unitDef.GetDenominator();
    double  prod2 = unitDef.GetNumerator() * csUnitInfo->factor;

    return (1.0e-9 * fabs(prod1 + prod2) >= fabs(prod1 - prod2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DgnGCS::GetCSUnitName
(
Utf8StringR         csUnitName,
UnitDefinitionCR    unitDef
)
    {
    for (int icsUnit = 0; ; icsUnit++)
        {
        CSUnitInfo const*  csUnitInfo;
        if (NULL == (csUnitInfo = CSMap::GetCSUnitInfo(icsUnit)))
            break;
        if (unitsSame(csUnitInfo, unitDef))
            {
            csUnitName = csUnitInfo->name;
            return SUCCESS;
            }
        }
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
double          DgnGCS::GetPaperScale () const
    {
    return m_paperScaleFromType66;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DgnGCS::SetPaperScale (double paperScale, DgnDbR project)
    {
    if (0.0 == paperScale)
        return ERROR;

    InitCacheParameters (project, paperScale);
    return SUCCESS;
    }


void DgnGCS::PublishedCreateGeoCoordType66(
short*                  type66AppData,
uint32_t&                 type66AppDataBytes, // <= set to sizeof of type66AppData in bytes
DgnDbR             project,
bool                    primary
) const
    {
    CreateGeoCoordType66(type66AppData, type66AppDataBytes, project, primary);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeoCoordinationAdmin*  DgnGeoCoordinationAdmin::Create(BeFileNameCR dataDirectory)
    {
    return new DgnGeoCoordinationAdmin(dataDirectory);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeoCoordinationAdmin::CompleteInitialization() const
    {
    RUNONCE_CHECK (m_initializationComplete)
    BentleyApi::GeoCoordinates::BaseGCS::Initialize(m_dataDirectory.GetNameUtf8().c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeoCoordinationAdmin::DgnGeoCoordinationAdmin(BeFileNameCR dataDirectory)
    {
    m_initializationComplete  = false;
    m_gcrp                    = NULL;
    m_dataDirectory = dataDirectory;
    m_otfEnabled = false;
    }
