/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include    <DgnPlatformInternal.h>
#include    <DgnPlatform/ModelSpatialClassifier.h>

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus   ModelSpatialClassifier::FromJson(BeJsConst value)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelSpatialClassifier::ToJson(BeJsValue value) const
    {
    value.SetEmptyObject();
    m_flags.ToJson(value["flags"]);
    value["expand"] = m_expandDistance;
    value["modelId"] = m_modelId;
    value["name"] = m_name;
    value["isActive"] = m_isActive;
    value["isVolumeClassifier"] = m_isVolumeClassifier;                // Added by PMC 6/2019 to denote planar vs. volume classifiers.

    if (m_categoryId.IsValid())
        value["categoryId"] = m_categoryId;

    if (m_elementId.IsValid())
        value["elementId"] = m_elementId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelSpatialClassifier::Flags::ToJson(BeJsValue value) const
    {
    value.SetEmptyObject();
    value["type"] = m_type;
    value["outside"] = m_outsideDisplay;
    value["inside"] = m_insideDisplay;
    value["selected"] = m_selectedDisplay;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ModelSpatialClassifier::Flags::FromJson(BeJsConst value)
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ModelSpatialClassifiers::FromJson(BeJsConst value)
    {
    if (!value.isArray())
        return ERROR;

    value.ForEachArrayMember([&](BeJsValue::ArrayIndex i, BeJsConst arrayMember) {
        ModelSpatialClassifier  classifier;
        if (SUCCESS != classifier.FromJson(arrayMember))
            return false;

        push_back(classifier);
        return false;
    });
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
void ModelSpatialClassifiers::ToJson(BeJsValue value) const
    {
    value.SetEmptyArray();
    for (auto& classifier : *this)
        classifier.ToJson(value.appendValue());
    }
