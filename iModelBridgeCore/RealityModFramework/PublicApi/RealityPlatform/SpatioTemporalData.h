/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/RealityPlatform/SpatioTemporalData.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

#include "RealityPlatformAPI.h"

#include <Bentley/DateTime.h>
#include <BeJsonCpp/BeJsonUtilities.h>

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
    REALITYDATAPLATFORM_EXPORT static SpatioTemporalDataPtr Create(Utf8StringCR identifier, const DateTime& date, const double& resolution, const bvector<GeoPoint2d>& footprint, Utf8StringCR name = "", Json::Value entityJson = Json::Value());

    //! Get identifier.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetIdentifier() const;

    //! Get name.
    REALITYDATAPLATFORM_EXPORT Utf8StringCR GetName() const;

    //! Get raw json.
    REALITYDATAPLATFORM_EXPORT Json::Value GetRawJson() const;

    //! Get propertyFromJson
    REALITYDATAPLATFORM_EXPORT Json::Value GetValueFromJson(Utf8String propertyName) const;

    //! Get date.
    REALITYDATAPLATFORM_EXPORT const DateTime& GetDate() const;

    //! Get resolution.
    REALITYDATAPLATFORM_EXPORT const double& GetResolution() const;

    //! Get footprint.
    REALITYDATAPLATFORM_EXPORT ImagePP::HFCPtr<ImagePP::HGF2DShape> GetFootprint() const;

private:
    SpatioTemporalData(Utf8StringCR identifier, const DateTime& date, const double& resolution, const bvector<GeoPoint2d>& footprint, Utf8StringCR name, Json::Value entityJson);
    ~SpatioTemporalData();

    Utf8String m_identifier;
    Utf8String m_name;
    Json::Value m_entityWithDetailsView;
    DateTime m_date;
    double m_resolution; 
    ImagePP::HFCPtr<ImagePP::HGF2DShape> m_footprint;
    };

END_BENTLEY_REALITYPLATFORM_NAMESPACE