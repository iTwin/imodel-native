/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/SpatioTemporalData.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

#include "StdAfx.h"
#include "RealityPlatformAPI.h"

#include <Bentley/DateTime.h>

BEGIN_BENTLEY_REALITYPLATFORM_NAMESPACE

struct SpatioTemporalData;

//=====================================================================================
//! @bsiclass                                   Jean-Francois.Cote               10/2015
//=====================================================================================
struct SpatioTemporalDataset : public RefCountedBase
    {
public:
    //! Create from Json.
    REALITYDATAPLATFORM_EXPORT static SpatioTemporalDatasetPtr CreateFromJson(Utf8CP data);

    //! Get imagery data.
    REALITYDATAPLATFORM_EXPORT const bvector<SpatioTemporalDataPtr>&   GetImageryGroup() const;
    REALITYDATAPLATFORM_EXPORT bvector<SpatioTemporalDataPtr>&         GetImageryGroupR();

    //! Get terrain data.
    REALITYDATAPLATFORM_EXPORT const bvector<SpatioTemporalDataPtr>&   GetTerrainGroup() const;
    REALITYDATAPLATFORM_EXPORT bvector<SpatioTemporalDataPtr>&         GetTerrainGroupR();

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
    REALITYDATAPLATFORM_EXPORT static SpatioTemporalDataPtr Create(Utf8CP identifier, const DateTime& date, const double& resolution, const bvector<GeoPoint2d>& footprint);

    //! Get identifier.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetIdentifier() const;

    //! Get date.
    REALITYDATAPLATFORM_EXPORT const DateTime& GetDate() const;

    //! Get resolution.
    REALITYDATAPLATFORM_EXPORT const double& GetResolution() const;

    //! Get footprint.
    REALITYDATAPLATFORM_EXPORT HFCPtr<HGF2DShape> GetFootprint() const;

private:
    SpatioTemporalData(Utf8CP identifier, const DateTime& date, const double& resolution, const bvector<GeoPoint2d>& footprint);
    ~SpatioTemporalData();

    Utf8String m_identifier;
    DateTime m_date;
    double m_resolution; 
    HFCPtr<HGF2DShape> m_footprint;
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE