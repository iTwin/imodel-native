/*--------------------------------------------------------------------------------------+
|
|     $Source: PointCloudSchema/PointCloudRamps.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include <PointCloudSchemaInternal.h>

USING_NAMESPACE_BENTLEY_POINTCLOUDSCHEMA
USING_NAMESPACE_BENTLEY_BEPOINTCLOUD

/*---------------------------------------------------------------------------------**//**
* This class delete the static PointCloudRamps pointer in its destructor.
* @bsiclass                                                    StephanePoulin  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
BEGIN_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE
struct RampsDestroyer
    {
    public:
        RampsDestroyer() {}
        ~RampsDestroyer()
            {
            delete PointCloudRamps::s_ramps;
            }
    private:
        RampsDestroyer(RampsDestroyer const&);  //disabled
        RampsDestroyer& operator=(RampsDestroyer const&);   //disabled
    };
END_BENTLEY_POINTCLOUDSCHEMA_NAMESPACE

PointCloudRamps* PointCloudRamps::s_ramps = NULL;
RampsDestroyer s_destroyer;

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudRamps& PointCloudRamps::GetInstance()
    {
    if (NULL == s_ramps)
        s_ramps = new PointCloudRamps();

    return *s_ramps;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudRamps::PointCloudRamps()
    {
    // Add custom ramp
    WString blackRamp(BLACK_RAMP);
    float positions[] = {0.0, 1.0};
    Byte colors[]    = {0, 0, 0, 0, 0, 0};

    PointCloudVortex::AddCustomRamp(blackRamp.GetWCharCP(), 2, positions, colors, false/*interpolateHSL*/);

    int numRamps = PointCloudVortex::NumRamps();
    uint32_t index = 0;
    for (int i = 0; i < numRamps; ++i)
        {
        PtRampType rtype;
        const wchar_t* wname = PointCloudVortex::RampInfo( i, &rtype );

        if (rtype & INTENSITY_RAMP_TYPE)
            {
            RampInfo rampInfo;
            rampInfo.m_index = index++;
            rampInfo.m_name = WString (wname);

            // Don't include the BlackRamp
            if (rampInfo.m_name == blackRamp)
                continue;
            
            m_intensityRampInfos.insert(RampInfoMap::value_type(rampInfo.m_name, rampInfo));
            m_intensityRampNames.push_back(rampInfo.m_name);
            }
        }

    index = 0;
    for (int i = 0; i < numRamps; ++i)
        {
        PtRampType rtype;
        const wchar_t* wname = PointCloudVortex::RampInfo( i, &rtype );

        if (rtype & PLANE_RAMP_TYPE)
            {
            RampInfo rampInfo;
            rampInfo.m_index = index++;
            rampInfo.m_name = WString (wname);
            
            if (rampInfo.m_name == blackRamp)
                {
                m_customPlaneRampInfos.insert(RampInfoMap::value_type(rampInfo.m_name, rampInfo));
                }
            else
                {
                m_planeRampInfos.insert(RampInfoMap::value_type(rampInfo.m_name, rampInfo));
                m_planeRampNames.push_back(rampInfo.m_name);
                }
            }
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudRamps::~PointCloudRamps()
    {

    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t PointCloudRamps::GetIntensityRampIndex(WStringCR name) const
    {
    RampInfoMap::const_iterator itr (m_intensityRampInfos.find(name));
    if (itr != m_intensityRampInfos.end())
        return itr->second.m_index;

    return INVALID_RAMP_INDEX;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t PointCloudRamps::GetPlaneRampIndex(WStringCR name) const
    {
    RampInfoMap::const_iterator itr (m_planeRampInfos.find(name));
    if (itr != m_planeRampInfos.end())
        return itr->second.m_index;

    itr = m_customPlaneRampInfos.find(name);
    if (itr != m_customPlaneRampInfos.end())
        return itr->second.m_index;

    return INVALID_RAMP_INDEX;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR PointCloudRamps::GetIntensityRampName(uint32_t index) const
    {
    for (RampInfoMap::const_iterator itr(m_intensityRampInfos.begin()); itr != m_intensityRampInfos.end(); ++itr)
        {
        if (itr->second.m_index == index)
            return itr->second.m_name;
        }

    return m_emptyString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCR PointCloudRamps::GetPlaneRampName(uint32_t index) const
    {
    for (RampInfoMap::const_iterator itr(m_planeRampInfos.begin()); itr != m_planeRampInfos.end(); ++itr)
        {
        if (itr->second.m_index == index)
            return itr->second.m_name;
        }

    for (RampInfoMap::const_iterator itr(m_customPlaneRampInfos.begin()); itr != m_customPlaneRampInfos.end(); ++itr)
        {
        if (itr->second.m_index == index)
            return itr->second.m_name;
        }
    return m_emptyString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudRamps::RampNameVector const& PointCloudRamps::GetIntensityRamps () const
    {
    return m_intensityRampNames;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    StephanePoulin  03/2010
+---------------+---------------+---------------+---------------+---------------+------*/
PointCloudRamps::RampNameVector const& PointCloudRamps::GetPlaneRamps () const
    {
    return m_planeRampNames;
    }
