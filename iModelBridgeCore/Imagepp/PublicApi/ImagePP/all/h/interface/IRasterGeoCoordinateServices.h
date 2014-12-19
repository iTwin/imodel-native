//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: PublicApi/ImagePP/all/h/interface/IRasterGeoCoordinateServices.h $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
//:>+--------------------------------------------------------------------------------------
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <BentleyApi\BentleyApi.h>
#include <Bentley\RefCounted.h>

// Forward declarations
BEGIN_BENTLEY_API_NAMESPACE
namespace GeoCoordinates { class BaseGCS; }
END_BENTLEY_API_NAMESPACE

struct IGeoTiffKeysList;

struct  IRasterBaseGcs;
typedef RefCountedPtr<IRasterBaseGcs>         IRasterBaseGcsPtr;

//=======================================================================================
//! Interface that provides methods to query Raster Base GCS information.
// @bsiclass                                                       Marc.Bedard    06/2011
struct  IRasterBaseGcs : public IRefCounted
    {
private:

protected:
    _HDLLg virtual IRasterBaseGcsPtr _Clone() const=0;

    _HDLLg virtual GeoCoordinates::BaseGCS* _GetBaseGCS() const=0;


    _HDLLg virtual StatusInt        _InitFromGeoTiffKeys (StatusInt*          warning,           // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
                                                          WString*            warningOrErrorMsg,  // Error message.
                                                          IGeoTiffKeysList*   geoTiffKeys)=0;       // The GeoTiff key list

    _HDLLg virtual StatusInt        _InitFromWellKnownText(StatusInt*        warning,            // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
                                                           WString*          warningOrErrorMsg,  // Error message.
                                                           int32_t           wktFlavor,          // The WKT Flavor.
                                                           WCharCP           wellKnownText)=0;     // The Well Known Text specifying the coordinate system.

    _HDLLg virtual StatusInt        _InitFromERSIDS(WStringCR projection,
                                                    WStringCR datum,
                                                    WStringCR unit) = 0;

    _HDLLg virtual StatusInt        _Reproject (double*          outCartesianX,
                                                double*          outCartesianY,
                                                double           inCartesianX,
                                                double           inCartesianY,
                                                IRasterBaseGcs&   dstGcs) const=0;

    _HDLLg virtual double           _GetUnitsFromMeters() const=0;

    _HDLLg virtual int              _GetEPSGUnitCode() const=0;

    _HDLLg virtual StatusInt        _GetGeoTiffKeys(IGeoTiffKeysList* pList) const=0;

    _HDLLg virtual StatusInt        _GetWellKnownText(WStringR wellKnownText, int32_t wktFlavor) const=0;   // The WKT Flavor.

    _HDLLg virtual bool             _IsValid() const=0;

    _HDLLg virtual bool             _IsProjected() const=0;

    _HDLLg virtual bool             _IsEquivalent(IRasterBaseGcs& dstGcs) const=0;

    _HDLLg virtual StatusInt        _GetCartesianFromLatLong(double*  pCartesianPt,double*  pGeoPt) const=0;

    _HDLLg virtual StatusInt        _SetQuadrant(short quadrant)=0;

    _HDLLg virtual double           _GetVerticalUnits() const = 0;

    _HDLLg virtual StatusInt        _SetVerticalUnits(double pi_RatioToMeter) = 0;


    //__PUBLISH_CLASS_VIRTUAL__
public:

    _HDLLg IRasterBaseGcsPtr        Clone() const;

    _HDLLg GeoCoordinates::BaseGCS* GetBaseGCS() const;

    _HDLLg StatusInt                InitFromGeoTiffKeys (StatusInt*          warning,           // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
                                                         WString*            warningOrErrorMsg,  // Error message.
                                                         IGeoTiffKeysList*   geoTiffKeys);       // The GeoTiff key list

    _HDLLg StatusInt                InitFromWellKnownText(StatusInt*        warning,            // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
                                                          WString*          warningOrErrorMsg,  // Error message.
                                                          int32_t           wktFlavor,          // The WKT Flavor.
                                                          WCharCP           wellKnownText);     // The Well Known Text specifying the coordinate system.

    _HDLLg StatusInt                InitFromERSIDS(WStringCR projection,
                                                   WStringCR datum,
                                                   WStringCR unit);

    _HDLLg StatusInt                Reproject (double*          outCartesianX,
                                               double*          outCartesianY,
                                               double           inCartesianX,
                                               double           inCartesianY,
                                               IRasterBaseGcs&   dstGcs) const;

    _HDLLg double                   GetUnitsFromMeters() const;

    _HDLLg int                      GetEPSGUnitCode() const;

    _HDLLg StatusInt                GetGeoTiffKeys(IGeoTiffKeysList* pList) const;

    _HDLLg StatusInt                GetWellKnownText(WStringR wellKnownText, int32_t wktFlavor) const;   // The WKT Flavor.

    _HDLLg bool                     IsValid() const;

    _HDLLg bool                     IsProjected() const;

    _HDLLg bool                     IsEquivalent(IRasterBaseGcs& dstGcs) const;

    _HDLLg StatusInt                GetCartesianFromLatLong(double*  pCartesianPt, double*  pGeoPt) const;

    _HDLLg StatusInt                SetQuadrant(short quadrant);

    _HDLLg double                   GetVerticalUnits() const;

    _HDLLg StatusInt                SetVerticalUnits(double pi_RatioToMeter);


    };

/*---------------------------------------------------------------------------------**//**
* This interface provides services required to interpret raster GeoCoordinate.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct          IRasterGeoCoordinateServices
    {
//__PUBLISH_CLASS_VIRTUAL__
public:
    enum WellKnownTextFlavor
        {
        WktFlavorOGC        = 1,    // Open Geospatial Consortium flavor
        WktFlavorGeoTiff    = 2,    // GeoTiff flavor.
        WktFlavorESRI       = 3,    // ESRI flavor.
        WktFlavorOracle     = 4,    // Oracle flavor.
        WktFlavorGeoTools   = 5,    // GeoTools flavor
        WktFlavorEPSG       = 6,    // EPSG flavor
        WktFlavorUnknown    = 7,    // used if the flavor is unknoWn. InitFromWellKnoWnText Will do its best to figure it out.
        WktFlavorAppAlt     = 8,    // Not yet supported
        WktFlavorLclAlt     = 9,    // Not yet supported
        };

    _HDLLg virtual bool                 _IsAvailable()  const=0;

    _HDLLg virtual WCharCP              _GetServiceName() const=0;

    _HDLLg virtual IRasterBaseGcsPtr    _CreateRasterBaseGcs() const=0;

    _HDLLg virtual IRasterBaseGcsPtr    _CreateRasterBaseGcsFromKeyName(WCharCP keyName) const=0;
    
    _HDLLg virtual IRasterBaseGcsPtr    _CreateRasterBaseGcsFromBaseGcs(RefCountedPtr<GeoCoordinates::BaseGCS>& pBaseGcs) const=0;

    _HDLLg virtual void                 _GetErrorMessage (WStringR errorStr, int errorCode) const=0;
    };

/*=================================================================================**//**
* The HRFGeoCoordinateProvider provides access to certain GeoCoordination services.
* @bsiclass
+===============+===============+===============+===============+===============+======*/
class HRFGeoCoordinateProvider
    {
public:
    _HDLLg static IRasterGeoCoordinateServices*   GetServices();

    static IRasterBaseGcsPtr   CreateRasterGcsFromERSIDS(WStringCR projection, 
                                                         WStringCR datum, 
                                                         WStringCR unit);

    static IRasterBaseGcsPtr   CreateRasterGcsFromGeoTiffKeys(StatusInt*          warning,              // Warning. Function may succeed, but some warning described in warningOrErrorMsg, passed back.
                                                              WString*            warningOrErrorMsg,    // Error message.
                                                              IGeoTiffKeysList&   geoTiffKeys);         // The GeoTiff key list

    static IRasterBaseGcsPtr  CreateRasterGcsFromFromWKT(StatusInt*        warning,            // Warning. Function may succeed, but some warning described in warningOrErrorMsg, passed back.
                                                         WString*          warningOrErrorMsg,  // Error message.
                                                         int32_t           wktFlavor,          // The WKT Flavor.
                                                         WCharCP           wellKnownText);     // The Well Known Text specifying the coordinate system.

    };  // HRFGeoCoordinateProvider

#define GCSServices HRFGeoCoordinateProvider::GetServices()

