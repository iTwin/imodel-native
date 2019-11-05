/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>
#include "ContentModifier.h"

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Base class for all ContentSpecifications.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ContentSpecification : PresentationRuleSpecification
{
private:
    int m_priority;
    bool m_showImages;
    ContentModifiersList m_modifiers;
    RelatedInstanceSpecificationList m_relatedInstances;

protected:
    ECPRESENTATION_EXPORT ContentSpecification();
    ECPRESENTATION_EXPORT ContentSpecification(int priority, bool showImages = false);
    ECPRESENTATION_EXPORT ContentSpecification(ContentSpecificationCR);
    ECPRESENTATION_EXPORT ContentSpecification(ContentSpecification&&);

    ECPRESENTATION_EXPORT virtual bool _ReadXml(BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT virtual void _WriteXml(BeXmlNodeP xmlNode) const override;

    ECPRESENTATION_EXPORT virtual bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT virtual void _WriteJson(JsonValueR json) const override;

    //! Clones this content specification.
    virtual ContentSpecification* _Clone() const = 0;

    //! Computes specification hash.
    ECPRESENTATION_EXPORT virtual MD5 _ComputeHash(Utf8CP parentHash) const override;

public:
    ECPRESENTATION_EXPORT static ContentSpecificationP Create(JsonValueCR);

    //! Destructor.
    ECPRESENTATION_EXPORT virtual ~ContentSpecification();

    //! Clones this content specification.
    ContentSpecification* Clone() const {return _Clone();}

    //! Priority of the specification, defines the order in which specifications are evaluated and executed.
    int GetPriority() const {return m_priority;}

    //! Sets the priority of the specification.
    ECPRESENTATION_EXPORT void SetPriority(int value);

    //! Should ImageIds be determined for the content.
    bool GetShowImages() const {return m_showImages;}

    //! Sets whether ImageIds should be determined for the content.
    ECPRESENTATION_EXPORT void SetShowImages(bool value);

    //! A list of related instance specifications.
    ECPRESENTATION_EXPORT void AddRelatedInstance(RelatedInstanceSpecificationR relatedInstance);
    RelatedInstanceSpecificationList const& GetRelatedInstances() const {return m_relatedInstances;}    

    RelatedPropertiesSpecificationList const& GetRelatedProperties() const {return m_modifiers.GetRelatedProperties();}
    void AddRelatedProperty(RelatedPropertiesSpecificationR specification) {m_modifiers.AddRelatedProperty(specification);}
    CalculatedPropertiesSpecificationList const& GetCalculatedProperties() const {return m_modifiers.GetCalculatedProperties();}
    void AddCalculatedProperty(CalculatedPropertiesSpecificationR specification) {m_modifiers.AddCalculatedProperty(specification);}
    PropertyCategorySpecificationsList const& GetPropertyCategories() const {return m_modifiers.GetPropertyCategories();}
    void AddPropertyCategory(PropertyCategorySpecificationR specification) {m_modifiers.AddPropertyCategory(specification);}
    PropertySpecificationsList const& GetPropertyOverrides() const {return m_modifiers.GetPropertyOverrides();}
    void AddPropertyOverride(PropertySpecificationR specification) {m_modifiers.AddPropertyOverride(specification);}
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
