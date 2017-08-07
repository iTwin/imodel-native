/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECPresentationRules/PresentationRulesTypes.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/

#pragma once
/*__PUBLISH_SECTION_START__*/

#include <ECObjects/ECObjectsAPI.h>

EC_TYPEDEFS (SubCondition);
EC_TYPEDEFS (ChildNodeRule);
EC_TYPEDEFS (ContentRule);
EC_TYPEDEFS (ImageIdOverride);
EC_TYPEDEFS (PresentationKey);
EC_TYPEDEFS (PresentationRule);
EC_TYPEDEFS (PresentationRuleSet);
EC_TYPEDEFS (CustomizationRule);
EC_TYPEDEFS (RootNodeRule);
EC_TYPEDEFS (LabelOverride);
EC_TYPEDEFS (StyleOverride);
EC_TYPEDEFS (CheckBoxRule);
EC_TYPEDEFS (RenameNodeRule);
EC_TYPEDEFS (SortingRule);
EC_TYPEDEFS (DisplayRelatedItemsSpecification);
EC_TYPEDEFS (GroupingRule);
EC_TYPEDEFS (ContentModifier);
EC_TYPEDEFS (GroupSpecification);
EC_TYPEDEFS (SameLabelInstanceGroup);
EC_TYPEDEFS (ClassGroup);
EC_TYPEDEFS (PropertyGroup);
EC_TYPEDEFS (PropertyRangeGroupSpecification);
EC_TYPEDEFS (LocalizationResourceKeyDefinition);
EC_TYPEDEFS (UserSettingsGroup);
EC_TYPEDEFS (UserSettingsItem);
EC_TYPEDEFS (RelatedInstanceSpecification);
EC_TYPEDEFS (ChildNodeSpecification);
EC_TYPEDEFS (AllInstanceNodesSpecification);
EC_TYPEDEFS (AllRelatedInstanceNodesSpecification);
EC_TYPEDEFS (CustomNodeSpecification);
EC_TYPEDEFS (InstanceNodesOfSpecificClassesSpecification);
EC_TYPEDEFS (RelatedInstanceNodesSpecification);
EC_TYPEDEFS (SearchResultInstanceNodesSpecification);
EC_TYPEDEFS (QuerySpecification);
EC_TYPEDEFS (StringQuerySpecification);
EC_TYPEDEFS (ECPropertyValueQuerySpecification);
EC_TYPEDEFS (ContentSpecification);
EC_TYPEDEFS (RelatedPropertiesSpecification);
EC_TYPEDEFS (PropertiesDisplaySpecification);
EC_TYPEDEFS (PropertyEditorsSpecification);
EC_TYPEDEFS (CalculatedPropertiesSpecification);
EC_TYPEDEFS (ContentInstancesOfSpecificClassesSpecification);
EC_TYPEDEFS (ContentRelatedInstancesSpecification);
EC_TYPEDEFS (SelectedNodeInstancesSpecification);
EC_TYPEDEFS (PresentationRuleSpecificationVisitor);

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

enum RequiredRelationDirection : unsigned; 

typedef RefCountedPtr<PresentationRuleSet>    		PresentationRuleSetPtr;
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
typedef bvector<RenameNodeRuleP>                    RenameNodeRuleList;
typedef bvector<SortingRuleP>                       SortingRuleList;
typedef bvector<ContentModifierP>                   ContentModifierList;

END_BENTLEY_ECOBJECT_NAMESPACE
