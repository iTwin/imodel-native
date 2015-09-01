/*--------------------------------------------------------------------------------------+
|
|     $Source: all/utl/hfc/src/HFCRasterGeoCoordinateServices.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------+
|   Header File Dependencies
+----------------------------------------------------------------------------*/
#include <ImagePPInternal/hstdcpp.h>

#include <GeoCoord\BaseGeoCoord.h>
#include <ImagePP/all/h/HFCRasterGeoCoordinateServices.h>

HFC_IMPLEMENT_SINGLETON(HFCRasterGeoCoordinateServices)

/*---------------------------------------------------------------------------------**//**
* Local wrapper over BaseGCS                           Marc.Bedard  07/2011
+---------------+---------------+---------------+---------------+---------------+------*/
struct  HFCRasterBaseGcs : public RefCounted <ImagePP::IRasterBaseGcs>
    {
    private:
        GeoCoordinates::BaseGCSPtr  m_pBaseGcs;
        double                      m_VerticalUnitsRatioToMeter;
    protected:
        HFCRasterBaseGcs()
            {
            m_pBaseGcs = GeoCoordinates::BaseGCS::CreateGCS();
            m_VerticalUnitsRatioToMeter = 1.0;
            }
        HFCRasterBaseGcs(GeoCoordinates::BaseGCSP pBaseGcs):m_pBaseGcs(pBaseGcs)
            {
            m_VerticalUnitsRatioToMeter = 1.0;
            }
        HFCRasterBaseGcs(GeoCoordinates::BaseGCSP pBaseGcs,
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
            IRasterBaseGcsPtr pRasterBaseGcsCloned = new HFCRasterBaseGcs();
            if (m_pBaseGcs.IsValid())
                {
                GeoCoordinates::BaseGCSPtr baseGCSPtr(GeoCoordinates::BaseGCS::CreateGCS(*m_pBaseGcs));

                pRasterBaseGcsCloned = new HFCRasterBaseGcs(baseGCSPtr.get(), m_VerticalUnitsRatioToMeter);
                }

            return pRasterBaseGcsCloned;
            }

        virtual StatusInt        _InitFromGeoTiffKeys (StatusInt *         warning,           // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
                                                        WString*  warningOrErrorMsg,  // Error message.
                                                        IGeoTiffKeysList*  geoTiffKeys) override      // The GeoTiff key list
            {
            return m_pBaseGcs->InitFromGeoTiffKeys(warning, warningOrErrorMsg, geoTiffKeys);
            }

        virtual StatusInt        _InitFromWellKnownText(StatusInt*        warning,            // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
                                                        WString* warningOrErrorMsg,  // Error message.
                                                        int32_t           wktFlavor,          // The WKT Flavor.
                                                        WCharCP           wellKnownText) override     // The Well Known Text specifying the coordinate system.
            {
            return m_pBaseGcs->InitFromWellKnownText(warning, warningOrErrorMsg, (GeoCoordinates::BaseGCS::WktFlavor)wktFlavor, wellKnownText);
            }

        virtual StatusInt        _InitFromEPSGCode(StatusInt*        warning,            // Warning. Function returns SUCCESS, but some warning desribed in ERRMSG and warning, passed back.
                                                   WString* warningOrErrorMsg,  // Error message.
                                                   uint32_t          epsgCode) override
            {
            return m_pBaseGcs->InitFromEPSGCode(warning, warningOrErrorMsg, epsgCode);
            }

        virtual StatusInt        _Reproject (double*            outCartesianX,
                                             double*            outCartesianY,
                                             double             inCartesianX,
                                             double             inCartesianY,
                                             IRasterBaseGcsCR    dstGcs) const override
            {
            GeoCoordinates::BaseGCS* pBaseGcsDst = dynamic_cast<const HFCRasterBaseGcs&>(dstGcs).m_pBaseGcs.get();

            DPoint3d inCartesian;
            inCartesian.x = inCartesianX;
            inCartesian.y = inCartesianY;
            inCartesian.z = 0.0;

            StatusInt   stat1;
            StatusInt   stat2;
            StatusInt   stat3;

            GeoPoint inLatLong;
            stat1 = m_pBaseGcs->LatLongFromCartesian (inLatLong, inCartesian);

            GeoPoint outLatLong;
            stat2 = m_pBaseGcs->LatLongFromLatLong(outLatLong, inLatLong, *pBaseGcsDst);

            DPoint3d outCartesian;
            stat3 = pBaseGcsDst->CartesianFromLatLong(outCartesian, outLatLong);

            StatusInt status = SUCCESS;

            // Status returns hardest error found in the three error statuses
            // The hardest error is the first one encountered that is not a warning (value 1 [cs_CNVRT_USFL])
            if (SUCCESS != stat1)
                status = stat1;
            if ((SUCCESS != stat2) && ((SUCCESS == status) || (1 == status))) // If stat2 has error and status not already hard error
                {
                if (0 > stat2) // If stat2 is negative ... this is the one ...
                    status = stat2;
                else  // Both are positive (status may be SUCCESS) we use the highest value which is either warning or error
                    status = (stat2 > status ? stat2 : status);
                }
            if ((SUCCESS != stat3) && ((SUCCESS == status) || (1 == status))) // If stat3 has error and status not already hard error
                {
                if (0 > stat3) // If stat3 is negative ... this is the one ...
                    status = stat3;
                else  // Both are positive (status may be SUCCESS) we use the highest value
                    status = (stat3 > status ? stat3 : status);
                }

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

        virtual int              _GetEPSGCode() const override
            {
            return m_pBaseGcs->GetEPSGCode();
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


        virtual bool             _IsEquivalent(IRasterBaseGcsCR dstGcs) const override
            {
            GeoCoordinates::BaseGCS* pBaseGcsDst = dynamic_cast<const HFCRasterBaseGcs&>(dstGcs).m_pBaseGcs.get();
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

        virtual StatusInt        _GetLatLongFromCartesian(double*  pGeoPt, double*  pCartesianPt) const override
            {
            DPoint3d  cartesianPt = {pCartesianPt[0], pCartesianPt[1], pCartesianPt[2]};

            GeoPoint  geoPt;
            StatusInt status = m_pBaseGcs->LatLongFromCartesian (geoPt, cartesianPt);

            pGeoPt[0] = geoPt.longitude;
            pGeoPt[1] = geoPt.latitude;
            pGeoPt[1] = geoPt.elevation;
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


        virtual ImagePP::ProjectionCodeValue        
                                 _GetProjectionCode() const override
            {
                return (ImagePP::ProjectionCodeValue)m_pBaseGcs->GetProjectionCode();
            }

        virtual ImagePP::WGS84ConvertCode        
                                 _GetDatumConvertMethod() const override
            {
                return (ImagePP::WGS84ConvertCode)m_pBaseGcs->GetDatumConvertMethod();
            }


        virtual double           _GetCentralMeridian() const override
            {
                return m_pBaseGcs->GetCentralMeridian();
            }

        virtual double           _GetOriginLongitude() const override
            {
                return m_pBaseGcs->GetOriginLongitude();
            }

        virtual double           _GetOriginLatitude() const override
            {
                return m_pBaseGcs->GetOriginLatitude();
            }

        virtual int              _GetDanishSys34Region() const override
            {
                return m_pBaseGcs->GetDanishSys34Region();
            }

        virtual double           _GetStandardParallel1() const override
            {
                return m_pBaseGcs->GetStandardParallel1();
            }

        virtual double           _GetStandardParallel2() const override
            {
                return m_pBaseGcs->GetStandardParallel2();
            }

        virtual double           _GetMinimumUsefulLongitude() const override
            {
                return m_pBaseGcs->GetMinimumUsefulLongitude();
            }

        virtual double           _GetMaximumUsefulLongitude() const override
            {
                return m_pBaseGcs->GetMaximumUsefulLongitude();
            }

        virtual double           _GetMinimumUsefulLatitude() const override
            {
                return m_pBaseGcs->GetMinimumUsefulLatitude();
            }

        virtual double           _GetMaximumUsefulLatitude() const override
            {
                return m_pBaseGcs->GetMaximumUsefulLatitude();
            }

        virtual int              _GetUTMZone() const override
            {
                return m_pBaseGcs->GetUTMZone();
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

        static IRasterBaseGcsPtr      CreateFromBaseGcs(GeoCoordinates::BaseGCSP pBaseGcs)
            {
            return new HFCRasterBaseGcs(pBaseGcs);
            }
   };

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
    return HFCRasterBaseGcs::Create();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterBaseGcsPtr HFCRasterGeoCoordinateServices::_CreateRasterBaseGcsFromKeyName(WCharCP keyName) const
    {
    return HFCRasterBaseGcs::CreateFromKeyName(keyName);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
IRasterBaseGcsPtr HFCRasterGeoCoordinateServices::_CreateRasterBaseGcsFromBaseGcs(GeoCoordinates::BaseGCS* pBaseGcs) const
    {
    return HFCRasterBaseGcs::CreateFromBaseGcs(pBaseGcs);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     06/2011
+---------------+---------------+---------------+---------------+---------------+------*/
void HFCRasterGeoCoordinateServices::_GetErrorMessage (WStringR errorStr, int errorCode) const
{
    GeoCoordinates::BaseGCS::GetErrorMessage (errorStr, errorCode);
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Marc.Bedard                     01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool HFCRasterGeoCoordinateServices::_GetUnitsFromMeters(double& unitFromMeter, uint32_t EPSGUnitCode) const 
    {
    bool  isUnitWasFound(false);

    unitFromMeter=1.0;
    T_WStringVector* pAllUnitName = GeoCoordinates::BaseGCS::GetUnitNames();
    for (T_WStringVector::const_iterator itr = pAllUnitName->begin(); itr != pAllUnitName->end(); ++itr)
        {
        GeoCoordinates::UnitCP pUnit(GeoCoordinates::Unit::FindUnit(itr->c_str()));
        if (pUnit->GetEPSGCode() == EPSGUnitCode)
            {
            unitFromMeter = 1.0/pUnit->GetConversionFactor();
            isUnitWasFound = true;
            break;
            }
        }

    return isUnitWasFound;
    }
