/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/Rules/InstanceNodesOfSpecificClassesSpecification.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECPresentation/RulesDriven/Rules/PresentationRuleSet.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
This specification returns instance nodes of defined classes.
* @bsiclass                                     Eligijus.Mauragas               10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE InstanceNodesOfSpecificClassesSpecification : public ChildNodeSpecification
    {
    private:
        bool     m_groupByClass;
        bool     m_groupByLabel;
        bool     m_showEmptyGroups;
        bool     m_arePolymorphic;
        Utf8String  m_instanceFilter;
        Utf8String  m_classNames;

    protected:
        //! Allows the visitor to visit this specification.
        ECPRESENTATION_EXPORT virtual void _Accept(PresentationRuleSpecificationVisitor& visitor) const override;

        //! Returns XmlElement name that is used to read/save this rule information.
        ECPRESENTATION_EXPORT virtual CharCP               _GetXmlElementName () const override;

        //! Reads rule information from XmlNode, returns true if it can read it successfully.
        ECPRESENTATION_EXPORT virtual bool                 _ReadXml (BeXmlNodeP xmlNode) override;

        //! Writes rule information to given XmlNode.
        ECPRESENTATION_EXPORT virtual void                 _WriteXml (BeXmlNodeP xmlNode) const override;
        
        //! Clones this specification.
        virtual ChildNodeSpecification* _Clone() const override {return new InstanceNodesOfSpecificClassesSpecification(*this);}

        //! Computes specification hash.
        ECPRESENTATION_EXPORT virtual MD5 _ComputeHash(Utf8CP parentHash) const override;

    public:
        //! Constructor. It is used to initialize the rule with default settings.
        ECPRESENTATION_EXPORT InstanceNodesOfSpecificClassesSpecification ();

        //! Constructor.
        ECPRESENTATION_EXPORT InstanceNodesOfSpecificClassesSpecification (int priority, bool alwaysReturnsChildren, bool hideNodesInHierarchy,
                                                                      bool hideIfNoChildren, bool groupByClass, bool groupByLabel, bool showEmptyGroups,
                                                                      Utf8StringCR instanceFilter, Utf8StringCR classNames, bool arePolymorphic);

        //! Returns true if grouping by class should be applied.
        ECPRESENTATION_EXPORT bool                         GetGroupByClass (void) const;

        //! Set GroupByClass value. Can be boolean.
        ECPRESENTATION_EXPORT void                         SetGroupByClass (bool value);

        //! Returns true if grouping by label should be applied.
        ECPRESENTATION_EXPORT bool                         GetGroupByLabel (void) const;

        //! Set GroupByLabel value. Can be boolean.
        ECPRESENTATION_EXPORT void                         SetGroupByLabel (bool value);

        //! Returns true if class grouping nodes should be shown even if there are no 
        //! ECInstances of those classes. Grouping nodes will be generated for all listed classes.
        ECPRESENTATION_EXPORT bool                         GetShowEmptyGroups (void) const;

        //! Set ShowEmptyGroups value. Can be boolean.
        ECPRESENTATION_EXPORT void                         SetShowEmptyGroups (bool value);

        //! Class names. Format: "SchemaName1:ClassName11,ClassName12;SchemaName2:ClassName21,ClassName22"
        ECPRESENTATION_EXPORT Utf8StringCR                 GetClassNames (void) const;

        //! Set class names. Can be string.
        ECPRESENTATION_EXPORT void                         SetClassNames (Utf8String value);

        //! This flag identifies whether ECClasses defined in this specification should be marked as polymorphic in the Query.
        ECPRESENTATION_EXPORT bool                         GetArePolymorphic (void) const;

        //! Set ArePolymorphic value. Can be boolean.
        ECPRESENTATION_EXPORT void                         SetArePolymorphic (bool value);

        //! InstanceFiler is spacially formated string that represents WhereCriteria in 
        //! ECQuery that is used to filter query results (ChildNodes).
        ECPRESENTATION_EXPORT Utf8StringCR                 GetInstanceFilter (void) const;
        
        //! Set instance filter. Can be string.
        ECPRESENTATION_EXPORT void                         SetInstanceFilter (Utf8String value);
    };

END_BENTLEY_ECPRESENTATION_NAMESPACE
