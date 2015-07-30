/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/ContentInstancesOfSpecificClassesSpecification.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/
/** @cond BENTLEY_SDK_Internal */

#include <ECPresentationRules/ContentSpecification.h>
#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
Specification that creates content ECQueries for predefined ECClasses.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentInstancesOfSpecificClassesSpecification : public ContentSpecification
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        Utf8String  m_instanceFilter;
        Utf8String  m_classNames;
        bool     m_arePolymorphic;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP               _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode);

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode);

    /*__PUBLISH_SECTION_START__*/
    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT ContentInstancesOfSpecificClassesSpecification ();

        //! Constructor.
        ECOBJECTS_EXPORT ContentInstancesOfSpecificClassesSpecification (int priority, Utf8StringCR instanceFilter, Utf8StringCR classNames, bool arePolymorphic);

        //! Class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECOBJECTS_EXPORT Utf8StringCR                 GetClassNames (void) const;

        //! This flag identifies whether ECClasses defined in this specification should be marked as polymorphic in the Query.
        ECOBJECTS_EXPORT bool                         GetArePolymorphic (void) const;

        //! InstanceFiler is specially formated string that represents WhereCriteria in 
        //! ECQuery that is used to filter query results.
        ECOBJECTS_EXPORT Utf8StringCR                 GetInstanceFilter (void) const;

    };

END_BENTLEY_ECOBJECT_NAMESPACE

/** @endcond */
