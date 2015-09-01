//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/interface/IRasterGeoCoordinateServices.h $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <Bentley\Bentley.h>
#include <Bentley\RefCounted.h>

// Forward declarations
BEGIN_BENTLEY_NAMESPACE
namespace GeoCoordinates { class BaseGCS; }
END_BENTLEY_NAMESPACE

IMAGEPP_TYPEDEFS(IRasterBaseGcs)
IMAGEPP_REF_COUNTED_PTR(IRasterBaseGcs)

struct IGeoTiffKeysList;

BEGIN_IMAGEPP_NAMESPACE


class HCPGeoTiffKeys;

// COPIED FROM BASEGEOCOORD.H
enum ProjectionCodeValue
    {
    pcvInvalid                                      = 0,
    pcvUnity                                        = 1,
    pcvTransverseMercator                           = 3,
    pcvAlbersEqualArea                              = 4,
    pcvHotineObliqueMercator                        = 5,
    pcvMercator                                     = 6,
    pcvLambertEquidistantAzimuthal                  = 7,
    pcvLambertTangential                            = 8,
    pcvAmericanPolyconic                            = 9,
    pcvModifiedPolyconic                            = 10,
    pcvLambertEqualAreaAzimuthal                    = 11,
    pcvEquidistantConic                             = 12,
    pcvMillerCylindrical                            = 13,
    pcvModifiedStereographic                        = 15,
    pcvNewZealandNationalGrid                       = 16,
    pcvSinusoidal                                   = 17,
    pcvOrthographic                                 = 18,
    pcvGnomonic                                     = 19,
    pcvEquidistantCylindrical                       = 20,
    pcvVanderGrinten                                = 21,
    pcvCassini                                      = 22,
    pcvRobinsonCylindrical                          = 23,
    pcvBonne                                        = 24,
    pcvEckertIV                                     = 25,
    pcvEckertVI                                     = 26,
    pcvMollweide                                    = 27,
    pcvGoodeHomolosine                              = 28,
    pcvEqualAreaAuthalicNormal                      = 29,
    pcvEqualAreaAuthalicTransverse                  = 30,
    pcvBipolarObliqueConformalConic                 = 31,
    pcvObliqueCylindricalSwiss                      = 32,
    pcvPolarStereographic                           = 33,
    pcvObliqueStereographic                         = 34,
    pcvSnyderObliqueStereographic                   = 35,
    pcvLambertConformalConicOneParallel             = 36,
    pcvLambertConformalConicTwoParallel             = 37,
    pcvLambertConformalConicBelgian                 = 38,
    pcvLambertConformalConicWisconsin               = 39,
    pcvTransverseMercatorWisconsin                  = 40,
    pcvLambertConformalConicMinnesota               = 41,
    pcvTransverseMercatorMinnesota                  = 42,
    pcvSouthOrientedTransverseMercator              = 43,
    pcvUniversalTransverseMercator                  = 44,
    pcvSnyderTransverseMercator                     = 45,
    pcvGaussKrugerTranverseMercator                 = 46,
    pcvCzechKrovak                                  = 47,
    pcvCzechKrovakObsolete                          = 48,
    pcvMercatorScaleReduction                       = 49,
    pcvObliqueConformalConic                        = 50,
    pcvCzechKrovak95                                = 51,
    pcvCzechKrovak95Obsolete                        = 52,
    pcvPolarStereographicStandardLatitude           = 53,
    pcvTransverseMercatorAffinePostProcess          = 54,
    pcvNonEarth                                     = 55,
    pcvObliqueCylindricalHungary                    = 56,
    pcvTransverseMercatorDenmarkSys34               = 57,
    pcvTransverseMercatorOstn97                     = 58,
    pcvAzimuthalEquidistantElevatedEllipsoid        = 59,
    pcvTransverseMercatorOstn02                     = 60,
    pcvTransverseMercatorDenmarkSys3499             = 61,
    pcvTransverseMercatorKruger                     = 62,
    pcvWinkelTripel                                 = 63,
    pcvNonEarthScaleRotation                        = 64,
    pcvLambertConformalConicAffinePostProcess       = 65,
    pcvTransverseMercatorDenmarkSys3401             = 66,
    pcvEquidistantCylindricalEllipsoid              = 67,
    pcvPlateCarree                                  = 68,
    pcvPopularVisualizationPseudoMercator           = 69,
    pcvHotineObliqueMercator1UV                     = (pcvHotineObliqueMercator * 256) + 1,
    pcvHotineObliqueMercator1XY                     = (pcvHotineObliqueMercator * 256) + 2,
    pcvHotineObliqueMercator2UV                     = (pcvHotineObliqueMercator * 256) + 3,
    pcvHotineObliqueMercator2XY                     = (pcvHotineObliqueMercator * 256) + 4,
    pcvRectifiedSkewOrthomorphic                    = (pcvHotineObliqueMercator * 256) + 5,
    pcvRectifiedSkewOrthomorphicCentered            = (pcvHotineObliqueMercator * 256) + 6,
    pcvRectifiedSkewOrthomorphicOrigin              = (pcvHotineObliqueMercator * 256) + 7,
    pcvTotalUniversalTransverseMercator             = 490,
    pcvTotalTransverseMercatorBF                    = 491,
    pcvObliqueMercatorMinnesota                     = 492,
    };

enum WGS84ConvertCode
    {
    ConvertType_NONE      =   0,
    ConvertType_MOLO      =   1,
    ConvertType_MREG      =   2,
    ConvertType_BURS      =   3,
    ConvertType_NAD27     =   4,
    ConvertType_NAD83     =   5,
    ConvertType_WGS84     =   6,
    ConvertType_WGS72     =   7,
    ConvertType_HPGN      =   8,
    ConvertType_7PARM     =   9,
    ConvertType_AGD66     =   10,
    ConvertType_3PARM     =   11,
    ConvertType_6PARM     =   12,
    ConvertType_4PARM     =   13,
    ConvertType_AGD84     =   14,
    ConvertType_NZGD4     =   15,
    ConvertType_ATS77     =   16,
    ConvertType_GDA94     =   17,
    ConvertType_NZGD2K    =   18,
    ConvertType_CSRS      =   19,
    ConvertType_TOKYO     =   20,
    ConvertType_RGF93     =   21,
    ConvertType_ED50      =   22,
    ConvertType_DHDN      =   23,
    ConvertType_ETRF89    =   24,
    ConvertType_GEOCTR    =   25,
    ConvertType_CHENYX    =   26,
    ConvertType_GENGRID   =   27,      
    ConvertType_MAXVALUE  =   27,       // the maximum allowable value.
    };


//=======================================================================================
//! Interface that provides methods to query Raster Base GCS information.
// @bsiclass                                                       Marc.Bedard    06/2011
struct  IRasterBaseGcs : public IRefCounted
    {
private:

protected:
    virtual IRasterBaseGcsPtr _Clone() const=0;

    virtual GeoCoordinates::BaseGCS* _GetBaseGCS() const=0;


    virtual StatusInt        _InitFromGeoTiffKeys (StatusInt*          warning,           // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
                                                          WString*            warningOrErrorMsg,  // Error message.
                                                          IGeoTiffKeysList*   geoTiffKeys)=0;       // The GeoTiff key list

    virtual StatusInt        _InitFromWellKnownText(StatusInt*        warning,            // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
                                                           WString*          warningOrErrorMsg,  // Error message.
                                                           int32_t           wktFlavor,          // The WKT Flavor.
                                                           WCharCP           wellKnownText)=0;     // The Well Known Text specifying the coordinate system.

    virtual StatusInt        _InitFromEPSGCode(StatusInt*        warning,            // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
                                               WString*          warningOrErrorMsg,  // Error message.
                                               uint32_t          epsgCode)=0;        // The EPSG code.

    virtual StatusInt        _Reproject (double*          outCartesianX,
                                                       double*          outCartesianY,
                                                       double           inCartesianX,
                                                       double           inCartesianY,
                                                       IRasterBaseGcsCR  dstGcs) const=0;

    virtual double           _GetUnitsFromMeters() const=0;

    virtual int              _GetEPSGUnitCode() const = 0;

    virtual int              _GetEPSGCode() const = 0;

    virtual StatusInt        _GetGeoTiffKeys(IGeoTiffKeysList* pList) const = 0;

    virtual StatusInt        _GetWellKnownText(WStringR wellKnownText, int32_t wktFlavor) const=0;   // The WKT Flavor.

    virtual bool             _IsValid() const=0;

    virtual bool             _IsProjected() const=0;

    virtual bool             _IsEquivalent(IRasterBaseGcsCR dstGcs) const=0;

    virtual StatusInt        _GetCartesianFromLatLong(double*  pCartesianPt,double*  pGeoPt) const=0;

    virtual StatusInt        _GetLatLongFromCartesian(double* pGeoPt, double*  pCartesianPt) const=0;

    virtual StatusInt        _SetQuadrant(short quadrant)=0;

    virtual double           _GetVerticalUnits() const = 0;

    virtual StatusInt        _SetVerticalUnits(double pi_RatioToMeter) = 0;


    virtual ImagePP::ProjectionCodeValue    
                                           _GetProjectionCode() const = 0;
     
    virtual ImagePP::WGS84ConvertCode _GetDatumConvertMethod() const = 0;

    virtual double           _GetCentralMeridian() const = 0;

    virtual double           _GetOriginLongitude() const = 0;

    virtual double           _GetOriginLatitude() const = 0;

    virtual int              _GetDanishSys34Region() const = 0;

    virtual double           _GetStandardParallel1() const = 0;

    virtual double           _GetStandardParallel2() const = 0;

    virtual double           _GetMinimumUsefulLongitude() const = 0;

    virtual double           _GetMaximumUsefulLongitude() const = 0;

    virtual double           _GetMinimumUsefulLatitude() const = 0;

    virtual double           _GetMaximumUsefulLatitude() const = 0;

    virtual int              _GetUTMZone() const = 0;

    //__PUBLISH_CLASS_VIRTUAL__
public:

    IMAGEPP_EXPORT IRasterBaseGcsPtr        Clone() const;

    IMAGEPP_EXPORT GeoCoordinates::BaseGCS* GetBaseGCS() const;

    IMAGEPP_EXPORT StatusInt                InitFromGeoTiffKeys (StatusInt*          warning,           // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
                                                         WString*            warningOrErrorMsg,  // Error message.
                                                         IGeoTiffKeysList*   geoTiffKeys);       // The GeoTiff key list

    IMAGEPP_EXPORT StatusInt                InitFromWellKnownText(StatusInt*        warning,            // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
                                                          WString*          warningOrErrorMsg,  // Error message.
                                                          int32_t           wktFlavor,          // The WKT Flavor.
                                                          WCharCP           wellKnownText);     // The Well Known Text specifying the coordinate system.

    IMAGEPP_EXPORT StatusInt                InitFromEPSGCode(StatusInt*        warning,            // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
                                                             WString*          warningOrErrorMsg,  // Error message.
                                                             uint32_t          epsgCode);         // The WKT Flavor.

    IMAGEPP_EXPORT StatusInt                Reproject (double*          outCartesianX,
                                               		double*          outCartesianY,
                                               		double           inCartesianX,
                                               		double           inCartesianY,
                                               		IRasterBaseGcsCR dstGcs) const;

    IMAGEPP_EXPORT double                   GetUnitsFromMeters() const;

    IMAGEPP_EXPORT int                      GetEPSGUnitCode() const;

    IMAGEPP_EXPORT int                      GetEPSGCode() const;

    IMAGEPP_EXPORT StatusInt                GetGeoTiffKeys(IGeoTiffKeysList* pList) const;

    IMAGEPP_EXPORT StatusInt                GetWellKnownText(WStringR wellKnownText, int32_t wktFlavor) const;   // The WKT Flavor.

    IMAGEPP_EXPORT bool                     IsValid() const;

    IMAGEPP_EXPORT bool                     IsProjected() const;

    IMAGEPP_EXPORT bool                     IsEquivalent(IRasterBaseGcsCR dstGcs) const;

    IMAGEPP_EXPORT StatusInt                GetCartesianFromLatLong(double*  pCartesianPt, double*  pGeoPt) const;

    IMAGEPP_EXPORT StatusInt                GetLatLongFromCartesian(double*  pCartesianPt, double*  pGeoPt) const;

    IMAGEPP_EXPORT StatusInt                SetQuadrant(short quadrant);

    IMAGEPP_EXPORT double                   GetVerticalUnits() const;

    IMAGEPP_EXPORT StatusInt                SetVerticalUnits(double pi_RatioToMeter);

    IMAGEPP_EXPORT ImagePP::ProjectionCodeValue      GetProjectionCode() const;
    IMAGEPP_EXPORT ImagePP::WGS84ConvertCode         GetDatumConvertMethod() const;

    IMAGEPP_EXPORT double                   GetCentralMeridian() const;
    IMAGEPP_EXPORT double                   GetOriginLongitude() const;
    IMAGEPP_EXPORT double                   GetOriginLatitude() const;
    IMAGEPP_EXPORT int                      GetDanishSys34Region() const;
    IMAGEPP_EXPORT double                   GetStandardParallel1() const;
    IMAGEPP_EXPORT double                   GetStandardParallel2() const;

    IMAGEPP_EXPORT double                   GetMinimumUsefulLongitude() const;
    IMAGEPP_EXPORT double                   GetMaximumUsefulLongitude() const;
    IMAGEPP_EXPORT double                   GetMinimumUsefulLatitude() const;
    IMAGEPP_EXPORT double                   GetMaximumUsefulLatitude() const;


    IMAGEPP_EXPORT int                      GetUTMZone() const;
    };

/*---------------------------------------------------------------------------------**//**
* This interface provides services required to interpret raster GeoCoordinate.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct          IRasterGeoCoordinateServices
    {
//__PUBLISH_CLASS_VIRTUAL__
public:
    
    //Copied from BaseGeoCoord.h 
    enum WellKnownTextFlavor
        {
        WktFlavorOGC        = 1,    // Open Geospatial Consortium flavor
        WktFlavorGeoTiff    = 2,    // GeoTiff flavor.
        WktFlavorESRI       = 3,    // ESRI flavor.
        WktFlavorOracle     = 4,    // Oracle flavor.
        WktFlavorGeoTools   = 5,    // GeoTools flavor
        WktFlavorEPSG       = 6,    // EPSG flavor
        WktFlavorOracle9    = 7,    // Oracle 9 flavor
        WktFlavorAutodesk   = 8,    // Autodesk and default value since CSMAP was bought

        // Note concerning these last entries and specificaly wktFlavorUnknown
        // The values for these used to be Unknown = 7 AppAlt = 8 and LclAlt = 9
        // As the latter two were yet unsupported they cannot cause any issue but the 
        // Change for the Unknown may lead to a backward compatibily issue for libraries if additional
        // values are added. Make sure these changes occur in between major versions only.
        WktFlavorUnknown    = 9,    // used if the flavor is unknown. InitFromWellKnownText will do its best to figure it out.
        WktFlavorAppAlt     = 10,   // Not yet supported
        WktFlavorLclAlt     = 11,   // Not yet supported
        };

    virtual bool                 _IsAvailable()  const=0;

    virtual WCharCP              _GetServiceName() const=0;

    virtual IRasterBaseGcsPtr    _CreateRasterBaseGcs() const=0;

    virtual IRasterBaseGcsPtr    _CreateRasterBaseGcsFromKeyName(WCharCP keyName) const=0;
    
    virtual IRasterBaseGcsPtr    _CreateRasterBaseGcsFromBaseGcs(GeoCoordinates::BaseGCS* pBaseGcs) const=0;

    virtual bool                 _GetUnitsFromMeters(double& unitFromMeter,  uint32_t EPSGUnitCode) const = 0;

    virtual void                 _GetErrorMessage (WStringR errorStr, int errorCode) const=0;
    };

/*=================================================================================**//**
* The HRFGeoCoordinateProvider provides access to certain GeoCoordination services.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
class HRFGeoCoordinateProvider
    {
public:
    IMAGEPP_EXPORT static IRasterGeoCoordinateServices*   GetServices();

    IMAGEPP_EXPORT static IRasterBaseGcsPtr   CreateRasterGcsFromERSIDS(uint32_t  EPSGCode,
                                                         WStringCR projection, 
                                                         WStringCR datum, 
                                                         WStringCR unit);

    IMAGEPP_EXPORT static IRasterBaseGcsPtr   CreateRasterGcsFromGeoTiffKeys(StatusInt*          warning,              // Warning. Function may succeed, but some warning described in warningOrErrorMsg, passed back.
                                                                            WString*            warningOrErrorMsg,    // Error message.
                                                                            IGeoTiffKeysList&   geoTiffKeys);         // The GeoTiff key list

    IMAGEPP_EXPORT static IRasterBaseGcsPtr   CreateRasterGcsFromWKT(StatusInt*  warning,            // Warning. Function may succeed, but some warning described in warningOrErrorMsg, passed back.
                                                                    WString*    warningOrErrorMsg,  // Error message.
                                                                    int32_t     wktFlavor,          // The WKT Flavor.
                                                                    WCharCP     wellKnownText);     // The Well Known Text specifying the coordinate system.

    IMAGEPP_EXPORT static IRasterBaseGcsPtr   CreateRasterGcsFromEPSGCode(StatusInt*  warning,            // Warning. Function may succeed, but some warning described in warningOrErrorMsg, passed back.
                                                                    WString*    warningOrErrorMsg,  // Error message.
                                                                    uint32_t    epsgCode);     // The Well Known Text specifying the coordinate system.

    IMAGEPP_EXPORT static IRasterBaseGcsPtr   CreateRasterGcsFromKeyName(StatusInt*  warning,            // Warning. Function may succeed, but some warning described in warningOrErrorMsg, passed back.
                                                                        WString*    warningOrErrorMsg,  // Error message.
                                                                        WString&    keyname);

    //Extract units from meters form a GeoTiffKeys List 
    //Return true if found, false otherwise
    IMAGEPP_EXPORT static bool   GetUnitsFromMeters(double& unitFromMeter, const HCPGeoTiffKeys& geoTiffKeys, bool  pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation);

    };  // HRFGeoCoordinateProvider

#define GCSServices HRFGeoCoordinateProvider::GetServices()

END_IMAGEPP_NAMESPACE
