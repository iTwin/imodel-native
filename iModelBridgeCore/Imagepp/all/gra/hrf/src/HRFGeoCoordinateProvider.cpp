//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFGeoCoordinateProvider.cpp $
//:>
//:>  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
//:>
// ----------------------------------------------------------------------------

#include <ImagePP/h/hstdcpp.h>
#include <ImagePP/h/HDllSupport.h>
#include <ImagePP/all/h/HFCMacros.h>

#include <Imagepp/all/h/interface/IRasterGeoCoordinateServices.h>

//:>+--------------------------------------------------------------------------------------
// Class: HRFGeoCoordinateProvider
// ----------------------------------------------------------------------------

class NoGeoCoordinateServices : public IRasterGeoCoordinateServices
    {
    HFC_DECLARE_SINGLETON(NoGeoCoordinateServices);

public:
    /*----------------------------------------------------------------------------+
    |   Public Member Functions
    +----------------------------------------------------------------------------*/
    NoGeoCoordinateServices()           {}
    virtual ~NoGeoCoordinateServices()  {}

    virtual bool              _IsAvailable() const override {
        return false;
    }

    virtual WCharCP           _GetServiceName() const override {
        return L" ";
    }

    virtual IRasterBaseGcsPtr _CreateRasterBaseGcs() const override {
        return NULL;
    }

    virtual IRasterBaseGcsPtr _CreateRasterBaseGcsFromKeyName(WCharCP keyName) const override {
        return NULL;
    }

    virtual IRasterBaseGcsPtr _CreateRasterBaseGcsFromBaseGcs(RefCountedPtr<GeoCoordinates::BaseGCS>& pBaseGcs) const override {
        return NULL;
    }

    virtual void            _GetErrorMessage(WStringR errorStr, int errorCode) const override   {}
    };
HFC_IMPLEMENT_SINGLETON(NoGeoCoordinateServices)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterGeoCoordinateServices*   HRFGeoCoordinateProvider::GetServices()
    {
    IRasterGeoCoordinateServices* pServices = ImagePP::ImageppLib::GetHost().GetImageppLibAdmin()._GetIRasterGeoCoordinateServicesImpl();
    if (pServices==NULL)
        return NoGeoCoordinateServices::GetInstance();

    return pServices;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterBaseGcsPtr HRFGeoCoordinateProvider::CreateRasterGcsFromERSIDS(WStringCR projection, 
                                                                      WStringCR datum, 
                                                                      WStringCR unit)
    {
    IRasterGeoCoordinateServices* pService = HRFGeoCoordinateProvider::GetServices();
    if(pService == NULL)
        return NULL;

    IRasterBaseGcsPtr pBaseGcsPtr = pService->_CreateRasterBaseGcs();
    if(pBaseGcsPtr == NULL)
        return NULL;

    //&&AR when failing is it OK to return NULL? or we have something partially valid that will preserve unknown data or something?
    if (SUCCESS != pBaseGcsPtr->InitFromERSIDS(projection, datum, unit))
        return NULL;

    return pBaseGcsPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterBaseGcsPtr   HRFGeoCoordinateProvider::CreateRasterGcsFromGeoTiffKeys(StatusInt*          warning,              // Warning. Function may succeed, but some warning described in warningOrErrorMsg, passed back.
                                                                             WString*            warningOrErrorMsg,    // Error message.
                                                                             IGeoTiffKeysList&   geoTiffKeys)          // The GeoTiff key list
    {
    IRasterGeoCoordinateServices* pService = HRFGeoCoordinateProvider::GetServices();
    if(pService == NULL)
        return NULL;

    IRasterBaseGcsPtr pBaseGcsPtr = pService->_CreateRasterBaseGcs();
    if(pBaseGcsPtr == NULL)
        return NULL;

    //&&AR when failing is it OK to return NULL? or we have something partially valid that will preserve unknown data or something?
    if(SUCCESS != pBaseGcsPtr->InitFromGeoTiffKeys (warning, warningOrErrorMsg, &geoTiffKeys))
        return NULL;

    return pBaseGcsPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Mathieu.Marchand  09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterBaseGcsPtr  HRFGeoCoordinateProvider::CreateRasterGcsFromFromWKT(StatusInt*        warning,            // Warning. Function may succeed, but some warning described in warningOrErrorMsg, passed back.
                                                                        WString*          warningOrErrorMsg,  // Error message.
                                                                        int32_t           wktFlavor,          // The WKT Flavor.
                                                                        WCharCP           wellKnownText)      // The Well Known Text specifying the coordinate system.
    {
    IRasterGeoCoordinateServices* pService = HRFGeoCoordinateProvider::GetServices();
    if(pService == NULL)
        return NULL;

    IRasterBaseGcsPtr pBaseGcsPtr = pService->_CreateRasterBaseGcs();
    if(pBaseGcsPtr == NULL)
        return NULL;

    //&&AR when failing is it OK to return NULL? or we have something partially valid that will preserve unknown data or something?
    if(SUCCESS != pBaseGcsPtr->InitFromWellKnownText (warning, warningOrErrorMsg, wktFlavor, wellKnownText))
        return NULL;

    return pBaseGcsPtr;
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterBaseGcsPtr IRasterBaseGcs::Clone() const
    {
    return _Clone();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
GeoCoordinates::BaseGCS* IRasterBaseGcs::GetBaseGCS() const
    {
    return _GetBaseGCS();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IRasterBaseGcs::InitFromGeoTiffKeys
(
    StatusInt*         warning,           // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
    WString*           warningOrErrorMsg,  // Error message.
    IGeoTiffKeysList*  geoTiffKeys
)
    {
    return _InitFromGeoTiffKeys(warning,warningOrErrorMsg,geoTiffKeys);
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IRasterBaseGcs::InitFromWellKnownText
(
    StatusInt*        warning,            // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
    WString*          warningOrErrorMsg,  // Error message.
    int32_t           wktFlavor,          // The WKT Flavor.
    WCharCP           wellKnownText
)
    {
    return _InitFromWellKnownText(warning,warningOrErrorMsg,wktFlavor,wellKnownText);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IRasterBaseGcs::InitFromERSIDS
(
WStringCR projection,
WStringCR datum,
WStringCR unit
)
    {
    return _InitFromERSIDS(projection, datum, unit);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IRasterBaseGcs::Reproject
(
    double*          outCartesianX,
    double*          outCartesianY,
    double           inCartesianX,
    double           inCartesianY,
    IRasterBaseGcs&   dstGcs
) const
    {
    return _Reproject(outCartesianX,outCartesianY,inCartesianX,inCartesianY,dstGcs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
double IRasterBaseGcs::GetUnitsFromMeters() const
    {
    return _GetUnitsFromMeters();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
int IRasterBaseGcs::GetEPSGUnitCode() const
    {
    return _GetEPSGUnitCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IRasterBaseGcs::GetGeoTiffKeys(IGeoTiffKeysList* pList) const
    {
    return _GetGeoTiffKeys(pList);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IRasterBaseGcs::GetWellKnownText(WStringR wellKnownText, int32_t wktFlavor) const
    {
    return _GetWellKnownText(wellKnownText,wktFlavor);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool IRasterBaseGcs::IsValid() const
    {
    return _IsValid();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool IRasterBaseGcs::IsProjected() const
    {
    return _IsProjected();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool IRasterBaseGcs::IsEquivalent(IRasterBaseGcs& dstGcs) const
    {
    return _IsEquivalent(dstGcs);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IRasterBaseGcs::GetCartesianFromLatLong(double*  pCartesianPt,double*  pGeoPt) const
    {
    return _GetCartesianFromLatLong(pCartesianPt,pGeoPt);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IRasterBaseGcs::SetQuadrant(short quadrant)
    {
    return _SetQuadrant(quadrant);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
double IRasterBaseGcs::GetVerticalUnits() const
    {
    return _GetVerticalUnits();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt IRasterBaseGcs::SetVerticalUnits(double pi_RatioToMeters)
    {
    return _SetVerticalUnits(pi_RatioToMeters);
    }


