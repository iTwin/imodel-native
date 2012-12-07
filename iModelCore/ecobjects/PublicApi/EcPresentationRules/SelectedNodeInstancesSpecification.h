/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/SelectedNodeInstancesSpecification.h $
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
Specification that creates content ECQueries for selected items.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct SelectedNodeInstancesSpecification : public ContentSpecification
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        WString  m_acceptableSchemaName;
        WString  m_acceptableClassNames;
        bool     m_acceptablePolymorphically;
        bool     m_onlyIfNotHandled;

    protected:
    /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT virtual CharCP               _GetXmlElementName ();
        ECOBJECTS_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode);
        ECOBJECTS_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode);

    public:
        ECOBJECTS_EXPORT SelectedNodeInstancesSpecification () 
            : ContentSpecification (), m_onlyIfNotHandled (false), m_acceptableSchemaName (L""), m_acceptableClassNames (L""), m_acceptablePolymorphically (false)
            {
            }

        ECOBJECTS_EXPORT SelectedNodeInstancesSpecification (int priority, bool onlyIfNotHandled, WStringCR acceptableSchemaName, WStringCR acceptableClassNames, bool acceptablePolymorphically) 
            : ContentSpecification (priority), m_onlyIfNotHandled (onlyIfNotHandled), m_acceptableSchemaName (acceptableSchemaName), m_acceptableClassNames (acceptableClassNames), m_acceptablePolymorphically (acceptablePolymorphically)
            {
            }

        //! Returns true if this rule should be executed only in the case where there are no other higher priority rules for this particular cotext.
        ECOBJECTS_EXPORT bool                         GetOnlyIfNotHandled (void) const             { return m_onlyIfNotHandled; }

        //! Acceptable schema name of ECInstances that will be shown in the content.
        ECOBJECTS_EXPORT WStringCR                    GetAcceptableSchemaName (void) const         { return m_acceptableSchemaName; }

        //! Acceptable class names of ECInstances that will be shown in the content.
        ECOBJECTS_EXPORT WStringCR                    GetAcceptableClassNames (void) const         { return m_acceptableClassNames; }

        //! Identifies whether AcceptableClasses should be check polymorphically.
        ECOBJECTS_EXPORT bool                         GetAcceptablePolymorphically (void) const    { return m_acceptablePolymorphically; }

    };

END_BENTLEY_ECOBJECT_NAMESPACE