/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealityPlatformAdmins.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

struct MyImageppLibHost : ImagePP::ImageppLib::Host
    {
    MyImageppLibHost();

    virtual ImagePP::ImageppLibAdmin&   _SupplyImageppLibAdmin() override;
    virtual void                        _RegisterFileFormat() override;
    };

struct MyImageppLibAdmin : ImagePP::ImageppLibAdmin
    {
    DEFINE_T_SUPER(ImagePP::ImageppLibAdmin)
    virtual ~MyImageppLibAdmin() {}
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE