/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/InstanceNodesOfSpecificClassesSpecification.h $
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
This specification returns instance nodes of defined classes.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceNodesOfSpecificClassesSpecification : public ChildNodeSpecification
    {
    /*__PUBLISH_SECTION_END__*/
    private:
        bool     m_groupByClass;
        bool     m_groupByLabel;
        bool     m_showEmptyGroups;
        WString  m_instanceFilter;
        WString  m_schemaName;
        WString  m_classNames;

    protected:
    /*__PUBLISH_SECTION_START__*/
        ECOBJECTS_EXPORT virtual CharCP               _GetXmlElementName ();
        ECOBJECTS_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode);
        ECOBJECTS_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode);

    public:
        ECOBJECTS_EXPORT InstanceNodesOfSpecificClassesSpecification ()
            : ChildNodeSpecification (), m_groupByClass (true), m_groupByLabel (true), m_showEmptyGroups (false),
            m_instanceFilter (L""), m_schemaName (L""), m_classNames (L"")
            {
            }

        ECOBJECTS_EXPORT InstanceNodesOfSpecificClassesSpecification (int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy, 
                                                     bool groupByClass, bool groupByLabel, bool showEmptyGroups,
                                                     WStringCR instanceFilter, WStringCR schemaName, WStringCR classNames)
            : ChildNodeSpecification (priority, alwaysReturnsChildren, hideNodesInHierarchy), 
            m_groupByClass (groupByClass), m_groupByLabel (groupByLabel), m_showEmptyGroups (showEmptyGroups), 
            m_instanceFilter (instanceFilter), m_schemaName (schemaName), m_classNames (classNames)
            {
            }

        //! Returns true if grouping by class should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByClass (void) const       { return m_groupByClass; }

        //! Returns true if grouping by label should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByLabel (void) const       { return m_groupByLabel; }

        //! Returns true if class grouping nodes should be shown even if there are no 
        //! ECInstances of those classes. Grouping nodes will be generated for all listed classes.
        ECOBJECTS_EXPORT bool                         GetShowEmptyGroups (void) const    { return m_showEmptyGroups; }

        //! Schema name of specified classes.
        ECOBJECTS_EXPORT WStringCR                    GetSchemaName (void) const         { return m_schemaName; }

        //! Class names separated by comma that should be used by this specification.
        ECOBJECTS_EXPORT WStringCR                    GetClassNames (void) const         { return m_classNames; }

        //! InstanceFiler is spacially formated string that represents WhereCriteria in 
        //! ECQuery that is used to filter query results (ChildNodes).
        ECOBJECTS_EXPORT WStringCR                    GetInstanceFilter (void) const     { return m_instanceFilter; }

    };

END_BENTLEY_ECOBJECT_NAMESPACE