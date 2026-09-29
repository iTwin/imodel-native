/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/Rules/PresentationRulesTypes.h>
#include <ECPresentation/Rules/PresentationRule.h>
#include <ECPresentation/Rules/DefaultPropertyCategoryOverride.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct DefaultPropertyCategoryOverride : PresentationRule
{
    DEFINE_T_SUPER(PresentationRule)

private:
    PropertyCategorySpecificationP m_specification;

protected:
    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(BeJsConst json) override;
    ECPRESENTATION_EXPORT void _WriteJson(BeJsValue json) const override;

    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

public:
    DefaultPropertyCategoryOverride() : PresentationRule(), m_specification(nullptr) {}
    ECPRESENTATION_EXPORT DefaultPropertyCategoryOverride(DefaultPropertyCategoryOverride const&);
    ECPRESENTATION_EXPORT DefaultPropertyCategoryOverride(DefaultPropertyCategoryOverride&&);
    DefaultPropertyCategoryOverride(PropertyCategorySpecificationR specification, int priority = 1000, bool onlyIfNotHandled = false)
        : PresentationRule(priority, onlyIfNotHandled), m_specification(&specification)
        {}
    ECPRESENTATION_EXPORT ~DefaultPropertyCategoryOverride();
    PropertyCategorySpecificationR GetSpecification() const {return *m_specification;}
    void SetSpecification(PropertyCategorySpecificationR specification) {DELETE_AND_CLEAR(m_specification); InvalidateHash(); m_specification = &specification;}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
