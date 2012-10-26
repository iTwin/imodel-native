/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/ContentInstancesOfSpecificClassesSpecification.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*__PUBLISH_SECTION_START__*/

/*---------------------------------------------------------------------------------**//**
Specification that creates content ECQueries for predefined ECClasses.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ContentInstancesOfSpecificClassesSpecification : public ContentSpecification
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        WString  m_instanceFilter;
        WString  m_schemaName;
        WString  m_classNames;

    protected:
    /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT virtual CharCP               _GetXmlElementName ();
        ECOBJECTS_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode);
        ECOBJECTS_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode);

    public:
        ECOBJECTS_EXPORT ContentInstancesOfSpecificClassesSpecification () 
            : ContentSpecification (), m_instanceFilter (L""), m_schemaName (L""), m_classNames (L"")
            {
            }

        ECOBJECTS_EXPORT ContentInstancesOfSpecificClassesSpecification (int priority, bool onlyIfNotHandled, WStringCR instanceFilter, WStringCR schemaName, WStringCR classNames) 
            : ContentSpecification (priority, onlyIfNotHandled), m_instanceFilter (instanceFilter), m_schemaName (schemaName), m_classNames (classNames)
            {
            }

        //! Schema name of specified classes.
        ECOBJECTS_EXPORT WStringCR                    GetSchemaName (void) const         { return m_schemaName; }

        //! Class names separated by comma that should be used by this specification.
        ECOBJECTS_EXPORT WStringCR                    GetClassNames (void) const         { return m_classNames; }

        //! InstanceFiler is spacially formated string that represents WhereCriteria in 
        //! ECQuery that is used to filter query results.
        ECOBJECTS_EXPORT WStringCR                    GetInstanceFilter (void) const     { return m_instanceFilter; }

    };

END_BENTLEY_ECOBJECT_NAMESPACE