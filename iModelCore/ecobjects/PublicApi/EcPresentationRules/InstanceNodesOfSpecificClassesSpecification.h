/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/InstanceNodesOfSpecificClassesSpecification.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <ECPresentationRules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

/*---------------------------------------------------------------------------------**//**
This specification returns instance nodes of defined classes.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct InstanceNodesOfSpecificClassesSpecification : public ChildNodeSpecification
    {
    private:
        bool     m_groupByClass;
        bool     m_groupByLabel;
        bool     m_showEmptyGroups;
        bool     m_arePolymorphic;
        WString  m_instanceFilter;
        WString  m_classNames;

    protected:
        //! Returns XmlElement name that is used to read/save this rule information.
        ECOBJECTS_EXPORT virtual CharCP               _GetXmlElementName ();

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECOBJECTS_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode);

        //! Writes rule information to given XmlNode.
        ECOBJECTS_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode);

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECOBJECTS_EXPORT InstanceNodesOfSpecificClassesSpecification ();

        //! Constructor.
        ECOBJECTS_EXPORT InstanceNodesOfSpecificClassesSpecification (int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy,
                                                                      bool hideIfNoChildren, bool groupByClass, bool groupByLabel, bool showEmptyGroups,
                                                                      WStringCR instanceFilter, WStringCR classNames, bool arePolymorphic);

        //! Returns true if grouping by class should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByClass (void) const;

        //! Set GroupByClass value. Can be boolean.
        ECOBJECTS_EXPORT void                         SetGroupByClass (bool value);

        //! Returns true if grouping by label should be applied.
        ECOBJECTS_EXPORT bool                         GetGroupByLabel (void) const;

        //! Set GroupByLabel value. Can be boolean.
        ECOBJECTS_EXPORT void                         SetGroupByLabel (bool value);

        //! Returns true if class grouping nodes should be shown even if there are no 
        //! ECInstances of those classes. Grouping nodes will be generated for all listed classes.
        ECOBJECTS_EXPORT bool                         GetShowEmptyGroups (void) const;

        //! Set ShowEmptyGroups value. Can be boolean.
        ECOBJECTS_EXPORT void                         SetShowEmptyGroups (bool value);

        //! Class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECOBJECTS_EXPORT WStringCR                    GetClassNames (void) const;

        //! Set class names. Can be string.
        ECOBJECTS_EXPORT void                         SetClassNames (WString value);

        //! This flag identifies whether ECClasses defined in this specification should be marked as polymorphic in the Query.
        ECOBJECTS_EXPORT bool                         GetArePolymorphic (void) const;

        //! Set ArePolymorphic value. Can be boolean.
        ECOBJECTS_EXPORT void                         SetArePolymorphic (bool value);

        //! InstanceFiler is spacially formated string that represents WhereCriteria in 
        //! ECQuery that is used to filter query results (ChildNodes).
        ECOBJECTS_EXPORT WStringCR                    GetInstanceFilter (void) const;
        
        //! Set instance filter. Can be string.
        ECOBJECTS_EXPORT void                         SetInstanceFilter (WString value);
    };

END_BENTLEY_ECOBJECT_NAMESPACE