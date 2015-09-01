/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ImagePP/all/h/HFCRasterGeoCoordinateServices.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once


#include <GeoCoord\BaseGeoCoord.h>
#include <ImagePP/all/h/interface/IRasterGeoCoordinateServices.h>
#include <ImagePP/all/h/HFCMacros.h>

BEGIN_IMAGEPP_NAMESPACE


class HFCRasterGeoCoordinateServices : public IRasterGeoCoordinateServices
    {
    HFC_DECLARE_SINGLETON_DLL(IMAGEPP_EXPORT, HFCRasterGeoCoordinateServices);

public:
    /*----------------------------------------------------------------------------+
    |   Public Member Functions
    +----------------------------------------------------------------------------*/
    HFCRasterGeoCoordinateServices();
    virtual ~HFCRasterGeoCoordinateServices();

    virtual bool              _IsAvailable() const override;

    virtual WCharCP           _GetServiceName() const override;

    virtual IRasterBaseGcsPtr _CreateRasterBaseGcs() const override;

    virtual IRasterBaseGcsPtr _CreateRasterBaseGcsFromKeyName(WCharCP keyName) const override;
    
    virtual IRasterBaseGcsPtr _CreateRasterBaseGcsFromBaseGcs(GeoCoordinates::BaseGCS* pBaseGcs) const override;


    virtual bool              _GetUnitsFromMeters(double& unitFromMeter, uint32_t EPSGUnitCode) const override;

    virtual void              _GetErrorMessage(WStringR errorStr, int errorCode) const override;


    };

END_IMAGEPP_NAMESPACE
