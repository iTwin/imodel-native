/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

BEGIN_BENTLEY_POINTCLOUD_NAMESPACE

#define INVALID_RAMP_INDEX (UINT_MAX)
#define BLACK_RAMP L"[__BlackRamp__]"

struct RampsDestroyer;

/*---------------------------------------------------------------------------------**//**
* @bsiclass                                                    StephanePoulin  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
struct PointCloudRamps
    {
    friend RampsDestroyer;
    private:
        struct RampInfo
            {
            uint32_t m_index;
            WString m_name;
            };


        typedef bvector<WString>        RampNameVector;
        typedef bmap<WString, RampInfo> RampInfoMap;

        RampInfoMap     m_intensityRampInfos;
        RampInfoMap     m_planeRampInfos;
        RampInfoMap     m_customPlaneRampInfos;

        RampNameVector  m_intensityRampNames;
        RampNameVector  m_planeRampNames;

        WString         m_emptyString;

        static PointCloudRamps* s_ramps;

    private:
        PointCloudRamps();
        ~PointCloudRamps();

    public:
        POINTCLOUD_EXPORT static PointCloudRamps& GetInstance();

        POINTCLOUD_EXPORT uint32_t                GetIntensityRampIndex(WStringCR name) const;
        POINTCLOUD_EXPORT uint32_t                GetPlaneRampIndex(WStringCR name) const;

        POINTCLOUD_EXPORT WStringCR               GetIntensityRampName(uint32_t index) const;
        POINTCLOUD_EXPORT WStringCR               GetPlaneRampName(uint32_t index) const;

        POINTCLOUD_EXPORT RampNameVector const&   GetIntensityRamps () const;
        POINTCLOUD_EXPORT RampNameVector const&   GetPlaneRamps () const;


    };

END_BENTLEY_POINTCLOUD_NAMESPACE
