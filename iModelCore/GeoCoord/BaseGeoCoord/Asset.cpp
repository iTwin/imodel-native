/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include <GeoCoord/Asset.h>

using namespace BENTLEY_NAMESPACE_NAME::GeoCoordinates;

#define JSON_Id "id"
#define JSON_Description "description"
#define JSON_Version "version"
#define JSON_Packages "packages"
#define JSON_Date "date"
#define JSON_UserId "userId"
#define JSON_UltimateRefId "ultimateRefId"

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Package::ToJson(BeJsValue out) const
{    
    out[JSON_Version] = m_version;
    if (!m_description.empty())
        out[JSON_Description] = m_description;

    out[JSON_Date] = m_date;
    out[JSON_UserId] = m_userId;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Package::FromJson(BeJsConst value)
{
    m_version = value[JSON_Version].asCString();

    auto descriptionValue = value[JSON_Description];
    if(!descriptionValue.isNull())
        m_description = descriptionValue.asCString();

    m_date = value[JSON_Date].asCString();
    m_userId = value[JSON_UserId].asCString();

    return BentleyStatus::SUCCESS;
}

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
AssetPtr Asset::Create(Utf8String jsonData)
    {
    BeJsDocument value(jsonData.c_str());
    if (value.hasParseError())
        return nullptr;

    AssetPtr asset = std::make_shared<Asset>();
    if (SUCCESS == asset->FromJson(value))
        return asset;

    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void Asset::ToJson(BeJsValue out) const
    {
    out[JSON_Id] = m_id;
    if (!m_description.empty())
        out[JSON_Description] = m_description;

    out[JSON_UserId] = m_userId;
    out[JSON_UltimateRefId] = m_ultimateRefId;

    auto packages = out[JSON_Packages];
    packages.SetEmptyArray();

    for (auto index = 0; index < (int)m_packages.size(); ++index)
        m_packages[index].ToJson(packages[index]);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus Asset::FromJson(BeJsConst value)
    {
    BentleyStatus status = BentleyStatus::SUCCESS;

    m_id = value[JSON_Id].asCString();
    if (m_id.empty())
        return ERROR;

    auto descriptionValue = value[JSON_Description];
    m_description = descriptionValue.isNull() ? "" : descriptionValue.asCString();

    m_userId = value[JSON_UserId].asCString();
    m_ultimateRefId = value[JSON_UltimateRefId].asCString();

    m_packages.clear();
    auto packagesValue = value[JSON_Packages];
    if (packagesValue.isArray())
        {
        m_packages.resize(packagesValue.size());
        for (uint32_t index = 0; index < packagesValue.size(); ++index)
            {
            if (SUCCESS != (status = m_packages[index].FromJson(packagesValue[index])))
                return status;
            }
        }

    return status;
    }

