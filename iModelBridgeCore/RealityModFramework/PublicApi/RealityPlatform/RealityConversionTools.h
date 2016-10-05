/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/RealityConversionTools.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include <RealityPlatform/RealityPlatformAPI.h>

#include <RealityPlatform/WebResourceData.h>
#include <RealityPackage/RealityDataPackage.h>
#include <RealityPlatform/RealityDataDownload.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

struct RealityConversionTools
    {
public:
    //Fills a bvector with WebResourceData objects, created with data extracted from JSON
    REALITYDATAPLATFORM_EXPORT static StatusInt JsonToWebResourceData(Utf8CP data, bvector<WebResourceDataPtr>* outData);

    REALITYDATAPLATFORM_EXPORT static RealityDataDownload::Link_File_wMirrors_wSisters PackageStringToDownloadOrder(Utf8CP source, WStringP pParseError = NULL);
    REALITYDATAPLATFORM_EXPORT static RealityDataDownload::Link_File_wMirrors_wSisters PackageFileToDownloadOrder(BeFileNameCR filename, WStringP pParseError = NULL);
    REALITYDATAPLATFORM_EXPORT static RealityDataDownload::Link_File_wMirrors_wSisters PackageToDownloadOrder(RealityPackage::RealityDataPackagePtr package);
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE