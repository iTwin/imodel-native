/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityAdmin/RealityDataHandler.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "stdafx.h"

#include "RealityDataHandler.h"

USING_NAMESPACE_BENTLEY_REALITYPLATFORM

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Jean-Francois.Cote              02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
StatusInt RealityData::GetFootprint(bvector<DPoint2d>* pFootprint, DRange2dP pFootprintExtents) const
    {
    return _GetFootprint(pFootprint, pFootprintExtents);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
StatusInt RealityData::GetThumbnail(bvector<Byte>& buffer, uint32_t width, uint32_t height) const
    {
    return _GetThumbnail(buffer, width, height);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 8/2015
//-------------------------------------------------------------------------------------
StatusInt RealityData::GetThumbnail(HBITMAP* pThumbnailBmp, uint32_t width, uint32_t height) const
    {
    return _GetThumbnail(pThumbnailBmp, width, height);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
StatusInt RealityData::SaveFootprint(bvector<DPoint2d>& data, BeFileNameCR outFilename) const
    {
    return _SaveFootprint(data, outFilename);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
StatusInt RealityData::SaveThumbnail(const bvector<Byte>& data, BeFileNameCR outFilename) const
    {
    return _SaveThumbnail(data, outFilename);
    }

//-------------------------------------------------------------------------------------
// @bsimethod                                   Jean-Francois.Cote         		 9/2015
//-------------------------------------------------------------------------------------
StatusInt RealityData::SaveThumbnail(const HBITMAP* pThumbnailBmp, BeFileNameCR outFilename) const
    {
    return _SaveThumbnail(pThumbnailBmp, outFilename);
    }
