/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/ContentSpecification.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

typedef bvector<RelatedPropertiesSpecificationP>   RelatedPropertiesSpecificationList;
typedef bvector<DisplayRelatedItemsSpecificationP> DisplayRelatedItemsSpecificationList;

/*---------------------------------------------------------------------------------**//**
Base class for all ContentSpecifications.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE ContentSpecification : PresentationRuleSpecification
    {
private:
    int                                  m_priority;
    RelatedPropertiesSpecificationList   m_relatedPropertiesSpecification;
    DisplayRelatedItemsSpecificationList m_displayRelatedItemsSpecification;

protected:
    //! Constructor. It is used to initialize the rule with default settings.
    ECOBJECTS_EXPORT ContentSpecification ();

    //! Constructor.
    ECOBJECTS_EXPORT ContentSpecification (int priority);

    //! Returns XmlElement name that is used to read/save this rule information.
    ECOBJECTS_EXPORT virtual CharCP                       _GetXmlElementName () const = 0;

    //! Reads rule information from XmlNode, returns true if it can read it successfully.
    ECOBJECTS_EXPORT virtual bool                         _ReadXml (BeXmlNodeP xmlNode) = 0;

    //! Writes rule information to given XmlNode.
    ECOBJECTS_EXPORT virtual void                         _WriteXml (BeXmlNodeP xmlNode) const = 0;

public:
    //! Destructor.
    ECOBJECTS_EXPORT virtual                              ~ContentSpecification (void);

    //! Reads specification from XML.
    ECOBJECTS_EXPORT bool                                 ReadXml (BeXmlNodeP xmlNode);

    //! Writes specification to xml node.
    ECOBJECTS_EXPORT void                                 WriteXml (BeXmlNodeP parentXmlNode) const;

    //! Priority of the specification, defines the order in which specifications are evaluated and executed.
    ECOBJECTS_EXPORT int                                  GetPriority (void) const;

    //! Sets the priority of the specification.
    ECOBJECTS_EXPORT void                                 SetPriority (int value);

    //! Related properties of acceptable ECInstances, that will be shown next to ECInstance proerties (the same row for example).
    ECOBJECTS_EXPORT RelatedPropertiesSpecificationList const&   GetRelatedProperties(void) const;
    
    //! Related properties of acceptable ECInstances, that will be shown next to ECInstance proerties (the same row for example).
    ECOBJECTS_EXPORT RelatedPropertiesSpecificationList&   GetRelatedPropertiesR(void);

    //! Include related items with current instances when display commands are executed.
    ECOBJECTS_EXPORT DisplayRelatedItemsSpecificationList const& GetDisplayRelatedItems(void) const;    

    //! Include related items with current instances when display commands are executed.
    ECOBJECTS_EXPORT DisplayRelatedItemsSpecificationList& GetDisplayRelatedItems (void);
    };

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
