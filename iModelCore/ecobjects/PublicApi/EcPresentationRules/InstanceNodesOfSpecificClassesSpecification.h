/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/InstanceNodesOfSpecificClassesSpecification.h $
|
|  $Copyright: (c) 2013 Bentley Systems, Incorporated. All rights reserved. $
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
        bool     m_arePolymorphic;
        WString  m_instanceFilter;
        WString  m_classNames;

    protected:
    /*__PUBLISH_SECTION_START__*/
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP               _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode);

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode);

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT InstanceNodesOfSpecificClassesSpecification ()
            : ChildNodeSpecification (), m_groupByClass (true), m_groupByLabel (true), m_showEmptyGroups (false),
            m_instanceFilter (L""), m_classNames (L""), m_arePolymorphic (false)
            {
            }

        //! Constructor.
        ECOBJECTS_EXPORT InstanceNodesOfSpecificClassesSpecification (int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy, bool hideIfNoChildren,
                                                     bool groupByClass, bool groupByLabel, bool showEmptyGroups,
                                                     WStringCR instanceFilter, WStringCR classNames, bool arePolymorphic)
            : ChildNodeSpecification (priority, alwaysReturnsChildren, hideNodesInHierarchy, hideIfNoChildren), 
            m_groupByClass (groupByClass), m_groupByLabel (groupByLabel), m_showEmptyGroups (showEmptyGroups), 
            m_instanceFilter (instanceFilter), m_classNames (classNames), m_arePolymorphic (arePolymorphic)
            {
            }

        //! Returns true if grouping by class should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByClass (void) const       { return m_groupByClass; }

        //! Returns true if grouping by label should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByLabel (void) const       { return m_groupByLabel; }

        //! Returns true if class grouping nodes should be shown even if there are no 
        //! ECInstances of those classes. Grouping nodes will be generated for all listed classes.
        ECOBJECTS_EXPORT bool                         GetShowEmptyGroups (void) const    { return m_showEmptyGroups; }

        //! Class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECOBJECTS_EXPORT WStringCR                    GetClassNames (void) const         { return m_classNames; }

        //! This flag identifies whether ECClasses defined in this specification should be marked as polymorphic in the Query.
        ECOBJECTS_EXPORT bool                         GetArePolymorphic (void) const     { return m_arePolymorphic; }

        //! InstanceFiler is spacially formated string that represents WhereCriteria in 
        //! ECQuery that is used to filter query results (ChildNodes).
        ECOBJECTS_EXPORT WStringCR                    GetInstanceFilter (void) const     { return m_instanceFilter; }

    };

END_BENTLEY_ECOBJECT_NAMESPACE