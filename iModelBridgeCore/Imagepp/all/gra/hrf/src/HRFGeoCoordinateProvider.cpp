//:>--------------------------------------------------------------------------------------+
//:>
//:>     $Source: all/gra/hrf/src/HRFGeoCoordinateProvider.cpp $
//:>
//:>  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
//:>
// ----------------------------------------------------------------------------

#include <ImagePPInternal/hstdcpp.h>

#include <ImagePP/all/h/HFCMacros.h>

#include <Imagepp/all/h/interface/IRasterGeoCoordinateServices.h>
#include <Imagepp/all/h/HCPGeoTiffKeys.h>
#include <Imagepp/all/h/HRFGdalUtilities.h>



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

    virtual IRasterBaseGcsPtr _CreateRasterBaseGcsFromBaseGcs(GeoCoordinates::BaseGCS* pBaseGcs) const override {
        return NULL;
    }

    virtual bool _GetUnitsFromMeters(double& unitFromMeter, uint32_t EPSGUnitCode) const  override {
        unitFromMeter=1.0;
        return false;
        }


    virtual void            _GetErrorMessage(WStringR errorStr, int errorCode) const override   {}
    };
HFC_IMPLEMENT_SINGLETON(NoGeoCoordinateServices)

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterGeoCoordinateServices*   HRFGeoCoordinateProvider::GetServices()
    {
    try
        {
        IRasterGeoCoordinateServices* pServices = ImageppLib::GetHost().GetImageppLibAdmin()._GetIRasterGeoCoordinateServicesImpl();
        if (pServices==NULL)
            return NoGeoCoordinateServices::GetInstance();
        return pServices;
        }
    catch (...)
        {
        BeAssert(!"Unexpected error in HRFGeoCoordinateProvider::GetServices.");
        }

    return NoGeoCoordinateServices::GetInstance();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterBaseGcsPtr HRFGeoCoordinateProvider::CreateRasterGcsFromERSIDS(uint32_t  pi_EPSGCode,
                                                                      WStringCR pi_rErmProjection, 
                                                                      WStringCR pi_rErmDatum, 
                                                                      WStringCR pi_rErmUnits)
    {
    IRasterGeoCoordinateServices* pService = HRFGeoCoordinateProvider::GetServices();
    if(pService == NULL)
        return NULL;

    IRasterBaseGcsPtr pBaseGcsPtr = pService->_CreateRasterBaseGcs();
    if(pBaseGcsPtr == NULL)
        return NULL;

    StatusInt            Status = SUCCESS;
    StatusInt            warning;             // Warning. Function may succeed, but some warning described in warningOrErrorMsg, passed back.
    WString              warningOrErrorMsg;

    uint32_t ModelType;
    HFCPtr<HCPGeoTiffKeys> pGeokeys(new HCPGeoTiffKeys());

    // First part ... we try to set the geokeys by ourselves ...
    if (pi_rErmProjection == L"RAW")
        return NULL;// no geotiff info

    else if ((pi_rErmProjection == L"GEODETIC") || (pi_rErmProjection == L"LOCAL"))
        ModelType = TIFFGeo_ModelTypeGeographic;
    else if (pi_rErmProjection == L"GEOCENTRIC")
        {
        ModelType = TIFFGeo_ModelTypeGeocentric;
        HASSERT(ModelType != 3); // not supported for now, see next statement!

        // Should be TIFFGeo_ModelTypeGeocentric but it is not supported by HRF, so treat file as projected
        ModelType = TIFFGeo_ModelTypeProjected; // assure it will be accepted by HRF!
        }
    else
        ModelType = TIFFGeo_ModelTypeProjected; // projected

    // We compute the unit key
    uint32_t ProjLinearUnit = 0;
    uint32_t GeogAngularUnit = 0;

    // Set Unit field
    if ((pi_rErmUnits == L"FEET") || (pi_rErmUnits == L"U.S. SURVEY FOOT"))
        ProjLinearUnit = (uint32_t)TIFFGeo_Linear_Foot_US_Survey; // US Foot
    else if (pi_rErmUnits ==  L"IFEET")
        ProjLinearUnit = (uint32_t)TIFFGeo_Linear_Foot; // Foot
    else if (pi_rErmUnits ==  L"DEGREES")
        GeogAngularUnit = 9102; // Degree
    else if (pi_rErmUnits == L"METERS")
        ProjLinearUnit = (uint32_t)TIFFGeo_Linear_Meter; // Meter
    else if (pi_rErmUnits == L"IMPERIAL YARD")
        ProjLinearUnit = (uint32_t)TIFFGeo_British_Yard_1895;
    else
        {
        HASSERT_DATA(0); //Check if it isn't a new unit that should be handled by a special case
        }

    if (pi_EPSGCode != TIFFGeo_UserDefined)
        {

        pGeokeys->AddKey(GTModelType, ModelType);

        if (ModelType == TIFFGeo_ModelTypeGeographic)
            {
            pGeokeys->AddKey(GeographicType, (uint32_t)pi_EPSGCode);
            }
        else
            {
            if (pi_EPSGCode > USHRT_MAX)
                {
                pGeokeys->AddKey(ProjectedCSTypeLong, (uint32_t)pi_EPSGCode);
                }
            else
                {
                pGeokeys->AddKey(ProjectedCSType, (uint32_t)pi_EPSGCode);
                }
            }

        if (ProjLinearUnit != 0)
            {
            pGeokeys->AddKey(ProjLinearUnits, ProjLinearUnit);
            }

        if (GeogAngularUnit != 0)
            {
            pGeokeys->AddKey(GeogAngularUnits, GeogAngularUnit);
            }
        }


    if(SUCCESS != pBaseGcsPtr->InitFromGeoTiffKeys(&warning, &warningOrErrorMsg, pGeokeys.GetPtr()))
        {
        WString wkt;

        if (GCSServices->_IsAvailable())
            {
            HRFGdalUtilities::ConvertERMToOGCWKT(wkt, pi_rErmProjection, pi_rErmDatum, pi_rErmUnits);
            }

        if (WString::IsNullOrEmpty(wkt.c_str()))
            {
            pGeokeys->AddKey(GTModelType, ModelType);

            if (ModelType == TIFFGeo_ModelTypeGeographic)
                {
                pGeokeys->AddKey(GeographicType, (uint32_t)pi_EPSGCode);
                }
            else
                {
                HASSERT(pi_EPSGCode == TIFFGeo_UserDefined);
                pGeokeys->AddKey(ProjectedCSType, (uint32_t)pi_EPSGCode);
                }

            if (ProjLinearUnit != 0)
                {
                pGeokeys->AddKey(ProjLinearUnits, ProjLinearUnit);
                }

            if (GeogAngularUnit != 0)
                {
                pGeokeys->AddKey(GeogAngularUnits, GeogAngularUnit);
                }
            }
        else // Fallback solution only available if baseGeoCoord is loaded
            {
            int Warning;
            IRasterBaseGcsPtr pBaseGeoCoord(GCSServices->_CreateRasterBaseGcs());

            Status = pBaseGeoCoord->InitFromWellKnownText(&Warning, NULL,0, wkt.c_str());

            if (SUCCESS != Status)
                {
                pGeokeys->AddKey(GTModelType, ModelType);

                if (ModelType == TIFFGeo_ModelTypeGeographic)
                    {
                    pGeokeys->AddKey(GeographicType, (uint32_t)pi_EPSGCode);
                    }
                else
                    {
                    if (pi_EPSGCode > USHRT_MAX)
                        {   
                        pGeokeys->AddKey(ProjectedCSTypeLong, (uint32_t)pi_EPSGCode);
                        }
                    else
                        {
                        pGeokeys->AddKey(ProjectedCSType, (uint32_t)pi_EPSGCode);
                        }                        
                    }

                if (ProjLinearUnit != 0)
                    {
                    pGeokeys->AddKey(ProjLinearUnits, ProjLinearUnit);
                    }        

                if (GeogAngularUnit != 0)
                    {
                    pGeokeys->AddKey(GeogAngularUnits, GeogAngularUnit);
                    }  
                }
            }
        }

    //&&AR when failing is it OK to return NULL? or we have something partially valid that will preserve unknown data or something?
    if(SUCCESS != pBaseGcsPtr->InitFromGeoTiffKeys(&warning, &warningOrErrorMsg, pGeokeys.GetPtr()))
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
IRasterBaseGcsPtr  HRFGeoCoordinateProvider::CreateRasterGcsFromWKT(StatusInt*        warning,            // Warning. Function may succeed, but some warning described in warningOrErrorMsg, passed back.
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
* @bsimethod                                                   Alexandre.Gariepy  07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterBaseGcsPtr  HRFGeoCoordinateProvider::CreateRasterGcsFromEPSGCode(StatusInt*       warning,            // Warning. Function may succeed, but some warning described in warningOrErrorMsg, passed back.
                                                                        WString*          warningOrErrorMsg,  // Error message.
                                                                        uint32_t          epsgCode)
    {
    IRasterGeoCoordinateServices* pService = HRFGeoCoordinateProvider::GetServices();
    if(pService == NULL)
        return NULL;

    IRasterBaseGcsPtr pBaseGcsPtr = pService->_CreateRasterBaseGcs();
    if(pBaseGcsPtr == NULL)
        return NULL;

    //&&AR when failing is it OK to return NULL? or we have something partially valid that will preserve unknown data or something?
    if(SUCCESS != pBaseGcsPtr->InitFromEPSGCode (warning, warningOrErrorMsg, epsgCode))
        return NULL;

    return pBaseGcsPtr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                   Alexandre.Gariepy  07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterBaseGcsPtr  HRFGeoCoordinateProvider::CreateRasterGcsFromKeyName(StatusInt*       warning,            // Warning. Function may succeed, but some warning described in warningOrErrorMsg, passed back.
                                                                        WString*          warningOrErrorMsg,  // Error message.
                                                                        WString&          keyname)
    {
    IRasterGeoCoordinateServices* pService = HRFGeoCoordinateProvider::GetServices();
    if(pService == NULL)
        return NULL;

    IRasterBaseGcsPtr pBaseGcsPtr = pService->_CreateRasterBaseGcsFromKeyName(keyname.c_str());

    return pBaseGcsPtr;
    }

/*---------------------------------------------------------------------------------**//**
* Extract units from meters form a GeoTiffKeys List 
* @bsimethod                                    Marc.Bedard                     01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool  HRFGeoCoordinateProvider::GetUnitsFromMeters
(
double&                unitFromMeter, 
const HCPGeoTiffKeys&  geoTiffKeys,
bool                   pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation
)
    {
    unitFromMeter=1.0;
    bool isUnitWasFound(false);

    IRasterGeoCoordinateServices* pService = HRFGeoCoordinateProvider::GetServices();
    if (pService == NULL)
        return isUnitWasFound;

    HFCPtr<HCPGeoTiffKeys>  pGeoTiffKeys(geoTiffKeys.Clone()); //take a copy, we don;t want to modify original
    IRasterBaseGcsPtr pInputGeocoding = CreateRasterGcsFromGeoTiffKeys(NULL, NULL, *pGeoTiffKeys);

    if ((pInputGeocoding != NULL) && (pInputGeocoding->IsValid()))
        {
        IRasterBaseGcsPtr pGeocoding = pInputGeocoding;

        //TRICKY: By default, GeoGCS will use the ProjLinearUnits geokey to override default unit found in PCS.
        //If user ask otherwise (pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation==false)
        //we will remove ProjLinearUnits from geokeys and re-create geocoding to get unit from the original PCS.
        //We do this only temporarily here because setting can change during session...
        if (!pi_ProjectedCSTypeDefinedWithProjLinearUnitsInterpretation && geoTiffKeys.HasKey(ProjLinearUnits))
            {
            pGeoTiffKeys->EraseKey(ProjLinearUnits);
            pGeocoding = CreateRasterGcsFromGeoTiffKeys(NULL, NULL, *pGeoTiffKeys);
            }

        if (pGeocoding != NULL && pGeocoding->IsValid())
            {
            unitFromMeter = pGeocoding->GetUnitsFromMeters();
            isUnitWasFound = true;
            }
        }
    else if (pGeoTiffKeys->HasKey(ProjLinearUnits))
        {
        //The GCS is not valid but there is a unit defined by the geokeys
        //Interpret this unit.
        //scan all supported unit name and then check for corresponding EPSG code
        uint32_t unitCode;
        pGeoTiffKeys->GetValue(ProjLinearUnits, &unitCode);

        isUnitWasFound = pService->_GetUnitsFromMeters(unitFromMeter,unitCode);

        }
    return isUnitWasFound;
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
StatusInt IRasterBaseGcs::InitFromEPSGCode
(
    StatusInt*        warning,            // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
    WString*          warningOrErrorMsg,  // Error message.
    uint32_t          epsgCode
)
    {
    return _InitFromEPSGCode(warning,warningOrErrorMsg,epsgCode);
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
    IRasterBaseGcs const&   dstGcs
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
int IRasterBaseGcs::GetEPSGCode() const
    {
    return _GetEPSGCode();
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
bool IRasterBaseGcs::IsEquivalent(IRasterBaseGcsCR dstGcs) const
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
StatusInt IRasterBaseGcs::GetLatLongFromCartesian(double* pGeoPt, double* pCartesianPt) const
    {
    return _GetLatLongFromCartesian(pGeoPt, pCartesianPt);
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

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alain.Robert                     07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePP::ProjectionCodeValue IRasterBaseGcs::GetProjectionCode() const
    {
    return _GetProjectionCode();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alain.Robert                     07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePP::WGS84ConvertCode IRasterBaseGcs::GetDatumConvertMethod() const
    {
    return _GetDatumConvertMethod();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alain.Robert                     07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
double IRasterBaseGcs::GetCentralMeridian() const
    {
    return _GetCentralMeridian();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alain.Robert                     07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
double IRasterBaseGcs::GetOriginLongitude() const
    {
    return _GetOriginLongitude();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alain.Robert                     07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
double IRasterBaseGcs::GetOriginLatitude() const
    {
    return _GetOriginLatitude();
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alain.Robert                     07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
int IRasterBaseGcs::GetDanishSys34Region() const
    {
    return _GetDanishSys34Region();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alain.Robert                     07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
double IRasterBaseGcs::GetStandardParallel1() const
    {
    return _GetStandardParallel1();
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alain.Robert                     07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
double IRasterBaseGcs::GetStandardParallel2() const
    {
    return _GetStandardParallel2();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alain.Robert                     08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
double IRasterBaseGcs::GetMinimumUsefulLongitude() const
    {
    return _GetMinimumUsefulLongitude();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alain.Robert                     08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
double IRasterBaseGcs::GetMaximumUsefulLongitude() const
    {
    return _GetMaximumUsefulLongitude();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alain.Robert                     08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
double IRasterBaseGcs::GetMinimumUsefulLatitude() const
    {
    return _GetMinimumUsefulLatitude();
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alain.Robert                     08/2014
+---------------+---------------+---------------+---------------+---------------+------*/
double IRasterBaseGcs::GetMaximumUsefulLatitude() const
    {
    return _GetMaximumUsefulLatitude();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Alain.Robert                     07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
int IRasterBaseGcs::GetUTMZone() const
    {
    return _GetUTMZone();
    }


