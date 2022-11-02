/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once

#include <ECPresentation/ECPresentation.h>

ECPRESENTATION_TYPEDEFS(SubCondition);
ECPRESENTATION_TYPEDEFS(ChildNodeRule);
ECPRESENTATION_TYPEDEFS(ContentRule);
ECPRESENTATION_TYPEDEFS(ImageIdOverride);
ECPRESENTATION_TYPEDEFS(PresentationKey);
ECPRESENTATION_TYPEDEFS(PresentationRule);
ECPRESENTATION_TYPEDEFS(PresentationRuleSet);
ECPRESENTATION_TYPEDEFS(CustomizationRule);
ECPRESENTATION_TYPEDEFS(ConditionalPresentationRule);
ECPRESENTATION_TYPEDEFS(RootNodeRule);
ECPRESENTATION_TYPEDEFS(LabelOverride);
ECPRESENTATION_TYPEDEFS(InstanceLabelOverride);
ECPRESENTATION_TYPEDEFS(StyleOverride);
ECPRESENTATION_TYPEDEFS(CheckBoxRule);
ECPRESENTATION_TYPEDEFS(SortingRule);
ECPRESENTATION_TYPEDEFS(GroupingRule);
ECPRESENTATION_TYPEDEFS(ExtendedDataRule);
ECPRESENTATION_TYPEDEFS(NodeArtifactsRule);
ECPRESENTATION_TYPEDEFS(ContentModifier);
ECPRESENTATION_TYPEDEFS(DefaultPropertyCategoryOverride);
ECPRESENTATION_TYPEDEFS(GroupSpecification);
ECPRESENTATION_TYPEDEFS(SameLabelInstanceGroup);
ECPRESENTATION_TYPEDEFS(ClassGroup);
ECPRESENTATION_TYPEDEFS(PropertyGroup);
ECPRESENTATION_TYPEDEFS(PropertyRangeGroupSpecification);
ECPRESENTATION_TYPEDEFS(LocalizationResourceKeyDefinition);
ECPRESENTATION_TYPEDEFS(UserSettingsGroup);
ECPRESENTATION_TYPEDEFS(UserSettingsItem);
ECPRESENTATION_TYPEDEFS(RelatedInstanceSpecification);
ECPRESENTATION_TYPEDEFS(ChildNodeSpecification);
ECPRESENTATION_TYPEDEFS(AllInstanceNodesSpecification);
ECPRESENTATION_TYPEDEFS(AllRelatedInstanceNodesSpecification);
ECPRESENTATION_TYPEDEFS(CustomNodeSpecification);
ECPRESENTATION_TYPEDEFS(InstanceNodesOfSpecificClassesSpecification);
ECPRESENTATION_TYPEDEFS(RelatedInstanceNodesSpecification);
ECPRESENTATION_TYPEDEFS(SearchResultInstanceNodesSpecification);
ECPRESENTATION_TYPEDEFS(QuerySpecification);
ECPRESENTATION_TYPEDEFS(StringQuerySpecification);
ECPRESENTATION_TYPEDEFS(ECPropertyValueQuerySpecification);
ECPRESENTATION_TYPEDEFS(ContentSpecification);
ECPRESENTATION_TYPEDEFS(PropertySpecification);
ECPRESENTATION_TYPEDEFS(PropertyCategorySpecification);
ECPRESENTATION_TYPEDEFS(RelatedPropertiesSpecification);
ECPRESENTATION_TYPEDEFS(PropertyEditorSpecification);
ECPRESENTATION_TYPEDEFS(PropertyEditorParametersSpecification);
ECPRESENTATION_TYPEDEFS(CustomRendererSpecification);
ECPRESENTATION_TYPEDEFS(CalculatedPropertiesSpecification);
ECPRESENTATION_TYPEDEFS(ContentInstancesOfSpecificClassesSpecification);
ECPRESENTATION_TYPEDEFS(ContentRelatedInstancesSpecification);
ECPRESENTATION_TYPEDEFS(SelectedNodeInstancesSpecification);
ECPRESENTATION_TYPEDEFS(PresentationRuleSpecificationVisitor);
ECPRESENTATION_TYPEDEFS(RequiredSchemaSpecification);

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

//! This enumerator allows to chose which direction should be honored
//! when selecting relationships in the query.
enum RequiredRelationDirection : unsigned
    {
    //! Folows relationships in both directions (default).
    RequiredRelationDirection_Both = 0,
    //! Folows only Forward relationships.
    RequiredRelationDirection_Forward = 1,
    //! Folows only Backward relationships.
    RequiredRelationDirection_Backward = 2
    };

/*---------------------------------------------------------------------------------**//**
* @bsiclass
+---------------+---------------+---------------+---------------+---------------+------*/
struct Version
{
private:
    unsigned m_major;
    unsigned m_minor;
    unsigned m_patch;
public:
    Version() : m_major(0), m_minor(0), m_patch(0) {}
    Version(unsigned major, unsigned minor, unsigned patch)
        : m_major(major), m_minor(minor), m_patch(patch)
        {}
    unsigned GetMajor() const {return m_major;}
    unsigned GetMinor() const {return m_minor;}
    unsigned GetPatch() const {return m_patch;}

    int CompareTo(Version const& other) const
        {
        int majorDiff = m_major - other.m_major;
        if (majorDiff != 0)
            return majorDiff;
        int minorDiff = m_minor - other.m_minor;
        if (minorDiff != 0)
            return minorDiff;
        int patchDiff = m_patch - other.m_patch;
        return patchDiff;
        }
    bool operator<(Version const& other) const {return CompareTo(other) < 0;}
    bool operator<=(Version const& other) const {return CompareTo(other) <= 0;}
    bool operator>(Version const& other) const {return CompareTo(other) > 0;}
    bool operator>=(Version const& other) const {return CompareTo(other) >= 0;}
    bool operator==(Version const& other) const {return CompareTo(other) == 0;}

    ECPRESENTATION_EXPORT static BentleyStatus FromString(Version& version, Utf8CP str);
    ECPRESENTATION_EXPORT Utf8String ToString() const;
};

enum class RelationshipMeaning;

typedef RefCountedPtr<PresentationRuleSet>    		PresentationRuleSetPtr;
typedef RefCountedPtr<PresentationRuleSet const>    PresentationRuleSetCPtr;
typedef bvector<RootNodeRuleP>                      RootNodeRuleList;
typedef bvector<ChildNodeRuleP>                     ChildNodeRuleList;
typedef bvector<ContentRuleP>                       ContentRuleList;
typedef bvector<ImageIdOverrideP>                   ImageIdOverrideList;
typedef bvector<LabelOverrideP>                     LabelOverrideList;
typedef bvector<StyleOverrideP>                     StyleOverrideList;
typedef bvector<GroupingRuleP>                      GroupingRuleList;
typedef bvector<LocalizationResourceKeyDefinitionP> LocalizationResourceKeyDefinitionList;
typedef bvector<UserSettingsGroupP>                 UserSettingsGroupList;
typedef bvector<CheckBoxRuleP>                      CheckBoxRuleList;
typedef bvector<SortingRuleP>                       SortingRuleList;
typedef bvector<ContentModifierP>                   ContentModifierList;
typedef bvector<InstanceLabelOverrideP>             InstanceLabelOverrideList;
typedef bvector<PropertyEditorParametersSpecificationP> PropertyEditorParametersList;
typedef bvector<RelatedInstanceSpecificationP>      RelatedInstanceSpecificationList;
typedef bvector<ExtendedDataRuleP>                  ExtendedDataRuleList;
typedef bvector<NodeArtifactsRuleP>                 NodeArtifactsRuleList;
typedef bvector<RelatedPropertiesSpecificationP>    RelatedPropertiesSpecificationList;
typedef bvector<CalculatedPropertiesSpecificationP> CalculatedPropertiesSpecificationList;
typedef bvector<PropertyCategorySpecificationP>     PropertyCategorySpecificationsList;
typedef bvector<PropertySpecificationP>             PropertySpecificationsList;
typedef bvector<DefaultPropertyCategoryOverrideP>   DefaultPropertyCategoryOverridesList;
typedef bvector<RequiredSchemaSpecificationP>       RequiredSchemaSpecificationsList;

END_BENTLEY_ECPRESENTATION_NAMESPACE
