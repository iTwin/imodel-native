/*--------------------------------------------------------------------------------------+
|
|     $Source: RealityPlatform/SpatioTemporalData.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "StdAfx.h"

#include <Bentley/DateTime.h>
#include <RealityPlatform/RealityPlatformAPI.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

struct SpatioTemporalData;

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote               10/2015
//=====================================================================================
struct SpatioTemporalDataset : public RefCountedBase
    {
public:
    //! Create from Json.
    static SpatioTemporalDatasetPtr CreateFromJson(Utf8CP data);

    //! Get dataset.
    const bvector<SpatioTemporalDataPtr>& GetImageryGroup() const;
    const bvector<SpatioTemporalDataPtr>& GetTerrainGroup() const;

private:
    SpatioTemporalDataset();
    ~SpatioTemporalDataset();

    bvector<SpatioTemporalDataPtr> m_imageryGroup;
    bvector<SpatioTemporalDataPtr> m_terrainGroup;
    };

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote               10/2015
//=====================================================================================
struct SpatioTemporalData : public RefCountedBase
    {
public:
    //! Create from Json.
    static SpatioTemporalDataPtr Create(Utf8CP identifier, const DateTime& date, const double& resolution, const bvector<GeoPoint2d>& footprint);

    //! Get identifier.
    Utf8StringCR GetIdentifier() const;

    //! Get date.
    const DateTime& GetDate() const;

    //! Get resolution.
    const double& GetResolution() const;

    //! Get footprint.
    HFCPtr<HGF2DShape> GetFootprint() const;

private:
    SpatioTemporalData(Utf8CP identifier, const DateTime& date, const double& resolution, const bvector<GeoPoint2d>& footprint);
    ~SpatioTemporalData();

    Utf8String m_identifier;
    DateTime m_date;
    double m_resolution; 
    HFCPtr<HGF2DShape> m_footprint;
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE