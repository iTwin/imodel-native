/*--------------------------------------------------------------------------------------+
|
|  Copyright (c) Bentley Systems, Incorporated. All rights reserved.
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

typedef bvector<RelatedPropertiesSpecificationP>    RelatedPropertiesSpecificationList;
typedef bvector<PropertiesDisplaySpecificationP>    PropertiesDisplaySpecificationList;
typedef bvector<CalculatedPropertiesSpecificationP> CalculatedPropertiesSpecificationList;
typedef bvector<PropertyEditorsSpecificationP>      PropertyEditorsSpecificationList;

/*---------------------------------------------------------------------------------**//**
Base class for all ContentSpecifications.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ContentSpecification : PresentationRuleSpecification
    {
private:
    int                                  m_priority;
    bool                                 m_showImages;
    RelatedPropertiesSpecificationList   m_relatedPropertiesSpecification;
    PropertiesDisplaySpecificationList    m_propertiesDisplaySpecification;
    CalculatedPropertiesSpecificationList m_calculatedPropertiesSpecification;
    PropertyEditorsSpecificationList     m_propertyEditorsSpecification;
    RelatedInstanceSpecificationList     m_relatedInstances;

protected:
    //! Constructor. It is used to initialize the rule with default settings.
    ECPRESENTATION_EXPORT ContentSpecification();

    //! Constructor.
    ECPRESENTATION_EXPORT ContentSpecification(int priority, bool showImages = false);

    //! Copy constructor.
    ECPRESENTATION_EXPORT ContentSpecification(ContentSpecificationCR);
    
    ECPRESENTATION_EXPORT virtual bool _ReadXml (BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT virtual void _WriteXml (BeXmlNodeP xmlNode) const override;
        
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
    ECPRESENTATION_EXPORT int GetPriority (void) const;

    //! Sets the priority of the specification.
    ECPRESENTATION_EXPORT void SetPriority (int value);

    //! Should ImageIds be determined for the content.
    bool GetShowImages() const {return m_showImages;}

    //! Sets whether ImageIds should be determined for the content.
    void SetShowImages(bool value) {m_showImages = value;}

    //! Related properties of acceptable ECInstances, that will be shown next to ECInstance proerties (the same row for example).
    ECPRESENTATION_EXPORT RelatedPropertiesSpecificationList const&   GetRelatedProperties(void) const;
    
    //! Add related property of acceptable ECInstances, that will be shown next to ECInstance proerties (the same row for example).
    ECPRESENTATION_EXPORT void AddRelatedProperty(RelatedPropertiesSpecificationR specification);

    //! Properties that will be displayed or hidden.
    PropertiesDisplaySpecificationList const& GetPropertiesDisplaySpecifications() const {return m_propertiesDisplaySpecification;}
    
    //! Add property that will be displayed or hidden.
    ECPRESENTATION_EXPORT void AddPropertiesDisplaySpecification(PropertiesDisplaySpecificationR specification);

    //! Additional calculated properties included in the content
    CalculatedPropertiesSpecificationList const& GetCalculatedProperties() const {return m_calculatedPropertiesSpecification;}

    //! Add Additional calculated property included in the content
    ECPRESENTATION_EXPORT void AddCalculatedProperty(CalculatedPropertiesSpecificationR specification);

    //! Custom editors for properties
    PropertyEditorsSpecificationList const& GetPropertyEditors() const { return m_propertyEditorsSpecification; }

    //! Add custom editor for properties
    ECPRESENTATION_EXPORT void AddPropertyEditor(PropertyEditorsSpecificationR specification);

    //! A writable list of related instance specifications.
    ECPRESENTATION_EXPORT void AddRelatedInstance(RelatedInstanceSpecificationR relatedInstance);

    //! A const list of related instance specifications.
    RelatedInstanceSpecificationList const& GetRelatedInstances() const {return m_relatedInstances;}
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
