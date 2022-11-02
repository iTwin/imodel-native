/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/

#pragma once

#include <ECPresentation/Rules/PresentationRuleSet.h>
#include <ECPresentation/Rules/MultiSchemaClass.h>

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

/*---------------------------------------------------------------------------------**//**
This specification returns instance nodes of defined classes.
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct EXPORT_VTABLE_ATTRIBUTE InstanceNodesOfSpecificClassesSpecification : ChildNodeSpecification
{
    DEFINE_T_SUPER(ChildNodeSpecification)

private:
    bool     m_groupByClass;
    bool     m_groupByLabel;
    bool     m_showEmptyGroups;
    Utf8String  m_instanceFilter;
    bvector<MultiSchemaClass*> m_classes;
    bvector<MultiSchemaClass*> m_excludedClasses;

protected:
    //! Allows the visitor to visit this specification.
    ECPRESENTATION_EXPORT void _Accept(PresentationRuleSpecificationVisitor& visitor) const override;

    ECPRESENTATION_EXPORT bool _ShallowEqual(PresentationKeyCR other) const override;

    ECPRESENTATION_EXPORT Utf8CP _GetXmlElementName () const override;
    ECPRESENTATION_EXPORT bool _ReadXml (BeXmlNodeP xmlNode) override;
    ECPRESENTATION_EXPORT void _WriteXml (BeXmlNodeP xmlNode) const override;

    ECPRESENTATION_EXPORT Utf8CP _GetJsonElementType() const override;
    ECPRESENTATION_EXPORT bool _ReadJson(JsonValueCR json) override;
    ECPRESENTATION_EXPORT void _WriteJson(JsonValueR json) const override;

    //! Clones this specification.
    ChildNodeSpecification* _Clone() const override {return new InstanceNodesOfSpecificClassesSpecification(*this);}

    //! Computes specification hash.
    ECPRESENTATION_EXPORT MD5 _ComputeHash() const override;

public:
    //! Constructor. It is used to initialize the rule with default settings.
    ECPRESENTATION_EXPORT InstanceNodesOfSpecificClassesSpecification ();

    //! Constructor.
    //! @deprecated Use InstanceNodesOfSpecificClassesSpecification(int, ChildrenHint, bool, bool, bool, bool, Utf8StringCR, bvector<MultiSchemaClass*>, bvector<MultiSchemaClass*>)
    ECPRESENTATION_EXPORT InstanceNodesOfSpecificClassesSpecification (int priority, bool alwaysReturnsChildren,
        bool hideNodesInHierarchy, bool hideIfNoChildren, bool groupByClass, bool groupByLabel, bool showEmptyGroups,
        Utf8StringCR instanceFilter, Utf8StringCR classNames, bool arePolymorphic);

    //! Constructor.
    //! @deprecated Use InstanceNodesOfSpecificClassesSpecification (int, ChildrenHint, bool, bool, bool, bool, Utf8StringCR, bvector<MultiSchemaClass*>, bvector<MultiSchemaClass*>)
    ECPRESENTATION_EXPORT InstanceNodesOfSpecificClassesSpecification (int priority, ChildrenHint hasChildren, bool hideNodesInHierarchy,
        bool hideIfNoChildren, bool groupByClass, bool groupByLabel, Utf8StringCR instanceFilter, Utf8StringCR classNames, bool arePolymorphic);

    //! Constructor.
    ECPRESENTATION_EXPORT InstanceNodesOfSpecificClassesSpecification(int priority, ChildrenHint hasChildren, bool hideNodesInHierarchy,
        bool hideIfNoChildren, bool groupByClass, bool groupByLabel, Utf8StringCR instanceFilter, bvector<MultiSchemaClass*> classes,
        bvector<MultiSchemaClass*> excludedClasses);

    //! Copy constructor.
    ECPRESENTATION_EXPORT InstanceNodesOfSpecificClassesSpecification(InstanceNodesOfSpecificClassesSpecification const&);

    //! Move constructor.
    ECPRESENTATION_EXPORT InstanceNodesOfSpecificClassesSpecification(InstanceNodesOfSpecificClassesSpecification&&);

    //! Destructor
    ECPRESENTATION_EXPORT ~InstanceNodesOfSpecificClassesSpecification(void);

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
    //! @deprecated
    ECPRESENTATION_EXPORT bool                         GetShowEmptyGroups (void) const;

    //! Set ShowEmptyGroups value. Can be boolean.
    //! @deprecated
    ECPRESENTATION_EXPORT void                         SetShowEmptyGroups (bool value);

    //! Returns a vector of instance classes which should be included.
    bvector<MultiSchemaClass*> const&                  GetClasses(void) const {return m_classes;}

    //! Sets a vector of instance classes which should be included.
    ECPRESENTATION_EXPORT void                         SetClasses(bvector<MultiSchemaClass*> value);

    //! Returns a vector of instance classes which should be excluded.
    bvector<MultiSchemaClass*> const&                  GetExcludedClasses(void) const {return m_excludedClasses;}

    //! Sets a vector of instance classes which should be excluded.
    ECPRESENTATION_EXPORT void                         SetExcludedClasses(bvector<MultiSchemaClass*> value);

    //! InstanceFiler is spacially formated string that represents WhereCriteria in
    //! ECQuery that is used to filter query results (ChildNodes).
    ECPRESENTATION_EXPORT Utf8StringCR                 GetInstanceFilter (void) const;

    //! Set instance filter. Can be string.
    ECPRESENTATION_EXPORT void                         SetInstanceFilter (Utf8String value);
};

END_BENTLEY_ECPRESENTATION_NAMESPACE
