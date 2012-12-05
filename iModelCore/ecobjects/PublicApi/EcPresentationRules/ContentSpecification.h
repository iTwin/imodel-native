/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/ContentSpecification.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_START__*/

typedef bvector<RelatedPropertiesSpecificationP>   RelatedPropertiesSpecificationList;
typedef bvector<DisplayRelatedItemsSpecificationP> DisplayRelatedItemsSpecificationList;

/*---------------------------------------------------------------------------------**//**
Base class for all ContentSpecifications.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentSpecification
    {
//__PUBLISH_SECTION_END__
private:
    int                                  m_priority;
    RelatedPropertiesSpecificationList   m_relatedPropertiesSpecification;
    DisplayRelatedItemsSpecificationList m_displayRelatedItemsSpecification;

//__PUBLISH_CLASS_VIRTUAL__
//__PUBLISH_SECTION_START__
protected:
    ECOBJECTS_EXPORT ContentSpecification () : m_priority (1000)
        {
        }

    ECOBJECTS_EXPORT ContentSpecification (int priority) : m_priority (priority)
        {
        }

    ECOBJECTS_EXPORT virtual CharCP                       _GetXmlElementName () = 0;
    ECOBJECTS_EXPORT virtual bool                         _ReadXml (BeXmlNodeP xmlNode) = 0;
    ECOBJECTS_EXPORT virtual void                         _WriteXml (BeXmlNodeP xmlNode) = 0;

public:
    //! Destructor.
    ECOBJECTS_EXPORT                                      ~ContentSpecification (void);

    //! Reads specification from XML.
    ECOBJECTS_EXPORT bool                                 ReadXml (BeXmlNodeP xmlNode);

    //! Writes specification to xml node.
    ECOBJECTS_EXPORT void                                 WriteXml (BeXmlNodeP parentXmlNode);

    //! Priority of the specification, defines the order in which specifications are evaluated and executed.
    ECOBJECTS_EXPORT int                                  GetPriority (void) const         { return m_priority; }

    //! Related properties of acceptable ECInstances, that will be shown next to ECInstance proerties (the same row for example).
    ECOBJECTS_EXPORT RelatedPropertiesSpecificationList&   GetRelatedProperties (void)     { return m_relatedPropertiesSpecification; }

    //! Include related items with current instances when display commands are executed.
    ECOBJECTS_EXPORT DisplayRelatedItemsSpecificationList& GetDisplayRelatedItems (void)   { return m_displayRelatedItemsSpecification; }
    };

END_BENTLEY_ECOBJECT_NAMESPACE