/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicAPI/ECPresentation/RulesDriven/Rules/PresentationRulesTypes.h $
|
|  $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

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
ECPRESENTATION_TYPEDEFS(ContentModifier);
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
ECPRESENTATION_TYPEDEFS(RelatedPropertiesSpecification);
ECPRESENTATION_TYPEDEFS(PropertiesDisplaySpecification);
ECPRESENTATION_TYPEDEFS(PropertyEditorsSpecification);
ECPRESENTATION_TYPEDEFS(PropertyEditorParametersSpecification);
ECPRESENTATION_TYPEDEFS(CalculatedPropertiesSpecification);
ECPRESENTATION_TYPEDEFS(ContentInstancesOfSpecificClassesSpecification);
ECPRESENTATION_TYPEDEFS(ContentRelatedInstancesSpecification);
ECPRESENTATION_TYPEDEFS(SelectedNodeInstancesSpecification);
ECPRESENTATION_TYPEDEFS(PresentationRuleSpecificationVisitor);

BEGIN_BENTLEY_ECPRESENTATION_NAMESPACE

#define LOGGER_NAMESPACE_ECPRESENTATION_RULES   LOGGER_NAMESPACE_ECPRESENTATION ".Rules"
#define ECPRENSETATION_RULES_LOG (*NativeLogging::LoggingManager::GetLogger(LOGGER_NAMESPACE_ECPRESENTATION_RULES))

enum RequiredRelationDirection : unsigned; 
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

END_BENTLEY_ECPRESENTATION_NAMESPACE
