/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Licensing/FeatureTrackingData.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Licensing/FeatureTrackingData.h>

USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Wil.Maier       04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
FeatureTrackingData::FeatureTrackingData() :
m_deviceID(""),
m_imsUserID(""),
m_productID(""),
m_version(""),
m_projectID(""),
m_featureID(""),
m_usageStartDate(""),
m_usageEndDate("")
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Wil.Maier       04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
FeatureTrackingData::FeatureTrackingData(Utf8StringCR device, Utf8StringCR userId, Utf8StringCR productId, Utf8StringCR version, Utf8StringCR projectId, Utf8StringCR featureId, DateTimeCR usageStartDate, DateTimeCR usageEndDate) :
m_deviceID(device),
m_imsUserID(userId),
m_productID(productId),
m_version(version),
m_projectID(projectId),
m_featureID(featureId),
m_usageStartDate(""),
m_usageEndDate("")
    {
    if (usageStartDate.IsValid())
        {
        m_usageStartDate = Utf8PrintfString("%d-%d-%d", usageStartDate.GetYear(), usageStartDate.GetMonth(), usageStartDate.GetDay());
        if (usageEndDate.IsValid() && usageStartDate != usageEndDate)
            m_usageEndDate = Utf8PrintfString("%d-%d-%d", usageEndDate.GetYear(), usageEndDate.GetMonth(), usageEndDate.GetDay());
        }
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Wil.Maier       04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value FeatureTrackingData::ToJson()
    {
    Json::Value usage = Json::objectValue;

    if (IsEmpty())
        {
        return Json::nullValue;
        }

    usage["DeviceID"] = m_deviceID.c_str();
    m_imsUserID.ToLower();
    usage["ImsUserID"] = m_imsUserID.c_str();
    usage["ProductID"] = m_productID.c_str();
    if (!Utf8String::IsNullOrEmpty(m_usageStartDate.c_str()))
        {
        if (Utf8String::IsNullOrEmpty(m_usageEndDate.c_str()))
            usage["TimeStamp"] = m_usageStartDate.c_str();
        else
            {
            usage["Started"] = m_usageStartDate.c_str();
            usage["Ended"] = m_usageEndDate.c_str();
            }
        }
    usage["Version"] = m_version.c_str();
    usage["FeatureName"] = m_featureID.c_str();

    if (!Utf8String::IsNullOrEmpty(m_projectID.c_str()))
        {
        usage["ProjectID"] = m_projectID.c_str();
        }

    return usage;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Wil.Maier       04/2016
+---------------+---------------+---------------+---------------+---------------+------*/
bool FeatureTrackingData::IsEmpty()
    {
    return (Utf8String::IsNullOrEmpty(m_deviceID.c_str()) &&
            Utf8String::IsNullOrEmpty(m_imsUserID.c_str()) &&
            Utf8String::IsNullOrEmpty(m_productID.c_str()) &&
            Utf8String::IsNullOrEmpty(m_version.c_str()) &&
            Utf8String::IsNullOrEmpty(m_projectID.c_str()) &&
            Utf8String::IsNullOrEmpty(m_featureID.c_str()) &&
            Utf8String::IsNullOrEmpty(m_usageStartDate.c_str()) &&
            Utf8String::IsNullOrEmpty(m_usageEndDate.c_str()));
    }
