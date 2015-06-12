/*--------------------------------------------------------------------------------------+
|
|     $Source: RasterSchema/ImagePPAdmin.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_RASTERSCHEMA_NAMESPACE

struct MyImageppLibHost : ImagePP::ImageppLib::Host
    {
    MyImageppLibHost();

    virtual ImagePP::ImageppLibAdmin&   _SupplyImageppLibAdmin() override;
    virtual void                        _RegisterFileFormat() override;
    };

struct MyImageppLibAdmin : ImagePP::ImageppLibAdmin
    {
    DEFINE_T_SUPER(ImagePP::ImageppLibAdmin)

    virtual ImagePP::IRasterGeoCoordinateServices* _GetIRasterGeoCoordinateServicesImpl() const override;
    virtual ~MyImageppLibAdmin() {}
    };

END_BENTLEY_RASTERSCHEMA_NAMESPACE