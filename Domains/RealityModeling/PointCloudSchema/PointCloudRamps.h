/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudRamps.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------+
|   NOTE: This file was moved from $(SrcRoot)MstnPlatform\PPModules\PointCloud\Component\PointCloudHandler\PointCloudRamps.h
+--------------------------------------------------------------------------------------*/
#pragma once

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

    public:
        static PointCloudRamps& GetInstance();

        PointCloudRamps();
        ~PointCloudRamps();

        uint32_t                GetIntensityRampIndex(WStringCR name) const;
        uint32_t                GetPlaneRampIndex(WStringCR name) const;

        WStringCR               GetIntensityRampName(uint32_t index) const;
        WStringCR               GetPlaneRampName(uint32_t index) const;

        RampNameVector const&   GetIntensityRamps () const;
        RampNameVector const&   GetPlaneRamps () const;


    };

END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE