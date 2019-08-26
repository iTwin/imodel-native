/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

BEGIN_BENTLEY_RASTER_NAMESPACE

struct MyImageppLibHost : ImagePP::ImageppLib::Host
    {
    MyImageppLibHost();

    virtual ImagePP::ImageppLibAdmin&               _SupplyImageppLibAdmin() override;
    virtual void                                    _RegisterFileFormat() override;
    };

struct MyImageppLibAdmin : ImagePP::ImageppLibAdmin
    {
    DEFINE_T_SUPER(ImagePP::ImageppLibAdmin)

    virtual BentleyStatus                           _GetDefaultTempDirectory(BeFileName& tempFileName) const override;
    virtual BentleyStatus                           _GetLocalCacheDirPath(BeFileName& tempPath, bool checkForChange=false) const override;
    virtual BentleyStatus                           _GetGDalDataPath(BeFileNameR gdalDataPath) const override;
    virtual BentleyStatus                           _GetECWDataPath(BeFileNameR ecwDataPath) const override;
    virtual                                         ~MyImageppLibAdmin() {}
    };

END_BENTLEY_RASTER_NAMESPACE
