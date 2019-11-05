/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/
#include <Bentley/Nullable.h>
#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>
#include "PropertyEditorSpecification.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Specification for a property and optional overrides.
* @bsiclass                                     Grigas.Petraitis                10/2019
+---------------+---------------+---------------+---------------+---------------+------*/
struct PropertySpecification : HashableBase
{
private:
    Utf8String m_propertyName;
    int m_overridesPriority;
    Utf8String m_labelOverride;
    Nullable<bool> m_isDisplayed;
    PropertyEditorSpecification* m_editorOverride;
    Utf8String m_categoryId;

protected:
    //! Computes specification hash.
    ECPRESENTATION_EXPORT MD5 _ComputeHash(Utf8CP parentHash) const override;

public:
    PropertySpecification(): m_overridesPriority(1000), m_editorOverride(nullptr) {}
    PropertySpecification(Utf8String propertyName, int overridesPriority = 1000, Utf8String labelOverride = "", Utf8String categoryId = "", Nullable<bool> isDisplayed = nullptr, PropertyEditorSpecification* editorOverride = nullptr)
        : m_overridesPriority(overridesPriority), m_propertyName(propertyName), m_labelOverride(labelOverride), m_isDisplayed(isDisplayed), m_editorOverride(editorOverride), m_categoryId(categoryId)
        {}
    ~PropertySpecification() {DELETE_AND_CLEAR(m_editorOverride);}
    
    //! Reads rule information from Json, returns true if it can read it successfully.
    ECPRESENTATION_EXPORT bool ReadJson(JsonValueCR json);
    //! Reads rule information from deprecated PropertyEditorSpecification json, returns true if it can read it successfully.
    ECPRESENTATION_EXPORT bool ReadEditorSpecificationJson(JsonValueCR json);

    //! Reads rule information from Json, returns true if it can read it successfully.
    ECPRESENTATION_EXPORT Json::Value WriteJson() const;
        
    int GetOverridesPriority() const {return m_overridesPriority;}
    Utf8StringCR GetPropertyName() const {return m_propertyName;}
    Utf8StringCR GetLabelOverride() const {return m_labelOverride;}
    Nullable<bool> const& IsDisplayed() const {return m_isDisplayed;}
    PropertyEditorSpecification const* GetEditorOverride() const {return m_editorOverride;}
    Utf8StringCR GetCategoryId() const {return m_categoryId;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
