/*--------------------------------------------------------------------------------------+
|
|     $Source: all/utl/hfc/src/HFCRasterGeoCoordinateServices.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------+
|   Header File Dependencies
+----------------------------------------------------------------------------*/
#include <ImagePP/h/hstdcpp.h>

#if defined (ANDROID) || defined (__APPLE__)
//DM-Android
    #include <ImagePP/all/h/HFCRasterGeoCoordinateServices.h>

#elif defined (_WIN32)
    #include <GeoCoord\BaseGeoCoord.h>

    #include <ImagePP/all/h/HFCRasterGeoCoordinateServices.h>

    BEGIN_IMAGEPP_NAMESPACE
    HFC_IMPLEMENT_SINGLETON(HFCRasterGeoCoordinateServices)
    END_IMAGEPP_NAMESPACE
#endif



USING_NAMESPACE_IMAGEPP

// BEIJING_WIP_RASTER ; EpsgTable probably need be remove
#if 0
const WString ProjectionData_tableName(L"projectiondata.txt");
const WString CoordSysData_tableName(L"coordsysdata.txt");
const WString UnitsData_tableName(L"unitsdata.txt");



/*=================================================================================**//**
* @bsiclass                                     		Marc.Bedard     06/2011
+===============+===============+===============+===============+===============+======*/
class       EpsgTableManager
{
HFC_DECLARE_SINGLETON(EpsgTableManager)
private:
    bool m_isCoordSysTableAllocated;
    bool m_isUnitsTableAllocated;
    bool m_isProjectionTableAllocated;

public:
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Marc.Bedard                     06/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    EpsgTableManager():m_isCoordSysTableAllocated(false),m_isUnitsTableAllocated(false),m_isProjectionTableAllocated(false) {}
    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Marc.Bedard                     06/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    ~EpsgTableManager() 
        {
        Terminate();
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Marc.Bedard                     06/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    void Init()
        {
        WString geotiffcs;
        WString units_l;
        WString projection;

        try
            {
            //Check if an environment variable is defined
            WString     cfgVarExpansion;
            if (BSISUCCESS != ConfigurationManager::GetVariable (cfgVarExpansion, MS_RASTER_EPSGTABLE_PATH))
                return;

            // Found this environment variable
            //Remove trailing backslash if found
            size_t  strlen = cfgVarExpansion.length();
            if (cfgVarExpansion[strlen-1] == DIR_SEPARATOR_CHAR)
                cfgVarExpansion.erase (strlen-1);

            //Append filename to path
            geotiffcs = WString(cfgVarExpansion.c_str()) + WString(L"\\") + CoordSysData_tableName;
            units_l = WString(cfgVarExpansion.c_str()) + WString(L"\\") + UnitsData_tableName;
            projection = WString(cfgVarExpansion.c_str()) + WString(L"\\") + ProjectionData_tableName;

            if (SUCCESS == util_findFile  (NULL,NULL,geotiffcs.c_str(),NULL,NULL,0))
                {
                //Create URL and set table in HRFGeotiff
                HRFGeoTiffCoordSysTable::GetInstance()->SetTableFile(HFCURL::Instanciate(WString(L"file://") + geotiffcs));
                m_isCoordSysTableAllocated=true;
                }

            if (SUCCESS == util_findFile  (NULL,NULL,units_l.c_str(),NULL,NULL,0))
                {
                //Create URL and set table in HRFGeotiff
                HRFGeoTiffUnitsTable::GetInstance()->SetTableFile(HFCURL::Instanciate(WString(L"file://") + units_l));
                m_isUnitsTableAllocated=true;
                }

            if (SUCCESS == util_findFile  (NULL,NULL,projection.c_str(),NULL,NULL,0))
                {
                //Create URL and set table in HRFGeotiff
                HRFGeoTiffProjectionTable::GetInstance()->SetTableFile(HFCURL::Instanciate(WString(L"file://") + projection));
                m_isProjectionTableAllocated=true;
                }

            HRFGeoTiffCoordSysTable::GetInstance()->LockTable();
            HRFGeoTiffUnitsTable::GetInstance()->LockTable();
            HRFGeoTiffProjectionTable::GetInstance()->LockTable();
            }
        catch (...)
            {
            BeAssert(false);
            return;
            }
        }

    /*---------------------------------------------------------------------------------**//**
    * @bsimethod                                    Marc.Bedard                     06/2011
    +---------------+---------------+---------------+---------------+---------------+------*/
    void Terminate()
        {
        try
            {
            if (m_isCoordSysTableAllocated)
                {
                HRFGeoTiffCoordSysTable::GetInstance()->ReleaseTable();
                m_isCoordSysTableAllocated=false;
                }
            if (m_isUnitsTableAllocated)
                {
                HRFGeoTiffUnitsTable::GetInstance()->ReleaseTable();
                m_isUnitsTableAllocated=false;
                }
            if (m_isProjectionTableAllocated)
                {
                HRFGeoTiffProjectionTable::GetInstance()->ReleaseTable();
                m_isProjectionTableAllocated=false;
                }
            }
        catch (...)
            {
            return;
            }
        }
}; // EpsgTableManager

HFC_IMPLEMENT_SINGLETON(EpsgTableManager);
#endif
#if defined (ANDROID) || defined (__APPLE__)
//DM-Android
#elif defined (_WIN32)
/*---------------------------------------------------------------------------------**//**
* Local wrapper over BaseGCS                           Marc.Bedard  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct  HFCRasterBaseGcs : public RefCounted <IRasterBaseGcs>
    {
    private:
        GeoCoordinates::BaseGCSPtr m_pBaseGcs;
        double                              m_VerticalUnitsRatioToMeter;
    protected:
        HFCRasterBaseGcs()
            {
            m_pBaseGcs = GeoCoordinates::BaseGCS::CreateGCS();
            m_VerticalUnitsRatioToMeter = 1.0;
            }
        HFCRasterBaseGcs(GeoCoordinates::BaseGCSPtr& pBaseGcs):m_pBaseGcs(pBaseGcs)
            {
            m_VerticalUnitsRatioToMeter = 1.0;
            }
        HFCRasterBaseGcs(GeoCoordinates::BaseGCSPtr& pBaseGcs,
                         double pi_VerticalUnitsRatioToMeter) : m_pBaseGcs(pBaseGcs)
            {
            m_VerticalUnitsRatioToMeter = pi_VerticalUnitsRatioToMeter;
            }
        HFCRasterBaseGcs(WCharCP keyName)
            {
            m_pBaseGcs = GeoCoordinates::BaseGCS::CreateGCS(keyName);
            m_VerticalUnitsRatioToMeter = 1.0;
            }

        virtual ~HFCRasterBaseGcs()
            {
            }

        virtual GeoCoordinates::BaseGCS* _GetBaseGCS() const override
            {
            return m_pBaseGcs.get();
            }

        virtual IRasterBaseGcsPtr _Clone() const override
            {
            GeoCoordinates::BaseGCSPtr baseGCSPtr(GeoCoordinates::BaseGCS::CreateGCS(*m_pBaseGcs));

            IRasterBaseGcsPtr pRasterBaseGcsCloned = new HFCRasterBaseGcs(baseGCSPtr, m_VerticalUnitsRatioToMeter);

            return pRasterBaseGcsCloned;
            }

        virtual StatusInt        _InitFromGeoTiffKeys (StatusInt *         warning,           // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
                                                        WString*  warningOrErrorMsg,  // Error message.
                                                        IGeoTiffKeysList*  geoTiffKeys) override      // The GeoTiff key list
            {
            return m_pBaseGcs->InitFromGeoTiffKeys(warning, warningOrErrorMsg, geoTiffKeys);
            }

        virtual StatusInt        _InitFromERSIDS(WStringCR pi_rErmProjection,
                                                 WStringCR pi_rErmDatum,
                                                 WStringCR pi_rErmUnits)
            {
                StatusInt            Status = SUCCESS;

#if (0)
                uint32_t ModelType;

                // First part ... we try to set the geokeys by ourselves ...
                if (pi_rErmProjection == L"RAW")
                    ModelType = 0;// no geotiff info
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


                // We try to obtain the EPSG code

                size_t  destinationBuffSize = pi_rErmProjection.GetMaxLocaleCharBytes();
                char*  szProjectionMBS= (char*)_alloca (destinationBuffSize);
                BeStringUtilities::WCharToCurrentLocaleChar(szProjectionMBS,pi_rErmProjection.c_str(),destinationBuffSize);
                destinationBuffSize = pi_rErmDatum.GetMaxLocaleCharBytes();
                char*  szDatumMBS= (char*)_alloca (destinationBuffSize);
                BeStringUtilities::WCharToCurrentLocaleChar(szDatumMBS,pi_rErmDatum.c_str(),destinationBuffSize);
                destinationBuffSize = pi_rErmUnits.GetMaxLocaleCharBytes();
                char*  szUnitsMBS= (char*)_alloca (destinationBuffSize);
                BeStringUtilities::WCharToCurrentLocaleChar(szUnitsMBS,pi_rErmUnits.c_str(),destinationBuffSize);

                INT32 nEPSGCode = TIFFGeo_UserDefined;

            #if defined(IPP_HAVE_ERMAPPER_SUPPORT) 

                if (NCS_SUCCESS != NCSGetEPSGCode(szProjectionMBS, szDatumMBS, &nEPSGCode))
                    {
                    if (NCS_SUCCESS != NCSGetEPSGCode(szDatumMBS, szProjectionMBS, &nEPSGCode))
                        {
                        nEPSGCode = TIFFGeo_UserDefined;
                        }
                    }
            #endif

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
                    HASSERT(0); //Check if it isn't a new unit that should be handled by a special case
                    }

                if (nEPSGCode != TIFFGeo_UserDefined)
                    {

                    this->AddKey(GTModelType, ModelType);

                    if (ModelType == TIFFGeo_ModelTypeGeographic)
                        {
                        this->AddKey(GeographicType, (uint32_t)nEPSGCode);
                        }
                    else
                        {
                        if ((uint32_t)nEPSGCode > USHRT_MAX)
                            {
                            this->AddKey(ProjectedCSTypeLong, (uint32_t)nEPSGCode);
                            }
                        else
                            {
                            this->AddKey(ProjectedCSType, (uint32_t)nEPSGCode);
                            }
                        }

                    if (ProjLinearUnit != 0)
                        {
                        this->AddKey(ProjLinearUnits, ProjLinearUnit);
                        }

                    if (GeogAngularUnit != 0)
                        {
                        this->AddKey(GeogAngularUnits, GeogAngularUnit);
                        }
                    }
                if (!m_pBaseGCS->IsValid())
                    {
                    char* wkt = 0;
                    
                    OGRSpatialReference oSRS;

                    if (GCSServices->_IsAvailable())
                    {
                        // If the projection is user-defined or unknown ... we will try to use GDAL to obtain the information required to use this "user-defined"
                        if( oSRS.importFromERM( szProjectionMBS,
                                                szDatumMBS,
                                                szUnitsMBS ) == OGRERR_NONE )
                            {
                            if (OGRERR_NONE == oSRS.exportToWkt(&wkt))
                                {
                                HASSERT(wkt != 0);

                                char localCsPrefix[] = "LOCAL_CS";
                    
                                if (strncmp(localCsPrefix, wkt, strlen(localCsPrefix)) == 0)
                                    {
                                    delete wkt;

                                    wkt = 0;
                                    }
                                }
                            }
                        }
        
                    if (wkt == 0)
                        {
                        this->AddKey(GTModelType, ModelType);

                        if (ModelType == TIFFGeo_ModelTypeGeographic)
                            {
                            this->AddKey(GeographicType, (uint32_t)nEPSGCode);
                            }
                        else
                            {
                            HASSERT(nEPSGCode == TIFFGeo_UserDefined);
                            this->AddKey(ProjectedCSType, (uint32_t)nEPSGCode);
                            }

                        if (ProjLinearUnit != 0)
                            {
                            this->AddKey(ProjLinearUnits, ProjLinearUnit);
                            }

                        if (GeogAngularUnit != 0)
                            {
                            this->AddKey(GeogAngularUnits, GeogAngularUnit);
                            }
                        }
                    else // Fallback solution only available if baseGeoCoord is loaded
                        {
                        int Warning;
                        IRasterBaseGcsPtr pBaseGeoCoord(GCSServices->_CreateRasterBaseGcs());

                        Status = pBaseGeoCoord->InitFromWellKnownText(&Warning, NULL,0, WString(wkt).c_str());

                        delete wkt;
                    
                        if (SUCCESS != Status)
                            {
                            this->AddKey(GTModelType, ModelType);

                            if (ModelType == TIFFGeo_ModelTypeGeographic)
                                {
                                    this->AddKey(GeographicType, (uint32_t)nEPSGCode);
                                }
                            else
                                {
                                    if ((uint32_t)nEPSGCode > USHRT_MAX)
                                    {   
                                        this->AddKey(ProjectedCSTypeLong, (uint32_t)nEPSGCode);
                                    }
                                else
                                    {
                                        this->AddKey(ProjectedCSType, (uint32_t)nEPSGCode);
                                    }                        
                                }

                            if (ProjLinearUnit != 0)
                                {
                                this->AddKey(ProjLinearUnits, ProjLinearUnit);
                                }        

                            if (GeogAngularUnit != 0)
                                {
                                this->AddKey(GeogAngularUnits, GeogAngularUnit);
                                }  
                            }
                        else
                            FinalizeInit(Status, pBaseGeoCoord, true);
                        }
                    }
#endif
            return Status;

            }

        virtual StatusInt        _InitFromWellKnownText(StatusInt*        warning,            // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
                                                        WString* warningOrErrorMsg,  // Error message.
                                                        int32_t           wktFlavor,          // The WKT Flavor.
                                                        WCharCP           wellKnownText) override     // The Well Known Text specifying the coordinate system.
            {
            return m_pBaseGcs->InitFromWellKnownText(warning, warningOrErrorMsg, (GeoCoordinates::BaseGCS::WktFlavor)wktFlavor, wellKnownText);
            }

        virtual StatusInt        _Reproject (double*            outCartesianX,
                                             double*            outCartesianY,
                                             double             inCartesianX,
                                             double             inCartesianY,
                                             IRasterBaseGcs&    dstGcs) const override
            {
            GeoCoordinates::BaseGCS* pBaseGcsDst = dynamic_cast<HFCRasterBaseGcs&>(dstGcs).m_pBaseGcs.get();

            DPoint3d inCartesian;
            inCartesian.x = inCartesianX;
            inCartesian.y = inCartesianY;
            inCartesian.z = 0.0;

            GeoPoint inLatLong;
            StatusInt status = m_pBaseGcs->LatLongFromCartesian (inLatLong, inCartesian);

            if (status != SUCCESS)
                return status;

            GeoPoint outLatLong;
            status = m_pBaseGcs->LatLongFromLatLong(outLatLong, inLatLong, *pBaseGcsDst);

            if (status != SUCCESS)
                return status;

            DPoint3d outCartesian;
            status = m_pBaseGcs->CartesianFromLatLong(outCartesian, outLatLong);

            if (status != SUCCESS)
                return status;

            *outCartesianX = outCartesian.x;
            *outCartesianY = outCartesian.y;

            return status;
            }

        virtual double           _GetUnitsFromMeters() const override
            {
            return m_pBaseGcs->UnitsFromMeters();
            }

        virtual int              _GetEPSGUnitCode() const override
            {
            return m_pBaseGcs->GetEPSGUnitCode();
            }

        virtual StatusInt        _GetGeoTiffKeys(IGeoTiffKeysList* pList) const override
            {
            return m_pBaseGcs->SetGeoTiffKeys(pList);
            }

        virtual StatusInt        _GetWellKnownText(WStringR wellKnownText, int32_t wktFlavor) const override   // The WKT Flavor.
            {
            return m_pBaseGcs->GetWellKnownText(wellKnownText, (GeoCoordinates::BaseGCS::WktFlavor)wktFlavor);
            }

        virtual bool             _IsValid() const override
            {
            return m_pBaseGcs->IsValid();
            }

        virtual bool             _IsProjected() const override
            {
            return (m_pBaseGcs->GetProjectionCode() != 1);
//            return (m_pBaseGcs->GetProjectionCode() != GeoCoordinates::ProjectionCodeValue::pcvUnity);
            }


        virtual bool             _IsEquivalent(IRasterBaseGcs& dstGcs) const override
            {
            GeoCoordinates::BaseGCS* pBaseGcsDst = dynamic_cast<HFCRasterBaseGcs&>(dstGcs).m_pBaseGcs.get();
            return m_pBaseGcs->IsEquivalent(*pBaseGcsDst);
            }

        virtual StatusInt        _GetCartesianFromLatLong(double*  pCartesianPt,double*  pGeoPt) const override
            {
            DPoint3d  cartesianPt;

            GeoPoint  geoPt  = {pGeoPt[0], pGeoPt[1], pGeoPt[2]};
            StatusInt status = m_pBaseGcs->CartesianFromLatLong (cartesianPt, geoPt);

            pCartesianPt[0] = cartesianPt.x;
            pCartesianPt[1] = cartesianPt.y;
            pCartesianPt[2] = cartesianPt.z;

            return status;
            }

        virtual StatusInt        _SetQuadrant(short quadrant) override
            {
            StatusInt Status;

            Status = m_pBaseGcs->SetQuadrant(quadrant);

            if (Status == SUCCESS)
                Status = m_pBaseGcs->DefinitionComplete();

            return Status;
            }
    
        virtual double            _GetVerticalUnits() const override
            {
                return m_VerticalUnitsRatioToMeter;
            }

        virtual StatusInt         _SetVerticalUnits(double pi_RatioToMeter) override
            {
                HASSERT(pi_RatioToMeter > 0.0);
                m_VerticalUnitsRatioToMeter = pi_RatioToMeter;
                return SUCCESS;
            }


    public:
        static IRasterBaseGcsPtr      Create()
            {
            return new HFCRasterBaseGcs();
            }
        static IRasterBaseGcsPtr      CreateFromKeyName(WCharCP keyName)
            {
            return new HFCRasterBaseGcs(keyName);
            }

        static IRasterBaseGcsPtr      CreateFromBaseGcs(GeoCoordinates::BaseGCSPtr& pBaseGcs)
            {
            return new HFCRasterBaseGcs(pBaseGcs);
            }
   };
#endif // _WIN32

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
HFCRasterGeoCoordinateServices::HFCRasterGeoCoordinateServices()
    {
    //NEEDS_WORK
#if 0
    EpsgTableManager::GetInstance()->Init();
#endif
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
HFCRasterGeoCoordinateServices::~HFCRasterGeoCoordinateServices()
    {
    //NEEDS_WORK
#if 0
    EpsgTableManager::GetInstance()->Terminate();
#endif

    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool HFCRasterGeoCoordinateServices::_IsAvailable() const
    {
    return true;
    }
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP  HFCRasterGeoCoordinateServices::_GetServiceName() const
    {
    return L"basegeocoord.dll";
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterBaseGcsPtr HFCRasterGeoCoordinateServices::_CreateRasterBaseGcs() const
    {
#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
    return NULL;
#elif defined (_WIN32)
    return HFCRasterBaseGcs::Create();
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterBaseGcsPtr HFCRasterGeoCoordinateServices::_CreateRasterBaseGcsFromKeyName(WCharCP keyName) const
    {
#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
    return NULL;
#elif defined (_WIN32)
    return HFCRasterBaseGcs::CreateFromKeyName(keyName);
#endif
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)
IRasterBaseGcsPtr HFCRasterGeoCoordinateServices::_CreateRasterBaseGcsFromBaseGcs(GeoCoordinates::BaseGCSPtr& pBaseGcs) const
    {
    return HFCRasterBaseGcs::CreateFromBaseGcs(pBaseGcs);
    }
#endif

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void HFCRasterGeoCoordinateServices::_GetErrorMessage (WStringR errorStr, int errorCode) const
{
#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)
    GeoCoordinates::BaseGCS::GetErrorMessage (errorStr, errorCode);
#endif
}
