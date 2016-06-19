/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/PointCloudSchema/PointCloudRamps.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once

//__BENTLEY_INTERNAL_ONLY__

BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE

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
        POINTCLOUDSCHEMA_EXPORT static PointCloudRamps& GetInstance();

        POINTCLOUDSCHEMA_EXPORT uint32_t                GetIntensityRampIndex(WStringCR name) const;
        POINTCLOUDSCHEMA_EXPORT uint32_t                GetPlaneRampIndex(WStringCR name) const;

        POINTCLOUDSCHEMA_EXPORT WStringCR               GetIntensityRampName(uint32_t index) const;
        POINTCLOUDSCHEMA_EXPORT WStringCR               GetPlaneRampName(uint32_t index) const;

        POINTCLOUDSCHEMA_EXPORT RampNameVector const&   GetIntensityRamps () const;
        POINTCLOUDSCHEMA_EXPORT RampNameVector const&   GetPlaneRamps () const;


    };

END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE