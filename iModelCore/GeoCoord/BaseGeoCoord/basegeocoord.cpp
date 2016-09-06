/*----------------------------------------------------------------------+
|
|   $Source: BaseGeoCoord/basegeocoord.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+----------------------------------------------------------------------*/
#ifdef _MSC_VER
#pragma  warning(disable:4242) // toupper returns an int which is stuffed into this char string.
#pragma  warning(disable:4189) // local variable is initialized but not referenced
#endif

//#include    <windows.h>
#include    <GeoCoord/BaseGeoCoord.h>
#include    <GeoCoord/basegeocoordapi.h>
#include    <GeoCoord/GCSLibrary.h>
#include    <csmap/csNameMapperSupport.h>
#include    <csmap/cs_map.h>
#include    <csmap/cs_Legacy.h>
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <stdlib.h>
#include    <stdio.h>
#if defined (BENTLEY_WIN32) || defined (BENTLEY_WINRT)
#include	<direct.h>
#include    <io.h>
#include    <sstream>
#else

#endif


#include    <Bentley/BeFileName.h>

#include <algorithm>
#include <cctype>

// cs_wkt requires __CPP__ to be defined and the class TrcWktElement to be defined (what's up with that?)
#define __CPP__
class TrcWktElement;
#include    <csmap/CS_wkt.h>
#undef  __CPP__

extern "C" int      cs_Error;
extern "C" char     csErrnam[];
extern "C" struct   cs_Unittab_     cs_Unittab[];
extern "C" struct   cs_PrjprmMap_   cs_PrjprmMap[];
extern "C" struct   cs_Prjtab_      cs_Prjtab[];
extern "C" struct   cs_Prjprm_      csPrjprm[];
extern "C" char     cs_Csname[];
extern "C" char     cs_Dir[];
extern "C" char*    cs_DirP;
extern "C" int32_t csLatFrmt;
extern "C" int32_t csLngFrmt;
extern "C" int32_t csAnglFrmt;
extern "C" int32_t csXyFrmt;
extern "C" int32_t csZzFrmt;
extern "C" int32_t csRedFrmt;
extern "C" int32_t csSclFrmt;
extern "C" int32_t csCoefFrmt;
extern "C" const uint32_t KcsNmInvNumber;

#if defined (GEOCOORD_ENHANCEMENT)
extern  bool CS_wktDatumLookUp (const char* datumNameInWkt, char* csDatumName);
#endif

#define DIM(a) (sizeof(a)/sizeof(a[0]))

#define CSMAP_FREE_AND_CLEAR(ptr) {if (NULL != ptr){CSMap::CS_free (ptr) ; ptr=NULL;}}

BEGIN_BENTLEY_NAMESPACE

namespace GeoCoordinates {

bool BaseGCS::s_geoCoordInitialized = false;

struct CoordSysData
{
DgnProjectionTypes              dgnProjectionType;  // this is the value saved in our coordinate system elements.
CharCP                          csMapKeyName;       // this is the CS-Map keyname.
BaseGCS::ProjectionCodeValue    csMapProjCodeValue; // this is the CS-Map Projection Code.
};

static  CoordSysData    csDataMap[] =
{
  { COORDSYS_ALBER,     CS_ALBER,   BaseGCS::pcvAlbersEqualArea,                            /* cs_PRJCOD_ALBER    */ },
  { COORDSYS_AZMEA,     CS_AZMEA,   BaseGCS::pcvLambertEqualAreaAzimuthal,                  /* cs_PRJCOD_AZMEA    */ },
  { COORDSYS_AZMED,     CS_AZMED,   BaseGCS::pcvLambertEquidistantAzimuthal,                /* cs_PRJCOD_AZMED    */ },
  { COORDSYS_BONNE,     CS_BONNE,   BaseGCS::pcvBonne,                                      /* cs_PRJCOD_BONNE    */ },
  { COORDSYS_BPCNC,     CS_BPCNC,   BaseGCS::pcvBipolarObliqueConformalConic,               /* cs_PRJCOD_BPCNC    */ },
  { COORDSYS_CSINI,     CS_CSINI,   BaseGCS::pcvCassini,                                    /* cs_PRJCOD_CSINI    */ },
  { COORDSYS_EDCNC,     CS_EDCNC,   BaseGCS::pcvEquidistantConic,                           /* cs_PRJCOD_EDCNC    */ },
  { COORDSYS_EDCYL,     CS_EDCYL,   BaseGCS::pcvEquidistantCylindrical,                     /* cs_PRJCOD_EDCYL    */ },
  { COORDSYS_EKRT4,     CS_EKRT4,   BaseGCS::pcvEckertIV,                                   /* cs_PRJCOD_EKRT4    */ },
  { COORDSYS_EKRT6,     CS_EKRT6,   BaseGCS::pcvEckertVI,                                   /* cs_PRJCOD_EKRT6    */ },
  { COORDSYS_GNOMC,     CS_GNOMC,   BaseGCS::pcvGnomonic,                                   /* cs_PRJCOD_GNOMC    */ },
  { COORDSYS_HMLSN,     CS_HMLSN,   BaseGCS::pcvGoodeHomolosine,                            /* cs_PRJCOD_HMLSN    */ },
  { COORDSYS_LMBRT,     CS_LMBRT,   BaseGCS::pcvLambertConformalConicTwoParallel,           /* cs_PRJCOD_LM2SP    */ },
  { COORDSYS_LMTAN,     CS_LMTAN,   BaseGCS::pcvLambertTangential,                          /* cs_PRJCOD_LMTAN    */ },
  { COORDSYS_MILLR,     CS_MILLR,   BaseGCS::pcvMillerCylindrical,                          /* cs_PRJCOD_MILLR    */ },
  { COORDSYS_MODPC,     CS_MODPC,   BaseGCS::pcvModifiedPolyconic,                          /* cs_PRJCOD_MODPC    */ },
  { COORDSYS_MOLWD,     CS_MOLWD,   BaseGCS::pcvMollweide,                                  /* cs_PRJCOD_MOLWD    */ },
  { COORDSYS_MRCAT,     CS_MRCAT,   BaseGCS::pcvMercator,                                   /* cs_PRJCOD_MRCAT    */ },
  { COORDSYS_MSTRO,     CS_MSTRO,   BaseGCS::pcvModifiedStereographic,                      /* cs_PRJCOD_MSTRO    */ },
  { COORDSYS_NACYL,     CS_NACYL,   BaseGCS::pcvEqualAreaAuthalicNormal,                    /* cs_PRJCOD_NACYL    */ },
  { COORDSYS_NZLND,     CS_NZLND,   BaseGCS::pcvNewZealandNationalGrid,                     /* cs_PRJCOD_NZLND    */ },
  { COORDSYS_OBLQ1,     CS_OBLQ1,   BaseGCS::pcvHotineObliqueMercator1XY,                   /* cs_PRJCOD_HOM1XY   */ },
  { COORDSYS_OBLQ2,     CS_OBLQ2,   BaseGCS::pcvHotineObliqueMercator2XY,                   /* cs_PRJCOD_HOM2XY   */ },
  { COORDSYS_ORTHO,     CS_ORTHO,   BaseGCS::pcvOrthographic,                               /* cs_PRJCOD_ORTHO    */ },
  { COORDSYS_PLYCN,     CS_PLYCN,   BaseGCS::pcvAmericanPolyconic,                          /* cs_PRJCOD_PLYCN    */ },
  { COORDSYS_ROBIN,     CS_ROBIN,   BaseGCS::pcvRobinsonCylindrical,                        /* cs_PRJCOD_ROBIN    */ },
  { COORDSYS_SINUS,     CS_SINUS,   BaseGCS::pcvSinusoidal,                                 /* cs_PRJCOD_SINUS    */ },
  { COORDSYS_STERO,     CS_STERO,   BaseGCS::pcvSnyderObliqueStereographic,                 /* cs_PRJCOD_SSTRO    */ },
  { COORDSYS_TACYL,     CS_TACYL,   BaseGCS::pcvEqualAreaAuthalicTransverse,                /* cs_PRJCOD_TACYL    */ },
  { COORDSYS_TRMER,     CS_TRMER,   BaseGCS::pcvTransverseMercator,                         /* cs_PRJCOD_TRMER    */ },
  { COORDSYS_UNITY,     CS_UNITY,   BaseGCS::pcvUnity,                                      /* cs_PRJCOD_UNITY    */ },
  { COORDSYS_VDGRN,     CS_VDGRN,   BaseGCS::pcvVanderGrinten,                              /* cs_PRJCOD_VDGRN    */ },

  { COORDSYS_UTMZN,     CS_UTMZN,   BaseGCS::pcvUniversalTransverseMercator,                /* cs_PRJCOD_UTM      */ },
  { COORDSYS_LM1SP,     CS_LM1SP,   BaseGCS::pcvLambertConformalConicOneParallel,           /* cs_PRJCOD_LM1SP    */ },
  { COORDSYS_OSTRO,     CS_OSTRO,   BaseGCS::pcvObliqueStereographic,                       /* cs_PRJCOD_OSTRO    */ },
  { COORDSYS_PSTRO,     CS_PSTRO,   BaseGCS::pcvPolarStereographic,                         /* cs_PRJCOD_PSTRO    */ },
  { COORDSYS_RSKWC,     CS_RSKWC,   BaseGCS::pcvRectifiedSkewOrthomorphicCentered,          /* cs_PRJCOD_RSKEWC   */ },
  { COORDSYS_RSKEW,     CS_RSKEW,   BaseGCS::pcvRectifiedSkewOrthomorphic,                  /* cs_PRJCOD_RSKEW    */ },
  { COORDSYS_SWISS,     CS_SWISS,   BaseGCS::pcvObliqueCylindricalSwiss,                    /* cs_PRJCOD_SWISS    */ },
  { COORDSYS_LMBLG,     CS_LMBLG,   BaseGCS::pcvLambertConformalConicBelgian,               /* cs_PRJCOD_LMBLG    */ },
  { COORDSYS_SOTRM,     CS_SOTRM,   BaseGCS::pcvSouthOrientedTransverseMercator,            /* cs_PRJCOD_SOTRM    */ },
  { COORDSYS_HOM1U,     CS_HOM1U,   BaseGCS::pcvHotineObliqueMercator1UV,                   /* cs_PRJCOD_HOM1UV   */ },
  { COORDSYS_HOM2U,     CS_HOM2U,   BaseGCS::pcvHotineObliqueMercator2UV,                   /* cs_PRJCOD_HOM2UV   */ },

  { COORDSYS_GAUSK,     CS_GAUSK,   BaseGCS::pcvGaussKrugerTranverseMercator,               /* cs_PRJCOD_GAUSSK   */ },
  { COORDSYS_KRVKP,     CS_KRVKP,   BaseGCS::pcvCzechKrovak,                                /* cs_PRJCOD_KROVAK   */ },
  { COORDSYS_KRVKR,     CS_KRVKR,   BaseGCS::pcvCzechKrovakObsolete,                        /* cs_PRJCOD_KROVK1   */ },     // There is something wrong with this one (Krovak rounded origin) - can't really find it in CS_Map.
  { COORDSYS_MRCSR,     CS_MRCSR,   BaseGCS::pcvMercatorScaleReduction,                     /* cs_PRJCOD_MRCATK   */ },
  { COORDSYS_OCCNC,     CS_OCCNC,   BaseGCS::pcvObliqueConformalConic,                      /* cs_PRJCOD_OCCNC    */ },     // oblique conformal conic. Doesn't look like it was ever really supported.
  { COORDSYS_KRVKG,     CS_KRVKG,   BaseGCS::pcvCzechKrovakObsolete,                        /* cs_PRJCOD_KROVAK   */ },     // There is something wrong with this one (Krovak generalized) - can't really find it in CS_Map
  { COORDSYS_TRMAF,     CS_TRMAF,   BaseGCS::pcvTransverseMercatorAffinePostProcess,        /* cs_PRJCOD_TRMERAF  */ },
  { COORDSYS_PSTSL,     CS_PSTSL,   BaseGCS::pcvPolarStereographicStandardLatitude,         /* cs_PRJCOD_PSTROSL  */ },
  { COORDSYS_NERTH,     CS_NERTH,   BaseGCS::pcvNonEarth,                                   /* cs_PRJCOD_NERTH    */ },
  { COORDSYS_SPCSL,     0,          BaseGCS::pcvInvalid,                                    /* 0                  */ },     // state plane - should never be projType, requires special treatment

  { COORDSYS_HUEOV,     CS_HUEOV,   BaseGCS::pcvObliqueCylindricalHungary,                  /* cs_PRJCOD_OBQCYL   */ },
  { COORDSYS_SYS34,     CS_SYS34,   BaseGCS::pcvTransverseMercatorDenmarkSys34,             /* cs_PRJCOD_SYS34    */ },
  { COORDSYS_OST97,     CS_OST97,   BaseGCS::pcvTransverseMercatorOstn97,                   /* cs_PRJCOD_OSTN97   */ },
  { COORDSYS_OST02,     CS_OST02,   BaseGCS::pcvTransverseMercatorOstn02,                   /* cs_PRJCOD_OSTN02   */ },
  { COORDSYS_S3499,     CS_S3499,   BaseGCS::pcvTransverseMercatorDenmarkSys3499,           /* cs_PRJCOD_SYS34_99 */ },
  { COORDSYS_AZEDE,     CS_AZEDE,   BaseGCS::pcvAzimuthalEquidistantElevatedEllipsoid,      /* cs_PRJCOD_AZEDE    */ },
  { COORDSYS_KEYNM,     0,          BaseGCS::pcvInvalid,                                    /* 0                  */ },     // key name - doesn't appear in type 66.
  { COORDSYS_LMMIN,     CS_LMMIN,   BaseGCS::pcvLambertConformalConicMinnesota,             /* cs_PRJCOD_MNDOTL   */ },
  { COORDSYS_LMWIS,     CS_LMWIS,   BaseGCS::pcvLambertConformalConicWisconsin,             /* cs_PRJCOD_WCCSL    */ },
  { COORDSYS_TMMIN,     CS_TMMIN,   BaseGCS::pcvTransverseMercatorMinnesota,                /* cs_PRJCOD_MNDOTT   */ },
  { COORDSYS_TMWIS,     CS_TMWIS,   BaseGCS::pcvTransverseMercatorWisconsin,                /* cs_PRJCOD_WCCST    */ },
  { COORDSYS_RSKWO,     CS_RSKWO,   BaseGCS::pcvRectifiedSkewOrthomorphicOrigin,            /* cs_PRJCOD_RSKEWO   */ },
  { COORDSYS_WINKT,     CS_WINKT,   BaseGCS::pcvWinkelTripel,                               /* cs_PRJCOD_WINKL    */ },
  { COORDSYS_TMKRG,     CS_TMKRG,   BaseGCS::pcvTransverseMercatorKruger,                   /* cs_PRJCOD_TRMRKRG  */ },
  { COORDSYS_NESRT,     CS_NESRT,   BaseGCS::pcvNonEarthScaleRotation,                      /* cs_PRJCOD_NRTHSRT  */ },
  { COORDSYS_LMBRTAF,   CS_LMBRTAF, BaseGCS::pcvLambertConformalConicAffinePostProcess,     /* cs_PRJCOD_LMBRTAF  */ },
  { COORDSYS_UTMZNBF,   CS_UTMZNBF, BaseGCS::pcvTotalUniversalTransverseMercator,           /* cs_PRJCOD_UTMZNBF  */ },     // Total Transverse Mercator UTM Zone using Bernard Flaceliere calculation
  { COORDSYS_TRMERBF,   CS_TRMERBF, BaseGCS::pcvTotalTransverseMercatorBF,                  /* cs_PRJCOD_TRMERBF  */ },     // Total Transverse Mercator using Bernard Flaceliere calculation
  { COORDSYS_FAVOR,     0,          BaseGCS::pcvInvalid,                                    /* 0                  */ },
  { COORDSYS_S3401,     CS_S3401,   BaseGCS::pcvTransverseMercatorDenmarkSys3401,           /* cs_PRJCOD_SYS34_01 */ },
  { COORDSYS_EDCYLE,    CS_EDCYLE,  BaseGCS::pcvEquidistantCylindricalEllipsoid,            /* cs_PRJCOD_EDCYLE   */ },
  { COORDSYS_PCARREE,   CS_PCARREE, BaseGCS::pcvPlateCarree,                                /* cs_PRJCOD_PCARREE  */ },
  { COORDSYS_MRCATPV,   CS_MRCATPV, BaseGCS::pcvPopularVisualizationPseudoMercator,         /* cs_PRJCOD_MRCATPV  */ },
};

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            doubleSame
(
double  val1,
double  val2
)
    {
    double  denom = fabs (val1) + fabs (val2);

    if (denom == 0)
        return true;

    double  scaledDiff = fabs ((val1 - val2)/denom);

    return (scaledDiff < 1e-11);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu St-Pierre   04/08
+---------------+---------------+---------------+---------------+---------------+------*/
void             stripWhite(char *str)
    {
    char *p;

    p = str;
    do
    if (!isspace(*p=*str))p++;
    while (*str++);
    }


/*=================================================================================**//**
*
* SRS WKT Parser class: Can be used as an alternate WKT parser to CSMAP.
*
* NOTE: Error processing is still minimal. The functions will return the generic error ERROR
* most if not all of the times. Error processing will be completed later on in the
* development process
*
+===============+===============+===============+===============+===============+======*/
class SRSWKTParser
{
public:

    enum class AxisDirection
        {
        NORTH,
        SOUTH,
        EAST,
        WEST,
        UP,
        DOWN,
        OTHER,
        UNDEFINED
        };

/*---------------------------------------------------------------------------------**//**
*   @bsimethod                                                  Alain Robert 2004/03
+---------------+---------------+---------------+---------------+---------------+------*/
SRSWKTParser()
    {
    }
/*---------------------------------------------------------------------------------**//**
*   @bsimethod                                                  Alain Robert 2004/03
+---------------+---------------+---------------+---------------+---------------+------*/
virtual ~SRSWKTParser()
    {
    }

/*---------------------------------------------------------------------------------**//**
*   @bsimethod                                                  Alain Robert 2004/08
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Process (BaseGCSR baseGCS, WCharCP wktChar)
    {
    WString wkt(wktChar);
    StatusInt status = SUCCESS;

    // First thing we do is allocate the m_csParameter structure of the GCS (using a known definition)
    baseGCS.SetFromCSName (L"LL84");
    baseGCS.SetName (L"");
    baseGCS.SetDescription (L"");
    baseGCS.SetSource(L"");
    // Wipe the group name for future definition complete
    if (baseGCS.GetCSParameters() != NULL)
        {
        baseGCS.GetCSParameters()->csdef.group[0] = '\0';
        // Wipe min / max so they can be computed
        baseGCS.GetCSParameters()->csdef.ll_min[LNG] = 0.0;
        baseGCS.GetCSParameters()->csdef.ll_min[LAT] = 0.0;
        baseGCS.GetCSParameters()->csdef.ll_max[LNG] = 0.0;
        baseGCS.GetCSParameters()->csdef.ll_max[LAT] = 0.0;
        baseGCS.GetCSParameters()->csdef.xy_min[XX] = 0.0;
        baseGCS.GetCSParameters()->csdef.xy_min[YY] = 0.0;
        baseGCS.GetCSParameters()->csdef.xy_max[XX] = 0.0;
        baseGCS.GetCSParameters()->csdef.xy_max[YY] = 0.0;
        }

    if ((wkt.length() >= 6) && (wkt.substr (0, 6) == (L"PROJCS")))
        status = GetProjected (baseGCS, wkt);
    else if ((wkt.length() >= 6) && (wkt.substr (0, 6) == (L"GEOGCS")))
        status = GetGeographic (baseGCS, wkt);
    else if ((wkt.length() >= 8) && (wkt.substr (0, 8) == (L"LOCAL_CS")))
        status = GetLocal (baseGCS, wkt);
    else if ((wkt.length() >= 8) && (wkt.substr (0, 8) == (L"COMPD_CS")))
        status = GetCompound (baseGCS, wkt);
    else
        status = ERROR;

    return status;
    }


private:


/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts a projection coordinate reference
*   system from provided WKT stream.
*
*   @param baseGCS OUT The BaseGCS to fill definition of
*
*   @param wkt IN The WKT stream to obtain projected CRS from.
*
*   @return SUCCESS or error value
*
*   @bsimethod                                                  Alain Robert 2004/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetProjected (BaseGCSR baseGCS, WStringR wkt) const
    {
    double conversionToDegree = 1.0;
    bool geocsPresent = false;

    // Init units to meter (to be used as default for some WKTs)
    GeoCoordinates::UnitEnumerator* unitEnumerator = new GeoCoordinates::UnitEnumerator();
    GeoCoordinates::UnitCP currentUnit;
    int currentUnitCode = 0;
    int foundUnitCode = -1;
    double unitFactor = 1.0;

    while ((foundUnitCode < 0) && (unitEnumerator->MoveNext()))
        {
        currentUnit = unitEnumerator->GetCurrent();

        if (currentUnit->GetBase() == GeoUnitBase::Meter)
            {
            double dictConversionFactor = currentUnit->GetConversionFactor();
            if ((unitFactor < dictConversionFactor + 0.00000001) && (unitFactor > dictConversionFactor - 0.00000001))
                foundUnitCode = currentUnitCode;
            }
        currentUnitCode++;
        }
    if (foundUnitCode < 0)
        return ERROR;

    baseGCS.SetUnitCode (foundUnitCode);


    wkt.Trim();

    // Validate that this is the proper section (must start with PROJCS)
    if ((wkt.length() < 6) || (!(wkt.substr (0, 6) == L"PROJCS")))
        {
        return ERROR;
        }

    // Remove keyword
    wkt = wkt.substr (6);

    // Trim again
    wkt.Trim();

    // Make sure that remainder starts with [
    if ((wkt.length() < 1) || (!(wkt.substr (0, 1) == L"[")))
        {
        return ERROR;
        }

    wkt = wkt.substr (1);

    // The first member is the name
    WString name = GetName (wkt);
    WString authorityID = GetAuthorityIdFromNameOracleStyle(name);
    bool sectionCompleted = false;
    size_t previousLength;
    while (wkt.length() > 0 && !sectionCompleted)
        {
        previousLength = wkt.length();

        // Trim of whites
        wkt.Trim();

        // Trim commas
        if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
            {
            wkt = wkt.substr(1);
            }

        if ((wkt.length() >= 9) && (wkt.substr (0, 9) == (L"AUTHORITY")))
            authorityID = GetAuthority (wkt);

        if ((wkt.length() >= 6) && (wkt.substr (0, 6) == (L"GEOGCS")))
            {
            geocsPresent = true;
            if (SUCCESS != GetGeographicToProjected (wkt, &conversionToDegree, baseGCS))
                return ERROR;
            }

        if ((wkt.length() >= 10) && (wkt.substr (0, 10) == (L"PROJECTION")))
            if (SUCCESS != GetProjectionToCoordSys (wkt, conversionToDegree, baseGCS))
                return ERROR;

        if ((wkt.length() >= 4) && (wkt.substr (0, 4) == (L"UNIT")))
            if (SUCCESS != GetLinearUnitToCoordSys (wkt, baseGCS))
                return ERROR;

        if ((wkt.length() >= 9) && (wkt.substr (0, 9) == (L"EXTENSION"))) // PROJ4 Addition (Sigh!)
            {
            WString     extensionName;
            WString     extensionText;
            if (SUCCESS != GetExtension (wkt, extensionName, extensionText))
                return ERROR;
            }

        // Optional
        if ((wkt.length() >= 4) && (wkt.substr (0, 4) == (L"AXIS")))
            {
            // For a projcs clause two axises must be specified one after the other
            // We cannot at this time process all the configuration of the AXIS clause ... the parser fails
            // unless the order is exactly EAST then NORTH whatever the label used.
            if (GetAxis(wkt) != AxisDirection::EAST)
                return ERROR;

            // Trim of whites
            wkt.Trim();

            // Trim commas
            if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
                {
                wkt = wkt.substr(1);
                }
            // Trim of whites
            wkt.Trim();

            // The second AXIS Clause is required accordinag to specs.
            if ((wkt.length() < 4) || (wkt.substr (0, 4) != (L"AXIS")))
                return ERROR;

            if (GetAxis(wkt) != AxisDirection::NORTH)
                return ERROR;
            }

        if ((wkt.length() >= 1) && (wkt.substr (0, 1) == (L"]")))
            {
            wkt = wkt.substr (1);
            sectionCompleted = true;
            }

        if (wkt.length() == previousLength)
            {
            return ERROR;
            }
        }

#ifdef NOT_YET
    if (authorityID.length() > 0)
        {
//TBD Search for an existing equivalent
//compare if required
//        baseGCS.SetKey (authorityID->GetKey());
        }
#endif

    // Some WKT do not have GEOGCS Clauses which we do not have any default
    if (!geocsPresent)
        return ERROR;

    baseGCS.SetName (name.c_str());
    baseGCS.SetDescription (name.c_str());
    baseGCS.SetSource(L"WKT");

    return baseGCS.DefinitionComplete();
    }

/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts a projection coordinate reference
*   system from provided WKT stream.
*
*   @param baseGCS OUT The BaseGCS to fill definition of
*
*   @param wkt IN The WKT stream to obtain projected CRS from.
*
*   @return SUCCESS or error value
*
*   @bsimethod                                                  Alain Robert 2015/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetCompound (BaseGCSR baseGCS, WStringR wkt) const
    {
    StatusInt status = SUCCESS;

    wkt.Trim();

    // Validate that this is the proper section (must start with PROJCS)
    if ((wkt.length() < 8) || (!(wkt.substr (0, 8) == L"COMPD_CS")))
        {
        return ERROR;
        }

    // Remove keyword
    wkt = wkt.substr (8);

    // Trim again
    wkt.Trim();

    // Make sure that remainder starts with [
    if ((wkt.length() < 1) || (!(wkt.substr (0, 1) == L"[")))
        {
        return ERROR;
        }

    wkt = wkt.substr (1);

    wkt.Trim();
    WString name = GetName (wkt);

    // Trim of whites
    wkt.Trim();

    // Trim commas
    if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
         {
         wkt = wkt.substr(1);
         }

    // The first member must be either a PROJCS or a GEOGCS
    wkt.Trim();
    if ((wkt.length() >= 6) && (wkt.substr (0, 6) == (L"PROJCS")))
        status = GetProjected (baseGCS, wkt);
    else if ((wkt.length() >= 6) && (wkt.substr (0, 6) == (L"GEOGCS")))
        status = GetGeographic (baseGCS, wkt);
    else
        status = ERROR;

    if (ERROR != status)
        {
        // Now we should have a valid BaseGCS properly filled with the projected or geographic
        // coordinate system. Since we are dealing with a compound CS there is a second section
        // We only support the VERT_CS as second coordinate system.
        wkt.Trim();

        // Trim commas
        if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
            wkt = wkt.substr(1);
        wkt.Trim();

        if ((wkt.length() >= 7) && (wkt.substr (0, 7) == (L"VERT_CS")))
            status = SetVerticalCS (baseGCS, wkt);
        else
            status = ERROR;
        }
    
    // Complete BaseGCS
    if (ERROR != status)
        {
        status =  baseGCS.DefinitionComplete();
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts a vertical datum
*   from provided WKT stream.
*
*   @param wkt IN The WKT stream to obtain vertical datum from. The WKT should start with the
*                 VERT_DATUM clause and contain the whole definition. Additional characters
*                 after the end of the clause are ignored and returned in the wkt
*                 stripped out of the whole VERT_DATUM clause.
*
*   @return the VertDatumCode or vdcDatumNone if datum could not be determined.
*
*   @bsimethod                                                  Alain Robert 2015/07
+---------------+---------------+---------------+---------------+---------------+------*/
VertDatumCode GetVerticalDatum (WStringR wkt) const
    {
    if ((wkt.length() < 10) || (!(wkt.substr (0, 10) == L"VERT_DATUM")))
        return vdcFromDatum;

    // Remove keyword
    wkt = wkt.substr (10);

    // Trim again
    wkt.Trim();

    // Make sure that remainder starts with [
    if ((wkt.length() < 1) || (!(wkt.substr (0, 1) == L"[")))
        {
        return vdcFromDatum;
        }

    wkt = wkt.substr (1);

    WString name = GetName (wkt);
    WString authorityID;
    
    // The name should be immediately followed by a number indicating the vertical datum type.

    // Trim whites and comma
    wkt.Trim();
    if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
        wkt = wkt.substr(1);
    wkt.Trim();

    int WKTDatumCode = GetInteger(wkt);

    bool sectionCompleted = false;
    size_t previousLength;
    while (wkt.length() > 0 && !sectionCompleted)
        {
        previousLength = wkt.length();

        // Trim of whites
        wkt.Trim();

        // Trim commas
        if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
            {
            wkt = wkt.substr(1);
            }

        if ((wkt.length() >= 9) && (wkt.substr (0, 9) == (L"AUTHORITY")))
            authorityID = GetAuthority (wkt);

        if ((wkt.length() >= 9) && (wkt.substr (0, 9) == (L"EXTENSION"))) // PROJ4 addition
            {
            WString     extensionName;
            WString     extensionText;

        // We do not check for an error ... the EXTENSION clause is ill-formed then
            // likely the vertical datum will be invalid
            GetExtension (wkt, extensionName, extensionText);
            }

        if ((wkt.length() >= 1) && (wkt.substr (0, 1) == (L"]")))
            {
            wkt = wkt.substr (1);
            sectionCompleted = true;
            }

        if (wkt.length() == previousLength)
            {
            return vdcFromDatum;
            }
        }

    
    // We map the WKT datum code to the GeoCoord datum code ... this process is still incomplete as
    // we support little vertical datums
    VertDatumCode vertDatum = vdcFromDatum;

    // We do not support 2003 (Barometric altitude, and 2006 (Depth) but we set vertical datum to ellipsoidal height
    // We consider 2002 (ellipsoidal), 2004 (Normal) and 2000 (Other) as ellipsoidal height which is the default. 
    if (2005 == WKTDatumCode || 2001 == WKTDatumCode)
        {
        // This is a geoid based datum (we consider orthometric datum (2001) the same as Geoid)
        // Technically there are various geoid datums but with csmap we are stuck with the 
        // fact. We first rely on the authority code
        vertDatum = vdcGeoid; 

        if (authorityID.length() != 0)
            {
            if (authorityID == L"EPSG:5102")
                vertDatum = vdcNGVD29;
            else if (authorityID == L"EPSG:5103")
                vertDatum = vdcNAVD88;
            }
            
        }

    return vertDatum;
    }

/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts a vertical coordinate reference
*   system from provided WKT stream.
*
*   @param baseGCS IN/OUT The BaseGCS to fill definition of vertical CS. The remainder
*                         of the BaseGCS is left untouched and should already contain
*                         the non-vertical portion of the GCS since some vertical
*                         coordinate systems have limitations related to the nature of the
*                         datum used by the GCS.
*
*   @param wkt IN The WKT stream to obtain vertical cs from. The WKT should start with the
*                 VERT_CS clause and contain the whole definition. Additional characters
*                 after the end of the clause are ignored and returned in the wkt
*                 stripped out of the whole VERT_CS clause.
*
*   @return SUCCESS or error value
*
*   @bsimethod                                                  Alain Robert 2015/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SetVerticalCS (BaseGCSR baseGCS, WStringR wkt) const
    {
    if ((wkt.length() < 7) || (!(wkt.substr (0, 7) == L"VERT_CS")))
        return ERROR;

    // Remove keyword
    wkt = wkt.substr (7);

    // Trim again
    wkt.Trim();

    // Make sure that remainder starts with [
    if ((wkt.length() < 1) || (!(wkt.substr (0, 1) == L"[")))
        {
        return ERROR;
        }

    wkt = wkt.substr (1);

    WString name = GetName (wkt);

    VertDatumCode vertDatum = vdcFromDatum;
    WString authorityID;

    bool sectionCompleted = false;
    size_t previousLength;
    while (wkt.length() > 0 && !sectionCompleted)
        {
        previousLength = wkt.length();

        // Trim whites and comma
        wkt.Trim();
        if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
            wkt = wkt.substr(1);

        if ((wkt.length() >= 9) && (wkt.substr (0, 9) == (L"AUTHORITY")))
            authorityID = GetAuthority (wkt);

        if ((wkt.length() >= 10) && (wkt.substr (0, 10) == (L"VERT_DATUM")))
            vertDatum = GetVerticalDatum (wkt);

        //We only make sure the AXIS clause contains UP (We do not support anything else)
        if ((wkt.length() >= 4) && (wkt.substr (0, 4) == (L"AXIS")))
            if (GetAxis(wkt) != AxisDirection::UP)
                return ERROR;

        // We do not care about the content of the UNIT clause
        if ((wkt.length() >= 4) && (wkt.substr (0, 4) == (L"UNIT")))
            {
            double      unitFactor;
            WString     unitName;
            StatusInt   status;
            if (SUCCESS != (status = GetUnit (wkt, unitName, &unitFactor)))
                return status;
            }

        if ((wkt.length() >= 1) && (wkt.substr (0, 1) == (L"]")))
            {
            wkt = wkt.substr (1);
            sectionCompleted = true;
            }

        if (wkt.length() == previousLength)
            {
            return ERROR;
            }
        }

    // If the datum has not been resolved ... we try to do it. This should not happen normally
    // but we make sure in case the VERT_DATUM clause was badly formed or the vertical datum
    // was geoid based but had not authority ID
    if (vdcFromDatum == vertDatum || vdcGeoid == vertDatum)
        {
        if (authorityID == L"EPSG:5702")
            vertDatum = vdcNGVD29;
        else if (authorityID == L"EPSG:5703")
            vertDatum = vdcNAVD88;
        else if (authorityID == L"EPSG:5773")
            vertDatum = vdcGeoid;
        }

    // NOTE: We only rely on the authority ID because the name is unthrustworty but if some
    // standard emerges we will be happy to check the vertical cs names to resolve.

    baseGCS.SetVerticalDatumCode (vertDatum);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts a geographic coordinate reference
*   system from provided WKT stream.
*
*   @param baseGCS OUT The BaseGCS to fill definition of
*
*   @param wkt IN The WKT stream to obtain projected CRS from.
*
*   @return SUCCESS or error value
*
*   @bsimethod                                                  Alain Robert 2004/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetGeographic (BaseGCSR baseGCS, WStringR wkt) const
    {
    if ((wkt.length() < 6) || (!(wkt.substr (0, 6) == L"GEOGCS")))
        return ERROR;

//    baseGCS->SetProjectionCode (pcvUnity);

    StatusInt status;
    WString geographicName;
    WString geographicAuthorityID;
    double conversionToDegree = 1.0;

    if (SUCCESS == (status = GetGeographicToCoordSys (wkt, geographicName, geographicAuthorityID, &conversionToDegree, baseGCS)))
        {

#ifdef NOT_YET
        if (geographicAuthorityID.length() > 0)
            {
// TBD Search for existing entry in dictionary and compare...
//        baseGCS.SetKey (authorityID->GetKey());
            }
#endif

        baseGCS.SetName (geographicName.c_str());
        baseGCS.SetDescription (geographicName.c_str());
        baseGCS.SetSource(L"WKT");

        return baseGCS.DefinitionComplete();
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts a local coordinate reference
*   system from provided WKT stream.
*
*   @param baseGCS OUT The BaseGCS to fill definition of
*
*   @param wkt IN The WKT stream to obtain projected CRS from.
*
*   @return SUCCESS or error value
*
*   @bsimethod                                                  Alain Robert 2004/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetLocal (BaseGCSR baseGCS, WStringR wkt) const
    {
    StatusInt status = SUCCESS;

    wkt.Trim();

    // Validate that this is the proper section (must start with LOCAL_CS)
    if ((wkt.length() < 8) || (!(wkt.substr (0, 8) == L"LOCAL_CS")))
        return ERROR;

    // Remove keyword
    wkt = wkt.substr (8);

    // Trim again
    wkt.Trim();

    // Make sure that remainder starts with [
    if ((wkt.length() < 1) || (!(wkt.substr (0, 1) == L"[")))
        return ERROR;

    wkt = wkt.substr (1);

    // The first member is the name
    WString name = GetName (wkt);
    WString authorityID = GetAuthorityIdFromNameOracleStyle(name);


    baseGCS.SetProjectionCode (BaseGCS::pcvNonEarth);
    baseGCS.SetDatumCode (-1);

    bool sectionCompleted = false;
    size_t previousLength;
    while (wkt.length() > 0 && !sectionCompleted)
        {
        previousLength = wkt.length();

        // Trim of whites
        wkt.Trim();

        // Trim commas
        if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
            {
            wkt = wkt.substr(1);
            }

        if ((wkt.length() >= 11) && (wkt.substr (0, 11) == (L"LOCAL_DATUM")))
            if (SUCCESS != (status = GetLocalDatumToCoordSys (wkt, baseGCS)))
                return status;

        if ((wkt.length() >= 4) && (wkt.substr (0, 4) == (L"UNIT")))
            if (SUCCESS != (status = GetLinearUnitToCoordSys (wkt, baseGCS)))
                return status;

        // Optional component
        if ((wkt.length() >= 4) && (wkt.substr (0, 4) == (L"AXIS")))
            {
            // We currently do not support the AXIS clause ... we fail
            return ERROR;
            }

        if ((wkt.length() >= 9) && (wkt.substr (0, 9) == (L"AUTHORITY")))
            authorityID = GetAuthority (wkt);

        if ((wkt.length() >= 1) && (wkt.substr (0, 1) == (L"]")))
            {
            wkt = wkt.substr (1);
            sectionCompleted = true;
            }

        if (wkt.length() == previousLength)
            return ERROR;
        }

#ifdef NOT_YET

    if (authorityID.length() > 0)
        {
// TBD Search for entry in dictionary
//        baseGCS.SetKey (authorityID->GetKey());
        }
#endif

    baseGCS.SetName (name.c_str());
    baseGCS.SetDescription (name.c_str());
    baseGCS.SetSource(L"WKT");

    return baseGCS.DefinitionComplete();
    }


/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts the datum, spheroid and meridian
*   definition and sets the appropriate fields in the given coordinate system.
*
*   @param wkt IN The WKT containing the geographic coordinate reference system to extract.
*
*   @param conversionToDegree OUT Receives the angular unit definition factor for interpretation
*   of angular parameters.
*
*   @param coordinateSystem IN|OUT The coordinate system to set datum, spheroid and prime
*           meridian of.
*
*   @bsimethod                                                  Alain Robert 2004/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetGeographicToProjected (WStringR wkt, double* conversionToDegree, BaseGCSR coordinateSystem) const
    {
    WString geographicName;
    WString geographicAuthorityID;

    // We do not care about name and ID
    return GetGeographicToCoordSys (wkt, geographicName, geographicAuthorityID, conversionToDegree, coordinateSystem);
    }

/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts the datum, spheroid and meridian
*   definition and sets the appropriate fields in the given coordinate system.
*   The GEOGCS AuthorityID and name are returned in separate field for the caller
*   to use as sees fit.
*
*   @param wkt IN The WKT containing the geographic coordinate reference system to extract.
*
*   @param geographicName OUT Reference to a string that will receive the name of the GEOGCS.
*
*   @param geographicAuthorityID OUT Reference to a string that will receive the Authority ID
*           if present
*
*   @param conversionToDegree OUT Receives the angular unit definition factor for interpretation
*   of angular parameters.
*
*   @param coordinateSystem IN|OUT The coordinate system to set datum, spheroid and prime
*           meridian of.
*
*   @return SUCCESS if operation sucessful or another value otherwise.
*
*   @bsimethod                                                  Alain Robert 2004/04
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetGeographicToCoordSys (WStringR wkt, WStringR geographicName, WStringR geographicAuthorityID, double* conversionToDegree, BaseGCSR coordinateSystem) const
    {
    StatusInt status = SUCCESS;

    wkt.Trim();

    // Validate that this is the proper section (must start with GEOCS)
    if ((wkt.length() < 6) || (!(wkt.substr (0, 6) == L"GEOGCS")))
        return ERROR;

    // Remove keyword
    wkt = wkt.substr (6);

    // Trim again
    wkt.Trim();

    // Make sure that remainder starts with [
    if ((wkt.length() < 1) || (!(wkt.substr (0, 1) == L"[")))
        return ERROR;

    // Trim [ and ]
    wkt = wkt.substr (1);

    // The first member is the name
    geographicName = GetName (wkt);
    bool sectionCompleted = false;

    coordinateSystem.SetProjectionCode (BaseGCS::pcvUnity);

    size_t previousLength;
    while (wkt.length() > 0 && !sectionCompleted)
        {
        previousLength = wkt.length();
        // Trim of whites
        wkt.Trim();

        // Trim commas
        if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
            {
            wkt = wkt.substr(1);
            }

        if ((wkt.length() >= 9) && (wkt.substr (0, 9) == (L"AUTHORITY")))
            geographicAuthorityID = GetAuthority (wkt);

        if ((wkt.length() >= 5) && (wkt.substr (0, 5) == (L"DATUM")))
            if (SUCCESS != (status = GetHorizontalDatumToCoordSys (wkt, coordinateSystem)))
                return status;

        if ((wkt.length() >= 6) && (wkt.substr (0, 6) == (L"PRIMEM")))
            if (SUCCESS != (status = GetPrimeMeridianToCoordSys (wkt, coordinateSystem)))
                return status;

        if ((wkt.length() >= 4) && (wkt.substr (0, 4) == (L"UNIT")))
            if (SUCCESS != (status = GetAngleUnit (wkt, conversionToDegree)))
                return status;

        // Optional AXIS
        if ((wkt.length() >= 4) && (wkt.substr (0, 4) == (L"AXIS")))
            {
            // The axis clause is not currently supported ... we fail
            return ERROR;
            }

        if ((wkt.length() >= 1) && (wkt.substr (0, 1) == (L"]")))
            {
            wkt = wkt.substr (1);
            sectionCompleted = true;
            }

        if (wkt.length() == previousLength)
            {
            return ERROR;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool    WKTDatumLookup (WCharCP name, WStringR alternateName) const
    {
    char        mbAlternateName[100];
    AString     mbName (name);
    if (CS_wktDatumLookUp (mbName.c_str(), mbAlternateName))
        {
        alternateName.AssignA (mbAlternateName);
        return true;
        }
    else
        {
        alternateName.clear();
        return false;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/12
+---------------+---------------+---------------+---------------+---------------+------*/
int     FindDatumIndex (WCharCP datumName) const
    {
    AString mbDatumName (datumName);
    int     index;
    int     foundIndex = -1;
    char    dtKeyName[128];
    for (index = 0; ((foundIndex < 0) && (0 < CSMap::CS_dtEnum (index, dtKeyName, sizeof(dtKeyName)))); index++)
        {
        if (0 == BeStringUtilities::Stricmp (mbDatumName.c_str(), dtKeyName))
            foundIndex = index;
        }

    return foundIndex;
    }

/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts from the provided stream the datum
*   and sets it in the given coordinate system.
*   The complete WKT datum section must be provided including the DATUM[ ] keyword.
*   The wkt text stream may contain additional text that is returned with the DATUM section
*   removed.
*
*   IMPORTANT NOTE: At the moment we only support datums for which the definition
*   is already known in the dictionary. Although we do have the ability to parse
*   custom datum definition parameters (TOWGS84) or Oracle strange datum transformation
*   parameter format, custom datum will fail and result in an error.
*
*   @param wkt IN/OUT The WKT portion that contains the DATUM to extract.
*
*   @param coordinateSystem IN|OUT The coordinate system that gets filled with projection
*       code and parameter values.
*
*   @return SUCCESS if successful or another value otherwise.
*
*   @bsimethod                                                  Alain Robert 2004/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetHorizontalDatumToCoordSys (WStringR wkt, BaseGCSR coordinateSystem) const
    {
    StatusInt status = SUCCESS;
    wkt.Trim();

    // Validate that this is the proper section (must start with ")
    if ((wkt.length() < 5) || (!(wkt.substr (0, 5) == L"DATUM")))
        return ERROR;

    // Remove keyword
    wkt = wkt.substr (5);

    // Trim again
    wkt.Trim();

    // Make sure that remainder starts with [
    if ((wkt.length() < 1) || (!(wkt.substr (0, 1) == L"[")))
        return ERROR;

    wkt = wkt.substr (1);

    // The first member is the name
    WString name = GetName (wkt);
    WString authorityID = GetAuthorityIdFromNameOracleStyle(name);
    bool sectionCompleted = false;

    size_t previousLength;
    while (wkt.length() > 0 && !sectionCompleted)
        {
        previousLength = wkt.length();
        // Trim of whites
        wkt.Trim();

        // Trim commas
        if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
            {
            wkt = wkt.substr(1);
            }

        if ((wkt.length() >= 9) && (wkt.substr (0, 9) == (L"AUTHORITY")))
            authorityID = GetAuthority (wkt);

        if ((wkt.length() >= 8) && (wkt.substr (0, 8) == (L"SPHEROID")))
            if (SUCCESS != (status = GetEllipsoidToCoordSys (wkt, coordinateSystem)))
                return status;

        if ((wkt.length() >= 7) && (wkt.substr (0, 7) == (L"TOWGS84")))
            if (SUCCESS != (status = GetTOWGS84ToCoordSys (wkt, coordinateSystem)))
                return status;

        // Check end of section
        if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L"]")))
            {
            wkt = wkt.substr(1);
            sectionCompleted = true;
            }

        if (wkt.length() == previousLength)
            {
            // We have the special Oracle dialect where the 7 parameters transformation is provided
            // without TOWGS84 section.
            Get7ParamsDatumTransformationToCoordSys (wkt, coordinateSystem);

            wkt.Trim();

            // In this case the section end is mandatory
            if ((wkt.length() < 1) || (!(wkt.substr(0, 1) == L"]")))
                {
                return ERROR;
                }

            wkt = wkt.substr(1);
            sectionCompleted = true;
            }

        }


    // Check if datum name is known ...
    WString finalDatumName;
    DatumCP namedDatum = Datum::CreateDatum (name.c_str());
    if (namedDatum == NULL || ! namedDatum->IsValid())
        {
        // Datum not found ... try with authority ID if defined
        if (authorityID.length() > 0)
            {
            namedDatum = Datum::CreateDatum (authorityID.c_str());
            if (namedDatum != NULL && namedDatum->IsValid())
                finalDatumName = authorityID;
            }
        }
    else
        finalDatumName = name;

#if defined (GEOCOORD_ENHANCEMENT)

    if (finalDatumName.length() == 0)
        {
        // We do not have the datum name yet ... try looking it up in the alias table.
        WString     alternateName;

        if (WKTDatumLookup (name.c_str(), alternateName))
            {
            namedDatum = Datum::CreateDatum (alternateName.c_str());
            if (namedDatum != NULL && namedDatum->IsValid())
                finalDatumName = alternateName;
            }

        // Either there is no alias or the alias is not good ... if authority ID is present ...
        if ((finalDatumName.length() == 0) && (authorityID.length() != 0))
            {
            if (WKTDatumLookup (authorityID.c_str(), alternateName))
                {
                // Alternate name should be valid
                namedDatum = Datum::CreateDatum (authorityID.c_str());
                if (namedDatum != NULL && namedDatum->IsValid())
                    finalDatumName = authorityID;
                }
            }
        }
#endif

    if (finalDatumName.length() != 0)
        {
        // We have a datum name ... we simply need to set it now ... in order to do this we need the code
        int foundIndex = FindDatumIndex (finalDatumName.c_str());

        if (foundIndex >= 0)
            coordinateSystem.SetDatumCode (foundIndex);
        else
            return ERROR;

        }
    else
        return ERROR;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts from the provided stream the ellipsoid
*   and sets it in the provided coordinate system.
*   The complete WKT ellipsoid section must be provided including the SPHEROID[ ] keyword.
*   The wkt text stream may contain additional text that is returned with the SPHEROID section
*   removed.
*
*   @param wkt IN/OUT The WKT portion that contains the ellipsoid to extract.
*
*   @return The ellipsoid
*
*   @bsimethod                                                  Alain Robert 2004/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetEllipsoidToCoordSys (WStringR wkt, BaseGCSR coordinateSystem) const
    {
    StatusInt status = SUCCESS;
    wkt.Trim();

    // Validate that this is the proper section (must start with ")
    if ((wkt.length() < 8) || (!(wkt.substr (0, 8) == L"SPHEROID")))
        return ERROR;

    // Remove keyword
    wkt = wkt.substr (8);

    // Trim again
    wkt.Trim();

    // Make sure that remainder starts with [
    if ((wkt.length() < 1) || (!(wkt.substr (0, 1) == L"[")))
        {
        return ERROR;
        }

    wkt = wkt.substr (1);

    // The first member is the name
    WString name = GetName (wkt);
    WString authorityID = GetAuthorityIdFromNameOracleStyle(name);

    wkt.Trim();
    if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
        wkt = wkt.substr(1);
    /*double semiMajorAxis =*/ GetDouble (wkt);
    wkt.Trim();
    if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
        wkt = wkt.substr(1);
    /*double inverseFlattening =*/ GetDouble (wkt);
    wkt.Trim();


    // AUTHORITY MAY BE PRECEDED WITH COMMA
    if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
        {
        wkt = wkt.substr(1);
        wkt.Trim();
        }

    if ((wkt.length() >= 9) && (wkt.substr (0, 9) == (L"AUTHORITY")))
        authorityID = GetAuthority (wkt);


    // Check end of section
    if ((wkt.length() < 1) || (!(wkt.substr(0, 1) == L"]")))
        {
        return ERROR;
        }

    wkt = wkt.substr(1);

    // At the moment since WKT does not currently support ellipsoid based coordinate system and
    // We do not support custom datums we will simply ignore the spheroid definition (although it is parsed out)
    // If we want to allow ellipsoid based GCS originating from unrecognised datums we may do so
    // by setting the ellipsoid definition here.!

    return status;
    }


/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts from the provided stream the prime meridian
*   The complete WKT prime meridian section must be provided including the PRIMEM[ ] keyword.
*   The wkt text stream may contain additional text that is returned with the PRIMEM section
*   removed.
*
*   @param wkt IN/OUT The WKT portion that contains the prime meridian to extract.
*
*   @return The prime meridian
*
*   @bsimethod                                                  Alain Robert 2004/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetPrimeMeridianToCoordSys (WStringR wkt, BaseGCSR coordinateSystem) const
    {
    StatusInt status = SUCCESS;
    wkt.Trim();

    // Validate that this is the proper section (must start with ")
    if ((wkt.length() < 6) || (!(wkt.substr (0, 6) == L"PRIMEM")))
        return ERROR;

    // Remove keyword
    wkt = wkt.substr (6);

    // Trim again
    wkt.Trim();

    // Make sure that remainder starts with [
    if ((wkt.length() < 1) || (!(wkt.substr (0, 1) == L"[")))
        {
        return ERROR;
        }

    wkt = wkt.substr (1);

    WString name = GetName (wkt);
    WString authorityID = GetAuthorityIdFromNameOracleStyle(name);
    wkt.Trim();
    if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
        wkt = wkt.substr(1);
    double longitude = GetDouble (wkt);
    wkt.Trim();

    // AUTHORITY MAY BE PRECEDED WITH COMMA
    if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
        {
        wkt = wkt.substr(1);
        wkt.Trim();
        }

    if ((wkt.length() >= 9) && (wkt.substr (0, 9) == (L"AUTHORITY")))
        authorityID = GetAuthority (wkt);

    wkt.Trim();

    // Check end of section
    if ((wkt.length() < 1) || (!(wkt.substr(0, 1) == L"]")))
        {
        return ERROR;
        }

    wkt = wkt.substr(1);


    // CSMAP only supports prime meridian values other than Greenwish for
    // lat/long GCS all other projections must use Greenwish.

    if (BaseGCS::pcvUnity == coordinateSystem.GetProjectionCode())
        {
        coordinateSystem.SetOriginLongitude (longitude);
        }
    else if ((longitude > (0.00000001)) || (longitude < (-0.00000001))) // Check longitude is zero for any other projections
        return ERROR;

    return status;
    }


/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts from the provided stream the projection
*   The complete WKT projection and related parameters section must be provided including the
*   PROJECTION[ ] and PARAMETER[] keywords.
*   The wkt text stream may contain additional text that is returned with the PROJECTION
*   and PARAMETER sections removed.
*
*   @param wkt IN/OUT The WKT portion that contains the projection to extract.
*
*   @param conversionToDegree IN The conversion factor to degree for angular parameters.
*
*   @param coordinateSystem IN|OUT The coordinate system that gets filled with projection
*       code and parameter values.
*
*   @return SUCCESS if successful or another value in case of error.
*
*   @bsimethod                                                  Alain Robert 2004/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetProjectionToCoordSys (WStringR wkt,double conversionToDegree, BaseGCSR coordinateSystem) const
    {

    StatusInt status = SUCCESS;
    wkt.Trim();

    // Validate that this is the proper section (must start with ")
    if ((wkt.length() < 10) || (!(wkt.substr (0, 10) == L"PROJECTION")))
        return ERROR;

    // Remove keyword
    wkt = wkt.substr (10);

    // Trim again
    wkt.Trim();

    // Make sure that remainder starts with [
    if ((wkt.length() <1) || (!(wkt.substr (0, 1) == L"[")))
        {
        return ERROR;
        }
    wkt = wkt.substr (1);


    // The first member is the name
    WString name = GetName (wkt);
    WString authorityID = GetAuthorityIdFromNameOracleStyle(name);

    BaseGCS::ProjectionCodeValue projectionCode = GetProjectionCodeFromWKTName (name);

    if (BaseGCS::pcvInvalid == projectionCode)
        return ERROR;

    coordinateSystem.SetProjectionCode (projectionCode);

    wkt.Trim();

    if ((wkt.length() >= 9) && (wkt.substr (0, 9) == (L"AUTHORITY")))
        authorityID = GetAuthority (wkt);

    // Check end of section
    if ((wkt.length() < 1) || (!(wkt.substr(0, 1) == L"]")))
        return ERROR;

    wkt = wkt.substr(1);

    wkt.Trim();

    bool sectionCompleted = false;

    size_t previousLength;
    while (wkt.length() > 0 && !sectionCompleted)
        {
        previousLength = wkt.length();
        // Trim of whites
        wkt.Trim();

        // Trim commas
        if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
            {
            wkt = wkt.substr(1);
            }

        if ((wkt.length() >= 9) && (wkt.substr (0, 9) == (L"PARAMETER")))
            {
            double parameterValue;
            WString parameterStringValue;
            WString parameterName;
            if (SUCCESS != (status = GetParameter (wkt, parameterName, &parameterValue, parameterStringValue)))
                return status;

            if (parameterName.length() > 0)
                {
                if (SUCCESS != (status = SetParameterToCoordSys (parameterName, parameterStringValue, parameterValue, conversionToDegree, coordinateSystem)))
                    return status;
                }
            }

        if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L"]")))
            {
            // Although not specs compliant we accept missing UNIT section
            wkt = wkt.substr(1);
            sectionCompleted = true;
            }

        if ((wkt.length() >= 4) && (wkt.substr (0, 4) == (L"UNIT")))
            {
            sectionCompleted = true;
            }

        // Even though the specs call for the linear units following immediately the PARAMETERS
        // sometimes it is not the case and we learn to live with the fact within limits.
        if ((wkt.length() >= 9) && (wkt.substr (0, 9) == (L"AUTHORITY")))
            {
            sectionCompleted = true;
            }

        if ((wkt.length() >= 4) && (wkt.substr (0, 4) == (L"AXIS")))
            {
            sectionCompleted = true;
            }

        if (wkt.length() == previousLength)
            {
            return ERROR;
            }

        }


    return status;
    }


/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts from the provided stream the parameter
*   The complete WKT parameter section must be provided including the PARAMETER[ ] keyword.
*   The wkt text stream may contain additional text that is returned with the PARAMETER section
*   removed.
*
*   @param wkt IN/OUT The WKT portion that contains the parameter to extract.
*
*   @param parameterName OUT reference to a string that receives the parameter name.
*
*   @param parameterValue OUT Pointer to double that receives the floating point value of
*   parameter.
*
*   @param parameterStringValue OUT Reference to a string that receives the string value
*       of the parameter. Usually the parameter value is always numeric but some
*       dialect use strings values to specify Hemisphere or zones.
*
*   @return SUCCESS is successful or another value in case of error.
*
*   @bsimethod                                                  Alain Robert 2004/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetParameter (WStringR wkt, WStringR parameterName, double* parameterValue, WStringR parameterStringValue) const
    {

    wkt.Trim();

    // Validate that this is the proper section (must start with ")
    if ((wkt.length() < 9) || (!(wkt.substr (0, 9) == L"PARAMETER")))
        return ERROR;

    // Remove keyword
    wkt = wkt.substr (9);

    // Trim again
    wkt.Trim();

    // Make sure that remainder starts with [
    if ((wkt.length() < 1) || (!(wkt.substr (0, 1) == L"[")))
        return ERROR;

    wkt = wkt.substr (1);

    parameterName = GetName (wkt);
    wkt.Trim();
    if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
        wkt = wkt.substr(1);
    *parameterValue = GetDoubleAndString (wkt, parameterStringValue);
    wkt.Trim();

    // Check end of section
    if ((wkt.length() < 1) || (!(wkt.substr(0, 1) == L"]")))
        {
        return ERROR;
        }

    wkt = wkt.substr(1);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts from the provided stream the unit
*   The complete WKT unit section must be provided including the UNIT[ ] keyword.
*   The wkt text stream may contain additional text that is returned with the UNIT section
*   removed.
*
*   @param wkt IN/OUT The WKT portion that contains the unit to extract.
*
*   @param unitName OUT Reference to a string that receives the unit name.
*
*   @param unitFactor OUT The unit factor as set in the unit clause.
*
*   @return SUCCESS if succesful or another value otherwise.
*
*   @bsimethod                                                  Alain Robert 2004/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetUnit (WStringR wkt, WStringR unitName, double* unitFactor) const
    {

    wkt.Trim();

    // Validate that this is the proper section (must start with ")
    if ((wkt.length() < 4) || (!(wkt.substr (0, 4) == L"UNIT")))
        return ERROR;

    // Remove keyword
    wkt = wkt.substr (4);

    // Trim again
    wkt.Trim();

    // Make sure that remainder starts with [
    if ((wkt.length() < 1) || (!(wkt.substr (0, 1) == L"[")))
        return ERROR;

    wkt = wkt.substr (1);

    unitName = GetName (wkt);
    WString authorityID = GetAuthorityIdFromNameOracleStyle(unitName);

    wkt.Trim();
    if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
        wkt = wkt.substr(1);
    *unitFactor = GetDouble (wkt);
    wkt.Trim();

    // AUTHORITY MAY BE PRECEDED WITH COMMA
    if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
        {
        wkt = wkt.substr(1);
        wkt.Trim();
        }

    // Optional component
    if ((wkt.length() >= 9) && (wkt.substr (0, 9) == (L"AUTHORITY")))
        authorityID = GetAuthority (wkt);

    wkt.Trim();

    // Check end of section
    if ((wkt.length() < 1) || (!(wkt.substr(0, 1) == L"]")))
        return ERROR;

    wkt = wkt.substr(1);

#ifdef NOT_YET
    if (authorityID.length() > 0)
    {
//        pUnit->SetKey (authorityID->GetKey());
    }
#endif

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts from the provided stream the extension
*   The complete WKT extension section must be provided including the EXTENSION[ ] keyword.
*   The wkt text stream may contain additional text that is returned with the EXTENSION section
*   removed.
*
*   @param wkt IN/OUT The WKT portion that contains the extension to extract.
*
*   @param extensionName OUT Reference to a string that receives the extension name.
*
*   @param extentionText OUT The text associated with extension
*
*   @return SUCCESS if succesful or another value otherwise.
*
*   @bsimethod                                                  Alain Robert 2009/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetExtension (WStringR wkt, WStringR extensionName, WStringR extensionText) const
    {
    wkt.Trim();

    // Validate that this is the proper section (must start with ")
    if ((wkt.length() < 9) || (!(wkt.substr (0, 9) == L"EXTENSION")))
        return ERROR;

    // Remove keyword
    wkt = wkt.substr (9);

    // Trim again
    wkt.Trim();

    // Make sure that remainder starts with [
    if ((wkt.length() < 1) || (!(wkt.substr (0, 1) == L"[")))
        return ERROR;

    wkt = wkt.substr (1);

    extensionName = GetName (wkt);

    wkt.Trim();

    if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
        wkt = wkt.substr(1);

    wkt.Trim();
    extensionText = GetName (wkt);

    wkt.Trim();

    // Check end of section
    if ((wkt.length() < 1) || (!(wkt.substr(0, 1) == L"]")))
        return ERROR;

    wkt = wkt.substr(1);

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts from the provided stream the linear unit
*   and sets it in the provided coordinate system.
*
*   @param wkt IN/OUT The WKT portion that contains the unit to extract.
*
*   @param coordinateSystem IN/OUT The coordinate system to set the units of.
*
*   @return SUCCESS if successful or another value in case of error.
*
*   @bsimethod                                                  Alain Robert 2004/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetLinearUnitToCoordSys (WStringR wkt, BaseGCSR coordinateSystem) const
    {
    StatusInt   status = SUCCESS;
    double      unitFactor;
    WString     unitName;
    if (SUCCESS != (status = GetUnit (wkt, unitName, &unitFactor)))
        return status;

    GeoCoordinates::UnitEnumerator* unitEnumerator = new GeoCoordinates::UnitEnumerator();
    GeoCoordinates::UnitCP currentUnit;
    int currentUnitCode = 0;
    int foundUnitCode = -1;
    while ((foundUnitCode < 0) && (unitEnumerator->MoveNext()))
        {
        currentUnit = unitEnumerator->GetCurrent();

        if (currentUnit->GetBase() == GeoUnitBase::Meter)
            {
            double dictConversionFactor = currentUnit->GetConversionFactor();
            if ((unitFactor < dictConversionFactor + 0.00000001) && (unitFactor > dictConversionFactor - 0.00000001))
                foundUnitCode = currentUnitCode;
            }
        currentUnitCode++;
        }

    if (foundUnitCode < 0)
        return ERROR;

    coordinateSystem.SetUnitCode (foundUnitCode);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts from the provided stream the angle unit
*   and returns the conversion factor to meter.
*   The complete WKT unit section must be provided including the UNIT[ ] keyword.
*   The wkt text stream may contain additional text that is returned with the UNIT section
*   removed.
*
*   @param wkt IN/OUT The WKT portion that contains the unit to extract.
*
*   @param conversion
*
*   @return The angle unit
*
*   @bsimethod                                                  Alain Robert 2004/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetAngleUnit (WStringR wkt, double* conversionToDegree) const
    {
    StatusInt   status = SUCCESS;
    WString     unitName;
    double      conversionToRadians = 1.0;
    status = GetUnit (wkt, unitName, &conversionToRadians);
    *conversionToDegree = conversionToRadians * 180.0 / PI;

    // If the unit name is grad ... we reject. The support of GRAD WKT is a nightmare since everybody has
    // implemented anything and everything concerning angular unit interpretation in this case especially ESRI and Oracle
    if ((unitName == L"grad") || (unitName == L"Grad") || (unitName == L"GRAD"))
        return ERROR;

    return status;
    }


/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts from the provided stream the name
*   which must be enclosed within double-quotes. The first non white character must be
*   the opening double quote. The stream is returned with name component removed
*
*   @param wkt IN The WKT portion that contains the name to extract.
*
*   @return The name
*
*   @bsimethod                                                  Alain Robert 2004/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString     GetName (WStringR wkt) const
    {
    wkt.Trim ();

    // Validate that this is the proper section (must start with ")
    if ((wkt.length() < 1) || (!(wkt.substr (0, 1) == L"\"")))
        return L"";

    // Remove keyword
    wkt = wkt.substr (1);

    // Obtain the next double quote location
    size_t index = wkt.find_first_of (L"\"");

    if (index == WString::npos)
        return L"";

    WString name = wkt.substr (0, index);

    // Remove name section from text stream
    wkt = wkt.substr (index + 1);

    return name;
    }

/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts from the provided stream the keyword
*   which must start at the first non blank character and end with a section end or a comma.
*
*   @param wkt IN The WKT portion that contains the keyword to extract.
*
*   @return The keyword
*
*   @bsimethod                                                  Alain Robert 2004/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString     GetKeyword (WStringR& wkt) const
    {
    wkt.Trim();

    // Obtain the next double quote location
    size_t index1 = wkt.find_first_of (L",");
    size_t index2 = wkt.find_first_of (L"]");

    size_t index = 0;
    if (index1 != WString::npos && index2 != WString::npos)
        index = (index1 < index2 ? index1 : index2);
    else
        {
        if (index1 != WString::npos)
            index = index1;
        else
            index = index2;
        }

    if (0 == index)
        {
        return L"";
        }

    if (index == WString::npos)
        index = wkt.length();

    WString keyword = wkt.substr (0, index);

    wkt = wkt.substr (index);

    return keyword;
    }

/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts from the provided stream the authority ID
*   The complete WKT authority section must be provided including the AUTHORITY[ ] keyword.
*   The wkt text stream may contain additional text that is returned with the AUTHORITY section
*   removed.
*
*   @param wkt IN/OUT The WKT portion that contains the authority to extract.
*
*   @return The authority identifier
*
*   @bsimethod                                                  Alain Robert 2004/09
+---------------+---------------+---------------+---------------+---------------+------*/
WString GetAuthority (WStringR wkt) const
    {
    WString     authorityID;
    wkt.Trim();

    // Validate that this is the proper section (must start with ")
    if ((wkt.length() < 9) || (!(wkt.substr (0, 9) == L"AUTHORITY")))
        return L"";

    // Remove keyword
    wkt = wkt.substr (9);

    // Trim again
    wkt.Trim();

    // Make sure that remainder starts with [
    if ((wkt.length() < 1) || (!(wkt.substr (0, 1) == L"[")))
        {
        return L"";
        }
    wkt = wkt.substr (1);

    WString     authorityName = GetName (wkt);
    wkt.Trim();
    if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
        wkt = wkt.substr(1);
    WString     authorityCode = GetName (wkt);
    wkt.Trim();

    // Check end of section
    if ((wkt.length() < 1) || (!(wkt.substr(0, 1) == L"]")))
        return L"";

    wkt = wkt.substr(1);

    if (authorityName.length() > 0)
        {
        if (authorityCode.length() > 0)
            {
            authorityID = authorityName + L":" + authorityCode;
            }
        else
            authorityID = authorityName;
        }
    else
        authorityID = authorityCode;

    return authorityID;
    }

/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts from the provided stream the axis definition
*   The complete WKT authority section must be provided including the AXIS[ ] keyword.
*   The wkt text stream may contain additional text that is returned with the AXIS section
*   removed.
*
*   @param wkt IN/OUT The WKT portion that contains the axis to extract.
*
*   @return The axis identifier
*
*   @bsimethod                                                  Alain Robert 2004/09
+---------------+---------------+---------------+---------------+---------------+------*/
AxisDirection GetAxis (WStringR wkt) const
    {
    WString     authorityID;
    wkt.Trim();

    // Validate that this is the proper section (must start with ")
    if ((wkt.length() < 4) || (!(wkt.substr (0, 4) == L"AXIS")))
        return AxisDirection::UNDEFINED;

    // Remove keyword
    wkt = wkt.substr (4);

    // Trim again
    wkt.Trim();

    // Make sure that remainder starts with [
    if ((wkt.length() < 1) || (!(wkt.substr (0, 1) == L"[")))
        {
        return AxisDirection::UNDEFINED;
        }
    wkt = wkt.substr (1);

    WString     name = GetName (wkt);
    wkt.Trim();
    if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
        wkt = wkt.substr(1);

    WString     keyword = GetKeyword (wkt);
    wkt.Trim();

    // Check end of section
    if ((wkt.length() < 1) || (!(wkt.substr(0, 1) == L"]")))
        return AxisDirection::UNDEFINED;

    wkt = wkt.substr(1);

    if (keyword == L"NORTH")
        return AxisDirection::NORTH;
    if (keyword == L"SOUTH")
        return AxisDirection::SOUTH;
    if (keyword == L"EAST")
        return AxisDirection::EAST;
    if (keyword == L"WEST")
        return AxisDirection::WEST;
    if (keyword == L"UP")
        return AxisDirection::UP;
    if (keyword == L"DOWN")
        return AxisDirection::DOWN;
    if (keyword == L"OTHER")
        return AxisDirection::OTHER;
    
    return AxisDirection::UNDEFINED;
    }


/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts from the provided stream the local datum
*   The complete WKT authority section must be provided including the LOCAL_DATUM[ ] keyword.
*   The wkt text stream may contain additional text that is returned with the LOCAL_DATUM section
*   removed.
*
*   @param wkt IN/OUT The WKT portion that contains the local datum to extract.
*
*   @return SUCCESS if successful or another value in case of error.
*
*   @bsimethod                                                  Alain Robert 2004/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetLocalDatumToCoordSys (WStringR wkt, BaseGCSR coordinateSystem) const
    {
    StatusInt status = SUCCESS;

    wkt.Trim();

    // Validate that this is the proper section (must start with ")
    if ((wkt.length() < 11) || (!(wkt.substr (0, 11) == L"LOCAL_DATUM")))
        return ERROR;

    // Remove keyword
    wkt = wkt.substr (11);

    // Trim again
    wkt.Trim();

    // Make sure that remainder starts with [
    if ((wkt.length() < 1) || (!(wkt.substr (0, 1) == L"[")))
        return ERROR;
    wkt = wkt.substr (1);

    WString name = GetName (wkt);
    WString authorityID = GetAuthorityIdFromNameOracleStyle(name);

    wkt.Trim();
    if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
        wkt = wkt.substr(1);
    /*int32_t type = (int32_t)*/(GetDouble (wkt));
    wkt.Trim();

    if ((wkt.length() >= 9) && (wkt.substr (0, 9) == (L"AUTHORITY")))
        authorityID = GetAuthority (wkt);

    wkt.Trim();

    // Check end of section
    if ((wkt.length() < 1) || (!(wkt.substr(0, 1) == L"]")))
        return ERROR;

    wkt = wkt.substr(1);


    // Since the NERTH coordinate system does not make any use
    // of a local datum concept, we will simply ignore the data that
    // has been parsed out of the WKT stream

    return status;
    }


/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts from the provided stream the TOWGS84
*   horizontal datum transformation.
*   The complete WKT authority section must be provided including the TOWGS84[ ] keyword.
*   The wkt text stream may contain additional text that is returned with the TOWGS84 section
*   removed.
*
*   @param wkt IN/OUT The WKT portion that contains the horizontal datum transformation to extract.
*
*   @return The horizontal datum transformation
*
*   @bsimethod                                                  Alain Robert 2004/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt GetTOWGS84ToCoordSys (WStringR wkt, BaseGCSR coordinateSystem) const
    {
    StatusInt status = SUCCESS;

    wkt.Trim();

    // Validate that this is the proper section (must start with ")
    if ((wkt.length() < 7) || (!(wkt.substr (0, 7) == L"TOWGS84")))
        {
        return ERROR;
        }

    // Remove keyword
    wkt = wkt.substr (7);

    // Trim again
    wkt.Trim();

    // Make sure that remainder starts with [
    if ((wkt.length() < 1) || (!(wkt.substr (0, 1) == L"[")))
        {
        return ERROR;
        }
    wkt = wkt.substr (1);


    status = Get7ParamsDatumTransformationToCoordSys (wkt, coordinateSystem);

    // Check end of section
    if ((wkt.length() < 1) || (!(wkt.substr(0, 1) == L"]")))
        {
        return ERROR;
        }

    wkt = wkt.substr(1);

    // Since custom datum specifications are not currently supported we simply go on with parsed out
    // TOWGS84 clause without setting any GCS member.

    return status;
    }

/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts from the provided stream the 7 parameters
*   horizontal datum transformation.
*   The complete 7 double WKT authority section must be provided.
*   The wkt text stream may contain additional text that is returned with the 7 parameters section
*   removed.
*
*   @param wkt IN/OUT The WKT portion that contains the horizontal datum transformation to extract.
*
*   @return The horizontal datum transformation
*
*   @bsimethod                                                  Alain Robert 2004/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Get7ParamsDatumTransformationToCoordSys (WStringR wkt, BaseGCSR coordinateSystem) const
    {
    StatusInt status = SUCCESS;
    double shiftX;
    double shiftY;
    double shiftZ;
    double rotationX;
    double rotationY;
    double rotationZ;
    double scale;


    wkt.Trim();


    // This may mean that the datum contains the datum shift definitions
    // 7 numbers to follow
    shiftX = GetDouble (wkt);
    wkt.Trim();
    if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
        wkt = wkt.substr(1);

    shiftY = GetDouble (wkt);
    wkt.Trim();
    if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
        wkt = wkt.substr(1);

    shiftZ = GetDouble (wkt);
    wkt.Trim();
    if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
        wkt = wkt.substr(1);

    rotationX = GetDouble (wkt);
    wkt.Trim();
    if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
        wkt = wkt.substr(1);

    rotationY = GetDouble (wkt);
    wkt.Trim();
    if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
        wkt = wkt.substr(1);

    rotationZ = GetDouble (wkt);
    wkt.Trim();
    if ((wkt.length() >= 1) && (wkt.substr(0, 1) ==(L",")))
        wkt = wkt.substr(1);

    scale = GetDouble (wkt);
    wkt.Trim();

    // In this case the section end is mandatory
    if ((wkt.length() < 1) || (!(wkt.substr(0, 1) == L"]")))
        {
        return ERROR;
        }

    // Since we do not currently support custom datum definitions we will
    // simply ignore the extracted parameters ... given we want to allow
    // custom datum definitions we may simpky set the extracted
    // parameters using the 7-parameter method.
    return status;
    }



/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts from the provided stream the double
*   The first non white character must be the number to extract.
*
*   @param wkt IN The WKT portion that contains the number to extract.
*
*   @return The number
*
*   @bsimethod                                                  Alain Robert 2004/09
+---------------+---------------+---------------+---------------+---------------+------*/
double GetDouble (WStringR wkt) const
    {
    wkt.Trim();

    // Obtain the next double quote location
    size_t index1 = wkt.find_first_of (L",");
    size_t index2 = wkt.find_first_of (L"]");

    size_t index = 0;
    if (index1 != WString::npos && index2 != WString::npos)
        index = (index1 < index2 ? index1 : index2);
    else
        {
        if (index1 != WString::npos)
            index = index1;
        else
            index = index2;
        }

    if (0 == index)
        {
        return 0.0; // Return default value and let parser fail elsewhere in case of structural problem
        }

    if (index == WString::npos)
        index = wkt.length();

    double value = BeStringUtilities::Wtof (wkt.substr(0, index).c_str());

    wkt = wkt.substr (index);

    return value;
    }

/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts from the provided stream the integer
*   The first non white character must be the number to extract.
*
*   @param wkt IN The WKT portion that contains the number to extract.
*
*   @return The number
*
*   @bsimethod                                                  Alain Robert 2015/07
+---------------+---------------+---------------+---------------+---------------+------*/
long GetInteger (WStringR wkt) const
    {
    wkt.Trim();

    // Obtain the next double quote location
    size_t index1 = wkt.find_first_of (L",");
    size_t index2 = wkt.find_first_of (L"]");

    size_t index = 0;
    if (index1 != WString::npos && index2 != WString::npos)
        index = (index1 < index2 ? index1 : index2);
    else
        {
        if (index1 != WString::npos)
            index = index1;
        else
            index = index2;
        }

    if (0 == index)
        {
        return 0; // Return default value and let parser fail elsewhere in case of structural problem
        }

    if (index == WString::npos)
        index = wkt.length();

    long value = BeStringUtilities::Wtoi (wkt.substr(0, index).c_str());

    wkt = wkt.substr (index);

    return value;
    }


/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts from the provided stream the double
*   The first non white character must be the number ot extract.
*
*   @param wkt IN The WKT portion that contains the number to extract.
*
*   @param stringValue OUT A reference to a string that will receive the string value
*   prior to conversion to a floating-point value. In rare dialects the parameter
*   value is in text form for obscure parameter types such as Zone or Hemisphere.
*
*   @return The number
*
*   @bsimethod                                                  Alain Robert 2004/09
+---------------+---------------+---------------+---------------+---------------+------*/
double GetDoubleAndString (WStringR wkt, WStringR stringValue) const
    {
    wkt.Trim();

    // Obtain the next double quote location
    size_t index1 = wkt.find_first_of (L",");
    size_t index2 = wkt.find_first_of (L"]");

    size_t index = 0;
    if (index1 != WString::npos && index2 != WString::npos)
        index = (index1 < index2 ? index1 : index2);
    else
        {
        if (index1 != WString::npos)
            index = index1;
        else
            index = index2;
        }

    if (0 == index)
        {
        return 0.0; // Return default value and let parser fail elsewhere in case of structural problem
        }

    if (index == WString::npos)
        index = wkt.length();

    stringValue = wkt.substr(0, index);
    double value = BeStringUtilities::Wtof (stringValue.c_str());

    wkt = wkt.substr (index);

    return value;
    }


/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method converts the projection method name as
*   extracted from the WKT to the CSMAP projection code. Some projection code have
*   no WKT equivalent and some WKT projection names have no equivalent projection code.
*
*   @param name IN The projection name as extracted from the WKT.
*
*   @return The projection code or a negative value if projection is not supported.
*
*   @bsimethod                                                  Alain Robert 2004/09
+---------------+---------------+---------------+---------------+---------------+------*/
BaseGCS::ProjectionCodeValue GetProjectionCodeFromWKTName (WStringR name) const
    {

    BaseGCS::ProjectionCodeValue ID = BaseGCS::pcvInvalid;

    WString upperMethodName = name;
    std::transform(upperMethodName.begin(), upperMethodName.end(), upperMethodName.begin(), (int (*)(int))std::toupper);

    if (upperMethodName == L"GEOGRAPHIC (LAT/LONG)") // Strange value ... should not occur but it does ..
        ID = BaseGCS::pcvUnity;
    if ((upperMethodName == L"ALBERS") ||
        (upperMethodName == L"ALBERS_CONIC_EQUAL_AREA") ||  // Added OGR Name
        (upperMethodName == L"ALBERS CONICAL EQUAL AREA"))   // Oracle name
        ID = BaseGCS::pcvAlbersEqualArea;
    else if ((upperMethodName == L"LAMBERT_CONFORMAL_CONIC") ||
             (upperMethodName == L"LAMBERT_CONFORMAL_CONIC_2SP") || // OGR name
             (upperMethodName == L"LAMBERT CONFORMAL CONIC") || // Oracle name
             (upperMethodName == L"LAMBERT CONFORMAL CONIC, TWO STANDARD PARALLELS")) // Some weird variant encountered in POD files
        ID = BaseGCS::pcvLambertConformalConicTwoParallel;
    else if (upperMethodName == L"LAMBERT_CONFORMAL_CONIC_1SP") // Name from OGR
        ID = BaseGCS::pcvLambertConformalConicOneParallel;
    else if ((upperMethodName == L"MERCATOR") ||
             (upperMethodName == L"MERCATOR_2SP")) // Added OGR Name
        ID = BaseGCS::pcvMercator;
    else if (upperMethodName == L"MERCATOR_1SP") // Name from OGR
        ID = BaseGCS::pcvMercatorScaleReduction;
    else if ((upperMethodName == L"POLAR_STEREOGRAPHIC") ||
             (upperMethodName == L"POLAR STEREOGRAPHIC"))
        ID = BaseGCS::pcvPolarStereographic;
    else if (upperMethodName == L"OBLIQUE_STEREOGRAPHIC") // Name comes from OGR
        ID = BaseGCS::pcvObliqueStereographic;
    else if (upperMethodName == L"POLYCONIC")
        ID = BaseGCS::pcvAmericanPolyconic;
    else if ((upperMethodName == L"EQUIDISTANT_CONIC") ||
             (upperMethodName == L"EQUIDISTANT CONIC"))
        ID = BaseGCS::pcvEquidistantConic;
    else if (upperMethodName == L"GAUSS_KRUGER")
        ID = BaseGCS::pcvGaussKrugerTranverseMercator;
    else if ((upperMethodName == L"TRANSVERSE_MERCATOR") ||
             (upperMethodName == L"TRANSVERSE MERCATOR") ||
             (upperMethodName == L"CT_TRANSVERSEMERCATOR") ||
             (upperMethodName == L"TRANSVERSE_MERCATOR_MAPINFO_21") ||
             (upperMethodName == L"TRANSVERSE_MERCATOR_MAPINFO_22") ||
             (upperMethodName == L"TRANSVERSE_MERCATOR_MAPINFO_23") ||
             (upperMethodName == L"TRANSVERSE_MERCATOR_MAPINFO_24") ||
             (upperMethodName == L"TRANSVERSE_MERCATOR_MAPINFO_25"))
        ID = BaseGCS::pcvTransverseMercator;
#if defined(TOTAL_SPECIAL)
    else if (upperMethodName == L"TRANSVERSE_MERCATOR_COMPLEX")
        ID = BaseGCS::pcvTotalTransverseMercatorBF;
#endif
    else if (upperMethodName == L"STEREOGRAPHIC")
        ID = BaseGCS::pcvSnyderObliqueStereographic;
    else if ((upperMethodName == L"LAMBERT_AZIMUTHAL_EQUAL_AREA") ||
             (upperMethodName == L"LAMBERT AZIMUTHAL EQUAL AREA"))
        ID = BaseGCS::pcvLambertEqualAreaAzimuthal;
    else if ((upperMethodName == L"AZIMUTHAL_EQUIDISTANT") ||
             (upperMethodName == L"AZIMUTHAL EQUIDISTANT")) // Oracle name
        ID = BaseGCS::pcvLambertEquidistantAzimuthal;
    else if ((upperMethodName == L"EQUIDISTANT_CYLINDRICAL") ||
             (upperMethodName == L"EQUIRECTANGULAR"))
        ID = BaseGCS::pcvEquidistantCylindrical;
    else if (upperMethodName == L"GNOMONIC")
        ID = BaseGCS::pcvGnomonic;
    else if (upperMethodName == L"ORTHOGRAPHIC")
        ID = BaseGCS::pcvOrthographic;
    else if (upperMethodName == L"SINUSOIDAL")
        ID = BaseGCS::pcvSinusoidal;
#ifdef NOT_YET
// Support of Krovak projection is just too much error prone ... There are various ways to express it and sometimes the
// angle complements appear provided instead of angles proper.
// Users should simply select the appropriate projection in dictionary for the moment
    else if (upperMethodName == L"KROVAK")
        ID = BaseGCS::pcvCzechKrovak; // ?? cs_PRJCOD_KRVK95
#endif
    else if ((upperMethodName == L"MILLER_CYLINDRICAL") ||
             (upperMethodName == L"MILLER CYLINDRICAL"))
        ID = BaseGCS::pcvMillerCylindrical;
    else if ((upperMethodName == L"VAN_DER_GRINTEN_I") ||
             (upperMethodName == L"VANDERGRINTEN") ||
             (upperMethodName == L"VAN DER GRINTEN"))
        ID = BaseGCS::pcvVanderGrinten;
    else if ((upperMethodName == L"HOTINE_OBLIQUE_MERCATOR_AZIMUTHAL_NATURAL_ORIGIN") ||
             (upperMethodName == L"HOTINE OBLIQUE MERCATOR") ||
             (upperMethodName == L"HOTINE_OBLIQUE_MERCATOR"))
        ID = BaseGCS::pcvHotineObliqueMercator1XY;
    else if (upperMethodName == L"RECTIFIED_SKEW_ORTHOMORPHIC_NATURAL_ORIGIN")
        ID = BaseGCS::pcvRectifiedSkewOrthomorphic;
    else if (upperMethodName == L"ROBINSON")
        ID = BaseGCS::pcvRobinsonCylindrical;
    else if ((upperMethodName == L"INTERRUPTED_GOODE_HOMOLSINE") ||
             (upperMethodName == L"INTERRUPTED GOODE HOMOLSINE") ||
             (upperMethodName == L"GOODE_HOMOLSINE"))
        ID = BaseGCS::pcvGoodeHomolosine;
    else if ((upperMethodName == L"MOLLWEIDE") ||
            (upperMethodName == L"INTERRUPTED MOLLWEIDE"))
        ID = BaseGCS::pcvMollweide;
    else if ((upperMethodName == L"ECKERT_IV") ||
             (upperMethodName == L"ECKERT IV"))
        ID = BaseGCS::pcvEckertIV;
    else if ((upperMethodName == L"ECKERT_VI") ||
             (upperMethodName == L"ECKERT VI"))
        ID = BaseGCS::pcvEckertVI;
    else if ((upperMethodName == L"LAMBERT_CONFORMAL_CONIC_2SP_BELGIUM") ||
             (upperMethodName == L"LAMBERT CONFORMAL CONIC (BELGIUM 1972)"))
        ID = BaseGCS::pcvLambertConformalConicBelgian;
    else if ((upperMethodName == L"NEW_ZEALAND_MAP_GRID") ||
             (upperMethodName == L"NEW ZEALAND MAP GRID"))
        ID = BaseGCS::pcvNewZealandNationalGrid;
    else if ((upperMethodName == L"CYLINDRICAL_EQUAL_AREA") ||
             (upperMethodName == L"CYLINDRICAL EQUAL AREA"))
        ID = BaseGCS::pcvEqualAreaAuthalicNormal;
    else if ((upperMethodName == L"SWISS_OBLIQUE_CYLINDRICAL")||
             (upperMethodName == L"SWISS_OBLIQUE_MERCATOR") ||
             (upperMethodName == L"SWISS OBLIQUE MERCATOR"))
        ID = BaseGCS::pcvObliqueCylindricalSwiss;
#ifdef NOT_YET
// Something wrong in our interpretation of Bonne projection ... to be done
// No hurry this one is very rarely used (if at all)
    else if (upperMethodName == L"BONNE")
        ID = BaseGCS::pcvBonne;
#endif
    else if ((upperMethodName == L"CASSINI") || (upperMethodName == L"CASSINI_SOLDNER")) // Added OGR Name
        ID = BaseGCS::pcvCassini;
    else if (upperMethodName == L"WINKEL_TRIPEL")
        ID = BaseGCS::pcvWinkelTripel;

// The following known WKT names have no mapping to CSMAP entries.

// These projections are cute little projections for historical purposes or for atlas
// views of the world. Support is certainly not essential:

// "Space Oblique Mercator"
// "Hammer"
// "Wagner IV"
// "Wagner VII"
// "Oblated Equal Area"
// "Gall_Stereographic"
// "Aitoff"
// "Craster_Parabolic"
// "Behrmann"
// "Hammer_Aitoff"
// "Eckert_I"
// "Eckert_II"
// "Eckert_III"
// "Eckert_V"
// "Flat_Polar_Quartic"
// "Loximuthal"
// "Quartic_Authalic"
// "Times"
// "Cube"
// "Fuller"
// "Winkel_I"
// "Winkel_II"

// The following could probably be supported but would require additional study and in some
// case the imposition of specific parameters.
// "PLATE_CARREE"
// "EQUIRECTANGULAR"
// "HOTINE_OBLIQUE_MERCATOR_AZIMUTH_CENTER"
// "Alaska Conformal"
// "Transverse Mercator Danish System 45 Bornholm"
// "Transverse Mercator Danish System 34 Jylland-Fyn"
// "Transverse Mercator Sjaelland"
// "Transverse Mercator Finnish KKJ"
// "Stereographic_North_Pole"
// "Stereographic_South_Pole"

// Not supported but could probably be useful
// "Tunisia_Mining_Grid"
// "Vertical_Near_Side_Perspective" OR "General Vertical Near-Side Perspective"


// The following CSMAP projections do not appear to have equivalent WKT entries.
// cs_PRJCOD_LMTAN
// cs_PRJCOD_TACYL
// cs_PRJCOD_HOM2XY
// cs_PRJCOD_MSTRO
// cs_PRJCOD_SOTRM
// cs_PRJCOD_UTM
// cs_PRJCOD_RSKEWC
// cs_PRJCOD_OBQCYL
// cs_PRJCOD_OSTN97
// cs_PRJCOD_OSTN02
// cs_PRJCOD_SYS34_99
// cs_PRJCOD_SYS34_01
// cs_PRJCOD_EDCYLE
// cs_PRJCOD_PCARREE
// cs_PRJCOD_MRCATPV
// cs_PRJCOD_AZEDE
// cs_PRJCOD_PSTROSL
// cs_PRJCOD_TRMERAF
// cs_PRJCOD_MODPC
// cs_PRJCOD_BPCNC
// cs_PRJCOD_MNDOTL
// cs_PRJCOD_WCCSL
// cs_PRJCOD_MNDOTT
// cs_PRJCOD_WCCST
// cs_PRJCOD_RSKEWO
// cs_PRJCOD_TRMRKRG

    return ID;

    }
/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method sets the parameter value based on the
*   parameter name and projection code used by the coordinate system.
*
*   @param parameterName IN The parameter name as extracted from WKT.
*
*   @param parameterStringValue IN The parameter string value as in the WKT. This value
*   can be used when parameter calls for a string value instead of a numeric value (rarely)
*
*   @param parameterValue IN The value of the parameter.
*
*   @param IN/OUT The coordinate system to set the parameter value of. The ProjectionCode
*   must already have been properly set as it is used in the interpretation of the
*   parameter.
*
*   @return SUCCESS if successful and any other value in case of error.
*
*   @bsimethod                                                  Alain Robert 2004/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt SetParameterToCoordSys (WStringR parameterName, WStringR parameterStringValue, double parameterValue, double conversionToDegree, BaseGCSR coordinateSystem) const
    {

    // Obtain uppercase value
    WString upperParameterName = parameterName;
    std::transform(upperParameterName.begin(), upperParameterName.end(), upperParameterName.begin(), (int (*)(int))std::toupper);


    if ((upperParameterName == L"FALSE_EASTING") ||
        (upperParameterName == L"FALSEEASTING"))
        coordinateSystem.SetFalseEasting (parameterValue);
    else if ((upperParameterName == L"FALSE_NORTHING") ||
             (upperParameterName == L"FALSENORTHING"))
        coordinateSystem.SetFalseNorthing (parameterValue);
    else if ((upperParameterName == L"LATITUDE_OF_ORIGIN") ||
             (upperParameterName == L"LATITUDE_OF_CENTER") ||
             (upperParameterName == L"CENTRAL_PARALLEL") ||
             (upperParameterName == L"NATORIGINLAT"))
        {
        if ((BaseGCS::pcvHotineObliqueMercator1XY == coordinateSystem.GetProjectionCode()) ||
            (BaseGCS::pcvRectifiedSkewOrthomorphic == coordinateSystem.GetProjectionCode()))
            coordinateSystem.SetCentralPointLatitude (parameterValue * conversionToDegree);
        else
            coordinateSystem.SetOriginLatitude (parameterValue * conversionToDegree);
        }
    else if (upperParameterName == L"CENTRAL_MERIDIAN")
        {
        if ((BaseGCS::pcvObliqueCylindricalSwiss == coordinateSystem.GetProjectionCode()) ||
            (BaseGCS::pcvLambertConformalConicTwoParallel == coordinateSystem.GetProjectionCode()) ||
            (BaseGCS::pcvLambertConformalConicOneParallel == coordinateSystem.GetProjectionCode()) ||     // This fixes TR# 288399
            (BaseGCS::pcvLambertConformalConicBelgian == coordinateSystem.GetProjectionCode()) ||
            (BaseGCS::pcvAlbersEqualArea == coordinateSystem.GetProjectionCode()) )
            coordinateSystem.SetOriginLongitude (parameterValue * conversionToDegree);
        else if ((BaseGCS::pcvHotineObliqueMercator1XY == coordinateSystem.GetProjectionCode()) ||
                 (BaseGCS::pcvRectifiedSkewOrthomorphic == coordinateSystem.GetProjectionCode()))
            coordinateSystem.SetCentralPointLongitude (parameterValue * conversionToDegree);
        else
            coordinateSystem.SetCentralMeridian (parameterValue * conversionToDegree);
        }
    else if ((upperParameterName == L"SCALE_FACTOR") ||
             (upperParameterName == L"SCALEATNATORIGIN"))
        coordinateSystem.SetScaleReduction (parameterValue);
    else if (upperParameterName == L"STANDARD_PARALLEL_1")
        {
        if (BaseGCS::pcvBonne == coordinateSystem.GetProjectionCode())
            coordinateSystem.SetOriginLatitude (parameterValue * conversionToDegree); // Weird occurence !
        else
            coordinateSystem.SetStandardParallel1 (parameterValue * conversionToDegree);
        }
    else if (upperParameterName == L"STANDARD_PARALLEL_2")
        coordinateSystem.SetStandardParallel2 (parameterValue * conversionToDegree);
    else if (upperParameterName == L"AZIMUTH")
        coordinateSystem.SetAzimuth (parameterValue * conversionToDegree);
        else if (upperParameterName == L"ZONENUMBER")
        coordinateSystem.SetUTMZone ((int)parameterValue);
        else if (upperParameterName == L"HEMISPHERE")
        {
        int hemisphere;
        if (parameterStringValue.length() == 0)
            hemisphere = 1;
        else if (parameterStringValue == L"N")
            hemisphere = 1;
        else
            hemisphere = -1;
        coordinateSystem.SetHemisphere (hemisphere);
        }
        else if ((upperParameterName == L"LONGITUDE_OF_ORIGIN") ||
                 (upperParameterName == L"LONGITUDE_OF_CENTER") ||
                 (upperParameterName == L"NATORIGINLONG"))
        {
        switch (coordinateSystem.GetProjectionCode())
            {
            case BaseGCS::pcvGaussKrugerTranverseMercator:
            case BaseGCS::pcvTransverseMercator:
            case BaseGCS::pcvTotalTransverseMercatorBF:
            case BaseGCS::pcvCassini:
            case BaseGCS::pcvMillerCylindrical:
            case BaseGCS::pcvAmericanPolyconic:
            case BaseGCS::pcvSouthOrientedTransverseMercator:
            case BaseGCS::pcvMercator:
            case BaseGCS::pcvMercatorScaleReduction:
            case BaseGCS::pcvModifiedPolyconic:
            case BaseGCS::pcvTransverseMercatorMinnesota:
            case BaseGCS::pcvTransverseMercatorWisconsin:
            case BaseGCS::pcvObliqueCylindricalSwiss:
                coordinateSystem.SetCentralMeridian (parameterValue * conversionToDegree);
                break;

            case BaseGCS::pcvHotineObliqueMercator1XY:
            case BaseGCS::pcvRectifiedSkewOrthomorphic:
                coordinateSystem.SetCentralPointLongitude (parameterValue * conversionToDegree);
                break;
            default:
                coordinateSystem.SetOriginLongitude (parameterValue * conversionToDegree);
            }
        }

        else if (upperParameterName == L"HEIGHT") // Elevation
        coordinateSystem.SetElevationAboveGeoid (parameterValue);
    else if (upperParameterName == L"PSEUDO_STANDARD_PARALLEL_1")
        coordinateSystem.SetStandardParallel1 (parameterValue * conversionToDegree);
    else if (upperParameterName == L"XY_PLANE_ROTATION") // OGR weirdness ... Not really supported ... a variant for skew angle which is not supported by CSMAP
        return ERROR;
    else
        return ERROR;

    return SUCCESS;

    }

/*---------------------------------------------------------------------------------**//**
*   @description PRIVATE This private method extracts, strips and returns the Oracle style
*   authority ID from the datum, spehroid, prime meridian, or operation name. At one
*   point Oracle used to append to the object name an authority ID in between parenthesis
*   of the form "Anguilla 1957 (EPSG ID 6600)". This function will extract this authroity ID
*   and return it in the form "EPSG:6600" and remove the part from the name.
*
*   @param name IN/OUT The name as extracted from WKT. On output it will contain the stripped name
*               if applicable
*
*   @return The authority ID or an empty string if none can be found
*
*   @bsimethod                                                  Alain Robert 2009/06
+---------------+---------------+---------------+---------------+---------------+------*/
WString GetAuthorityIdFromNameOracleStyle(WStringR name) const
    {
    WString EPSGNumber;
    size_t StartEPSG = name.find(L"EPSG ID ");
    if (StartEPSG != WString::npos)
        {
        size_t EndEPSG = name.find (L")", StartEPSG);
        if (EndEPSG != WString::npos && EndEPSG > StartEPSG)
            {
            EPSGNumber = L"EPSG:" + name.substr (StartEPSG + 8, EndEPSG - StartEPSG - 8);
            name = name.substr(0, StartEPSG-1);
            name.Trim();
            }
        }
    return EPSGNumber;
    }


}; // End class WKT Parser



/*=================================================================================**//**
*
* GeoTiffKey interpreter class.
*
+===============+===============+===============+===============+===============+======*/
struct GeoTiffKeyInterpreter
{
friend struct GeoTiffKeyCreator;
private:

// These are directly cut from http://www.remotesensing.org/geotiff/spec/geotiff6.html#6.1
enum    GeoKeyIds
    {
    // 6.2.1 GeoTIFF Configuration Keys
    GTModelTypeGeoKey               = 1024, /*  Section 6.3.1.1 Codes       */
    GTRasterTypeGeoKey              = 1025, /*  Section 6.3.1.2 Codes       */
    GTCitationGeoKey                = 1026, /* documentation */


    // 6.2.2 Geographic CS Parameter Keys
    GeographicTypeGeoKey            = 2048, /*  Section 6.3.2.1 Codes     */
    GeogCitationGeoKey              = 2049, /* documentation             */
    GeogGeodeticDatumGeoKey         = 2050, /*  Section 6.3.2.2 Codes     */
    GeogPrimeMeridianGeoKey         = 2051, /*  Section 6.3.2.4 codes     */
    GeogLinearUnitsGeoKey           = 2052, /*  Section 6.3.1.3 Codes     */
    GeogLinearUnitSizeGeoKey        = 2053, /* meters                    */
    GeogAngularUnitsGeoKey          = 2054, /*  Section 6.3.1.4 Codes     */
    GeogAngularUnitSizeGeoKey       = 2055, /* radians                   */
    GeogEllipsoidGeoKey             = 2056, /*  Section 6.3.2.3 Codes     */
    GeogSemiMajorAxisGeoKey         = 2057, /* GeogLinearUnits           */
    GeogSemiMinorAxisGeoKey         = 2058, /* GeogLinearUnits           */
    GeogInvFlatteningGeoKey         = 2059, /* ratio                     */
    GeogAzimuthUnitsGeoKey          = 2060, /*  Section 6.3.1.4 Codes     */
    GeogPrimeMeridianLongGeoKey     = 2061, /* GeogAngularUnit           */


    // 6.2.3 Projected CS Parameter Keys
    ProjectedCSTypeGeoKey           = 3072, /*  Section 6.3.3.1 codes   */
    PCSCitationGeoKey               = 3073, /* documentation           */
    ProjectionGeoKey                = 3074, /*  Section 6.3.3.2 codes   */
    ProjCoordTransGeoKey            = 3075, /*  Section 6.3.3.3 codes   */
    ProjLinearUnitsGeoKey           = 3076, /*  Section 6.3.1.3 codes   */
    ProjLinearUnitSizeGeoKey        = 3077, /* meters                  */
    ProjStdParallel1GeoKey          = 3078, /* GeogAngularUnit */
    ProjStdParallel2GeoKey          = 3079, /* GeogAngularUnit */
    ProjNatOriginLongGeoKey         = 3080, /* GeogAngularUnit */
    ProjNatOriginLatGeoKey          = 3081, /* GeogAngularUnit */
    ProjFalseEastingGeoKey          = 3082, /* ProjLinearUnits */
    ProjFalseNorthingGeoKey         = 3083, /* ProjLinearUnits */
    ProjFalseOriginLongGeoKey       = 3084, /* GeogAngularUnit */
    ProjFalseOriginLatGeoKey        = 3085, /* GeogAngularUnit */
    ProjFalseOriginEastingGeoKey    = 3086, /* ProjLinearUnits */
    ProjFalseOriginNorthingGeoKey   = 3087, /* ProjLinearUnits */
    ProjCenterLongGeoKey            = 3088, /* GeogAngularUnit */
    ProjCenterLatGeoKey             = 3089, /* GeogAngularUnit */
    ProjCenterEastingGeoKey         = 3090, /* ProjLinearUnits */
    ProjCenterNorthingGeoKey        = 3091, /* ProjLinearUnits */
    ProjScaleAtNatOriginGeoKey      = 3092, /* ratio   */
    ProjScaleAtCenterGeoKey         = 3093, /* ratio   */
    ProjAzimuthAngleGeoKey          = 3094, /* GeogAzimuthUnit */
    ProjStraightVertPoleLongGeoKey  = 3095, /* GeogAngularUnit */

    // 6.2.4 Vertical CS Keys
    VerticalCSTypeGeoKey            = 4096, /*  Section 6.3.4.1 codes   */
    VerticalCitationGeoKey          = 4097, /* documentation */
    VerticalDatumGeoKey             = 4098, /*  Section 6.3.4.2 codes   */
    VerticalUnitsGeoKey             = 4099, /*  Section 6.3.1.3 codes   */

    //Private Keys
    ProjectedCSTypeGeoKeyLong       = 60000 /* ProjectedCSTypeGeoKey for values greater than 65535 - Used by Imagepp*/
    };

enum    GeoKeyModelType
    {
    ModelTypeUndefined              = 0, /* not defined */
    ModelTypeProjected              = 1, /* Projection Coordinate System         */
    ModelTypeGeographic             = 2, /* Geographic latitude-longitude System */
    ModelTypeGeocentric             = 3, /* Geocentric (X,Y,Z) Coordinate System */
    };

enum    PrimeMeridians
    {
    PM_Greenwich    = 8901,
    PM_Lisbon       = 8902,
    PM_Paris        = 8903,
    PM_Bogota       = 8904,
    PM_Madrid       = 8905,
    PM_Rome         = 8906,
    PM_Bern         = 8907,
    PM_Jakarta      = 8908,
    PM_Ferro        = 8909,
    PM_Brussels     = 8910,
    PM_Stockholm    = 8911,
    PM_Athens       = 8912,
    PM_Oslo         = 8913,
    };

enum    CoordTransCodes
    {
    CT_TransverseMercator               = 1,
    CT_TransvMercator_Modified_Alaska   = 2,
    CT_ObliqueMercator                  = 3,
    CT_ObliqueMercator_Laborde          = 4,
    CT_ObliqueMercator_Rosenmund        = 5,
    CT_ObliqueMercator_Spherical        = 6,
    CT_Mercator                         = 7,
    CT_LambertConfConic_2SP             = 8,
    CT_LambertConfConic_Helmert         = 9,
    CT_LambertAzimEqualArea             = 10,
    CT_AlbersEqualArea                  = 11,
    CT_AzimuthalEquidistant             = 12,
    CT_EquidistantConic                 = 13,
    CT_Stereographic                    = 14,
    CT_PolarStereographic               = 15,
    CT_ObliqueStereographic             = 16,
    CT_Equirectangular                  = 17,
    CT_CassiniSoldner                   = 18,
    CT_Gnomonic                         = 19,
    CT_MillerCylindrical                = 20,
    CT_Orthographic                     = 21,
    CT_Polyconic                        = 22,
    CT_Robinson                         = 23,
    CT_Sinusoidal                       = 24,
    CT_VanDerGrinten                    = 25,
    CT_NewZealandMapGrid                = 26,
    CT_TransvMercator_SouthOriented     = 27,
    };

enum VerticalCSCode
    {
    // All entries from 5000 to 5099 refer to non-Orthometric (ellipsoid) vertical datums
    // A set of geotiff keys can define a vertical CS even if no Geographic CS is defined. 
    // Since a BaseGCS requires the definition of aqn horizontal Geographic Coordinate System and
    // allowing the vertical CS to refer to a different geodetic datum and ellipsoid would not make sense at all
    // For this reason we will interpret all values non-orthometric as plain ellipsoidal (refering to the geodetic datum)
    // regardless the ellipsoid fit or not.
    VertCS_Newlyn =  5101,
    VertCS_North_American_Vertical_Datum_1929 =  5102,
    VertCS_North_American_Vertical_Datum_1988 =  5103,
    VertCS_Yellow_Sea_1956 = 5104,
    VertCS_Baltic_Sea =  5105,
    VertCS_Caspian_Sea = 5106,
    };

CSDefinition            m_csDef;
CSDatumDef              m_csDatumDef;
CSEllipsoidDef          m_csEllipsoidDef;
GeoKeyModelType         m_modelType;
bool                    m_userDefinedGeoCS;
bool                    m_userDefinedProjectedCS;
bool                    m_haveEllipsoid;
bool                    m_haveDatum;
bool                    m_haveCS;
bool                    m_haveUserOriginLongitude;
bool                    m_haveUserOriginLatitude;
bool                    m_haveFalseEasting;
bool                    m_haveFalseNorthing;
int                     m_coordSys;
double                  m_angularUnitsToDegrees;
double                  m_azimuthUnitsToDegrees;
double                  m_linearUnitsToMeters;
VertDatumCode           m_verticalDatum;

#define UserDefinedKeyValue 32767
#define UnDefinedKeyValue   0            // GeoTIFF indicates value 0 is "undefined"

#define PROJ_COORD_TRANS_NAME 60

typedef struct ProjTransInCitation
{
    int32_t m_csCoordSys;
    char m_csName[PROJ_COORD_TRANS_NAME];
    char m_nameInCitation[PROJ_COORD_TRANS_NAME];
} ProjTransInCitation;

public:
GeoTiffKeyInterpreter
(
)
    {
    memset (&m_csDef,           0, sizeof(m_csDef));
    memset (&m_csDatumDef,      0, sizeof(m_csDatumDef));
    memset (&m_csEllipsoidDef,  0, sizeof(m_csEllipsoidDef));

    // initialize paper scale.
    m_csDef.map_scl             = 1.0;
    m_csDef.scl_red             = 1.0;
    m_angularUnitsToDegrees     = 1.0;
    m_azimuthUnitsToDegrees     = 1.0;
    m_linearUnitsToMeters       = 1.0;

    m_modelType                 = ModelTypeUndefined;
    m_coordSys                  = 0;
    m_userDefinedGeoCS          = false;
    m_userDefinedProjectedCS    = false;
    m_haveEllipsoid             = false;
    m_haveDatum                 = false;
    m_haveCS                    = false;

    m_haveUserOriginLongitude   = false;
    m_haveUserOriginLatitude    = false;
    m_haveFalseEasting          = false;
    m_haveFalseNorthing         = false;

    m_verticalDatum             = vdcFromDatum;

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       Process
(
BaseGCSR                outGCS,
StatusInt              *warning,            // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
WStringP                warningOrErrorMsg,  // Error message.
IGeoTiffKeysList const& geoTiffKeys,        // The GeoTiff key list
bool                    allowUnitsOverride   // Indicates if the presence of a unit can override GCS units. 
)
    {
    if (NULL != warning)
        *warning = SUCCESS;

    // step through the GeoTiffKeys.
    StatusInt                           status = GEOCOORDERR_UnexpectedGeoTiffModelType;
    IGeoTiffKeysList::GeoKeyItem        geoKey;
    bool                                gotKey;
    int                                 iItem;

    // this will cause the type 66 to be saved with coordSysId set the same as projType.
    outGCS.m_coordSysId = 0;

    for (iItem=0, gotKey = geoTiffKeys.GetFirstKey (&geoKey); gotKey && iItem < 200; gotKey = geoTiffKeys.GetNextKey (&geoKey), iItem++)
        {
        switch (geoKey.KeyID)
            {
            case GTModelTypeGeoKey:
                if (IsFatalGeoTiffError (status = ProcessModelTypeKey (geoKey)))
                    return status;
                break;

            case GeographicTypeGeoKey:
                if (IsFatalGeoTiffError (status = ProcessGeographicTypeKey (geoKey)))
                    return status;
                break;

            case GeogCitationGeoKey:
                if (IsFatalGeoTiffError (status = ProcessGeographicCitationKey (geoKey)))
                    return status;
                break;

            case GeogGeodeticDatumGeoKey:
                if (IsFatalGeoTiffError (status = ProcessGeodeticDatumKey (geoKey)))
                    return status;
                break;

            case GeogPrimeMeridianGeoKey:
                if (IsFatalGeoTiffError (status = ProcessPrimeMeridianKey (geoKey)))
                    return status;
                break;

            case GeogLinearUnitsGeoKey:
                // I think the linear units keys for geographic coordinate systems affect only the user defined ellipse major and minor axes.
                if (IsFatalGeoTiffError (status = ProcessLinearUnitsKey (geoKey, false, true)))
                    return status;
                break;

            case GeogLinearUnitSizeGeoKey:
                // Even if GeogLinearUnitsGeoKey is not user-defined the Unit Size if specified (this should be an error)
                // will be applied
                if (IsFatalGeoTiffError (status = ProcessLinearUnitsSizeKey (geoKey, false, true)))
                    return status;
                break;

            case GeogAngularUnitsGeoKey:
                if (IsFatalGeoTiffError (status = ProcessAngularUnitsKey (geoKey, m_userDefinedGeoCS)))
                    return status;
                break;

            case GeogAngularUnitSizeGeoKey:
                if (IsFatalGeoTiffError (status = ProcessAngularUnitsSizeKey (geoKey, m_userDefinedGeoCS)))
                    return status;
                break;

            case GeogEllipsoidGeoKey:
                if (IsFatalGeoTiffError (status = ProcessEllipsoidKey (geoKey)))
                    return status;
                break;

            case GeogSemiMajorAxisGeoKey:
                if (IsFatalGeoTiffError (status = ProcessSemiMajorAxisKey (geoKey)))
                    return status;
                break;

            case GeogSemiMinorAxisGeoKey:
                if (IsFatalGeoTiffError (status = ProcessSemiMinorAxisKey (geoKey)))
                    return status;
                break;

            case GeogInvFlatteningGeoKey:
                if (IsFatalGeoTiffError (status = ProcessInvFlatteningKey (geoKey)))
                    return status;
                break;

            case GeogAzimuthUnitsGeoKey:
                if (IsFatalGeoTiffError (status = ProcessAzimuthUnitsKey (geoKey)))
                    return status;
                break;

            case GeogPrimeMeridianLongGeoKey:
                if (IsFatalGeoTiffError (status = ProcessPrimeMeridianLongitudeKey (geoKey)))
                    return status;
                break;

            case ProjectedCSTypeGeoKey:
            case ProjectedCSTypeGeoKeyLong:
                if (IsFatalGeoTiffError (status = ProcessProjectedCSTypeKey (geoKey)))
                    return status;
                break;

            case PCSCitationGeoKey:
                if (IsFatalGeoTiffError (status = ProcessPCSCitationKey (geoKey)))
                    return status;
                break;

            case ProjectionGeoKey:
                if (IsFatalGeoTiffError (status = ProcessProjectionKey (geoKey)))
                    return status;
                break;

            case ProjCoordTransGeoKey:
                if (IsFatalGeoTiffError (status = ProcessCoordTransKey (geoKey)))
                    return status;
                break;

            case ProjLinearUnitsGeoKey:

                if (IsFatalGeoTiffError (status = ProcessLinearUnitsKey (geoKey, true, allowUnitsOverride)))
                    return status;
                break;

            case ProjLinearUnitSizeGeoKey:
                // Even if GeogLinearUnitsGeoKey is not user-defined the Unit Size if specified (this should be an error)
                // will be applied
                if (IsFatalGeoTiffError (status = ProcessLinearUnitsSizeKey (geoKey, true, allowUnitsOverride)))
                    return status;
                break;

            case ProjStdParallel1GeoKey:
            case ProjStdParallel2GeoKey:
                if (IsFatalGeoTiffError (status = ProcessStandardParallelKey (geoKey, ProjStdParallel1GeoKey == geoKey.KeyID)))
                    return status;
                break;

            case ProjNatOriginLongGeoKey:
            case ProjNatOriginLatGeoKey:
                if (IsFatalGeoTiffError (status = ProcessOriginOrCenterLLKey (geoKey, ProjNatOriginLongGeoKey == geoKey.KeyID)))
                    return status;
                break;

            case ProjFalseEastingGeoKey:
            case ProjFalseNorthingGeoKey:
                if (IsFatalGeoTiffError (status = ProcessFalseENKey (geoKey, ProjFalseEastingGeoKey == geoKey.KeyID)))
                    return status;
                break;

            case ProjFalseOriginLongGeoKey:
            case ProjFalseOriginLatGeoKey:
                if (IsFatalGeoTiffError (status = ProcessOriginOrCenterLLKey (geoKey, ProjFalseOriginLongGeoKey == geoKey.KeyID)))
                    return status;
                break;

            case ProjFalseOriginEastingGeoKey:
            case ProjFalseOriginNorthingGeoKey:
                if (IsFatalGeoTiffError (status = ProcessFalseENKey (geoKey, ProjFalseOriginEastingGeoKey == geoKey.KeyID)))
                    return status;
                break;

            case ProjCenterLongGeoKey:
            case ProjCenterLatGeoKey:
                if (IsFatalGeoTiffError (status = ProcessOriginOrCenterLLKey (geoKey, ProjCenterLongGeoKey == geoKey.KeyID)))
                    return status;
                break;

            case ProjCenterEastingGeoKey:
            case ProjCenterNorthingGeoKey:
                if (IsFatalGeoTiffError (status = ProcessFalseENKey (geoKey, ProjCenterEastingGeoKey == geoKey.KeyID)))
                    return status;
                break;

            case ProjScaleAtNatOriginGeoKey:
                if (IsFatalGeoTiffError (status = ProcessScaleAtNatOriginKey (geoKey)))
                    return status;
                break;

            case ProjScaleAtCenterGeoKey:
                if (IsFatalGeoTiffError (status = ProcessScaleAtCenterKey (geoKey)))
                    return status;
                break;

            case ProjAzimuthAngleGeoKey:
                if (IsFatalGeoTiffError (status = ProcessAzimuthAngleKey (geoKey)))
                    return status;
                break;

            case ProjStraightVertPoleLongGeoKey:
                if (IsFatalGeoTiffError (status = ProcessStraightVertPoleLongKey (geoKey)))
                    return status;
                break;

            // I don't know what to do with these.
            case VerticalCSTypeGeoKey:
                if (IsFatalGeoTiffError (status = ProcessVerticalCSTypeKey (geoKey)))
                    return status;
                break;

            // The three following are simply ignored.
            case VerticalCitationGeoKey: // This is informative only
            case VerticalDatumGeoKey:    // missdefinition of standard ... may conflict with VerticalCSType
            case VerticalUnitsGeoKey:    // BaseGCS cannot have vertical units different than horizontal units (meters imposed for lat/long)
                break;
            }
        }

    // here we have gone through all of the keys, without error. We have to construct the coordinate system from
    // m_csDef, m_datumDef, and m_ellipsoidDef.
    if (!m_userDefinedGeoCS && !m_userDefinedProjectedCS)
        {
        if (!m_haveCS)
            return GEOCOORDERR_CoordSysSpecificationIncomplete;

        if (NULL == (outGCS.m_csParameters = CSMap::CScsloc1 (&m_csDef)))
            return cs_Error;

        outGCS.m_sourceLibrary = LibraryManager::Instance()->GetSystemLibrary();

        }
    else
        {
        if (!m_haveEllipsoid)
            {
            if (!m_haveDatum)
                return GEOCOORDERR_CoordSysSpecificationIncomplete;

            // fill in the ellipsoid from the keyname in the datum definition.
            CSEllipsoidDef* ellipsoidDef;
            if (NULL == (ellipsoidDef = CSMap::CS_eldef (m_csDatumDef.ell_knm)))
                return cs_Error;

            m_csEllipsoidDef = *ellipsoidDef;
            m_haveEllipsoid  = true;
            CSMap::CS_free (ellipsoidDef);
            }

        //In the case of a projection model, now that the ellipsoid and datum
        //have been found, check that a projection has been at least specified
        if ( (m_userDefinedProjectedCS == true) && (0 == m_coordSys) )
            return GEOCOORDERR_CoordSysSpecificationIncomplete;

        // try to get the csParameters
        // It seems to me that CSCsloc2 could copy the datum and ellipse names from m_csDatumDef and m_csEllipsoidDef to m_csDef, but for some reason, it doesn't,
        //   and the CS_cschk function fails when they're not in there.
        if (m_haveDatum)
            CS_stncp (m_csDef.dat_knm, m_csDatumDef.key_nm, sizeof(m_csDef.dat_knm));
        CS_stncp (m_csDef.elp_knm, m_csEllipsoidDef.key_nm, sizeof(m_csDef.elp_knm));

        if (NULL == (outGCS.m_csParameters = CSMap::CScsloc2 (&m_csDef, &m_csDatumDef, &m_csEllipsoidDef)))
            return cs_Error;

        outGCS.m_sourceLibrary = LibraryManager::Instance()->GetSystemLibrary();

        }

    // Now we set the vertical datum regardless GCS is user-defined or not.
    outGCS.SetVerticalDatumCode(m_verticalDatum);

    return SUCCESS;
    }

private:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    barry.bentley                   04/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool                IsFatalGeoTiffError (StatusInt  status)
    {
    if (status == SUCCESS)
        return false;

    // Don't consider unnecessary parameter or redundant parameter an error.
    if ( (status == GEOCOORDERR_CoordParamNotNeededForTrans) || (status == GEOCOORDERR_CoordParamRedundant))
        {
        // catch during debugging
        BeAssert (true);
        return false;
        }

    return true;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessModelTypeKey (IGeoTiffKeysList::GeoKeyItem& geoKey)
    {
    BeAssert (IGeoTiffKeysList::LONG == geoKey.KeyDataType);
    m_modelType = (GeoKeyModelType) geoKey.KeyValue.LongVal;

    switch (m_modelType)
        {
        case ModelTypeProjected:
        case ModelTypeGeographic:
            break;

        // we don't support
        case ModelTypeGeocentric:
            BeAssert (false);
            return GEOCOORDERR_GeocentricNotSupported;

        default:
            BeAssert (false);
            return GEOCOORDERR_UnexpectedGeoTiffModelType;
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessGeographicTypeKey (IGeoTiffKeysList::GeoKeyItem& geoKey)
    {
    BeAssert (IGeoTiffKeysList::LONG == geoKey.KeyDataType);
    BeAssert ( (ModelTypeProjected == m_modelType) || (ModelTypeGeographic == m_modelType) );

    int     geoCode = geoKey.KeyValue.LongVal;
    BeAssert ( ((geoCode >= 4000) && (geoCode < 5000)) ||
             ((geoCode >= UserDefinedKeyValue) && (geoCode <= USHRT_MAX)) );

    // NOTE: The GeographicTypeGeoKey gives us a Datum or Ellipsoid, and a prime meridian.
    //       That's what you need for a LL coordinate system in CS_Map.
    char    coordSysName[128];
    if (geoCode < 5000)
        {
        enum EcsMapSt csMapSt;
        coordSysName[0] = '\0';

        csMapSt = csMapIdToNameC (csMapGeographicCSysKeyName,
                                  coordSysName,
                                  sizeof (coordSysName),
                                  csMapFlvrCsMap,
                                  csMapFlvrEpsg,
                                  static_cast<uint32_t>(geoCode));
        if (csMapSt != csMapOk)
            {
            // try a name based on the EPSG number
            sprintf (coordSysName, "EPSG:%d", geoCode);
            }
        }
    else if ((UserDefinedKeyValue <= geoCode) && (USHRT_MAX >= geoCode))
        {
        // set up for user defined Geographic Coordinate System.
        // initialize for LatLong by looking up the "LL" coordinate system.
        strcpy (coordSysName, "LL");
        m_userDefinedGeoCS = true;

        //Don't use a default Geographic coordinate system
        return SUCCESS;
        }

    CSDefinition* csDef;
    if (NULL == (csDef = CSMap::CS_csdef (coordSysName)))
        return cs_Error;
    //If the model type is projected, GeographicTypeGeoKey should be used only
    //to get the datum.
    if (m_modelType == ModelTypeProjected)
        {
        CSDatumDef* datumDef;
        if (NULL == (datumDef = CSMap::CS_dtdef (csDef->dat_knm)))
            {
            //Try with the ellipsoid name
            if (csDef->elp_knm[0] != 0)
                {
                CSEllipsoidDef* ellipsoidDef;
                if (NULL == (ellipsoidDef = CSMap::CS_eldef (csDef->elp_knm)))
                    return cs_Error;
                else
                    {
                    // copy and free the ellipsoid.
                    m_csEllipsoidDef = *ellipsoidDef;
                    m_haveEllipsoid  = true;

                    CSMap::CS_free (ellipsoidDef);
                    }
                }
            else
                {
                return cs_Error;
                }
            }
        else
            {
            // copy and free the datum.
            m_csDatumDef    = *datumDef;
            m_haveDatum     = true;
            CSMap::CS_free (datumDef);
            }
        }
    else
        {
        m_csDef     = *csDef;
        m_haveCS    = true;
        }

    CSMap::CS_free (csDef);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessGeographicCitationKey (IGeoTiffKeysList::GeoKeyItem& geoKey)
    {
    BeAssert (IGeoTiffKeysList::ASCII == geoKey.KeyDataType);
    BeAssert ( (ModelTypeProjected == m_modelType) || (ModelTypeGeographic == m_modelType) );

    // if it's not a user defined ellipsoid, we have no use for the citation, since we're looking up the coordinate system.
    if (m_userDefinedGeoCS)
        {
        CS_stncp (m_csEllipsoidDef.source, geoKey.KeyValue.StringVal, sizeof(m_csEllipsoidDef.source));
        char MagicString[] = "IMAGINE GeoTIFF Support";

        if (strncmp(MagicString, geoKey.KeyValue.StringVal, strlen(MagicString)) == 0)
            {
            char        seps[]   = "\t\n\r";
            char*       citationCopy = new char[strlen(geoKey.KeyValue.StringVal) + 1];

            strcpy(citationCopy, geoKey.KeyValue.StringVal);

            char* line = strtok(citationCopy, seps);

            while (line != NULL)
                {
                // While there are tokens in "string"
                if (strstr(line, "Ellipsoid") != NULL)
                    {
                    char* value = strchr(line, '=');

                    if (value != NULL)
                        {
                        CSEllipsoidDef* ellipsoidDef;

                        //Names in the ellipsoid table have no white space.
                        stripWhite(++value);

                        if (NULL != (ellipsoidDef = CSMap::CS_eldef (value)))
                            {
                            // copy and free the datum.
                            m_csEllipsoidDef    = *ellipsoidDef;
                            m_haveEllipsoid     = true;
                            CSMap::CS_free (ellipsoidDef);
                            }
                        }
                    }
                else
                if (strstr(line, "Datum") != NULL)
                    {
                    char* value = strchr(line, '=');

                    if (value != NULL)
                        {
                        CSDatumDef* datumDef;
                        char        datumName[50];

                        //Trim any leading and trailing white space.
                        CS_trim(++value);

                        if (NULL == (datumDef = CSMap::CS_dtdef (value)))
                            {
#if defined (GEOCOORD_ENHANCEMENT)
                            if (CS_wktDatumLookUp(value, datumName))
                                {
                                datumDef = CSMap::CS_dtdef (datumName);
                                }
#endif
                            }

                        if (datumDef != NULL)
                            {
                            // copy and free the datum.
                            m_csDatumDef    = *datumDef;
                            m_haveDatum     = true;
                            CSMap::CS_free (datumDef);
                            }
                        }
                    }
                    // Get next token:
                    line = strtok(NULL, seps);
                }

            delete[] citationCopy;

            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessGeodeticDatumKey (IGeoTiffKeysList::GeoKeyItem& geoKey)
    {
    BeAssert (IGeoTiffKeysList::LONG == geoKey.KeyDataType);
    BeAssert ( (ModelTypeProjected == m_modelType) || (ModelTypeGeographic == m_modelType) );

    int     geoCode = geoKey.KeyValue.LongVal;
    BeAssert ( ((geoCode >= 6000) && (geoCode < 7000)) || (geoCode == UserDefinedKeyValue) );

    // NOTE: Since the GeodeticDatumKey gives us only a Datum or Ellipsoid rather than a full coordinate
    //       system, that's all we can look up. There might be a prime meridian geoKey later?
    if (geoCode < 6100)
        {
        char        ellipsoidName[128];


        enum EcsMapSt csMapSt;
        ellipsoidName[0] = '\0';

        // We first try the csmap flavor
        csMapSt = csMapIdToNameC (csMapEllipsoidKeyName,
                                  ellipsoidName,
                                  sizeof (ellipsoidName),
                                  csMapFlvrCsMap,
                                  csMapFlvrEpsg,
                                  static_cast<uint32_t>(geoCode + 1000));
        if (csMapSt != csMapOk)
            {
            // Since it failed we try the Autodesk flavor as a lot of IDs were added from this source
            csMapSt = csMapIdToNameC (csMapEllipsoidKeyName,
                                      ellipsoidName,
                                      sizeof (ellipsoidName),
                                      csMapFlvrAutodesk,
                                      csMapFlvrEpsg,
                                      static_cast<uint32_t>(geoCode + 1000));
            }
        if (csMapSt != csMapOk)

            {
            // try a name base on the EPSG number
            sprintf (ellipsoidName, "EPSG:%d", geoCode+1000);
            }

        CSEllipsoidDef* ellipsoidDef;
        if (NULL == (ellipsoidDef = CSMap::CS_eldef (ellipsoidName)))
            return cs_Error;

        m_csEllipsoidDef = *ellipsoidDef;
        m_haveEllipsoid  = true;
        CSMap::CS_free (ellipsoidDef);
        }
    if (geoCode < 7000)
        {
        char        datumName[128];

        enum EcsMapSt csMapSt;
        datumName[0] = '\0';

        // We first try the csmap flavor
        csMapSt = csMapIdToNameC (csMapDatumKeyName,
                                  datumName,
                                  sizeof (datumName),
                                  csMapFlvrCsMap,
                                  csMapFlvrEpsg,
                                  static_cast<uint32_t>(geoCode));
        if (csMapSt != csMapOk)
            {
            // Since it failed we try the Autodesk flavor as a lot of IDs were added from this source
            csMapSt = csMapIdToNameC (csMapDatumKeyName,
                                      datumName,
                                      sizeof (datumName),
                                      csMapFlvrAutodesk,
                                      csMapFlvrEpsg,
                                      static_cast<uint32_t>(geoCode));
            }
 
        if (csMapSt != csMapOk)
            {
            // try a name based on the EPSG number
            sprintf (datumName, "EPSG:%d", geoCode);
            }

        CSDatumDef* datumDef;
        if (NULL == (datumDef = CSMap::CS_dtdef (datumName)))
            return cs_Error;

        // copy and free the datum.
        m_csDatumDef    = *datumDef;
        m_haveDatum     = true;
        CSMap::CS_free (datumDef);
        }
    else if (UserDefinedKeyValue == geoCode)
        {
        // set up for user defined Geographic Coordinate System
        m_userDefinedGeoCS = true;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessPrimeMeridianKey (IGeoTiffKeysList::GeoKeyItem& geoKey)
    {
    BeAssert (IGeoTiffKeysList::LONG == geoKey.KeyDataType);
    BeAssert ( (ModelTypeProjected == m_modelType) || (ModelTypeGeographic == m_modelType) );

    int     geoCode = geoKey.KeyValue.LongVal;
    BeAssert ( ((geoCode >= 8900) && (geoCode < 9000)) || (geoCode != UserDefinedKeyValue) );

    switch (geoCode)
        {
        case PM_Greenwich:
            m_csDef.org_lng = 0.0;
            break;
        case PM_Lisbon:
            m_csDef.org_lng = -9.0754862;
            break;
        case PM_Paris:
            m_csDef.org_lng = 2.337229167;
            break;
        case PM_Bogota:
            m_csDef.org_lng = -74.04513;
            break;
        case PM_Madrid:
            m_csDef.org_lng = -3.411658;
            break;
        case PM_Rome:
            m_csDef.org_lng = 12.27084;
            break;
        case PM_Bern:
            m_csDef.org_lng = 7.26225;
            break;
        case PM_Jakarta:
            m_csDef.org_lng = 106.482779;
            break;
        case PM_Ferro:
            m_csDef.org_lng = -17.4;
            break;
        case PM_Brussels:
            m_csDef.org_lng = 4.220471;
            break;
        case PM_Stockholm:
            m_csDef.org_lng = 18.03298;
            break;
        case PM_Athens:
            m_csDef.org_lng = 23.4258815;
            break;
        case PM_Oslo:
            m_csDef.org_lng = 10.43225;
            break;
        case UserDefinedKeyValue:
            // we'll expect a PrimeMeridianLongGeoKey then.
            break;
        default:
            return GEOCOORDERR_UnexpectedGeoTiffPrimeMeridian;
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessPrimeMeridianLongitudeKey (IGeoTiffKeysList::GeoKeyItem& geoKey)
    {
    BeAssert (IGeoTiffKeysList::DOUBLE == geoKey.KeyDataType);
    BeAssert ( (ModelTypeProjected == m_modelType) || (ModelTypeGeographic == m_modelType) );

    double geoValue = geoKey.KeyValue.DoubleVal * m_angularUnitsToDegrees;
    m_csDef.org_lng = geoValue;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessEllipsoidKey (IGeoTiffKeysList::GeoKeyItem& geoKey)
    {
    BeAssert (IGeoTiffKeysList::LONG == geoKey.KeyDataType);
    BeAssert ( (ModelTypeProjected == m_modelType) || (ModelTypeGeographic == m_modelType) );

    int     geoCode = geoKey.KeyValue.LongVal;
    BeAssert ( ((geoCode >= 7000) && (geoCode < 8000)) || (geoCode == UserDefinedKeyValue) );


    if (geoCode != UserDefinedKeyValue)
        {
        // look up the ellipsoid. Name will be "EPSG:%d".
        char    ellipsoidName[64];
        sprintf (ellipsoidName, "EPSG:%d", geoCode);

        CSEllipsoidDef* ellipsoidDef;
        if (NULL == (ellipsoidDef = CSMap::CS_eldef (ellipsoidName)))
            return cs_Error;

        // copy and free the ellipsoid
        m_csEllipsoidDef = *ellipsoidDef;
        m_haveEllipsoid  = true;
        CSMap::CS_free (ellipsoidDef);
        }
    else
        {
        // set up for user defined Geographic Coordinate System
        m_userDefinedGeoCS = true;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessLinearUnitsKey (IGeoTiffKeysList::GeoKeyItem& geoKey, bool projectedCS, bool allowUnitsOverride)
    {
    // Even though the allowUnitsOverride is false and the GCS user defined we will store the linear unit definiton
    // for the interpretation of the ellispoid dimension yet we will not change the current CS definition unless it is
    // not a predefined GCS. (user defined GCS will have units applied)

    BeAssert (IGeoTiffKeysList::LONG == geoKey.KeyDataType);
    BeAssert ( (ModelTypeProjected == m_modelType) || (ModelTypeGeographic == m_modelType) );

    int     geoCode = geoKey.KeyValue.LongVal;
    BeAssert ( ((geoCode >= 9000) && (geoCode < 9100)) || (geoCode != UserDefinedKeyValue) );

    const struct cs_Unittab_  *pUnit;
    for (pUnit = cs_Unittab; cs_UTYP_END != pUnit->type; pUnit++)
        {
        if ( (pUnit->type == cs_UTYP_LEN) && (geoCode == pUnit->epsgCode) )
            {
            // put the unit info into csDef. Probably all we need is the name.
            // In order to apply to the cs definition we expect the cs to be user defined
            // or that the unit override be allowed otherwise we use the units
            // directly related to the definition of the ProjectCSType key (EPSG code)
            if (projectedCS && (allowUnitsOverride || !m_haveCS || m_userDefinedProjectedCS))
                {
                // convert the offsets to the new unit.
                double oldToNewUnitConvFactor = m_csDef.unit_scl / pUnit->factor;
                m_csDef.unit_scl = pUnit->factor;
                m_csDef.x_off *= oldToNewUnitConvFactor;
                m_csDef.y_off *= oldToNewUnitConvFactor;
                CS_stncp (m_csDef.unit, pUnit->name, sizeof(m_csDef.unit));

                // If the GCS is keyname based or has a keyname set ...
                if (strlen(m_csDef.key_nm) > 0)
                    {
                    // Change the keyname and description since the dictionary entry and changed GCS do not fit
                    // Remove keyname
                    CS_stncp (m_csDef.key_nm, "", DIM(m_csDef.key_nm));

                    // Append unit name at the end of description (which unfortunately often contain the unit name)
                    char  proposedDescription[1024];
                    sprintf (proposedDescription, "%s - %s", m_csDef.desc_nm, pUnit->name);

                    // make sure the description is not too long.
                    proposedDescription[63] = 0;
                    CS_stncp(m_csDef.desc_nm, proposedDescription, DIM(m_csDef.desc_nm));
                    }
                }
            m_linearUnitsToMeters = pUnit->factor;
            return SUCCESS;
            }
        }
    return GEOCOORDERR_UnrecognizedLinearUnit;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessLinearUnitsSizeKey (IGeoTiffKeysList::GeoKeyItem& geoKey, bool projectedCS, bool allowUnitsOverride)
    {
    // Even though the allowUnitsOverride is false and the GCS user defined we will store the linear unit definiton
    // for the interpretation of the ellispoid dimension yet we will not change the current CS definition unless it is
    // not a predefined GCS. (user defined GCS will have units applied)

    BeAssert (IGeoTiffKeysList::DOUBLE == geoKey.KeyDataType);
    BeAssert ( (ModelTypeProjected == m_modelType) || (ModelTypeGeographic == m_modelType) );

    double geoValue = geoKey.KeyValue.DoubleVal;
    m_csDef.unit_scl = geoValue;

    const struct cs_Unittab_  *pUnit;
    for (pUnit = cs_Unittab; cs_UTYP_END != pUnit->type; pUnit++)
        {
        if ( (pUnit->type == cs_UTYP_LEN) && doubleSame (pUnit->factor, geoValue) )
            {
            // In order to apply to the cs definition we expect the cs to be user defined
            // or that the unit override be allowed otherwise we use the units
            // directly related to the definition of the ProjectCSType key (EPSG code)
            if (projectedCS && (allowUnitsOverride || !m_haveCS || m_userDefinedProjectedCS))
                {
                // convert the offsets to the new unit.
                double oldToNewUnitConvFactor = m_csDef.unit_scl / geoValue;
                m_csDef.unit_scl = geoValue;
                m_csDef.x_off *= oldToNewUnitConvFactor;
                m_csDef.y_off *= oldToNewUnitConvFactor;

                // put the unit info into csDef. Probably all we need is the name.
                CS_stncp (m_csDef.unit, pUnit->name, sizeof(m_csDef.unit));
                }
            m_linearUnitsToMeters = geoValue;
            return SUCCESS;
            }
        }

    return GEOCOORDERR_UnrecognizedLinearUnit;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessAngularUnitsKey (IGeoTiffKeysList::GeoKeyItem& geoKey, bool isGeographicCS)
    {
    BeAssert (IGeoTiffKeysList::LONG == geoKey.KeyDataType);
    BeAssert ( (ModelTypeProjected == m_modelType) || (ModelTypeGeographic == m_modelType) );

    int     geoCode = geoKey.KeyValue.LongVal;
    BeAssert ( ((geoCode >= 9100) && (geoCode < 9200)) || (geoCode != UserDefinedKeyValue) );

    const struct cs_Unittab_  *pUnit;
    for (pUnit = cs_Unittab; cs_UTYP_END != pUnit->type; pUnit++)
        {
        if ( (pUnit->type == cs_UTYP_ANG) && (geoCode == pUnit->epsgCode) )
            {
            m_angularUnitsToDegrees = pUnit->factor;

            // If it's a Geographic (Lat/Long) CS, put the unit name into the coordinate system unit name.
            if (isGeographicCS)
                CS_stncp (m_csDef.unit, pUnit->name, sizeof(m_csDef.unit));
            return SUCCESS;
            }
        }
    return GEOCOORDERR_UnrecognizedAngularUnit;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessAngularUnitsSizeKey (IGeoTiffKeysList::GeoKeyItem& geoKey, bool isGeographicCS)
    {
    BeAssert (IGeoTiffKeysList::DOUBLE == geoKey.KeyDataType);
    BeAssert ( (ModelTypeProjected == m_modelType) || (ModelTypeGeographic == m_modelType) );

    double geoValue = geoKey.KeyValue.DoubleVal;
    const struct cs_Unittab_  *pUnit;

    // according to the GeoTiff documentation, this is in radians, but CSMap works in degrees, so convert it.
    geoValue = BaseGCS::DegreesFromRadians (geoValue);

    for (pUnit = cs_Unittab; cs_UTYP_END != pUnit->type; pUnit++)
        {
        if ( (pUnit->type == cs_UTYP_ANG) && doubleSame (geoValue, pUnit->factor) )
            {
            m_angularUnitsToDegrees = geoValue;
            if (isGeographicCS)
                {
                // If it's a Geographic (Lat/Long) CS, put the unit name into the coordinate system unit name.
                if (isGeographicCS)
                    CS_stncp (m_csDef.unit, pUnit->name, sizeof(m_csDef.unit));
                }
            return SUCCESS;
            }
        }
    return GEOCOORDERR_UnrecognizedAngularUnit;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessSemiMajorAxisKey (IGeoTiffKeysList::GeoKeyItem& geoKey)
    {
    BeAssert (IGeoTiffKeysList::DOUBLE == geoKey.KeyDataType);
    BeAssert ( (ModelTypeProjected == m_modelType) || (ModelTypeGeographic == m_modelType) );

    double geoValue = geoKey.KeyValue.DoubleVal * m_linearUnitsToMeters;

    // put value into the ellipse definition.
    m_csEllipsoidDef.e_rad = geoValue;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessSemiMinorAxisKey (IGeoTiffKeysList::GeoKeyItem& geoKey)
    {
    BeAssert (IGeoTiffKeysList::DOUBLE == geoKey.KeyDataType);
    BeAssert ( (ModelTypeProjected == m_modelType) || (ModelTypeGeographic == m_modelType) );

    double geoValue = geoKey.KeyValue.DoubleVal * m_linearUnitsToMeters;
    // put value into the ellipse definition.
    m_csEllipsoidDef.p_rad = geoValue;

    if (Ellipsoid::CalculateParameters (m_csEllipsoidDef.flat, m_csEllipsoidDef.ecent, m_csEllipsoidDef.e_rad, m_csEllipsoidDef.p_rad))
        m_haveEllipsoid = true;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessInvFlatteningKey (IGeoTiffKeysList::GeoKeyItem& geoKey)
    {
    BeAssert (IGeoTiffKeysList::DOUBLE == geoKey.KeyDataType);
    BeAssert ( (ModelTypeProjected == m_modelType) || (ModelTypeGeographic == m_modelType) );

    double geoValue = geoKey.KeyValue.DoubleVal;
    m_csEllipsoidDef.flat = 1.0 / geoValue;
    if (0 != m_csEllipsoidDef.e_rad)
        {
        m_csEllipsoidDef.p_rad  = m_csEllipsoidDef.e_rad * (1.0 - m_csEllipsoidDef.flat);
        Ellipsoid::CalculateParameters (m_csEllipsoidDef.flat, m_csEllipsoidDef.ecent, m_csEllipsoidDef.e_rad, m_csEllipsoidDef.p_rad);
        return SUCCESS;
        }
    else if (0 != m_csEllipsoidDef.p_rad)
        {
        m_csEllipsoidDef.e_rad  = m_csEllipsoidDef.p_rad / (1.0 - m_csEllipsoidDef.flat);
        if (Ellipsoid::CalculateParameters (m_csEllipsoidDef.flat, m_csEllipsoidDef.ecent, m_csEllipsoidDef.e_rad, m_csEllipsoidDef.p_rad))
            m_haveEllipsoid         = true;
        return SUCCESS;
        }
    else
        return GEOCOORDERR_BadEllipsoidDefinition;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessAzimuthUnitsKey (IGeoTiffKeysList::GeoKeyItem& geoKey)
    {
    BeAssert (IGeoTiffKeysList::LONG == geoKey.KeyDataType);
    BeAssert ( (ModelTypeProjected == m_modelType) || (ModelTypeGeographic == m_modelType) );

    int     geoCode = geoKey.KeyValue.LongVal;
    BeAssert ( ((geoCode >= 9100) && (geoCode < 9200)) || (geoCode != UserDefinedKeyValue) );

    const struct cs_Unittab_  *pUnit;
    for (pUnit = cs_Unittab; cs_UTYP_END != pUnit->type; pUnit++)
        {
        if ( (pUnit->type == cs_UTYP_ANG) && (geoCode == pUnit->epsgCode) )
            {
            m_azimuthUnitsToDegrees = pUnit->factor;
            return SUCCESS;
            }
        }
    return GEOCOORDERR_UnrecognizedAngularUnit;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessProjectedCSTypeKey (IGeoTiffKeysList::GeoKeyItem& geoKey)
    {
    BeAssert (IGeoTiffKeysList::LONG == geoKey.KeyDataType);
    BeAssert (ModelTypeProjected == m_modelType);

    int     geoCode = geoKey.KeyValue.LongVal;


    // Code 0 is a GeoTIFF code for undefined yet the CSMAP lookup process uses code 0 for deprecated entries
    // using code 0 will simply return the first EPSG deprecated entry (which is usually PulkovoGK/CM-15E)
    // which is meaningless.
    if (UnDefinedKeyValue == geoCode)
        return SUCCESS;

    // look up the coordinate system
    if (UserDefinedKeyValue != geoCode)
        {
        char    coordSysName[128];

        enum EcsMapSt csMapSt;
        coordSysName[0] = '\0';

        // Attention! The use of csMapProjGeoCSys instead of csMapGeographicCSysKeyName is intentional here
        // as we use this function to process geographic lat/long coordinate systems sometimes during GeoTIFF key generation
        // even if not stored in the ProjectedCSKey. Notice that it is historically likely users create
        // GeoTIFF files with a lat/long GCS identifier in the ProjectedGeoKey
        csMapSt = csMapIdToNameC (csMapProjGeoCSys,
                                  coordSysName,
                                  sizeof (coordSysName),
                                  csMapFlvrCsMap,
                                  csMapFlvrEpsg,
                                  static_cast<uint32_t>(geoCode));
        if (csMapSt != csMapOk)

            {
            // try a name based on the EPSG number
            sprintf (coordSysName, "EPSG:%d", geoCode);
            }

        CSDefinition* csDef;
        if (NULL == (csDef = CSMap::CS_csdef (coordSysName)))
            return cs_Error;
        // copy and free
        m_csDef     = *csDef;
        m_haveCS    = true;
        CSMap::CS_free (csDef);
        }
    else
        {
        // set up for user defined Geographic Coordinate System.
        m_userDefinedProjectedCS = true;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessPCSCitationKey (IGeoTiffKeysList::GeoKeyItem& geoKey)
    {
    BeAssert (IGeoTiffKeysList::ASCII == geoKey.KeyDataType);
    BeAssert (ModelTypeProjected == m_modelType);

    static ProjTransInCitation projTransInCitation[] = {
        //From GCoord
        {COORDSYS_EKRT4, CS_EKRT4, "ECKERT IV"}                      ,
        {COORDSYS_BONNE, CS_BONNE, "BONNE"}                          ,
        {0, "", ""}
    };

    // if it's not a user defined projection, we have no use for the citation, since we're looking up the coordinate system.
    if (m_userDefinedProjectedCS)
        {
        CS_stncp (m_csDef.source, geoKey.KeyValue.StringVal, sizeof(m_csDef.source));

        bool isFound = FALSE;
        int  methIter = 0;

        while (!isFound && projTransInCitation[methIter].m_csCoordSys > 0)
            {
            if ((strlen (projTransInCitation[methIter].m_nameInCitation) < strlen(m_csDef.source)) &&
                0 == BeStringUtilities::Strnicmp (m_csDef.source, projTransInCitation[methIter].m_nameInCitation, strlen (projTransInCitation[methIter].m_nameInCitation)))
                {
                m_coordSys = projTransInCitation[methIter].m_csCoordSys;
                CS_stncp (m_csDef.prj_knm, projTransInCitation[methIter].m_csName, sizeof(m_csDef.prj_knm));
                return SUCCESS;
                }
            methIter++;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessProjectionKey (IGeoTiffKeysList::GeoKeyItem& geoKey)
    {
    BeAssert (IGeoTiffKeysList::LONG == geoKey.KeyDataType);

    int geoValue      = geoKey.KeyValue.LongVal;

    // Imagepp was setting this dummy value (16001) in the past when ProjectionCS is User Defined.
    // It now set UserDefinedKeyValue.
    // In both cases, we want to accept these as valid since ArcGIS seems to required these values.
    if ((geoValue == UserDefinedKeyValue) || (geoValue == 16001))
        return SUCCESS;

    return GEOCOORDERR_ProjectionGeoKeyNotSupported;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessCoordTransKey (IGeoTiffKeysList::GeoKeyItem& geoKey)
    {
    BeAssert (IGeoTiffKeysList::LONG == geoKey.KeyDataType);
    BeAssert (ModelTypeProjected == m_modelType);

    int         geoCode      = geoKey.KeyValue.LongVal;
    const char *csProjection = NULL;

    // look for CS_Map projection
    switch (geoCode)
        {
        case CT_TransverseMercator:
            csProjection = CS_TRMER;
            m_coordSys   = COORDSYS_TRMER;
            break;

        case CT_TransvMercator_Modified_Alaska:
            // we don't support this one.
            break;

        case CT_ObliqueMercator:
            csProjection = CS_OBLQ1;
            m_coordSys   = COORDSYS_OBLQ1;
            break;

        case CT_ObliqueMercator_Laborde:
            // we don't support this one.
            break;

        case CT_ObliqueMercator_Rosenmund:
            csProjection = CS_SWISS;
            m_coordSys   = COORDSYS_SWISS;
            break;

        case CT_ObliqueMercator_Spherical:
            // we don't support this one.
            break;

        case CT_Mercator: // It could be either Mercator with standard parallel or Mercator with scale reduction depending on the keys offered
            csProjection = CS_MRCSR;
            m_coordSys   = COORDSYS_MRCSR;
            break;

        case CT_LambertConfConic_2SP:
            csProjection = CS_LMBRT;
            m_coordSys   = COORDSYS_LMBRT;
            break;

        case CT_LambertConfConic_Helmert:
            csProjection = CS_LM1SP;
            m_coordSys   = COORDSYS_LM1SP;
            break;

        case CT_LambertAzimEqualArea:
            csProjection = CS_AZMEA;
            m_coordSys   = COORDSYS_AZMEA;
            break;

        case CT_AlbersEqualArea:
            csProjection = CS_ALBER;
            m_coordSys   = COORDSYS_ALBER;
            break;

        case CT_AzimuthalEquidistant:
            csProjection = CS_AZMED;
            m_coordSys   = COORDSYS_AZMED;
            break;

        case CT_EquidistantConic:
            csProjection = CS_EDCNC;
            m_coordSys   = COORDSYS_EDCNC;
            break;

        case CT_Stereographic:
            csProjection = CS_STERO;
            m_coordSys   = COORDSYS_STERO;
            break;

        case CT_PolarStereographic:
            csProjection = CS_PSTRO;
            m_coordSys   = COORDSYS_PSTRO;
            break;

        case CT_ObliqueStereographic:
            csProjection = CS_OSTRO;
            m_coordSys   = COORDSYS_OSTRO;
            break;

        case CT_Equirectangular:
            csProjection = CS_EDCYL;
            m_coordSys   = COORDSYS_EDCYL;
            break;

        case CT_CassiniSoldner:
            csProjection = CS_CSINI;
            m_coordSys   = COORDSYS_CSINI;
            break;

        case CT_Gnomonic:
            csProjection = CS_GNOMC;
            m_coordSys   = COORDSYS_GNOMC;
            break;

        case CT_MillerCylindrical:
            csProjection = CS_MILLR;
            m_coordSys   = COORDSYS_MILLR;
            break;

        case CT_Orthographic:
            csProjection = CS_ORTHO;
            m_coordSys   = COORDSYS_ORTHO;
            break;

        case CT_Polyconic:
            csProjection = CS_PLYCN;
            m_coordSys   = COORDSYS_PLYCN;
            break;

        case CT_Robinson:
            csProjection = CS_ROBIN;
            m_coordSys   = COORDSYS_ROBIN;
            break;

        case CT_Sinusoidal:
            csProjection = CS_SINUS;
            m_coordSys   = COORDSYS_SINUS;
            break;

        case CT_VanDerGrinten:
            csProjection = CS_VDGRN;
            m_coordSys   = COORDSYS_VDGRN;
            break;

        case CT_NewZealandMapGrid:
            csProjection = CS_NZLND;
            m_coordSys   = COORDSYS_NZLND;
            break;

        case CT_TransvMercator_SouthOriented:
            csProjection = CS_SOTRM;
            m_coordSys   = COORDSYS_SOTRM;
            break;

        //The coordinate transformation might have been found in the PCSCitationGeoKey's string.
        case UserDefinedKeyValue:
            return SUCCESS;
        }

    if (NULL != csProjection)
        {
        CS_stncp (m_csDef.prj_knm, csProjection, sizeof(m_csDef.prj_knm));
        return SUCCESS;
        }

    return GEOCOORDERR_CoordTransNotSupported;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessStandardParallelKey (IGeoTiffKeysList::GeoKeyItem& geoKey, bool isFirst)
    {
    BeAssert (IGeoTiffKeysList::DOUBLE == geoKey.KeyDataType);
    BeAssert (ModelTypeProjected == m_modelType);

    double geoValue = geoKey.KeyValue.DoubleVal * m_angularUnitsToDegrees;

    switch (m_coordSys)
        {
        case COORDSYS_LMBRT:
        case COORDSYS_ALBER:
        case COORDSYS_EDCNC:
            if (isFirst)
                m_csDef.prj_prm1 = geoValue;
            else
                m_csDef.prj_prm2 = geoValue;
            return SUCCESS;

        case COORDSYS_EDCYL:
            // need only one standard parallel
            if (isFirst)
                {
                m_csDef.prj_prm1 = geoValue;
                return SUCCESS;
                }
            break;
        case COORDSYS_MRCAT:
            if (isFirst)
                {
                m_csDef.prj_prm2 = geoValue;
                return SUCCESS;
                }
            break;
        case COORDSYS_MRCSR:
            if (isFirst)
                {
                // If we get there it is because we have a standard parallel specified which changes the nature
                // of the mercator projection to Mercator with Standard parallel ... we change the method type
                m_coordSys   = COORDSYS_MRCAT;
                CS_stncp (m_csDef.prj_knm, CS_MRCAT, sizeof(m_csDef.prj_knm));
                m_csDef.prj_prm2 = geoValue;
                return SUCCESS;
                }
            break;

        case COORDSYS_TRMER:
        case COORDSYS_OBLQ1:
        case COORDSYS_LM1SP:
        case COORDSYS_AZMEA:
        case COORDSYS_AZMED:
        case COORDSYS_STERO:
        case COORDSYS_OSTRO:
        case COORDSYS_PSTRO:
        case COORDSYS_CSINI:
        case COORDSYS_GNOMC:
        case COORDSYS_MILLR:
        case COORDSYS_ORTHO:
        case COORDSYS_PLYCN:
        case COORDSYS_ROBIN:
        case COORDSYS_SINUS:
        case COORDSYS_VDGRN:
        case COORDSYS_NZLND:
        case COORDSYS_SOTRM:
            break;
        }
    return GEOCOORDERR_CoordParamNotNeededForTrans;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessOriginOrCenterLLKey (IGeoTiffKeysList::GeoKeyItem& geoKey, bool isLongitude)
    {
    BeAssert (IGeoTiffKeysList::DOUBLE == geoKey.KeyDataType);
    BeAssert (ModelTypeProjected == m_modelType);

    // if a previous key specifed the origin longitude or latitude, simply ignore a repeated attempt to set it.
    if (isLongitude && m_haveUserOriginLongitude)
        return GEOCOORDERR_CoordParamRedundant;
    else if (!isLongitude && m_haveUserOriginLatitude)
        return GEOCOORDERR_CoordParamRedundant;

    double geoValue = geoKey.KeyValue.DoubleVal * m_angularUnitsToDegrees;

    switch (m_coordSys)
        {
        case COORDSYS_ALBER:
        case COORDSYS_AZMEA:
        case COORDSYS_AZMED:
        case COORDSYS_CSINI:
        case COORDSYS_EDCNC:
        case COORDSYS_EDCYL:
        case COORDSYS_GNOMC:
        case COORDSYS_LM1SP:
        case COORDSYS_LMBRT:
        case COORDSYS_NZLND:
        case COORDSYS_ORTHO:
        case COORDSYS_STERO:
        case COORDSYS_OSTRO:
        case COORDSYS_PSTRO:
        case COORDSYS_SWISS:
        //Found in the PCSCitationGeoKey
        case COORDSYS_BONNE:
            if (isLongitude)
                {
                m_haveUserOriginLongitude = true;
                m_csDef.org_lng = geoValue;
                }
            else
                {
                m_haveUserOriginLatitude = true;
                m_csDef.org_lat = geoValue;
                }
            return SUCCESS;

        case COORDSYS_TRMER:
        case COORDSYS_SOTRM:
        case COORDSYS_MILLR:

            if (isLongitude)
                {
                m_haveUserOriginLongitude = true;
                m_csDef.prj_prm1 = geoValue;
                }
             else
                {
                m_haveUserOriginLatitude = true;
                m_csDef.org_lat = geoValue;
                }
            return SUCCESS;
        case COORDSYS_MRCSR:
            if (isLongitude)
                {

                m_haveUserOriginLongitude = true;
                m_csDef.prj_prm1 = geoValue;
                }
            else
                {    // If we get there it is because we have a standard parallel specified which changes the nature
                     // of the mercator projection to Mercator with Standard parallel ... we change the method type
                m_coordSys   = COORDSYS_MRCAT;
                CS_stncp (m_csDef.prj_knm, CS_MRCAT, sizeof(m_csDef.prj_knm));
                m_haveUserOriginLatitude = true;
                m_csDef.prj_prm2 = geoValue;
                }
            return SUCCESS;

        case COORDSYS_OBLQ1:
        case COORDSYS_MRCAT:
            if (isLongitude)
                {
                m_haveUserOriginLongitude = true;
                m_csDef.prj_prm1 = geoValue;
                }
            else
                {
                m_haveUserOriginLatitude = true;
                m_csDef.prj_prm2 = geoValue;
                }
            return SUCCESS;

        case COORDSYS_PLYCN:
            if (isLongitude)
                {
                m_haveUserOriginLongitude = true;
                m_csDef.prj_prm1 = geoValue;
                }
            else
                {
                m_haveUserOriginLatitude = true;
                m_csDef.org_lat  = geoValue;
                }
            return SUCCESS;

        // these take only org_lng, not org_lat.
        case COORDSYS_ROBIN:
        case COORDSYS_SINUS:
        case COORDSYS_VDGRN:
        //Found in the PCSCitationGeoKey
        case COORDSYS_EKRT4:
            if (isLongitude)
                {
                m_haveUserOriginLongitude = true;
                m_csDef.org_lng  = geoValue;
                }
            return SUCCESS;
        }

    return GEOCOORDERR_CoordParamNotNeededForTrans;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessFalseENKey (IGeoTiffKeysList::GeoKeyItem& geoKey, bool isEasting)
    {
    BeAssert (IGeoTiffKeysList::DOUBLE == geoKey.KeyDataType);
    BeAssert (ModelTypeProjected == m_modelType);

    if (isEasting && m_haveFalseEasting)
        return GEOCOORDERR_CoordParamRedundant;
    else if (!isEasting && m_haveFalseNorthing)
        return GEOCOORDERR_CoordParamRedundant;

    double geoValue = geoKey.KeyValue.DoubleVal;

    if (isEasting)
        {
        m_haveFalseEasting  = true;
        m_csDef.x_off       = geoValue;
        }
    else
        {
        m_haveFalseNorthing = true;
        m_csDef.y_off       = geoValue;
        }

    // this is following what the old geocoordinate library did for the ProjCenterEasting/Northing key (check near zero for MRCSR). Not sure where it came from.
    if (COORDSYS_MRCSR == m_coordSys)
        {
        if (fabs (geoValue) >= 1e-6)
            return GEOCOORDERR_ProjectionParamNotSupported;
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessScaleAtNatOriginKey (IGeoTiffKeysList::GeoKeyItem& geoKey)
    {
    BeAssert (IGeoTiffKeysList::DOUBLE == geoKey.KeyDataType);
    BeAssert (ModelTypeProjected == m_modelType);

    double geoValue = geoKey.KeyValue.DoubleVal;

    // always treat this as the scale reduction.
    m_csDef.scl_red = geoValue;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessScaleAtCenterKey (IGeoTiffKeysList::GeoKeyItem& geoKey)
    {
    BeAssert (IGeoTiffKeysList::DOUBLE == geoKey.KeyDataType);
    BeAssert (ModelTypeProjected == m_modelType);

    double geoValue = geoKey.KeyValue.DoubleVal;

    // always treat this as the scale reduction.
    m_csDef.scl_red = geoValue;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessAzimuthAngleKey (IGeoTiffKeysList::GeoKeyItem& geoKey)
    {
    BeAssert (IGeoTiffKeysList::DOUBLE == geoKey.KeyDataType);
    BeAssert (ModelTypeProjected == m_modelType);

    double geoValue = geoKey.KeyValue.DoubleVal * m_azimuthUnitsToDegrees;

    switch (m_coordSys)
        {
        case COORDSYS_OSTRO:
        case COORDSYS_STERO:
        case COORDSYS_PSTRO:        // I don't think the azimuth angle is used for PSTRO.
        case COORDSYS_AZMED:
        case COORDSYS_AZMEA:
            m_csDef.prj_prm1 = geoValue;
            return SUCCESS;

        case COORDSYS_OBLQ1:
            m_csDef.prj_prm3 = geoValue;
            return SUCCESS;

        // none of these need an azimuth.
        case COORDSYS_TRMER:
        case COORDSYS_MRCSR:
        case COORDSYS_LMBRT:
        case COORDSYS_LM1SP:
        case COORDSYS_ALBER:
        case COORDSYS_EDCNC:
        case COORDSYS_EDCYL:
        case COORDSYS_CSINI:
        case COORDSYS_GNOMC:
        case COORDSYS_MILLR:
        case COORDSYS_ORTHO:
        case COORDSYS_PLYCN:
        case COORDSYS_ROBIN:
        case COORDSYS_SINUS:
        case COORDSYS_VDGRN:
        case COORDSYS_NZLND:
        case COORDSYS_SOTRM:
            break;
        }
    return GEOCOORDERR_CoordParamNotNeededForTrans;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessStraightVertPoleLongKey (IGeoTiffKeysList::GeoKeyItem& geoKey)
    {
    BeAssert (IGeoTiffKeysList::DOUBLE == geoKey.KeyDataType);
    BeAssert (ModelTypeProjected == m_modelType);

    double geoValue = geoKey.KeyValue.DoubleVal * m_angularUnitsToDegrees;

    switch (m_coordSys)
        {
        case COORDSYS_PSTRO:
            m_csDef.org_lng = geoValue;
            return SUCCESS;

        // none of these need a vert pole longitude key.
        case COORDSYS_OSTRO:
        case COORDSYS_OBLQ1:
        case COORDSYS_AZMED:
        case COORDSYS_AZMEA:
        case COORDSYS_TRMER:
        case COORDSYS_MRCSR:
        case COORDSYS_LMBRT:
        case COORDSYS_LM1SP:
        case COORDSYS_ALBER:
        case COORDSYS_EDCNC:
        case COORDSYS_EDCYL:
        case COORDSYS_CSINI:
        case COORDSYS_GNOMC:
        case COORDSYS_MILLR:
        case COORDSYS_ORTHO:
        case COORDSYS_PLYCN:
        case COORDSYS_ROBIN:
        case COORDSYS_SINUS:
        case COORDSYS_VDGRN:
        case COORDSYS_NZLND:
        case COORDSYS_SOTRM:
            break;
        }
    return GEOCOORDERR_CoordParamNotNeededForTrans;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alain.Robert                   11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       ProcessVerticalCSTypeKey (IGeoTiffKeysList::GeoKeyItem& geoKey)
    {
    BeAssert (IGeoTiffKeysList::LONG == geoKey.KeyDataType);

    long verticalCSCode = geoKey.KeyValue.LongVal;

	// Values under 5100 are based on ellipsoid (From Datum)
    m_verticalDatum = vdcFromDatum;
    if (verticalCSCode > 5099)
        {
        if (VerticalCSCode::VertCS_North_American_Vertical_Datum_1929 == verticalCSCode)
            m_verticalDatum = vdcNGVD29;
        else if(VerticalCSCode::VertCS_North_American_Vertical_Datum_1988 == verticalCSCode)
            m_verticalDatum = vdcNAVD88;
        else
            m_verticalDatum = vdcGeoid; // All other values over 5100 is a geoid vertical datum
        }

    return SUCCESS;
    }
};

/*=================================================================================**//**
*
* GeoTiffKey creator class.
*
+===============+===============+===============+===============+===============+======*/
struct GeoTiffKeyCreator
{
private:
BaseGCSCR           m_inGCS;
IGeoTiffKeysList&   m_geoTiffKeys;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
GeoTiffKeyCreator
(
BaseGCSCR               inGCS,
IGeoTiffKeysList&       geoTiffKeys         // The GeoTiff key list
) : m_inGCS (inGCS), m_geoTiffKeys (geoTiffKeys)
    {
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       SaveGCS
(
)
    {
    if (NULL == m_inGCS.m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    bool isGeographic = (cs_PRJCOD_UNITY == m_inGCS.m_csParameters->prj_code);
    m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::GTModelTypeGeoKey, (uint32_t) (isGeographic ? GeoTiffKeyInterpreter::ModelTypeGeographic : GeoTiffKeyInterpreter::ModelTypeProjected));

    // if the coordinate system has an EPSG code, then we can use that.
    int     epsgCode = m_inGCS.GetEPSGCode();
    if (isGeographic)
        {
        // BeAssert ( (epsgCode >= 4000) && (epsgCode <= 4999) );
        // There are some EPSG LL coordinate systems in our coordsys.asc (e.g., 104107) that are outside of the range, but might match. We used to BeAssert and return error,
        // but I changed it to just ignore them.
        if ( (epsgCode < 4000) || (epsgCode > 4999) )
            epsgCode = 0;
        }

    unsigned short CoordSysTypeKeyID;

    if (isGeographic)
        {
        CoordSysTypeKeyID = GeoTiffKeyInterpreter::GeographicTypeGeoKey;
        }
    else
        {
        // use the private key when the EPSG code is greater than 65535 since,
        // according to the GeoTIFF specification, the value of the key
        // ProjectedCSTypeGeoKey cannot be greater than 65535.
        if (epsgCode > 65535)
            {
            CoordSysTypeKeyID = GeoTiffKeyInterpreter::ProjectedCSTypeGeoKeyLong;
            }
        else
            {
            CoordSysTypeKeyID = GeoTiffKeyInterpreter::ProjectedCSTypeGeoKey;
            }
        }

    // add the appropriate coordinate system type key. Either the EPSG code, or "User Defined".
    m_geoTiffKeys.AddKey (CoordSysTypeKeyID,
                          (uint32_t) ((0 != epsgCode) ? epsgCode : UserDefinedKeyValue));
    if (0 != epsgCode)
        {
        IGeoTiffKeysList::GeoKeyItem projectedCSType;
        projectedCSType.KeyDataType      = IGeoTiffKeysList::LONG;
        projectedCSType.KeyValue.LongVal = epsgCode;

        GeoTiffKeyInterpreter geoTiffKeyInterpreter;

        geoTiffKeyInterpreter.m_modelType = GeoTiffKeyInterpreter::ModelTypeProjected;
        geoTiffKeyInterpreter.ProcessProjectedCSTypeKey (projectedCSType);

        // add the ProjLinearUnitsGeoKey if the unit of the CGS is different than the unit of the
        // coordinate system defined by the GCS\92s EPSG code.
        if (geoTiffKeyInterpreter.m_csDef.unit_scl != m_inGCS.m_csParameters->csdef.unit_scl)
            SaveProjLinearUnitsKey();

        return SUCCESS;
        }

    // here we have a coordinate system we have to represent as a User Defined CS.
    if (isGeographic)
        return SaveGeographicUserDefinition ();
    else
        return SaveProjectedUserDefinition ();
    return SUCCESS;
    }

private:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       SaveGeographicUserDefinition
(
)
    {
    // we have to be able to find a datum.
    int epsgDatumCode;
    int epsgEllipsoidCode;

    if (0 != (epsgDatumCode = m_inGCS.GetEPSGDatumCode()))
        {
        int     offset;

        // GeogGeodeticDatumGeoKey
        // ellipsoid only must be stored in the range 6000-6200, but the EPSG designations are 7000-7200.
        // Those numbers are also EPSG datum codes, so offset them by -1000 before storing..
        if ( (epsgDatumCode >= 7000) && (epsgDatumCode < 7200) )
            offset = -1000;
        else
            offset = 0;
        m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::GeogGeodeticDatumGeoKey, (uint32_t) (epsgDatumCode + offset));
        // complete the definition with the prime meridian
        SavePrimeMeridian ();
        }
    else if (0 != (epsgEllipsoidCode = m_inGCS.GetEPSGEllipsoidCode()))
        {
        // Prime Meridian stored before Ellipsoid code
        SavePrimeMeridian ();

        // I don'think this is any different than storing the Datum code that refers only to the Ellipsoid in the Datum case above.
        m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::GeogEllipsoidGeoKey, (uint32_t) epsgEllipsoidCode);
        }
    else
        {
        // Prime Meridian stored before Ellipsoid definition
        SavePrimeMeridian ();
        // set geographic linear units to meters.
        m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::GeogLinearUnitsGeoKey, (uint32_t) 9001);
        m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::GeogSemiMajorAxisGeoKey, m_inGCS.m_csParameters->datum.e_rad);
        m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::GeogSemiMinorAxisGeoKey, m_inGCS.m_csParameters->datum.p_rad);
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       SavePrimeMeridian
(
)
    {
    // first see if we can get a EPSG code.
    double  primeMeridian = m_inGCS.m_csParameters->csdef.org_lng;
    int     epsgCode = 0;

    m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::GeogAngularUnitsGeoKey, (uint32_t)9102);

    if (doubleSame (primeMeridian, 0.0))
        epsgCode = GeoTiffKeyInterpreter::PM_Greenwich;
    else if (doubleSame (primeMeridian, -9.0754862))
        epsgCode =  GeoTiffKeyInterpreter::PM_Lisbon;
    else if (doubleSame (primeMeridian, 2.337229167))
        epsgCode =  GeoTiffKeyInterpreter::PM_Paris;
    else if (doubleSame (primeMeridian, -74.04513))
        epsgCode =  GeoTiffKeyInterpreter::PM_Bogota;
    else if (doubleSame (primeMeridian, -3.411658))
        epsgCode =  GeoTiffKeyInterpreter::PM_Madrid;
    else if (doubleSame (primeMeridian, 12.27084))
        epsgCode =  GeoTiffKeyInterpreter::PM_Rome;
    else if (doubleSame (primeMeridian, 7.26225))
        epsgCode =  GeoTiffKeyInterpreter::PM_Bern;
    else if (doubleSame (primeMeridian, 106.482779))
        epsgCode =  GeoTiffKeyInterpreter::PM_Jakarta;
    else if (doubleSame (primeMeridian, -17.4))
        epsgCode =  GeoTiffKeyInterpreter::PM_Ferro;
    else if (doubleSame (primeMeridian, 4.220471))
        epsgCode =  GeoTiffKeyInterpreter::PM_Brussels;
    else if (doubleSame (primeMeridian, 18.03298))
        epsgCode =  GeoTiffKeyInterpreter::PM_Stockholm;
    else if (doubleSame (primeMeridian, 23.4258815))
        epsgCode =  GeoTiffKeyInterpreter::PM_Athens;
    else if (doubleSame (primeMeridian, 10.43225))
        epsgCode =  GeoTiffKeyInterpreter::PM_Oslo;

    if (0 != epsgCode)
        {
        m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::GeogPrimeMeridianGeoKey, (uint32_t) epsgCode);
        return SUCCESS;
        }

    m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::GeogPrimeMeridianLongGeoKey, primeMeridian);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       SaveProjectedUserDefinition
(
)
    {
    // check to see whether the projection we have can be saved to EPSG.
    const CSDefinition*     csDef  = &m_inGCS.m_csParameters->csdef;
    StatusInt               status;

    if (SUCCESS != (status = SaveGeographicUserDefinition ()))
        return status;

    SaveProjLinearUnitsKey();

    SaveFalseENKeys();

    switch (m_inGCS.m_csParameters->prj_code)
        {
        case cs_PRJCOD_TRMER:
#if defined(TOTAL_SPECIAL)
        case cs_PRJCOD_TRMERBF: // We save the BF variation as plain TRMER as it is a problem of application in computation ... the projection principle of the
                                // method is preserved. In any case this projection method is unknown by GeoTIFF so instead of discaring it we
                                // simplify it
#endif
            {
            m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCoordTransGeoKey, (uint32_t)GeoTiffKeyInterpreter::CT_TransverseMercator);
            SaveNatOriginKeys (csDef->prj_prm1, csDef->org_lat);
            SaveScaleReductionKey();
            break;
            }
        case cs_PRJCOD_HOM1XY:
            {
            m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCoordTransGeoKey, (uint32_t)GeoTiffKeyInterpreter::CT_ObliqueMercator);
            SaveFalseENKeys();
            SaveCenterKeys (csDef->prj_prm1, csDef->prj_prm2);
            SaveScaleReductionKey();
            SaveAzimuthAngleKey (csDef->prj_prm3);
            break;
            }
        case cs_PRJCOD_MRCAT:
        case cs_PRJCOD_MRCATK:
            {
            m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCoordTransGeoKey, (uint32_t)GeoTiffKeyInterpreter::CT_Mercator);
            SaveStandardParallelKeys (csDef->prj_prm2, 0.0, false);
            SaveCenterLongKey (csDef->prj_prm1);    // not sure whether we need CenterLongitude or NatOriginLong key, save both.
            SaveNatOriginLongKey (csDef->prj_prm1);
            if (cs_PRJCOD_MRCATK == m_inGCS.m_csParameters->prj_code)
                SaveScaleReductionKey();
            break;
            }
        case cs_PRJCOD_LM2SP:
            {
            m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCoordTransGeoKey, (uint32_t)GeoTiffKeyInterpreter::CT_LambertConfConic_2SP);
            SaveStandardParallelKeys (csDef->prj_prm1, csDef->prj_prm2, true);
            SaveNatOriginKeys (csDef->org_lng, csDef->org_lat);
            break;
            }
        case cs_PRJCOD_LM1SP:
            {
            SaveNatOriginKeys (csDef->org_lng, csDef->org_lat);
            m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCoordTransGeoKey, (uint32_t)GeoTiffKeyInterpreter::CT_LambertConfConic_Helmert);
            break;
            }
        case cs_PRJCOD_AZMEA:
            {
            SaveAzimuthAngleKey (csDef->prj_prm1);
            SaveNatOriginKeys (csDef->org_lng, csDef->org_lat);
            m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCoordTransGeoKey, (uint32_t)GeoTiffKeyInterpreter::CT_LambertAzimEqualArea);
            break;
            }
        case cs_PRJCOD_ALBER:
            {
            SaveStandardParallelKeys (csDef->prj_prm1, csDef->prj_prm2, true);
            SaveNatOriginKeys (csDef->org_lng, csDef->org_lat);
            m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCoordTransGeoKey, (uint32_t)GeoTiffKeyInterpreter::CT_AlbersEqualArea);
            break;
            }
        case cs_PRJCOD_AZMED:
            {
            SaveAzimuthAngleKey (csDef->prj_prm1);
            SaveNatOriginKeys (csDef->org_lng, csDef->org_lat);
            m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCoordTransGeoKey, (uint32_t)GeoTiffKeyInterpreter::CT_AzimuthalEquidistant);
            break;
            }
        case cs_PRJCOD_EDCNC:
            {
            SaveStandardParallelKeys (csDef->prj_prm1, csDef->prj_prm2, true);
            SaveNatOriginKeys (csDef->org_lng, csDef->org_lat);
            m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCoordTransGeoKey, (uint32_t)GeoTiffKeyInterpreter::CT_EquidistantConic);
            break;
            }
        case cs_PRJCOD_SSTRO:
            {
            m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCoordTransGeoKey, (uint32_t)GeoTiffKeyInterpreter::CT_Stereographic);
            SaveAzimuthAngleKey (csDef->prj_prm3);
            SaveNatOriginKeys (csDef->org_lng, csDef->org_lat);
            SaveScaleReductionKey();
            break;
            }
        case cs_PRJCOD_PSTRO:
            {
            m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCoordTransGeoKey, (uint32_t)GeoTiffKeyInterpreter::CT_PolarStereographic);
            SaveAzimuthAngleKey (csDef->prj_prm3);
            SaveNatOriginKeys (csDef->org_lng, csDef->org_lat);
            SaveScaleReductionKey();
            break;
            }
        case cs_PRJCOD_OSTRO:
            {
            m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCoordTransGeoKey, (uint32_t)GeoTiffKeyInterpreter::CT_ObliqueStereographic);
            SaveAzimuthAngleKey (csDef->prj_prm3);
            SaveNatOriginKeys (csDef->org_lng, csDef->org_lat);
            SaveScaleReductionKey();
            break;
            }
        case cs_PRJCOD_EDCYL:
            {
            SaveStandardParallelKeys (csDef->org_lat, 0.0, false);
            SaveNatOriginKeys (csDef->org_lng, csDef->org_lat);
            m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCoordTransGeoKey, (uint32_t)GeoTiffKeyInterpreter::CT_Equirectangular);
            }
        case cs_PRJCOD_CSINI:
            {
            SaveNatOriginKeys (csDef->org_lng, csDef->org_lat);
            m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCoordTransGeoKey, (uint32_t)GeoTiffKeyInterpreter::CT_CassiniSoldner);
            break;
            }
        case cs_PRJCOD_GNOMC:
            {
            SaveNatOriginKeys (csDef->org_lng, csDef->org_lat);
            m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCoordTransGeoKey, (uint32_t)GeoTiffKeyInterpreter::CT_Gnomonic);
            break;
            }
        case cs_PRJCOD_MILLR:
            {
            SaveCenterLongKey (csDef->prj_prm1);    // not sure whether we need CenterLongitude or NatOriginLong key, save both.
            SaveNatOriginLongKey (csDef->prj_prm1);
            m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCoordTransGeoKey, (uint32_t)GeoTiffKeyInterpreter::CT_MillerCylindrical);
            break;
            }
        case cs_PRJCOD_ORTHO:
            {
            SaveNatOriginKeys (csDef->org_lng, csDef->org_lat);
            m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCoordTransGeoKey, (uint32_t)GeoTiffKeyInterpreter::CT_Orthographic);
            break;
            }
        case cs_PRJCOD_PLYCN:
            {
            SaveCenterLongKey (csDef->prj_prm1);    // not sure whether we need CenterLongitude or NatOriginLong key, save both.
            SaveNatOriginLongKey (csDef->prj_prm1);
            m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCoordTransGeoKey, (uint32_t)GeoTiffKeyInterpreter::CT_Polyconic);
            break;
            }
        case cs_PRJCOD_ROBIN:
            {
            SaveNatOriginLongKey (csDef->org_lng);
            m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCoordTransGeoKey, (uint32_t)GeoTiffKeyInterpreter::CT_Robinson);
            break;
            }
        case cs_PRJCOD_SINUS:
            {
            SaveNatOriginLongKey (csDef->org_lng);
            m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCoordTransGeoKey, (uint32_t)GeoTiffKeyInterpreter::CT_Sinusoidal);
            break;
            }
        case cs_PRJCOD_VDGRN:
            {
            SaveNatOriginLongKey (csDef->org_lng);
            m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCoordTransGeoKey, (uint32_t)GeoTiffKeyInterpreter::CT_VanDerGrinten);
            break;
            }
        case cs_PRJCOD_NZLND:
            {
            SaveNatOriginKeys (csDef->org_lng, csDef->org_lat);
            SaveScaleReductionKey();
            m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCoordTransGeoKey, (uint32_t)GeoTiffKeyInterpreter::CT_NewZealandMapGrid);
            break;
            }
        case cs_PRJCOD_SOTRM:
            {
            SaveNatOriginKeys (csDef->prj_prm1, csDef->org_lat);
            m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCoordTransGeoKey, (uint32_t)GeoTiffKeyInterpreter::CT_TransvMercator_SouthOriented);
            break;
            }
        default:
            {
            return GEOCOORDERR_CantSaveGCS;
            }
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       SaveProjLinearUnitsKey
(
)
    {
    // find the units key that's appropriate
    const struct cs_Unittab_  *pUnit;
    for (pUnit = cs_Unittab; cs_UTYP_END != pUnit->type; pUnit++)
        {
        // find the units we're using.
        if ( (pUnit->type == cs_UTYP_LEN) && (0 == BeStringUtilities::Stricmp (m_inGCS.m_csParameters->csdef.unit, pUnit->name)))
            {
            if (0 != pUnit->epsgCode)
                {
                m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjLinearUnitsGeoKey, (uint32_t) pUnit->epsgCode);
                return SUCCESS;
                }
            else
                {
                break;
                }
            }
        }

    if (pUnit->type != cs_UTYP_END)
        {
        // did not find our units, or unit did not have EPSG code. Have to use the UnitSize key
        m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjLinearUnitSizeGeoKey, pUnit->factor);
        }

    return GEOCOORDERR_CantSaveGCS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            SaveFalseENKeys
(
)
    {
    m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjFalseEastingGeoKey, m_inGCS.m_csParameters->csdef.x_off);
    m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjFalseNorthingGeoKey, m_inGCS.m_csParameters->csdef.y_off);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            SaveNatOriginKeys
(
double  longitude,
double  latitude
)
    {
    m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjNatOriginLongGeoKey, longitude);
    m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjNatOriginLatGeoKey, latitude);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            SaveNatOriginLongKey
(
double  value
)
    {
    m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjNatOriginLongGeoKey, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            SaveCenterKeys
(
double  longitude,
double  latitude
)
    {
    m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCenterLongGeoKey, longitude);
    m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCenterLatGeoKey, latitude);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            SaveCenterLongKey
(
double value
)
    {
    m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjCenterLongGeoKey, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            SaveScaleReductionKey
(
)
    {
    m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjScaleAtNatOriginGeoKey, m_inGCS.m_csParameters->csdef.scl_red);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            SaveStandardParallelKeys
(
double  parallel1,
double  parallel2,
bool    saveBoth
)
    {
    m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjStdParallel1GeoKey, parallel1);
    if (saveBoth)
        m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjStdParallel2GeoKey, parallel2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            SaveAzimuthAngleKey
(
double  value
)
    {
    // save the GeoAzimuthUnitsKey as degrees
    m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::GeogAzimuthUnitsGeoKey, (uint32_t)9102);
    m_geoTiffKeys.AddKey (GeoTiffKeyInterpreter::ProjAzimuthAngleGeoKey, value);
    }

};


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
struct VerticalDatumConverter
{
private:
    bool            m_inputLatLongInNAD27;      // which latLongs are considered to be in NAD27.
    bool            m_fromNGVD29toNAVD88;       // direction
    VertDatumCode   m_fromVDC;
    VertDatumCode   m_toVDC;

public:
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
VerticalDatumConverter (bool inputIsInNAD27, VertDatumCode inputVdc, VertDatumCode outputVdc)
    {
    BeAssert (inputVdc != outputVdc);

    m_fromVDC = inputVdc;
    m_toVDC = outputVdc;

    // These two parameters are only used if NAVD88 to/from NGVD29 is to be performed.
    m_inputLatLongInNAD27 = inputIsInNAD27;
    m_fromNGVD29toNAVD88  = (vdcNGVD29 == inputVdc);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
~VerticalDatumConverter ()
    {
    if (!BaseGCS::IsLibraryInitialized())
        return;

    CSvrtconCls();
    CS_geoidCls();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   ConvertElevation
(
GeoPointR   outLatLong,
GeoPointCR  inLatLong
)
    {
    if (!BaseGCS::IsLibraryInitialized())
        return GEOCOORDERR_GeoCoordNotInitialized;

    // The process of datum conversion can be complex here depending on the set of in/out
    // vertical datums. Note that the vdcFromDatum value indicates that the ellipsoidal height is
    // used. This height is related to the horizontal datum and conversion may be necessary
    // using CSMAP prior to application of the vertical datum.
    // All other vertical datums supported are Geoid based (orthometric)
    // Here is a map of sequence to be applied
    //      VERT1             VERT2
    //   vdcFromDatum    vdcFromDatum      - CSMAP should take care of vertical elevation changes. The vertical datum should not exist
    //   vdcFromDatum    vdcNAVD88         - Should not happen. Application of NAVD88 implies that source is NGVD29 
    //   vdcFromDatum    vdcNGVD29         - Should not happen. Application of NGVD29 implies that source is NAVD88 
    //   vdcFromDatum    vdcGeoid          - First ellipsoidal height change must be applied then Geoid value at location added to result.
    //   vdcNAVD88       vdcNGVD29         - Normal VERTCON application.
    //   vdcNAVD88       vdcGeoid          - We do nothing. We consider NAVD88 to be coincident to Geoid
    //   vdcNGVD29       vdcGeoid          - We do nothing. We consider NGVD29 to be coincident to Geoid
    
    // vdcGeoid    vdcGeoid - In this specific case we remove Geoid elev, apply ellipsoidal height diff then apply Geoid at new location.
    //                        although this case would give approximatively the same result the slight
    //                        lat/long value may introduce a very small change in elevation.

    // If we have NGVD29 to NAVD88 conversion
    if ((m_fromVDC == vdcNGVD29 || m_fromVDC == vdcNAVD88) && (m_toVDC == vdcNGVD29 || m_toVDC == vdcNAVD88) && (m_toVDC != m_fromVDC))
        {
        // The CSvrtcon29To88 function takes as input the Lat/Long in NAD27.
        double  elevationDelta;
        if (0 == CSvrtcon29To88 (&elevationDelta, (m_inputLatLongInNAD27 ? (const double *) &inLatLong : (const double *) &outLatLong)))
            {
            // Notice that ellipsoidal elevation changes are discarded in this case since the datum pair fully specifies the
            // elevation delta to be applied.
            if (m_fromNGVD29toNAVD88)
                outLatLong.elevation = inLatLong.elevation + elevationDelta;
            else
                outLatLong.elevation = inLatLong.elevation - elevationDelta;
            return SUCCESS;
            }

        // Something went wrong in VERTCON application
        return GEOCOORDERR_VerticalDatumConversion;

        }
    // Otherwise if either vertical datum is NAVD88 or NGVD29  but the other is not then we do nothing
    else if (m_fromVDC != vdcNGVD29 && m_fromVDC != vdcNAVD88 && m_toVDC != vdcNGVD29 && m_toVDC != vdcNAVD88)
        {
        // The ellipsoidal height diff is already applied but additions and substraction are commutative so we do not care
        // about the order of application given we use the proper lat/long combination.
        // In every case the output point should already have a meaningful elevation to correct.
        double  elevationDelta;
        if (m_fromVDC == vdcGeoid)
            {
            if (0 == CS_geoidHgt((const double *) &inLatLong, &elevationDelta))
                outLatLong.elevation += elevationDelta;
            else
                return GEOCOORDERR_VerticalDatumConversion;
            }

        if (m_toVDC == vdcGeoid)
            {
            if (0 == CS_geoidHgt((double *) &outLatLong, &elevationDelta))
                outLatLong.elevation -= elevationDelta;
            else
                return GEOCOORDERR_VerticalDatumConversion;
            }
        }
        
        
    return SUCCESS;
    }

};

/*=================================================================================**//**
*
* The static variable and these 3 static functions are uniquely intended for use
* for the user override geodetic transform compilation process part of the Initialize
* process. 
*
* The three methods enable opening, closing and writing to the error log file.
*
+===============+===============+===============+===============+===============+======*/

static FILE* s_errorLogFile;
static AString s_errorLogFileName;
// Dummy function required for error management of GX compilation during initialization.
int GeodeticCompilationErrorLog (char *mesg) 
    {
    // Create/Recreate file the first time.
    if (NULL == s_errorLogFile)
        {
        s_errorLogFile = fopen(s_errorLogFileName.c_str(), "w");
        }
        
    if (s_errorLogFile != NULL)
        fprintf(s_errorLogFile, "%s\n", mesg);

    return SUCCESS;
    }

// Dummy function required for error management of GX compilation during initialization.
int GeodeticCompilationErrorOpen (BeFileNameCR errorLogFileName) 
    {
	s_errorLogFileName = AString (errorLogFileName.c_str());

	return 0;
    }

// Dummy function required for error management of GX compilation during initialization.
int GeodeticCompilationErrorClose () 
    {
    if (s_errorLogFile != NULL)
        fclose(s_errorLogFile);

    return SUCCESS;
    }

#define WTOUTF8(W) Utf8String(W).c_str()

/*=================================================================================**//**
*
* Geographic Coordinate System class.
*
+===============+===============+===============+===============+===============+======*/
void BaseGCS::Initialize (WCharCP dataDirectory)
    {
    BeFileName dir (dataDirectory);


    ::CS_csfnm ("coordsys.dty");
    ::CS_dtfnm ("datum.dty");
    ::CS_elfnm ("ellipsoid.dty");
    ::CS_gxfnm ("GeodeticTransform.dty");
    ::CS_gpfnm ("GeodeticPath.dty");
    ::CS_altdr (WTOUTF8(dir));

    // At this stage the directory is sufficiently initialized to state that the system should be operational
    s_geoCoordInitialized = true;

    // The remainder concerns user definition overrides.
    // This sets up the user dictionary override
    BeFileName userDir(dir); userDir.AppendToPath (L"UserOverrides");
    ::CS_usrdr(WTOUTF8(userDir));

    // We check for the presence of a UserOverride GeodeticTransform ASCII file
    BeFileName userOverrideGxFileAscii (dir); userOverrideGxFileAscii.AppendToPath (L"UserOverrideGeodeticTransform.asc");
    BeFileName userOverrideGxFileBinary (userDir); userDir.AppendToPath (L"GeodeticTransform.dty");

    // We use the CSMAP function that are readily available and Android and WinCE compliant
    cs_Time_ asciiTime = CS_fileModTime (WTOUTF8(userOverrideGxFileAscii));
    cs_Time_ binaryTime = CS_fileModTime (WTOUTF8(userOverrideGxFileBinary));

    BeFileName datumFile (dir); datumFile.AppendToPath (L"datum.dty");
    BeFileName geodeticTransformOverrrides (userDir); geodeticTransformOverrrides.AppendToPath (L"GeodeticTransform.dty");


    // Check if the ascii file exists. Note that if it does not but the binary file does we leave everthing as it is.
    // Odd situation but indicates the user has installed a binary file instead of the Ascii for overrides.
    if (asciiTime > 0)
        {
        // If the ascii file time is smaller than the binary then the binary is up to date and we need not do anything
        // Note that is the binary file does not exist then access time is 0 which ascii time cannot be
        if (asciiTime > binaryTime)
            {
            // Times are different ... we need to recompile the geodetic transformation user overrides.
            
            // We sure the directory exists
			if (CS_fileModTime (WTOUTF8(userDir)) == 0)
				{
				BeFileName::CreateNewDirectory(userDir.c_str());
				}

            // Initialise the error log
            BeFileName errorLogFileName (userDir); errorLogFileName.AppendToPath (L"GeodeticTransformErrors.txt");
            GeodeticCompilationErrorOpen (errorLogFileName);
        
        	int err_cnt;
        	err_cnt = CSgxcomp (WTOUTF8(userOverrideGxFileAscii), WTOUTF8(geodeticTransformOverrrides), cs_CMPLR_CRYPT | cs_CMPLR_EXTENTS, WTOUTF8(datumFile), GeodeticCompilationErrorLog);
        	if (err_cnt != 0)
                {
                // What do we do then? Not much that can be done ... The user should look at the error log file
                }

            GeodeticCompilationErrorClose();
            }
        }
    }

/*=================================================================================**//**
*
* static method that returns true if the library was initialized and false otherwise
+===============+===============+===============+===============+===============+======*/
bool BaseGCS::IsLibraryInitialized ()
    {
    return s_geoCoordInitialized;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BaseGCS::BaseGCS ()
    {
    Init();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BaseGCS::BaseGCS
(
WCharCP         coordinateSystemName
)
    {
    Init();

    // Given the library is NOT initialized ...
    if (!IsLibraryInitialized())
        {
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        return;
        }

   
    if (NULL == (m_csParameters = LibraryManager::Instance()->GetCS (m_sourceLibrary, coordinateSystemName)))
        m_csError = cs_Error;
    else
        // since we looked it up, we put COORDSYS_KEYNM as the coordsys in the type 66 element.
        m_coordSysId = COORDSYS_KEYNM;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BaseGCS::BaseGCS
(
CSParameters&   csParameters,
int32_t         coordinateSystemId,
LibraryP        sourceLibrary
)
    {
    Init();

    // Given the library is NOT initialized ...
    if (!IsLibraryInitialized())
        {
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        return;
        }

    m_csParameters  = &csParameters;
    m_coordSysId    = coordinateSystemId;
    m_sourceLibrary = sourceLibrary;

    if (NULL == m_csParameters)
        m_csError = GEOCOORDERR_InvalidCoordSys;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BaseGCS::BaseGCS (BaseGCSCR source)
    {
    Init();

    // Given the library is NOT initialized ...
    if (!IsLibraryInitialized())
        {
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        return;
        }

    BeAssert ( NULL != source.m_csParameters );

    // copy the parameters structure. 
    if (NULL != source.m_csParameters)
        {
        m_csParameters = (CSParameters *) CS_malc (sizeof (CSParameters));
        memcpy (m_csParameters, source.m_csParameters, sizeof (CSParameters));
        }

    m_coordSysId        = source.m_coordSysId;
    m_verticalDatum     = source.m_verticalDatum;
    m_sourceLibrary     = source.m_sourceLibrary;

    if (source.m_localTransformer.IsValid())
        m_localTransformer  = source.m_localTransformer->Copy();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
void            BaseGCS::Init ()
    {
    m_csParameters                  = NULL;
    m_csError                       = 0;
    m_datumConverter                = NULL;
	m_destinationGCS                = NULL;
    m_coordSysId                    = 0;
    m_canEdit                       = false;
    m_reprojectElevation            = false;
    m_verticalDatum                 = vdcFromDatum;
    m_sourceLibrary                 = NULL;
    m_failedToFindSourceLibrary     = false;

    m_nameString                    = NULL;
    m_descriptionString             = NULL;
    m_projectionString              = NULL;
    m_datumNameString               = NULL;
    m_datumDescriptionString        = NULL;
    m_ellipsoidNameString           = NULL;
    m_ellipsoidDescriptionString    = NULL;

    m_originalWKT                   = NULL;
    m_modified                      = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BaseGCS::SetFromCSName (WCharCP coordinateSystemKeyName)
    {
    // Given the library is NOT initialized ...
    if (!IsLibraryInitialized())
        {
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        return m_csError;
        }

    m_csError       = 0;

    if (NULL != m_datumConverter)
        m_datumConverter->Destroy();

    // replace current contents of
    if (NULL == (m_csParameters = LibraryManager::Instance()->ReplaceCSContents (m_sourceLibrary, coordinateSystemKeyName, m_csParameters)))
        m_csError = cs_Error;

    return m_csError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
BaseGCSPtr      BaseGCS::CreateGCS ()
    {
    return new BaseGCS();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
BaseGCSPtr      BaseGCS::CreateGCS (WCharCP coordinateSystemKeyName)
    {
    return new BaseGCS (coordinateSystemKeyName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
BaseGCSPtr      BaseGCS::CreateGCS
(
CSParameters const& csParameters,
int32_t             coordSysId,
LibraryP            sourceLibrary
)
    {
    // copy the csParameters because the original might not have been allocated by CS_malc.
    CSParameters* copied = (CSParameters *) CS_malc (sizeof (CSParameters));
    *copied = csParameters;
    return new BaseGCS (*copied, coordSysId, sourceLibrary);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   10/07
+---------------+---------------+---------------+---------------+---------------+------*/
BaseGCSPtr      BaseGCS::CreateGCS
(
CSParameters const& csParameters,
int32_t             coordSysId
)
    {
    return CreateGCS (csParameters, coordSysId, NULL);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BaseGCSPtr BaseGCS::CreateGCS (BaseGCSCR baseGcs)
    {
    return new BaseGCS(baseGcs);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BaseGCS::InitAzimuthalEqualArea
(
WStringP                errorMsg,
WCharCP                 datumName,
WCharCP                 unitName,
double                  originLongitude,
double                  originLatitude,
double                  azimuthAngle,
double                  scale,              // this argument is ignored!
double                  falseEasting,
double                  falseNorthing,
int                     quadrant
)
    {
    // Given the library is NOT initialized ...
    if (!IsLibraryInitialized())
        {
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        return m_csError;
        }

    SetModified(true);
    

    CSDefinition        csDef;
    memset (&csDef, 0, sizeof(csDef));

    CSMap::CS_stncp (csDef.prj_knm, CS_AZMEA, DIM(csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, AString(unitName).c_str(), DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, AString(datumName).c_str(), DIM(csDef.dat_knm));

    csDef.prj_prm1  = azimuthAngle;
    csDef.org_lng   = originLongitude;
    csDef.org_lat   = originLatitude;
    // Note: We do not allow CSMap to handle scale, the scale argument is ignored.
    csDef.map_scl   = 1.0;
    csDef.x_off     = falseEasting;
    csDef.y_off     = falseNorthing;
    csDef.quad      = (short) quadrant;

    if (NULL == (m_csParameters = CSMap::CScsloc1 (&csDef)))
        {
        if (NULL != errorMsg)
            {
            char    csErrorMsg[512];
            CSMap::CS_errmsg (csErrorMsg, DIM(csErrorMsg));
            errorMsg->AssignA (csErrorMsg);
            BeAssert (false);
            }
        m_csError = cs_Error;
        return cs_Error;
        }

    m_sourceLibrary = LibraryManager::Instance()->GetSystemLibrary();

    m_coordSysId = COORDSYS_AZMEA;

    SetModified(false);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Alain.Robert   03/2016
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BaseGCS::InitTransverseMercator
(
WStringP                errorMsg,
WCharCP                 datumName,
WCharCP                 unitName,
double                  centralMeridian,
double                  originLatitude,
double                  scale,              
double                  falseEasting,
double                  falseNorthing,
int                     quadrant
)
    {
    // Given the library is NOT initialized ...
    if (!IsLibraryInitialized())
        {
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        return m_csError;
        }

/* &&AR WORK TO BE DONE CONCERNING PARAMS */
    SetModified(true);
    

    CSDefinition        csDef;
    memset (&csDef, 0, sizeof(csDef));

    CSMap::CS_stncp (csDef.prj_knm, CS_TRMER, DIM(csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, AString(unitName).c_str(), DIM(csDef.unit));
    CSMap::CS_stncp (csDef.dat_knm, AString(datumName).c_str(), DIM(csDef.dat_knm));

    // csDef.prj_prm1  =;
    csDef.org_lng   = centralMeridian;
    csDef.org_lat   = originLatitude;

    csDef.map_scl   = scale;
    csDef.x_off     = falseEasting;
    csDef.y_off     = falseNorthing;
    csDef.quad      = (short) quadrant;

    if (NULL == (m_csParameters = CSMap::CScsloc1 (&csDef)))
        {
        if (NULL != errorMsg)
            {
            char    csErrorMsg[512];
            CSMap::CS_errmsg (csErrorMsg, DIM(csErrorMsg));
            errorMsg->AssignA (csErrorMsg);
            BeAssert (false);
            }
        m_csError = cs_Error;
        return cs_Error;
        }

    m_sourceLibrary = LibraryManager::Instance()->GetSystemLibrary();

    m_coordSysId = COORDSYS_AZMEA;

    SetModified(false);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   12/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BaseGCS::InitLatLong
(
WStringP                errorMsg,
WCharCP                 datumName,          // Datum
WCharCP                 ellipseName,        // only if datum is NULL.
WCharCP                 unitName,           // usually "DEGREE"
double                  originLongitude,    // displacement from Greenwich
double                  originLatitude      // displacement from Greenwich
)
    {
    // Given the library is NOT initialized ...
    if (!IsLibraryInitialized())
        {
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        return m_csError;
        }

    SetModified(true);

    CSDefinition        csDef;
    memset (&csDef, 0, sizeof(csDef));

    CSMap::CS_stncp (csDef.prj_knm, CS_UNITY, DIM(csDef.prj_knm));
    CSMap::CS_stncp (csDef.unit, AString(unitName).c_str(), DIM(csDef.unit));
    if (NULL != datumName)
        CSMap::CS_stncp (csDef.dat_knm, AString(datumName).c_str(), DIM(csDef.dat_knm));
    if (NULL != ellipseName)
        CSMap::CS_stncp (csDef.elp_knm, AString(ellipseName).c_str(), DIM(csDef.elp_knm));

    csDef.org_lng   = originLongitude;
    csDef.org_lat   = originLatitude;

    if (NULL == (m_csParameters = CSMap::CScsloc1 (&csDef)))
        {
        if (NULL != errorMsg)
            {
            char    csErrorMsg[512];
            CSMap::CS_errmsg (csErrorMsg, DIM(csErrorMsg));
            errorMsg->AssignA (csErrorMsg);
            BeAssert (false);
            }
        m_csError = cs_Error;
        return cs_Error;
        }

    m_sourceLibrary = LibraryManager::Instance()->GetSystemLibrary();


    m_coordSysId = COORDSYS_UNITY;

    SetModified(false);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BaseGCS::InitFromWellKnownText
(
StatusInt              *warning,            // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
WStringP                warningOrErrorMsg,  // Error message.
WktFlavor               wktFlavor,          // The WKT Flavor.
WCharCP                 wellKnownText       // The Well Known Text specifying the coordinate system.
)
    {
    // Given the library is NOT initialized ...
    if (!IsLibraryInitialized())
        {
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        return m_csError;
        }

    SetModified(true);

    CSDefinition        csDef;
    CSDatumDef          csDatumDef;
    CSEllipsoidDef      csEllipsoidDef;

    if (NULL != warning)
        *warning = SUCCESS;

    AString mbWellKnownText (wellKnownText);
    StatusInt           status = CSMap::CS_wktToCsEx (&csDef, &csDatumDef, &csEllipsoidDef, wktFlavor, mbWellKnownText.c_str());

    if (SUCCESS != status)
        {
        if (NULL != warningOrErrorMsg)
            {
            char    csErrorMsg[512];
            CSMap::CS_errmsg (csErrorMsg, DIM(csErrorMsg));
            warningOrErrorMsg->AssignA (csErrorMsg);
//            BeAssert (false);
            }
        // process warnings.
        if ((status & ~(StatusInt)(cs_EL2WKT_NMTRUNC | cs_DT2WKT_NMTRUNC | cs_CS2WKT_NMTRUNC | cs_DT2WKT_DTDEF | cs_DT2WKT_NODEF)) == 0)
            {
            if (NULL != warning)
                *warning = status;
            status = SUCCESS;
            }
        else
            m_csError = cs_Error;
        }

    // We impose that the datum be known in the dictionary ... something the WKT parser does not require
    // If the datum is not part for the known datums then we set the return status to ERROR and the process will get cought
    // by the fallback solution.
    if (NULL == CSMap::CS_dtdef (csDatumDef.key_nm))
        status = ERROR;

    if (SUCCESS == status)
        {
        if (NULL == (m_csParameters = CSMap::CScsloc2 (&csDef, &csDatumDef, &csEllipsoidDef)))
            {
            if (NULL != warningOrErrorMsg)
                {
                char    csErrorMsg[512];
                CSMap::CS_errmsg (csErrorMsg, DIM(csErrorMsg));
                warningOrErrorMsg->AssignA (csErrorMsg);
                BeAssert (false);
                }
            m_csError = cs_Error;
            status = cs_Error;
            }
        }
        else
        {
            // The datum is valid and defined in the system library
            m_sourceLibrary = LibraryManager::Instance()->GetSystemLibrary();
        }

    // This section was added as an alternate WKT parser. For some reason the CSMAP parser
    // has some difficulties with many existing WKTs. The homemade parser does a far better job
    // of interpretating general WKTs but may have some difficulties with specific obscure projections
    // We thus use our parser only as a fallback solution, and any error that may occur
    // will simply be disregarded and previous CSMAP related error will be returned.
    if (SUCCESS != status)
        {
        try {

            SRSWKTParser theWKTParser;
            StatusInt status2 = theWKTParser.Process (*this, wellKnownText);
            if ((SUCCESS == status2) && (IsValid()))
                {
                // Clear error in case it occured during previous CScsloc2
                m_csError = 0;
                if (warningOrErrorMsg)
                    warningOrErrorMsg->clear();
                status = SUCCESS;
                }
            else
                {
                // This must be done in case of error since the parser allocates the structure but cannot destroy it
                CSMAP_FREE_AND_CLEAR (m_csParameters);
                }

            }
        catch (...)
            {
            // We could not succeed ... let previous csmap error go through
            // This must be done in case of error since the parser allocates the structure but cannot destroy it
            CSMAP_FREE_AND_CLEAR (m_csParameters);
            }
        }

    // this will cause the type 66 to be saved with coordSysId set the same as projType.
    m_coordSysId = 0;

    // Even if invalid it is indicated as unmodified.
    SetModified(false);

    // Save the original WKT in case the user wants it back
    if (IsValid() && (SUCCESS == status))
        {
        DELETE_AND_CLEAR (m_originalWKT);
        m_originalWKT = new WString(wellKnownText);
        }

    return status;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BaseGCS::InitFromEPSGCode
(
StatusInt              *warning,            // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
WStringP                warningOrErrorMsg,  // Error message.
int                     epsgCode
)
    {
    // Given the library is NOT initialized ...
    if (!IsLibraryInitialized())
        {
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        return m_csError;
        }

    SetModified(true);

    if (NULL != warning)
        *warning = SUCCESS;

    // first try to find a coordinate system called EPSG:nnnnn
    WChar        csName[256];
    BeStringUtilities::Snwprintf (csName, L"EPSG:%d", epsgCode);

    if (NULL == (m_csParameters = LibraryManager::Instance()->GetCS (m_sourceLibrary, csName)))
        {
        m_csError = cs_Error;
        }
    else
        {
        // since we looked it up, we put COORDSYS_KEYNM as the coordsys in the type 66 element.
        m_coordSysId = COORDSYS_KEYNM;
        SetModified(false);
        return SUCCESS;
        }

    // if we get here, we didn't find an EPSG GCS from the synthesized name. We'll look in CS-Map's look-up table.

    char            coordSysName[128];
    enum EcsMapSt   csMapSt;
    coordSysName[0] = '\0';

    csMapSt = csMapIdToNameC (csMapProjGeoCSys,
                              coordSysName,
                              sizeof (coordSysName),
                              csMapFlvrCsMap,
                              csMapFlvrEpsg,
                              static_cast<uint32_t>(epsgCode));

    if (csMapSt == csMapOk)
        {
        if (NULL != (m_csParameters = LibraryManager::Instance()->GetCS (m_sourceLibrary, WString(coordSysName,false).c_str())))
            {
            m_coordSysId = COORDSYS_KEYNM;
            SetModified(false);
            return SUCCESS;
            }
        }
    SetModified(false);
    return m_csError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BaseGCS::GetWellKnownText
(
WStringR            wellKnownText,      // The WKT.
WktFlavor           wktFlavor,          // The WKT Flavor.
bool                originalIfPresent   // true indicates that if the BaseGCS originates from a WKT fragment then this WKT should be returned
) const
    {
    // Given the library is NOT initialized ...
    if (!IsLibraryInitialized())
        {
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        return m_csError;
        }

    if (originalIfPresent && m_originalWKT != nullptr && !m_originalWKT->empty())
        {
        // NOTE: Even if the original WKT was a COMPD_CS we return it as it is
        // regardless the method usually returns non-COMPD_CS
        wellKnownText = *m_originalWKT;
        return SUCCESS;

        }
    char        stringBuf[10240];
    StatusInt   status;
    wellKnownText.clear();
    if (0 > (status = CScs2WktEx (stringBuf, sizeof(stringBuf), (ErcWktFlavor) wktFlavor, &m_csParameters->csdef, NULL, NULL, wktFlavorUnknown == wktFlavor ? 0 : cs_WKTFLG_MAPNAMES)))
        return status;
    wellKnownText = WString(stringBuf,false);
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Alain.Robert   07/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BaseGCS::GetCompoundCSWellKnownText
(
WStringR            wellKnownText,      // The WKT.
WktFlavor           wktFlavor,           // The WKT Flavor.
bool                originalIfPresent   // true indicates that if the BaseGCS originates from a WKT fragment then this WKT should be returned
) const
    {
    // Given the library is NOT initialized ...
    if (!IsLibraryInitialized())
        {
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        return m_csError;
        }

    // Obtain the GCS well known text
    WString temp;
    StatusInt status = SUCCESS;

    // If original WKT is requested and present we copy it otherwise we generate the WKT
    if (originalIfPresent && m_originalWKT != nullptr && !m_originalWKT->empty())
        temp = *m_originalWKT;
    else
        status = GetWellKnownText(temp, wktFlavor, false);

    if (SUCCESS == status)
        {
        // If the WKT comes from the original WKT it may already contain a COMPD clause 
        // in this case we do not add one.
        if (temp.substr(0, 8) == L"COMPD_CS")
            wellKnownText = temp;
        else
            {
            // No COMPD_CS clause ... we add one and add VERT_CS clause at end
            wellKnownText = L"COMPD_CS[\"" + WString(GetName()) + L"\",";
            wellKnownText += temp;            // Add plain WKT PROJCS or GEOCS section
 
            // We complete with Vertical Datum section
            // First determine the texts to be added for various items.
            // WKT texts are not usually meant to be translatable so we use English names when applicable
            VertDatumCode verticalCode = GetVerticalDatumCode();
            WString verticalCSName;
            WString verticalDatumWKTCode;
            WString verticalCSAuthorityName = L"EPSG"; // We only support EPSG authority name at the moment
            WString verticalCSAuthorityCode;
            WString verticalDatumName;
            WString verticalDatumAuthorityName = L"EPSG"; // We only support EPSG authority name at the moment
            WString verticalDatumAuthorityCode;
            
            if (vdcFromDatum == verticalCode)
                {
                verticalCSName = L"Ellipsoid Height";
                verticalDatumWKTCode = L"2002";
                verticalDatumName = L"Ellipsoid";
                // EPSG database defines no code for ellipsoidal height since it is not really a vertical datum
                }
            else if (vdcNGVD29 == verticalCode)
                {
                verticalCSName = L"NGVD29";
             
                verticalDatumWKTCode = L"2005";  // Although we use it differently NGVD29 is a geoid vertical CS
                verticalDatumName = L"NGVD29";
                verticalCSAuthorityCode = L"5702";
                verticalDatumAuthorityCode = L"5102";
                }
            else if (vdcNAVD88 == verticalCode)
                {
                verticalCSName = L"NAVD88";
                verticalDatumWKTCode = L"2005";  // Although we use it differently NAVD88 is a geoid vertical CS
                verticalDatumName = L"NAVD88";
                verticalCSAuthorityCode = L"5703";
                verticalDatumAuthorityCode = L"5103";
                }
            else if (vdcGeoid == verticalCode)
                {
                verticalCSName = L"Generic Geoid";
                verticalDatumWKTCode = L"2005";
                verticalDatumName = L"Generic Vertical Datum";
                // Since we are dealing with a generic Geoid there is no authority code defined
                }
            else
                {
                // This section is only useful in case future version data that uses new datum code unknown to present
                // still results in a valid compound WKT though vertical cs identifiers can then only be guesses.
                verticalCSName = L"Unknown Vertical CS";
                verticalDatumName = L"Unknown Vertical Datum";
                verticalDatumWKTCode = L"2000";  // We use the code for 'other' vertical cs
                }
           
            wellKnownText += L",VERT_CS[\"" + verticalCSName + L"\",VERT_DATUM[\"" + verticalDatumName + L"\"," + verticalDatumWKTCode;
 
            // We only add authority names and code for the OGC flavor
            if ((wktFlavorOGC == wktFlavor) && (verticalDatumAuthorityCode != L""))
                {
                wellKnownText += L",AUTHORITY[\"" + verticalDatumAuthorityName + L"\",\"" + verticalDatumAuthorityCode + L"\"]";
                }
 
            // Close VERT_DATUM section
            wellKnownText += L"]";
 
            // UNIT Section
            wellKnownText += L",UNIT[\"";
            wchar_t conversionFactor[20];
            swprintf(conversionFactor, 20, L"%lf", UnitsFromMeters());
            WString unitName;
            GetUnits(unitName);
            // NOTE: When we have a lat/long coordinate system then vertical units are 'meters' assumed.
            if (GetProjectionCode() ==  BaseGCS::pcvUnity)
                unitName = L"Meters";

            wellKnownText +=  unitName + L"\"," + conversionFactor;
 
            // We only add authority names and code for the OGC flavor
            if ((wktFlavorOGC == wktFlavor) && GetEPSGUnitCode() != 0)
                {
                wchar_t unitCode[10];
                BeStringUtilities::Itow(unitCode, GetEPSGUnitCode(), 10, 10);
                wellKnownText += L",AUTHORITY[\"EPSG\",\"" + WString(unitCode) + L"\"]";
                }
 
            wellKnownText += L"]";
 
            // We only add AXIS section for the OGC flavor
            if (wktFlavorOGC == wktFlavor)
                wellKnownText += L",AXIS[\"Up\", UP]"; // We only support the UP axis direction
            
            // We only add authority names and code for the OGC flavor
            if ((wktFlavorOGC == wktFlavor) && (verticalCSAuthorityCode != L""))
                {
                wellKnownText += L",AUTHORITY[\"" + verticalCSAuthorityName + L"\",\"" + verticalCSAuthorityCode + L"\"]";
                }
 
            // Close both VERT_CS section and COMPD_CS section     
            wellKnownText += L"]]";
            }
        }
    
    return status;
    }
/*---------------------------------------------------------------------------------**//**
*   @bsimethod                                                    Barry.Bentley   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BaseGCS::InitFromGeoTiffKeys
(
StatusInt*              warning,            // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
WStringP                warningOrErrorMsg,  // Error message.
IGeoTiffKeysList const* geoTiffKeys,         // The GeoTiff key list
bool                    allowUnitsOverride   // Indicates if the presence of a unit can override GCS units.
)
    {
    // Given the library is NOT initialized ...
    if (!IsLibraryInitialized())
        {
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        return m_csError;
        }

    SetModified(true);

    StatusInt status;
    // The GeoTiffKeyInterpreter maintains state while parsing the geoTiffKeys, and we can have private methods without having to put then on BaseGCS.
    if (NULL == geoTiffKeys)
        return GEOCOORDERR_BadArg;

    GeoTiffKeyInterpreter   interpreter;
    status = interpreter.Process (*this, warning, warningOrErrorMsg, *geoTiffKeys, allowUnitsOverride);
    if (SUCCESS == status)
        {
        // We keep a copy of the geotiff keys to be able to give it back if the BaseGCS owner wants it.
        bool                                gotKey;
        IGeoTiffKeysList::GeoKeyItem        geoKey;

        for (gotKey = geoTiffKeys->GetFirstKey (&geoKey); gotKey ; gotKey = geoTiffKeys->GetNextKey (&geoKey))
            m_originalGeoKeys.push_back(geoKey);
        }

    SetModified(false);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
*   @bsimethod                                                    Barry.Bentley   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BaseGCS::GetGeoTiffKeys
(
IGeoTiffKeysList*       geoTiffKeys,         // The GeoTiff key list to add geokeys to
bool                    originalsIfPresent   // true indicates the original geokeys should be returned
) const
    {
    // Given the library is NOT initialized ...
    if (!IsLibraryInitialized())
        {
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        return m_csError;
        }

    // The GeoTiffKeyInterpreter maintains state while parsing the geoTiffKeys, and we can have private methods without having to put then on BaseGCS.
    if (NULL == geoTiffKeys)
        return GEOCOORDERR_BadArg;

    if (originalsIfPresent && m_originalGeoKeys.size() > 0)
        {
        for (size_t idx = 0 ; idx < m_originalGeoKeys.size() ; idx++)
            geoTiffKeys->AddKey(m_originalGeoKeys[idx]);

        return SUCCESS;
        }

    // Original geo keys not present or not requested ... we build a new set
    GeoTiffKeyCreator   creator (*this, *geoTiffKeys);
    return creator.SaveGCS ();
    }

/*---------------------------------------------------------------------------------**//**
*   @bsimethod                                                    Barry.Bentley   09/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BaseGCS::CanSaveDatumToGeoTiffKeys () const
    {
    // Given the library is NOT initialized ...
    if (!IsLibraryInitialized())
        {
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        return false;
        }

    if (0 != m_csParameters->datum.key_nm[0])
        return (0 != GetEPSGDatumCode());
    else if (0 != m_csParameters->datum.ell_knm[0])
        return (0 != GetEPSGEllipsoidCode());
    else // user defined prime meridian, etc.
        return true;
    }

/*---------------------------------------------------------------------------------**//**
*   @bsimethod                                                    Barry.Bentley   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BaseGCS::CanSaveToGeoTiffKeys () const
    {
    // Given the library is NOT initialized ...
    if (!IsLibraryInitialized())
        {
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        return false;
        }

    bool isGeographic = (cs_PRJCOD_UNITY == m_csParameters->prj_code);
    int  epsgCode;

    // first see if it's an EPSG without doing the search through the other CS's for a match. That's quick.
    epsgCode = GetEPSGCode (true);

    // Check if an EPSG code was found. If it is not geographic then we go with it. If it is geographic
    // we impose that the EPSG code be a valid EPSG entry in the 4000ths (we tolerate Datum codes instead 6000ths of GCS codes as it happens frequently)
    if ( (0 != epsgCode) && (!isGeographic || ((epsgCode >= 6000) && (epsgCode < 7000)) || ((epsgCode >= 4000) && (epsgCode < 5000)) ))
        return true;

    // then do the relatively quick check to see if it's one we could save as user define if we have to.
    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_TRMER:
        case cs_PRJCOD_HOM1XY:
        case cs_PRJCOD_MRCAT:
        case cs_PRJCOD_MRCATK:
        case cs_PRJCOD_LM2SP:
        case cs_PRJCOD_LM1SP:
        case cs_PRJCOD_AZMEA:
        case cs_PRJCOD_ALBER:
        case cs_PRJCOD_AZMED:
        case cs_PRJCOD_EDCNC:
        case cs_PRJCOD_SSTRO:
        case cs_PRJCOD_PSTRO:
        case cs_PRJCOD_OSTRO:
        case cs_PRJCOD_EDCYL:
        case cs_PRJCOD_CSINI:
        case cs_PRJCOD_GNOMC:
        case cs_PRJCOD_MILLR:
        case cs_PRJCOD_ORTHO:
        case cs_PRJCOD_PLYCN:
        case cs_PRJCOD_ROBIN:
        case cs_PRJCOD_SINUS:
        case cs_PRJCOD_VDGRN:
        case cs_PRJCOD_NZLND:
        case cs_PRJCOD_SOTRM:
        case cs_PRJCOD_UNITY:
            if (CanSaveDatumToGeoTiffKeys())
                return true;
        }

#if defined (NOTNOW)
    // not obvious that it's EPSG, can't save as user defined. look for matching EPSG (time consuming).
    epsgCode = GetEPSGCode (false);
    if ( (0 != epsgCode) && (!isGeographic || ((epsgCode >= 6000) && (epsgCode < 7000) )))
        return true;
#endif

    // couldn't find a way to save as GeoTiff
    return false;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BaseGCS::~BaseGCS
(
)
    {



    if (m_destinationGCS != NULL)
        {
        m_destinationGCS->UnRegisterIsADestinationOf(*this);
        m_destinationGCS = NULL;
        }


    if (NULL != m_datumConverter)
        {
        m_datumConverter->Destroy();
        m_datumConverter = NULL;
        }

    // Clear the link between other BaseGCS to this one used as a cached destination
    for (size_t i = 0 ; i < m_listOfPointingGCS.size() ; i++)
        m_listOfPointingGCS[i]->ClearCache();

    m_listOfPointingGCS.clear();

    CSMAP_FREE_AND_CLEAR (m_csParameters);

    DELETE_AND_CLEAR (m_nameString);
    DELETE_AND_CLEAR (m_descriptionString);
    DELETE_AND_CLEAR (m_projectionString);
    DELETE_AND_CLEAR (m_datumNameString);
    DELETE_AND_CLEAR (m_datumDescriptionString);
    DELETE_AND_CLEAR (m_ellipsoidNameString);
    DELETE_AND_CLEAR (m_ellipsoidDescriptionString);

    DELETE_AND_CLEAR (m_originalWKT);

    m_originalGeoKeys.clear();
        
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
CSParameters*   BaseGCS::GetCSParameters () const
    {
    return m_csParameters;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
size_t          BaseGCS::GetCSParametersSize () const
    {
    return sizeof(*m_csParameters);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   02/07
+---------------+---------------+---------------+---------------+---------------+------*/
int             BaseGCS::Matches
(
char const * const * matchStrings,
int                  numMixedCase,
int                  numUpperCase,
bool                 anyWord
) const
    {
    // Given the library is NOT initialized ...
    if (!IsLibraryInitialized())
        {
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        return 0;
        }

    if ( (NULL == m_csParameters) || (numMixedCase <= 0) )
        return 0;

    // the number of mixed case should always equal or exceed the number of uppercase. Exceed happens when you enter something such that strupr(string).Equals(string).
    BeAssert (numMixedCase >= numUpperCase);

    char concatString[4096];
    strcpy (concatString, m_csParameters->csdef.key_nm);
    strcat (concatString, m_csParameters->csdef.dat_knm);
    strcat (concatString, m_csParameters->csdef.elp_knm);
    strcat (concatString, m_csParameters->csdef.desc_nm);
    strcat (concatString, m_csParameters->csdef.group);
    strcat (concatString, m_csParameters->csdef.locatn);
    strcat (concatString, m_csParameters->csdef.source);
    int epsgCode;
    if (0 != (epsgCode = GetEPSGCode (true)))
        {
        char epsgString[40];
        sprintf (epsgString, "%d", epsgCode);
        strcat (concatString, epsgString);
        }

    int score=0;

    if (anyWord)
        {
        // can match any word, so use a score based on the word position, and whether it matches the original or upper case.
        // first half of the input strings are the users typed-in case versions. Those are more valuable matches.
        int iScoreMultiple = numMixedCase;
        for (int iString=0; iString < numMixedCase; iString++, iScoreMultiple--)
            {
            char const* searchString        = matchStrings[iString];
            if (NULL != strstr (concatString, searchString))
                score += iScoreMultiple * 10;
            }

        // second half of the input strings are the upper case versions.
        BeStringUtilities::Strupr (concatString);
        iScoreMultiple = numMixedCase + 1;
        for (int iString=numMixedCase; iString < numMixedCase + numUpperCase; iString++, iScoreMultiple--)
            {
            char const* searchString        = matchStrings[iString];
            if (NULL != strstr (concatString, searchString))
                score += iScoreMultiple * 7;
            }
        }
    else
        {
        char upperCaseConcatString[4096];
        strcpy (upperCaseConcatString, concatString);
        BeStringUtilities::Strupr (upperCaseConcatString);

        // can match any word, so use a score based on the word position, and whether it matches the original or upper case.
        // first half of the input strings are the users typed-in case versions. Those are more valuable matches.
        int iScoreMultiple = numMixedCase + numUpperCase + 1;
        for (int iString=0; iString < numMixedCase; iString++, iScoreMultiple--)
            {
            char const* searchString            = matchStrings[iString];
            char const* upperCaseSearchString   = matchStrings[numMixedCase + iString];

            if (NULL != strstr (concatString, searchString))
                score += iScoreMultiple * 10;
            else if ( (iString < numUpperCase) && (NULL != strstr (upperCaseConcatString, upperCaseSearchString)) )
                score += iScoreMultiple * 7;
            else
                return 0;
            }
        return score;
        }

    BeAssert (score >= 0);
    return score;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BaseGCS::Validate
(
T_WStringVector&    errorList
) const
    {
    // Given the library is NOT initialized ...
    if (!IsLibraryInitialized())
        {
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        return false;
        }

    int csMapErrors[128];
    int errorCount;

    // check the datum or ellipsoid separately, because those might be coming from user libraries.
    errorCount = CS_cschk (&m_csParameters->csdef, 0, csMapErrors, DIM (csMapErrors));

    if (0 != m_csParameters->csdef.dat_knm[0])
        {
        CSDatumDef* datumDef;
        if (NULL == (datumDef = LibraryManager::Instance()->GetDatumDefFromGCS (*this)))
            csMapErrors[errorCount++] = cs_CSQ_INVDTM;
        else
            CSMap::CS_free (datumDef);
        }
    else if (0 != m_csParameters->csdef.elp_knm[0])
        {
        CSEllipsoidDef* ellipsoidDef;
        if (NULL == (ellipsoidDef = LibraryManager::Instance()->GetEllipsoidDefFromGCS (*this)))
            csMapErrors[errorCount++] = cs_CSQ_INVELP;
        else
            CSMap::CS_free (ellipsoidDef);
        }

    for (int iError=0; iError < errorCount; iError++)
        {
        WString thisError;
        errorList.push_back (GetErrorMessage (thisError, csMapErrors[iError]));
        }

    return (0 == errorCount);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BaseGCS::IsValid () const
    {
    return  ((NULL != m_csParameters) && (0 == m_csError));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BaseGCS::IsStandard () const
    {
    // Given the library is NOT initialized ...
    if (!IsLibraryInitialized())
        {
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        return false;
        }

    return  COORDSYS_KEYNM == m_coordSysId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
int             BaseGCS::GetError () const
    {
    return m_csError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         BaseGCS::GetErrorMessage (WStringR errorMsg) const
    {
    char    csErrorMsg[512];
    CSMap::CS_errmsg (csErrorMsg, DIM(csErrorMsg));
    errorMsg.AssignA (csErrorMsg);
    return errorMsg.c_str();
    }

#if defined (WIP_L10N)

static RscFileHandle    s_rscFileHandle;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
void            OpenResourceFile()
    {
#if defined (BENTLEY_WIN32)
    if (0 == s_rscFileHandle)
        {
        // get the dll instance
        WChar       dllFileName[512];
        WChar       resourceFileName[512];
        // initialze resourceFileName
        wcscpy (resourceFileName, L"baseGeoCoord.dll.mui");
        HINSTANCE dllHInstance = GetModuleHandleW (L"basegeocoord.dll");
        if (0 != GetModuleFileNameW (dllHInstance, dllFileName, _countof (dllFileName)))
            wcscpy (resourceFileName, dllFileName);

        if (SUCCESS != mdlResource_openFile (&s_rscFileHandle, resourceFileName, 0))
            s_rscFileHandle = -1;
        }
#else
    s_rscFileHandle = -1;
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            GetLocalizedErrorMessage
(
WStringR    message,
int         messageOffset,
bool        geoCoordError       // true for our error codes, false for CSMap error codes.
)
    {
    OpenResourceFile();

    if (-1 != s_rscFileHandle)
        {
        if (SUCCESS == s_geoCoordResources->GetString (message, messageOffset, MSGLISTID_GeoCoordErrors))
            return;
        }
    message.Sprintf (L"GeoCoord error %d\n", messageOffset + GeoCoordErrorBase);
    }

#else

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
void            GetLocalizedErrorMessage
(
WStringR    message,
int         messageOffset,
bool        geoCoordError       // true for our error codes, false for CSMap error codes.
)
    {
    message.Sprintf (L"GeoCoord error %d\n", messageOffset + GeoCoordErrorBase);
    }

#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         BaseGCS::GetErrorMessage (WStringR errorMsg, StatusInt  errorCode)
    {
    // this needs work for internationalization, and for handling non CSMap errors.
    errorMsg.clear();

    // the GeoCoordErrors are negative, starting at GeoCoordErrorBase and working to larger negative numbers.
    if ( (errorCode < GeoCoordErrorBase) && (errorCode > GeoCoordErrorEnd) )
        GetLocalizedErrorMessage (errorMsg, GeoCoordErrorBase - errorCode, true);
    else if (cs_CNVRT_RNG == errorCode)
        GetLocalizedErrorMessage (errorMsg, GeoCoordErrorBase - GEOCOORDERR_CoordinateRange, true);
    else
        {
        char msg[1024];
        CSMap::CSerpt(msg, DIM(msg), errorCode);
        errorMsg.AssignA (msg);
        }

    return errorMsg.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP        BaseGCS::GetName () const
    {
    if (NULL == m_csParameters)
        return NULL;

    if (NULL == m_nameString)
        m_nameString = new WString (m_csParameters->csdef.key_nm,false);

    return m_nameString->c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetName (WCharCP name)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);
	
    CS_stncp (m_csParameters->csdef.key_nm, AString(name).c_str(), sizeof (m_csParameters->csdef.key_nm));

    if (NULL != m_nameString)
        m_nameString->assign (name);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP     BaseGCS::GetDescription () const
    {
    if (NULL == m_csParameters)
        return NULL;

    if (NULL == m_descriptionString)
        m_descriptionString = new WString (m_csParameters->csdef.desc_nm,false);

    return  m_descriptionString->c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetDescription (WCharCP description)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;
		
    SetModified(true);

    CS_stncp (m_csParameters->csdef.desc_nm, AString(description).c_str(), sizeof (m_csParameters->csdef.desc_nm));

    if (NULL != m_descriptionString)
        m_descriptionString->assign (description);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP    BaseGCS::GetProjection () const
    {
    if (NULL == m_csParameters)
        return NULL;

    if (NULL == m_projectionString)
        m_projectionString = new WString (m_csParameters->csdef.prj_knm,false);

    return  m_projectionString->c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
BaseGCS::ProjectionCodeValue  BaseGCS::GetProjectionCode () const
    {
    if (NULL != m_csParameters)
        return (BaseGCS::ProjectionCodeValue) m_csParameters->prj_code;
    else
        return pcvInvalid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
int         BaseGCS::SetProjectionCode
(
BaseGCS::ProjectionCodeValue  value
)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;
		
    SetModified(true);		

    int     oldCode = m_csParameters->prj_code;

    // We want to set both the prj_code and the prj_knm
    // make sure we can find the projection code in the table.
    const struct cs_Prjtab_   *projection;
    for (projection = cs_Prjtab; 0 != projection->key_nm[0]; projection++)
        {
        if (projection->code == value)
            {
            m_csParameters->prj_code = value;
            CS_stncp (m_csParameters->csdef.prj_knm, projection->key_nm, sizeof (m_csParameters->csdef.prj_knm));
            break;
            }
        }
    // if we go to the end without finding the code, error.
    if (0 == projection->key_nm[0])
        return GEOCOORDERR_InvalidCoordinateCode;

    // Now determine what to do with the parameters in the 'new' projection.


    // look at the flags to determine what "standard" parameters aren't used, and zero those out.
    // no false origin if cs_PRJFLG_ORGFLS is set.
    if (0 != (projection->flags & cs_PRJFLG_ORGFLS))
        {
        m_csParameters->csdef.x_off = 0.0;
        m_csParameters->csdef.y_off = 0.0;
        }

    // scale reduction supported only if cs_PRJFLG_SCLRED set.
    if ( (0 == (projection->flags & cs_PRJFLG_SCLRED)) || (0.0 == m_csParameters->csdef.scl_red) )
        m_csParameters->csdef.scl_red = 1.0;

    // no origin latitude if cs_PRJFLG_ORGLAT is set.
    if (0 != (projection->flags & cs_PRJFLG_ORGLAT))
        m_csParameters->csdef.org_lat = 0.0;

    // no origin longitude if cs_PRJFLG_ORGLNF is set.
    if (0 != (projection->flags & cs_PRJFLG_ORGLNG))
        m_csParameters->csdef.org_lng = 0.0;


    // find the old an new cs_PrjprmMap_ structure
    struct cs_PrjprmMap_ *oldParamMap = NULL;
    struct cs_PrjprmMap_ *newParamMap = NULL;
    struct cs_PrjprmMap_ *mp;
    for (mp = cs_PrjprmMap; mp->prj_code != cs_PRJCOD_END; mp++)
        {
        if (mp->prj_code == m_csParameters->prj_code)
            newParamMap = mp;
        if (mp->prj_code == oldCode)
            oldParamMap = mp;
        if ( (newParamMap != NULL) && (oldParamMap != NULL) )
            break;
        }

    // shouldn't happen.
    if ( (newParamMap == NULL) || (oldParamMap == NULL) )
        return SUCCESS;

    // step through the paramMaps
    double *paramP;
    int     iParam;
    for (iParam=0, paramP = &m_csParameters->csdef.prj_prm1; iParam <24; iParam++, paramP++)
        {
        int oldParamIndex = oldParamMap->prm_types[iParam];
        int newParamIndex = newParamMap->prm_types[iParam];
        // zero out any that are unused, or completely different between the old an new projection.
        if ( (0 == oldParamIndex) || (0 == newParamIndex))
            *paramP = 0.0;
        else
            {
            // both used. Keep if there's a chance it's usable.
            // That only happens if the log_type is useful.
            cs_Prjprm_  *oldParamDef = &csPrjprm[oldParamIndex];
            cs_Prjprm_  *newParamDef = &csPrjprm[newParamIndex];
            if (oldParamDef->log_type != newParamDef->log_type)
                *paramP = 0.0;
            }
        }

    // string now wrong.
    DELETE_AND_CLEAR (m_projectionString);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP    BaseGCS::GetGroup (WStringR groupName) const
    {
    groupName.clear();

    if (NULL != m_csParameters)
        groupName.AssignA (m_csParameters->csdef.group);

    return groupName.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt  BaseGCS::SetGroup (WCharCP group)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;
		
    SetModified(true);		

    // We cannot set an arbitrary name ... it must be an existing group
    GroupEnumerator* theGroupEnumerator = Group::GetGroupEnumerator();
    bool found = false;
    while (!found && theGroupEnumerator->MoveNext())
        {
        found = (0 == wcscmp (group, theGroupEnumerator->GetCurrent()->GetName()));
        }

    if (!found)
        return GEOCOORDERR_BadArg;

    CSMap::CS_stncp (m_csParameters->csdef.group, AString(group).c_str(), DIM(m_csParameters->csdef.group));

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         BaseGCS::GetLocation (WStringR location) const
    {
    if (NULL != m_csParameters)
        location.AssignA (m_csParameters->csdef.locatn);
    else
        location.clear();

    return location.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         BaseGCS::GetSource (WStringR source) const
    {
    if (NULL != m_csParameters)
        source.AssignA (m_csParameters->csdef.source);
    else
        source.clear();

    return source.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetSource (WCharCP source)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;
		
    SetModified(true);		

    CSMap::CS_stncp (m_csParameters->csdef.source, AString(source).c_str(), DIM(m_csParameters->csdef.source));

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         BaseGCS::GetUnits (WStringR units) const
    {
    if (NULL != m_csParameters)
        units.AssignA (m_csParameters->csdef.unit);
    else
        units.clear();

    return units.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
int         BaseGCS::GetUnitCode () const
    {
    if (NULL == m_csParameters)
        return -1;

    const struct cs_Unittab_   *pUnit;
    int                         index;
    for (index = 0, pUnit = cs_Unittab; cs_UTYP_END != pUnit->type; pUnit++)
        {
        if (0 == BeStringUtilities::Stricmp (m_csParameters->csdef.unit, pUnit->name))
            return index;
        index++;
        }
    return -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu St-Pierre   01/08
+---------------+---------------+---------------+---------------+---------------+------*/
int         BaseGCS::GetEPSGUnitCode () const
    {
    int UnitCode     = GetUnitCode();

    if (UnitCode != -1)
        return cs_Unittab[UnitCode].epsgCode;
    else
        return 0;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetUnitCode
(
int     code
)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;
		
    SetModified(true);


    const struct cs_Unittab_   *pUnit;
    int                         index;
    for (index = 0, pUnit = cs_Unittab; cs_UTYP_END != pUnit->type; pUnit++)
        {
        if (pUnit->type == cs_UTYP_LEN)
            {
            if (code == index)
                {
                CSMap::CS_stncp (m_csParameters->csdef.unit, pUnit->name, DIM (m_csParameters->csdef.unit));
                return SUCCESS;
                }
            index++;
            }
        /* If the unit type is not length then it must be angular and can only be used to set klat/long geographic coordinate systems */
        else if (cs_PRJCOD_UNITY == m_csParameters->prj_code)
            {
            if (code == index)
                {
                CSMap::CS_stncp (m_csParameters->csdef.unit, pUnit->name, DIM (m_csParameters->csdef.unit));
                return SUCCESS;
                }
            index++;
            }
        /* If the unit type is not length then it must be angular and can only be used to set klat/long geographic coordinate systems */
        else if (cs_PRJCOD_UNITY == m_csParameters->prj_code)
            {
            if (code == index)
                {
                CSMap::CS_stncp (m_csParameters->csdef.unit, pUnit->name, DIM (m_csParameters->csdef.unit));
                return SUCCESS;
                }
            index++;
            }
        }
    return GEOCOORDERR_InvalidUnitCode;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetOriginLatitude () const
    {
    if (NULL != m_csParameters)
        return m_csParameters->csdef.org_lat;
    else
        return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetOriginLatitude (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;
		
    SetModified(true);


    m_csParameters->csdef.org_lat = value;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetOriginLongitude () const
    {
    if (NULL != m_csParameters)
        return m_csParameters->csdef.org_lng;
    else
        return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetOriginLongitude (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);


    m_csParameters->csdef.org_lng = value;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetStandardParallel1 () const
    {
    if (NULL == m_csParameters)
        return 0.0;

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_ALBER:
        case cs_PRJCOD_EDCNC:
        case cs_PRJCOD_EDCYL:
        case cs_PRJCOD_EDCYLE:
        case cs_PRJCOD_NACYL:
        case cs_PRJCOD_LM2SP:
        case cs_PRJCOD_LMBLG:
        case cs_PRJCOD_LMBRTAF:
        case cs_PRJCOD_WCCSL:
        case cs_PRJCOD_MNDOTL:
        case cs_PRJCOD_PSTROSL:
        case cs_PRJCOD_OBQCYL:      // We use StandardParallel1 to get the latitude where the Gaussian sphere is calculated.
        case cs_PRJCOD_WINKL:       // called reference parallel in release notes for 11.13.
            return m_csParameters->csdef.prj_prm1;

        case cs_PRJCOD_MRCAT:
            return m_csParameters->csdef.prj_prm2;

        case cs_PRJCOD_KROVAK:
        case cs_PRJCOD_KROVK1:
        case cs_PRJCOD_KRVK95:
        case cs_PRJCOD_KRVK951:
        case cs_PRJCOD_MODPC:
            return m_csParameters->csdef.prj_prm3;
        }
    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetStandardParallel1 (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_ALBER:
        case cs_PRJCOD_EDCNC:
        case cs_PRJCOD_EDCYL:
        case cs_PRJCOD_EDCYLE:
        case cs_PRJCOD_NACYL:
        case cs_PRJCOD_LM2SP:
        case cs_PRJCOD_LMBLG:
        case cs_PRJCOD_LMBRTAF:
        case cs_PRJCOD_WCCSL:
        case cs_PRJCOD_MNDOTL:
        case cs_PRJCOD_PSTROSL:
        case cs_PRJCOD_OBQCYL:      // We use StandardParallel1 to get the latitude where the Gaussian sphere is calculated.
        case cs_PRJCOD_WINKL:       // called reference parallel in release notes for 11.13.
            m_csParameters->csdef.prj_prm1 = value;
            return SUCCESS;

        case cs_PRJCOD_MRCAT:
            m_csParameters->csdef.prj_prm2 = value;
            return SUCCESS;

        case cs_PRJCOD_KROVAK:
        case cs_PRJCOD_KROVK1:
        case cs_PRJCOD_KRVK95:
        case cs_PRJCOD_KRVK951:
        case cs_PRJCOD_MODPC:
            m_csParameters->csdef.prj_prm3 = value;
            return SUCCESS;
        }
    return GEOCOORDERR_ProjectionDoesntUseParameter;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetStandardParallel2 () const
    {
    if (NULL == m_csParameters)
        return 0.0;

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_ALBER:
        case cs_PRJCOD_EDCNC:
        case cs_PRJCOD_LM2SP:
        case cs_PRJCOD_LMBLG:
        case cs_PRJCOD_LMBRTAF:
        case cs_PRJCOD_WCCSL:
        case cs_PRJCOD_MNDOTL:
            return m_csParameters->csdef.prj_prm2;

        case cs_PRJCOD_MODPC:
            return m_csParameters->csdef.prj_prm4;
        }
    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetStandardParallel2 (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_ALBER:
        case cs_PRJCOD_EDCNC:
        case cs_PRJCOD_LM2SP:
        case cs_PRJCOD_LMBLG:
        case cs_PRJCOD_LMBRTAF:
        case cs_PRJCOD_WCCSL:
        case cs_PRJCOD_MNDOTL:
            m_csParameters->csdef.prj_prm2 = value;
            return SUCCESS;

        case cs_PRJCOD_MODPC:
            m_csParameters->csdef.prj_prm4 = value;
            return SUCCESS;
        }
    return GEOCOORDERR_ProjectionDoesntUseParameter;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetFalseEasting () const
    {
    if (NULL != m_csParameters)
        return m_csParameters->csdef.x_off;
    else
        return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetFalseEasting (double value)
    {

    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    m_csParameters->csdef.x_off = value;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetFalseNorthing () const
    {
    if (NULL != m_csParameters)
        return m_csParameters->csdef.y_off;
    else
        return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetFalseNorthing (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    m_csParameters->csdef.y_off = value;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetScaleReduction () const
    {
    if (NULL != m_csParameters)
        return m_csParameters->csdef.scl_red;
    else
        return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetScaleReduction (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    m_csParameters->csdef.scl_red = value;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetMinimumLongitude () const
    {
    if (NULL == m_csParameters)
        return 0.0;

    if (cs_PRJCOD_UNITY == m_csParameters->prj_code)
        return m_csParameters->csdef.prj_prm1;
    else
        return m_csParameters->csdef.ll_min[0];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetMinimumLongitude (double value)
    {

    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    if (cs_PRJCOD_UNITY == m_csParameters->prj_code)
        m_csParameters->csdef.prj_prm1 = value;
    else
        m_csParameters->csdef.ll_min[0] = value;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetMaximumLongitude () const
    {
    if (NULL == m_csParameters)
        return 0.0;

    if (cs_PRJCOD_UNITY == m_csParameters->prj_code)
        return m_csParameters->csdef.prj_prm2;
    else
        return m_csParameters->csdef.ll_max[0];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetMaximumLongitude (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    if (cs_PRJCOD_UNITY == m_csParameters->prj_code)
        m_csParameters->csdef.prj_prm2 = value;
    else
        m_csParameters->csdef.ll_max[0] = value;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetMinimumLatitude () const
    {
    if (NULL == m_csParameters)
        return 0.0;
    else if ((cs_PRJCOD_UNITY == m_csParameters->prj_code) && 
             (m_csParameters->csdef.ll_max[1] <= m_csParameters->csdef.ll_min[1]))
        return -90.0;
    else
        return m_csParameters->csdef.ll_min[1];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetMinimumLatitude (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    m_csParameters->csdef.ll_min[1] = value;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetMaximumLatitude () const
    {
    if (NULL == m_csParameters)
        return 0.0;
    else if ((cs_PRJCOD_UNITY == m_csParameters->prj_code) && 
             (m_csParameters->csdef.ll_max[1] <= m_csParameters->csdef.ll_min[1]))
        return 90.0;
    else
        return m_csParameters->csdef.ll_max[1];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetMaximumLatitude (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    m_csParameters->csdef.ll_max[1] = value;

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
double          BaseGCS::GetMinimumUsefulLongitude() const
    {
    if (NULL == m_csParameters)
        return 0.0;
    else
        {
        // Often, the value -180 (or 180 West) is stored as +180 which makes no sense as minimum ... we convert it to -180
        // The exact float compare operation is here intentional
        if (180.0 == m_csParameters->min_ll[0])
            return m_csParameters->cent_mer - 179.99999999;

        return m_csParameters->cent_mer + m_csParameters->min_ll[0];
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
double          BaseGCS::GetMaximumUsefulLongitude() const
    {
    if (NULL == m_csParameters)
        return 0.0;
    else
        return m_csParameters->cent_mer + m_csParameters->max_ll[0];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
double          BaseGCS::GetMinimumUsefulLatitude() const
    {
    if (NULL == m_csParameters)
        return 0.0;
    else
        return m_csParameters->min_ll[1];
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
double          BaseGCS::GetMaximumUsefulLatitude() const
    {
    if (NULL == m_csParameters)
        return 0.0;
    else
        return m_csParameters->max_ll[1];
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetCentralMeridian () const
    {
    if (NULL == m_csParameters)
        return 0.0;

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_TRMER:
        case cs_PRJCOD_TRMRKRG:
        case cs_PRJCOD_GAUSSK:
        case cs_PRJCOD_SOTRM:
        case cs_PRJCOD_WCCST:
        case cs_PRJCOD_MNDOTT:
        case cs_PRJCOD_TRMERAF:
        case cs_PRJCOD_TRMRS:
        case cs_PRJCOD_PLYCN:
        case cs_PRJCOD_CSINI:
        case cs_PRJCOD_MRCAT:
        case cs_PRJCOD_MRCATK:
        case cs_PRJCOD_MRCATPV:
        case cs_PRJCOD_MILLR:
        case cs_PRJCOD_MODPC:
#if defined (TOTAL_SPECIAL)
        /* TOTAL Transverse Mercator projection, using the Bernard Flaceliere calculation. (Added by BJB 3/2007). */
        case cs_PRJCOD_TRMERBF:
#endif
            return m_csParameters->csdef.prj_prm1;

        case cs_PRJCOD_SINUS:
        case cs_PRJCOD_ROBIN:
        case cs_PRJCOD_NACYL:
            return m_csParameters->csdef.org_lng;
        }
    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetCentralMeridian (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_TRMER:
        case cs_PRJCOD_TRMRKRG:
        case cs_PRJCOD_GAUSSK:
        case cs_PRJCOD_SOTRM:
        case cs_PRJCOD_WCCST:
        case cs_PRJCOD_MNDOTT:
        case cs_PRJCOD_TRMERAF:
        case cs_PRJCOD_TRMRS:
        case cs_PRJCOD_PLYCN:
        case cs_PRJCOD_CSINI:
        case cs_PRJCOD_MRCAT:
        case cs_PRJCOD_MRCATK:
        case cs_PRJCOD_MRCATPV:
        case cs_PRJCOD_MILLR:
        case cs_PRJCOD_MODPC:
#if defined (TOTAL_SPECIAL)
        /* TOTAL Transverse Mercator projection, using the Bernard Flaceliere calculation. (Added by BJB 3/2007). */
        case cs_PRJCOD_TRMERBF:
#endif
            m_csParameters->csdef.prj_prm1 = value;
            return SUCCESS;

        case cs_PRJCOD_SINUS:
        case cs_PRJCOD_ROBIN:
        case cs_PRJCOD_NACYL:
            m_csParameters->csdef.org_lng = value;
            return SUCCESS;
        }
    return GEOCOORDERR_ProjectionDoesntUseParameter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetEasternMeridian () const
    {
    if (NULL == m_csParameters)
        return 0.0;

    int prjCode = m_csParameters->prj_code;
    if (cs_PRJCOD_MODPC == prjCode)
        return m_csParameters->csdef.prj_prm2;
    else
        return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetEasternMeridian (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    int prjCode = m_csParameters->prj_code;
    if (cs_PRJCOD_MODPC == prjCode)
        {
        m_csParameters->csdef.prj_prm2 = value;
        return SUCCESS;
        }
    return GEOCOORDERR_ProjectionDoesntUseParameter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetCentralPointLongitude () const
    {
    if (NULL == m_csParameters)
        return 0.0;

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_HOM1UV:
        case cs_PRJCOD_HOM1XY:
        case cs_PRJCOD_MNDOTOBL:
        case cs_PRJCOD_RSKEW:
        case cs_PRJCOD_RSKEWO:
        case cs_PRJCOD_RSKEWC:
            return m_csParameters->csdef.prj_prm1;
        }
    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetCentralPointLongitude (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_HOM1UV:
        case cs_PRJCOD_HOM1XY:
        case cs_PRJCOD_MNDOTOBL:
        case cs_PRJCOD_RSKEW:
        case cs_PRJCOD_RSKEWO:
        case cs_PRJCOD_RSKEWC:
            m_csParameters->csdef.prj_prm1 = value;
            return SUCCESS;
        }

    return GEOCOORDERR_ProjectionDoesntUseParameter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetCentralPointLatitude () const
    {
    if (NULL == m_csParameters)
        return 0.0;

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_HOM1UV:
        case cs_PRJCOD_HOM1XY:
        case cs_PRJCOD_MNDOTOBL:
        case cs_PRJCOD_RSKEW:
        case cs_PRJCOD_RSKEWO:
        case cs_PRJCOD_RSKEWC:
            return m_csParameters->csdef.prj_prm2;
        }
    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetCentralPointLatitude (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_HOM1UV:
        case cs_PRJCOD_HOM1XY:
        case cs_PRJCOD_MNDOTOBL:
        case cs_PRJCOD_RSKEW:
        case cs_PRJCOD_RSKEWO:
        case cs_PRJCOD_RSKEWC:
            m_csParameters->csdef.prj_prm2 = value;
            return SUCCESS;
        }

    return GEOCOORDERR_ProjectionDoesntUseParameter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetPoint1Longitude () const
    {
    if (NULL == m_csParameters)
        return 0.0;

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_HOM2UV:
        case cs_PRJCOD_HOM2XY:
        case cs_PRJCOD_KROVAK:
        case cs_PRJCOD_KROVK1:
        case cs_PRJCOD_KRVK95:
        case cs_PRJCOD_KRVK951:
            return m_csParameters->csdef.prj_prm1;
        }
    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetPoint1Longitude (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_HOM2UV:
        case cs_PRJCOD_HOM2XY:
        case cs_PRJCOD_KROVAK:
        case cs_PRJCOD_KROVK1:
        case cs_PRJCOD_KRVK95:
        case cs_PRJCOD_KRVK951:
            m_csParameters->csdef.prj_prm1 = value;
            return SUCCESS;
        }

    return GEOCOORDERR_ProjectionDoesntUseParameter;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetPoint1Latitude () const
    {
    if (NULL == m_csParameters)
        return 0.0;

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_HOM2UV:
        case cs_PRJCOD_HOM2XY:
        case cs_PRJCOD_KROVAK:
        case cs_PRJCOD_KROVK1:
        case cs_PRJCOD_KRVK95:
        case cs_PRJCOD_KRVK951:
            return m_csParameters->csdef.prj_prm2;
        }
    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetPoint1Latitude (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_HOM2UV:
        case cs_PRJCOD_HOM2XY:
        case cs_PRJCOD_KROVAK:
        case cs_PRJCOD_KROVK1:
        case cs_PRJCOD_KRVK95:
        case cs_PRJCOD_KRVK951:
            m_csParameters->csdef.prj_prm2 = value;
            return SUCCESS;
        }

    return GEOCOORDERR_ProjectionDoesntUseParameter;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetPoint2Longitude () const
    {
    if (NULL == m_csParameters)
        return 0.0;

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_HOM2UV:
        case cs_PRJCOD_HOM2XY:
            return m_csParameters->csdef.prj_prm3;
        }
    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetPoint2Longitude (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_HOM2UV:
        case cs_PRJCOD_HOM2XY:
            m_csParameters->csdef.prj_prm3 = value;
            return SUCCESS;
        }

    return GEOCOORDERR_ProjectionDoesntUseParameter;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetPoint2Latitude () const
    {
    if (NULL == m_csParameters)
        return 0.0;

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_HOM2UV:
        case cs_PRJCOD_HOM2XY:
            return m_csParameters->csdef.prj_prm4;
        }
    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetPoint2Latitude (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_HOM2UV:
        case cs_PRJCOD_HOM2XY:
            m_csParameters->csdef.prj_prm4 = value;
            return SUCCESS;
        }

    return GEOCOORDERR_ProjectionDoesntUseParameter;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetAzimuth () const
    {
    if (NULL == m_csParameters)
        return 0.0;

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_AZMEA:
        case cs_PRJCOD_AZMED:
        case cs_PRJCOD_AZEDE:
        case cs_PRJCOD_SSTRO:
            return m_csParameters->csdef.prj_prm1;

        case cs_PRJCOD_HOM1UV:
        case cs_PRJCOD_HOM1XY:
        case cs_PRJCOD_MNDOTOBL:
        case cs_PRJCOD_RSKEW:
        case cs_PRJCOD_RSKEWO:
        case cs_PRJCOD_RSKEWC:
            return m_csParameters->csdef.prj_prm3;
        }
    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetAzimuth (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_AZMEA:
        case cs_PRJCOD_AZMED:
        case cs_PRJCOD_AZEDE:
        case cs_PRJCOD_SSTRO:
            m_csParameters->csdef.prj_prm1 = value;
            return SUCCESS;

        case cs_PRJCOD_HOM1UV:
        case cs_PRJCOD_HOM1XY:
        case cs_PRJCOD_MNDOTOBL:
        case cs_PRJCOD_RSKEW:
        case cs_PRJCOD_RSKEWO:
        case cs_PRJCOD_RSKEWC:
            m_csParameters->csdef.prj_prm3 = value;
            return SUCCESS;
        }

    return GEOCOORDERR_ProjectionDoesntUseParameter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetGeoidSeparation () const
    {
    if (NULL == m_csParameters)
        return 0.0;

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_WCCST:
            return m_csParameters->csdef.prj_prm2;
        case cs_PRJCOD_WCCSL:
            return m_csParameters->csdef.prj_prm3;
        }
    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetGeoidSeparation (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_WCCST:
            m_csParameters->csdef.prj_prm2 = value;
            return SUCCESS;
        case cs_PRJCOD_WCCSL:
            m_csParameters->csdef.prj_prm3 = value;
            return SUCCESS;
        }

    return GEOCOORDERR_ProjectionDoesntUseParameter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetElevationAboveGeoid () const
    {
    if (NULL == m_csParameters)
        return 0.0;

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_MNDOTT:
        case cs_PRJCOD_AZEDE:
            return m_csParameters->csdef.prj_prm2;

        case cs_PRJCOD_WCCST:
        case cs_PRJCOD_MNDOTL:
            return m_csParameters->csdef.prj_prm3;

        case cs_PRJCOD_WCCSL:
        case cs_PRJCOD_MNDOTOBL:
            return m_csParameters->csdef.prj_prm4;
        }
    return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetElevationAboveGeoid (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_MNDOTT:
        case cs_PRJCOD_AZEDE:
            m_csParameters->csdef.prj_prm2 = value;
            return SUCCESS;

        case cs_PRJCOD_WCCST:
        case cs_PRJCOD_MNDOTL:
            m_csParameters->csdef.prj_prm3 = value;
            return SUCCESS;

        case cs_PRJCOD_WCCSL:
        case cs_PRJCOD_MNDOTOBL:
            m_csParameters->csdef.prj_prm4 = value;
            return SUCCESS;
        }

    return GEOCOORDERR_ProjectionDoesntUseParameter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
int BaseGCS::GetUTMZone () const
    {
    if (NULL == m_csParameters)
        return -1;

    int prjCode = m_csParameters->prj_code;
#if defined(TOTAL_SPECIAL)
    if ((cs_PRJCOD_UTM == prjCode) || (cs_PRJCOD_UTMZNBF == prjCode))
#else
    if (cs_PRJCOD_UTM == prjCode)
#endif
        return (int) m_csParameters->csdef.prj_prm1;

    return -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetUTMZone (int value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    int prjCode = m_csParameters->prj_code;
#if defined(TOTAL_SPECIAL)
    if ((cs_PRJCOD_UTM == prjCode) || (cs_PRJCOD_UTMZNBF == prjCode))
#else
    if (cs_PRJCOD_UTM == prjCode)
#endif
        {
        m_csParameters->csdef.prj_prm1 = value;
        return SUCCESS;
        }

    return GEOCOORDERR_ProjectionDoesntUseParameter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
int BaseGCS::GetHemisphere () const
    {
    if (NULL == m_csParameters)
        return 0;

    int prjCode = m_csParameters->prj_code;
#if defined(TOTAL_SPECIAL)
    if ((cs_PRJCOD_UTM == prjCode) || (cs_PRJCOD_UTMZNBF == prjCode))
#else
    if (cs_PRJCOD_UTM == prjCode)
#endif
        return (int) m_csParameters->csdef.prj_prm2;

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetHemisphere (int value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    int prjCode = m_csParameters->prj_code;
#if defined(TOTAL_SPECIAL)
    if ((cs_PRJCOD_UTM == prjCode) || (cs_PRJCOD_UTMZNBF == prjCode))
#else
    if (cs_PRJCOD_UTM == prjCode)
#endif
    {
        m_csParameters->csdef.prj_prm2 = value;
        return SUCCESS;
        }

    return GEOCOORDERR_ProjectionDoesntUseParameter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
int BaseGCS::GetQuadrant () const
    {
    if (NULL == m_csParameters)
        return -1;

    // south oriented transverse mercator are already inverted, account for that. See cs_QuadMap and cs_QuadMapSO in csdata.c
    if (cs_PRJCOD_SOTRM == m_csParameters->prj_code)
        {
        switch (m_csParameters->csdef.quad)
            {
            case 0:
            case 1:
                return 3;
            case 2:
                return 4;
            case 3:
                return 1;
            case 4:
                return 2;
            }
        }

    return m_csParameters->csdef.quad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetQuadrant (short value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    // south oriented transverse mercator needs to be inverted, account for that. See cs_QuadMap and cs_QuadMapSO in csdata.c
    if (cs_PRJCOD_SOTRM == m_csParameters->prj_code)
        {
        switch (value)
            {
            case 0:
            case 1:
                value = 3;
                break;
            case 2:
                value = 4;
                break;
            case 3:
                value = 1;
                break;
            case 4:
                value = 2;
                break;
            }
        }

    m_csParameters->csdef.quad = value;
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
int BaseGCS::GetDanishSys34Region () const
    {
    if (NULL == m_csParameters)
        return -1;

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_SYS34:
        case cs_PRJCOD_SYS34_99:
        case cs_PRJCOD_SYS34_01:
            return (int) m_csParameters->csdef.prj_prm1;
        }

    return -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetDanishSys34Region (int value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_SYS34:
        case cs_PRJCOD_SYS34_99:
        case cs_PRJCOD_SYS34_01:
            m_csParameters->csdef.prj_prm1 = value;
            return SUCCESS;
        }

    return GEOCOORDERR_ProjectionDoesntUseParameter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetAffineA0 () const
    {
    if (NULL == m_csParameters)
        return 0.0;

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_TRMERAF:
            return m_csParameters->csdef.prj_prm2;

        case cs_PRJCOD_LMBRTAF:
            return m_csParameters->csdef.prj_prm3;
        }

    return 0.01;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetAffineA0 (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_TRMERAF:
            m_csParameters->csdef.prj_prm2 = value;
            return SUCCESS;

        case cs_PRJCOD_LMBRTAF:
            m_csParameters->csdef.prj_prm3 = value;
            return SUCCESS;
        }

    return GEOCOORDERR_ProjectionDoesntUseParameter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetAffineA1 () const
    {
    if (NULL == m_csParameters)
        return 0.0;

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_TRMERAF:
            return m_csParameters->csdef.prj_prm4;

        case cs_PRJCOD_LMBRTAF:
            return m_csParameters->csdef.prj_prm5;
        }

    return 0.01;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetAffineA1 (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_TRMERAF:
            m_csParameters->csdef.prj_prm4 = value;
            return SUCCESS;

        case cs_PRJCOD_LMBRTAF:
            m_csParameters->csdef.prj_prm5 = value;
            return SUCCESS;
        }

    return GEOCOORDERR_ProjectionDoesntUseParameter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetAffineA2 () const
    {
    if (NULL == m_csParameters)
        return 0.0;

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_TRMERAF:
            return m_csParameters->csdef.prj_prm5;

        case cs_PRJCOD_LMBRTAF:
            return m_csParameters->csdef.prj_prm6;
        }

    return 0.01;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetAffineA2 (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_TRMERAF:
            m_csParameters->csdef.prj_prm5 = value;
            return SUCCESS;

        case cs_PRJCOD_LMBRTAF:
            m_csParameters->csdef.prj_prm6 = value;
            return SUCCESS;
        }

    return GEOCOORDERR_ProjectionDoesntUseParameter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetAffineB0 () const
    {
    if (NULL == m_csParameters)
        return 0.0;

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_TRMERAF:
            return m_csParameters->csdef.prj_prm3;

        case cs_PRJCOD_LMBRTAF:
            return m_csParameters->csdef.prj_prm4;
        }

    return 0.01;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetAffineB0 (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_TRMERAF:
            m_csParameters->csdef.prj_prm3 = value;
            return SUCCESS;

        case cs_PRJCOD_LMBRTAF:
            m_csParameters->csdef.prj_prm4 = value;
            return SUCCESS;
        }

    return GEOCOORDERR_ProjectionDoesntUseParameter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetAffineB1 () const
    {
    if (NULL == m_csParameters)
        return 0.0;

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_TRMERAF:
            return m_csParameters->csdef.prj_prm6;

        case cs_PRJCOD_LMBRTAF:
            return m_csParameters->csdef.prj_prm7;
        }

    return 0.01;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetAffineB1 (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_TRMERAF:
            m_csParameters->csdef.prj_prm6 = value;
            return SUCCESS;

        case cs_PRJCOD_LMBRTAF:
            m_csParameters->csdef.prj_prm7 = value;
            return SUCCESS;
        }

    return GEOCOORDERR_ProjectionDoesntUseParameter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetAffineB2 () const
    {
    if (NULL == m_csParameters)
        return 0.0;

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_TRMERAF:
            return m_csParameters->csdef.prj_prm7;

        case cs_PRJCOD_LMBRTAF:
            return m_csParameters->csdef.prj_prm8;
        }

    return 0.01;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   BaseGCS::SetAffineB2 (double value)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_TRMERAF:
            m_csParameters->csdef.prj_prm7 = value;
            return SUCCESS;

        case cs_PRJCOD_LMBRTAF:
            m_csParameters->csdef.prj_prm8 = value;
            return SUCCESS;
        }

    return GEOCOORDERR_ProjectionDoesntUseParameter;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Alain.Robert   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
void BaseGCS::GetAffineParameters (double* A0, double* A1, double* A2, double* B0, double* B1, double* B2) const
    {
    if (NULL == m_csParameters)
        return;

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_TRMERAF:
            if (NULL != A0)
                *A0 = m_csParameters->csdef.prj_prm2;
            if (NULL != B0)
                *B0 = m_csParameters->csdef.prj_prm3;
            if (NULL != A1)
                *A1 = m_csParameters->csdef.prj_prm4;
            if (NULL != A2)
                *A2 = m_csParameters->csdef.prj_prm5;
            if (NULL != B1)
                *B1 = m_csParameters->csdef.prj_prm6;
            if (NULL != B2)
                *B2 = m_csParameters->csdef.prj_prm7;
            return;


        case cs_PRJCOD_LMBRTAF:
            if (NULL != A0)
                *A0 = m_csParameters->csdef.prj_prm3;
            if (NULL != B0)
                *B0 = m_csParameters->csdef.prj_prm4;
            if (NULL != A1)
                *A1 = m_csParameters->csdef.prj_prm5;
            if (NULL != A2)
                *A2 = m_csParameters->csdef.prj_prm6;
            if (NULL != B1)
                *B1 = m_csParameters->csdef.prj_prm7;
            if (NULL != B2)
                *B2 = m_csParameters->csdef.prj_prm8;
            return;
        }
    return;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Alain.Robert   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BaseGCS::SetAffineParameters (double A0, double A1, double A2, double B0, double B1, double B2)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    double  determinant = (A1 * B2) - (A2 * B1);
    // The exact floating point compare operation is intentional ... the purpose is to check that
    // no two members of same line or column of matrix are exactly equal to 0.0 which would result
    // in an invalid transformation.
    if (determinant == 0.0)
        return GEOCOORDERR_InvalidAffineParameters;

    switch (m_csParameters->prj_code)
        {
        case cs_PRJCOD_TRMERAF:
            m_csParameters->csdef.prj_prm2 = A0;
            m_csParameters->csdef.prj_prm3 = B0;
            m_csParameters->csdef.prj_prm4 = A1;
            m_csParameters->csdef.prj_prm5 = A2;
            m_csParameters->csdef.prj_prm6 = B1;
            m_csParameters->csdef.prj_prm7 = B2;
            return SUCCESS;

        case cs_PRJCOD_LMBRTAF:
            m_csParameters->csdef.prj_prm3 = A0;
            m_csParameters->csdef.prj_prm4 = B0;
            m_csParameters->csdef.prj_prm5 = A1;
            m_csParameters->csdef.prj_prm6 = A2;
            m_csParameters->csdef.prj_prm7 = B1;
            m_csParameters->csdef.prj_prm8 = B2;
            return SUCCESS;
        }

    return GEOCOORDERR_ProjectionDoesntUseParameter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BaseGCS::DefinitionComplete ()
    {
    CSParameters*   newParams;

    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    if (NULL == (newParams = CSMap::CScsloc1 (&m_csParameters->csdef)))
        {
        // An error was returned likely because the datum used is unknown in the system dictionary.
        // It is possible that the datum comes from a user-library

        // Check if a library is set and it is a user-library.
        // Note that in this specific case the GCS being defined should technically be a GCS meant for
        // addition (or replacement) to the user-library since we expect user-library datums to be used
        // exclusively by user-library stored GCS.
        if (m_sourceLibrary != NULL && m_sourceLibrary->IsUserLibrary())
            {
            // Try to locate datum and ellipsoid definition in this library. 
            CSDatumDef* datum = m_sourceLibrary->GetDatum (WString(m_csParameters->csdef.dat_knm, false).c_str());

            // If no datum found then ... too bad.
            if (NULL == datum)
                return cs_Error;

            CSEllipsoidDef* ellipsoid = m_sourceLibrary->GetEllipsoid (WString(m_csParameters->csdef.elp_knm, false).c_str());

            // If no ellipsoid found then ... too bad.
            if (NULL == ellipsoid)
                return cs_Error;

            // We have everything we technically need ...
            if (NULL == (newParams = CSMap::CScsloc2 (&m_csParameters->csdef, datum, ellipsoid)))
                return cs_Error;
            }
        else
            {
            // No library set and datum not in the system library ... we cannot do much with such a definition
            // thus return an error.
            return cs_Error;
            }

        }
    else
        {
        // Since the datum is located in the system dictionary we indicate that the 
        // GCS just completed is System Library related.
        m_sourceLibrary = LibraryManager::Instance()->GetSystemLibrary();
        }
    
    delete m_csParameters;
    m_csParameters = newParams;

    // Clear cached destination GCS and datum converter if any
    if (m_destinationGCS != NULL)
        {
        m_destinationGCS->UnRegisterIsADestinationOf(*this);
        m_destinationGCS = NULL;
        }


    if (NULL != m_datumConverter)
        {
        m_datumConverter->Destroy();
        m_datumConverter = NULL;
        }

    SetModified(false);
    
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP    BaseGCS::GetDatumName () const
    {
    if (NULL == m_csParameters)
        return NULL;

    if (NULL == m_datumNameString)
        m_datumNameString = new WString (m_csParameters->datum.key_nm,false);

    return  m_datumNameString->c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
int         BaseGCS::GetDatumCode () const
    {
    if (NULL == m_csParameters)
        return -1;

    // if no dat_knm, return none.
    if (0 == m_csParameters->csdef.dat_knm[0])
        return -1;

    WString searchName (m_csParameters->csdef.dat_knm,false);

    // check user library.
    LibraryP sourceLibrary = GetSourceLibrary();
    if ( (NULL != sourceLibrary) && (sourceLibrary->IsUserLibrary()) )
        {
        uint32_t datumCount = (uint32_t) m_sourceLibrary->GetDatumCount();
        for (uint32_t iDatum=0; iDatum < datumCount; iDatum++)
            {
            WString datumName;
            m_sourceLibrary->GetDatumName (iDatum, datumName);
            if (0 == searchName.CompareToI (datumName))
                return 1000000 + iDatum;
            }
        }

    // check system library/
    char    mbDatumName[512];
    for (int index = 0; (0 < CSMap::CS_dtEnum (index, mbDatumName, sizeof(mbDatumName))); index++)
        {
        if (0 == BeStringUtilities::Stricmp (m_csParameters->csdef.dat_knm, mbDatumName))
            return index;
        }

    return -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BaseGCS::SetDatumCode (int datumCode)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    // -1 means no datum, ellipse only.
    if (-1 == datumCode)
        {
        // set the ellipse keyname from the datum before clearing the datum.
        CSMap::CS_stncp (m_csParameters->csdef.elp_knm, m_csParameters->datum.ell_knm, sizeof (m_csParameters->csdef.elp_knm));
        memset (m_csParameters->csdef.dat_knm, 0, sizeof (m_csParameters->csdef.dat_knm));
        memset (m_csParameters->datum.key_nm, 0, sizeof (m_csParameters->datum.key_nm));
        memset (m_csParameters->datum.dt_name, 0, sizeof (m_csParameters->datum.dt_name));
        m_csParameters->datum.delta_X = m_csParameters->datum.delta_Y = m_csParameters->datum.delta_Z = 0.0;
        m_csParameters->datum.rot_X   = m_csParameters->datum.rot_Y   = m_csParameters->datum.rot_Z   = 0.0;
        m_csParameters->datum.bwscale = 0.0;
        m_csParameters->datum.to84_via = cs_DTCTYP_NONE;

        // the effect of this line is to look up the ellipse information and install it into csdef.
        SetEllipsoidCode (GetEllipsoidCode());

        return SUCCESS;
        }

    // 2000000 is the separator, do nothing.
    if (2000000 == datumCode)
        return GEOCOORDERR_InvalidDatumCode;

    // library datum codes are offset by 1000000.
    if ( (datumCode >= 1000000) &&  (NULL != m_sourceLibrary) && m_sourceLibrary->IsUserLibrary ())
        {
        CSDatumDef* datumDef;
        if (NULL != (datumDef = m_sourceLibrary->GetDatum (datumCode - 1000000)))
            {
            DatumCP     datum = Datum::CreateDatum (*datumDef, m_sourceLibrary);
            CSMap::CS_stncp (m_csParameters->csdef.dat_knm, datumDef->key_nm, sizeof (m_csParameters->csdef.dat_knm));
            m_csParameters->datum = *datum->GetCSDatum();
            datum->Destroy();
            }
        }
    else
        {
        char    dtKeyName[128];
        if (0 > CSMap::CS_dtEnum (datumCode, dtKeyName, sizeof(dtKeyName)))
            return GEOCOORDERR_InvalidDatumCode;

        CSDatum*       datum;
        if (NULL == (datum = CSMap::CS_dtloc (dtKeyName)))
            return GEOCOORDERR_InvalidDatumCode;

        m_csParameters->datum = *datum;
        CSMap::CS_stncp (m_csParameters->csdef.dat_knm, datum->key_nm, sizeof (m_csParameters->csdef.dat_knm));
        CSMap::CS_free (datum);
        }

    // doing this causes the Vertical Datum to get set correctly when changing the horizontal datum.
    // (i.e., stays the same when going from NAD83 <-> NAD27, otherwise gets reset to vdcFromDatum.
    SetVerticalDatumCode (GetVerticalDatumCode());

    DELETE_AND_CLEAR (m_datumNameString);
    DELETE_AND_CLEAR (m_datumDescriptionString);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP    BaseGCS::GetDatumDescription () const
    {
    if (NULL == m_csParameters)
        return NULL;

    if (NULL == m_datumDescriptionString)
        m_datumDescriptionString = new WString (m_csParameters->datum.dt_name,false);

    return  m_datumDescriptionString->c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP     BaseGCS::GetDatumSource (WStringR datumSource) const
    {
    datumSource.empty();

    if (NULL != m_csParameters)
        {
        if (0 != m_csParameters->datum.key_nm[0])
            {
            CSDatumDef* datumDef;
            if (NULL != (datumDef = LibraryManager::Instance()->GetDatumDefFromGCS (*this)))
                {
                datumSource.AssignA (datumDef->source);
                // free the datum.
                CSMap::CS_free (datumDef);
                }
            }
        }
    return datumSource.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
WGS84ConvertCode  BaseGCS::GetDatumConvertMethod() const
    {
    if (NULL == m_csParameters)
        return ConvertType_NONE;

    if (0 == m_csParameters->datum.key_nm[0])
        return ConvertType_NONE;

    return (WGS84ConvertCode) m_csParameters->datum.to84_via;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void        BaseGCS::GetDatumDelta (DPoint3dR delta) const
    {
    bool             deltaValid, rotationValid, scaleValid;
    DatumParametersValid (deltaValid, rotationValid, scaleValid);
    if (!deltaValid)
        delta.x = delta.y = delta.z = 0.0;
    else
        {
        delta.x = m_csParameters->datum.delta_X;
        delta.y = m_csParameters->datum.delta_Y;
        delta.z = m_csParameters->datum.delta_Z;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void        BaseGCS::GetDatumRotation (DPoint3dR rotation) const
    {
    bool             deltaValid, rotationValid, scaleValid;
    DatumParametersValid (deltaValid, rotationValid, scaleValid);
    if (!rotationValid)
        rotation.x = rotation.y = rotation.z = 0.0;
    else
        {
        rotation.x = m_csParameters->datum.rot_X;
        rotation.y = m_csParameters->datum.rot_Y;
        rotation.z = m_csParameters->datum.rot_Z;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
double      BaseGCS::GetDatumScale () const
    {
    bool             deltaValid, rotationValid, scaleValid;
    DatumParametersValid (deltaValid, rotationValid, scaleValid);
    return (scaleValid) ? m_csParameters->datum.bwscale : 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool        BaseGCS::DatumParametersValid (bool& deltaValid, bool& rotationValid, bool& scaleValid) const
    {

    // initialize to defaults.
    deltaValid = rotationValid = scaleValid = false;

    if (NULL == m_csParameters)
        return false;

    switch (GetDatumConvertMethod())
        {
        case ConvertType_MOLO:
        case ConvertType_3PARM:
        case ConvertType_GEOCTR:
            deltaValid = true;
            break;

        case ConvertType_BURS:
        case ConvertType_7PARM:
            deltaValid = rotationValid = scaleValid = true;
            break;

        case ConvertType_6PARM:
            deltaValid = rotationValid = true;
            break;

        case ConvertType_4PARM:
            deltaValid = scaleValid = true;
            break;
        }

    return (deltaValid || rotationValid || scaleValid);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BaseGCS::IsNAD27 () const
    {
    if (NULL == m_csParameters)
        return false;

    if (0 == BeStringUtilities::Stricmp (m_csParameters->datum.key_nm, "NAD27"))
        return true;
    if (0 == BeStringUtilities::Stricmp (m_csParameters->datum.key_nm, "EPSG:6267"))
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BaseGCS::IsNAD83 () const
    {
    if (NULL == m_csParameters)
        return false;

    if (0 == BeStringUtilities::Stricmp (m_csParameters->datum.key_nm, "NAD83"))
        return true;
    if (0 == BeStringUtilities::Stricmp (m_csParameters->datum.key_nm, "EPSG:6269"))
        return true;
    if (0 == BeStringUtilities::Stricmp (m_csParameters->datum.key_nm, "HPGN"))
        return true;
    if (0 == BeStringUtilities::Stricmp (m_csParameters->datum.key_nm, "EPSG:6152"))
        return true;
    if (0 == BeStringUtilities::Stricmp (m_csParameters->datum.key_nm, "NAD83/2011"))
        return true;

    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         BaseGCS::GetVerticalDatumName () const
    {
    if (NULL == m_csParameters)
        return NULL;

    bool    isNAD27 = this->IsNAD27();
    bool    isNAD83 = this->IsNAD83();
    if ( !isNAD27 && !isNAD83 )
        return GetDatumName();

    if ( (vdcNGVD29 == m_verticalDatum) || ( (vdcFromDatum == m_verticalDatum) && isNAD27) )
        return L"NGVD29";

     if ( (vdcNAVD88 == m_verticalDatum) || ( (vdcFromDatum == m_verticalDatum) && isNAD83) )
        return L"NAVD88";

    return GetDatumName();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
VertDatumCode   BaseGCS::GetVerticalDatumCode () const
    {
    if (NULL == m_csParameters)
        return vdcFromDatum;

    return m_verticalDatum;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BaseGCS::SetVerticalDatumCode
(
VertDatumCode   verticalDatumCode
)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    // There are limitations to using NGVD29 and NAVD88 (Others are worldwide)
    if ((vdcNGVD29 == verticalDatumCode && !this->IsNAD27()) ||
        (vdcNAVD88 == verticalDatumCode && !this->IsNAD83()))
        {
        m_verticalDatum = vdcFromDatum;
        return GEOCOORDERR_CantSetVerticalDatum;
        }

    if (NULL != m_datumConverter)
        {
        m_datumConverter->Destroy();
        m_datumConverter = NULL;
        }

    m_verticalDatum = verticalDatumCode;

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP     BaseGCS::GetEllipsoidName () const
    {
    if (NULL == m_csParameters)
        return NULL;

    if (NULL == m_ellipsoidNameString)
        m_ellipsoidNameString = new WString (m_csParameters->datum.ell_knm,false);

    return  m_ellipsoidNameString->c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
int         BaseGCS::GetEllipsoidCode () const
    {
    if (NULL == m_csParameters)
        return -1;


    WString     searchName (m_csParameters->datum.ell_knm,false);

    // check user library.
    LibraryP sourceLibrary = GetSourceLibrary();
    if ( (NULL != sourceLibrary) && (sourceLibrary->IsUserLibrary()) )
        {
        uint32_t ellipsoidCount = (uint32_t) m_sourceLibrary->GetEllipsoidCount();
        for (uint32_t iEllipsoid=0; iEllipsoid < ellipsoidCount; iEllipsoid++)
            {
            WString     ellipsoidName;
            m_sourceLibrary->GetEllipsoidName (iEllipsoid, ellipsoidName);
            if (0 == searchName.CompareToI (ellipsoidName))
                return 1000000 + iEllipsoid;
            }
        }

    // check system library/
    char    mbEllipsoidName[512];
    for (int index = 0; (0 < CS_elEnum (index, mbEllipsoidName, sizeof(mbEllipsoidName))); index++)
        {
        if (0 == BeStringUtilities::Stricmp (m_csParameters->datum.ell_knm, mbEllipsoidName))
            return index;
        }

    return -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BaseGCS::SetEllipsoidCode
(
int     ellipsoidCode
)
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    SetModified(true);

    if (-1 != GetDatumCode())
        return GEOCOORDERR_CantSetEllipsoid;

    // 2000000 is the separator, do nothing.
    if (2000000 == ellipsoidCode)
        return GEOCOORDERR_InvalidEllipsoidCode;

    CSEllipsoidDef* ellipsoidDef = NULL;
    if ( (ellipsoidCode >= 1000000) &&  (NULL != m_sourceLibrary) && m_sourceLibrary->IsUserLibrary ())
        ellipsoidDef = m_sourceLibrary->GetEllipsoid (ellipsoidCode - 1000000);

    if (NULL == ellipsoidDef)
        {
        char    elKeyName[128];
        if (0 > CS_elEnum (ellipsoidCode, elKeyName, sizeof(elKeyName)))
            return GEOCOORDERR_InvalidEllipsoidCode;

        if (NULL == (ellipsoidDef = CSMap::CS_eldef (elKeyName)))
            return GEOCOORDERR_InvalidEllipsoidCode;
        }

    CSDatum*       datum;
    if (NULL == (datum = CSdtloc2 (NULL, ellipsoidDef)))
        {
        CSMap::CS_free (ellipsoidDef);
        return GEOCOORDERR_InvalidEllipsoidCode;
        }

    m_csParameters->datum = *datum;
    CSMap::CS_stncp (m_csParameters->csdef.elp_knm, ellipsoidDef->key_nm, sizeof (m_csParameters->csdef.elp_knm));
    CSMap::CS_free (datum);
    CSMap::CS_free (ellipsoidDef);

    DELETE_AND_CLEAR (m_ellipsoidNameString);
    DELETE_AND_CLEAR (m_ellipsoidDescriptionString);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP     BaseGCS::GetEllipsoidDescription () const
    {
    if (NULL == m_csParameters)
        return NULL;

    if (NULL == m_ellipsoidDescriptionString)
        m_ellipsoidDescriptionString = new WString (m_csParameters->datum.el_name,false);

    return  m_ellipsoidDescriptionString->c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         BaseGCS::GetEllipsoidSource (WStringR ellipsoidSource) const
    {
    ellipsoidSource.clear();

    if (NULL != m_csParameters)
        {
        if (0 != m_csParameters->datum.ell_knm[0])
            {
            CSEllipsoidDef* ellipsoidDef;
            if (NULL != (ellipsoidDef = CSMap::CS_eldef (m_csParameters->datum.ell_knm)))
                {
                ellipsoidSource.AssignA (ellipsoidDef->source);
                // free the ellipsoid.
                CSMap::CS_free (ellipsoidDef);
                }
            }
        }
    return ellipsoidSource.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetEllipsoidEquatorialRadius () const
    {
    if (NULL != m_csParameters)
        return m_csParameters->datum.e_rad;
    else
        return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetEllipsoidPolarRadius () const
    {
    if (NULL != m_csParameters)
        return m_csParameters->datum.p_rad;
    else
        return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
double BaseGCS::GetEllipsoidEccentricity () const
    {
    if (NULL != m_csParameters)
        return m_csParameters->datum.ecent;
    else
        return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
bool    BaseGCS::GetCanEdit () const
    {
    return m_canEdit;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
void    BaseGCS::SetCanEdit
(
bool    value
)
    {
    m_canEdit = value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
double          BaseGCS::GetScaleAlongMeridian
(
GeoPointCR      point
) const
    {
    if (NULL != m_csParameters)
        return CSMap::CS_cssch (m_csParameters, &point);
    else
        return 1.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
double          BaseGCS::GetScaleAlongParallel
(
GeoPointCR      point
) const
    {
    if (NULL != m_csParameters)
        return CSMap::CS_cssck (m_csParameters, &point);
    else
        return 1.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
double          BaseGCS::GetGridScale
(
GeoPointCR      point
) const
    {
    if (NULL != m_csParameters)
        return CSMap::CS_csscl (m_csParameters, &point);
    else
        return 1.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
double          BaseGCS::GetConvergenceAngle
(
GeoPointCR      point
) const
    {
    if (NULL != m_csParameters)
        return CSMap::CS_cscnv (m_csParameters, &point);
    else
        return 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BaseGCS::GetDistance
(
double      *distance,
double      *azimuth,
GeoPointCR  startPoint,
GeoPointCR  endPoint
) const
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    double  tempDistance;
    double  tempAzimuth;
    if (NULL == distance)
        distance = &tempDistance;
    if (NULL == azimuth)
        azimuth = &tempAzimuth;

    double  equatorialRadius = GetEllipsoidEquatorialRadius ();
    double  eccentricity     = GetEllipsoidEccentricity ();

    *azimuth = CSMap::CS_llazdd (equatorialRadius, eccentricity * eccentricity, &startPoint, &endPoint, distance);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/07
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       BaseGCS::GetCenterPoint
(
GeoPointR       centerPoint
) const
    {
    if (NULL == m_csParameters)
        return GEOCOORDERR_InvalidCoordSys;

    if (0 != m_csError)
        {
        centerPoint.Init (0.0, 0.0, 0.0);
        return m_csError;
        }
    // In examiming it, I discovered that CS_fillIn seems to always set csdef.org_lng and csdef.org_lat.
    // Thus I use it rather than try to figure out how to use the cs_prjprm function. It seemed to me that
    // we needed access to their cs_prjTab structure, but that they did not give access through the API.
    CSDefinition tempCS = m_csParameters->csdef;
    CSMap::CS_fillIn (&tempCS);
    centerPoint.Init (tempCS.org_lng, tempCS.org_lat, 0.0);

    // if we have a LocalTransformer, we want to go to Cartesian, transform, then go back to LL.
    if (m_localTransformer.IsValid())
        {
        DPoint3d    cartesian = {0.0, 0.0, 0.0};

        LatLongFromCartesian (centerPoint, cartesian);

#if defined (VERIFICATION_DEBUGGING)
        // use the CS-map function directly to get the center point in the underlying GCS's Cartesian coordinates.
        CSMap::CS_ll3cs (m_csParameters, &internalCartesian, &centerPoint);

        // get that cartesian point in the
        DPoint3d    cartesian1, cartesian2;
        InternalCartesianFromCartesian (cartesian1, internalCartesian);
        CartesianFromInternalCartesian (cartesian2, internalCartesian);

        DPoint3d    check1, check2;
        InternalCartesianFromCartesian (check2, cartesian2);
        CartesianFromInternalCartesian (check1, cartesian1);

        CSMap::CS_cs3ll (m_csParameters, &center1, &cartesian1);
        CSMap::CS_cs3ll (m_csParameters, &center2, &cartesian2);
#endif

        }

    return SUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BaseGCS::IsEquivalent (BaseGCSCR compareTo) const
    {
    bool    datumDifferent, csDifferent, verticalDatumDifferent, localTransformDifferent;
    return Compare (compareTo, datumDifferent, csDifferent, verticalDatumDifferent, localTransformDifferent, true);
    }

#define SET_RETURN_OPT(var)   {var=true;if(stopFirstDifference) return false;}
#define SET_RETURN(var)       {var=true;return false;}
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BaseGCS::Compare (BaseGCSCR compareTo, bool& datumDifferent, bool& csDifferent, bool& verticalDatumDifferent, bool& localTransformDifferent, bool stopFirstDifference) const
    {
    datumDifferent          = false;
    csDifferent             = false;
    verticalDatumDifferent  = false;
    localTransformDifferent = false;

    if (NULL == m_csParameters)
        return false;

    // compare the easiest ones first.

    // the projection codes have to match.
    if (m_csParameters->prj_code != compareTo.m_csParameters->prj_code)
        SET_RETURN_OPT (csDifferent)

    // the projection flags have to match.
    if (m_csParameters->prj_flags != compareTo.m_csParameters->prj_flags)
        SET_RETURN_OPT (csDifferent)

    if (!HasEquivalentDatum (compareTo))
        SET_RETURN_OPT (datumDifferent)

    if (m_verticalDatum != compareTo.m_verticalDatum)
        SET_RETURN_OPT (verticalDatumDifferent)

    if (!LocalTransformer::IsEquivalent (m_localTransformer, compareTo.m_localTransformer))
        SET_RETURN_OPT (localTransformDifferent)

    CSDefinition    thisDef    = m_csParameters->csdef;
    CSDefinition    compareDef = compareTo.m_csParameters->csdef;

    // unless the prj_flgs says no origin longitude is used (note bit set means not used), they must be equal.
    if ( (0 == (m_csParameters->prj_flags & cs_PRJFLG_ORGLNG)) && !doubleSame (thisDef.org_lng, compareDef.org_lng) )
        SET_RETURN (csDifferent)

    // unless the prj_flgs says no origin latitude is used (note bit set means not used), they must be equal.
    if ( (0 == (m_csParameters->prj_flags & cs_PRJFLG_ORGLAT)) && !doubleSame (thisDef.org_lat, compareDef.org_lat) )
        SET_RETURN (csDifferent)

    // if the scale is not the same, they're not same units, can't be the same.
    if (!doubleSame (thisDef.scale, compareDef.scale))
        SET_RETURN (csDifferent)

    // unless the prj_flgs says no false easting/northing is used (note bit set means not use), they must be equal.
    if ( (0 == (m_csParameters->prj_flags & cs_PRJFLG_ORGFLS)) && (!doubleSame (thisDef.x_off, compareDef.x_off) || !doubleSame (thisDef.y_off, compareDef.y_off)) )
        SET_RETURN (csDifferent)

    // if the prj_flgs says a scale reduction is used (note bit set used), they must be equal.
    if ( (0 != (m_csParameters->prj_flags & cs_PRJFLG_SCLRED)) && !doubleSame (thisDef.scl_red, compareDef.scl_red))
        SET_RETURN (csDifferent)

    // quads must match.
    if (thisDef.quad != compareDef.quad)
        SET_RETURN (csDifferent)

    // find which parameters are needed for the projection by using cs_prjprm, compare those.

    // find the parameter map for this projection
    struct cs_PrjprmMap_ *mp;
    for (mp = cs_PrjprmMap; mp->prj_code != cs_PRJCOD_END; mp++)
        {
        if (mp->prj_code == m_csParameters->prj_code)
            break;
        }

    if (mp->prj_code == cs_PRJCOD_END)
        return true;

    double *thisDouble;
    double *compareDouble;
    int     iParam;
    for (iParam = 0, thisDouble = &thisDef.prj_prm1, compareDouble = &compareDef.prj_prm1; iParam < 24; iParam++, thisDouble++, compareDouble++)
        {
        // if the parameter index is 0, then that parameter's not used. There are never any embedded 0's so we can stop at the first one we encounter.
        // NOTE: we don't need to know what it's used for, just that it is used.
        int parameterIndex = mp->prm_types[iParam];
        if (parameterIndex <= 0)
            break;

        // for the northern/southern hemisphere parameter, 0 and 1 are the same.
        if (cs_PRMCOD_HSNS == parameterIndex)
           {
           if ( (*thisDouble >= 0.0) != (*compareDouble >= 0.0) )
                SET_RETURN (csDifferent)
           }
        else
            {
            if (!doubleSame (*thisDouble, *compareDouble))
                SET_RETURN (csDifferent)
            }
        }

    return true;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Alain.Robert    08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BaseGCSUtilGetRangeSpecified (bvector<GeoPoint>&     shape, 
                             double              minLongitude,
                             double              maxLongitude,
                             double              minLatitude,
                             double              maxLatitude)
    {
    GeoPoint point;
    point.Init(minLongitude, minLatitude, 0.0);
    shape.push_back(point);
    point.Init(minLongitude, maxLatitude, 0.0);
    shape.push_back(point);
    point.Init(maxLongitude, maxLatitude, 0.0);
    shape.push_back(point);
    point.Init(maxLongitude, minLatitude, 0.0);
    shape.push_back(point);
    point.Init(minLongitude, minLatitude, 0.0);
    shape.push_back(point);
    return BSISUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BaseGCSUtilGetRangeAboutPrimeMeridianAndEquator (bvector<GeoPoint>&  shape, 
                                                double              allowedDeltaAboutPrimeMeridian,
                                                double              allowedDeltaAboutEquator)
    {
    GeoPoint point;
    point.Init(-allowedDeltaAboutPrimeMeridian, -allowedDeltaAboutEquator, 0.0);
    shape.push_back(point);
    point.Init(-allowedDeltaAboutPrimeMeridian, allowedDeltaAboutEquator, 0.0);
    shape.push_back(point);
    point.Init(allowedDeltaAboutPrimeMeridian, allowedDeltaAboutEquator, 0.);
    shape.push_back(point);
    point.Init(allowedDeltaAboutPrimeMeridian, -allowedDeltaAboutEquator, 0.0);
    shape.push_back(point);
    point.Init(-allowedDeltaAboutPrimeMeridian, -allowedDeltaAboutEquator, 0.0);
    shape.push_back(point);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BaseGCSUtilGetRangeAboutMeridianAndEquator  (bvector<GeoPoint>&  shape, 
                                            double              specifiedMeridian,
                                            double              allowedDeltaAboutMeridian,
                                            double              allowedDeltaAboutEquator)
    {
    const double minLongitude = specifiedMeridian - allowedDeltaAboutMeridian;
    const double maxLongitude = specifiedMeridian + allowedDeltaAboutMeridian;
    const double minLatitude = -allowedDeltaAboutEquator;
    const double maxLatitude = allowedDeltaAboutEquator; 

    GeoPoint point;
    point.Init(minLongitude, minLatitude, 0.0);
    shape.push_back(point);
    point.Init(minLongitude, maxLatitude, 0.0);
    shape.push_back(point);
    point.Init(maxLongitude, maxLatitude, 0.0);
    shape.push_back(point);
    point.Init(maxLongitude, minLatitude, 0.0);
    shape.push_back(point);
    point.Init(minLongitude, minLatitude, 0.0);
    shape.push_back(point);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BaseGCSUtilGetRangeAboutMeridianAndParallel (bvector<GeoPoint>&     shape, 
                                            double              specifiedMeridian,
                                            double              allowedDeltaAboutMeridian,
                                            double              specifiedParallel,
                                            double              allowedDeltaAboutParallel)
    {
    const double minLongitude = specifiedMeridian - allowedDeltaAboutMeridian;
    const double maxLongitude = specifiedMeridian + allowedDeltaAboutMeridian;
    const double minLatitude = specifiedParallel - allowedDeltaAboutParallel;
    const double maxLatitude = specifiedParallel + allowedDeltaAboutParallel;   

    GeoPoint point;
    point.Init(minLongitude, minLatitude, 0.0);
    shape.push_back(point);
    point.Init(minLongitude, maxLatitude, 0.0);
    shape.push_back(point);
    point.Init(maxLongitude, maxLatitude, 0.0);
    shape.push_back(point);
    point.Init(maxLongitude, minLatitude, 0.0);
    shape.push_back(point);
    point.Init(minLongitude, minLatitude, 0.0);
    shape.push_back(point);
    return BSISUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BaseGCSUtilGetRangeAboutMeridianAndBoundParallel    (bvector<GeoPoint>&  shape, 
                                                    double              specifiedMeridian,
                                                    double              allowedDeltaAboutMeridian,
                                                    double              specifiedParallel,
                                                    double              allowedDeltaAboutParallel,
                                                    double              southMostAllowedParallel,
                                                    double              northMostAllowedParallel)
    {
    const double minLongitude = specifiedMeridian - allowedDeltaAboutMeridian;
    const double maxLongitude = specifiedMeridian + allowedDeltaAboutMeridian;

    double minLatitude = specifiedParallel - allowedDeltaAboutParallel;
    if (minLatitude < southMostAllowedParallel)
        minLatitude = southMostAllowedParallel;

    double maxLatitude = specifiedParallel + allowedDeltaAboutParallel;
    if (maxLatitude > northMostAllowedParallel)
        maxLatitude = northMostAllowedParallel;

    GeoPoint point;
    point.Init(minLongitude, minLatitude, 0.0);
    shape.push_back(point);
    point.Init(minLongitude, maxLatitude, 0.0);
    shape.push_back(point);
    point.Init(maxLongitude, maxLatitude, 0.0);
    shape.push_back(point);
    point.Init(maxLongitude, minLatitude, 0.0);
    shape.push_back(point);
    point.Init(minLongitude, minLatitude, 0.0);
    shape.push_back(point);
    return BSISUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BaseGCSUtilGetRangeAboutMeridianAndTwoStandardBoundParallel (bvector<GeoPoint>&  shape, 
                                                            double              specifiedMeridian,
                                                            double              allowedDeltaAboutMeridian,
                                                            double              standardParallel1,
                                                            double              standardParallel2,
                                                            double              allowedDeltaAboutParallels,
                                                            double              southMostAllowedParallel,
                                                            double              northMostAllowedParallel)
    {
    const double minLongitude = specifiedMeridian - allowedDeltaAboutMeridian;
    const double maxLongitude = specifiedMeridian + allowedDeltaAboutMeridian;

    double minLatitude;
    double maxLatitude;
    if (standardParallel1 < standardParallel2)
        {
        minLatitude = standardParallel1 - allowedDeltaAboutParallels;
        if (minLatitude < southMostAllowedParallel)
            minLatitude = southMostAllowedParallel;
        maxLatitude = standardParallel2 + allowedDeltaAboutParallels;
        if (maxLatitude > northMostAllowedParallel)
            maxLatitude = northMostAllowedParallel;
        }
    else
        {
        minLatitude = standardParallel2 - allowedDeltaAboutParallels;
        if (minLatitude < southMostAllowedParallel)
            minLatitude = southMostAllowedParallel;
        maxLatitude = standardParallel1 + allowedDeltaAboutParallels;
        if (maxLatitude > northMostAllowedParallel)
            maxLatitude = northMostAllowedParallel;
        }


    GeoPoint point;
    point.Init(minLongitude, minLatitude, 0.0);
    shape.push_back(point);
    point.Init(minLongitude, maxLatitude, 0.0);
    shape.push_back(point);
    point.Init(maxLongitude, maxLatitude, 0.0);
    shape.push_back(point);
    point.Init(maxLongitude, minLatitude, 0.0);
    shape.push_back(point);
    point.Init(minLongitude, minLatitude, 0.0);
    shape.push_back(point);

    return BSISUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BaseGCSUtilGetRangeAboutBoundMeridianAndBoundParallel   (bvector<GeoPoint>&     shape, 
                                                        double              specifiedMeridian,
                                                        double              allowedDeltaAboutMeridian,
                                                        double              westMostAllowedMeridian,
                                                        double              eastMostAllowedMeridian,
                                                        double              specifiedParallel,
                                                        double              allowedDeltaAboutParallel,
                                                        double              southMostAllowedParallel,
                                                        double              northMostAllowedParallel)
    {
    double minLongitude = specifiedMeridian - allowedDeltaAboutMeridian;
    if (minLongitude < westMostAllowedMeridian)
        minLongitude = westMostAllowedMeridian;
    double maxLongitude = specifiedMeridian + allowedDeltaAboutMeridian;
    if (maxLongitude > eastMostAllowedMeridian)
        maxLongitude = eastMostAllowedMeridian;
    double minLatitude = specifiedParallel - allowedDeltaAboutParallel;
    if (minLatitude < southMostAllowedParallel) 
        minLatitude = southMostAllowedParallel;   
    double maxLatitude = specifiedParallel + allowedDeltaAboutParallel; 
    if (maxLatitude > northMostAllowedParallel)
        maxLatitude = northMostAllowedParallel;   

    GeoPoint point;
    point.Init(minLongitude, minLatitude, 0.0);
    shape.push_back(point);
    point.Init(minLongitude, maxLatitude, 0.0);
    shape.push_back(point);
    point.Init(maxLongitude, maxLatitude, 0.0);
    shape.push_back(point);
    point.Init(maxLongitude, minLatitude, 0.0);
    shape.push_back(point);
    point.Init(minLongitude, minLatitude, 0.0);
    shape.push_back(point);
    return BSISUCCESS;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                Raymond.Gauthier    07/2012
+---------------+---------------+---------------+---------------+---------------+------*/
inline double BaseGCSUtilGetUTMZoneCenterMeridian(int zoneNumber)
    {
    return ((zoneNumber - 30) * 6) - 3;
    }


/*---------------------------------------------------------------------------------**//**
* Returns the domain of application for GCS. This domain is the math domain intersected
* with the logical domain if one is set.
* @bsimethod                                                    AlainRobert  2/2009
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BaseGCS::GetMathematicalDomain
(
bvector<GeoPoint>&    shape
) const 
    {
    // Some explanation about the values specified below and their intent.
    // First it must be inderstood that the current implementation is in progress.
    // The present implementation fixes some reported issues related to the
    // display and management of rasters when reprojection is invloved.
    // The principle attempts to define the geo domain of a specific projection using
    // extent defined as min and max longitude and latitude. Such definition is adequate
    // for many projections but not all. For example Lamber Comformal Conic domain is
    // domain is correctle defined using such definition. For transverse mercator and derivatives
    // the domain can likewise be defined using this method. Others like Oblique Mercator
    // or stereo graphic projections cannot as their area definition is not alligned
    // to latitude and longitudes. We assume that an smaller area can be defined using
    // plain geo extent but we are not sure. When the North and South pole are included we
    // have not yet defined a way to indicate this representation other than by specifying
    // exact min or max to either North or Sout pole latitude but the actual
    // case never occured so the implementation has currently been postponed
    // till more adequate research can be done.
    //
    // Concerning the definition of Transverse Mercators and derivative the mathematical domain
    // is usually defined from North to South pole on a longitude with of some
    // specific value ... We provide a very large area in this case. In practice we have had
    // cases where the datum shift during the reprojection process shifted the North and South pole
    // sufficiently that a longitude located on one side of the Earth became in the other datum
    // on the other size of the pole (17E Longitude became 163W Longitude)
    // For this reason we have decided to limit the upper and lower latitudes for all
    // projections to 89.9 degrees (any greater values resulted in the problem in our case)
    // This means that the zone will remain about 12 kilometers from the poles. For cartography
    // made in the pole areas, other projection methods will have to be used.

    const ProjectionCodeValue projectionCode = GetProjectionCode();

    // If datum transformation method is limitative by nature we will use the user-defined domain except for the danish systems.
    WGS84ConvertCode datumConvert = GetDatumConvertMethod();

    if ((projectionCode != pcvTransverseMercatorDenmarkSys34 && projectionCode != pcvTransverseMercatorDenmarkSys3499 && projectionCode != pcvTransverseMercatorDenmarkSys3401) &&
        ((ConvertType_MREG  == datumConvert) ||
         (ConvertType_NAD27 == datumConvert) ||
         (ConvertType_HPGN  == datumConvert) ||  
         (ConvertType_AGD66 == datumConvert) ||  
         (ConvertType_AGD84 == datumConvert) ||
         (ConvertType_NZGD4 == datumConvert) ||   
         (ConvertType_ATS77 == datumConvert) ||  
         (ConvertType_CSRS  == datumConvert) ||   
         (ConvertType_TOKYO == datumConvert) ||   
         (ConvertType_RGF93 == datumConvert) ||  
         (ConvertType_ED50  == datumConvert) ||    
         (ConvertType_DHDN  == datumConvert) ||
         (ConvertType_GENGRID == datumConvert) ||
         (ConvertType_CHENYX == datumConvert)))
        {

        double minLongitude = GetMinimumUsefulLongitude();
        double maxLongitude = GetMaximumUsefulLongitude();
        double minLatitude = GetMinimumUsefulLatitude();
        double maxLatitude = GetMaximumUsefulLatitude();
        if ((minLongitude != maxLongitude) && (minLatitude != minLongitude))
            {
            // The user-defined are as defined in the dictionary but CSMAP requires a tiny difference from absolute 
            // position specified (for example Transverse Mercator is technically valid up to 90 latitude but CSMAP requires a few centimeters appart
            // just in case. For this reason we minimise slightly the extent
            minLongitude += 0.0000028;
            maxLongitude -= 0.0000028;
            minLatitude += 0.0000028;
            maxLatitude -= 0.0000028;
            return BaseGCSUtilGetRangeSpecified(shape, minLongitude, maxLongitude, minLatitude, maxLatitude);
            }
        }


    switch (projectionCode)
        {
        case pcvCassini : // Not so sure about this one ... check http://www.radicalcartography.net/?projectionref
        case pcvEckertIV :
        case pcvEckertVI :
        case pcvMillerCylindrical :
        case pcvUnity :
        case pcvGoodeHomolosine :
        case pcvModifiedStereographic :
        case pcvEqualAreaAuthalicNormal :
        case pcvEqualAreaAuthalicTransverse :
        case pcvSinusoidal :
        case pcvVanderGrinten :
        case pcvRobinsonCylindrical :
        case pcvWinkelTripel :
        case pcvEquidistantCylindrical :
        case pcvEquidistantCylindricalEllipsoid :
        case pcvPlateCarree :
            // good around the globe          
            return BaseGCSUtilGetRangeAboutPrimeMeridianAndEquator (shape, 180.0, 89.9);

        case pcvMercatorScaleReduction :
        case pcvMercator :
        case pcvPopularVisualizationPseudoMercator :
            {
            // The mercator projection implementation limits somewhat the valid extent to the 
            // user domain specified.
            double minLongitude = GetMinimumUsefulLongitude();
            double maxLongitude = GetMaximumUsefulLongitude();
            double minLatitude = GetMinimumUsefulLatitude();
            double maxLatitude = GetMaximumUsefulLatitude();
            if ((minLongitude != maxLongitude) && (minLatitude != minLongitude))
                {
                // The user-defined are as defined in the dictionary but CSMAP requires a tiny difference from absolute 
                // position specified (for example Transverse Mercator is technically valid up to 90 latitude but CSMAP requires a few centimeters appart
                // just in case. For this reason we minimise slightly the extent
                minLongitude += 0.0000028;
                maxLongitude -= 0.0000028;
                minLatitude += 0.0000028;
                maxLatitude -= 0.0000028;
                return BaseGCSUtilGetRangeSpecified(shape, minLongitude, maxLongitude, minLatitude, maxLatitude);
                }

            // good pretty close 90 degrees east and west of central meridian
            return BaseGCSUtilGetRangeAboutMeridianAndEquator(shape, 
                                                   GetCentralMeridian (), 
                                                   179.999999, 
                                                   80.0);
            }
        case pcvLambertEquidistantAzimuthal :
        case pcvAzimuthalEquidistantElevatedEllipsoid :
        case pcvLambertEqualAreaAzimuthal :
        case pcvOrthographic :
        case pcvObliqueStereographic :
        case pcvSnyderObliqueStereographic :
        case pcvPolarStereographic :
        case pcvPolarStereographicStandardLatitude :
        case pcvGnomonic :
        case pcvBipolarObliqueConformalConic :
	    {
	    // Eventually we will study more attentively how to
	    // Compute the mathematical extent but for the moment we will limit to the
            // user extent
	    double minLongitude = GetMinimumUsefulLongitude();
	    double maxLongitude = GetMaximumUsefulLongitude();
	    double minLatitude = GetMinimumUsefulLatitude();
	    double maxLatitude = GetMaximumUsefulLatitude();
	    if ((minLongitude != maxLongitude) && (minLatitude != minLongitude))
		{
		return BaseGCSUtilGetRangeSpecified(shape, minLongitude, maxLongitude, minLatitude, maxLatitude);
		}

            // Even though it cannot be computed, the domain must be set as the caller may not check the return status.
            BaseGCSUtilGetRangeAboutPrimeMeridianAndEquator (shape, 180.0, 89.9);
            return BSIERROR; // return not implemented;
	    }

        case pcvTransverseMercator :
        case pcvGaussKrugerTranverseMercator :
        case pcvSouthOrientedTransverseMercator :
        case pcvTransverseMercatorAffinePostProcess :
        case pcvTransverseMercatorMinnesota :
        case pcvTransverseMercatorWisconsin:
        case pcvTransverseMercatorKruger :
            // Transverse Mercator will work relatively well from North to South pole and XX degrees either way of longitude of origin
            return BaseGCSUtilGetRangeAboutMeridianAndEquator(shape, 
                                                   GetCentralMeridian(), 15.0, 
                                                   89.9);

#if defined (TOTAL_SPECIAL)
	// This version of Transverse Mercator allows going further out of the zone center than the other
        // version.
        case pcvTotalTransverseMercatorBF :
            return BaseGCSUtilGetRangeAboutMeridianAndEquator(shape, 
                                                   GetCentralMeridian(), 30.0, 
                                                   89.9);
#endif

        // The following are close enough to TM but require latitude origin
        case pcvObliqueCylindricalHungary :
        case pcvTransverseMercatorOstn97 :
        case pcvTransverseMercatorOstn02 :
            // Transverse Mercator will work relatively well from North to South pole and XX degrees either way of longitude of origin
            return BaseGCSUtilGetRangeAboutMeridianAndEquator(shape, GetOriginLongitude(), 30.0, 89.9);

        case pcvCzechKrovak :
        case pcvCzechKrovakObsolete :
        case pcvCzechKrovak95 :
        case pcvCzechKrovak95Obsolete :
            // Hard-coded domain as origin longitude give 17.39W which couldn't be used as a central meridian for this
            // area. According to Alain Robert, these projections are oblique/conical and this strange origin longitude
            // could have been used as a mean of correction for non-standard prime meridian (not greenwich) used.
            return BaseGCSUtilGetRangeAboutMeridianAndParallel(shape, 17.5, 7.5, 49.5, 2.5);

        case pcvTransverseMercatorDenmarkSys34 :
        case pcvTransverseMercatorDenmarkSys3499 :
        case pcvTransverseMercatorDenmarkSys3401 :
            {
	    // The regions have specific and complex shapes. These shapes have
            // established by trial and error. They could not be made square because the
            // grid definition barely include the island it represents.
            int region = GetDanishSys34Region();

            // 1  ==> jylland
            // 2  ==> sjælland
            // 3  ==> bornholm
	    GeoPoint point;

            if (1 == region)
                {
		        point.Init(8.2930, 54.7757, 0.0);
		        shape.push_back(point);
		        point.Init(7.9743, 55.0112, 0.0);
		        shape.push_back(point);
		        point.Init(7.5544, 56.4801, 0.0);
		        shape.push_back(point);
		        point.Init(8.0280, 57.1564, 0.0);
		        shape.push_back(point);
		        point.Init(10.4167, 58.0417, 0.0);
		        shape.push_back(point);
		        point.Init(10.9897, 57.7786, 0.0);
		        shape.push_back(point);
		        point.Init(11.5395, 57.1551, 0.0);
		        shape.push_back(point);
		        point.Init(12.0059, 56.5088, 0.0);
		        shape.push_back(point);
		        point.Init(11.7200, 54.9853, 0.0);
		        shape.push_back(point);
		        point.Init(10.5938, 54.5951, 0.0);
		        shape.push_back(point);
		        point.Init(8.2930, 54.7757, 0.0);
		        shape.push_back(point);
                }
            else if (2 == region)
                {
#if (1)
                // For some obscure reason the domain of this zone used to be the one deactivated below
                // but now it is the one included here. I suspect this is because a reversibility test was either
                // added or the tolerance to the application of the reversibility test modified. 
                // &&AR To be checked.
		        point.Init(13.6160, 56.3874, 0.0);
		        shape.push_back(point);
		        point.Init(13.6803, 55.9697, 0.0);
		        shape.push_back(point);
		        point.Init(13.3465, 55.1436, 0.0);
		        shape.push_back(point);
		        point.Init(12.6916, 54.6633, 0.0);
		        shape.push_back(point);
		        point.Init(11.8615, 54.3089, 0.0);
		        shape.push_back(point);
		        point.Init(10.5746, 54.5406, 0.0);
		        shape.push_back(point);
		        point.Init(10.1280, 54.8928, 0.0);
		        shape.push_back(point);
		        point.Init(10.1619, 55.2380, 0.0);
		        shape.push_back(point);
		        point.Init(10.3488, 56.0007, 0.0);
		        shape.push_back(point);
		        point.Init(11.0446, 56.6799, 0.0);
		        shape.push_back(point);
		        point.Init(11.6792, 56.8603, 0.0);
		        shape.push_back(point);
		        point.Init(13.6160, 56.3874, 0.0);
		        shape.push_back(point);

#else
		        point.Init(11.5108, 54.4367, 0.0);
		        shape.push_back(point);
		        point.Init(10.2526, 54.6795, 0.0);
		        shape.push_back(point);
		        point.Init(9.6333, 55.0286, 0.0);
		        shape.push_back(point);
		        point.Init(9.6157, 55.3831, 0.0);
		        shape.push_back(point);
		        point.Init(10.0748, 56.0823, 0.0);
		        shape.push_back(point);
		        point.Init(11.5664, 56.9520, 0.0);
		        shape.push_back(point);
		        point.Init(13.2099, 56.5104, 0.0);
		        shape.push_back(point);
		        point.Init(13.2097, 54.8276, 0.0);
		        shape.push_back(point);
		        point.Init(12.8531, 54.6593, 0.0);
		        shape.push_back(point);
		        point.Init(12.1009, 54.5007, 0.0);
		        shape.push_back(point);
		        point.Init(11.5108, 54.4367, 0.0);
		        shape.push_back(point);
#endif
                }
            else 
                {
                BeAssert (3 == region);
		        point.Init(14.510, 54.942, 0.0);
		        shape.push_back(point);
		        point.Init(14.510, 55.431, 0.0);
		        shape.push_back(point);
		        point.Init(15.300, 55.431, 0.0);
		        shape.push_back(point);
		        point.Init(15.300, 54.942, 0.0);
		        shape.push_back(point);
		        point.Init(14.510, 54.942, 0.0);
		        shape.push_back(point);
                }


            }  
            return BSISUCCESS;

        // The conic
        case pcvAmericanPolyconic :
        case pcvModifiedPolyconic :
            // For conics we can extent 90 degrees east and west amd from xx degrees up or down from lowest/upper standard parallels
            return BaseGCSUtilGetRangeAboutMeridianAndBoundParallel(shape,
                                                         GetCentralMeridian(), 
							                             89.999999,
                                                         GetOriginLatitude(), 
							                             30.0,
                                                         -89.9, 
							                             89.9);
                                                                  
        case pcvLambertTangential :
        case pcvLambertConformalConicOneParallel :
        case pcvSnyderTransverseMercator :
            // For conics we can extent 90 degrees east and west amd from xx degrees up or down from lowest/upper standard parallels
            return BaseGCSUtilGetRangeAboutMeridianAndBoundParallel(shape, 
                                                         GetOriginLongitude(), 
							                             89.999999,
                                                         GetOriginLatitude(), 
							                             30.0,
                                                         -89.9, 
							                             89.9);

        case pcvBonne :
            return BaseGCSUtilGetRangeAboutMeridianAndBoundParallel(shape, 
                                                         GetOriginLongitude(), 
							                             170.999999,
                                                         GetOriginLatitude(), 
							                             60.0,
                                                         -89.9, 
							                             89.9);

        case pcvEquidistantConic :
        case pcvAlbersEqualArea :
        case pcvLambertConformalConicTwoParallel :
        case pcvLambertConformalConicWisconsin :
        case pcvLambertConformalConicBelgian :
        case pcvLambertConformalConicMinnesota:
        case pcvLambertConformalConicAffinePostProcess :
            // For conics we can extent 90 degrees east and west amd from xx degrees up or down from lowest/upper standard parallels
            return BaseGCSUtilGetRangeAboutMeridianAndTwoStandardBoundParallel(shape, 
                                                                    GetOriginLongitude(), 
								                                    89.9999,
                                                                    GetStandardParallel1(), 
								                                    GetStandardParallel2(), 
								                                    30.0,
                                                                    -80.0, 
								                                    80.0);

        case pcvObliqueCylindricalSwiss :
            // This projection is usually only used in Switzerland but can also be used in Hungary
            // we cannot hard code the extent based on the Switzerland extent but must instead compute the
            // extent based on the latitude and longitude of origin.
            return BaseGCSUtilGetRangeAboutMeridianAndParallel(shape, GetOriginLongitude(), 6.0, GetOriginLatitude(), 6.0);


        // Other local projections
        case pcvHotineObliqueMercator :
        case pcvMollweide :
        case pcvRectifiedSkewOrthomorphic :
        case pcvRectifiedSkewOrthomorphicCentered :
        case pcvRectifiedSkewOrthomorphicOrigin :
        case pcvHotineObliqueMercator1UV :
        case pcvHotineObliqueMercator1XY :
        case pcvHotineObliqueMercator2UV :
        case pcvHotineObliqueMercator2XY :
	        {
	        // We have not yet determined the mathematical extent of these.
	        // This should cause little problems as all are rarely used
	        double minLongitude = GetMinimumUsefulLongitude();
	        double maxLongitude = GetMaximumUsefulLongitude();
	        double minLatitude = GetMinimumUsefulLatitude();
	        double maxLatitude = GetMaximumUsefulLatitude();

	        if ((minLongitude != maxLongitude) && (minLatitude != minLongitude))
    	    	return BaseGCSUtilGetRangeSpecified(shape, minLongitude, maxLongitude, minLatitude, maxLatitude);

            // Even though it cannot be computed, the domain must be set as the caller may not check the return status.
            BaseGCSUtilGetRangeAboutPrimeMeridianAndEquator (shape, 180.0, 89.9);
            return BSIERROR; // return not implemented;
 	        }

        case pcvObliqueMercatorMinnesota :
        case pcvNewZealandNationalGrid :
    	    {
	        // These are local grids. Eventually we will study more attentively how to
	        // Compute the mathematical extent but for the moment we will limit to the
            // user extent
	        double minLongitude = GetMinimumUsefulLongitude();
	        double maxLongitude = GetMaximumUsefulLongitude();
	        double minLatitude = GetMinimumUsefulLatitude();
	        double maxLatitude = GetMaximumUsefulLatitude();
	        if ((minLongitude != maxLongitude) && (minLatitude != minLongitude))
        		return BaseGCSUtilGetRangeSpecified(shape, minLongitude, maxLongitude, minLatitude, maxLatitude);
		    
	        // User domain not set ... we will use the default
            // Even though it cannot be computed, the domain must be set as the caller may not check the return status.
            BaseGCSUtilGetRangeAboutPrimeMeridianAndEquator (shape, 180.0, 89.9);
            return BSIERROR; // return not implemented;
	        }
        // Other
        case pcvNonEarth :
        case pcvNonEarthScaleRotation :
        case pcvObliqueConformalConic :
            BaseGCSUtilGetRangeAboutPrimeMeridianAndEquator (shape, 180.0, 89.9);
            return BSIERROR; // return not implemented;

        case pcvUniversalTransverseMercator :
            return BaseGCSUtilGetRangeAboutMeridianAndEquator(shape, BaseGCSUtilGetUTMZoneCenterMeridian(GetUTMZone()), 15.0, 89.9);

#if defined (TOTAL_SPECIAL)
	    // This version of transverse mercator allow going further out of the zone center 
        case pcvTotalUniversalTransverseMercator :
	        return BaseGCSUtilGetRangeAboutMeridianAndEquator(shape, BaseGCSUtilGetUTMZoneCenterMeridian(GetUTMZone()), 30.0, 89.9);

#endif //TOTAL_SPECIAL
        default:
            break;
        }

    return BSIERROR; 
    }



#ifdef DICTIONARY_MANAGEMENT_ONLY
/*---------------------------------------------------------------------------------**//**
* @description: This method appears to have been originally written by Norm Olsen
* the person behind CSMAP. It appears to have been provided to Doug Bilinski outside
* CSMAP delivery. The result was addapted to Doug's "architecture" and was finally
* adpated to BaseGCS for dictionary management purposes only.
* @bsimethod                                                    Alain.Robert   07/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt BaseGCS::OutputAsASC
(
WStringR GCSAsASC
) const
    {
    StatusInt       status = SUCCESS;

    std::ostringstream GCSAsASCStream(GCSAsASC);

    if (!IsValid())
        return GEOCOORDERR_InvalidCoordSys;


    int order;
    int logTen;
    int prec;

    int32_t lngFrmt;
    int32_t latFrmt;
    int32_t xyFrmt;
    int32_t zzFrmt;
    int32_t anglFrmt;
    int32_t redFrmt;
    int32_t sclFrmt;
    int32_t coefFrmt;

    double tmpDbl;
    double zeroVal;
    struct cs_Prjtab_ *prjPtr;
    char ctemp [64];

    /* Locate the projection in the projection table. */
    for (prjPtr = cs_Prjtab; prjPtr->code != cs_PRJCOD_END; prjPtr += 1)
        {
        if (!strcmp (m_csParameters->csdef.prj_knm, prjPtr->key_nm))
            break;
        }
    if (prjPtr->code == cs_PRJCOD_END)
        {
        return 1;
        }

    /* Adjust the output value formatting as appropriate. */
    lngFrmt  = csLngFrmt;
    latFrmt  = csLatFrmt;
    xyFrmt   = csXyFrmt;
    zzFrmt   = csZzFrmt;
    anglFrmt = csAnglFrmt;
    redFrmt  = csRedFrmt;
    sclFrmt  = csSclFrmt;
    coefFrmt = csCoefFrmt;

    if ((prjPtr->flags & cs_PRJFLG_GEOGR) != 0)
        {
        /* Special changes for Unity projection here. */
        }

    UnitCP theUnit = Unit::FindUnit (m_csParameters->csdef.unit);

    /* Compute an apprropriate precision value based on the unit. */
    if ((prjPtr->flags & cs_PRJFLG_GEOGR) == 0)
        {
        tmpDbl = theUnit->GetConversionFactor();
        tmpDbl = log10 (tmpDbl);
        if (tmpDbl < 0.0)
            tmpDbl -= 0.4;
        else
            tmpDbl += 0.4;

        logTen = (int)tmpDbl;
        prec = 3 + logTen;
        zeroVal = pow (10.0, (double)(-prec));
        }
    else
        {
        tmpDbl = theUnit->GetConversionFactor();
        tmpDbl = log10 (tmpDbl);
        if (tmpDbl < 0.0)
            tmpDbl -= 0.4;
        else
            tmpDbl += 0.4;

        logTen = (int)tmpDbl;
        prec = 9 - logTen;
        zeroVal = pow (10.0, (double)(-prec));
        }
    xyFrmt = (xyFrmt & ~cs_ATOF_PRCMSK) | (prec + 1);

    /* We we do not have any Minimum non-zero values, create them now. */
    if (m_csParameters->csdef.zero [XX] == 0.0 && m_csParameters->csdef.zero [YY] == 0.0)
        {
        m_csParameters->csdef.zero [XX] = zeroVal;
        m_csParameters->csdef.zero [YY] = zeroVal;
        }

    /* Extract, and fprintf some stuff that's standard for all
       projections. Note, we try to stick to the basic order
       that was established, and somewhat maintained, since the
       first ASCII file was written. */
    GCSAsASCStream << "CS_NAME: " << m_csParameters->csdef.key_nm << std::endl
             << "          GROUP: " << m_csParameters->csdef.group << std::endl
             << "        DESC_NM: " << m_csParameters->csdef.desc_nm << std::endl
             << "         SOURCE: " << m_csParameters->csdef.source << std::endl;

    if (m_csParameters->csdef.dat_knm [0] != '\0')
        {
        GCSAsASCStream << "        DT_NAME: " << m_csParameters->csdef.dat_knm << std::endl;
        }
    else
        {
        GCSAsASCStream << "        EL_NAME: " << m_csParameters->csdef.elp_knm << std::endl;
        }
    GCSAsASCStream << "           PROJ: " << m_csParameters->csdef.prj_knm << std::endl
             << "           UNIT: " << m_csParameters->csdef.unit << std::endl;

    switch (m_csParameters->prj_code)
        {
        case  cs_PRJCOD_UNITY:
            if (m_csParameters->csdef.prj_prm1 != 0.0 || m_csParameters->csdef.prj_prm2 != 0.0)
                {
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
                GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, lngFrmt);
                GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
                }
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_TRMRKRG:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_TRMER:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

/*        case  cs_PRJCOD_TRMERBF:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            fprintf (fstr_out, "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            fprintf (fstr_out, "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            fprintf (fstr_out, "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            fprintf (fstr_out, "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            fprintf (fstr_out, "          Y_OFF: " << ctemp << std::endl;
            break;
*/

        case  cs_PRJCOD_ALBER:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, latFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, latFrmt);
            GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case cs_PRJCOD_MRCAT:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, latFrmt);
            GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case cs_PRJCOD_MRCATPV:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;


        case  cs_PRJCOD_AZMED:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, anglFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_LMTAN:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_PLYCN:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_MODPC:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, lngFrmt);
            GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm3, latFrmt);
            GCSAsASCStream << "          PARM3: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm4, latFrmt);
            GCSAsASCStream << "          PARM4: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_AZMEA:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, anglFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_EDCNC:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, latFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, latFrmt);
            GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_MILLR:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_MSTRO:
            if (m_csParameters->csdef.order != 0)
                {
                GCSAsASCStream << "          ORDER: " << m_csParameters->csdef.order << std::endl;
                }
            if      (m_csParameters->csdef.prj_prm23 != 0.0 || m_csParameters->csdef.prj_prm24 != 0.0)
                order = 12;
            else if (m_csParameters->csdef.prj_prm21 != 0.0 || m_csParameters->csdef.prj_prm22 != 0.0)
                order = 11;
            else if (m_csParameters->csdef.prj_prm19 != 0.0 || m_csParameters->csdef.prj_prm20 != 0.0)
                order = 10;
            else if (m_csParameters->csdef.prj_prm17 != 0.0 || m_csParameters->csdef.prj_prm18 != 0.0)
                order =  9;
            else if (m_csParameters->csdef.prj_prm15 != 0.0 || m_csParameters->csdef.prj_prm16 != 0.0)
                order =  8;
            else if (m_csParameters->csdef.prj_prm13 != 0.0 || m_csParameters->csdef.prj_prm14 != 0.0)
                order =  7;
            else if (m_csParameters->csdef.prj_prm11 != 0.0 || m_csParameters->csdef.prj_prm12 != 0.0)
                order =  6;
            else if (m_csParameters->csdef.prj_prm9  != 0.0 || m_csParameters->csdef.prj_prm10 != 0.0)
                order =  5;
            else if (m_csParameters->csdef.prj_prm7  != 0.0 || m_csParameters->csdef.prj_prm8  != 0.0)
                order =  4;
            else if (m_csParameters->csdef.prj_prm5  != 0.0 || m_csParameters->csdef.prj_prm6  != 0.0)
                order =  3;
            else if (m_csParameters->csdef.prj_prm3  != 0.0 || m_csParameters->csdef.prj_prm4  != 0.0)
                order =  2;
            else if (m_csParameters->csdef.prj_prm1  != 0.0 || m_csParameters->csdef.prj_prm2  != 0.0)
                order =  1;
            else
                order = 0;

            if (order >= 1)
                {
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, coefFrmt);
                GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, coefFrmt);
                GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
                }
            if (order >= 2)
                {
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm3, coefFrmt);
                GCSAsASCStream << "          PARM3: " << ctemp << std::endl;
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm4, coefFrmt);
                GCSAsASCStream << "          PARM4: " << ctemp << std::endl;
                }
            if (order >= 3)
                {
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm5, coefFrmt);
                GCSAsASCStream << "          PARM5: " << ctemp << std::endl;
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm6, coefFrmt);
                GCSAsASCStream << "          PARM6: " << ctemp << std::endl;
                }
            if (order >= 4)
                {
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm7, coefFrmt);
                GCSAsASCStream << "          PARM7: " << ctemp << std::endl;
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm8, coefFrmt);
                GCSAsASCStream << "          PARM8: " << ctemp << std::endl;
                }
            if (order >= 5)
                {
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm9, coefFrmt);
                GCSAsASCStream << "          PARM9: " << ctemp << std::endl;
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm10, coefFrmt);
                GCSAsASCStream << "          PARM10: " << ctemp << std::endl;
                }
            if (order >= 6)
                {
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm11, coefFrmt);
                GCSAsASCStream << "          PARM11: " << ctemp << std::endl;
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm12, coefFrmt);
                GCSAsASCStream << "          PARM12: " << ctemp << std::endl;
                }
            if (order >= 7)
                {
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm13, coefFrmt);
                GCSAsASCStream << "          PARM13: " << ctemp << std::endl;
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm14, coefFrmt);
                GCSAsASCStream << "          PARM14: " << ctemp << std::endl;
                }
            if (order >= 8)
                {
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm15, coefFrmt);
                GCSAsASCStream << "          PARM15: " << ctemp << std::endl;
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm16, coefFrmt);
                GCSAsASCStream << "          PARM16: " << ctemp << std::endl;
                }
            if (order >= 9)
                {
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm17, coefFrmt);
                GCSAsASCStream << "          PARM17: " << ctemp << std::endl;
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm18, coefFrmt);
                GCSAsASCStream << "          PARM18: " << ctemp << std::endl;
                }
            if (order >= 10)
                {
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm19, coefFrmt);
                GCSAsASCStream << "          PARM19: " << ctemp << std::endl;
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm20, coefFrmt);
                GCSAsASCStream << "          PARM20: " << ctemp << std::endl;
                }
            if (order >= 11)
                {
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm21, coefFrmt);
                GCSAsASCStream << "          PARM21: " << ctemp << std::endl;
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm22, coefFrmt);
                GCSAsASCStream << "          PARM22: " << ctemp << std::endl;
                }
            if (order >= 12)
                {
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm23, coefFrmt);
                GCSAsASCStream << "          PARM23: " << ctemp << std::endl;
                CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm24, coefFrmt);
                GCSAsASCStream << "          PARM24: " << ctemp << std::endl;
                }
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_NZLND:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_SINUS:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_ORTHO:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_GNOMC:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_EDCYL:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, latFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_EDCYLE:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, latFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_PCARREE:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_VDGRN:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_WINKL:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, latFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_CSINI:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_ROBIN:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_BONNE:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_EKRT4:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_EKRT6:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_MOLWD:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_HMLSN:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_NACYL:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, latFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_TACYL:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_BPCNC:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, latFrmt);
            GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm3, lngFrmt);
            GCSAsASCStream << "          PARM3: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm4, latFrmt);
            GCSAsASCStream << "          PARM4: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm5, anglFrmt);
            GCSAsASCStream << "          PARM5: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm6, latFrmt);
            GCSAsASCStream << "          PARM6: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm7, latFrmt);
            GCSAsASCStream << "          PARM7: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_SWISS:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_PSTRO:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_OSTRO:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_SSTRO:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_LM1SP:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_LM2SP:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, latFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, latFrmt);
            GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_LMBLG:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, latFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, latFrmt);
            GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_WCCSL:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, latFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, latFrmt);
            GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm3, zzFrmt);
            GCSAsASCStream << "          PARM3: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm4, zzFrmt);
            GCSAsASCStream << "          PARM4: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_WCCST:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, zzFrmt);
            GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm3, zzFrmt);
            GCSAsASCStream << "          PARM3: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_MNDOTL:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, latFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, latFrmt);
            GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm3, zzFrmt);
            GCSAsASCStream << "          PARM3: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_MNDOTT:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, zzFrmt);
            GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_SOTRM:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_UTM:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, 1L);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, 1L);
            GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
            break;

//case  cs_PRJCOD_UTMZNBF:
//            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, 1L);
//            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
//            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, 1L);
//            GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
//            break;

        case  cs_PRJCOD_TRMRS:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        //case  cs_PRJCOD_TRMERBF:
        //    CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
        //    GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
        //    CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
        //    GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
        //    CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
        //    GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
        //    CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
        //    GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
        //    CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
        //    GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
        //    break;

        case  cs_PRJCOD_HOM1UV:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, latFrmt);
            GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm3, anglFrmt);
            GCSAsASCStream << "          PARM3: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_HOM1XY:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, latFrmt);
            GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm3, anglFrmt);
            GCSAsASCStream << "          PARM3: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_MNDOTOBL:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, latFrmt);
            GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm3, anglFrmt);
            GCSAsASCStream << "          PARM3: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm4, zzFrmt);
            GCSAsASCStream << "          PARM4: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;


        case  cs_PRJCOD_HOM2UV:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, latFrmt);
            GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm3, lngFrmt);
            GCSAsASCStream << "          PARM3: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm4, latFrmt);
            GCSAsASCStream << "          PARM4: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case  cs_PRJCOD_HOM2XY:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, latFrmt);
            GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm3, lngFrmt);
            GCSAsASCStream << "          PARM3: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm4, latFrmt);
            GCSAsASCStream << "          PARM4: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;
        // COORDSYS_RSKEW
        case  cs_PRJCOD_RSKEW:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, latFrmt);
            GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm3, anglFrmt);
            GCSAsASCStream << "          PARM3: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;
        // COORDSYS_RSKWC
        case  cs_PRJCOD_RSKEWC:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, latFrmt);
            GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm3, anglFrmt);
            GCSAsASCStream << "          PARM3: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;
        // COORDSYS_GAUSK
        case  cs_PRJCOD_GAUSSK:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;
        // COORDSYS_KRVKP
        // COORDSYS_KRVKR
        // COORDSYS_KRVKG
        case cs_PRJCOD_KROVAK:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, latFrmt);
            GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm3, latFrmt);
            GCSAsASCStream << "          PARM3: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;
        case cs_PRJCOD_KROVK1:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, latFrmt);
            GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm3, latFrmt);
            GCSAsASCStream << "          PARM3: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG" << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;
        case cs_PRJCOD_KRVK95:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, latFrmt);
            GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm3, latFrmt);
            GCSAsASCStream << "          PARM3: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG" << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;
        case cs_PRJCOD_KRVK951:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, latFrmt);
            GCSAsASCStream << "          PARM2: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm3, latFrmt);
            GCSAsASCStream << "          PARM3: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG" << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case cs_PRJCOD_MRCATK:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.scl_red, redFrmt);
            GCSAsASCStream << "        SCL_RED: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

        case cs_PRJCOD_PSTROSL:
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, latFrmt);
            GCSAsASCStream << "          PARM1: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lng, lngFrmt);
            GCSAsASCStream << "        ORG_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.org_lat, latFrmt);
            GCSAsASCStream << "        ORG_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.x_off, xyFrmt);
            GCSAsASCStream << "          X_OFF: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.y_off, xyFrmt);
            GCSAsASCStream << "          Y_OFF: " << ctemp << std::endl;
            break;

                case cs_PRJCOD_SYS34:
                case cs_PRJCOD_SYS34_99:
                case cs_PRJCOD_SYS34_01:
                        /* These projection have every parameters hard coded ... nothing to add */

        case  cs_PRJCOD_OBLQM:
            /* Should never get here.  This code is never used as there
               are several variations of this projection, and the codes
               for each of the variations are the codes you will see. */

        case cs_PRJCOD_OCCNC:
            /* Should never get here.  This code was never used and was
               essentially established as a placeholder for a projection
               which never got implemented. */
    /*  case cs_PRJCOD_STERO: */
            /* This code is obsolete since about release 8.  The original
               stereographic has been replaced by the Polar Stereographic,
               the Oblique Stereographic, and the Snyder Stereographic. */
        default:
            /* Should never get here.  Probably should issue a message
               of some sort. */
        break;
        }

    /* Finish off with some standard stuff; i.e. applies to all projections. */
    if (m_csParameters->csdef.quad != 0)
        {
        GCSAsASCStream << "           QUAD: " << m_csParameters->csdef.quad << std::endl;
        }
    if (m_csParameters->csdef.hgt_lng != 0.0 || m_csParameters->csdef.hgt_lat != 0.0 || m_csParameters->csdef.hgt_zz != 0.0)
        {
        CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.hgt_lng, lngFrmt);
        GCSAsASCStream << "        HGT_LNG: " << ctemp << std::endl;
        CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.hgt_lat, latFrmt);
        GCSAsASCStream << "        HGT_LAT: " << ctemp << std::endl;
        CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.hgt_zz, zzFrmt);
        GCSAsASCStream << "         HGT_ZZ: " << ctemp << std::endl;
        }
    if (m_csParameters->csdef.ll_min [LNG] != 0.0 || m_csParameters->csdef.ll_min [LAT] != 0.0 ||
        m_csParameters->csdef.ll_max [LNG] != 0.0 || m_csParameters->csdef.ll_max [LAT] != 0.0)
        {
        if (m_csParameters->prj_code == cs_PRJCOD_UNITY)
            {
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm1, lngFrmt);
            GCSAsASCStream << "        MIN_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.ll_min [LAT], latFrmt);
            GCSAsASCStream << "        MIN_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.prj_prm2, lngFrmt);
            GCSAsASCStream << "        MAX_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.ll_max [LAT], latFrmt);
            GCSAsASCStream << "        MAX_LAT: " << ctemp << std::endl;
            }
        else
            {
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.ll_min [LNG], lngFrmt);
            GCSAsASCStream << "        MIN_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.ll_min [LAT], latFrmt);
            GCSAsASCStream << "        MIN_LAT: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.ll_max [LNG], lngFrmt);
            GCSAsASCStream << "        MAX_LNG: " << ctemp << std::endl;
            CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.ll_max [LAT], latFrmt);
            GCSAsASCStream << "        MAX_LAT: " << ctemp << std::endl;
            }
        }
    if (m_csParameters->csdef.xy_min [LNG] != 0.0 || m_csParameters->csdef.xy_min [LAT] != 0.0 ||
        m_csParameters->csdef.xy_max [LNG] != 0.0 || m_csParameters->csdef.xy_max [LAT] != 0.0)
        {
        CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.xy_min [LNG], xyFrmt);
        GCSAsASCStream << "         MIN_XX: " << ctemp << std::endl;
        CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.xy_min [LAT], xyFrmt);
        GCSAsASCStream << "         MIN_YY: " << ctemp << std::endl;
        CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.xy_max [LNG], xyFrmt);
        GCSAsASCStream << "         MAX_XX: " << ctemp << std::endl;
        CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.xy_max [LAT], xyFrmt);
        GCSAsASCStream << "         MAX_YY: " << ctemp << std::endl;
        }
    CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.zero [XX], xyFrmt);
    GCSAsASCStream << "         ZERO_X: " << ctemp << std::endl;
    CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.zero [YY], xyFrmt);
    GCSAsASCStream << "         ZERO_Y: " << ctemp << std::endl;
    CS_ftoa (ctemp, sizeof (ctemp), m_csParameters->csdef.map_scl, sclFrmt);
    GCSAsASCStream << "        MAP_SCL: " << ctemp << std::endl;

    /* Write an extra new line to indicate the end
       of the coordinate system.  Not necessary, but
       makes the string a lot easier to read. */

    GCSAsASCStream << std::endl;

    GCSAsASC = GCSAsASCStream.str();

    /* Return the status code. */
    return SUCCESS;
    }
#endif // DICTIONARY_MANAGEMENT_ONLY

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     DatumEquivalent
(
CSDatum&    datum1,
CSDatum&    datum2
)
    {
    if (!BaseGCS::IsLibraryInitialized())
        return false;

    // if keynames are the same we can skip this.
    if ( (0 != datum1.key_nm[0]) && (0 != datum2.key_nm[0]) && (0 == strcmp (datum1.key_nm, datum2.key_nm)) )
        return true;

    if (datum1.to84_via != datum2.to84_via)
        return false;

    if (!doubleSame (datum1.e_rad, datum2.e_rad))
        return false;

    if (!doubleSame (datum1.p_rad, datum2.p_rad))
        return false;

    if (!doubleSame (datum1.delta_X, datum2.delta_X))
        return false;

    if (!doubleSame (datum1.delta_Y, datum2.delta_Y))
        return false;

    if (!doubleSame (datum1.delta_Z, datum2.delta_Z))
        return false;

    if (!doubleSame (datum1.rot_X, datum2.rot_X))
        return false;

    if (!doubleSame (datum1.rot_Y, datum2.rot_Y))
        return false;

    if (!doubleSame (datum1.rot_Z, datum2.rot_Z))
        return false;

    // Starting with latest version of csmap the geodetic transformation is not part of the 
    // datum proper (except for fallback backward compatibility) so it may happen that
    // datum have identical definitions but should be considered different.
    // To verify we must generate the datum converter and verify they are equivalent
    //
    // Note that the following process may result into false-positives but we have decided that better be safe than sorry and
    // go through an unrequired reprojection instead to taking the risk of considering datums equivalent while they are not.
    CSDatumDef* wgs84Def = CSMap::CS_dtdef("WGS84");

    // If we did not get the datum definition then we will consider the datums equivalent
    if (NULL == wgs84Def)
        return true;

    CSDatum* wgs84 = CSdtloc1(wgs84Def);

    if (NULL == wgs84)
        {
        return true;
        }


    CSDatumConvert* theDatumConverter1 = CSMap::CSdtcsu (&datum1, wgs84);
    CSDatumConvert* theDatumConverter2 = CSMap::CSdtcsu (&datum2, wgs84);

    // If no datum converter can be created we cannot judge the equivalence.
    // We will consider the datums equal since in all likelyhood they effectively are.
    if (NULL == theDatumConverter1 && NULL == theDatumConverter2)
        return true;

    // If only one is null then we will assume different datums since they would not result into
    // any equivalent transformation
    if (NULL != theDatumConverter1 && NULL == theDatumConverter2)
        {
        CSMap::CS_dtcls (theDatumConverter1);
        return false;
        }
    if (NULL == theDatumConverter1 && NULL != theDatumConverter2)
        {
        CSMap::CS_dtcls (theDatumConverter2);
        return false;
        }


    // Now we must analyse the datum converter to determine if the transformation is equivalent.
    // Notice that there is no function provided by CSMAP for the purpose yet.
    bool datumsEquivalent = true;

    if (theDatumConverter1->xfrmCount != theDatumConverter2->xfrmCount)
        datumsEquivalent = false;

    // For every individual transformation part of the convertion path ...
    for (int idxXForms=0 ; datumsEquivalent && (idxXForms < theDatumConverter1->xfrmCount); idxXForms++)
        {
        // Compare selected fields only
        if ((theDatumConverter1->xforms[idxXForms]->methodCode != theDatumConverter1->xforms[idxXForms]->methodCode) ||
            (theDatumConverter1->xforms[idxXForms]->isNullXfrm != theDatumConverter1->xforms[idxXForms]->isNullXfrm) ||
            (theDatumConverter1->xforms[idxXForms]->maxItr != theDatumConverter1->xforms[idxXForms]->maxItr) ||
            (theDatumConverter1->xforms[idxXForms]->inverseSupported != theDatumConverter1->xforms[idxXForms]->inverseSupported) ||
            (theDatumConverter1->xforms[idxXForms]->maxIterations != theDatumConverter1->xforms[idxXForms]->maxIterations) ||
            (theDatumConverter1->xforms[idxXForms]->userDirection != theDatumConverter1->xforms[idxXForms]->userDirection) ||
            (theDatumConverter1->xforms[idxXForms]->cnvrgValue != theDatumConverter1->xforms[idxXForms]->cnvrgValue) ||
            (theDatumConverter1->xforms[idxXForms]->errorValue != theDatumConverter1->xforms[idxXForms]->errorValue) ||
            (theDatumConverter1->xforms[idxXForms]->accuracy != theDatumConverter1->xforms[idxXForms]->accuracy))
            datumsEquivalent = false;

        // So far so good but now we must check the specific parameters. Except for grid shift files we can assume
        // a simple compare will do the job.
        // NOTE: Abridged Molodenski stores for debugging purposes the names of the datums
        // as part of its structure. As the names may be different this would result into those being
        // considered different. Since csmap has not  activated Abridged Molodenski yet an we do not intend to use it we will
        // simply live with these eventual false-negatives.
        // For grid shift files since the pointers to file names will be different even if refering to the same file we must be 
        // more precise.
        if (datumsEquivalent)
            {
            // If it is one of the grid shift file method
            if ((cs_DTCPRMTYP_GRIDINTP & cs_DTCPRMTYP_MASK) == (theDatumConverter1->xforms[idxXForms]->methodCode & cs_DTCPRMTYP_MASK))
                {
                struct csGridi_* datum1GridXForm = (struct csGridi_*)&(theDatumConverter1->xforms[idxXForms]->xforms.gridi);
                struct csGridi_* datum2GridXForm = (struct csGridi_*)&(theDatumConverter2->xforms[idxXForms]->xforms.gridi);

                // First test the various numeric parameters
                if ((datum1GridXForm->maxIterations != datum2GridXForm->maxIterations) ||
                    (datum1GridXForm->userDirection != datum2GridXForm->userDirection) ||
                    (datum1GridXForm->useBest != datum2GridXForm->useBest) ||
                    (datum1GridXForm->fallbackDir != datum2GridXForm->fallbackDir) ||
                    (datum1GridXForm->fileCount != datum2GridXForm->fileCount))
                    datumsEquivalent = false;

                // Fallback requires some detail checking
                if (datumsEquivalent)
                    {
                    if (datum1GridXForm->fallback != datum2GridXForm->fallback)
                        {
                        // The fallbacks pointed may be different but then they must refer to the same fallback transform name.
                        if ((NULL == datum1GridXForm->fallback) || (NULL == datum2GridXForm->fallback))
                            datumsEquivalent = false;

                        if ((datumsEquivalent) && (0 != strncmp(datum1GridXForm->fallback->xfrmName, datum2GridXForm->fallback->xfrmName, sizeof (datum2GridXForm->fallback->xfrmName))))
                            datumsEquivalent = false;     
                        }
                    }

                if (datumsEquivalent)
                    {
                    // All that remains is to compare file names and individual grid shift file method params. Notice that the order of the files is important
                    // for grid shift files so we impose the exact same order also
                    for (short fileIdx = 0 ; datumsEquivalent && (fileIdx < datum1GridXForm->fileCount) ; fileIdx++)
                        {
                        // Only selected fields are tested as the structure contains cache and buffering members.
                        if ((datum1GridXForm->gridFiles[fileIdx]->direction != datum2GridXForm->gridFiles[fileIdx]->direction) ||
                            (datum1GridXForm->gridFiles[fileIdx]->format != datum2GridXForm->gridFiles[fileIdx]->format) ||
                            (datum1GridXForm->gridFiles[fileIdx]->density != datum2GridXForm->gridFiles[fileIdx]->density) ||
                            (datum1GridXForm->gridFiles[fileIdx]->errorValue != datum2GridXForm->gridFiles[fileIdx]->errorValue) ||
                            (datum1GridXForm->gridFiles[fileIdx]->cnvrgValue != datum2GridXForm->gridFiles[fileIdx]->cnvrgValue) ||
                            (datum1GridXForm->gridFiles[fileIdx]->maxIterations != datum2GridXForm->gridFiles[fileIdx]->maxIterations))
                            datumsEquivalent = false;

                        // All that remains to check is the filename
                        if (datumsEquivalent)
                            {
                            datumsEquivalent = (0 == strncmp(datum1GridXForm->gridFiles[fileIdx]->filePath, datum2GridXForm->gridFiles[fileIdx]->filePath, MAXPATH));
                            }
                        }
                    }
                }
            else
                {
                // None of the other methods have pointer outside theuir structure except for Abridged Molodenski we do not use anyway
                // and currently deactivated by csmap we simply compare byte-wise
                datumsEquivalent = (0 == memcmp((Byte*) &(theDatumConverter1->xforms[idxXForms]), (Byte*) &(theDatumConverter2->xforms[idxXForms]), sizeof(theDatumConverter1->xforms[idxXForms])));
                } 
            }
        }
        
    // Release the datum converters.
    CSMap::CS_dtcls (theDatumConverter1);
    CSMap::CS_dtcls (theDatumConverter2);

    return datumsEquivalent;
    }

#ifdef UNUSED_CODE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
static bool     EllipsoidEquivalent
(
CSDatum&            datum,
CSEllipsoidDef&     ellipsoid
)
    {
    return true;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   06/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BaseGCS::HasEquivalentDatum
(
BaseGCSCR        compareTo
) const
    {
    return DatumEquivalent (m_csParameters->datum, compareTo.m_csParameters->datum);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
static int      EPSGCodeFromName
(
CharCP    epsgName
)
    {
    // if the name is of the form EPSG:xxxx, use the xxxx
    if (0 == strncmp (epsgName, "EPSG:", 5))
        {
        int     epsgNum;
        if (1 == sscanf (&epsgName[5], "%d", &epsgNum))
            return epsgNum;
        }
    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
int             BaseGCS::GetEPSGCode (bool noSearch) const
    {
    if (NULL == m_csParameters)
        return 0;

    // if the epsgNbr is in the CS, use that.
    if (0 != m_csParameters->csdef.epsgNbr)
        return m_csParameters->csdef.epsgNbr;

    int     epsgNum;
    if (0 != (epsgNum = EPSGCodeFromName (m_csParameters->csdef.key_nm)))
        return epsgNum;

    // try CS_Map's internal lookup table.
    if (KcsNmInvNumber != (epsgNum = csMapNameToIdC (csMapProjGeoCSys,
                                                     csMapFlvrEpsg,
                                                     csMapFlvrCsMap,
                                                     m_csParameters->csdef.key_nm)))
        return epsgNum;

    if (noSearch)
        return 0;

    // we didn't find an EPSG Number the easy way, have to search.
    int         index;
    char        csKeyName[128];
    BaseGCSP    gcs = NULL;
    for (index = 0; (0 < CSMap::CS_csEnum (index, csKeyName, sizeof(csKeyName))); index++)
        {
        WString keyNameString (csKeyName,false);

        if (NULL == gcs)
            gcs = new BaseGCS (keyNameString.c_str());
         else
            gcs->SetFromCSName (keyNameString.c_str());

        if (gcs->IsValid())
            {
            if ( (0 != (epsgNum = gcs->m_csParameters->csdef.epsgNbr)) || (0 != (epsgNum = EPSGCodeFromName (gcs->m_csParameters->csdef.key_nm))) )
                {
                if (this->IsEquivalent (*gcs))
                    {
                    delete gcs;
                    return epsgNum;
                    }
                }
            }
        }

    if (NULL != gcs)
        delete gcs;

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
int             BaseGCS::GetEPSGDatumCode
(
bool    noSearch
) const
    {
    if (NULL == m_csParameters)
        return 0;

    int     epsgNum;
    if (0 != (epsgNum = EPSGCodeFromName (m_csParameters->datum.key_nm)))
        return epsgNum;

    // CS_Map does not copy the epsgNbr from the _CS_dtdef structure to the _CS_datum structure, so we have to look up the cs_dtdef
    CSDatumDef *dtDef;
    if (NULL != (dtDef = CSMap::CS_dtdef (m_csParameters->datum.key_nm)))
        {
        epsgNum = dtDef->epsgNbr;
        CSMap::CS_free (dtDef);
        if (0 != epsgNum)
            return epsgNum;
        }

    // try CS_Map's internal lookup table.
    if (KcsNmInvNumber != (epsgNum = csMapNameToIdC (csMapDatumKeyName,
                                                     csMapFlvrEpsg,
                                                     csMapFlvrCsMap,
                                                     m_csParameters->datum.key_nm)))
        return epsgNum;

    // Also try the autodesk flavor as we did introduce quite a few of these
    if (KcsNmInvNumber != (epsgNum = csMapNameToIdC (csMapDatumKeyName,
                                                     csMapFlvrEpsg,
                                                     csMapFlvrAutodesk,
                                                     m_csParameters->datum.key_nm)))
        return epsgNum;

    if (noSearch)
        return 0;

    // we didn't find an EPSG Number the easy way, have to search.
    int     index;
    char    dtKeyName[128];
    for (index = 0; (0 < CSMap::CS_dtEnum (index, dtKeyName, sizeof(dtKeyName))); index++)
        {
        if (NULL != (dtDef = CSMap::CS_dtdef (dtKeyName)))
            {
            if (0 == (epsgNum = dtDef->epsgNbr))
                epsgNum = EPSGCodeFromName (dtDef->key_nm);

            CSMap::CS_free (dtDef);

            // if it might be an EPSG datum, compare it to our datum.
            if (0 != epsgNum)
                {
                CSDatum*       datum;
                if (NULL != (datum = CSMap::CS_dtloc (dtKeyName)))
                    {
                    if (DatumEquivalent (m_csParameters->datum, *datum))
                        {
                        CSMap::CS_free (datum);
                        return epsgNum;
                        }
                    CSMap::CS_free (datum);
                    }
                }
            }
        }

    return 0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
int             BaseGCS::GetEPSGEllipsoidCode
(
bool    noSearch
) const
    {
    if (NULL == m_csParameters)
        return 0;

    int     epsgNum;
    if (0 != (epsgNum = EPSGCodeFromName (m_csParameters->datum.ell_knm)))
        return epsgNum;

    // CS_Map does not copy the epsgNbr from the _CS_eldef structure to the _CS_datum structure, so we have to look up the cs_eldef
    CSEllipsoidDef *elDef;
    if (NULL != (elDef = CSMap::CS_eldef (m_csParameters->datum.ell_knm)))
        {
        epsgNum = elDef->epsgNbr;
        CSMap::CS_free (elDef);
        if (0 != epsgNum)
            return epsgNum;
        }

    // try CS_Map's internal lookup table.
    if (KcsNmInvNumber != (epsgNum = csMapNameToIdC (csMapEllipsoidKeyName,
                                                     csMapFlvrEpsg,
                                                     csMapFlvrCsMap,
                                                     m_csParameters->datum.ell_knm)))
        return epsgNum;

    if (KcsNmInvNumber != (epsgNum = csMapNameToIdC (csMapEllipsoidKeyName,
                                                     csMapFlvrEpsg,
                                                     csMapFlvrAutodesk,
                                                     m_csParameters->datum.ell_knm)))
        return epsgNum;


    if (noSearch)
        return 0;

    // we didn't find an EPSG Number the easy way, have to search.
    int     index;
    char    elKeyName[128];
    for (index = 0; (0 < CS_elEnum (index, elKeyName, sizeof(elKeyName))); index++)
        {
        if (NULL != (elDef = CSMap::CS_eldef (elKeyName)))
            {
            if (0 == (epsgNum = elDef->epsgNbr))
                epsgNum = EPSGCodeFromName (elDef->key_nm);

            // if it might be an EPSG ellipsoid , compare it to the info in our datum.
            if (0 != epsgNum)
                {
                if (doubleSame (m_csParameters->datum.e_rad, elDef->e_rad) && doubleSame (m_csParameters->datum.p_rad, elDef->p_rad))
                    {
                    CSMap::CS_free (elDef);
                    return epsgNum;
                    }
                }
            CSMap::CS_free (elDef);
            }
        }

    return 0;

    }

// ----------------------------------------------------------------------------------
// These Methods are related to the ability to have a local coordinate system that is
// related to the Cartesian coordinate system by the LocalTransformerP

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            BaseGCS::SetLocalTransformer
(
LocalTransformerP   transformer
)
    {
    m_localTransformer = transformer;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
LocalTransformerP   BaseGCS::GetLocalTransformer () const
    {
    return m_localTransformer.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            BaseGCS::InternalCartesianFromCartesian
(
DPoint3dR       outInternalCartesian,
DPoint3dCR      inCartesian
) const
    {
    if (m_localTransformer.IsNull())
        outInternalCartesian = inCartesian;
    else
        m_localTransformer->InternalCartesianFromCartesian (outInternalCartesian, inCartesian);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            BaseGCS::CartesianFromInternalCartesian
(
DPoint3dR       outCartesian,
DPoint3dCR      inInternalCartesian
) const
    {
    if (m_localTransformer.IsNull())
        outCartesian = inInternalCartesian;
    else
        m_localTransformer->CartesianFromInternalCartesian (outCartesian, inInternalCartesian);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            BaseGCS::InternalCartesianFromCartesian2D
(
DPoint2dR       outInternalCartesian,
DPoint2dCR      inCartesian
) const
    {
    if (m_localTransformer.IsNull())
        outInternalCartesian = inCartesian;
    else
        m_localTransformer->InternalCartesianFromCartesian2D (outInternalCartesian, inCartesian);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            BaseGCS::CartesianFromInternalCartesian2D
(
DPoint2dR       outCartesian,
DPoint2dCR      inInternalCartesian
) const
    {
    if (m_localTransformer.IsNull())
        outCartesian = inInternalCartesian;
    else
        m_localTransformer->CartesianFromInternalCartesian2D (outCartesian, inInternalCartesian);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
LocalTransformerP       LocalTransformer::CreateLocalTransformer (LocalTransformType transformType, double parameters[12])
    {
    if (TRANSFORM_Helmert == transformType)
        return HelmertLocalTransformer::Create (parameters[0], parameters[1], parameters[2], parameters[3], parameters[4]);

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool                    LocalTransformer::IsEquivalent (LocalTransformerPtr const& transformer1, LocalTransformerPtr const& transformer2)
    {
    if ( transformer1.IsNull() != transformer2.IsNull() )
        return false;

    if (transformer1.IsNull())
        return true;

    return transformer1->IsEquivalent (transformer2.get());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/10
+---------------+---------------+---------------+---------------+---------------+------*/
LocalTransformer::LocalTransformer () {}
LocalTransformer::~LocalTransformer () {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
HelmertLocalTransformer::HelmertLocalTransformer (double a, double b, double c, double d, double e)
    {
    m_a = a;
    m_b = b;
    m_c = c;
    m_d = d;
    m_e = e;

    ComputeInverse ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
HelmertLocalTransformer*         HelmertLocalTransformer::Create (double a, double b, double c, double d, double e)
    {
    return new HelmertLocalTransformer (a, b, c, d, e);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            HelmertLocalTransformer::ComputeInverse ()
    {
    double  aSqPlusbSq = m_a * m_a + m_b * m_b;
    m_inverseA =  m_a         / aSqPlusbSq;
    m_inverseB = -1.0 * m_b   / aSqPlusbSq;
    m_inverseC = -1.0 * (m_a * m_c + m_b * m_d) / aSqPlusbSq;
    m_inverseD = (m_b * m_c - m_a * m_d) / aSqPlusbSq;
    m_inverseE = -1.0 * m_e;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            HelmertLocalTransformer::InternalCartesianFromCartesian (DPoint3dR outInternalCartesian, DPoint3dCR inCartesian) const
    {
    // this must work when outInternalCartesian and inCartesian are the same reference.
    double x = inCartesian.x;
    double y = inCartesian.y;
    outInternalCartesian.x = m_a * x - m_b * y + m_c;
    outInternalCartesian.y = m_b * x + m_a * y + m_d;
    outInternalCartesian.z = inCartesian.z + m_e;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            HelmertLocalTransformer::CartesianFromInternalCartesian (DPoint3dR outCartesian, DPoint3dCR inInternalCartesian) const
    {
    // this must work when outCartesian and inInternalCartesian are the same reference.
    double x = inInternalCartesian.x;
    double y = inInternalCartesian.y;
    outCartesian.x = m_inverseA * x - m_inverseB * y + m_inverseC;
    outCartesian.y = m_inverseB * x + m_inverseA * y + m_inverseD;
    outCartesian.z = inInternalCartesian.z + m_inverseE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            HelmertLocalTransformer::InternalCartesianFromCartesian2D (DPoint2dR outInternalCartesian, DPoint2dCR inCartesian) const
    {
    // this must work when outInternalCartesian and inCartesian are the same reference.
    double x = inCartesian.x;
    double y = inCartesian.y;
    outInternalCartesian.x = m_a * x - m_b * y + m_c;
    outInternalCartesian.y = m_b * x + m_a * y + m_d;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            HelmertLocalTransformer::CartesianFromInternalCartesian2D (DPoint2dR outCartesian, DPoint2dCR inInternalCartesian) const
    {
    // this must work when outCartesian and inInternalCartesian are the same reference.
    double x = inInternalCartesian.x;
    double y = inInternalCartesian.y;
    outCartesian.x = m_inverseA * x - m_inverseB * y + m_inverseC;
    outCartesian.y = m_inverseB * x + m_inverseA * y + m_inverseD;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            HelmertLocalTransformer::GetInternalCartesianFromCartesianTransform (TransformR transform)
    {
    transform.form3d[0][0] = m_a;
    transform.form3d[0][1] = -1.0 * m_b;
    transform.form3d[0][2] = 0.0;
    transform.form3d[0][3] = m_c;

    transform.form3d[1][0] = m_b;
    transform.form3d[1][1] = m_a;
    transform.form3d[1][2] = 0.0;
    transform.form3d[1][3] = m_d;

    transform.form3d[2][0] = 0.0;
    transform.form3d[2][1] = 0.0;
    transform.form3d[2][2] = 1.0;
    transform.form3d[2][3] = m_e;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            HelmertLocalTransformer::GetCartesianFromInternalCartesianTransform (TransformR transform)
    {
    transform.form3d[0][0] = m_inverseA;
    transform.form3d[0][1] = -1.0 * m_inverseB;
    transform.form3d[0][2] = 0.0;
    transform.form3d[0][3] = m_inverseC;

    transform.form3d[1][0] = m_inverseB;
    transform.form3d[1][1] = m_inverseA;
    transform.form3d[1][2] = 0.0;
    transform.form3d[1][3] = m_inverseD;

    transform.form3d[2][0] = 0.0;
    transform.form3d[2][1] = 0.0;
    transform.form3d[2][2] = 1.0;
    transform.form3d[2][3] = m_inverseE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            HelmertLocalTransformer::GetDescription (WString& description) const
    {
    description.assign (L"Helmert Transformation");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            HelmertLocalTransformer::IsEquivalent (LocalTransformerCP other) const
    {
    HelmertLocalTransformer const*  otherHelmert;
    if (NULL == (otherHelmert = dynamic_cast <HelmertLocalTransformer const*> (other)))
        return false;
    return ( (m_a == otherHelmert->m_a) && (m_b == otherHelmert->m_b) && (m_c == otherHelmert->m_c) && (m_d == otherHelmert->m_d) && (m_e == otherHelmert->m_e));
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            HelmertLocalTransformer::SaveParameters (uint16_t& transformType, double parameters[12]) const
    {
    transformType = TRANSFORM_Helmert;
    parameters[0] = m_a;
    parameters[1] = m_b;
    parameters[2] = m_c;
    parameters[3] = m_d;
    parameters[4] = m_e;
    }

/*---------------------------------------------------------------------------------**//**
* Read local transform parameters from memory.
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            HelmertLocalTransformer::ReadParameters (double parameters[12])
    {
    m_a = parameters[0];
    m_b = parameters[1];
    m_c = parameters[2];
    m_d = parameters[3];
    m_e = parameters[4];

    ComputeInverse();
    }

double          HelmertLocalTransformer::GetA () const {return m_a;}
double          HelmertLocalTransformer::GetB () const {return m_b;}
double          HelmertLocalTransformer::GetC () const {return m_c;}
double          HelmertLocalTransformer::GetD () const {return m_d;}
double          HelmertLocalTransformer::GetE () const {return m_e;}

void            HelmertLocalTransformer::SetA (double val) { m_a = val; ComputeInverse(); }
void            HelmertLocalTransformer::SetB (double val) { m_b = val; ComputeInverse(); }
void            HelmertLocalTransformer::SetC (double val) { m_c = val; ComputeInverse(); }
void            HelmertLocalTransformer::SetD (double val) { m_d = val; ComputeInverse(); }
void            HelmertLocalTransformer::SetE (double val) { m_e = val; ComputeInverse(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/10
+---------------+---------------+---------------+---------------+---------------+------*/
LocalTransformerP   HelmertLocalTransformer::Copy () const
    {
    return new HelmertLocalTransformer (m_a, m_b, m_c, m_d, m_e);
    }

// End of Local Coordinate methods.
// ----------------------------------------------------------------------------------


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus BaseGCS::LatLongFromCartesian
(
GeoPointR       outLatLong,         // <= latitude longitude
DPoint3dCR      inCartesian         // => cartesian, in GCS's units.
) const
    {
    if (NULL == m_csParameters)
        return (ReprojectStatus)GEOCOORDERR_InvalidCoordSys;

    DPoint3d    internalCartesian;
    InternalCartesianFromCartesian (internalCartesian, inCartesian);
    return (ReprojectStatus) CSMap::CS_cs3ll (m_csParameters, &outLatLong, &internalCartesian);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus BaseGCS::LatLongFromCartesian2D
(
GeoPoint2dR     outLatLong,         // <= latitude longitude
DPoint2dCR      inCartesian         // => cartesian, in GCS's units.
) const
    {
    if (NULL == m_csParameters)
        return (ReprojectStatus)GEOCOORDERR_InvalidCoordSys;

    DPoint2d    internalCartesian;
    InternalCartesianFromCartesian2D (internalCartesian, inCartesian);

    // unfortunately, CS_cs2ll takes 3d points.
    DPoint3d    cartesian3d = {internalCartesian.x, internalCartesian.y, 0.0};
    GeoPoint    outLatLong3d;
    StatusInt status = CSMap::CS_cs2ll (m_csParameters, &outLatLong3d, &cartesian3d);
    outLatLong.Init (outLatLong3d.longitude, outLatLong3d.latitude);

    return (ReprojectStatus) status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus BaseGCS::CartesianFromLatLong
(
DPoint3dR       outCartesian,       // <= cartesian, in GCS's units.
GeoPointCR      inLatLong           // => latitude longitude
) const
    {
    if (NULL == m_csParameters)
        return (ReprojectStatus)GEOCOORDERR_InvalidCoordSys;

    ReprojectStatus     status;
    DPoint3d    internalCartesian;

    status = (ReprojectStatus) CSMap::CS_ll3cs (m_csParameters, &internalCartesian, &inLatLong);

    // In case a hard error occured ... we zero out all values
    if ((REPROJECT_Success != status) && (REPROJECT_CSMAPERR_OutOfUsefulRange != status) && (REPROJECT_CSMAPERR_VerticalDatumConversionError != status) )
        outCartesian.x = outCartesian.y = outCartesian.z = 0.0;
    else
        CartesianFromInternalCartesian (outCartesian, internalCartesian);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus BaseGCS::CartesianFromLatLong2D
(
DPoint2dR       outCartesian,       // <= cartesian, in GCS's units.
GeoPoint2dCR    inLatLong           // => latitude longitude
) const
    {
    if (NULL == m_csParameters)
        return (ReprojectStatus)GEOCOORDERR_InvalidCoordSys;

    StatusInt   status;

    GeoPoint    inLatLong3d;
    inLatLong3d.Init (inLatLong.longitude, inLatLong.latitude, 0.0);

    DPoint3d    internalCartesian3d;
    status = CSMap::CS_ll2cs (m_csParameters, &internalCartesian3d, &inLatLong3d);

    // In case a hard error occured ... we zero out all values
    if ((SUCCESS != status) && (cs_CNVRT_USFL != status))
        outCartesian.x = outCartesian.y = 0.0;
    else
        {
        outCartesian.x = internalCartesian3d.x;
        outCartesian.y = internalCartesian3d.y;
        CartesianFromInternalCartesian2D (outCartesian, outCartesian);
        }

    return (ReprojectStatus) status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
double          BaseGCS::UnitsFromMeters
(
) const
    {
    if (NULL == m_csParameters)
        return 1.0; // We return 1 in case someone tries to invert without checking against zero

    return m_csParameters->csdef.scale;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
BaseGCS::RangeTestResult BaseGCS::CheckGeoPointRange
(
GeoPointCR      points,
int             numPoints
) const
    {
    if (NULL == m_csParameters)
        return RangeTestOutsideMathDomain;

    return (BaseGCS::RangeTestResult) CSMap::CS_llchk (m_csParameters, numPoints, &points);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
BaseGCS::RangeTestResult BaseGCS::CheckCartesianRange
(
DPoint3dCR      points,
int             numPoints
) const
    {
    if (NULL == m_csParameters)
        return RangeTestOutsideMathDomain;

    return (BaseGCS::RangeTestResult) CSMap::CS_xychk (m_csParameters, numPoints, &points);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnProjectionTypes     BaseGCS::DgnProjectionTypeFromCSDefName
(
CharCP        projectionKeyName
)
    {
    if (!BaseGCS::IsLibraryInitialized())
        return COORDSYS_NONE;

    int             iCoordSys;
    CoordSysData*   coordSys;

    for (iCoordSys=0, coordSys = csDataMap; iCoordSys < DIM (csDataMap); iCoordSys++, coordSys++)
        {
        if ( (NULL != coordSys->csMapKeyName) && (0 == strcmp (projectionKeyName, coordSys->csMapKeyName)) )
            return coordSys->dgnProjectionType;
        }
    return COORDSYS_NONE;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/13
+---------------+---------------+---------------+---------------+---------------+------*/
DgnProjectionTypes     BaseGCS::DgnProjectionTypeFromCSMapProjectionCode
(
BaseGCS::ProjectionCodeValue projectionCode
)
    {
    if (!BaseGCS::IsLibraryInitialized())
        return COORDSYS_NONE;

    int             iCoordSys;
    CoordSysData*   coordSys;

    for (iCoordSys=0, coordSys = csDataMap; iCoordSys < DIM (csDataMap); iCoordSys++, coordSys++)
        {
        if ( (pcvInvalid != coordSys->csMapProjCodeValue) && (projectionCode == coordSys->csMapProjCodeValue) )
            return coordSys->dgnProjectionType;
        }
    return COORDSYS_NONE;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
LibraryP            BaseGCS::GetSourceLibrary () const
    {
    // if we don't have a source library, see if we can find it. If the GCS came from a user library, that library 
    //  might not be available, but we look for it.
    if ( (NULL == m_sourceLibrary) && !m_failedToFindSourceLibrary)
        {
        if ( (NULL != m_csParameters) && (0 != m_csParameters->csdef.key_nm[0]) )
            {
            WString     keyName(m_csParameters->csdef.key_nm,false);
            if (NULL == (m_sourceLibrary = LibraryManager::Instance()->FindSourceLibrary (keyName.c_str())))
                m_failedToFindSourceLibrary = true;
            }
        }
    return m_sourceLibrary;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
DatumConverterP BaseGCS::SetDatumConverter
(
BaseGCSCR        destGCS         // => destination coordinate system
)
    {
    return SetupDatumConverterFor(destGCS);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Stephane.Poulin                 10/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DatumConverterP BaseGCS::SetupDatumConverterFor
(
BaseGCSCR        destGCS         // => destination coordinate system
) const
    {
    if (NULL != m_datumConverter)
        {
        m_datumConverter->Destroy();
        m_datumConverter = NULL;
        }
    m_datumConverter = DatumConverter::Create (*this, destGCS);

    if (m_destinationGCS != NULL)
        m_destinationGCS->UnRegisterIsADestinationOf(*this);
        
    m_destinationGCS = &destGCS;
    m_destinationGCS->RegisterIsADestinationOf(*this);

    if (m_datumConverter!=NULL)
        m_datumConverter->SetReprojectElevation (m_reprojectElevation);

    return m_datumConverter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alain.Robert                  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void BaseGCS::ClearCache() const
	{
	// Clean up cache data so we release our hold upon other BaseGCS
    if (NULL != m_datumConverter)
        {
        m_datumConverter->Destroy();
        m_datumConverter = NULL;
        }

	// Normally the caller is the pointed GCS of which the address is m_destinationGCS
	// We do not need to unregister the present GCS from the list of pointed GCS of the caller.
	m_destinationGCS = NULL;
	}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alain.Robert                  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void BaseGCS::RegisterIsADestinationOf(BaseGCSCR baseGCSThatUsesCurrentAsADestination) const
    {
    m_listOfPointingGCS.push_back(&baseGCSThatUsesCurrentAsADestination);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alain.Robert                  05/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void BaseGCS::UnRegisterIsADestinationOf(BaseGCSCR baseGCSThatUsesCurrentAsADestination) const
    {
    for (bvector<BaseGCSCP>::iterator itr = m_listOfPointingGCS.begin() ; itr != m_listOfPointingGCS.end() ; itr++)
        {
        if ((*itr) == &baseGCSThatUsesCurrentAsADestination)
			{
            m_listOfPointingGCS.erase(itr);
			break;
			}
        }
    }

/*---------------------------------------------------------------------------------**//**

* @bsimethod                                    Alain.Robert                   11/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void BaseGCS::SetModified(bool modified)
    {
    if (modified)
        {
        // Clear original settings
        m_originalGeoKeys.clear();
        if (nullptr != m_originalWKT)
            m_originalWKT->clear();
        }

    m_modified = modified;
    }
    

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BaseGCS::SetReprojectElevation (bool value)
    {
    bool    returnValue = m_reprojectElevation;
    m_reprojectElevation = value;

    if (NULL != m_datumConverter)
        m_datumConverter->SetReprojectElevation (value);

    return returnValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            BaseGCS::GetReprojectElevation () const
    {
    return m_reprojectElevation;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus BaseGCS::LatLongFromLatLong
(
GeoPointR       outLatLong,         // <= latitude longitude in destGCS
GeoPointCR      inLatLong,          // => latitude longitude in this GCS
BaseGCSCR       destinationGCS      // => destination coordinate system
) const
    {
    // make sure datum converter is set up for the destination.
    if (&destinationGCS != m_destinationGCS)
        SetupDatumConverterFor(destinationGCS);

    ReprojectStatus status = REPROJECT_Success;
    if (NULL != m_datumConverter)
        status = m_datumConverter->ConvertLatLong3D (outLatLong, inLatLong);
    else
		{
        outLatLong = inLatLong;
	    status = REPROJECT_CSMAPERR_DatumConverterNotSet; // May be interpreted as a warning.
		}

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus BaseGCS::LatLongFromLatLong2D
(
GeoPoint2dR     outLatLong,         // <= latitude longitude in destGCS
GeoPoint2dCR    inLatLong,          // => latitude longitude in this GCS
BaseGCSCR       destinationGCS      // => destination coordinate system
) const
    {


    // make sure datum converter is set up for the destination.
	if (&destinationGCS != m_destinationGCS)
		SetupDatumConverterFor (destinationGCS);

    ReprojectStatus status = REPROJECT_Success;
    if (NULL != m_datumConverter)
        status = m_datumConverter->ConvertLatLong2D (outLatLong, inLatLong);
    else
		{
        outLatLong = inLatLong;
		status = REPROJECT_CSMAPERR_DatumConverterNotSet; // May be interpreted as a warning.
		}

    return status;
    }

static bool     s_radiansPerDegreeInitialized = false;
static double   s_radiansPerDegree;
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
double          BaseGCS::RadiansFromDegrees
(
double      inDegrees
)
    {
    if (!s_radiansPerDegreeInitialized)
        {
        s_radiansPerDegree            = atan (1.0) / 45.0;
        s_radiansPerDegreeInitialized = true;
        }
    return inDegrees * s_radiansPerDegree;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
double          BaseGCS::DegreesFromRadians
(
double      inRadians
)
    {
    if (!s_radiansPerDegreeInitialized)
        {
        s_radiansPerDegree            = atan (1.0) / 45.0;
        s_radiansPerDegreeInitialized = true;
        }
    return inRadians /  s_radiansPerDegree;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Alain.Robert   2016/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED ReprojectStatus   BaseGCS::LatLongFromXYZ
(
GeoPointR       outLatLong,
DPoint3dCR      inXYZ
) const
    {
    if (NULL == m_csParameters)
        return (ReprojectStatus)GEOCOORDERR_InvalidCoordSys;



    CSMap::CS_xyzToLlh (&outLatLong, &inXYZ, m_csParameters->datum.e_rad, m_csParameters->datum.ecent * m_csParameters->datum.ecent);

    if (vdcFromDatum != GetVerticalDatumCode())
        {
        // Elevation is not ellipsoid based which is the result of the previous function
        // We must convert
        VertDatumCode elevationDatumCode = GetVerticalDatumCode();

        // If elevation datum is NGVD29 based then we need to initialize the vertical datum conversion.
        if (vdcNGVD29 == elevationDatumCode)
            if (0 != CSvrtconInit())
                return REPROJECT_CSMAPERR_VerticalDatumConversionError;

        VerticalDatumConverter* vertConverter =  new VerticalDatumConverter (IsNAD27(), vdcFromDatum, elevationDatumCode);
            
        GeoPoint inLatLong = {outLatLong.longitude, outLatLong.latitude, outLatLong.elevation};

        vertConverter->ConvertElevation (outLatLong, inLatLong);

        // I know that not caching the vertical datum converter may prove inefficient for multiple calls
        // but I elected not to clutter the BaseGCS class with yet another cached member since GeoCentric conversion
        // is rare and if used will be few. If usage proves me wrong feel free to add required member.
        delete vertConverter;
        }

    return REPROJECT_Success;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Alain.Robert   2016/08
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED ReprojectStatus   BaseGCS::XYZFromLatLong
(
DPoint3dR       outXYZ,
GeoPointCR      inLatLong
) const
    {
    if (NULL == m_csParameters)
        return (ReprojectStatus)GEOCOORDERR_InvalidCoordSys;

    GeoPoint effectInLatLong = inLatLong;

    if (vdcFromDatum != GetVerticalDatumCode())
        {
        // Elevation is not ellipsoid based which is the required input of the geocentric function
        // We must convert
        VertDatumCode elevationDatumCode = GetVerticalDatumCode();

        // If elevation datum is NGVD29 based then we need to initialize the vertical datum conversion.
        if (vdcNGVD29 == elevationDatumCode)
            if (0 != CSvrtconInit())
                return REPROJECT_CSMAPERR_VerticalDatumConversionError;

        VerticalDatumConverter* vertConverter =  new VerticalDatumConverter (IsNAD27(), elevationDatumCode, vdcFromDatum);
            
        vertConverter->ConvertElevation (effectInLatLong, inLatLong);

        // I know that not caching the vertical datum converter may prove inefficient for multiple calls
        // but I elected not to clutter the BaseGCS class with yet another cached member since GeoCentric conversion
        // is rare and if used will be few. If usage proves me wrong feel free to add required member.
        delete vertConverter;
        }

	CSMap::CS_llhToXyz (&outXYZ, &effectInLatLong, m_csParameters->datum.e_rad, m_csParameters->datum.ecent * m_csParameters->datum.ecent);

    return REPROJECT_Success;
    }


#if defined (TRAVERSE_UNITS)
typdef void (*UnitCallback)(void* callbackArg, CharCP unitName, CharCP pluralName, int system, double factor, int32_t epsgCode, int index);
/*---------------------------------------------------------------------------------**//**
* Traverses all linear units in the CSMap unit table.
* @return   Angular value in degrees
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void     TraverseLinearUnits
(
UnitCallback    callback,
void*           callbackArg
)
    {
    if (!BaseGCS::IsLibraryInitialized())
        {
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        return;
        }

    const struct cs_Unittab_   *pUnit;
    int                         index;
    for (index = 0, pUnit = cs_Unittab; cs_UTYP_END != pUnit->type; pUnit++)
        {
        if (pUnit->type == cs_UTYP_LEN)
            callback (callbackArg, pUnit->name, pUnit->pluralName, pUnit->system, pUnit->factor, pUnit->epsgCode, index);
        }
    }

/*---------------------------------------------------------------------------------**//**
* Traverses all angular units in the CSMap unit table.
* @return   Angular value in degrees
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
void     TraverseAngularUnits
(
UnitCallback    callback,
void*           callbackArg
)
    {
    if (!BaseGCS::IsLibraryInitialized())
        {
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        return;
        }

    const struct cs_Unittab_   *pUnit;
    int                         index;
    for (index = 0, pUnit = cs_Unittab; cs_UTYP_END != pUnit->type; pUnit++)
        {
        if (pUnit->type == cs_UTYP_ANG)
            callback (callbackArg, pUnit->name, pUnit->pluralName, pUnit->system, pUnit->factor, pUnit->epsgCode, index);
        }
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
T_WStringVector*   BaseGCS::GetLinearUnitNames
(
)
    {
    T_WStringVector*    unitNames = new T_WStringVector();

    if (!BaseGCS::IsLibraryInitialized())
        return unitNames;

    const struct cs_Unittab_   *pUnit;
    for (pUnit = cs_Unittab; cs_UTYP_END != pUnit->type; pUnit++)
        {
        if (pUnit->type == cs_UTYP_LEN)
            {
            unitNames->push_back (WString(pUnit->name,false));
            }
        }

    return unitNames;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
T_WStringVector*   BaseGCS::GetUnitNames
(
)
    {
    T_WStringVector*    unitNames = new T_WStringVector();

    if (!BaseGCS::IsLibraryInitialized())
        return unitNames;

    const struct cs_Unittab_   *pUnit;
    for (pUnit = cs_Unittab; cs_UTYP_END != pUnit->type; pUnit++)
        {
        unitNames->push_back (WString(pUnit->name,false));
        }

    return unitNames;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
T_WStringVector*   BaseGCS::GetDatumNames
(
)
    {
    T_WStringVector*    datumNames = new T_WStringVector();

    if (!BaseGCS::IsLibraryInitialized())
        return datumNames;

    int     index;
    char    dtKeyName[128];
    for (index = 0; (0 < CSMap::CS_dtEnum (index, dtKeyName, sizeof(dtKeyName))); index++)
        {
        datumNames->push_back (WString(dtKeyName,false));
        }

    return datumNames;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
T_WStringVector*   BaseGCS::GetEllipsoidNames
(
)
    {
    T_WStringVector*    ellipsoidNames = new T_WStringVector();

    if (!BaseGCS::IsLibraryInitialized())
        return ellipsoidNames;

    int     index;
    char    elKeyName[128];
    for (index = 0; (0 < CS_elEnum (index, elKeyName, sizeof(elKeyName))); index++)
        {
        ellipsoidNames->push_back (WString(elKeyName,false));
        }

    return ellipsoidNames;
    }




/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
DatumConverter::DatumConverter
(
CSDatumConvert*         datumConvert,
VerticalDatumConverter* verticalDatumConverter
)
    {
    if (!BaseGCS::IsLibraryInitialized())
        return;


    m_datumConvert              = datumConvert;
    m_verticalDatumConverter    = verticalDatumConverter;

    // Allow soft errors to go through and let caller decide of the way to process ... (Soft errors will have return status greater than 0)
    if (NULL != m_datumConvert)
        m_datumConvert->block_err   = cs_DTCFLG_BLK_W;

    // by default, we don't convert elevations when we're converting 3D.
    // to make it do so, call SetReprojectElevation (true);
    m_3dDatumConvertFunc        = CSMap::CS_dtcvt;
    m_reprojectElevation        = false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
static VertDatumCode   NetVerticalDatum (BaseGCSCR gcs)
    {
    VertDatumCode vdc = gcs.GetVerticalDatumCode();

    if (vdcFromDatum == vdc)
        {
        if (gcs.IsNAD27())
            return vdcNGVD29;
        else if (gcs.IsNAD83())
            return vdcNAVD88;
        }


    return vdc;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   09/09
+---------------+---------------+---------------+---------------+---------------+------*/
VerticalDatumConverter*         GetVerticalDatumConverter
(
BaseGCSCR       from,
BaseGCSCR       to
)
    {
    if (!BaseGCS::IsLibraryInitialized())
        return NULL;

    if (!from.IsValid() || !to.IsValid())
        return NULL;

    // Handled by CS-Map? CS-Map will handle ellipsoidal height (vdcFromDatum) conversion given 3D transformations
    // are requested. 
    if ( (vdcFromDatum == from.GetVerticalDatumCode()) && (vdcFromDatum == to.GetVerticalDatumCode()) )
        return NULL;

    // Given at least one GCS uses a non-ellipsoidal height then some vertical datum must be provided.

    // The case that is not handled directly by CS_Map is when we have a vertical datum that does
    // not match the Horizontal Datum. This is a capability we added for 08.11.07 for the Caltrans
    // benchmark. As of 08.11.07, only NAD27 and NAD83 datums were handled, and in those cases,
    // the vertical datum can be set to either NGVD29 or NAVD88.
    // Both from and to have to be NAD27 or NAD83 for us to take action.
    // Starting with CONNECT edition we support the generic Geoid datum 
    // This will use the data specified in the GeoidHeight.gdc catalog

    // are they the same?
    VertDatumCode   fromVDC = NetVerticalDatum (from);
    VertDatumCode   toVDC   = NetVerticalDatum (to);
    if (fromVDC == toVDC)
        return NULL;

    // If either vertical datum codes are NGVD29 or NAVD88 then we init
    // the VERTCON american vertical datum system
    if (fromVDC == vdcNGVD29 || fromVDC == vdcNAVD88 || toVDC == vdcNGVD29 || toVDC == vdcNAVD88)
        if (0 != CSvrtconInit())
            return NULL;

    // This value is irrelevant when we are not performing NGVD29/NAVD88 vertical datum shift.
    bool fromIsNAD27 = from.IsNAD27();
    return new VerticalDatumConverter (fromIsNAD27, fromVDC, toVDC);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
DatumConverterP         DatumConverter::Create
(
BaseGCSCR       from,
BaseGCSCR       to
)
    {
    if (!BaseGCS::IsLibraryInitialized())
        return NULL;

    if (!from.IsValid() || !to.IsValid())
        return NULL;

    VerticalDatumConverter* verticalDatumConverter  = GetVerticalDatumConverter (from, to);

    CSDatumConvert  *datumConvert = CSMap::CS_dtcsu (from.GetCSParameters(), to.GetCSParameters());

    if ( (NULL != datumConvert) || (NULL != verticalDatumConverter) )
        return new DatumConverter (datumConvert, verticalDatumConverter);
    else
        return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
DatumConverterP         DatumConverter::Create
(
DatumCR     from,
DatumCR     to
)
    {
    if (!BaseGCS::IsLibraryInitialized())
        return NULL;

    if (!from.IsValid() || !to.IsValid())
        return NULL;

    CSDatum* srcCSDatum = from.GetCSDatum();
    CSDatum* dstCSDatum = to.GetCSDatum();
    if ( (NULL == srcCSDatum) || (NULL == dstCSDatum) )
        return NULL;

    CSDatumConvert  *datumConvert = CSMap::CSdtcsu (srcCSDatum, dstCSDatum);

    if (NULL != datumConvert)
        return new DatumConverter (datumConvert, NULL);
    else
        return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
DatumConverter::~DatumConverter
(
)
    {
    if (!BaseGCS::IsLibraryInitialized())
        return ;

    if (NULL != m_datumConvert)
        {
        CSMap::CS_dtcls (m_datumConvert);
        m_datumConvert = NULL;
        }
    if (NULL != m_verticalDatumConverter)
        {
        delete m_verticalDatumConverter;
        m_verticalDatumConverter = NULL;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            DatumConverter::Destroy () const { delete this; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus DatumConverter::ConvertLatLong3D
(
GeoPointR   outLatLong,
GeoPointCR  inLatLong
) const
    {
    ReprojectStatus status = REPROJECT_Success;
    if (NULL != m_datumConvert)
        status = (ReprojectStatus) (*m_3dDatumConvertFunc) (m_datumConvert, &inLatLong, &outLatLong);
    else
        outLatLong = inLatLong;

    if (m_reprojectElevation && (NULL != m_verticalDatumConverter))
        {
        StatusInt verticalStatus; 
        verticalStatus = m_verticalDatumConverter->ConvertElevation (outLatLong, inLatLong);
        
        // horizontal status has precedence
        if ((REPROJECT_Success == status) && (SUCCESS != verticalStatus))
            status = REPROJECT_CSMAPERR_VerticalDatumConversionError;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/07
+---------------+---------------+---------------+---------------+---------------+------*/
ReprojectStatus DatumConverter::ConvertLatLong2D
(
GeoPoint2dR   outLatLong,
GeoPoint2dCR  inLatLong
) const
    {
    if (NULL == m_datumConvert)
        {
        // this is the case where we had to Create a non-null DatumConverter because of a vertical datum shift.
        outLatLong = inLatLong;
        return REPROJECT_Success;
        }

    GeoPoint inLatLong3d;
    inLatLong3d.Init (inLatLong.longitude, inLatLong.latitude, 0.0);

    GeoPoint outLatLong3d;
    ReprojectStatus status = (ReprojectStatus) CSMap::CS_dtcvt (m_datumConvert, &inLatLong3d, &outLatLong3d);
    outLatLong.Init (outLatLong3d.longitude, outLatLong3d.latitude);

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DatumConverter::SetReprojectElevation   // <= returns old value.
(
bool            reprojectElevation
)
    {
    bool    wasReprojecting = m_reprojectElevation;

    if (true == (m_reprojectElevation = reprojectElevation))
        {
        m_3dDatumConvertFunc    = (NULL == m_verticalDatumConverter) ? CSMap::CS_dtcvt3D : CSMap::CS_dtcvt;
        }
    else
        {
        m_3dDatumConvertFunc    = CSMap::CS_dtcvt;
        }

    return wasReprojecting;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool            DatumConverter::GetReprojectElevation
(
) const
    {
    return m_reprojectElevation;
    }



/*=================================================================================**//**
* Group class.
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP             Group::GetName() { return m_name.c_str(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP             Group::GetDescription () { return m_description.c_str(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
MemberEnumerator*   Group::GetMemberEnumerator() { return new MemberEnumerator (m_name.c_str()); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
GroupEnumerator*    Group::GetGroupEnumerator() { return new GroupEnumerator (); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
Group::~Group () {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            Group::Destroy () const { delete this; }

/*=================================================================================**//**
* GroupEnumerator class.
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
GroupEnumerator::GroupEnumerator () {m_currentIndex = -1; m_currentGroup = NULL;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            GroupEnumerator::MoveNext()
    {
    if (!BaseGCS::IsLibraryInitialized())
        return false;

    m_currentIndex++;

    if (NULL != m_currentGroup)
        {
        delete m_currentGroup;
        m_currentGroup = NULL;
        }

    char        groupName[1024];
    char        groupDescription[2048];
    if (0 < CSMap::CS_csGrpEnum (m_currentIndex, groupName, sizeof(groupName), groupDescription, sizeof groupDescription))
        {
        m_currentGroup = new Group (WString(groupName,false), WString(groupDescription,false));
        return true;
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
Group*          GroupEnumerator::GetCurrent() 
    { 
    return m_currentGroup; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
GroupEnumerator::~GroupEnumerator ()
    {
    if (NULL != m_currentGroup)
        delete m_currentGroup;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            GroupEnumerator::Destroy () const { delete this; }


/*=================================================================================**//**
* MemberEnumerator class.
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
MemberEnumerator::MemberEnumerator (WCharCP groupName)
    {
    m_groupName     = groupName;
    m_currentIndex  = -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
MemberEnumerator::~MemberEnumerator () {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            MemberEnumerator::Destroy () const { delete this; }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
bool            MemberEnumerator::MoveNext()
    {
    if (!BaseGCS::IsLibraryInitialized())
        return false;

    m_currentIndex++;
    CSGroupList         csGroupList;
    AString             groupName (m_groupName.c_str());
    if (0 < CSMap::CS_csEnumByGroup (m_currentIndex, groupName.c_str(), &csGroupList))
        {
        // found a coordinate system.
        m_currentGCSName.AssignA (csGroupList.key_nm);
        m_currentGCSDescription.AssignA (csGroupList.descr);
        return true;
        }

    m_currentGCSName.clear();
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         MemberEnumerator::GetCurrentGCSName() { return m_currentGCSName.c_str(); }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   11/07
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP         MemberEnumerator::GetCurrentGCSDescription() { return m_currentGCSDescription.c_str(); }


/*=================================================================================**//**
* Unit Class - exposes CSMap unit information
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
UnitCP          Unit::FindUnit (WCharCP unitName)
    {
    if (!BaseGCS::IsLibraryInitialized())
        return NULL;

    const struct cs_Unittab_   *pUnit;
    int                         index;
    AString                     searchString (unitName);
    for (index=0, pUnit = cs_Unittab; cs_UTYP_END != pUnit->type; pUnit++, index++)
        {
        if (0 == BeStringUtilities::Stricmp (searchString.c_str(), pUnit->name))
            return new Unit (index);
        }

    return NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
Unit::Unit (int index)
    {
    if (!BaseGCS::IsLibraryInitialized())
        m_csUnit = NULL;
    else
        m_csUnit = &cs_Unittab[index];

    m_nameString            = NULL;
    m_pluralNameString      = NULL;
    m_abbreviationString    = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP             Unit::GetName() const           
    { 
    if (NULL == m_csUnit)
        return NULL;

    if (NULL == m_nameString)
        m_nameString = new WString (m_csUnit->name,false); 
    return m_nameString->c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP             Unit::GetPluralName() const
    {
    if (NULL == m_csUnit)
        return NULL;

    if (NULL == m_pluralNameString)
        m_pluralNameString = new WString (m_csUnit->pluralName,false); 
    return m_pluralNameString->c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP             Unit::GetAbbreviation() const
    {
    if (NULL == m_csUnit)
        return NULL;

    if (NULL == m_abbreviationString)
        m_abbreviationString = new WString (m_csUnit->abrv,false); 
    return m_abbreviationString->c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
GeoUnitSystem       Unit::GetSystem() const
    {
    if (NULL == m_csUnit)
        return GeoUnitSystem::None;

    // make this line up with MicroStation's definitions.
    if (cs_USYS_Metric == m_csUnit->system)
        return GeoUnitSystem::Metric;
    else if (cs_USYS_English == m_csUnit->system)
        return GeoUnitSystem::English;
    return GeoUnitSystem::None;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
GeoUnitBase     Unit::GetBase() const
    {
    if (NULL == m_csUnit)
        return GeoUnitBase::None;

    if (cs_UTYP_LEN == m_csUnit->type)
        return GeoUnitBase::Meter;
    else if (cs_UTYP_ANG == m_csUnit->type)
        return GeoUnitBase::Degree;
    return GeoUnitBase::None;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
int   Unit::GetEPSGCode() const       
    { 
    if (NULL == m_csUnit)
        return 0;

    return m_csUnit->epsgCode; 
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
double              Unit::GetConversionFactor() const  
    { 
    if (NULL == m_csUnit)
        return 1.0; // Return 1 instead of 0 in case invert is performed without checking

    return m_csUnit->factor; 
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
Unit::~Unit () 
    {
    DELETE_AND_CLEAR (m_nameString);
    DELETE_AND_CLEAR (m_pluralNameString);
    DELETE_AND_CLEAR (m_abbreviationString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            Unit::Destroy () const { delete this; }

/*=================================================================================**//**
* UnitEnumerator Class
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
UnitEnumerator::UnitEnumerator () {m_currentIndex = -1;}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool                UnitEnumerator::MoveNext ()
    {
    if (!BaseGCS::IsLibraryInitialized())
        return false;

    if (m_currentIndex < -1)
        return false;

    m_currentIndex++;
    if ( (cs_UTYP_END != cs_Unittab[m_currentIndex].type) && (cs_UTYP_OFF != cs_Unittab[m_currentIndex].type) )
        return true;

    // set up for repeated failures.
    m_currentIndex = -2;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
UnitCP              UnitEnumerator::GetCurrent()
    {
    if (!BaseGCS::IsLibraryInitialized())
        return NULL;

    if (m_currentIndex < 0)
        return NULL;

    return new Unit(m_currentIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
UnitEnumerator::~UnitEnumerator() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            UnitEnumerator::Destroy () const { delete this; }

/*=================================================================================**//**
* EllipsoidEnumerator Class
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
EllipsoidEnumerator::EllipsoidEnumerator ()
{
m_currentIndex              = -1;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool                EllipsoidEnumerator::MoveNext ()
    {
    if (!BaseGCS::IsLibraryInitialized())
        return false;

    if (m_currentIndex < -1)
        return false;

    m_currentIndex++;

    char    ellipsoidName[512];
    if (1 == CS_elEnum (m_currentIndex, ellipsoidName, _countof (ellipsoidName)))
        {
        m_currentEllipsoidName.AssignA (ellipsoidName);
        return true;
        }

    // set up for repeated failures.
    m_currentEllipsoidName.clear();
    m_currentIndex = -2;
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
EllipsoidCP         EllipsoidEnumerator::GetCurrent()
    {
    if (!BaseGCS::IsLibraryInitialized())
        return NULL;

    if (m_currentIndex < 0)
        return NULL;

    return Ellipsoid::CreateEllipsoid (m_currentEllipsoidName.c_str());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
EllipsoidEnumerator::~EllipsoidEnumerator() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            EllipsoidEnumerator::Destroy () const { delete this; }


/*=================================================================================**//**
* Ellipsoid Class
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
EllipsoidEnumeratorP  Ellipsoid::CreateEnumerator ()
    {
    return new EllipsoidEnumerator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
EllipsoidCP           Ellipsoid::CreateEllipsoid (WCharCP keyName)
    {
    return new Ellipsoid (keyName, NULL);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
EllipsoidCP           Ellipsoid::CreateEllipsoid (WCharCP keyName, LibraryP sourceLibrary)
    {
    return new Ellipsoid (keyName, sourceLibrary);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
EllipsoidCP           Ellipsoid::CreateEllipsoid (EllipsoidCR source)
    {
    return new Ellipsoid (source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
EllipsoidCP           Ellipsoid::CreateEllipsoid (const CSEllipsoidDef& ellipsoidDef, LibraryP sourceLibrary)
    {
    return new Ellipsoid (ellipsoidDef, sourceLibrary);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
Ellipsoid::Ellipsoid (WCharCP keyName, LibraryP sourceLibrary)
    {
    m_ellipsoidDef = NULL;
    m_nameString        = NULL;
    m_descriptionString = NULL;
	m_sourceLibrary     = NULL;

    if (!BaseGCS::IsLibraryInitialized())
        {
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        return;
        }

    // if there's a source library, we need to look in it first.
    if (NULL != sourceLibrary)
        m_ellipsoidDef = sourceLibrary->GetEllipsoid (keyName);

    // if we didn't find it yet, try system library.
    if (NULL == m_ellipsoidDef)
        {
        AString mbEllipsoidName (keyName);
        if (NULL != (m_ellipsoidDef = CSMap::CS_eldef (mbEllipsoidName.c_str())))
            m_sourceLibrary = LibraryManager::Instance()->GetSystemLibrary();
        }
    else
        {
        m_sourceLibrary = sourceLibrary;
        }

    m_csError = (NULL != m_ellipsoidDef) ? 0 : cs_Error;

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
Ellipsoid::Ellipsoid (EllipsoidCR source)
    {
    m_ellipsoidDef      = NULL;
    m_sourceLibrary     = NULL;
    m_nameString        = NULL;
    m_descriptionString = NULL;

    if (!BaseGCS::IsLibraryInitialized())
        {
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        return;
        }

    if (NULL == source.m_ellipsoidDef)
        {
        m_ellipsoidDef = NULL;
        m_sourceLibrary = NULL;
        m_csError = source.m_csError;
        }
    else
        {
        m_ellipsoidDef  = (CSEllipsoidDef*) CS_malc (sizeof(CSEllipsoidDef));
        memcpy (m_ellipsoidDef, source.m_ellipsoidDef, sizeof (CSEllipsoidDef));
        m_sourceLibrary = source.m_sourceLibrary;
        m_csError = source.m_csError;
        }

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
Ellipsoid::Ellipsoid (const CSEllipsoidDef &ellipsoidDef, LibraryP sourceLibrary)
    {
    m_ellipsoidDef  = (CSEllipsoidDef*) CS_malc (sizeof (CSEllipsoidDef));
    memcpy (m_ellipsoidDef, &ellipsoidDef, sizeof(CSEllipsoidDef));
    m_sourceLibrary = sourceLibrary;
    m_csError       = 0;

    m_nameString        = NULL;
    m_descriptionString = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
EllipsoidP          Ellipsoid::Clone () const
    {
    if (NULL == m_ellipsoidDef)
        return NULL;

    return const_cast <EllipsoidP> (CreateEllipsoid (*m_ellipsoidDef, NULL));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool                Ellipsoid::IsValid () const
    {
    return  NULL != m_ellipsoidDef;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
int                 Ellipsoid::GetError () const
    {
    return m_csError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP             Ellipsoid::GetErrorMessage (WStringR errorMsg) const
    {
    if (!BaseGCS::IsLibraryInitialized())
        return errorMsg.c_str();

    char    csErrorMsg[512];
    CSMap::CS_errmsg (csErrorMsg, DIM(csErrorMsg));
    errorMsg.AssignA (csErrorMsg);
    return errorMsg.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP             Ellipsoid::GetName() const
    {
    if (NULL == m_ellipsoidDef)
        return NULL;

    if (NULL == m_nameString)
        m_nameString = new WString (m_ellipsoidDef->key_nm,false);

    return m_nameString->c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           Ellipsoid::SetName (WCharCP value)
    {
    if (NULL == m_ellipsoidDef)
        return GEOCOORDERR_InvalidEllipsoid;

    AString mbName (value);
    if (mbName.length() >= _countof (m_ellipsoidDef->key_nm))
        return GEOCOORDERR_StringTooLong;

    char    copy[1024];
    CS_stncp (copy, mbName.c_str(), _countof (copy));
    if (0 != CS_nampp (copy))
        return GEOCOORDERR_CoordSysIllegalName;

    CS_stncp (m_ellipsoidDef->key_nm, mbName.c_str(), _countof (m_ellipsoidDef->key_nm));
    m_ellipsoidDef->epsgNbr = 0;

    DELETE_AND_CLEAR (m_nameString);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP             Ellipsoid::GetDescription() const
    {
    if (NULL == m_ellipsoidDef)
        return NULL;

    if (NULL == m_descriptionString)
        m_descriptionString = new WString (m_ellipsoidDef->name,false);

    return m_descriptionString->c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           Ellipsoid::SetDescription (WCharCP value)
    {
    if (NULL == m_ellipsoidDef)
        return GEOCOORDERR_InvalidEllipsoid;

    AString mbDescription (value);

    if (mbDescription.length() >= _countof (m_ellipsoidDef->name))
        return GEOCOORDERR_StringTooLong;

    CS_stncp (m_ellipsoidDef->name, mbDescription.c_str(), _countof(m_ellipsoidDef->name));

    DELETE_AND_CLEAR (m_descriptionString);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP             Ellipsoid::GetSource (WStringR source) const
    {
    source.clear();
    if (NULL != m_ellipsoidDef)
        source.AssignA (m_ellipsoidDef->source);

    return source.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           Ellipsoid::SetSource (WCharCP value)
    {
    if (NULL == m_ellipsoidDef)
        return GEOCOORDERR_InvalidEllipsoid;

    AString mbSource (value);
    if (mbSource.length() >= _countof (m_ellipsoidDef->source))
        return GEOCOORDERR_StringTooLong;

    CS_stncp (m_ellipsoidDef->source, mbSource.c_str(), _countof(m_ellipsoidDef->source));

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
double              Ellipsoid::GetPolarRadius() const
    {
    return (NULL == m_ellipsoidDef) ? 0.0 : m_ellipsoidDef->p_rad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void                Ellipsoid::SetPolarRadius (double value)
    {
    if (NULL != m_ellipsoidDef)
        {
        m_ellipsoidDef->p_rad = value;
        CalculateParameters (m_ellipsoidDef->flat, m_ellipsoidDef->ecent, m_ellipsoidDef->e_rad, m_ellipsoidDef->p_rad);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
double              Ellipsoid::GetEquatorialRadius() const
    {
    return (NULL == m_ellipsoidDef) ? 0.0 : m_ellipsoidDef->e_rad;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
void                Ellipsoid::SetEquatorialRadius (double value)
    {
    if (NULL != m_ellipsoidDef)
        {
        m_ellipsoidDef->e_rad = value;
        CalculateParameters (m_ellipsoidDef->flat, m_ellipsoidDef->ecent, m_ellipsoidDef->e_rad, m_ellipsoidDef->p_rad);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
double              Ellipsoid::GetEccentricity() const
    {
    return (NULL == m_ellipsoidDef) ? 0.0 : m_ellipsoidDef->ecent;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
int                 Ellipsoid::GetEPSGCode() const
    {
    return (NULL == m_ellipsoidDef) ? 0 : m_ellipsoidDef->epsgNbr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool                Ellipsoid::NameUnique (bool& inSystemLibrary) const
    {
    // it is only valid to call this BEFORE the datum has been added to the library.
    Library*    systemLibrary = LibraryManager::Instance()->GetSystemLibrary();
    WCharCP     name          = this->GetName();

    inSystemLibrary = false;

    if ( (NULL != m_sourceLibrary) && (m_sourceLibrary->EllipsoidInLibrary (name)) )
        return false;

    if ( (NULL != systemLibrary) && (systemLibrary->EllipsoidInLibrary (name)) )
        {
        inSystemLibrary = true;
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           Ellipsoid::AddToLibrary() const
    {
    if (NULL == m_ellipsoidDef)
        return GEOCOORDERR_InvalidEllipsoid;

    bool    inSystemLibrary;
    if (!NameUnique (inSystemLibrary))
        return GEOCOORDERR_DatumNoUniqueName;

    if ( (NULL == m_sourceLibrary) || !m_sourceLibrary->IsUserLibrary())
        return GEOCOORDERR_NotInUserLibrary;

    return m_sourceLibrary->AddEllipsoid (*m_ellipsoidDef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           Ellipsoid::ReplaceInLibrary (EllipsoidP replacement) const
    {
    if (NULL == replacement || NULL == replacement->m_ellipsoidDef)
        return GEOCOORDERR_InvalidEllipsoid;

    if (NULL == m_ellipsoidDef)
        return GEOCOORDERR_InvalidEllipsoid;

    if ( (NULL == m_sourceLibrary) || !m_sourceLibrary->IsUserLibrary())
        return GEOCOORDERR_NotInUserLibrary;

    StatusInt   status;
    if (BSISUCCESS == (status = m_sourceLibrary->ReplaceEllipsoid (*m_ellipsoidDef, *replacement->m_ellipsoidDef)))
        replacement->m_sourceLibrary = m_sourceLibrary;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool            Ellipsoid::CalculateParameters (double& flattening, double& eccentricity, double equatorialRadius, double polarRadius)
    {
    if (0.0 == equatorialRadius)
        return false;

    flattening = (equatorialRadius - polarRadius) / equatorialRadius;
    eccentricity = sqrt (2.0 * flattening - (flattening * flattening));

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
Ellipsoid::~Ellipsoid()
    {
    CSMAP_FREE_AND_CLEAR (m_ellipsoidDef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            Ellipsoid::Destroy () const { delete this; }

#ifdef DICTIONARY_MANAGEMENT_ONLY // Used for internal dictionary management only
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alain.Robert                   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Ellipsoid::OutputAsASC
(
WStringR            EllipsoidAsASC      // The ASC Text
) const
    {
    if (NULL == m_ellipsoidDef)
        return GEOCOORDERR_InvalidEllipsoid;

    StatusInt       status = SUCCESS;

    std::ostringstream EllipsoidAsASCStream(EllipsoidAsASC);

    if (!IsValid())
        return ERROR;

    EllipsoidAsASCStream << "EL_NAME: " <<  m_ellipsoidDef->key_nm << std::endl
             << "        DESC_NM: " <<  m_ellipsoidDef->name << std::endl
             <<"         SOURCE: " << m_ellipsoidDef->source << std::endl
             <<"          E_RAD: " << m_ellipsoidDef->e_rad << std::endl
             <<"          P_RAD: " << m_ellipsoidDef->p_rad << std::endl
             << std::endl;

    EllipsoidAsASC = EllipsoidAsASCStream.str();

    return status;
    }
#endif



/*=================================================================================**//**
* DatumEnumerator Class
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
DatumEnumerator::DatumEnumerator ()
{
m_currentIndex  = -1;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool                DatumEnumerator::MoveNext ()
    {
    if (!BaseGCS::IsLibraryInitialized())
        return false;

    if (m_currentIndex < -1)
        return false;

    m_currentIndex++;

    char    datumName[512];
    if (1 == CSMap::CS_dtEnum (m_currentIndex, datumName, _countof (datumName)))
        {
        m_currentDatumName.AssignA (datumName);
        return true;
        }

    // set up for repeated failures.
    m_currentIndex = -2;
    m_currentDatumName.clear();
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
DatumCP             DatumEnumerator::GetCurrent()
    {
    if (!BaseGCS::IsLibraryInitialized())
        return NULL;

    for (;;)
        {
        if (m_currentIndex < 0)
            return NULL;

        // don't return invalid datum.
        DatumCP datum = Datum::CreateDatum (m_currentDatumName.c_str());
        if (datum->IsValid())
            return datum;

        datum->Destroy();

        if (!MoveNext())
            return NULL;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
DatumEnumerator::~DatumEnumerator() {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            DatumEnumerator::Destroy () const { delete this; }

/*=================================================================================**//**
* Datum Class
+===============+===============+===============+===============+===============+======*/
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
DatumEnumeratorP  Datum::CreateEnumerator ()
    {
    return new DatumEnumerator();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
DatumCP             Datum::CreateDatum (WCharCP keyName)
    {
    return new Datum (keyName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
Datum::Datum (WCharCP keyName)
    {
    if (!BaseGCS::IsLibraryInitialized())
        {
        m_datumDef = NULL;
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        }
    else
        {
        m_datumDef         = CSMap::CS_dtdef (AString (keyName).c_str());
        m_csError          = (NULL != m_datumDef) ? 0 : cs_Error;
        }

    m_csDatum                    = NULL;
    m_ellipsoid                  = NULL;
    m_sourceLibrary              = NULL;
    m_nameString                 = NULL;
    m_descriptionString          = NULL;
    m_ellipsoidNameString        = NULL;
    m_ellipsoidDescriptionString = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
DatumCP             Datum::CreateDatum (CSDatumDef const& datumDef, LibraryP sourceLibrary)
    {
    return new Datum (datumDef, sourceLibrary);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
Datum::Datum (CSDatumDef const& datumDef, LibraryP sourceLibrary)
    {
    if (!BaseGCS::IsLibraryInitialized())
        {
        m_datumDef = NULL;
        m_csError = GEOCOORDERR_GeoCoordNotInitialized;
        }
    else
        {
        m_datumDef                   = (CSDatumDef*) CS_malc (sizeof (CSDatumDef));
        memcpy (m_datumDef, &datumDef, sizeof(CSDatumDef));
        m_csError                    = 0;
        }


    m_csDatum                    = NULL;
    m_ellipsoid                  = NULL;
    m_sourceLibrary              = sourceLibrary;
    m_nameString                 = NULL;
    m_descriptionString          = NULL;
    m_ellipsoidNameString        = NULL;
    m_ellipsoidDescriptionString = NULL;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
DatumP          Datum::Clone () const
    {
    if (NULL == m_datumDef)
        return NULL;

    return const_cast <DatumP> (CreateDatum (*m_datumDef, m_sourceLibrary));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
bool                Datum::IsValid () const
    {
    return  NULL != m_datumDef;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
int                 Datum::GetError () const
    {
    return m_csError;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP             Datum::GetErrorMessage (WStringR errorMsg) const
    {
    if (!BaseGCS::IsLibraryInitialized())
        return errorMsg.c_str();

    char    csErrorMsg[512];
    CSMap::CS_errmsg (csErrorMsg, DIM(csErrorMsg));
    errorMsg.AssignA (csErrorMsg);
    return errorMsg.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP             Datum::GetName() const
    {
    if (NULL == m_datumDef)
        return NULL;

    if (NULL == m_nameString)
        m_nameString = new WString (m_datumDef->key_nm,false);

    return m_nameString->c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           Datum::SetName (WCharCP value)
    {
    if (NULL == m_datumDef)
        return GEOCOORDERR_InvalidDatum;

    AString mbName (value);
    if (mbName.length() >= _countof (m_datumDef->key_nm))
        return GEOCOORDERR_StringTooLong;

    char    copy[1024];
    CS_stncp (copy, mbName.c_str(), _countof (copy));
    if (0 != CS_nampp (copy))
        return GEOCOORDERR_CoordSysIllegalName;

    CS_stncp (m_datumDef->key_nm, mbName.c_str(), _countof (m_datumDef->key_nm));
    m_datumDef->epsgNbr = 0;
    CSMAP_FREE_AND_CLEAR (m_csDatum);

    DELETE_AND_CLEAR (m_nameString);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP             Datum::GetDescription() const
    {
    if (NULL == m_datumDef)
        return NULL;

    if (NULL == m_descriptionString)
        m_descriptionString = new WString (m_datumDef->name, false);

    return m_descriptionString->c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           Datum::SetDescription (WCharCP value)
    {
    if (NULL == m_datumDef)
        return GEOCOORDERR_InvalidDatum;

    AString mbDescription (value);
    if (mbDescription.length() >= _countof (m_datumDef->name))
        return GEOCOORDERR_StringTooLong;

    CS_stncp (m_datumDef->name, mbDescription.c_str(), _countof (m_datumDef->name));
    CSMAP_FREE_AND_CLEAR (m_csDatum);
    DELETE_AND_CLEAR (m_descriptionString);
    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP             Datum::GetSource (WStringR source) const
    {
    source.clear();

    if (NULL == m_datumDef)
        return NULL;

    source.AssignA (m_datumDef->source);

    return source.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           Datum::SetSource (WCharCP value)
    {
    if (NULL == m_datumDef)
        return GEOCOORDERR_InvalidDatum;

    AString mbSource (value);
    if (mbSource.length() >= _countof (m_datumDef->source))
        return GEOCOORDERR_StringTooLong;

    CS_stncp (m_datumDef->source, mbSource.c_str(), _countof (m_datumDef->source));
    CSMAP_FREE_AND_CLEAR (m_csDatum);

    return BSISUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
int                 Datum::GetEPSGCode() const
    {
    return (NULL == m_datumDef) ? 0 : m_datumDef->epsgNbr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
WGS84ConvertCode    Datum::GetConvertToWGS84MethodCode() const
    {
    return (WGS84ConvertCode) ((NULL == m_datumDef) ? 0 : m_datumDef->to84_via);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           Datum::SetConvertToWGS84MethodCode (WGS84ConvertCode value)
    {
    int     intValue = (int) value;
    if ( (intValue < 0) || (intValue > ConvertType_MAXVALUE) )
        return GEOCOORDERR_BadArg;

    if (NULL == m_datumDef)
        return GEOCOORDERR_InvalidDatum;

    m_datumDef->to84_via = static_cast<short>(intValue); // NEEDSWORK - is cast correct?  Can intValue be a short?
    CSMAP_FREE_AND_CLEAR (m_csDatum);
    return BSISUCCESS;;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
void                Datum::GetDelta (DPoint3dR delta) const
    {
    delta.x = delta.y = delta.z = 0.0;

    if (NULL == m_datumDef)
        return;

    bool             deltaValid, rotationValid, scaleValid;
    ParametersValid (deltaValid, rotationValid, scaleValid);
    if (deltaValid)
        {
        delta.x = m_datumDef->delta_X;
        delta.y = m_datumDef->delta_Y;
        delta.z = m_datumDef->delta_Z;
        }
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           Datum::SetDelta (DPoint3dCR delta)
    {
    if (NULL == m_datumDef)
        return GEOCOORDERR_InvalidDatum;

    bool             deltaValid, rotationValid, scaleValid;
    ParametersValid (deltaValid, rotationValid, scaleValid);
    if (deltaValid)
        {
        m_datumDef->delta_X = delta.x;
        m_datumDef->delta_Y = delta.y;
        m_datumDef->delta_Z = delta.z;
        CSMAP_FREE_AND_CLEAR (m_csDatum);
        return BSISUCCESS;
        }
    return GEOCOORDERR_ParameterNotUsed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
void                Datum::GetRotation (DPoint3dR rotation) const
    {
    rotation.x = rotation.y = rotation.z = 0.0;

    if (NULL == m_datumDef)
        return;

    bool             deltaValid, rotationValid, scaleValid;
    ParametersValid (deltaValid, rotationValid, scaleValid);
    if (rotationValid)
        {
        rotation.x = m_datumDef->rot_X;
        rotation.y = m_datumDef->rot_Y;
        rotation.z = m_datumDef->rot_Z;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           Datum::SetRotation (DPoint3dCR rotation)
    {
    if (NULL == m_datumDef)
        return GEOCOORDERR_InvalidDatum;

    bool             deltaValid, rotationValid, scaleValid;
    ParametersValid (deltaValid, rotationValid, scaleValid);
    if (rotationValid)
        {
        m_datumDef->rot_X = rotation.x;
        m_datumDef->rot_Y = rotation.y;
        m_datumDef->rot_Z = rotation.z;
        CSMAP_FREE_AND_CLEAR (m_csDatum);
        return BSISUCCESS;
        }
    return GEOCOORDERR_ParameterNotUsed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alain.Robert                   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
double              Datum::GetScale () const
    {
    if (NULL == m_datumDef)
        return 0.0;

    bool             deltaValid, rotationValid, scaleValid;
    ParametersValid (deltaValid, rotationValid, scaleValid);
    return (scaleValid) ? m_datumDef->bwscale : 0.0;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alain.Robert                   07/08
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           Datum::SetScale (double value)
    {
    if (NULL == m_datumDef)
        return GEOCOORDERR_InvalidDatum;

    bool             deltaValid, rotationValid, scaleValid;
    ParametersValid (deltaValid, rotationValid, scaleValid);
    if (scaleValid)
        {
        m_datumDef->bwscale = value;
        CSMAP_FREE_AND_CLEAR (m_csDatum);
        return BSISUCCESS;
        }
    return GEOCOORDERR_ParameterNotUsed;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP             Datum::GetEllipsoidName() const
    {
    if (NULL == m_datumDef)
        return NULL;

    if (NULL == m_ellipsoidNameString)
        m_ellipsoidNameString = new WString (m_datumDef->ell_knm,false);

    return m_ellipsoidNameString->c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
int                 Datum::GetEllipsoidCode () const
    {
    if (NULL == m_datumDef)
        return -1;

    WString searchName (m_datumDef->ell_knm,false);

    // check user library.
    LibraryP sourceLibrary = GetSourceLibrary();
    if ( (NULL != sourceLibrary) && (sourceLibrary->IsUserLibrary()) )
        {
        uint32_t ellipsoidCount = (uint32_t) m_sourceLibrary->GetEllipsoidCount();
        for (uint32_t iEllipsoid=0; iEllipsoid < ellipsoidCount; iEllipsoid++)
            {
            WString     ellipsoidName;
            m_sourceLibrary->GetEllipsoidName (iEllipsoid, ellipsoidName);
            if (0 == searchName.CompareToI (ellipsoidName))
                return 1000000 + iEllipsoid;
            }
        }

    // check system library/
    char mbEllipsoidName[512];
    for (int index = 0; (0 < CS_elEnum (index, mbEllipsoidName, _countof(mbEllipsoidName))); index++)
        {
        if (0 == BeStringUtilities::Stricmp (m_datumDef->ell_knm, mbEllipsoidName))
            return index;
        }

    return -1;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt       Datum::SetEllipsoidCode (int ellipsoidCode)
    {
    if (NULL == m_datumDef)
        return GEOCOORDERR_InvalidDatum;

    // 2000000 is the separator, do nothing.
    if (2000000 == ellipsoidCode)
        return GEOCOORDERR_InvalidEllipsoidCode;

    CSEllipsoidDef* ellipsoidDef    = NULL;
    bool            fromUserLibrary = false;
    if ( (ellipsoidCode >= 1000000) &&  (NULL != m_sourceLibrary) && m_sourceLibrary->IsUserLibrary ())
        {
        ellipsoidDef = m_sourceLibrary->GetEllipsoid (ellipsoidCode - 1000000);
        fromUserLibrary = true;
        }

    if (NULL == ellipsoidDef)
        {
        char    elKeyName[128];
        if (CS_elEnum (ellipsoidCode, elKeyName, sizeof(elKeyName)) <= 0)
            return GEOCOORDERR_InvalidEllipsoidCode;

        if (NULL == (ellipsoidDef = CSMap::CS_eldef (elKeyName)))
            return GEOCOORDERR_InvalidEllipsoidCode;
        }

    CSDatum*       datum;
    if (NULL == (datum = CSdtloc2 (NULL, ellipsoidDef)))
        {
        CSMap::CS_free (ellipsoidDef);
        return GEOCOORDERR_InvalidEllipsoidCode;
        }

    CSMap::CS_stncp (m_datumDef->ell_knm, ellipsoidDef->key_nm, sizeof (m_datumDef->ell_knm));

    // keep the new datum, discard the old.
    CSMAP_FREE_AND_CLEAR (m_csDatum);
    m_csDatum = datum;

    // create a new ellipsoid from the CSEllipsoidDef, discard the old.
    if (NULL != m_ellipsoid)
        m_ellipsoid->Destroy();
    m_ellipsoid = Ellipsoid::CreateEllipsoid (*ellipsoidDef, fromUserLibrary ? m_sourceLibrary : NULL);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP             Datum::GetEllipsoidDescription() const
    {
    if (NULL == m_datumDef)
        return NULL;

    if (NULL == m_ellipsoid)
        m_ellipsoid = Ellipsoid::CreateEllipsoid (WString(m_datumDef->ell_knm,false).c_str(), m_sourceLibrary);

    return (NULL == m_ellipsoid) ? NULL : m_ellipsoid->GetDescription();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP              Datum::GetEllipsoidSource (WStringR source) const
    {
    source.clear();

    if (NULL == m_datumDef)
        return NULL;

    if (NULL == m_ellipsoid)
        m_ellipsoid = Ellipsoid::CreateEllipsoid (WString(m_datumDef->ell_knm,false).c_str(), m_sourceLibrary);
    return (NULL == m_ellipsoid) ? NULL : m_ellipsoid->GetSource (source);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
double              Datum::GetEllipsoidPolarRadius () const
    {
    if (NULL == m_datumDef)
        return 0.0;

    if (NULL == m_ellipsoid)
        m_ellipsoid = Ellipsoid::CreateEllipsoid (WString(m_datumDef->ell_knm,false).c_str(), m_sourceLibrary);

    return (NULL == m_ellipsoid) ? 0.0 : m_ellipsoid->GetPolarRadius();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
double              Datum::GetEllipsoidEquatorialRadius () const
    {
    if (NULL == m_datumDef)
        return 0.0;

    if (NULL == m_ellipsoid)
        m_ellipsoid = Ellipsoid::CreateEllipsoid (WString(m_datumDef->ell_knm,false).c_str(), m_sourceLibrary);

    return (NULL == m_ellipsoid) ? 0.0 : m_ellipsoid->GetEquatorialRadius();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
double              Datum::GetEllipsoidEccentricity () const
    {
    if (NULL == m_datumDef)
        return 0.0;

    if (NULL == m_ellipsoid)
        m_ellipsoid = Ellipsoid::CreateEllipsoid (WString(m_datumDef->ell_knm,false).c_str(), m_sourceLibrary);

    return (NULL == m_ellipsoid) ? 0.0 : m_ellipsoid->GetEccentricity();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   02/08
+---------------+---------------+---------------+---------------+---------------+------*/
EllipsoidCP         Datum::GetEllipsoid() const
    {
    if (NULL == m_datumDef)
        return NULL;

    // copy an existing ellipsoid.
    if (NULL != m_ellipsoid)
        return Ellipsoid::CreateEllipsoid (*m_ellipsoid);

    return Ellipsoid::CreateEllipsoid (WString(m_datumDef->ell_knm,false).c_str(), m_sourceLibrary);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
void                Datum::ParametersValid (bool& deltaValid, bool& rotationValid, bool& scaleValid) const
    {
    // initialize to defaults.
    deltaValid = rotationValid = scaleValid = false;
    switch (GetConvertToWGS84MethodCode())
        {
        case ConvertType_MOLO:
        case ConvertType_3PARM:
        case ConvertType_GEOCTR:
            deltaValid = true;
            break;

        case ConvertType_BURS:
        case ConvertType_7PARM:
            deltaValid = rotationValid = scaleValid = true;
            break;

        case ConvertType_6PARM:
            deltaValid = rotationValid = true;
            break;

        case ConvertType_4PARM:
            deltaValid = scaleValid = true;
            break;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
bool                Datum::NameUnique (bool& inSystemLibrary) const
    {
    // it is only valid to call this BEFORE the datum has been added to the library.
    Library*    systemLibrary = LibraryManager::Instance()->GetSystemLibrary();
    WCharCP     name          = this->GetName();

    inSystemLibrary = false;

    if ( (NULL != m_sourceLibrary) && (m_sourceLibrary->DatumInLibrary (name)) )
        return false;

    if ( (NULL != systemLibrary) && (systemLibrary->DatumInLibrary (name)) )
        {
        inSystemLibrary = true;
        return false;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
LibraryP            Datum::GetSourceLibrary () const
    {
    return m_sourceLibrary;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           Datum::AddToLibrary() const
    {
    if (NULL == m_datumDef)
        return GEOCOORDERR_InvalidDatum;

    bool    inSystemLibrary;
    if (!NameUnique (inSystemLibrary))
        return GEOCOORDERR_DatumNoUniqueName;

    if ( (NULL == m_sourceLibrary) || !m_sourceLibrary->IsUserLibrary())
        return GEOCOORDERR_NotInUserLibrary;

    return m_sourceLibrary->AddDatum (*m_datumDef);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   05/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt           Datum::ReplaceInLibrary (DatumP replacement) const
    {
    if (NULL == replacement || NULL == replacement->m_datumDef)
        return GEOCOORDERR_InvalidDatum;

    if (NULL == m_datumDef)
        return GEOCOORDERR_InvalidDatum;

    if ( (NULL == m_sourceLibrary) || !m_sourceLibrary->IsUserLibrary())
        return GEOCOORDERR_NotInUserLibrary;

    StatusInt   status;
    if (BSISUCCESS == (status = m_sourceLibrary->ReplaceDatum (*m_datumDef, *replacement->m_datumDef)))
        replacement->m_sourceLibrary = m_sourceLibrary;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   01/10
+---------------+---------------+---------------+---------------+---------------+------*/
CSDatum*            Datum::GetCSDatum() const
    {
    if (NULL != m_csDatum)
        return m_csDatum;

    if (NULL == m_datumDef)
        return NULL;

    CSEllipsoidDef* ellipsoidDef = NULL;
    if ( (NULL != m_sourceLibrary) && m_sourceLibrary->IsUserLibrary ())
        ellipsoidDef = m_sourceLibrary->GetEllipsoid (WString(m_datumDef->ell_knm, false).c_str());

    if (NULL == ellipsoidDef)
        {
        if (NULL == (ellipsoidDef = CSMap::CS_eldef (m_datumDef->ell_knm)))
            return NULL;
        }

    m_csDatum = CSdtloc2 (m_datumDef, ellipsoidDef);
    CSMap::CS_free (ellipsoidDef);

    return m_csDatum;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   03/08
+---------------+---------------+---------------+---------------+---------------+------*/
Datum::~Datum()
    {
    if (NULL != m_datumDef)
        CSMap::CS_free (m_datumDef);

    if (NULL != m_csDatum)
        CSMap::CS_free (m_csDatum);

    if (NULL != m_ellipsoid)
        m_ellipsoid->Destroy();

    DELETE_AND_CLEAR (m_nameString);
    DELETE_AND_CLEAR (m_descriptionString);
    DELETE_AND_CLEAR (m_ellipsoidNameString);
    DELETE_AND_CLEAR (m_ellipsoidDescriptionString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   08/10
+---------------+---------------+---------------+---------------+---------------+------*/
void            Datum::Destroy () const { delete this; }


#ifdef DICTIONARY_MANAGEMENT_ONLY
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alain.Robert                   06/09
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt Datum::OutputAsASC
(
WStringR       DatumAsASC
) const
{
    if (NULL == m_datumDef)
        return GEOCOORDERR_InvalidDatum;

    StatusInt       status = SUCCESS;

    std::ostringstream DatumAsAscStream(DatumAsASC);

    if (!IsValid())
        return ERROR;

    uint32_t     gcTo84via = 0;
    char   szCsTo84Keyname[64];

    szCsTo84Keyname[0] = 0;

    DatumAsAscStream << "DT_NAME: " << m_datumDef->key_nm << std::endl
               << "        DESC_NM: " << m_datumDef->name << std::endl
               << "         SOURCE: " << m_datumDef->source << std::endl
               << "      ELLIPSOID: " << m_datumDef->ell_knm << std::endl;

    switch (m_datumDef->to84_via)
        {
        case cs_DTCTYP_MOLO:
            DatumAsAscStream << "            USE: MOLODENSKY" << std::endl;
            DatumAsAscStream << "        DELTA_X: " << m_datumDef->delta_X << std::endl
                       << "        DELTA_Y: " << m_datumDef->delta_Y << std::endl
                       << "        DELTA_Z: " << m_datumDef->delta_Z << std::endl;
            break;

        case cs_DTCTYP_MREG:
            DatumAsAscStream << "            USE: MULREG" << std::endl;
            //Values are carried but not used.  Meant to provide a quick way of using alternate almost equivalent definition.
            if (!doubleSame (0.0, m_datumDef->delta_X))
                DatumAsAscStream << "        DELTA_X: " << m_datumDef->delta_X << std::endl;
            if (!doubleSame (0.0, m_datumDef->delta_Y))
                DatumAsAscStream << "        DELTA_Y: " << m_datumDef->delta_Y << std::endl;
            if (!doubleSame (0.0, m_datumDef->delta_Z))
                DatumAsAscStream << "        DELTA_Z: " << m_datumDef->delta_Z << std::endl;
            break;

        case cs_DTCTYP_BURS:
            DatumAsAscStream << "            USE: BURSA" << std::endl;
            DatumAsAscStream << "        DELTA_X: " << m_datumDef->delta_X << std::endl
                       << "        DELTA_Y: " << m_datumDef->delta_Y << std::endl
                       << "        DELTA_Z: " << m_datumDef->delta_Z << std::endl
                       << "          ROT_X: " << m_datumDef->rot_X << std::endl
                       << "          ROT_Y: " << m_datumDef->rot_Y << std::endl
                       << "          ROT_Z: " << m_datumDef->rot_Z << std::endl
                       << "        BWSCALE: " << m_datumDef->bwscale << std::endl;
            break;

        case cs_DTCTYP_NAD27:
            DatumAsAscStream << "            USE: NAD27" << std::endl;
            break;

        case cs_DTCTYP_NAD83:
            DatumAsAscStream << "            USE: NAD83" << std::endl;
            break;

        case cs_DTCTYP_WGS84:
            DatumAsAscStream << "            USE: WGS84" << std::endl;
            break;

        case cs_DTCTYP_WGS72:
            DatumAsAscStream << "            USE: WGS72" << std::endl;
            break;

        case cs_DTCTYP_HPGN:
            DatumAsAscStream << "            USE: HPGN" << std::endl;
            break;

        case cs_DTCTYP_7PARM:
            DatumAsAscStream << "            USE: 7PARAMETER" << std::endl;
            DatumAsAscStream << "        DELTA_X: " << m_datumDef->delta_X << std::endl
                       << "        DELTA_Y: " << m_datumDef->delta_Y << std::endl
                       << "        DELTA_Z: " << m_datumDef->delta_Z << std::endl
                       << "          ROT_X: " << m_datumDef->rot_X << std::endl
                       << "          ROT_Y: " << m_datumDef->rot_Y << std::endl
                       << "          ROT_Z: " << m_datumDef->rot_Z << std::endl
                       << "        BWSCALE: " << m_datumDef->bwscale << std::endl;
            break;

        case cs_DTCTYP_AGD66:
            DatumAsAscStream << "            USE: AGD66" << std::endl;
            break;

        case cs_DTCTYP_3PARM:
            DatumAsAscStream << "            USE: 3PARAMETER" << std::endl;
            DatumAsAscStream << "        DELTA_X: " << m_datumDef->delta_X << std::endl
                       << "        DELTA_Y: " << m_datumDef->delta_Y << std::endl
                       << "        DELTA_Z: " << m_datumDef->delta_Z << std::endl;
            break;

        case cs_DTCTYP_6PARM:
            DatumAsAscStream << "            USE: 6PARAMETER" << std::endl;
            DatumAsAscStream << "        DELTA_X: " << m_datumDef->delta_X << std::endl
                       << "        DELTA_Y: " << m_datumDef->delta_Y << std::endl
                       << "        DELTA_Z: " << m_datumDef->delta_Z << std::endl
                       << "          ROT_X: " << m_datumDef->rot_X << std::endl
                       << "          ROT_Y: " << m_datumDef->rot_Y << std::endl
                       << "          ROT_Z: " << m_datumDef->rot_Z << std::endl;
            break;

        case cs_DTCTYP_4PARM:
            DatumAsAscStream << "            USE: 4PARAMETER" << std::endl;
            DatumAsAscStream << "        DELTA_X: " << m_datumDef->delta_X << std::endl
                       << "        DELTA_Y: " << m_datumDef->delta_Y << std::endl
                       << "        DELTA_Z: " << m_datumDef->delta_Z << std::endl
                       << "        BWSCALE: " << m_datumDef->bwscale << std::endl;
            break;

        case cs_DTCTYP_AGD84:
            DatumAsAscStream << "            USE: AGD84" << std::endl;
            break;

        case cs_DTCTYP_NZGD49:
            DatumAsAscStream << "            USE: NZGD49" << std::endl;
            break;

        case cs_DTCTYP_ATS77:
            DatumAsAscStream << "            USE: ATS77" << std::endl;
            break;

        case cs_DTCTYP_GDA94:
            DatumAsAscStream << "            USE: GDA94" << std::endl;
            break;

        case cs_DTCTYP_NZGD2K:
            DatumAsAscStream << "            USE: NZGD2K" << std::endl;
            break;

        case cs_DTCTYP_CSRS:
            DatumAsAscStream << "            USE: CSRS" << std::endl;
            break;

        case cs_DTCTYP_TOKYO:
            DatumAsAscStream << "            USE: JGD2K" << std::endl;
            break;

        case cs_DTCTYP_RGF93:
            DatumAsAscStream << "            USE: RGF93" << std::endl;
            break;

        case cs_DTCTYP_ED50:
            DatumAsAscStream << "            USE: ED50" << std::endl;
            break;

        case cs_DTCTYP_ETRF89:
            DatumAsAscStream << "            USE: ETRF89" << std::endl;
            break;

#ifdef GEOCOORD_ENHANCEMENT
        case cs_DTCTYP_GENGRID:
            DatumAsAscStream << "            USE: GENGRID" << std::endl;
            break;
#endif


        default:
            return ERROR;
        }

    DatumAsAscStream << std::endl;

    DatumAsASC = DatumAsAscStream.str();

    return status;
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Barry.Bentley   07/06
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP    Datum::GetGroup (WStringR groupName) const
    {
    groupName.clear();

    if (NULL != m_datumDef)
        groupName.AssignA (m_datumDef->group);

    return groupName.c_str();
    }

/*=================================================================================**//**
*
* This class has only static methods that map directly to CSMap functions.
*
+===============+===============+===============+===============+===============+======*/
typedef const double PointArray[3];

int             CSMap::CS_csGrpEnum (int index,char *grpName,int nameSize,char *descr,int descrSize) {return ::CS_csGrpEnum (index, grpName, nameSize, descr, descrSize);}
int             CSMap::CS_csEnum (int index,char *keyName,int nameSize) {return ::CS_csEnum (index, keyName, nameSize);}
int             CSMap::CS_csEnumByGroup (int index, Const char *grpName, CSGroupList *cs_descr) {return ::CS_csEnumByGroup (index, grpName, cs_descr);}
CSParameters*   CSMap::CScsloc2 (CSDefinition* csDef, CSDatumDef* dtDef, CSEllipsoidDef* elDef) {return ::CScsloc2 (csDef, dtDef, elDef);}
CSParameters*   CSMap::CScsloc1 (CSDefinition* csDef) {return ::CScsloc1 (csDef);}
char*           CSMap::CS_stncp (char *out, const char *in, int count) {return ::CS_stncp (out, in, count);}
void            CSMap::CS_errmsg (char *out, int bufSize) {::CS_errmsg (out, bufSize);}
int             CSMap::CS_cs3ll (const CSParameters* csprm, GeoPointP ll, DPoint3dCP xy) {return ::CS_cs3ll (csprm, (double*)ll, (const double*)xy);}
int             CSMap::CS_ll3cs (const CSParameters* csprm, DPoint3dP xy, GeoPointCP ll) {return ::CS_ll3cs (csprm, (double*)xy, (const double*)ll);}
int             CSMap::CS_dtcvt3D (CSDatumConvert* dtcvt, GeoPointCP ll_in, GeoPointP ll_out) {return ::CS_dtcvt3D (dtcvt, (const double*)ll_in, (double *)ll_out);}
int             CSMap::CS_cs2ll (const CSParameters* csprm, GeoPointP ll, DPoint3dCP xy) {return ::CS_cs2ll (csprm, (double*)ll, (const double*)xy);}
int             CSMap::CS_ll2cs (const CSParameters* csprm, DPoint3dP xy, GeoPointCP ll) {return ::CS_ll2cs (csprm, (double*)xy, (const double*)ll);}
int             CSMap::CS_dtcvt (CSDatumConvert* dtcvt, GeoPointCP ll_in, GeoPointP ll_out) {return ::CS_dtcvt (dtcvt, (const double*)ll_in, (double *)ll_out);}
CSDatumConvert* CSMap::CS_dtcsu (const CSParameters* src, const CSParameters* dest) {return ::CS_dtcsu (src, dest, cs_DTCFLG_DAT_F, cs_DTCFLG_BLK_F);}
void            CSMap::CS_dtcls (CSDatumConvert* datumConvert) {::CS_dtcls (datumConvert);}
double          CSMap::CS_cscnv (const CSParameters* csprm, GeoPointCP ll) {return ::CS_cscnv (csprm, (const double *)ll);}
double          CSMap::CS_cssch (const CSParameters* csprm, GeoPointCP ll) {return ::CS_cssch (csprm, (const double *)ll);}
double          CSMap::CS_cssck (const CSParameters* csprm, GeoPointCP ll) {return ::CS_cssck (csprm, (const double *)ll);}
double          CSMap::CS_csscl (const CSParameters* csprm, GeoPointCP ll) {return ::CS_csscl (csprm, (const double *)ll);}
void            CSMap::CS_fillIn (CSDefinition* csdef) {::CS_fillIn (csdef);}
int             CSMap::CSerpt (char *mesg,int size,int err_num) {return ::CSerpt (mesg, size, err_num);}
int             CSMap::CS_wktToCsEx (CSDefinition *csDef, CSDatumDef *dtDef, CSEllipsoidDef *elDef, GeoCoordinates::BaseGCS::WktFlavor flavor, CharCP wellKnownText) {return ::CS_wktToCsEx (csDef, dtDef, elDef, (ErcWktFlavor)flavor, wellKnownText, 1);}
bool            CSMap::CS_prjprm (CSParamInfo *info, int projectionCode, int paramNum) {return 0 < ::CS_prjprm (info, (short)projectionCode, paramNum);}
CSEllipsoidDef* CSMap::CS_eldef (const char * keyName) {return ::CS_eldef (keyName);}
CSDatumDef*     CSMap::CS_dtdef (const char * keyName) {return ::CS_dtdef (keyName);}
CSDefinition*   CSMap::CS_csdef (const char * keyName) {return ::CS_csdef (keyName);}
void            CSMap::CS_free  (void * mem) {::CS_free (mem);}
int             CSMap::CS_dtEnum (int index, char *key_name, int name_sz) {return ::CS_dtEnum (index, key_name, name_sz);}
int             CSMap::CS_elEnum (int index, char *key_name, int name_sz) {return ::CS_elEnum (index, key_name, name_sz);}
CSDatum*        CSMap::CS_dtloc (char *key_name) {return ::CS_dtloc (key_name);}
double          CSMap::CS_llazdd (double eRad, double eSq, GeoPointCP startPoint, GeoPointCP endPoint, double *distance) {return ::CS_llazdd (eRad, eSq, (const double *)startPoint, (const double *) endPoint, distance);}
int             CSMap::CS_llchk (const CSParameters* csprm, int numPoints, GeoPointCP ll) {return ::CS_llchk (csprm, numPoints, (PointArray*)ll);}
int             CSMap::CS_xychk (const CSParameters* csprm, int numPoints, DPoint3dCP xy) {return ::CS_llchk (csprm, numPoints, (PointArray*)xy);}
CSDatumConvert* CSMap::CSdtcsu (const CSDatum* src, const CSDatum* dest) {return ::CSdtcsu (src, dest, cs_DTCFLG_DAT_F, cs_DTCFLG_BLK_F);}
CSDatum*        CSMap::CSdtloc1 (const CSDatumDef* datumDef) {return ::CSdtloc1 (datumDef);}
CSUnitInfo const * CSMap::GetCSUnitInfo (int unitIndex)
    {
    CSUnitInfo const*    csUnitInfo = &cs_Unittab[unitIndex];
    return (cs_UTYP_END == csUnitInfo->type) ? NULL : csUnitInfo;
    }

CSMilitaryGrid* CSMap::CSnewMgrs (double e_rad, double e_sq, short bessel) {return ::CSnewMgrs (e_rad, e_sq, bessel); }
int             CSMap::CScalcMgrsFromLl (CSMilitaryGrid* mg, char* result, int size, GeoPoint2dP ll, int precision) {return ::CScalcMgrsFromLl (mg, result, size, (double *)ll, precision); }
int             CSMap::CScalcLlFromMgrs (CSMilitaryGrid* mg, GeoPoint2dP ll, const char* mgrsString) {return ::CScalcLlFromMgrs (mg, (double *)ll, mgrsString); }
void            CSMap::CSdeleteMgrs (CSMilitaryGrid* mg) {::CSdeleteMgrs (mg);}
void            CSMap::CS_llhToXyz (DPoint3dP xyz,const GeoPointCP llh, double e_rad, double e_sq) {::CS_llhToXyz((double*)xyz, (const double*)llh, e_rad, e_sq);}
int             CSMap::CS_xyzToLlh (GeoPointP llh,const DPoint3dCP xyz, double e_rad, double e_sq) {return ::CS_xyzToLlh((double*)llh, (const double*)xyz, e_rad, e_sq);}



#include "GCSLibrary.cpp"

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
MilitaryGridConverter::MilitaryGridConverter (BaseGCSR baseGCS, bool useBessel, bool useWGS84Datum)
    {
    m_toWGS84Converter          = NULL;
    m_fromWGS84Converter        = NULL;

    // can't have both useBessel and useWGS84Datum.
    BeAssert (!useBessel || !useWGS84Datum);

    if (!baseGCS.IsValid())
        {
        m_csMgrs = NULL;
        return;
        }

    if (useBessel)
        {
        // we use the GCS only to get the ellipsoid information.
        // we do not convert the LL to a different datum in this case.

        // use Bessel version of the grid labels for Clarke and Bessel ellipsoids.
        WCharCP         ellipsoidName   = baseGCS.GetEllipsoidName();
        bool            besselEllipsoid = (0 == wcsncmp (L"CLRK", ellipsoidName, 4)) || (0 == wcsncmp (L"BESL", ellipsoidName, 4)) || (0 == wcsncmp (L"BESSEL", ellipsoidName, 6));
        BeAssert (besselEllipsoid);

        double          eccentricity    = baseGCS.GetEllipsoidEccentricity();
        m_csMgrs                        = CSMap::CSnewMgrs (baseGCS.GetEllipsoidEquatorialRadius(), eccentricity*eccentricity, useBessel);
        }
    else
        {
        // this version converts the LL in the GCS to WGS84, and then uses the WGS84-based LL to get the Military Grid Coordinates.
        CSDatum*        sourceDatum     = NULL;
        CSParameters*   sourceCSParams;
        if (NULL != (sourceCSParams = baseGCS.GetCSParameters()))
            sourceDatum = &sourceCSParams->datum;

        double      eccentricity        = sourceDatum->ecent;
        double      equatorialRadius    = sourceDatum->e_rad;

        DatumCP     wgs84Datum          = NULL;
        CSDatum*    wgs84CSDatum        = NULL;
        if (useWGS84Datum)
            {
            // getting WGS84 should never fail.
            if (NULL == (wgs84Datum = GeoCoordinates::Datum::CreateDatum (L"WGS84")))
                {
                BeAssert (false);
                return;
                }

            if (NULL == (wgs84CSDatum = wgs84Datum->GetCSDatum()))
                {
                BeAssert (false);
                wgs84Datum->Destroy();
                return;
                }
            eccentricity     = wgs84CSDatum->ecent;
            equatorialRadius = wgs84CSDatum->e_rad;
            }

        m_csMgrs = CSMap::CSnewMgrs (equatorialRadius, eccentricity*eccentricity, false);

        if ( (NULL != sourceDatum) && (NULL != wgs84CSDatum) )
            {
            CSDatumConvert  *toWGS84CSDatumConvert = CSMap::CSdtcsu (sourceDatum, wgs84CSDatum);
            if (NULL != toWGS84CSDatumConvert)
                m_toWGS84Converter = new DatumConverter (toWGS84CSDatumConvert, NULL);

            CSDatumConvert  *fromWGS84CSDatumConvert = CSMap::CSdtcsu (wgs84CSDatum, sourceDatum);
            if (NULL != fromWGS84CSDatumConvert)
                m_fromWGS84Converter = new DatumConverter (fromWGS84CSDatumConvert, NULL);
            }

        wgs84Datum->Destroy();
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
MilitaryGridConverter::~MilitaryGridConverter ()
    {
    if (NULL != m_toWGS84Converter)
        delete m_toWGS84Converter;
    if (NULL != m_fromWGS84Converter)
        delete m_fromWGS84Converter;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
MilitaryGridConverterPtr   MilitaryGridConverter::CreateConverter (BaseGCSR baseGCS, bool useBessel, bool useWGS84)
    {
    if (!baseGCS.IsValid())
        return NULL;

    return new MilitaryGridConverter (baseGCS, useBessel, useWGS84);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   MilitaryGridConverter::LatLongFromMilitaryGrid (GeoPoint2dR outLatLong, WCharCP mgString)
    {
    StatusInt status = SUCCESS;
    
    GeoPoint2d  tmpLatLong;

    AString mbMGString (mgString);

    if ((NULL != m_csMgrs) && (0 == (status = CSMap::CScalcLlFromMgrs (m_csMgrs, &tmpLatLong, mbMGString.c_str()))))
        {
        if (NULL != m_fromWGS84Converter)
            m_fromWGS84Converter->ConvertLatLong2D (outLatLong, tmpLatLong);
        else
            outLatLong = tmpLatLong;
        }
    else
        {
        outLatLong.latitude = 0.0;
        outLatLong.longitude = 0.0;
        status = ERROR;
        }
    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   06/10
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt   MilitaryGridConverter::MilitaryGridFromLatLong (WString& mgString, GeoPoint2dCR inLatLong, int precision)
    {
    char    mgChar[512];

    GeoPoint2d    outLatLong = inLatLong;

    if (NULL != m_toWGS84Converter)
        m_toWGS84Converter->ConvertLatLong2D (outLatLong, inLatLong);

    // 5 is the maximum precision.
    StatusInt     status = SUCCESS;
    if ((NULL != m_csMgrs) && (0 == (status = CSMap::CScalcMgrsFromLl (m_csMgrs, mgChar, _countof (mgChar), &outLatLong, precision))))
        mgString.AssignA (mgChar);
    else
        {
        mgString.assign (L"");
        status = ERROR;
        }

    return status;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP    BaseGeoCoordResource::GetLocalizedProjectionName
(
WStringR                        outString,
BaseGCS::ProjectionCodeValue    projectionCode
)
    {
    DgnProjectionTypes  dgnProjectionType;

    if (DgnProjectionTypes::COORDSYS_NONE == (dgnProjectionType = BaseGCS::DgnProjectionTypeFromCSMapProjectionCode (projectionCode)))
        {
        outString.clear();
        return outString.c_str();
        }

    return GetLocalizedProjectionName (outString, dgnProjectionType);
    }
	
#if defined (WIP_L10N)


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP     BaseGeoCoordResource::GetLocalizedProjectionName
(
WStringR                    outString,
DgnProjectionTypes          projectionType
)
    {
    OpenResourceFile();

    if (-1 != s_rscFileHandle)
        {
        if (SUCCESS != s_geoCoordResources->GetString (outString, projectionType, MSGLISTID_GeoCoordNames))
            outString.clear();
        }

    return outString.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP BaseGeoCoordResource::GetLocalizedString
(
WStringR                outString,
DgnGeoCoordStrings      stringNum
)
    {
    OpenResourceFile();

    if (-1 != s_rscFileHandle)
        {
        if (SUCCESS != s_geoCoordResources->GetString (outString, stringNum, MSGLISTID_DgnGeoCoordStrings))
            outString.clear();
        }

    return outString.c_str();
    }

#else

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP    BaseGeoCoordResource::GetLocalizedProjectionName
(
WStringR                outString,
DgnProjectionTypes    projectionType
)
    {
    outString.clear();
    return outString.c_str();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   12/09
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP BaseGeoCoordResource::GetLocalizedString
(
WStringR                outString,
DgnGeoCoordStrings      stringNum
)
    {
    return NULL;
    }

#endif
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Mathieu.St-Pierre                  04/11
+---------------+---------------+---------------+---------------+---------------+------*/
//This class was created first to allow the extern C functions used by Imagepp to 
//access the protected and private part of BaseGCS.
class BaseGeoCoordFncHelper
{
private :

    BaseGeoCoordFncHelper();

public : 

    static BaseGCS* baseGeoCoord_clone
    (
    BaseGCS* pBaseGcs
    )
        {
#if defined (GET_RID_OF)
        return new GeoCoordinates::BaseGCS(pBaseGcs);
#else
        return NULL;
#endif
        }
    };
} // Ends GeoCoordinate namespace

END_BENTLEY_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Barry.Bentley                   07/07
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED   void    baseGeoCoord_setUserLibrary
(
WCharCP libPath,
WCharCP guiName
)
    {
    GeoCoordinates::LibraryManager::Instance()->AddUserLibrary(libPath, guiName);
    }

/*=================================================================================**//**
* Base GeoCoordinateSystem API
+===============+===============+===============+===============+===============+======*/
BEGIN_EXTERN_C

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu St-Pierre 12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void baseGeoCoord_initialize
(
WCharCP dataDirectory
)
    {
    GeoCoordinates::BaseGCS::Initialize (dataDirectory);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED   GeoCoordinates::BaseGCS* baseGeoCoord_allocate
(
)
    {
    GeoCoordinates::BaseGCSPtr  baseGCSPtr = GeoCoordinates::BaseGCS::CreateGCS();
    baseGCSPtr->AddRef();
    return baseGCSPtr.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MathieuSt-Pierre  03/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED   GeoCoordinates::BaseGCS* baseGeoCoord_allocateFromBaseGCSKeyName
(
WCharCP keyName // The coordinate system key name.
)
    {
    GeoCoordinates::BaseGCSPtr baseGCSPtr = GeoCoordinates::BaseGCS::CreateGCS(keyName);

    if (baseGCSPtr->IsValid())
        {
        baseGCSPtr->AddRef();
        return baseGCSPtr.get();
        }
    else
        {
        return NULL;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void baseGeoCoord_deallocate
(
GeoCoordinates::BaseGCS* pBaseGcs
)
    {
    pBaseGcs->Release();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MathieuSt-Pierre  08/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED GeoCoordinates::BaseGCS* baseGeoCoord_clone
(
GeoCoordinates::BaseGCS* pBaseGcs
)  
    {     
    GeoCoordinates::BaseGCSPtr baseGCSPtr = GeoCoordinates::BaseGCS::CreateGCS(*pBaseGcs);
    
    baseGCSPtr->AddRef();
    return baseGCSPtr.get();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt baseGeoCoord_initFromGeoTiffKeys
(
StatusInt*                        warning,            // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
WStringP                          warningOrErrorMsg,  // Error message.
IGeoTiffKeysList*                 geoTiffKeys,        // The GeoTiff key list
GeoCoordinates::BaseGCS* pBaseGcs
)
    {
    return pBaseGcs->InitFromGeoTiffKeys(warning, warningOrErrorMsg, geoTiffKeys, true);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt baseGeoCoord_initFromWellKnownText
(
StatusInt*                          warning,            // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
WStringP                            warningOrErrorMsg,  // Error message.
int32_t                             wktFlavor,          // The WKT Flavor.
WCharCP                             wellKnownText,      // The Well Known Text specifying the coordinate system.
GeoCoordinates::BaseGCS*   pBaseGcs
)
    {
    return pBaseGcs->InitFromWellKnownText(warning, warningOrErrorMsg, (GeoCoordinates::BaseGCS::WktFlavor)wktFlavor, wellKnownText);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MathieuSt-Pierre  07/2009
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt baseGeoCoord_setQuadrant
(
short                                value,
GeoCoordinates::BaseGCS*   pBaseGcs
)
    {
    StatusInt Status;

    Status = pBaseGcs->SetQuadrant(value);

    if (Status == SUCCESS)
        Status = pBaseGcs->DefinitionComplete();

    return Status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt baseGeoCoord_reproject
(
double* outCartesianX,
double* outCartesianY,
double  inCartesianX,
double  inCartesianY,
GeoCoordinates::BaseGCS* pSrcGcs,
GeoCoordinates::BaseGCS* pDstGcs
)
    {
    DPoint3d inCartesian;
    StatusInt   status = SUCCESS;
    StatusInt   stat1;
    StatusInt   stat2;
    StatusInt   stat3;


    inCartesian.x = inCartesianX;
    inCartesian.y = inCartesianY;
    inCartesian.z = 0.0;

    GeoPoint inLatLong;
    stat1 = pSrcGcs->LatLongFromCartesian (inLatLong, inCartesian);

    GeoPoint outLatLong;
    stat2 = pSrcGcs->LatLongFromLatLong(outLatLong, inLatLong, *pDstGcs);


    DPoint3d outCartesian;
    stat3 = pDstGcs->CartesianFromLatLong(outCartesian, outLatLong);

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
    


    *outCartesianX = outCartesian.x;
    *outCartesianY = outCartesian.y;

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED double baseGeoCoord_getUnitsFromMeters
(
GeoCoordinates::BaseGCS* pBaseGcs
)
    {
    return pBaseGcs->UnitsFromMeters();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    MathieuSt-Pierre  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED   int baseGeoCoord_getEPSGUnitCode
(
GeoCoordinates::BaseGCS* pBaseGcs
)
    {
    return pBaseGcs->GetEPSGUnitCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt baseGeoCoord_getGeoTiffKeys
(
IGeoTiffKeysList* pList,
GeoCoordinates::BaseGCS* pBaseGcs
)
    {
    return pBaseGcs->GetGeoTiffKeys(pList, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt baseGeoCoord_getWellKnownText
(
WStringR                            wellKnownText,      // The WKT.
int32_t                             wktFlavor,          // The WKT Flavor.
GeoCoordinates::BaseGCS*   pBaseGcs
)
    {
    return pBaseGcs->GetWellKnownText(wellKnownText, (GeoCoordinates::BaseGCS::WktFlavor)wktFlavor, false);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu St-Pierre 12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool baseGeoCoord_isValid
(
GeoCoordinates::BaseGCS* pBaseGcs
)
    {
    return pBaseGcs->IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  08/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED bool baseGeoCoord_isEquivalent
(
GeoCoordinates::BaseGCS* pBaseGcs1,
GeoCoordinates::BaseGCS* pBaseGcs2
)
    {
    return pBaseGcs1->IsEquivalent(*pBaseGcs2);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Marc.Bedard  11/2007
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void baseGeoCoord_getErrorMessage
(
WStringR    errorStr,
int         errorCode
)
    {
    GeoCoordinates::BaseGCS::GetErrorMessage (errorStr, errorCode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu St-Pierre  01/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED void baseGeoCoord_getKeyName
(
WStringR                      keyName,      // The key name
GeoCoordinates::BaseGCS* pBaseGcs
)
    {
    keyName = WString(pBaseGcs->GetName());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Mathieu St-Pierre  12/2008
+---------------+---------------+---------------+---------------+---------------+------*/
BASEGEOCOORD_EXPORTED StatusInt baseGeoCoord_getCartesianFromLatLong
(
double*                           pCartesianPt,
double*                           pGeoPt,
GeoCoordinates::BaseGCS* pBaseGcs
)
    {
    DPoint3d  cartesianPt;

    GeoPoint  geoPt  = {pGeoPt[0], pGeoPt[1], pGeoPt[2]};
    StatusInt status = pBaseGcs->CartesianFromLatLong (cartesianPt, geoPt);

    pCartesianPt[0] = cartesianPt.x;
    pCartesianPt[1] = cartesianPt.y;
    pCartesianPt[2] = cartesianPt.z;

    return status;
    }

END_EXTERN_C
