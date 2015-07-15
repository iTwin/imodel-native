/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/RealityDataHandler.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"

#include <RealityPlatform/RealityDataHandler.h>

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt RealityDataHandler::GetFootprint(DRange2dP pFootprint) const
    {
    return _GetFootprint(pFootprint);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt RealityDataHandler::GetThumbnail(HBITMAP *pThumbnailBmp) const
    {
    return _GetThumbnail(pThumbnailBmp);
    }
