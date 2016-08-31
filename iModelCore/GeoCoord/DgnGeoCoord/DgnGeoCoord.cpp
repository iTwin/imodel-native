/*----------------------------------------------------------------------+
|
|   $Source: DgnGeoCoord/DgnGeoCoord.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#include    <DgnPlatform\DgnPlatformApi.h>
#include    <Geom\GeomApi.h>
#include    <DgnGeoCoord\DgnGeoCoord.h>
#include    <DgnGeoCoord\DgnGeoCoordApi.h>
#include    <DgnGeoCoord\GeoCoordErrors.h>
#include    <DgnPlatform\Tools\ConfigurationManager.h>
#include    <DgnPlatform\Tools\fileutil.h>
#include    <cs_map.h>
#include    "geocoordelement.h"
#include    <GeoCoord\GCSLibrary.h>
#include    <RmgrTools\Tools\UglyStrings.h>

// tolerance is from old geocoord code
#define GEOCOORD_CLOSE_TOLERANCE (1e-5)
#define GEOCOORD66_SIZE (sizeof(GeoCoordType66) + 80*sizeof(double))
#define GEOCOORD66_VerticalDatumValid     (0x8117)
#if defined (TEST_TYPE66_CREATE)
static GeoCoordType66*     s_oldType66;
static ProjectionParams*   s_oldProjectionParams;
#endif

USING_NAMESPACE_BENTLEY
USING_NAMESPACE_BENTLEY_DGNPLATFORM

using namespace Bentley::GeoCoordinates;

#define RUNONCE_CHECK(var) {if (var) return; var=true;}
#define DIM(a) ((sizeof(a)/sizeof((a)[0])))
namespace Bentley {

#define GEOCOORD_ACS_EXTENDERID     0x04151956
#define GEOCOORD_MGRS_EXTENDERID    0x03271957

namespace GeoCoordinates {


/*=================================================================================**//**
* Formatter for reading/writing the Type 66 element.
* @bsiclass                                                     Barry.Bentley   10/06
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
* @bsimethod                                                    Barry.Bentley   10/06
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
* @bsimethod                                                    Barry.Bentley   10/06
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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
public:
static  CoordinateSystemDgnFormatter *GetInstance()
    {
    if (NULL == s_instance)
        s_instance = new CoordinateSystemDgnFormatter();

    return s_instance;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       UpgradeType66Data
(
GeoCoordType66      &extracted,
ProjectionParams    &projectionParams
)
    {
    // This crap taken directly from the old geocoordinate code.
    /* May 1998 */
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
            utmzn.hemisphere = (0 == _stricmp (extracted.grp_knm, "UTMS")) ? utmzn.hemisphere = OBE_HEMISPHERE_SOUTH : OBE_HEMISPHERE_NORTH;
            utmzn.gcDom     = projectionParams.trmer.gcDom;
            utmzn.paper_scl = projectionParams.trmer.paper_scl;
            utmzn.quad      = projectionParams.trmer.quad;
            CSMap::CS_stncp (utmzn.unit_nm, projectionParams.trmer.unit_nm, DIM (utmzn.unit_nm));
            CSMap::CS_stncp (utmzn.dat_knm, projectionParams.trmer.dat_knm, DIM (utmzn.dat_knm));
            CSMap::CS_stncp (utmzn.ell_knm, projectionParams.trmer.ell_knm, DIM (utmzn.ell_knm));

            /* Copy the whole thing into the projectionParams union. */
            projectionParams.utmzn = utmzn;
            }
        else if (COORDSYS_STERO == extracted.coordsys)
            {
            if (fabs (projectionParams.stero.org_lat - 90.0) < GEOCOORD_CLOSE_TOLERANCE)
                {
                /* Format is identical */
                extracted.prj_code  = cs_PRJCOD_PSTRO;
                extracted.projType  =  COORDSYS_PSTRO;
                strcpy (extracted.prj_knm, CS_PSTRO);
                }
            else
                {
                extracted.prj_code  = cs_PRJCOD_SSTRO;
                extracted.projType  =  COORDSYS_STERO;
                strcpy (extracted.prj_knm, CS_STERO);
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
            if ( (fabs (projectionParams.unity.domLL.min_lng) > GEOCOORD_CLOSE_TOLERANCE) ||
                 (fabs (projectionParams.unity.domLL.max_lng) > GEOCOORD_CLOSE_TOLERANCE) )
                {
                if ( (fabs (projectionParams.unity.domLL.min_lat) > GEOCOORD_CLOSE_TOLERANCE) &&
                     (fabs (projectionParams.unity.domLL.max_lat) > GEOCOORD_CLOSE_TOLERANCE) )
                    {
                    projectionParams.unity.domLL.min_lat = -90;
                    projectionParams.unity.domLL.max_lat =  90;
                    }
                }
            else if ( (fabs (projectionParams.unity.domLL.min_lat) > GEOCOORD_CLOSE_TOLERANCE) ||
                      (fabs (projectionParams.unity.domLL.max_lat) > GEOCOORD_CLOSE_TOLERANCE) )
                {
                if ( (fabs (projectionParams.unity.domLL.min_lng) > GEOCOORD_CLOSE_TOLERANCE) &&
                     (fabs (projectionParams.unity.domLL.max_lng) > GEOCOORD_CLOSE_TOLERANCE) )
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
* @bsimethod                                                    Barry.Bentley   10/06
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
    memset ((void*)&extracted, 0, sizeof (extracted));
    memset ((void*)&projectionParams, 0, sizeof (projectionParams));
    localTransformer = NULL;

    StatusInt   status = SUCCESS;

    // this used to use the resource conversion logic, but it turns out that the structure was set up
    // correctly, grouping the doubles and filling in the holes so just requires a memcpy.
    memcpy (&extracted, &type66AppData[1], sizeof (extracted));

    // check the version and size. In the old GeoCoord code, the type 66 was accepted unless its version was newer than GCOORD_CURRENT_VERSION, so do that here, too.
    if (extracted.version > GCOORD_08119_VERSION)
        return GeoCoordError_BadType66Version;

    // The ProjectionParams union was also set up correctly to allow a simple memcpy for every type of projection.
    memcpy (&projectionParams, &type66AppData[429], sizeof (projectionParams));

#if defined (NOTNOW)
    if (IsKeyNameCoordinateSystem (extracted.coordsys))
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
        UpgradeType66Data (extracted, projectionParams);

    if (GCOORD_08119_VERSION == extracted.version)
        localTransformer = LocalTransformer::CreateLocalTransformer ((LocalTransformType)extracted.localTransformType, extracted.transformParams);
    else
        extracted.dtmOrElpsdFromUserLib = 0;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
private:
void            CommonParamsToCSDef
(
CSDefinition        &csDef,
GeoCoordType66      &extracted
)
    {
    csDef.protect = static_cast<short>(extracted.protect); // NEEDSWORK - is cast correct?  Should protect be a short?

    CSMap::CS_stncp (csDef.group,   extracted.grp_knm, DIM(csDef.group));
    CSMap::CS_stncp (csDef.prj_knm, extracted.prj_knm, DIM(csDef.prj_knm));
    CSMap::CS_stncp (csDef.source,  extracted.source, DIM(csDef.source));
    CSMap::CS_stncp (csDef.desc_nm, extracted.desc_nm, DIM(csDef.desc_nm));

    // I think this will be ignored by cs_map (unless we do cs_csloc, which we don't). I put into the structure
    //   so that it will be retained and then saved back to the type 66, as that what I found in all existing type 66's.
    CSMap::CS_stncp (csDef.key_nm,  extracted.cs_knm, DIM(csDef.key_nm));

#if 0
    // truncate desc_nm at comma. (from old version, claims it was stripping unit name).
    char*   pComma;
    if (NULL  != (pComma = strrchr (csDef.desc_nm, ',')))
        *pComma = 0;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            AlbersToCsDef                   // COORDSYS_ALBER
(
CSDefinition        &csDef,
Proj_alber          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_ALBER, DIM(csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            AzimuthalEquidistantElevatedToCsDef     // COORDSYS_AZEDE
(
CSDefinition        &csDef,
Proj_azede          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_AZEDE, DIM(csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            AzimuthalEqualAreaToCsDef       // COORDYS_AZMEA
(
CSDefinition        &csDef,
Proj_azmea          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_AZMEA, DIM(csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.prj_prm1  = params.az;
    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            AzimuthalEquidistantToCsDef     // COORDSYS_AZMED
(
CSDefinition        &csDef,
Proj_azmed          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_AZMED, DIM(csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.prj_prm1  = params.az;
    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            BonneToCsDef                    // COORDSYS_BONNE
(
CSDefinition        &csDef,
Proj_bonne          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_BONNE, DIM(csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            BipolarObliqueConformalConicToCsDef             // COORDSYS_BPCNC
(
CSDefinition        &csDef,
Proj_bpcnc          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_BPCNC, DIM(csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            CassiniToCsDef                  // COORDSYS_CSINI
(
CSDefinition        &csDef,
Proj_csini          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_CSINI, DIM(csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.prj_prm1  = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            EquidistantConicToCsDef         // COORDSYS_EDCNC
(
CSDefinition        &csDef,
Proj_edcnc          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_EDCNC, DIM(csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            EquidistantCylindricalToCsDef   // COORDSYS_EDCYL, COORDSYS_EDCYLE
(
CSDefinition        &csDef,
Proj_edcyl          &params,
CharCP            csMapCoordName
)
    {
    CSMap::CS_stncp (csDef.prj_knm, csMapCoordName, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.prj_prm1  = params.std_lat;
    csDef.org_lat   = params.org_lat;
    csDef.org_lng   = params.org_lng;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Alain.Robert   02/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            PlateCarreeToCsDef   // COORDSYS_PCARREE
(
CSDefinition        &csDef,
Proj_pcarree         &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_PCARREE, DIM(csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.org_lat   = params.org_lat;
    csDef.org_lng   = params.org_lng;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            Eckert4ToCsDef                  // COORDSYS_EKRT4
(
CSDefinition        &csDef,
Proj_ekrt4          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_EKRT4, DIM(csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.prj_prm1  = params.org_lng;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            Eckert6ToCsDef                  // COORDSYS_EKRT6
(
CSDefinition        &csDef,
Proj_ekrt6          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_EKRT6, DIM(csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.prj_prm1  = params.org_lng;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            TransverseMercatorToCsDef       // COORDSYS_GAUSK
(
CSDefinition        &csDef,
Proj_trmer          &params,
CharCP            csMapCoordName
)
    {
    CSMap::CS_stncp (csDef.prj_knm, csMapCoordName, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.prj_prm1  = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.scl_red   = params.scl_red;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            TransverseMercatorMinnesotaToCsDef              // COORDSYS_TMMIN
(
CSDefinition        &csDef,
Proj_tmmin          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_TMMIN, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            TransverseMercatorWisconsinToCsDef              // COORDSYS_TMWIS
(
CSDefinition        &csDef,
Proj_tmwis          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_TMWIS, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            TransverseMercatorAffinePostProcessToCsDef      // COORDSYS_TRMAF
(
CSDefinition        &csDef,
Proj_trmaf          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_TRMAF, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            UniversalTransverseMercatorZoneToCsDef          // COORDSYS_UTMZN
(
CSDefinition        &csDef,
Proj_utmzn          &params,
CharCP            csMapCoordName
)
    {
    CSMap::CS_stncp (csDef.prj_knm, csMapCoordName, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.prj_prm1  = params.zoneNo;
    csDef.prj_prm2  = (OBE_HEMISPHERE_NORTH == params.hemisphere ? 1 : -1); // Any positive is North, any negative is south (0 is interpreted as North)

    // Due to a small mixup (see TR# 261800) sometimes the hemisphere of the UTM defined coordinate system is incorrectyl interpreted
    // Since the code was fixed this only applies to coordinate system stored for the very first released version of v8i of MicroStation
    // For compatibility of coordinate stored using this version we extract the CS definition from the dictionary if it exists and make sure the
    // hemisphere definition fit. If not then likely the UTM definition was incorrectly stored and we corect it internally (not re-stored)
    if (strlen(csDef.key_nm) > 0)
        {
        try
            {
            WString keyName (csDef.key_nm);
            BaseGCSPtr tempGCS = BaseGCS::CreateGCS(keyName.c_str());
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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            GnomicToCsDef                   // COORDSYS_GNOMC
(
CSDefinition        &csDef,
Proj_gnomc          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_GNOMC, DIM(csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            HomolsineToCsDef                // COORDSYS_HMLSN
(
CSDefinition        &csDef,
Proj_hmlsn          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_HMLSN, DIM(csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    ZonesToCsDef (&csDef.prj_prm1, params.zone, params.nZones);
    csDef.org_lng   = params.org_lng;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            ObliqueMercator1PointToCsDef    // COORDSYS_HOM1U, COORDSYS_OBLQ1, COORDSYS_RSKEW, COORDSYS_RSKWC, COORDSYS_RSKWO
(
CSDefinition        &csDef,
Proj_Oblq1          &params,
CharCP              csMapCoordName
)
    {
    CSMap::CS_stncp (csDef.prj_knm, csMapCoordName, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

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
* @bsimethod                                                    Alain.Robert   2011/05
+---------------+---------------+---------------+---------------+---------------+------*/
void            MinnesotaDOTObliqueMercatorToCsDef    // COORDSYS_MNDOTOBL
(
CSDefinition        &csDef,
Proj_Mndotobl       &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_MNDOTOBL, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            ObliqueMercator2PointToCsDef    // COORDSYS_HOM2U, COORDSYS_OBLQ2
(
CSDefinition        &csDef,
Proj_Oblq2          &params,
CharCP              csMapCoordName
)
    {
    CSMap::CS_stncp (csDef.prj_knm, csMapCoordName, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            HungarianEOVToCsDef             // COORDSYS_HUEOV
(
CSDefinition        &csDef,
Proj_Hueov          &params
)
    {
    // No CSMap documentation, looking at old geocoord stuff.
    CSMap::CS_stncp (csDef.prj_knm, CS_HUEOV, DIM(csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            KrovakToCsDef                   // COORDSYS_KRVKG,COORDSYS_KRVKP, COORDSYS_KRVK
(
CSDefinition        &csDef,
Proj_Krvak          &params,
CharCP         csMapCoordName
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord code.
    CSMap::CS_stncp (csDef.prj_knm, csMapCoordName, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

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
        CSMap::CS_stncp (csDef.prj_knm, CS_KRVK95, DIM (csDef.prj_knm));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            LambertTangentialToCsDef        // COORDSYS_LM1SP, COORDSYS_LMTAN
(
CSDefinition        &csDef,
Proj_Lmtan          &params,
CharCP            csMapCoordName
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord cold.
    CSMap::CS_stncp (csDef.prj_knm, csMapCoordName, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.scl_red   = params.scl_red;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            LambertConformalConicToCsDef    // COORDSYS_LMBRT, COORDSYS_LMBLG
(
CSDefinition        &csDef,
Proj_Lmbrt          &params,
CharCP         csMapCoordName
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord cold.
    CSMap::CS_stncp (csDef.prj_knm, csMapCoordName, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            LambertConformalConicMinnesotaToCsDef           // COORDSYS_LMMIN
(
CSDefinition        &csDef,
Proj_Lmmin          &params
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord code.
    CSMap::CS_stncp (csDef.prj_knm, CS_LMMIN, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            LambertConformalConicWisconsinToCsDef           // COORDSYS_LMWIS
(
CSDefinition        &csDef,
Proj_Lmwis          &params
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord code.
    CSMap::CS_stncp (csDef.prj_knm, CS_LMWIS, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            LambertAffinePostProcessToCsDef                 // COORDSYS_LMBRTAF
(
CSDefinition        &csDef,
Proj_lmbrtaf        &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_LMBRTAF, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            MillerToCsDef                   // COORDSYS_MILLR
(
CSDefinition        &csDef,
Proj_millr          &params
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord cold.
    CSMap::CS_stncp (csDef.prj_knm, CS_MILLR, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.prj_prm1  = params.org_lng;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            ModifiedPolyconicToCsDef        // COORDSYS_MODPC
(
CSDefinition        &csDef,
Proj_modpc          &params
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord cold.
    CSMap::CS_stncp (csDef.prj_knm, CS_MODPC, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            MollweideToCsDef                // COORDSYS_MOLWD
(
CSDefinition        &csDef,
Proj_molwd          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_MOLWD, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    ZonesToCsDef (&csDef.prj_prm1, params.zone, params.nZones);
    csDef.org_lng   = params.org_lng;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            MercatorToCsDef                 // COORDSYS_MRCAT
(
CSDefinition        &csDef,
Proj_mrcat          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_MRCAT, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.prj_prm1  = params.org_lng;
    csDef.prj_prm2  = params.std_lat;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            MercatorScaleReductionToCsDef   // COORDSYS_MRCSR
(
CSDefinition        &csDef,
Proj_mrcsr          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_MRCSR, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.prj_prm1  = params.org_lng;
    csDef.scl_red   = params.scl_red;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            PopularVisualizationMercatorToCsDef                 // COORDSYS_MRCATPV
(
CSDefinition        &csDef,
Proj_mrcatpv        &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_MRCATPV, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.prj_prm1  = params.org_lng;
    
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            ModifiedStereographicToCsDef    // COORDSYS_MSTRO
(
CSDefinition        &csDef,
Proj_mstro          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_MSTRO, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.scl_red   = params.scl_red;

    // copy the imaginary numbers to prj_prm1 - prj_prm24
    memcpy (&csDef.prj_prm1, params.ABary, 24*sizeof(double));

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            NormalAspectAuthalicCylindricalToCsDef          // COORDSYS_NACYL
(
CSDefinition        &csDef,
Proj_nacyl          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_NACYL, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.prj_prm1  = params.std_lat;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            NonEarthToCsDef                 // COORDSYS_NERTH
(
CSDefinition        &csDef,
Proj_nerth          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_NERTH, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            NonEarthScaleRotationTranslationToCsDef         // COORDSYS_NESRT
(
CSDefinition        &csDef,
Proj_nesrt          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_NESRT, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.prj_prm1  = params.scale;
    csDef.prj_prm2  = params.rotateDeg;
    csDef.prj_prm3  = params.x_origin;
    csDef.prj_prm4  = params.y_origin;

    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            NewZealandToCsDef               // COORDSYS_NZLND
(
CSDefinition        &csDef,
Proj_nzlnd          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_NZLND, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.scl_red   = 1.0;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            OrthographicToCsDef             // COORDSYS_ORTHO
(
CSDefinition        &csDef,
Proj_ortho          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_ORTHO, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            StereographicToCsDef            // COORDSYS_OSTRO, COORDSYS_PSTRO, COORDSYS_STERO
(
CSDefinition        &csDef,
Proj_stero          &params,
CharCP            csMapCoordName
)
    {
    CSMap::CS_stncp (csDef.prj_knm, csMapCoordName, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

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
* @bsimethod                                                    Alain.Robert   02/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            Ost97ToCsDef            // COORDSYS_OST97
(
CSDefinition        &csDef,
Proj_ost97          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_OST97, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);
    csDef.map_scl   = params.paper_scl;
    csDef.quad      = (short) params.quad;

    // In this projection, everything is hardcoded yet upon creation of the CSMAP structure (csloc1) it requires at the very least
    // the unit name and either the datum name or ellipsoid name. Since none is usually stored we set these to the
    // hard coded values. Take note also that previous version of GCS system (GeoGraphics/Map) did store this information so for
    // backward compatibility we should probably even though not useful (see Ost97FromCsDef)
    if (strlen (csDef.dat_knm) == 0)
        CSMap::CS_stncp (csDef.dat_knm, "WGS84", DIM (csDef.dat_knm));
    if (strlen (csDef.elp_knm) == 0)
        CSMap::CS_stncp (csDef.elp_knm, "WGS84", DIM (csDef.elp_knm));
    if (strlen (csDef.unit) == 0)
        CSMap::CS_stncp (csDef.unit, "METER", DIM (csDef.unit));
    if (0 == csDef.map_scl)
        csDef.map_scl = 1.0;
    if (0 == csDef.quad)
        csDef.quad = 1;

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Alain.Robert   02/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            Ost02ToCsDef            // COORDSYS_OST02
(
CSDefinition        &csDef,
Proj_ost97          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_OST02, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);
    csDef.map_scl   = params.paper_scl;
    csDef.quad      = (short) params.quad;

    // In this projection, everything is hardcoded yet upon creation of the CSMAP structure (csloc1) it requires at the very least
    // the unit name and either the datum name or ellipsoid name. Since none is usually stored we set these to the
    // hard coded values. Take note also that previous version of GCS system (GeoGraphics/Map) did store this information so for
    // backward compatibility we should probably even though not useful(see Ost02FromCsDef)
    if (strlen (csDef.dat_knm) == 0)
        CSMap::CS_stncp (csDef.dat_knm, "WGS84", DIM (csDef.dat_knm));
    if (strlen (csDef.elp_knm) == 0)
        CSMap::CS_stncp (csDef.elp_knm, "WGS84", DIM (csDef.elp_knm));
    if (strlen (csDef.unit) == 0)
        CSMap::CS_stncp (csDef.unit, "METER", DIM (csDef.unit));
    if (0 == csDef.map_scl)
        csDef.map_scl = 1.0;
    if (0 == csDef.quad)
        csDef.quad = 1;

    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            AmericanPolyconicToCsDef        // COORDSYS_PLYCN
(
CSDefinition        &csDef,
Proj_plycn          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_PLYCN, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.prj_prm1  = params.org_lng;
    csDef.org_lat   = params.org_lat;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            PolarStereographicStandardLatToCsDef            // COORDSYS_PSTSL:
(
CSDefinition        &csDef,
Proj_pstsl          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_PSTSL, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.prj_prm1  = params.std_lat;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            RobinsonToCsDef                 // COORDSYS_ROBIN
(
CSDefinition        &csDef,
Proj_robin          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_ROBIN, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            SinusoidalToCsDef               // COORDSYS_SINUS
(
CSDefinition        &csDef,
Proj_sinus          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_SINUS, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    ZonesToCsDef (&csDef.prj_prm1, params.zone, params.nZones);
    csDef.org_lng   = params.org_lng;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            SwissToCsDef                    // COORDSYS_SWISS
(
CSDefinition        &csDef,
Proj_swiss          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_SWISS, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            Danish34ToCsDef                  // COORDSYS_SYS34, COORDSYS_S3499, COORDSYS_S3401
(
CSDefinition        &csDef,
Proj_sys34          &params,
CharCP            csMapCoordName
)
    {
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.prj_knm, csMapCoordName, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.prj_prm1  = params.zoneNo;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            TransverseAuthalicCylindricalToCsDef            // COORDSYS_TACYL
(
CSDefinition        &csDef,
Proj_tacyl          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_TACYL, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.org_lat   = params.org_lat;
    csDef.scl_red   = params.scl_red;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            VanDerGrintenToCsDef            // COORDSYS_VDGRN
(
CSDefinition        &csDef,
Proj_vdgrn          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_VDGRN, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            WinkelTripelToCsDef             // COORDSYS_WINKT
(
CSDefinition        &csDef,
Proj_winkt          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_WINKT, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));
    DomainToCsDef (csDef, params.gcDom);

    csDef.org_lng   = params.org_lng;
    csDef.prj_prm1  = params.std_lat;

    csDef.map_scl   = params.paper_scl;
    csDef.x_off     = params.x_off;
    csDef.y_off     = params.y_off;
    csDef.quad      = (short) params.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            GeographicToCsDef               // COORDSYS_Geographic
(
CSDefinition        &csDef,
Proj_unity          &params
)
    {
    CSMap::CS_stncp (csDef.prj_knm, CS_UNITY, DIM (csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, params.unit_nm, DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, params.dat_knm, DIM(csDef.dat_knm));
    CSMap::CS_stncp (csDef.elp_knm, params.ell_knm, DIM(csDef.elp_knm));

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
    if (0 != _stricmp (pCsDef->unit, STRING_DEGREE))
        {
        /* CSMap structures carry range in units of the coordinate system.  Convert it if necessary */
        status = ucrdConvert_distanceDegreesToAngular (&pCsDef->prj_prm1, pCsDef->prj_prm1, pCsDef->unit);
        status = ucrdConvert_distanceDegreesToAngular (&pCsDef->prj_prm2, pCsDef->prj_prm2, pCsDef->unit);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProjectionParamsToCSDef
(
CSDefinition        &csDef,
double              &paperScale,
Int32               projectionType,
ProjectionParams    &params
)
    {
    switch (projectionType)
        {
        case COORDSYS_ALBER:    // Albers equal area projection
            AlbersToCsDef (csDef, params.alber);
            break;

        case COORDSYS_AZEDE:    // Azimuthal Equidistant Projection with Elevated Ellipsoid.
            AzimuthalEquidistantElevatedToCsDef (csDef, params.azede);
            break;

        case COORDSYS_AZMEA:    // Azimuthal Equal Area Projection.
            AzimuthalEqualAreaToCsDef (csDef, params.azmea);
            break;

        case COORDSYS_AZMED:    // Azimuthal Equidistant Projection.
            AzimuthalEquidistantToCsDef (csDef, params.azmed);
            break;

        case COORDSYS_BONNE:    // Bonne Projection
            BonneToCsDef (csDef, params.bonne);
            break;

        case COORDSYS_BPCNC:    // Bipolar Oblique Conic Projection
            BipolarObliqueConformalConicToCsDef (csDef, params.bpcnc);
            break;

        case COORDSYS_CSINI:    // Cassini Cylindrical Projection
            CassiniToCsDef (csDef, params.csini);
            break;

        case COORDSYS_EDCNC:    // Equidistant Conic Projection
            EquidistantConicToCsDef (csDef, params.edcnc);
            break;

        case COORDSYS_EDCYL:    // Equidistant Cylindrical Projection
            EquidistantCylindricalToCsDef (csDef, params.edcyl, CS_EDCYL);
            break;

        case COORDSYS_EKRT4:    // Eckert IV Projection
            Eckert4ToCsDef (csDef, params.ekrt4);
            break;

        case COORDSYS_EKRT6:    // Eckert VI Projection
            Eckert6ToCsDef (csDef, params.ekrt6);
            break;

        case COORDSYS_GAUSK:    // Gauss Kruger Projection (Transvers Mercator variant)
            TransverseMercatorToCsDef (csDef, params.trmer, CS_GAUSK);
            break;

        case COORDSYS_GNOMC:    // Gnomic Projection
            GnomicToCsDef (csDef, params.gnomc);
            break;

        case COORDSYS_HMLSN:    // Homolsine Projection
            HomolsineToCsDef (csDef, params.hmlsn);
            break;

        case COORDSYS_HOM1U:    // Hotine Oblique Mercator 1 Point Unrectified Projection (CSMap says rarely used).
        case COORDSYS_OBLQ1:    // CSMap calls it HOM1XY - Alask Variation, Rectified
            ObliqueMercator1PointToCsDef (csDef, params.oblq1, (COORDSYS_HOM1U == projectionType) ? CS_HOM1U : CS_OBLQ1);
            break;

        case COORDSYS_HOM2U:    // Hotine Oblique Mercator 1 Point Unrectified Projection (CSMap says rarely used).
        case COORDSYS_OBLQ2:
            ObliqueMercator2PointToCsDef (csDef, params.oblq2, (COORDSYS_HOM1U == projectionType) ? CS_HOM2U : CS_OBLQ2);
            break;

        case COORDSYS_HUEOV:    // Hungarian EOV Projection.
            HungarianEOVToCsDef (csDef, params.hueov);
            break;

        case COORDSYS_KRVKG:    // Krovak Oblique Conformal Conic
            KrovakToCsDef (csDef, params.krvkg, CS_KRVKG);
            break;
        case COORDSYS_KRVKP:
            KrovakToCsDef (csDef, params.krvkg, CS_KRVKP);
            break;
        case COORDSYS_KRVKR:
            KrovakToCsDef (csDef, params.krvkg, CS_KRVKR);
            break;

        case COORDSYS_LM1SP:    // Lambert Tangential Projection
        case COORDSYS_LMTAN:
            LambertTangentialToCsDef (csDef, params.lmtan, (COORDSYS_LM1SP == projectionType) ? CS_LM1SP : CS_LMTAN);
            break;

        case COORDSYS_LMBRT:    // Lambert Conformal Conic Projection
        case COORDSYS_LMBLG:    // Belgium variation
            LambertConformalConicToCsDef (csDef, params.lmbrt, (COORDSYS_LMBRT == projectionType) ? CS_LMBRT : CS_LMBLG);;
            break;

        case COORDSYS_LMMIN:    // Lambert Conformal Conic Projection, Minnesota variation
            LambertConformalConicMinnesotaToCsDef (csDef, params.lmmin);
            break;

        case COORDSYS_LMWIS:    // Lambert Conformal Conic Projection, Minnesota variation
            LambertConformalConicWisconsinToCsDef (csDef, params.lmwis);
            break;

        case COORDSYS_MILLR:    // Miller Projection
            MillerToCsDef (csDef, params.millr);
            break;

        case COORDSYS_MODPC:    // Modified Polyconic Projection
            ModifiedPolyconicToCsDef (csDef, params.modpc);
            break;

        case COORDSYS_MOLWD:    // Mollweide Projection
            MollweideToCsDef (csDef, params.molwd);
            break;

        case COORDSYS_MRCAT:    // Mercator Projection
            MercatorToCsDef (csDef, params.mrcat);
            break;

        case COORDSYS_MRCSR:    // Mercator Projection with Scale Reduction
            MercatorScaleReductionToCsDef (csDef, params.mrcsr);
            break;

        case COORDSYS_MSTRO:    // Modified Stereographic Projection
            ModifiedStereographicToCsDef (csDef, params.mstro);
            break;

        case COORDSYS_NACYL:    // Normal Aspect Authalic Cylindrical Projection
            NormalAspectAuthalicCylindricalToCsDef (csDef, params.nacyl);
            break;

        case COORDSYS_NERTH:    // Non-earth Projection
            NonEarthToCsDef (csDef, params.nerth);
            break;

        case COORDSYS_NESRT:    // Non-earth with scale, rotation, and translation
            NonEarthScaleRotationTranslationToCsDef (csDef, params.nesrt);
            break;

        case COORDSYS_NZLND:    // New Zealand Projection
            NewZealandToCsDef (csDef, params.nzlnd);
            break;

        case COORDSYS_ORTHO:    // Orthographic Projection
            OrthographicToCsDef (csDef, params.ortho);
            break;

        case COORDSYS_OSTRO:    // Oblique Stereographic Projection
            StereographicToCsDef (csDef, params.ostro, CS_OSTRO);
            break;

        case COORDSYS_PSTRO:    // Polar Sterographic Projection
            StereographicToCsDef (csDef, params.ostro, CS_PSTRO);
            break;

        case COORDSYS_OST97:    // Ordinance Survey Grid Transformation of 1997
            // This is a variation of Transverse Mercator (see csmap documentation) but there are no parameters - everything is hard coded.
            // yet for backward compatibility and due to a csmap issue we must still store as much as possible (datum, unit, ellipsoid...)
            Ost97ToCsDef (csDef, params.ost97);
            break;

        case COORDSYS_OST02:    // Ordinance Survey Grid Transformation of 2002
            // This is a variation of Transverse Mercator (see csmap documentation) but there are no parameters - everything is hard coded.
            // yet for backward compatibility and due to a csmap issue we must still store as much as possible (datum, unit, ellipsoid...)
            Ost02ToCsDef (csDef, params.ost97);
            break;

        case COORDSYS_PLYCN:    // American Polyconic Projection
            AmericanPolyconicToCsDef (csDef, params.plycn);
            break;

        case COORDSYS_PSTSL:    // Polar Stereographic with Standard Latitude
            PolarStereographicStandardLatToCsDef (csDef, params.pstsl);
            break;

        case COORDSYS_ROBIN:    // Robinson Projection
            RobinsonToCsDef (csDef, params.robin);
            break;

        case COORDSYS_RSKEW:    // Rectified Skew Orthomorphic
            ObliqueMercator1PointToCsDef (csDef, params.oblq1, CS_RSKEW);
            break;

        case COORDSYS_RSKWC:    // Rectified Skew Orthomorphic Centered
            ObliqueMercator1PointToCsDef (csDef, params.oblq1, CS_RSKWC);
            break;

        case COORDSYS_RSKWO:    // Rectified Skew Orthomorphic Origin
            ObliqueMercator1PointToCsDef (csDef, params.oblq1, CS_RSKWO);
            break;

        case COORDSYS_SINUS:    // Sinusoidal Projection
            SinusoidalToCsDef (csDef, params.sinus);
            break;

        case COORDSYS_SOTRM:    // Transverse Mercator South Oriented Projection
            TransverseMercatorToCsDef (csDef, params.trmer, CS_SOTRM);
            break;

        case COORDSYS_STERO:    // Stereographic Projection
            StereographicToCsDef (csDef, params.stero, CS_STERO);
            break;


        case COORDSYS_SWISS:    // Oblique CylindricalSwiss Projection
            SwissToCsDef (csDef, params.swiss);
            break;

        case COORDSYS_SYS34:    // Danish System 34
        case COORDSYS_S3499:    // Danish System 34 with 1999 Polynomial.
        case COORDSYS_S3401:    // Danish System 34 with 2001 Polynomial.
            Danish34ToCsDef (csDef, params.sys34, (COORDSYS_SYS34 == projectionType) ? CS_SYS34 : ((COORDSYS_S3499 == projectionType) ? CS_S3499 : CS_S3401));
            break;

        case COORDSYS_TACYL:    // Transverse Authalic Cylindrical Projection
            TransverseAuthalicCylindricalToCsDef (csDef, params.tacyl);
            break;

        case COORDSYS_TMMIN:    // Transverse Mercator, Minnesota
            TransverseMercatorMinnesotaToCsDef (csDef, params.tmmin);
            break;

        case COORDSYS_TMWIS:    // Transverse Mercator, Wisconsin County
            TransverseMercatorWisconsinToCsDef (csDef, params.tmwis);
            break;

        case COORDSYS_TRMAF:    // Transverse Mercator with Affine Post Process
            TransverseMercatorAffinePostProcessToCsDef (csDef, params.trmaf);
            break;

        case COORDSYS_TRMER:    // Transverse Mercator Projection
            TransverseMercatorToCsDef (csDef, params.trmer, CS_TRMER);
            break;

        case COORDSYS_TMKRG:    // Transverse Mercator Projection
            TransverseMercatorToCsDef (csDef, params.trmer, CS_TMKRG);
            break;

        case COORDSYS_UTMZN:    // Universal Tranverse Mercator Zone Projections
            UniversalTransverseMercatorZoneToCsDef (csDef, params.utmzn, CS_UTMZN);
            break;

        case COORDSYS_Geographic:   // Unity (Lat/Long) pseudo-projection.
            GeographicToCsDef (csDef, params.unity);
            break;

        case COORDSYS_VDGRN:        // Van der Grinten Projection
            VanDerGrintenToCsDef (csDef, params.vdgrn);
            break;

        case COORDSYS_WINKT:        // Winkel-Tripel Projection
            WinkelTripelToCsDef (csDef, params.winkt);
            break;

        case COORDSYS_LMBRTAF:      // Lambert with Affine Post Processor
            LambertAffinePostProcessToCsDef (csDef, params.lmbrtaf);
            break;

        // TOTAL_SPECIAL
        case COORDSYS_UTMZNBF:     // UTM Zone, using TOTal BF calculation
            UniversalTransverseMercatorZoneToCsDef (csDef, params.utmzn, CS_UTMZNBF);
            break;

        case COORDSYS_TRMERBF:     // Transverse Mercator, using TOTAL BF calculation
            TransverseMercatorToCsDef (csDef, params.trmer, CS_TRMERBF);
            break;

        case COORDSYS_EDCYLE:     // Equidistant Cylindrical Ellipsoid or Spherical form
            EquidistantCylindricalToCsDef (csDef, params.edcyl, CS_EDCYLE);
            break;

        case COORDSYS_PCARREE:     // Simple Cylindrical / Plate Carree
            PlateCarreeToCsDef (csDef, params.pcarree);
            break;

        case COORDSYS_MRCATPV:    // Popular Visualization Pseudo Mercator Projection
            PopularVisualizationMercatorToCsDef (csDef, params.mrcatpv);
            break;

       case COORDSYS_MNDOTOBL:    // Minnesota DOT Oblique Mercator
            MinnesotaDOTObliqueMercatorToCsDef (csDef, params.mndotobl);
            break;

        default:
            return ERROR;
        }

    // this was the tolerance used in the old code.
    if (fabs (csDef.map_scl) <= GEOCOORD_CLOSE_TOLERANCE)
        csDef.map_scl = 1.0;

    // we do not want to send map_scl to CSMap. We'll handle it internally. We set it to 1.0 in csDef, and we'll store it as a member of DgnGCS.
    paperScale      = csDef.map_scl;
    csDef.map_scl   = 1.0;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
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
    memset ((void*)&csDef, 0, sizeof(csDef));

    // get the strings from the type 66.
    CommonParamsToCSDef (csDef, extracted);
    ProjectionParamsToCSDef (csDef, paperScale, extracted.projType, projectionParams);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   11/06
+---------------+---------------+---------------+---------------+---------------+------*/
public:

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
private:
void            CommonParamsFromCsDef
(
GeoCoordType66      &type66,
const CSDefinition  &csDef
)
    {
    type66.protect = csDef.protect;

    CSMap::CS_stncp (type66.grp_knm, csDef.group,   DIM(type66.grp_knm));
    CSMap::CS_stncp (type66.prj_knm, csDef.prj_knm, DIM(type66.prj_knm));
    CSMap::CS_stncp (type66.source,  csDef.source,  DIM(type66.source));
    CSMap::CS_stncp (type66.desc_nm, csDef.desc_nm, DIM(type66.desc_nm));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
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
* @bsimethod                                                    Barry.Bentley   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            SaveEllipsoidName (CharP ellipsoidName, const CSDefinition& csDef, size_t charCount)
    {
    if ( (0 != csDef.elp_knm[0]) || (NULL == m_datum) )
        CSMap::CS_stncp (ellipsoidName, csDef.elp_knm, (int)charCount);
    else
        CSMap::CS_stncp (ellipsoidName, m_datum->ell_knm, (int)charCount);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            AlbersFromCsDef
(
Proj_alber          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));

    DomainFromCsDef (params.gcDom, csDef);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            AzimuthalEquidistantElevatedFromCsDef
(
Proj_azede          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            AzimuthalEqualAreaFromCsDef
(
Proj_azmea          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.az           = csDef.prj_prm1;
    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            AzimuthalEquidistantFromCsDef
(
Proj_azmed          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.az           = csDef.prj_prm1;
    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            BonneFromCsDef
(
Proj_bonne          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            BipolarObliqueConformalConicFromCsDef
(
Proj_bpcnc          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            CassiniFromCsDef
(
Proj_csini          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.org_lng      = csDef.prj_prm1;
    params.org_lat      = csDef.org_lat;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            EquidistantConicFromCsDef
(
Proj_edcnc          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            EquidistantCylindricalFromCsDef
(
Proj_edcyl          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.std_lat      = csDef.prj_prm1;
    params.org_lat      = csDef.org_lat;
    params.org_lng      = csDef.org_lng;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            Eckert4FromCsDef
(
Proj_ekrt4          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.org_lng      = csDef.prj_prm1;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            Eckert6FromCsDef
(
Proj_ekrt6          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.org_lng      = csDef.prj_prm1;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            TransverseMercatorFromCsDef
(
Proj_trmer          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    SaveEllipsoidName (params.ell_knm, csDef, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.org_lng      = csDef.prj_prm1;
    params.org_lat      = csDef.org_lat;
    params.scl_red      = csDef.scl_red;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            TransverseMercatorMinnesotaFromCsDef
(
Proj_tmmin          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            TransverseMercatorWisconsinFromCsDef
(
Proj_tmwis          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            TransverseMercatorAffinePostProcessFromCsDef
(
Proj_trmaf          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            UniversalTransverseMercatorZoneFromCsDef
(
Proj_utmzn          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.zoneNo       =  (long) csDef.prj_prm1;
    params.hemisphere   =  (long) (0 > csDef.prj_prm2 ?  OBE_HEMISPHERE_SOUTH : OBE_HEMISPHERE_NORTH);


    params.paper_scl    =  csDef.map_scl;
    params.quad         =  csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            GnomicFromCsDef
(
Proj_gnomc          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            ZonesFromCsDef
(
ZONE                *zone,
long                *numZones,
const double        *src
)
    {
    // set up the zones. NOTE: This did not appear to be supported in the old geocoordinate stuff.
    // I am going by the comments in the ZONE typedef, probably added by TI Doug Bilinski.
    for (*numZones = 0; *numZones < NUM_ZONE; *numZones++, zone++, src+=3)
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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            HomolsineFromCsDef
(
Proj_hmlsn          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    ZonesFromCsDef (params.zone, &params.nZones, &csDef.prj_prm1);
    params.org_lng      = csDef.org_lng;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            ObliqueMercator1PointFromCsDef
(
Proj_Oblq1          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            ObliqueMercator2PointFromCsDef
(
Proj_Oblq2          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

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
* @bsimethod                                                    Alain.Robert   2011/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            MinnesotaDOTObliqueMercatorFromCsDef
(
Proj_Mndotobl       &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            HungarianEOVFromCsDef
(
Proj_Hueov          &params,
const CSDefinition  &csDef
)
    {
    // No CSMap documentation, looking at old geocoord stuff.
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            KrovakFromCsDef
(
Proj_Krvak          &params,
const CSDefinition  &csDef
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord cold.
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

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

    if (0 == strcmp (csDef.prj_knm, CS_KRVK95))
        params.apply95 = 1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            LambertTangentialFromCsDef
(
Proj_Lmtan          &params,
const CSDefinition  &csDef
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord cold.
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;
    params.scl_red      = csDef.scl_red;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            LambertConformalConicFromCsDef
(
Proj_Lmbrt          &params,
const CSDefinition  &csDef
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord cold.
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            LambertConformalConicMinnesotaFromCsDef
(
Proj_Lmmin          &params,
const CSDefinition  &csDef
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord code.
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            LambertConformalConicWisconsinFromCsDef
(
Proj_Lmwis          &params,
const CSDefinition  &csDef
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord code.
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            MillerFromCsDef
(
Proj_millr          &params,
const CSDefinition  &csDef
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord cold.
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.org_lng      = csDef.prj_prm1;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            ModifiedPolyconicFromCsDef
(
Proj_modpc          &params,
const CSDefinition  &csDef
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord cold.
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

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
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            MollweideFromCsDef
(
Proj_molwd          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    ZonesFromCsDef (params.zone, &params.nZones, &csDef.prj_prm1);
    params.org_lng      = csDef.org_lng;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            MercatorFromCsDef
(
Proj_mrcat          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.org_lng      = csDef.prj_prm1;
    params.std_lat      = csDef.prj_prm2;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            MercatorScaleReductionFromCsDef
(
Proj_mrcsr          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.org_lng      = csDef.prj_prm1;
    params.scl_red      = csDef.scl_red;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            ModifiedStereographicFromCsDef
(
Proj_mstro          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;
    params.scl_red      = csDef.scl_red;

    // copy the imaginary numbers to prj_prm1 - prj_prm24
    memcpy (params.ABary, &csDef.prj_prm1, 24*sizeof(double));

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            NormalAspectAuthalicCylindricalFromCsDef
(
Proj_nacyl          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.org_lng      = csDef.org_lng;
    params.std_lat      = csDef.prj_prm1;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            NonEarthFromCsDef
(
Proj_nerth          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            NonEarthScaleRotationTranslationFromCsDef
(
Proj_nesrt          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.scale        = csDef.prj_prm1;
    params.rotateDeg    = csDef.prj_prm2;
    params.x_origin     = csDef.prj_prm3;
    params.y_origin     = csDef.prj_prm4;

    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            NewZealandFromCsDef
(
Proj_nzlnd          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.org_lng       = csDef.org_lng;
    params.org_lat       = csDef.org_lat;

    params.paper_scl     = csDef.map_scl;
    params.x_off         = csDef.x_off;
    params.y_off         = csDef.y_off;
    params.quad          = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            OrthographicFromCsDef           // COORDSYS_ORTHO
(
Proj_ortho          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.org_lng       = csDef.org_lng;
    params.org_lat       = csDef.org_lat;

    params.paper_scl     = csDef.map_scl;
    params.x_off         = csDef.x_off;
    params.y_off         = csDef.y_off;
    params.quad          = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            StereographicFromCsDef
(
Proj_stero          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

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
* @bsimethod                                                    Alain.Robert   02/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            Ost97FromCsDef
(
Proj_ost97          &params,
const CSDefinition  &csDef
)
    {
    // All parameters are hard-coded ... we only save as this information since it is required for
    // recreation of the GCS upon reload (CSMAP issue) and to reload the DGN into Map XM or GeoGraphics

    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.paper_scl    = csDef.map_scl;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Alain.Robert   02/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            Ost02FromCsDef
(
Proj_ost97          &params,
const CSDefinition  &csDef
)
    {
    // All parameters are hard-coded ... we only save as this information since it is required for
    // recreation of the GCS upon reload (CSMAP issue) and to reload the DGN into Map XM or GeoGraphics

    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.paper_scl    = csDef.map_scl;
    params.quad         = csDef.quad;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            AmericanPolyconicFromCsDef
(
Proj_plycn          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.org_lng      = csDef.prj_prm1;
    params.org_lat      = csDef.org_lat;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            PolarStereographicStandardLatFromCsDef
(
Proj_pstsl          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;
    params.std_lat      = csDef.prj_prm1;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            RobinsonFromCsDef
(
Proj_robin          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.org_lng      = csDef.org_lng;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            SinusoidalFromCsDef
(
Proj_sinus          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    ZonesFromCsDef (params.zone, &params.nZones, &csDef.prj_prm1);
    params.org_lng      = csDef.org_lng;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            SwissFromCsDef
(
Proj_swiss          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.org_lng       = csDef.org_lng;
    params.org_lat       = csDef.org_lat;

    params.paper_scl     = csDef.map_scl;
    params.x_off         = csDef.x_off;
    params.y_off         = csDef.y_off;
    params.quad          = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            Danish34FromCsDef
(
Proj_sys34          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.zoneNo       = (long) csDef.prj_prm1;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            TransverseAuthalicCylindricalFromCsDef
(
Proj_tacyl          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.org_lng      = csDef.org_lng;
    params.org_lat      = csDef.org_lat;
    params.scl_red      = csDef.scl_red;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            VanDerGrintenFromCsDef
(
Proj_vdgrn          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.org_lng      = csDef.org_lng;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            WinkelTripelFromCsDef
(
Proj_winkt          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.org_lng      = csDef.org_lng;
    params.std_lat      = csDef.prj_prm1;

    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            LambertAffinePostProcessFromCsDef
(
Proj_Lmbrtaf        &params,
const CSDefinition  &csDef
)
    {
    // The CSMap documentation doesn't say anything about these parameters. Based on old geocoord cold.
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

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
* @bsimethod                                                    Alain.Robert   03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            PlateCarreeFromCsDef
(
Proj_pcarree        &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.org_lat      = csDef.org_lat;
    params.org_lng      = csDef.org_lng;
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Alain.Robert   03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
void            PopularVisualizationMercatorFromCsDef
(
Proj_mrcatpv        &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit,    DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));
    DomainFromCsDef (params.gcDom, csDef);

    params.org_lng      = csDef.prj_prm1;
    
    params.paper_scl    = csDef.map_scl;
    params.x_off        = csDef.x_off;
    params.y_off        = csDef.y_off;
    params.quad         = csDef.quad;
    }




/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            GeographicFromCsDef
(
Proj_unity          &params,
const CSDefinition  &csDef
)
    {
    CSMap::CS_stncp (params.unit_nm, csDef.unit, DIM(params.unit_nm));
    CSMap::CS_stncp (params.dat_knm, csDef.dat_knm, DIM(params.dat_knm));
    CSMap::CS_stncp (params.ell_knm, csDef.elp_knm, DIM(params.ell_knm));

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
    if (0 != _stricmp (pCsDef->unit, STRING_DEGREE))
        {
        /* CSMap structures carry range in units of the coordinate system.  Convert it if necessary */
        status = ucrdConvert_distanceDegreesToAngular (&pCsDef->prj_prm1, pCsDef->prj_prm1, pCsDef->unit);
        status = ucrdConvert_distanceDegreesToAngular (&pCsDef->prj_prm2, pCsDef->prj_prm2, pCsDef->unit);
        }
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       FormatProjectionParams
(
Int32               projectionType,
ProjectionParams    &params,
const CSDefinition  &csDef
)
    {
    // make sure it starts out clear.
    memset (&params, 0, sizeof(params));

    switch (projectionType)
        {
        case COORDSYS_ALBER:    // Albers equal area projection
            AlbersFromCsDef (params.alber, csDef);
            break;

        case COORDSYS_AZEDE:    // Azimuthal Equidistant Projection with Elevated Ellipsoid.
            AzimuthalEquidistantElevatedFromCsDef (params.azede, csDef);
            break;

        case COORDSYS_AZMEA:    // Azimuthal Equal Area Projection.
            AzimuthalEqualAreaFromCsDef (params.azmea, csDef);
            break;

        case COORDSYS_AZMED:    // Azimuthal Equidistant Projection.
            AzimuthalEquidistantFromCsDef (params.azmed, csDef);
            break;

        case COORDSYS_BONNE:    // Bonne Projection
            BonneFromCsDef (params.bonne, csDef);
            break;

        case COORDSYS_BPCNC:    // Bipolar Oblique Conic Projection
            BipolarObliqueConformalConicFromCsDef (params.bpcnc, csDef);
            break;

        case COORDSYS_CSINI:    // Cassini Cylindrical Projection
            CassiniFromCsDef (params.csini, csDef);
            break;

        case COORDSYS_EDCNC:    // Equidistant Conic Projection
            EquidistantConicFromCsDef (params.edcnc, csDef);
            break;

        case COORDSYS_EDCYL:    // Equidistant Cylindrical Projection
            EquidistantCylindricalFromCsDef (params.edcyl, csDef);
            break;

        case COORDSYS_EKRT4:    // Eckert IV Projection
            Eckert4FromCsDef (params.ekrt4, csDef);
            break;

        case COORDSYS_EKRT6:    // Eckert VI Projection
            Eckert6FromCsDef (params.ekrt6, csDef);
            break;

        case COORDSYS_GAUSK:    // Gauss Kruger Projection (Transvers Mercator variant)
            TransverseMercatorFromCsDef (params.trmer, csDef);
            break;

        case COORDSYS_GNOMC:    // Gnomic Projection
            GnomicFromCsDef (params.gnomc, csDef);
            break;

        case COORDSYS_HMLSN:    // Homolsine Projection
            HomolsineFromCsDef (params.hmlsn, csDef);
            break;

        case COORDSYS_HOM1U:    // Hotine Oblique Mercator 1 Point Unrectified Projection (CSMap says rarely used).
        case COORDSYS_OBLQ1:    // CSMap calls it HOM1XY - Alask Variation, Rectified
            ObliqueMercator1PointFromCsDef (params.oblq1, csDef);
            break;

        case COORDSYS_HOM2U:    // Hotine Oblique Mercator 1 Point Unrectified Projection (CSMap says rarely used).
        case COORDSYS_OBLQ2:
            ObliqueMercator2PointFromCsDef (params.oblq2, csDef);
            break;

        case COORDSYS_HUEOV:    // Hungarian EOV Projection.
            HungarianEOVFromCsDef (params.hueov, csDef);
            break;

        case COORDSYS_KRVKG:    // Krovak Oblique Conformal Conic
            KrovakFromCsDef (params.krvkg, csDef);
            break;
        case COORDSYS_KRVKP:
            KrovakFromCsDef (params.krvkg, csDef);
            break;
        case COORDSYS_KRVKR:
            KrovakFromCsDef (params.krvkg, csDef);
            break;

        case COORDSYS_LM1SP:    // Lambert Tangential Projection
        case COORDSYS_LMTAN:
            LambertTangentialFromCsDef (params.lmtan, csDef);
            break;

        case COORDSYS_LMBRT:    // Lambert Conformal Conic Projection
        case COORDSYS_LMBLG:    // Belgium variation
            LambertConformalConicFromCsDef (params.lmbrt, csDef);;
            break;

        case COORDSYS_LMMIN:    // Lambert Conformal Conic Projection, Minnesota variation
            LambertConformalConicMinnesotaFromCsDef (params.lmmin, csDef);
            break;

        case COORDSYS_LMWIS:    // Lambert Conformal Conic Projection, Minnesota variation
            LambertConformalConicWisconsinFromCsDef (params.lmwis, csDef);
            break;

        case COORDSYS_MILLR:    // Miller Projection
            MillerFromCsDef (params.millr, csDef);
            break;

        case COORDSYS_MODPC:    // Modified Polyconic Projection
            ModifiedPolyconicFromCsDef (params.modpc, csDef);
            break;

        case COORDSYS_MOLWD:    // Mollweide Projection
            MollweideFromCsDef (params.molwd, csDef);
            break;

        case COORDSYS_MRCAT:    // Mercator Projection
            MercatorFromCsDef (params.mrcat, csDef);
            break;

        case COORDSYS_MRCSR:    // Mercator Projection with Scale Reduction
            MercatorScaleReductionFromCsDef (params.mrcsr, csDef);
            break;

        case COORDSYS_MSTRO:    // Modified Stereographic Projection
            ModifiedStereographicFromCsDef (params.mstro, csDef);
            break;

        case COORDSYS_NACYL:    // Normal Aspect Authalic Cylindrical Projection
            NormalAspectAuthalicCylindricalFromCsDef (params.nacyl, csDef);
            break;

        case COORDSYS_NERTH:    // Non-earth Projection
            NonEarthFromCsDef (params.nerth, csDef);
            break;

        case COORDSYS_NESRT:    // Non-earth with scale, rotation, and translation
            NonEarthScaleRotationTranslationFromCsDef (params.nesrt, csDef);
            break;

        case COORDSYS_NZLND:    // New Zealand Projection
            NewZealandFromCsDef (params.nzlnd, csDef);
            break;

        case COORDSYS_ORTHO:    // Orthographic Projection
            OrthographicFromCsDef (params.ortho, csDef);
            break;

        case COORDSYS_OSTRO:    // Oblique Stereographic Projection
            StereographicFromCsDef (params.ostro, csDef);
            break;

        case COORDSYS_PSTRO:    // Polar Sterographic Projection
            StereographicFromCsDef (params.ostro, csDef);
            break;

        case COORDSYS_OST97:    // Ordinance Survey Grid Transformation of 1997
            Ost97FromCsDef (params.ost97, csDef);
            break;

        case COORDSYS_OST02:    // Ordinance Survey Grid Transformation of 2002
            // This is a variation of Transverse Mercator (see csmap documentation) but there are no parameters - everything is hard coded.
            Ost02FromCsDef (params.ost97, csDef);
            break;

        case COORDSYS_PLYCN:    // American Polyconic Projection
            AmericanPolyconicFromCsDef (params.plycn, csDef);
            break;

        case COORDSYS_PSTSL:    // Polar Stereographic with Standard Latitude
            PolarStereographicStandardLatFromCsDef (params.pstsl, csDef);
            break;

        case COORDSYS_ROBIN:    // Robinson Projection
            RobinsonFromCsDef (params.robin, csDef);
            break;

        case COORDSYS_RSKEW:    // Rectified Skew Orthomorphic
            ObliqueMercator1PointFromCsDef (params.oblq1, csDef);
            break;

        case COORDSYS_RSKWC:    // Rectified Skew Orthomorphic Centered
            ObliqueMercator1PointFromCsDef (params.oblq1, csDef);
            break;

        case COORDSYS_RSKWO:    // Rectified Skew Orthomorphic Origin
            ObliqueMercator1PointFromCsDef (params.oblq1, csDef);
            break;

        case COORDSYS_SINUS:    // Sinusoidal Projection
            SinusoidalFromCsDef (params.sinus, csDef);
            break;

        case COORDSYS_SOTRM:    // Transverse Mercator South Oriented Projection
            TransverseMercatorFromCsDef (params.trmer, csDef);
            break;

        case COORDSYS_STERO:    // Stereographic Projection
            StereographicFromCsDef (params.stero, csDef);
            break;

        case COORDSYS_SWISS:    // Oblique CylindricalSwiss Projection
            SwissFromCsDef (params.swiss, csDef);
            break;

        case COORDSYS_SYS34:    // Danish System 34
        case COORDSYS_S3499:    // Danish System 34 with 1999 Polynomial.
        case COORDSYS_S3401:    // Danish System 34 with 2001 Polynomial.
            Danish34FromCsDef (params.sys34, csDef);
            break;

        case COORDSYS_TACYL:    // Transverse Authalic Cylindrical Projection
            TransverseAuthalicCylindricalFromCsDef (params.tacyl, csDef);
            break;

        case COORDSYS_TMMIN:    // Transverse Mercator, Minnesota
            TransverseMercatorMinnesotaFromCsDef (params.tmmin, csDef);
            break;

        case COORDSYS_TMWIS:    // Transverse Mercator, Wisconsin County
            TransverseMercatorWisconsinFromCsDef (params.tmwis, csDef);
            break;

        case COORDSYS_TRMAF:    // Transverse Mercator with Affine Post Process
            TransverseMercatorAffinePostProcessFromCsDef (params.trmaf, csDef);
            break;

        case COORDSYS_TRMER:    // Transverse Mercator Projection
            TransverseMercatorFromCsDef (params.trmer, csDef);
            break;

        case COORDSYS_TMKRG:    // Transverse Mercator Projection
            TransverseMercatorFromCsDef (params.trmer, csDef);
            break;

        case COORDSYS_UTMZN:    // Universal Tranverse Mercator Zone Projections
            UniversalTransverseMercatorZoneFromCsDef (params.utmzn, csDef);
            break;

        case COORDSYS_Geographic:   // Unity (Lat/Long) pseudo-projection.
            GeographicFromCsDef (params.unity, csDef);
            break;

        case COORDSYS_VDGRN:        // Van der Grinten Projection
            VanDerGrintenFromCsDef (params.vdgrn, csDef);
            break;

        case COORDSYS_WINKT:        // Winkel-Tripel Projection
            WinkelTripelFromCsDef (params.winkt, csDef);
            break;

        case COORDSYS_LMBRTAF:        // Winkel-Tripel Projection
            LambertAffinePostProcessFromCsDef (params.lmbrtaf, csDef);
            break;

        // TOTAL_SPECIAL
        case COORDSYS_UTMZNBF:
            UniversalTransverseMercatorZoneFromCsDef (params.utmzn, csDef);
            break;

        case COORDSYS_TRMERBF:
            TransverseMercatorFromCsDef (params.trmer, csDef);
            break;

        case COORDSYS_EDCYLE:    // Equidistant Cylindrical Ellipsoidal Projection
            EquidistantCylindricalFromCsDef (params.edcyl, csDef);
            break;

        case COORDSYS_PCARREE:    // Simple Cylindrical / Plate Carree
            PlateCarreeFromCsDef (params.pcarree, csDef);
            break;

        case COORDSYS_MRCATPV:    // Popular Visualization Pseudo Mercator
            PopularVisualizationMercatorFromCsDef (params.mrcatpv, csDef);
            break;

        case COORDSYS_MNDOTOBL:
            MinnesotaDOTObliqueMercatorFromCsDef (params.mndotobl, csDef);
            break;

        default:
            return ERROR;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   11/06
+---------------+---------------+---------------+---------------+---------------+------*/
private:
void                FormatType66Struct
(
GeoCoordType66      &type66,
CSParameters        &csParams,
Int32               coordSysId,
DgnModelP           cache,
bool                datumOrEllipsoidFromUserLib,
VertDatumCode       verticalDatum,
LocalTransformerP   localTransformer
)
    {
    // convert the type 66 and projectionParams to a CSDefinition/CSDatum pair that can be made into CSParameters.
    memset ((void*)&type66, 0, sizeof(type66));

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
    dgnModel_getGlobalOrigin (cache, (DPoint3d*)&type66.deprec_globorg);

    // this ridiculous unit stuff is only needed in the case where this file is saved to V7. I don't think the V8 geocoord stuff used it.
    double  subPerMast          = dgnModel_getSubPerMaster (cache);
    double  uorPerSub           = dgnModel_getUorPerSub (cache);
    type66.deprec_subpermast    = DataConvert::RoundDoubleToULong (subPerMast);
    type66.deprec_uorpersub     = DataConvert::RoundDoubleToULong (uorPerSub);

    DgnProjectionTypes  projectionType = BaseGCS::DgnProjectionTypeFromCSDefName (csParams.csdef.prj_knm);
    type66.protect              = csParams.csdef.protect;
    type66.projType             = projectionType;
    type66.family               = 0;    // not used
    type66.coordsys             = (0 != coordSysId) ? coordSysId : type66.projType;

    // this stuff is hopefully never used, just ancient stupidity.
    WChar masterUnitLabel[MAX_UNIT_NAME_LENGTH];
    WChar subUnitLabel[MAX_UNIT_NAME_LENGTH];
    dgnModel_getMasterUnitLabel (cache, masterUnitLabel);
    dgnModel_getSubUnitLabel (cache, subUnitLabel);
    type66.deprec_mastname[0] = (char) (masterUnitLabel[0] && 0xFF);
    type66.deprec_mastname[1] = (char) (masterUnitLabel[1] && 0xFF);
    type66.deprec_subname[0]  = (char) (subUnitLabel[0] && 0xFF);
    type66.deprec_subname[1]  = (char) (subUnitLabel[1] && 0xFF);
    type66.deprec_dgnUnitNm[UNTSZ];

    // used by some projections.
    CSMap::CS_stncp (type66.cs_knm,     csParams.csdef.key_nm,  DIM (type66.cs_knm));
    CSMap::CS_stncp (type66.grp_knm,    csParams.csdef.group,   DIM (type66.grp_knm));
    CSMap::CS_stncp (type66.prj_knm,    csParams.csdef.prj_knm, DIM (type66.prj_knm));
    CSMap::CS_stncp (type66.dat_nm,     csParams.datum.dt_name, DIM (type66.dat_nm));
    CSMap::CS_stncp (type66.ell_nm,     csParams.datum.el_name, DIM (type66.ell_nm));
    CSMap::CS_stncp (type66.desc_nm,    csParams.csdef.desc_nm, DIM (type66.desc_nm));
    CSMap::CS_stncp (type66.source,     csParams.csdef.source,  DIM (type66.source));

    if (0 != type66.projType)
        {
        WString projName;
        BaseGeoCoordResource::GetLocalizedProjectionName (projName, projectionType);
        strcpy (type66.prj_nm, UglyAsciiString(projName).GetCharCP());
        }

    // so far, in every case I've seen that zone_knm has been set, it's been the same as cs_knm. I think it was used differently in the dim(witted) past.
    if (IsKeyNameCoordinateSystem (type66.coordsys))
        CSMap::CS_stncp (type66.zone_knm,   csParams.csdef.key_nm, DIM (type66.zone_knm));

    type66.to84_via                 = csParams.datum.to84_via;
    type66.minor_version            = (GCOORD_COMPATIBLE_08117_VERSION == type66.version) ? GCOORD_COMPATIBLE_08117_MINOR_VERSION : GCOORD_08119_MINOR_VERSION;
    type66.attributes               = 0;    // couldn't find anything useful in old code.
    type66.prj_code                 = csParams.prj_code;

    type66.dtmOrElpsdFromUserLib    = datumOrEllipsoidFromUserLib ? SOURCELIB_USER : 0;

    // set up the VerticalDatum. It was new on MicroStation version 08.11.07, and takes up what was formerly part of the "filler" so we set a signature word
    //  indicating that it is valid. That's because we're not sure that the "filler" was properly initialized to zero in previous version.
    type66.verticalDatum        = (UInt16) verticalDatum;
    type66.verticalDatumValid   = GEOCOORD66_VerticalDatumValid;

    if (NULL != localTransformer)
        localTransformer->SaveParameters (type66.localTransformType, type66.transformParams);
    }

#if defined (TEST_TYPE66_CREATE)
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   11/06
+---------------+---------------+---------------+---------------+---------------+------*/
private:
void            CompareType66
(
GeoCoordType66      &oldType66,
GeoCoordType66      &newType66
)
    {
    if (oldType66.gcoord66_size != newType66.gcoord66_size)
        printf ("Error in gcoord_size, old is %d, new is %d\n", oldType66.gcoord66_size, newType66.gcoord66_size);

    if (oldType66.version != newType66.version)
        printf ("Error in version, old is %d, new is %d\n", oldType66.version, newType66.version);

    if (oldType66.e_rad != newType66.e_rad)
        printf ("Error in e_rad, old is %lf, new is %lf\n", oldType66.e_rad, newType66.e_rad);

    if (oldType66.p_rad != newType66.p_rad)
        printf ("Error in p_rad, old is %lf, new is %lf\n", oldType66.p_rad, newType66.p_rad);

    if (oldType66.delta_X != newType66.delta_X)
        printf ("Error in delta_X, old is %lf, new is %lf\n", oldType66.delta_X, newType66.delta_X);
    if (oldType66.delta_Y != newType66.delta_Y)
        printf ("Error in delta_Y, old is %lf, new is %lf\n", oldType66.delta_Y, newType66.delta_Y);
    if (oldType66.delta_Z != newType66.delta_Z)
        printf ("Error in delta_Z, old is %lf, new is %lf\n", oldType66.delta_Z, newType66.delta_Z);

    if (oldType66.rot_X != newType66.rot_X)
        printf ("Error in rot_X, old is %lf, new is %lf\n", oldType66.rot_X, newType66.rot_X);
    if (oldType66.rot_Y != newType66.rot_Y)
        printf ("Error in rot_Y, old is %lf, new is %lf\n", oldType66.rot_Y, newType66.rot_Y);
    if (oldType66.rot_Z != newType66.rot_Z)
        printf ("Error in rot_Z, old is %lf, new is %lf\n", oldType66.rot_Z, newType66.rot_Z);

    if (oldType66.bwscale != newType66.bwscale)
        printf ("Error in bwscale, old is %lf, new is %lf\n", oldType66.bwscale, newType66.bwscale);

    if (oldType66.deprec_globorg.x != newType66.deprec_globorg.x)
        printf ("Error in deprec_globorg.x, old is %lf, new is %lf\n", oldType66.deprec_globorg.x, newType66.deprec_globorg.x);
    if (oldType66.deprec_globorg.y != newType66.deprec_globorg.y)
        printf ("Error in deprec_globorg.y, old is %lf, new is %lf\n", oldType66.deprec_globorg.y, newType66.deprec_globorg.y);
    if (oldType66.deprec_globorg.z != newType66.deprec_globorg.z)
        printf ("Error in deprec_globorg.z, old is %lf, new is %lf\n", oldType66.deprec_globorg.z, newType66.deprec_globorg.z);

#if defined (DONT_CHECK)
    // most (but not all) of the type 66's I checked had deprec_mastunits set to 0. Certainly it is useless.
    if (oldType66.deprec_mastunits != newType66.deprec_mastunits)
        printf ("Error in deprec_mastunits, old is %d, new is %d\n", oldType66.deprec_mastunits, newType66.deprec_mastunits);
#endif
    if (oldType66.deprec_subpermast != newType66.deprec_subpermast)
        printf ("Error in deprec_subpermast, old is %d, new is %d\n", oldType66.deprec_subpermast, newType66.deprec_subpermast);
    if (oldType66.deprec_uorpersub != newType66.deprec_uorpersub)
        printf ("Error in deprec_uorpersub, old is %d, new is %d\n", oldType66.deprec_uorpersub, newType66.deprec_uorpersub);

    if (oldType66.protect != newType66.protect)
        printf ("Error in protect, old is %d, new is %d\n", oldType66.protect, newType66.protect);

    if (oldType66.projType != newType66.projType)
        printf ("Error in projType, old is %d, new is %d\n", oldType66.projType, newType66.projType);

    if (oldType66.family != newType66.family)
        printf ("Error in family, old is %d, new is %d\n", oldType66.family, newType66.family);

    if (oldType66.coordsys != newType66.coordsys)
        printf ("Error in coordsys, old is %d, new is %d\n", oldType66.coordsys, newType66.coordsys);

    if (oldType66.deprec_mastname[0] != newType66.deprec_mastname[0])
        printf ("Error in deprec_mastname[0], old is %c, new is %c\n", oldType66.deprec_mastname[0], newType66.deprec_mastname[0]);
    if (oldType66.deprec_mastname[1] != newType66.deprec_mastname[1])
        printf ("Error in deprec_mastname[1], old is %c, new is %c\n", oldType66.deprec_mastname[1], newType66.deprec_mastname[1]);
    if (oldType66.deprec_subname[0] != newType66.deprec_subname[0])
        printf ("Error in deprec_subname[0], old is %c, new is %c\n", oldType66.deprec_subname[0], newType66.deprec_subname[0]);
    if (oldType66.deprec_subname[1] != newType66.deprec_subname[1])
        printf ("Error in deprec_subname[1], old is %c, new is %c\n", oldType66.deprec_subname[1], newType66.deprec_subname[1]);

    if (0 != strcmp (oldType66.cs_knm, newType66.cs_knm))
        printf ("Error in cs_knm, old is '%hs', new is '%hs'\n", oldType66.cs_knm, newType66.cs_knm);
    if (0 != strcmp (oldType66.grp_knm, newType66.grp_knm))
        printf ("Error in grp_knm, old is '%hs', new is '%hs'\n", oldType66.grp_knm, newType66.grp_knm);
    if (0 != strcmp (oldType66.zone_knm, newType66.zone_knm))
        printf ("Error in zone_knm, old is '%hs', new is '%hs'\n", oldType66.zone_knm, newType66.zone_knm);
    if (0 != strcmp (oldType66.prj_knm, newType66.prj_knm))
        printf ("Error in prj_knm, old is '%hs', new is '%hs'\n", oldType66.prj_knm, newType66.prj_knm);

    // the old prj_nm's often have " Projection" at the end, as in "Universal Tranverse Mercator Projection", but I think that was later shortened, so don't count as error.
    if (0 != strncmp (oldType66.prj_nm, newType66.prj_nm, strlen(newType66.prj_nm)))
        printf ("Error in prj_nm, old is '%hs', new is '%hs'\n", oldType66.prj_nm, newType66.prj_nm);
    if (0 != strcmp (oldType66.dat_nm, newType66.dat_nm))
        printf ("Error in dat_nm, old is '%hs', new is '%hs'\n", oldType66.dat_nm, newType66.dat_nm);
    if (0 != strcmp (oldType66.ell_nm, newType66.ell_nm))
        printf ("Error in ell_nm, old is '%hs', new is '%hs'\n", oldType66.ell_nm, newType66.ell_nm);
    if (0 != strcmp (oldType66.desc_nm, newType66.desc_nm))
        printf ("Error in desc_nm, old is '%hs', new is '%hs'\n", oldType66.desc_nm, newType66.desc_nm);
    if (0 != strcmp (oldType66.source, newType66.source))
        printf ("Error in source, old is '%hs', new is '%hs'\n", oldType66.source, newType66.source);

    if (oldType66.to84_via != newType66.to84_via)
        printf ("Error in to84_via, old is %d, new is %d\n", oldType66.to84_via, newType66.to84_via);

    if (oldType66.minor_version != newType66.minor_version)
        printf ("Error in minor_version, old is %d, new is %d\n", oldType66.minor_version, newType66.minor_version);
    if (oldType66.attributes != newType66.attributes)
        printf ("Error in attributes, old is %d, new is %d\n", oldType66.attributes, newType66.attributes);
    if (oldType66.prj_code != newType66.prj_code)
        printf ("Error in prj_code, old is %d, new is %d\n", oldType66.prj_code, newType66.prj_code);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   11/06
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
* @bsimethod                                                    Barry.Bentley   11/06
+---------------+---------------+---------------+---------------+---------------+------*/
public:
StatusInt           FormatGeoCoordType66
(
ApplicationElm*     type66Element,          // <= filled in with data. Must be at least 768 words.
CSParameters        &csParams,
Int32               coordSysId,
DgnModelP           cache,
bool                primary,
double              paperScaleFromType66,
bool                datumOrEllipsoidFromUserLib,
VertDatumCode       verticalDatum,
LocalTransformerP   localTransformer
)
    {
    GeoCoordType66      type66;
    ProjectionParams    projectionParams;

    FormatType66Struct (type66, csParams, coordSysId, cache, datumOrEllipsoidFromUserLib, verticalDatum, localTransformer);

    // set map_scl only for the purpose of saving it to the type 66. We don't have CSMap handling it.
    csParams.csdef.map_scl = paperScaleFromType66;

    // if either the datum or the ellipsoid is from the user lib, we need to make sure the ellipsoid name is saved in the struct, even if there is a Datum.
    //  That's because the user library might not be present on the computer of the person opening this design file.
    //  So, we set the m_datum member so that we can extract the datum name from there. It isn't set in csParams.csdef unless there is no datum.
    if (datumOrEllipsoidFromUserLib)
        m_datum = &csParams.datum;
    else
        m_datum = NULL;
    FormatProjectionParams (type66.projType, projectionParams, csParams.csdef);
    // make sure we're not holding a stale datum.
    m_datum = NULL;

    // put map_scl back to 1.0;
    csParams.csdef.map_scl = 1.0;

#if defined (TEST_TYPE66_CREATE)
    if (NULL != s_oldType66)
        CompareType66 (*s_oldType66, type66);
    if (NULL != s_oldProjectionParams)
        CompareProjectionParams (*s_oldProjectionParams, projectionParams);
#endif

    // set up the type 66.
    memset (type66Element, 0, 768*sizeof(short));
    type66Element->ehdr.type        = MICROSTATION_ELM;
    type66Element->ehdr.level       = MSAPPINFO_LEVEL;
    type66Element->ehdr.isGraphics  = FALSE;
    type66Element->ehdr.elementSize = 0x2fe;    // this is 2 less than (GEOCOORD66_SIZE + sizeof(ApplicationElm)) / sizeof(short), but it's what the old version size came out
    type66Element->ehdr.attrOffset  = 0x2fe;
    type66Element->signatureWord    = MSGEOCOORD_SIGNATURE;
    type66Element->appData[0]       = primary ? GCOORD_66_MAS : GCOORD_66_ALT;

    // this used to use the resource conversion logic, but it turns out that the structure was set up
    // correctly, grouping the doubles and filling in the holes so just requires a memcpy.
    memcpy (&type66Element->appData[1], &type66, sizeof (GeoCoordType66));
    
    memcpy (&type66Element->appData[429], &projectionParams, sizeof(ProjectionParams));

    return SUCCESS;
    }

};

CoordinateSystemDgnFormatter*   CoordinateSystemDgnFormatter::s_instance;

T_GeoCoordEventHandlers*        DgnGCS::s_eventHandlers;

/*=================================================================================**//**
* MicroStation GeoCoordinateSystem Native class
* @bsiclass                                                     Barry.Bentley   10/06
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS::DgnGCS () : BaseGCS()
    {
    m_uorsPerBaseUnit               = 1.0;
    m_paperScaleFromType66          = 1.0;
    m_globalOrigin.zero();

    m_datumOrEllipsoidFromUserLib   = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS::DgnGCS 
(
WCharCP         coordinateSystemName
) : BaseGCS (coordinateSystemName)
    {
    m_uorsPerBaseUnit               = 1.0;
    m_paperScaleFromType66          = 1.0;
    m_globalOrigin.zero();
    m_datumOrEllipsoidFromUserLib   = false;
    SetDatumOrEllipsoidInUserLibrary();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS::DgnGCS 
(
WCharCP         coordinateSystemName,
DgnModelRefP    modelRef
) : BaseGCS (coordinateSystemName)
    {
    InitCacheParameters (modelRef ? modelRef->GetDgnModelP () : NULL, 1.0);
    SetDatumOrEllipsoidInUserLibrary();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS::DgnGCS 
(
CSParameters&   csParameters,
double          paperScale,
Int32           coordSysId,
DgnModelP       cache,
bool            datumOrEllipsoidFromUserLibrary
) : BaseGCS (csParameters, coordSysId, NULL)
    {
    // This is the constructor called when we extract a GCS from the type 66. That's a case where we don't have m_sourceLibrary.
    // However, whether the datum or ellipsoid is in a user library is stored in the type 66.
    // If we don't have the user library at runtime, we don't have a way of telling whether 
    //   the datum or ellipsoid came from there, so we need to keep what the type 66 tells us.
    InitCacheParameters (cache, paperScale);
    m_datumOrEllipsoidFromUserLib = datumOrEllipsoidFromUserLibrary;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS::DgnGCS 
(
BaseGCSCP       baseGCS,
DgnModelRefP    modelRef
) : BaseGCS (*baseGCS)
    {
    InitCacheParameters (modelRef ? modelRef->GetDgnModelP () : NULL, 1.0);
    DgnGCSCP    sourceMstnGCS;
    if (NULL != (sourceMstnGCS = dynamic_cast <DgnGCSCP> (baseGCS)))
        m_datumOrEllipsoidFromUserLib = sourceMstnGCS->m_datumOrEllipsoidFromUserLib;
    else
        {
        GetSourceLibrary();
        SetDatumOrEllipsoidInUserLibrary();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS::DgnGCS (DgnModelRefP modelRef) : BaseGCS ()
    {
    m_datumOrEllipsoidFromUserLib   = false;
    InitCacheParameters (modelRef ? modelRef->GetDgnModelP () : NULL, 1.0);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS::DgnGCS (DgnGCSCR sourceGcs) : BaseGCS(sourceGcs)
    {
    m_datumOrEllipsoidFromUserLib = sourceGcs.m_datumOrEllipsoidFromUserLib;
    m_paperScaleFromType66 = sourceGcs.m_paperScaleFromType66;
    m_uorsPerBaseUnit      = sourceGcs.m_uorsPerBaseUnit;
    m_globalOrigin         = sourceGcs.m_globalOrigin;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnGCS::InitCacheParameters
(
DgnModelP   cache,
double      paperScale
)
    {
    m_paperScaleFromType66          = paperScale;
    double      uorPerStorage = dgnModel_getUorPerStorage (cache);

    UnitInfo    storageUnitInfo;
    if (SUCCESS != dgnModel_getStorageUnit (cache, &storageUnitInfo))
        m_uorsPerBaseUnit = 100000.0;

    else
        {
        // this intentionally ignores the unit base.
        // Thus for a Lat/Long based coordinate system, if the design file storage units are UnitBase::Meter, a meter is treated as a degree.
        // Similarly, for a projected coordinate system, if the design file storage units are UnitBase::Degree, a degree is treated as a meter.
        m_uorsPerBaseUnit = (uorPerStorage * storageUnitInfo.numerator) / (storageUnitInfo.denominator * m_paperScaleFromType66);
        dgnModel_getGlobalOrigin (cache, &m_globalOrigin);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnGCS::SetDatumOrEllipsoidInUserLibrary ()
    {
    m_datumOrEllipsoidFromUserLib   = false;

    // If we have m_csParameters, we found the GCS. Check to see whether the Datum or Ellipsoid is in the user 
    if (NULL != m_csParameters)
        {
        // m_sourceLibrary should always be set by the BaseGCS constructor.
        assert (NULL != m_sourceLibrary);
        if ( (NULL != m_sourceLibrary) && m_sourceLibrary->IsUserLibrary() )
            {
            WString datumName (m_csParameters->csdef.dat_knm);
            WString ellipsoidName (m_csParameters->csdef.elp_knm);
            m_datumOrEllipsoidFromUserLib   = m_sourceLibrary->DatumInLibrary (datumName.c_str()) || m_sourceLibrary->EllipsoidInLibrary (ellipsoidName.c_str());
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DgnGCS::GetDatumOrEllipsoidInUserLibrary ()
    {
    return m_datumOrEllipsoidFromUserLib;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCS::~DgnGCS ()
    {
    }

/*---------------------------------------------------------------------------------**//**
* Public Factory Methods
+---------------+---------------+---------------+---------------+---------------+------*/

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCSPtr       DgnGCS::CreateGCS
(
WCharCP                 coordinateSystemName,
DgnModelRefP            modelRef
)
    {
    return new DgnGCS  (coordinateSystemName, modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCSPtr       DgnGCS::CreateGCS (DgnModelRefP modelRef)
    {
    return new DgnGCS  (modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCSPtr       DgnGCS::CreateGCS
(
BaseGCSCP               baseGCS,
DgnModelRefP            modelRef
)
    {
    return new DgnGCS  (baseGCS, modelRef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCSPtr DgnGCS::CreateGCS(DgnGCSCR dgnGcs)
    {
    return new DgnGCS(dgnGcs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnGCS::CartesianFromUors
(
DPoint3dR               outCartesian,       // <= cartesian, units of coordinate system
DPoint3dCR              inUors              // => UORS
) const
    {
    outCartesian.differenceOf (&inUors, &m_globalOrigin);
    outCartesian.scale (m_csParameters->csdef.scale / m_uorsPerBaseUnit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnGCS::UorsFromCartesian
(
DPoint3dR               outUors,            // <= UORS
DPoint3dCR              inCartesian         // => cartesian, units of coordinate system
) const
    {
    outUors = inCartesian;
    outUors.scale (m_uorsPerBaseUnit / m_csParameters->csdef.scale);
    outUors.sumOf (&m_globalOrigin, &outUors);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
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
* @bsimethod                                                    Barry.Bentley   01/07
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
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus DgnGCS::LatLongFromUors
(
GeoPointR               outLatLong,         // <= latitude longitude
DPoint3dCR              inUors              // => cartesian, in UORS
) const
    {
    DPoint3d    cartesian;
    CartesianFromUors (cartesian, inUors);
    return LatLongFromCartesian (outLatLong, cartesian);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus DgnGCS::UorsFromLatLong
(
DPoint3dR               outUors,            // <= cartesian, in UORS
GeoPointCR              inLatLong           // => latitude longitude
) const
    {
    DPoint3d cartesian;
    ReprojectStatus status = CartesianFromLatLong (cartesian, inLatLong);
    UorsFromCartesian (outUors, cartesian);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
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
    ReprojectStatus status = REPROJECT_Success;

    if ( (NULL == outUorsDest) || (NULL == inUors) )
        return REPROJECT_BadArgument;

    int iPoint;
    for (iPoint=0; iPoint < numPoints; iPoint++, inUors++, outUorsDest++)
        {
        ReprojectStatus   stat1;
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
        GeoPoint    srcLatLong;
        stat1 = LatLongFromUors (srcLatLong, *inUors);

        // does caller want the latlong in source coordinates?
        if (NULL != outLatLongSrc)
            *outLatLongSrc++ = srcLatLong;

        // convert srcLatLong to destLatLong
        GeoPoint    destLatLong;
        stat2 = LatLongFromLatLong (destLatLong, srcLatLong, destMstnGCS);

        // For datum conversion error 2 is in fact a strong warning instead of a hard math error ... we change to a warning
        if (REPROJECT_CSMAPERR_OutOfMathematicalDomain == stat2)
            stat2 = REPROJECT_CSMAPERR_OutOfUsefulRange;

        // does the caller want the destination LatLong?
        if (NULL != outLatLongDest)
            *outLatLongDest++ = destLatLong;

        // convert dstLatLong to destination cartesian.
        stat3 = destMstnGCS.UorsFromLatLong (*outUorsDest, destLatLong);

        if (SUCCESS == status)
            {
            // Status returns hardest error found in the three error statuses
            // The hardest error is the first one encountered that is not a warning (value 1 [cs_CNVRT_USFL])
            if (SUCCESS != stat1)
                status = stat1;
            if ((SUCCESS != stat2) && ((SUCCESS == status) || (cs_CNVRT_USFL == status))) // If stat2 has error and status not already hard error
                {
                if (0 > stat2) // If stat2 is negative ... this is the one ...
                    status = stat2;
                else  // Both are positive (status may be SUCCESS) we use the highest value which is either warning or error
                    status = (stat2 > status ? stat2 : status);
                }
            if ((SUCCESS != stat3) && ((SUCCESS == status) || (cs_CNVRT_USFL == status))) // If stat3 has error and status not already hard error
                {
                if (0 > stat3) // If stat3 is negative ... this is the one ...
                    status = stat3;
                else  // Both are positive (status may be SUCCESS) we use the highest value
                    status = (stat3 > status ? stat3 : status);
                }
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
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
* @bsimethod                                                    Barry.Bentley   01/07
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
* @bsimethod                                                    Barry.Bentley   01/07
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
        ReprojectStatus   stat1;
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

        if (SUCCESS == status)
            {
            // Status returns hardest error found in the three error statuses
            // The hardest error is the first one encountered that is not a warning (value 1 [cs_CNVRT_USFL])
            if (SUCCESS != stat1)
                status = stat1;
            if ((SUCCESS != stat2) && ((SUCCESS == status) || (cs_CNVRT_USFL == status))) // If stat2 has error and status not already hard error
                {
                if (0 > stat2) // If stat2 is negative ... this is the one ...
                    status = stat2;
                else  // Both are positive (status may be SUCCESS) we use the highest value which is either warning or error
                    status = (stat2 > status ? stat2 : status);
                }
            if ((SUCCESS != stat3) && ((SUCCESS == status) || (cs_CNVRT_USFL == status))) // If stat3 has error and status not already hard error
                {
                if (0 > stat3) // If stat3 is negative ... this is the one ...
                    status = stat3;
                else  // Both are positive (status may be SUCCESS) we use the highest value
                    status = (stat3 > status ? stat3 : status);
                }
            }
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Earlin.Lutz     01/07
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
        if ( (NULL != m_csParameters) && (0 == strcmp (m_csParameters->csdef.unit, "Degree")) )
            localExtent.x = localExtent.y = localExtent.z = m_csParameters->csdef.unit_scl / 10000;
        else
            localExtent.x = localExtent.y = localExtent.z = (m_uorsPerBaseUnit * sReferenceFrameSize);

        extent = &localExtent;
        }

    points[0] = elementOrigin;
    points[1].init (elementOrigin.x + extent->x, elementOrigin.y, elementOrigin.z);
    points[2].init (elementOrigin.x, elementOrigin.y + extent->y, elementOrigin.z);
    points[3].init (elementOrigin.x, elementOrigin.y, elementOrigin.z + extent->z);

    DPoint3d transformedPoints[4];
    ReprojectStatus status = ReprojectUors (transformedPoints, NULL, NULL, points, 4, destMstnGCS);

    frameA.initFrom4Points (&points[0], &points[1], &points[2], &points[3]);
    frameAInverse.inverseOf (&frameA);

    frameB.initFrom4Points (&transformedPoints[0], &transformedPoints[1], &transformedPoints[2], &transformedPoints[3]);
    RotMatrix axesA, axesB;
    axesA.initFrom (&frameA);
    axesB.initFrom (&frameB);

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
            axesA.getColumn (&xVectorA, 0);
            axesB.getColumn (&xVectorB, 0);
            double scale = xVectorB.magnitude () / xVectorA.magnitude ();
            axesB.scaleColumns (&axesA, scale, scale, scale);
            frameB.setMatrix (&axesB);
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
            axesB.getColumns (&xAxis, &yAxis, &zAxis);
            double f = ax / xAxis.magnitude ();
            xAxis.scale (f);
            yAxis.scale (f);
            zAxis.scale (f);
            axesB.initFromColumnVectors (&xAxis, &yAxis, &zAxis);
            frameB.setMatrix (&axesB);
            }
        else
            {
            // No change except uor effects ...
            axesB.initFromScaleFactors (ax, ax, ax);
            frameB.setMatrix (&axesB);
            }
        }

    // optionally force output frame to be perpendicular
    if (sDoPerpendicularAxes)
        {
        RotMatrix unitB;
        axesB.initFrom (&frameB);
        unitB.squareAndNormalizeColumns (&axesB, 0, 1);
        DVec3d xVectorB;
        axesB.getColumn (&xVectorB, 0);
        double b = xVectorB.magnitude ();
        axesB.scaleColumns  (&unitB, b, b, b);
        }


    outTransform->productOf (&frameB, &frameAInverse);
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCSP         DgnGCS::FromGeoCoordType66
(
const ApplicationElm*   type66,
DgnModelP               cache
)
    {
    // extract the parameters from the type 66.
    CoordinateSystemDgnFormatter*   csf = CoordinateSystemDgnFormatter::GetInstance();
    GeoCoordType66                  extracted;
    ProjectionParams                projectionParams;
    CSParameters*                   csParameters;
    LocalTransformerP               localTransformer;

    // first, extract the type 66 into its two components.
    StatusInt   status;
    if (SUCCESS != (status = csf->Extract (extracted, projectionParams, localTransformer, &type66->appData[0])))
        {
        if (GeoCoordError_BadType66Version != status)
            assert (false);
        return NULL;
        }

    double  paperScale = 1.0;

#if defined (NOTNOW)
    if (csf->IsKeyNameCoordinateSystem (extracted.coordsys))
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

        if (SUCCESS != csf->ConvertToCSDef (csProjectionDefinition, paperScale, extracted, projectionParams))
            {
            assert (false);
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
                
                assert ( (ConvertType_MOLO == convertType) || (ConvertType_3PARM == convertType) || (ConvertType_GEOCTR == convertType) || (ConvertType_BURS == convertType) ||
                         (ConvertType_7PARM == convertType) || (ConvertType_6PARM == convertType) || (ConvertType_4PARM == convertType) );

                memset (&datumDef, 0, sizeof(datumDef));
                datumDefP           = &datumDef;
                CSMap::CS_stncp (datumDef.key_nm, csProjectionDefinition.dat_knm, sizeof (datumDef.key_nm));
                CSMap::CS_stncp (datumDef.name, extracted.dat_nm, sizeof (datumDef.name));
                CSMap::CS_stncp (datumDef.ell_knm, csProjectionDefinition.elp_knm, sizeof (datumDef.ell_knm));
                datumDef.to84_via   = static_cast<short>(extracted.to84_via);   // NEEDSWORK - is cast correct?  Should to84_via be a short?
                datumDef.delta_X    = extracted.delta_X;
                datumDef.delta_Y    = extracted.delta_Y;
                datumDef.delta_Z    = extracted.delta_Z;
                datumDef.rot_X      = extracted.rot_X;
                datumDef.rot_Y      = extracted.rot_Y;
                datumDef.rot_Z      = extracted.rot_Z;
                datumDef.bwscale    = extracted.bwscale;
                }
            else if (0 == csProjectionDefinition.elp_knm[0])
                {
                assert (false);
                return NULL;
                }

            // we must always have the ellipsoid info.
            memset (&ellipsoidDef, 0, sizeof(ellipsoidDef));
            CSMap::CS_stncp (ellipsoidDef.key_nm, csProjectionDefinition.elp_knm, sizeof (ellipsoidDef.key_nm));
            CSMap::CS_stncp (ellipsoidDef.name, extracted.ell_nm, sizeof (ellipsoidDef.name));
            ellipsoidDef.e_rad  = extracted.e_rad;
            ellipsoidDef.p_rad  = extracted.p_rad;

            // fill in flattening and eccentricity.
            Ellipsoid::CalculateParameters (ellipsoidDef.flat, ellipsoidDef.ecent, ellipsoidDef.e_rad, ellipsoidDef.p_rad);
            if (NULL == (csParameters = CSMap::CScsloc2 (&csProjectionDefinition, datumDefP, &ellipsoidDef)))
                {
                assert (false);
                return NULL;
                }
            }

        else if (NULL == (csParameters = CSMap::CScsloc1 (&csProjectionDefinition)))
            {
//          char    errorMsg[512];
//          CSMap::CS_errmsg (errorMsg, DIM(errorMsg));
//          printf ("ERROR: %hs trying to create from saved type 66 parameters\n", errorMsg);
            assert (false);
            return NULL;
            }
        }

    // from the parameters, construct a new DgnGCS .
    DgnGCSP mstnGCS = new DgnGCS (*csParameters, paperScale, extracted.coordsys, cache, SOURCELIB_USER == extracted.dtmOrElpsdFromUserLib);

    // if the VerticalDatumValid is set correctly, extract VerticalDatum code.
    if (GEOCOORD66_VerticalDatumValid == extracted.verticalDatumValid)
        mstnGCS->SetVerticalDatumCode ((VertDatumCode) extracted.verticalDatum);

    // set the local transformer.
    mstnGCS->SetLocalTransformer (localTransformer);

#if defined (TEST_TYPE66_CREATE)
    MSElement   test66;
    s_oldType66             = &extracted;
    s_oldProjectionParams   = &projectionParams;
    mstnGCS->CreateGeoCoordType66 (&test66.applicationElm, cache, true);
    s_oldType66             = NULL;
    s_oldProjectionParams   = NULL;
    const UInt16  *t1, *t2;
    int      count;
    for (t1 = (UInt16*)&type66->appData[429], t2 = (UInt16*)&test66.applicationElm.appData[429], count=429; count < (GEOCOORD66_SIZE / sizeof(short)); count++, t1++, t2++)
        {
        if (*t1 != *t2)
            printf ("difference found at offset %d, existing is 0x%04x, new is 0x%04x\n", count, *t1, *t2);

        }
#endif
    return mstnGCS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
ElementRefP     DgnGCS::FindType66 (DgnModelP cache, bool primary)
    {
    short   desiredSubSig = primary ? GCOORD_66_MAS : GCOORD_66_ALT;

    // iterate the control elements of the modelRef to find the type 66's
    PersistentElementRefListIterator   elementIterator;
    elementIterator.GetFirstElementRef (*cache->GetControlElementsP());

    for (ElementRefP elemRef = elementIterator.GetCurrentElementRef(); NULL != elemRef; elemRef = elementIterator.GetNextElementRef())
        {
        if (MICROSTATION_ELM != elemRef->GetElementType())
            continue;
        if (MSAPPINFO_LEVEL != elemRef->GetLevel())
            continue;

        MSElementCP element;
        if (NULL == (element = elemRef->GetUnstableMSElementCP()))
            continue;

        if (MSGEOCOORD_SIGNATURE != element->applicationElm.signatureWord)
            continue;

        if (desiredSubSig != element->applicationElm.appData[0])
            continue;

        return elemRef;
        }
    return NULL;
    }

/*=================================================================================**//**
* @bsiclass                                                     Keith.Bentley   10/07
+===============+===============+===============+===============+===============+======*/
struct      NotFoundAppData: DgnModelAppData
{
static NotFoundAppData::Key const& GetKey(bool primary)
    {
    static DgnModelAppData::Key s_primary, s_alt;
    return primary ? s_primary : s_alt;
    }

    virtual void        _OnCleanup (DgnModelR) override {delete this;}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
DgnModelAppData::Key const& DgnGCS::GetKey (bool primary)
    {
    static DgnModelAppData::Key s_primary, s_alt;
    return primary ? s_primary : s_alt;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Keith.Bentley                   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnGCS::_OnCleanup (DgnModelR dgnCache)
    {
    Release();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     IsUntransformedAttachment (DgnAttachmentP refP)
    {
    DPoint3dR   masterOrigin = refP->GetStoredMasterOriginR();
    DPoint3dR   refOrigin    = refP->GetRefOriginR();

    if ( (0.0 != masterOrigin.x) || (0.0 != masterOrigin.y) || (0.0 != masterOrigin.z) )
        return false;
    if ( (0.0 != refOrigin.x) || (0.0 != refOrigin.y) || (0.0 != refOrigin.z) )
        return false;
    if (!refP->GetRotMatrixR().IsIdentity())
        return false;
    if (1.0 != refP->GetDisplayScale ())
        return false;

    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   04/08
+---------------+---------------+---------------+---------------+---------------+------*/
void            GetGeoAttachmentTarget
(
DgnModelRefP           &targetModelRefP,
DgnGCSP                &targetGCSP,
DgnModelRefP            refModelRef,
DgnModelRefP            fallbackParentModelRef,
bool                    allowDwg
)
    {
    DgnModelRefP    targetModelRef  = NULL;
    DgnGCSP         targetGCS       = NULL;
    DgnModelRefP    parentModelRef  = (NULL != refModelRef) ? refModelRef->GetParentModelRefP() : fallbackParentModelRef;

    for ( ; (NULL != parentModelRef); parentModelRef = parentModelRef->GetParentModelRefP())
        {
        DgnGCSP     parentGCS;

        // don't allow a DWG file to be involved in reprojection.
        DgnFileFormatType   format = DgnFileFormatType::DWG;
        dgnFileObj_getVersion (parentModelRef ? parentModelRef->GetDgnFileP () : NULL, &format, NULL, NULL);
        if (!allowDwg && ((DgnFileFormatType::DWG == format) || (DgnFileFormatType::DXF == format)) )
            {
            targetModelRef = NULL;
            targetGCS      = NULL;
            break;
            }

        if (NULL != (parentGCS = DgnGCS::FromModel (parentModelRef, true)))
            {
            targetModelRef = parentModelRef;
            targetGCS      = parentGCS;

            // if this modelRef's cache is not set to reprojected, or it has a non-identity transform, then it has a transform to get back to its parent (or its the root). 
            // (The non-identity transform happens in the case where the reference is attached as Reprojected, but we can calculate a linear transform to accomplish that).
            // We want to reproject our data to the GCS of the parent (which targetModelRef is currently set to).
            DgnAttachmentP  refP = parentModelRef->AsDgnAttachmentP();
            if (NULL == refP) 
                break;
            if ( (ATTACHMETHOD_GeographicProjected != refP->GetAttachMethod()) || !IsUntransformedAttachment (refP))
                break;
            }
        else
            {
            // there is no GCS associated with a parent reference. Thus it can't be attached geographically, and we don't want any
            // of it's children attached geographically either.
            targetModelRef = NULL;
            targetGCS      = NULL;
            break;
            }
        }

    targetModelRefP  = targetModelRef;
    targetGCSP       = targetGCS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/07
+---------------+---------------+---------------+---------------+---------------+------*/
void                DgnGCS::GetReprojectionTarget
(
DgnModelRefP           &targetModelRefP,
DgnGCSP                &targetGCSP,
DgnModelRefP            refModelRef,
DgnModelRefP            fallbackParentModelRef
)
    {
    GetGeoAttachmentTarget (targetModelRefP, targetGCSP, refModelRef, fallbackParentModelRef, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCSP         DgnGCS::FromModel
(
DgnModelRefP    modelRef,
bool            primary
)
    {
    return FromCache (modelRef ? modelRef->GetDgnModelP () : NULL, primary);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGCSP         DgnGCS::FromCache
(
DgnModelP       cache,
bool            primary
)
    {
    if ( (NULL == cache) || (DgnModelSections::None == cache->IsFilled(DgnModelSections::ControlElements)) )
        return NULL;

    DgnModelAppData::Key const&  userDataKey = GetKey (primary);

    DgnGCSP     saved = (DgnGCSP ) cache->FindAppData (userDataKey);
    if (NULL != saved)
        return saved;

    // if we already looked, don't continue.
    DgnModelAppData::Key const& notFoundKey = NotFoundAppData::GetKey (primary);
    if (NULL != cache->FindAppData (notFoundKey))
        return NULL;

    ElementRefP     type66Ref;
    DgnGCSP         mstnGCS = NULL;
    if (NULL != (type66Ref = DgnGCS::FindType66 (cache, primary)))
        mstnGCS = DgnGCS::FromGeoCoordType66 (&type66Ref->GetUnstableMSElementCP()->applicationElm, cache);

    // if we don't find the coordinate system, write a "NotFound" key to the modelRef's userData so we don't keep looking.
    if (NULL != mstnGCS)
        {
        mstnGCS->AddRef();
        cache->AddAppData (userDataKey, mstnGCS);
        }
    else
        cache->AddAppData (notFoundKey, new NotFoundAppData());

    return mstnGCS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DgnGCS::CreateGeoCoordType66
(
ApplicationElm*     type66Element,      // <= filled in with data. Must be at least 2000 words.
DgnModelP           cache,
bool                primary
) const
    {
    CoordinateSystemDgnFormatter*   csf = CoordinateSystemDgnFormatter::GetInstance();

    return csf->FormatGeoCoordType66 (type66Element, *m_csParameters, m_coordSysId, cache, primary, m_paperScaleFromType66, m_datumOrEllipsoidFromUserLib, m_verticalDatum, m_localTransformer.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   11/06
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP        DgnGCS::GetProjectionName
(
WStringR    outputBuffer
) const
    {
    DgnProjectionTypes projectionType = BaseGCS::DgnProjectionTypeFromCSDefName (m_csParameters->csdef.prj_knm);
    return BaseGeoCoordResource::GetLocalizedProjectionName (outputBuffer, projectionType);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         DgnGCS::GetDisplayName (WStringR outputBuffer) const
    {
    if ( (NULL != m_csParameters) && (0 != *m_csParameters->csdef.key_nm) )
        {
        outputBuffer.AssignA (m_csParameters->csdef.key_nm);
        }
    else 
        {
        GetProjectionName (outputBuffer);
        }

    if (m_localTransformer.IsValid())
        {
        WString transformerDescription;
        m_localTransformer->GetDescription(transformerDescription);

        if (!transformerDescription.empty())
            {
            outputBuffer.append (L" ");
            outputBuffer.append (transformerDescription);
            }
        }

    return outputBuffer.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DgnGCS::GetUnitDefinition
(
UnitDefinitionR unitDef,
StandardUnit&   standardUnitNumber
) const
    {
    standardUnitNumber = StandardUnit::None;

    int     csUnitCode;
    if (-1 == (csUnitCode = GetUnitCode ()))
        {
        // should never happen.
        assert (false);
        unitDef.Init (DgnPlatform::UnitBase::Meter, DgnPlatform::UnitSystem::Metric, 1.0, 1.0, L"Meters"); 
        return ERROR;
        }

    // convert from CS units to our UnitInfo.
    cs_Unittab_ const* csUnits = CSMap::GetCSUnitInfo (csUnitCode);

    DgnPlatform::UnitBase base = (cs_UTYP_LEN == csUnits->type) ? DgnPlatform::UnitBase::Meter : DgnPlatform::UnitBase::Degree;

    // CS_Map has only a numerator, no denominator.
    unitDef.Init (base, DgnPlatform::UnitSystem::Undefined, 1.0, csUnits->factor, L""); 

    standardUnitNumber = unitDef.IsStandardUnit ();
    if ( (StandardUnit::None == standardUnitNumber) || (StandardUnit::Custom == standardUnitNumber) )
        {
        DgnPlatform::UnitSystem system;

        if (cs_USYS_Metric == csUnits->system)
            system = UnitSystem::Metric;
        else if (cs_USYS_English == csUnits->system)
            system = UnitSystem::English;
        else
            system = UnitSystem::Undefined;

        WString label (csUnits->name);
        unitDef.Init (base, system, 1.0, csUnits->factor, label.c_str()); 
        }
    else
        {
        // get the MicroStation definition if we have one.
        unitDef = UnitDefinition::GetStandardUnit ((StandardUnit) standardUnitNumber);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DgnGCS::UnitsIdentical (DgnGCSCR other) const
    {
    if (m_uorsPerBaseUnit != other.m_uorsPerBaseUnit)
        return false;

    return m_globalOrigin.IsEqual (other.m_globalOrigin, 0.000001);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
static bool         unitsSame
(
CSUnitInfo const *csUnitInfo,
UnitDefinitionCR  unitDef
)
    {
    // match the "base" in MicroStation terms.
    if ( (cs_UTYP_LEN == csUnitInfo->type) && (UnitBase::Meter != unitDef.GetBase()) )
        return false;

    if ( (cs_UTYP_ANG == csUnitInfo->type) && (UnitBase::Degree != unitDef.GetBase()) )
        return false;

    // if it's not LEN or ANG, can't match.
    if ( (cs_UTYP_LEN != csUnitInfo->type) &&  (cs_UTYP_ANG != csUnitInfo->type) )
        return false;

    // adapted from units.c
    double  prod1 = unitDef.GetDenominator();
    double  prod2 = unitDef.GetNumerator() * csUnitInfo->factor;

    return (1.0e-9 * fabs (prod1 + prod2) >= fabs (prod1 - prod2));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DgnGCS::GetCSUnitName
(
WStringR            csUnitName,
UnitDefinitionCR    unitDef
)
    {
    for (int icsUnit = 0; ; icsUnit++)
        {
        CSUnitInfo const*  csUnitInfo;
        if (NULL == (csUnitInfo = CSMap::GetCSUnitInfo (icsUnit)))
            break;
        if (unitsSame (csUnitInfo, unitDef))
            {
            csUnitName.AssignA (csUnitInfo->name);
            return SUCCESS;
            }
        }
    return ERROR;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DgnGCS::ToModel
(
DgnModelRefP        modelRef,
bool                primary,
bool                writeToFile,
bool                reprojectData,
bool                reportProblems
)
    {
    DgnModelP   cache;
    if (NULL == (cache = (modelRef ? modelRef->GetDgnModelP () : NULL)))
        return GEOCOORDERR_BadArg;

    if (!IsValid())
        return GEOCOORDERR_InvalidCoordSys;

    // send out the BeforeCoordinateSystemChanged event to listeners.
    DgnGCSPtr     oldGCSPtr   = Bentley::GeoCoordinates::DgnGCS::FromModel (modelRef, primary);
    if (NULL != s_eventHandlers)
        {
        T_GeoCoordEventHandlers::iterator   listIterator;
        // send to each listener.
        for (listIterator = s_eventHandlers->begin(); listIterator != s_eventHandlers->end(); listIterator++)
            {
            IGeoCoordinateEventHandler*     handler = *listIterator;
            StatusInt                       handlerStatus;
            if (SUCCESS != (handlerStatus = handler->BeforeCoordinateSystemChanged (oldGCSPtr.get(), this, modelRef, primary, writeToFile, reprojectData)))
                return handlerStatus;
            }
        }

    // make sure the scale and global origin is set right.
    InitCacheParameters (cache, m_paperScaleFromType66);

    if (reprojectData && writeToFile)
        dgnGeoCoord_reprojectToGCS (modelRef, this, reportProblems);

    if (writeToFile)
        {
        MSElement   test66;

        StatusInt   status;
        if (SUCCESS != (status = this->CreateGeoCoordType66 (&test66.applicationElm, cache, primary)))
            {
            assert (false);
            return status;
            }

        EditElementHandle   newType66 (&test66, cache);
        ElementRefP         existingType66;
        if (NULL == (existingType66 = FindType66 (cache, primary)))
            newType66.AddToModel ();
        else
            newType66.ReplaceInModel (existingType66);
        }

    DgnModelAppData::Key const& userDataKey = GetKey (primary);
    DgnGCSP     saved = (DgnGCSP ) cache->FindAppData (userDataKey);
    if (this != saved)
        {
        AddRef();
        cache->AddAppData (userDataKey, this);
        }

    // clear any "not found" indication
    DgnModelAppData::Key const& notFoundKey = NotFoundAppData::GetKey (primary);
    cache->DropAppData (notFoundKey);

    ReloadGeoReferences (modelRef);

    // send out the AfterCoordinateSystemChanged event to listeners.
    if (NULL != s_eventHandlers)
        {
        T_GeoCoordEventHandlers::iterator   listIterator;
        // send to each listener.
        for (listIterator = s_eventHandlers->begin(); listIterator != s_eventHandlers->end(); listIterator++)
            (*listIterator)->AfterCoordinateSystemChanged (oldGCSPtr.get(), this, modelRef, primary, writeToFile, reprojectData);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnGCS::ReloadGeoReferences
(
DgnModelRefP    modelRef
)
    {
    // now we step through the references and reload all of those that are attached with ATTACHMETHOD_GeographicTransformed or ATTACHMETHOD_GeographicProjected.
    ModelRefIterator  mrIterator (modelRef, MRITERATE_PrimaryChildRefs, 0);
    DgnModelRefP      childModelRef;

    for (childModelRef = mrIterator.GetFirst (); NULL != childModelRef; childModelRef = mrIterator.GetNext ())
        {
        DgnAttachmentP   childRefP;
        if ( (NULL == (childRefP = childModelRef->AsDgnAttachmentP())) || childRefP->IsMissingFile())
            continue;

#if defined (BEIJING_DGNPLATFORM_WIP_GEOCOORD)
        if (ATTACHMETHOD_GeographicTransformed == childRefP->GetAttachMethod())
            mdlRefFile_reload (childModelRef, true, false);

        else if (ATTACHMETHOD_GeographicProjected == childRefP->GetAttachMethod())
            mdlRefFile_reload (childModelRef, true, true);
#endif
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DgnGCS::DeleteFromModel
(
DgnModelRefP    modelRef,
bool            primary
)
    {
    DgnModelP   cache = (modelRef ? modelRef->GetDgnModelP () : NULL);

    if (NULL == cache)
        return GEOCOORDERR_BadArg;

    // send out the BeforeCoordinateSystenDeleted event to listeners.
    DgnGCSPtr     currentGCSPtr   = Bentley::GeoCoordinates::DgnGCS::FromModel (modelRef, primary);
    if (NULL != s_eventHandlers)
        {
        T_GeoCoordEventHandlers::iterator   listIterator;
        // send to each listener.
        for (listIterator = s_eventHandlers->begin(); listIterator != s_eventHandlers->end(); listIterator++)
            {
            IGeoCoordinateEventHandler*     handler = *listIterator;
            StatusInt                       handlerStatus;
            if (SUCCESS != (handlerStatus = handler->BeforeCoordinateSystemDeleted (currentGCSPtr.get(), modelRef, primary)))
                return handlerStatus;
            }
        }

    DgnModelAppData::Key const& userDataKey = GetKey (primary);
    cache->DropAppData (userDataKey);

    DgnModelAppData::Key const& notFoundKey = NotFoundAppData::GetKey (primary);
    cache->DropAppData (notFoundKey);

    // find type 66 and delete it.
    ElementRefP         existingType66;
    if (NULL == (existingType66 = FindType66 (cache, primary)))
        return GEOCOORDERR_CoordSysNotFound;

    EditElementHandle handle (existingType66, modelRef);
    handle.DeleteFromModel ();

    SendAfterGeoCoordinateSystemDeleted (currentGCSPtr.get(), modelRef, primary);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnGCS::SendAfterGeoCoordinateSystemDeleted
(
DgnGCSP         currentGCSP,
DgnModelRefP    modelRef,
bool            primary
)
    {
    // send out the AfterCoordinateSystemDeleted event to listeners.
    if (NULL != s_eventHandlers)
        {
        T_GeoCoordEventHandlers::iterator   listIterator;
        // send to each listener.
        for (listIterator = s_eventHandlers->begin(); listIterator != s_eventHandlers->end(); listIterator++)
            (*listIterator)->AfterCoordinateSystemDeleted (currentGCSP, modelRef, primary);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnGCS::SetEventHandler
(
IGeoCoordinateEventHandler*     eventHandler
)
    {
    if (NULL == s_eventHandlers)
        s_eventHandlers = new T_GeoCoordEventHandlers;
    else
        {
        T_GeoCoordEventHandlers::iterator   listIterator;
        // don't duplicate
        for (listIterator = s_eventHandlers->begin(); listIterator != s_eventHandlers->end(); listIterator++)
            {
            IGeoCoordinateEventHandler*  testHandler = *listIterator;
            if (eventHandler == testHandler)
                return;
            }
        }
    s_eventHandlers->push_back (eventHandler);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            DgnGCS::RemoveEventHandler
(
IGeoCoordinateEventHandler*     eventHandler
)
    {
    if (NULL == s_eventHandlers)
        return;
    else
        {
        T_GeoCoordEventHandlers::iterator   listIterator;
        // remove it if we find it.
        for (listIterator = s_eventHandlers->begin(); listIterator != s_eventHandlers->end(); listIterator++)
            {
            IGeoCoordinateEventHandler*  testHandler = *listIterator;
            if (eventHandler == testHandler)
                {
                s_eventHandlers->erase (listIterator);
                return;
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
void                    DgnGCS::SendBeforeGeoCoordinationChanged
(
DgnModelRefP            modelRef,
GeoCoordinationState    oldState,
GeoCoordinationState    newState
)
    {
    // send out the AfterCoordinateSystemDeleted event to listeners.
    if (NULL != s_eventHandlers)
        {
        T_GeoCoordEventHandlers::iterator   listIterator;
        // send to each listener.
        for (listIterator = s_eventHandlers->begin(); listIterator != s_eventHandlers->end(); listIterator++)
            (*listIterator)->BeforeReferenceGeoCoordinationChanged (modelRef, oldState, newState);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
void                    DgnGCS::SendAfterGeoCoordinationChanged
(
DgnModelRefP            modelRef,
GeoCoordinationState    oldState,
GeoCoordinationState    newState
)
    {
    // send out the AfterCoordinateSystemDeleted event to listeners.
    if (NULL != s_eventHandlers)
        {
        T_GeoCoordEventHandlers::iterator   listIterator;
        // send to each listener.
        for (listIterator = s_eventHandlers->begin(); listIterator != s_eventHandlers->end(); listIterator++)
            (*listIterator)->AfterReferenceGeoCoordinationChanged (modelRef, oldState, newState);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool                    DgnGCS::UpdateUnitInfo (DgnModelP cache)
    {
    // returns true if there's a difference.
    if ( (NULL == cache) || (DgnModelSections::None == cache->IsFilled (DgnModelSections::ControlElements)) )
        return false;

    // only need to worry about it if it's cached.
    DgnModelAppData::Key const&  userDataKey = GetKey (true);
    DgnGCSP    cachedGCS;
    if (NULL == (cachedGCS = (DgnGCSP) cache->FindAppData (userDataKey)))
        return false;

    DPoint3d    globalOrigin;
    dgnModel_getGlobalOrigin (cache, &globalOrigin);
    if ( (globalOrigin.x != cachedGCS->m_globalOrigin.x) || (globalOrigin.y != cachedGCS->m_globalOrigin.y) || (globalOrigin.z != cachedGCS->m_globalOrigin.z) )
        {
        cachedGCS->m_globalOrigin = globalOrigin;
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
double          DgnGCS::GetPaperScale () const
    {
    return m_paperScaleFromType66;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       DgnGCS::SetPaperScale (double paperScale, DgnModelRefP modelRef)
    {
    DgnModelP   cache;
    if ( (NULL == modelRef) || (NULL == (cache = modelRef->GetDgnModelP ())) )
        return ERROR;

    if (0.0 == paperScale)
        return ERROR;

    InitCacheParameters (cache, paperScale);
    return SUCCESS;
    }

}   // End of GeoCoordinates namespace
}   // End of Bentley namespace

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   08/06
+---------------+---------------+---------------+---------------+---------------+------*/
DGNGEOCOORD_EXPORTED    DgnGCSP    dgnGeoCoord_readCoordinateSystem
(
DgnModelRefP    modelRef,
bool            primary
)
    {
    return Bentley::GeoCoordinates::DgnGCS::FromModel (modelRef, FALSE != primary);
    }


/*=================================================================================**//**
* Subtype Handler for Type66, level MSAPPINFO, signature MSGEOCOORD_SIGNATURE
* @bsiclass                                                     Barry.Bentley   11/07
+===============+===============+===============+===============+===============+======*/
struct      GeoCoordType66Handler : Type66Handler, ISubTypeHandlerQuery, ITransactionHandler, DgnHistory::UI::IGetDescription
{
DEFINE_T_SUPER(Type66Handler)
ELEMENTHANDLER_DECLARE_MEMBERS (GeoCoordType66Handler,)

/*---------------------------------------------------------------------------------**//**
* TODO: Not sure if this should be different
* @bsimethod                                                    Kevin.Nyman     09/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            _GetTypeName (WStringR string, UInt32 desiredLength)
    {
    Bentley::GeoCoordinates::BaseGeoCoordResource::GetLocalizedString (string, Bentley::GeoCoordinates::DGNGEOCOORD_Msg_ElementTypeName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    barry.bentley                   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void        _GetDescription (ElementHandleCR el, WStringR descr, UInt32 desiredLength) override
    {
    _GetTypeName(descr, desiredLength);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    barry.bentley                   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void        _GetDescription (WStringR desc, bool& advanced, ElementHandleCR eh, BentleyDgnHistoryElementChangeType ct, DgnFileP file, ModelId mid) override
    {
    return GetDescriptionVisible (desc, advanced, eh, ct, file, mid);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    barry.bentley                   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ITransactionHandlerP    _GetITransactionHandler() override
    {
    return this;
    }

/*=================================================================================**//**
* @bsiclass                                                     Barry.Bentley   03/11
+===============+===============+===============+===============+===============+======*/
struct          ReprojectTxn : DgnCacheTxn
{
ITxn*   m_oldTxn;

ReprojectTxn ()  { m_oldTxn =  &ITxnManager::GetManager().SetCurrentTxn (*this); }
~ReprojectTxn () { ITxnManager::GetManager().SetCurrentTxn (*m_oldTxn); }
protected:
    virtual void        _ClearReversedTxns() override {}
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    barry.bentley                   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void    _OnUndoRedo (ElementHandleP afterUndoRedo, ElementHandleP beforeUndoRedo, ChangeTrackAction action, bool isUndo, ChangeTrackSource source) override
    {
    ElementHandleP         elToCheck;
    bool                assigning;
    bool                primary;

    if (NULL != afterUndoRedo)
        {
        elToCheck   = afterUndoRedo;
        assigning   = true;
        }
    else if (NULL != beforeUndoRedo)
        {
        elToCheck   = beforeUndoRedo;
        assigning   = false;
        }
    else
        return;

    MSElementCP element = elToCheck->GetElementCP();

    if (GCOORD_66_MAS == element->applicationElm.appData[0])
        primary = true;
    else if (GCOORD_66_ALT == element->applicationElm.appData[0])
        primary = false;
    else
        return;

    DgnModelRefP    modelRef;
    if (NULL == (modelRef = elToCheck->GetModelRef()))
        return;

    DgnModelP   cache;
    if (NULL == (cache = (modelRef ? modelRef->GetDgnModelP () : NULL)))
        return;

    if (assigning)
        {
        // get the coordinate system from the model.
        // Assign it to the model, without writing it (it's already in the cache) and without reprojecting.
        DgnGCSP  mstnGCS;
        if (NULL == (mstnGCS = Bentley::GeoCoordinates::DgnGCS::FromGeoCoordType66 (&element->applicationElm, cache)))
            return;

        // Fix any attached references.
        // Before we do, we need to set up a non-undoable txn (which actually replaces the IllegalTxn that undo/redo uses.
        // This is for the case where a reloaded reference is a non-DGN file, where the loader/importer does some operations that do "AddElement".
        //   One such example is TR#314289, where there's an IFC nested reference file.
        // ReprojectTxn installs itself as the current Txn, and restores the previous Txn when its destructor is called.
        ReprojectTxn    reprojectTxn;
        mstnGCS->ToModel (modelRef, primary, false, false, false);
        }
    else
        {
        // If we're undoing an assignment, or redoing a delete
        DgnModelAppData::Key const& userDataKey = GeoCoordinates::DgnGCS::GetKey (primary);
        cache->DropAppData (userDataKey);

        DgnModelAppData::Key const&  notFoundKey = GeoCoordinates::NotFoundAppData::GetKey (primary);
        cache->DropAppData (notFoundKey);

        // get current GCS to send to asynch.
        DgnGCSP  mstnGCS = Bentley::GeoCoordinates::DgnGCS::FromGeoCoordType66 (&element->applicationElm, cache);

        // send out the AfterCoordinateSystemDeleted event to listeners.
        GeoCoordinates::DgnGCS::SendAfterGeoCoordinateSystemDeleted (mstnGCS, modelRef, primary);

        // fix any attached references.
        // See comment above about need for ReprojectTxn
        ReprojectTxn    reprojectTxn;
        GeoCoordinates::DgnGCS::ReloadGeoReferences (modelRef);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    barry.bentley                   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual void        _OnHistoryRestore (ElementHandleP after, ElementHandleP before, ChangeTrackAction action, BentleyDgnHistoryElementChangeType) override
    {
    _OnUndoRedo (after, before, action, false, ChangeTrackSource::HistoryRestore);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    barry.bentley                   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
virtual Handler*    _GetSubTypeHandlerForElement (ElementHandleCR eh) override
    {
    if (MSAPPINFO_LEVEL == eh.GetElementCP()->ehdr.level)
        {
        if (MSGEOCOORD_SIGNATURE == eh.GetElementCP()->applicationElm.signatureWord)
            {
            return this;
            }
        }
    return NULL;
    };

};


ELEMENTHANDLER_DEFINE_MEMBERS(GeoCoordType66Handler)

extern void baseGeoCoord_setUserLibrary (WCharCP libPath, WCharCP guiName);

/*---------------------------------------------------------------------------------**//**
* @bsistruct                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
struct GeoCoordECExtension : ACSElementECExtensionBase
    {
private:
    GeoCoordECExtension() : ACSElementECExtensionBase() { }

    virtual bool            _SupportsModifyACS() const override                             { return false; }
    virtual void            _GenerateACS (bvector<IAuxCoordSysPtr>& acsList, ElementHandleCR eh) const override;
public:
    static GeoCoordECExtension* Create() { return new GeoCoordECExtension(); }
    };

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Paul.Connelly   07/12
+---------------+---------------+---------------+---------------+---------------+------*/
void GeoCoordECExtension::_GenerateACS (bvector<IAuxCoordSysPtr>& acsList, ElementHandleCR eh) const
    {
    struct AcsFinder : IACSTraversalHandler
        {
    private:
        bvector<IAuxCoordSysPtr>&   m_acsList;
        virtual UInt32              _GetACSTraversalOptions() override { return 0; }
        virtual bool                _HandleACSTraversal (IAuxCoordSysR acs) override
            {
            if (acs.GetExtenderId() == GEOCOORD_ACS_EXTENDERID || acs.GetExtenderId() == GEOCOORD_MGRS_EXTENDERID)
                m_acsList.push_back (&acs);
            return false;
            }
    public:
        AcsFinder (bvector<IAuxCoordSysPtr>& acsList) : m_acsList(acsList) { }
        };

    AcsFinder finder (acsList);
    IACSManager::GetManager().Traverse (finder, eh.GetModelRef());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeoCoordinationAdmin*  DgnGeoCoordinationAdmin::Create (WCharCP dataDirectory, IACSManagerR mgr)
    {
    return new DgnGeoCoordinationAdmin (dataDirectory, mgr);
    }

// used for address to pass to BeGetModuleFileName.
static void dummy() {;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void DgnGeoCoordinationAdmin::CompleteInitialization () const
    {
    RUNONCE_CHECK (m_initializationComplete)

    // if not configured for OTF transform, don't do it.
    m_otfEnabled = (!ConfigurationManager::IsVariableDefined (L"MS_GEOCOORDINATE_NO_OTFTRANSFORM"));

    if (m_dataDirectory.empty())
        {
        WString     gcsDataDir;
        if (BSISUCCESS == ConfigurationManager::GetVariable (gcsDataDir, L"MS_GEOCOORDINATE_DATA"))
            {
            Bentley::GeoCoordinates::BaseGCS::Initialize (gcsDataDir.c_str());
            m_dataDirectory.assign (gcsDataDir);
            }
        else
            {
            // we did not get anything useful from the initial call to the DgnGeoCoordinationAdmin ::Create, nor was there
            //  anything useful in MS_GEOCOORDINATE_DATA. Therefore, we try to use the GeoCoordinateData subdirectory 
            //  of the directory from which this module was loaded.
            BeFileName  dllFileName;
            if (SUCCESS == Bentley::BeGetModuleFileName (dllFileName, (void*)&dummy))
                {
                WString device;
                WString dir;
                dllFileName.ParseName (&device, &dir, NULL, NULL);
                dir.append (L"GeoCoordinateData");

                BeFileName  geoCoordDataDir;
                geoCoordDataDir.BuildName (device.c_str(), dir.c_str(), NULL, NULL);
                Bentley::GeoCoordinates::BaseGCS::Initialize (geoCoordDataDir.GetName());
                m_dataDirectory.assign (geoCoordDataDir.GetName());
                }
            }
        }
    else
        {
        Bentley::GeoCoordinates::BaseGCS::Initialize (m_dataDirectory.c_str());
        }

    // Register an extension for EC properties
    ElementECExtension::RegisterExtension (GeoCoordType66Handler::GetInstance(), * GeoCoordECExtension::Create());

    // Set the additional GeoCoordinate libraries, if any.
    // The configuration variable, MS_GEOCOORDINATE_USERLIBRARIES, is string of the form "libraryPath1[Name1];libraryPath2[Name2]"
    // where libraryPath<x> is the full path to the library, and [Name<x>] is how the name appears in the GUI.
    // For example, suppose a consultant worked with the California and Oregon Departments of Transportation,
    // and each supplied a coordinate system library. The libraryList might look like:
    // "d:\geocoordinateLibraries\CaltransCoordSystems.dty[Caltrans GCS];d:\geocoordinateLibraries\OregonDOT.dty[Oregon DOT GCS]"
    // The library file spec can contain configuration variables within it like other configuration variables,
    // like "GEODATA:CaltransCoordSystems.dty[Caltrans GCS];GEODATA:OregonDOT.dty[Oregon DOT GCS]".
    // If the name is not specified, the root of the file name is used.
    WString userLibs;
    if (SUCCESS != ConfigurationManager::GetVariable(userLibs, L"MS_GEOCOORDINATE_USERLIBRARIES"))
        return;


    WString guiName;
    WString libName;
    for (;;)
        {
        guiName.clear();
        libName.clear();

        // skip any separators at the beginning.
        while (userLibs[0] == ';')
            userLibs.erase (0, 1);

        if (userLibs.empty())
            break;

        size_t  leftBracket;
        size_t  rightBracket;
        size_t  semicolon;
        if ( (WString::npos != (leftBracket = userLibs.find ('['))) && (WString::npos != (rightBracket = userLibs.find (']', leftBracket))) )
            {
            guiName = userLibs.substr (leftBracket + 1, (rightBracket - (leftBracket+1)));
            libName = userLibs.substr (0, leftBracket);
            userLibs.erase (0, rightBracket+1);
            }

        else if (WString::npos != (semicolon = userLibs.find (';')))
            {
            libName = userLibs.substr (0, semicolon);
            userLibs.erase (0, semicolon+1);
            }
        else
            {
            libName.assign (userLibs);
            userLibs.erase();
            }

        // now, libP = the null terminated library specification, and guiName is either NULL or the GUI Name for the library.
        BeFileName  outLib;
        if (SUCCESS == util_findFile (NULL, &outLib, libName.c_str(), L"MS_GEOCOORDINATE_DATA", L".dty", 0))
            baseGeoCoord_setUserLibrary (outLib.GetName(), guiName.empty() ? NULL : guiName.c_str());
        else
            {
            // show error message.
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
DgnGeoCoordinationAdmin::DgnGeoCoordinationAdmin (WCharCP dataDirectory, IACSManagerR mgr)
    {
    m_initializationComplete  = false;
    m_gcrp                    = NULL;
    if (NULL != dataDirectory)
        m_dataDirectory.assign (dataDirectory);
    m_otfEnabled = false;

    // register the IACS stuff
    AddAuxCoordSystemProcessor (mgr);

    Handler::RegisterSubTypeHandler (MICROSTATION_ELM, ELEMENTHANDLER_INSTANCE (GeoCoordType66Handler));
    }


