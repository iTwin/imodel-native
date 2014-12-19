/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ImagePP/all/h/HFCRasterGeoCoordinateServices.h $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*__BENTLEY_INTERNAL_ONLY__*/
#pragma once

#if defined (ANDROID) || defined (__APPLE__)
//DM-Android
#elif defined (_WIN32)
    #include <GeoCoord\BaseGeoCoord.h>
#endif
#include <ImagePP/all/h/interface/IRasterGeoCoordinateServices.h>

BEGIN_IMAGEPP_NAMESPACE


class HFCRasterGeoCoordinateServices : public IRasterGeoCoordinateServices
    {
    HFC_DECLARE_SINGLETON_DLL(_HDLLu,HFCRasterGeoCoordinateServices);

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
    
#if defined (ANDROID) || defined (__APPLE__)
    //DM-Android
#elif defined (_WIN32)
    virtual IRasterBaseGcsPtr _CreateRasterBaseGcsFromBaseGcs(GeoCoordinates::BaseGCSPtr& pBaseGcs) const override;
#endif

    virtual void              _GetErrorMessage(WStringR errorStr, int errorCode) const override;


    };

END_IMAGEPP_NAMESPACE
