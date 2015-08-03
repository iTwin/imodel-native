/*--------------------------------------------------------------------------------------+
|
|     $Source: Cache/Persistence/Core/CacheSettings.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#include "CacheSettings.h"

#include "CacheSchema.h"

// Version was saved in ECInstance up to version 5.
#define LEGACY_CLASS_CacheSettings                  "DSCacheSchema.Settings"
#define LEGACY_CLASS_CacheSettings_PROPERTY_Version "CacheFormatVersion"

// Version is saved as DB property from version 6.
#define PROPERTY_NAMESPACE  "CacheSettings"
#define PROPERTY_Version    "Version"

USING_NAMESPACE_BENTLEY_WEBSERVICES

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
CacheSettings::CacheSettings(ObservableECDb& db) :
m_dbAdapter(db),
m_version(0)
    {}

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CacheSettings::ReadLegacyVersion()
    {
    ECClassCP settingsClass = m_dbAdapter.GetECClass(LEGACY_CLASS_CacheSettings);
    if (nullptr == settingsClass)
        {
        return ERROR;
        }
    Json::Value settingsJson;
    if (SUCCESS != m_dbAdapter.GetJsonInstance(settingsJson, settingsClass))
        {
        return ERROR;
        }
    m_version = settingsJson[LEGACY_CLASS_CacheSettings_PROPERTY_Version].asInt();
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CacheSettings::Read()
    {
    PropertySpec propertySpec(PROPERTY_Version, PROPERTY_NAMESPACE);
    if (DbResult::BE_SQLITE_ROW == m_dbAdapter.GetECDb().QueryProperty(&m_version, sizeof(m_version), propertySpec))
        {
        return SUCCESS;
        }
    if (SUCCESS != ReadLegacyVersion())
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    02/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus CacheSettings::Save()
    {
    PropertySpec propertySpec(PROPERTY_Version, PROPERTY_NAMESPACE);
    if (DbResult::BE_SQLITE_OK != m_dbAdapter.GetECDb().SaveProperty(propertySpec, &m_version, sizeof(m_version)))
        {
        return ERROR;
        }
    return SUCCESS;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void CacheSettings::SetVersion(uint32_t version)
    {
    m_version = version;
    }

/*--------------------------------------------------------------------------------------+
* @bsimethod                                                    Vincas.Razma    12/2014
+---------------+---------------+---------------+---------------+---------------+------*/
uint32_t CacheSettings::GetVersion() const
    {
    return m_version;
    }