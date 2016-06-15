/*--------------------------------------------------------------------------------------+
 |
 |     $Source: Licensing/UsageTrackingData.cpp $
 |
 |  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
 |
 +--------------------------------------------------------------------------------------*/
#include "ClientInternal.h"
#include <WebServices/Licensing/UsageTrackingData.h>

USING_NAMESPACE_BENTLEY_DGNCLIENTFX_UTILS

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    George.Rodier   02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
UsageTrackingData::UsageTrackingData() :
m_deviceID(""),
m_imsUserID(""),
m_productID(""),
m_projectID(""),
m_usageDate(""),
m_version("")
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    George.Rodier   02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
UsageTrackingData::UsageTrackingData(Utf8StringCR device, Utf8StringCR userId, Utf8StringCR productId, Utf8StringCR projectId, DateTimeCR usageDate, Utf8StringCR version) :
m_deviceID(device),
m_imsUserID(userId),
m_productID(productId),
m_projectID(projectId),
m_version(version)
    {
    m_usageDate = Utf8PrintfString("%d-%d-%d", usageDate.GetYear(), usageDate.GetMonth(), usageDate.GetDay());
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    George.Rodier   02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value UsageTrackingData::ToJson()
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
    usage["UsageDate"] = m_usageDate.c_str();
    usage["Version"] = m_version.c_str();

    if (!Utf8String::IsNullOrEmpty(m_projectID.c_str()))
        {
        usage["ProjectID"] = m_projectID.c_str();
        }

    return usage;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    George.Rodier   02/2015
+---------------+---------------+---------------+---------------+---------------+------*/
bool UsageTrackingData::IsEmpty()
    {
    return (Utf8String::IsNullOrEmpty(m_deviceID.c_str()) &&
            Utf8String::IsNullOrEmpty(m_imsUserID.c_str()) &&
            Utf8String::IsNullOrEmpty(m_productID.c_str()) &&
            Utf8String::IsNullOrEmpty(m_projectID.c_str()) &&
            Utf8String::IsNullOrEmpty(m_version.c_str()) &&
            Utf8String::IsNullOrEmpty(m_usageDate.c_str()));
    }
