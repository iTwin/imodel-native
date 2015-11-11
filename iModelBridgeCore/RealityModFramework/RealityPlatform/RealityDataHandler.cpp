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
StatusInt RealityData::GetFootprint(DRange2dP pFootprint) const
    {
    return _GetFootprint(pFootprint);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
StatusInt RealityData::GetThumbnail(bvector<Byte>& buffer, uint32_t width, uint32_t height) const
    {
    return _GetThumbnail(buffer, width, height);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
StatusInt RealityData::SaveFootprint(const DRange2dR data, const BeFileName outFilename) const
    {
    return _SaveFootprint(data, outFilename);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
StatusInt RealityData::SaveThumbnail(const bvector<Byte>& data, const BeFileName outFilename) const
    {
    return _SaveThumbnail(data, outFilename);
    }
