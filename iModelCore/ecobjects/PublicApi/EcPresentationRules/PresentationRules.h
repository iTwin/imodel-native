/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/EcPresentationRules/PresentationRules.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#include <ECObjects/ECObjectsAPI.h>

EC_TYPEDEFS (ChildNodeRule);
EC_TYPEDEFS (ContentRule);
EC_TYPEDEFS (ImageIdOverride);
EC_TYPEDEFS (PresentationKey);
EC_TYPEDEFS (PresentationRule);
EC_TYPEDEFS (PresentationRulePreprocessor);
EC_TYPEDEFS (PresentationRuleSet);
EC_TYPEDEFS (RootNodeRule);
EC_TYPEDEFS (LabelOverride);
EC_TYPEDEFS (StyleOverride);
EC_TYPEDEFS (GroupingRule);
EC_TYPEDEFS (PropertyGroup);
EC_TYPEDEFS (PropertyRangeGroupSpecification);
EC_TYPEDEFS (LocalizationResourceKeyDefinition);
EC_TYPEDEFS (ChildNodeSpecification);
EC_TYPEDEFS (AllInstanceNodesSpecification);
EC_TYPEDEFS (AllRelatedInstanceNodesSpecification);
EC_TYPEDEFS (CustomNodeSpecification);
EC_TYPEDEFS (InstanceNodesOfSpecificClassesSpecification);
EC_TYPEDEFS (RelatedInstanceNodesSpecification);
EC_TYPEDEFS (SearchResultInstanceNodesSpecification);
EC_TYPEDEFS (ContentSpecification);
EC_TYPEDEFS (RelatedPropertiesSpecification);
EC_TYPEDEFS (ContentInstancesOfSpecificClassesSpecification);
EC_TYPEDEFS (ContentRelatedInstancesSpecification);
EC_TYPEDEFS (SelectedNodeInstancesSpecification);

BEGIN_BENTLEY_ECOBJECT_NAMESPACE
typedef RefCountedPtr<PresentationRuleSet>    PresentationRuleSetPtr;
END_BENTLEY_ECOBJECT_NAMESPACE

#include <ECPresentationRules/PresentationRule.h>
#include <ECPresentationRules/ChildNodeRule.h>
#include <ECPresentationRules/ChildNodeSpecification.h>
#include <ECPresentationRules/ContentRule.h>
#include <ECPresentationRules/ImageIdOverride.h>
#include <ECPresentationRules/PresentationRuleSet.h>
#include <ECPresentationRules/LabelOverride.h>
#include <ECPresentationRules/StyleOverride.h>
#include <ECPresentationRules/LocalizationResourceKeyDefinition.h>
#include <ECPresentationRules/GroupingRule.h>
#include <ECPresentationRules/AllInstanceNodesSpecification.h>
#include <ECPresentationRules/AllRelatedInstanceNodesSpecification.h>
#include <ECPresentationRules/CustomNodeSpecification.h>
#include <ECPresentationRules/InstanceNodesOfSpecificClassesSpecification.h>
#include <ECPresentationRules/RelatedInstanceNodesSpecification.h>
#include <ECPresentationRules/SearchResultInstanceNodesSpecification.h>
#include <ECPresentationRules/ContentSpecification.h>
#include <ECPresentationRules/RelatedPropertiesSpecification.h>
#include <ECPresentationRules/ContentInstancesOfSpecificClassesSpecification.h>
#include <ECPresentationRules/ContentRelatedInstancesSpecification.h>
#include <ECPresentationRules/SelectedNodeInstancesSpecification.h>
