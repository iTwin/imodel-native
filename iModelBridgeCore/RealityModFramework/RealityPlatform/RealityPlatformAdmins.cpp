/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealityPlatformAdmins.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"
//Will ignore non local file format (WMS; GeoRaster, etc...)

#include "RealityPlatformAdmins.h"

#include <ImagePP/all/h/HRFFileFormats.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
static void callHostOnAssert(WCharCP _Message, WCharCP _File, unsigned _Line, BeAssertFunctions::AssertType)
    {
    BeAssertFunctions::DefaultAssertionFailureHandler(_Message, _File, _Line);
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
MyImageppLibHost::MyImageppLibHost()
    {
    BeAssertFunctions::SetBeAssertHandler(callHostOnAssert);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
ImagePP::ImageppLibAdmin& MyImageppLibHost::_SupplyImageppLibAdmin()
    {
    return *new MyImageppLibAdmin();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              03/2015
+---------------+---------------+---------------+---------------+---------------+------*/
void MyImageppLibHost::_RegisterFileFormat()
    {
    REGISTER_SUPPORTED_FILEFORMAT
    }

