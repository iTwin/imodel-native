/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/Rules/PresentationRuleSet.h>
#include "ContentModifier.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* Base class for all ContentSpecifications.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ContentSpecification : PresentationRuleSpecification
{
    DEFINE_T_SUPER(PresentationRuleSpecification)

private:
    bool m_showImages;
    ContentModifiersList m_modifiers;
    RelatedInstanceSpecificationList m_relatedInstances;
    bool m_onlyIfNotHandled;

protected:
    ECPRESENTATION_EXPORT ContentSpecification();
    ECPRESENTATION_EXPORT ContentSpecification(int priority, bool showImages = false, bool onlyIfNotHandled = false);
    ECPRESENTATION_EXPORT ContentSpecification(ContentSpecificationCR);
    ECPRESENTATION_EXPORT ContentSpecification(ContentSpecification&&);

    ECPRESENTATION_EXPORT virtual bool _ReadXml(BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT virtual void _WriteXml(BeXmlNodeP xmlNode) const override;

    ECPRESENTATION_EXPORT virtual bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT virtual void _WriteJson(JsonValueR json) const override;

    //! Clones this content specification.
    virtual ContentSpecification* _Clone() const = 0;

    //! Computes specification hash.
    ECPRESENTATION_EXPORT virtual MD5 _ComputeHash() const override;
    ECPRESENTATION_EXPORT virtual bool _ShallowEqual(PresentationKeyCR other) const override;

public:
    ECPRESENTATION_EXPORT static ContentSpecificationP Create(JsonValueCR);

    //! Destructor.
    ECPRESENTATION_EXPORT virtual ~ContentSpecification();

    //! Clones this content specification.
    ContentSpecification* Clone() const {return _Clone();}

    //! Should ImageIds be determined for the content.
    bool GetShowImages() const {return m_showImages;}

    //! Sets whether ImageIds should be determined for the content.
    void SetShowImages(bool value) {InvalidateHash(); m_showImages = value;}

    //! Returns true if this rule should be executed only in the case where there are no other higher priority rules for this particular cotext.
    bool GetOnlyIfNotHandled(void) const {return m_onlyIfNotHandled;}

    //! Sets OnlyIfNotHandled value for the specification.
    void SetOnlyIfNotHandled(bool value) {InvalidateHash(); m_onlyIfNotHandled = value;};

    //! A list of related instance specifications.
    ECPRESENTATION_EXPORT void AddRelatedInstance(RelatedInstanceSpecificationR relatedInstance);
    RelatedInstanceSpecificationList const& GetRelatedInstances() const {return m_relatedInstances;}
    ECPRESENTATION_EXPORT void ClearRelatedInstances();

    RelatedPropertiesSpecificationList const& GetRelatedProperties() const {return m_modifiers.GetRelatedProperties();}
    void AddRelatedProperty(RelatedPropertiesSpecificationR specification) {InvalidateHash(); m_modifiers.AddRelatedProperty(specification);}
    void ClearRelatedProperties() {InvalidateHash(); m_modifiers.ClearRelatedProperties();}

    CalculatedPropertiesSpecificationList const& GetCalculatedProperties() const {return m_modifiers.GetCalculatedProperties();}
    void AddCalculatedProperty(CalculatedPropertiesSpecificationR specification) {InvalidateHash(); m_modifiers.AddCalculatedProperty(specification);}
    void ClearCalculatedProperties() {InvalidateHash(); m_modifiers.ClearCalculatedProperties();}

    PropertyCategorySpecificationsList const& GetPropertyCategories() const {return m_modifiers.GetPropertyCategories();}
    void AddPropertyCategory(PropertyCategorySpecificationR specification) {InvalidateHash(); m_modifiers.AddPropertyCategory(specification);}
    void ClearPropertyCategories() {InvalidateHash(); m_modifiers.ClearPropertyCategories();}

    PropertySpecificationsList const& GetPropertyOverrides() const {return m_modifiers.GetPropertyOverrides();}
    void AddPropertyOverride(PropertySpecificationR specification) {InvalidateHash(); m_modifiers.AddPropertyOverride(specification);}
    void ClearPropertyOverrides() {InvalidateHash(); m_modifiers.ClearPropertyOverrides();}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
