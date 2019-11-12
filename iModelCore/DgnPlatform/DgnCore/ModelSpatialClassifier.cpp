/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/ModelSpatialClassifier.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ModelSpatialClassifier::FromJson(Json::Value const& value)
    {
    if (value.isNull() || SUCCESS != m_flags.FromJson(value["flags"]) || !value.isMember("modelId"))
        return ERROR;

    m_modelId   = DgnModelId(value["modelId"].asUInt64());
    m_expandDistance = value["expand"].asDouble();
    m_name = value["name"].asString();
    m_isActive = value["isActive"].asBool();

    if (value.isMember("categoryId"))
        m_categoryId = DgnCategoryId(value["categoryId"].asUInt64());

    if (value.isMember("elementId"))
        m_elementId = DgnElementId(value["elementId"].asUInt64());


    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value ModelSpatialClassifier::ToJson() const
    {
    Json::Value     value;

    value["flags"] = m_flags.ToJson();
    value["expand"] = m_expandDistance;
    value["modelId"] = m_modelId.ToHexStr();
    value["name"] = m_name;
    value["isActive"] = m_isActive;
    value["isVolumeClassifier"] = m_isVolumeClassifier;                // Added by PMC 6/2019 to denote planar vs. volume classifiers.

    if (m_categoryId.IsValid())
        value["categoryId"] = m_categoryId.ToHexStr();

    if (m_elementId.IsValid())
        value["elementId"] = m_elementId.ToHexStr();

    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value ModelSpatialClassifier::Flags::ToJson() const
    {
    Json::Value     value;

    value["type"] = m_type;
    value["outside"] = m_outsideDisplay;
    value["inside"] = m_insideDisplay;
    value["selected"] = m_selectedDisplay;

    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ModelSpatialClassifier::Flags::FromJson(Json::Value const& value)
    {
    if (value.isNull() || !value.isMember("type"))
        return ERROR;

    m_type = value["type"].asUInt();
    m_outsideDisplay = value["outside"].asUInt();
    m_insideDisplay = value["inside"].asUInt();
    m_selectedDisplay = value["selected"].asUInt();

    return SUCCESS;
    }



/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ModelSpatialClassifiers::FromJson(Json::Value const& value)
    {
    if (!value.isArray())
        return ERROR;

    for (auto& arrayMember : value)
        {
        ModelSpatialClassifier  classifier;

        if (SUCCESS != classifier.FromJson(arrayMember))
            return ERROR;

        push_back(classifier);
        }
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Ray.Bentley     07/2017
+---------------+---------------+---------------+---------------+---------------+------*/
Json::Value ModelSpatialClassifiers::ToJson() const
    {
    Json::Value     value  = Json::arrayValue;

    for (auto& classifier : *this)
        value.append(classifier.ToJson());

    return value;
    }
