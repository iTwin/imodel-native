/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Specification for a custom property category
* @bsiclass                                     Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertyCategorySpecification : HashableBase
{
private:
    Utf8String m_id;
    Utf8String m_label;
    Utf8String m_description;
    int m_priority;
    bool m_autoExpand;

protected:
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

public:
    PropertyCategorySpecification(): m_priority(1000), m_autoExpand(false) {}
    PropertyCategorySpecification(Utf8String id, Utf8String label, Utf8String description = "", int priority = 1000, bool autoExpand = false)
        : m_id(id), m_label(label), m_description(description), m_priority(priority), m_autoExpand(autoExpand)
        {}

    ECPRESENTATION_EXPORT bool ReadJson(JsonValueCR json);
    ECPRESENTATION_EXPORT Json::Value WriteJson() const;

    Utf8StringCR GetId() const {return m_id;}
    Utf8StringCR GetLabel() const {return m_label;}
    Utf8StringCR GetDescription() const {return m_description;}
    int GetPriority() const {return m_priority;}
    bool ShouldAutoExpand() const {return m_autoExpand;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
