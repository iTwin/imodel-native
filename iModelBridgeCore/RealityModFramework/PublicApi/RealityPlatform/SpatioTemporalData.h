/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include "RealityPlatformAPI.h"

#include <Bentley/DateTime.h>
#include <BeJsonCpp/BeJsonUtilities.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote               10/2015
//=====================================================================================
struct SpatialEntityDataset : public RefCountedBase
    {
public:
    //! Create from Json.
    REALITYDATAPLATFORM_EXPORT static SpatialEntityDatasetPtr CreateFromJson(Utf8CP data);

    //! Get imagery data.
    REALITYDATAPLATFORM_EXPORT const bvector<SpatialEntityPtr>&   GetImageryGroup() const;
    REALITYDATAPLATFORM_EXPORT bvector<SpatialEntityPtr>&         GetImageryGroupR();

    //! Get terrain data.
    REALITYDATAPLATFORM_EXPORT const bvector<SpatialEntityPtr>&   GetTerrainGroup() const;
    REALITYDATAPLATFORM_EXPORT bvector<SpatialEntityPtr>&         GetTerrainGroupR();

private:
    SpatialEntityDataset();
    ~SpatialEntityDataset();

    bvector<SpatialEntityPtr> m_imageryGroup;
    bvector<SpatialEntityPtr> m_terrainGroup;
    };


END_BENTLEY_REALITYPLATFORM_NAMESPACE