/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/PresentationRuleXmlConstants.h $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#define COMMON_XML_ATTRIBUTE_VALUE_TARGET_TREE_MAINTREE                              L"MainTree"
#define COMMON_XML_ATTRIBUTE_VALUE_TARGET_TREE_SELECTIONTREE                         L"SelectionTree"
#define COMMON_XML_ATTRIBUTE_VALUE_TARGET_TREE_BOTH                                  L"Both"

#define COMMON_XML_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BOTH                           L"Both"
#define COMMON_XML_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_FORWARD                        L"Forward"
#define COMMON_XML_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BACKWARD                       L"Backward"

#define COMMON_XML_ATTRIBUTE_PRIORITY                                                 "Priority"
#define COMMON_XML_ATTRIBUTE_GROUPBYCLASS                                             "GroupByClass"
#define COMMON_XML_ATTRIBUTE_GROUPBYLABEL                                             "GroupByLabel"
#define COMMON_XML_ATTRIBUTE_SUPPORTEDSCHEMAS                                         "SupportedSchemas"
#define COMMON_XML_ATTRIBUTE_SKIPRELATEDLEVEL                                         "SkipRelatedLevel"
#define COMMON_XML_ATTRIBUTE_SHOWEMPTYGROUPS                                          "ShowEmptyGroups"
#define COMMON_XML_ATTRIBUTE_INSTANCEFILTER                                           "InstanceFilter"
#define COMMON_XML_ATTRIBUTE_REQUIREDDIRECTION                                        "RequiredDirection"
#define COMMON_XML_ATTRIBUTE_RELATIONSHIPSCHEMANAME                                   "RelationshipSchemaName"
#define COMMON_XML_ATTRIBUTE_RELATIONSHIPCLASSNAMES                                   "RelationshipClassNames"
#define COMMON_XML_ATTRIBUTE_RELATEDSCHEMANAME                                        "RelatedSchemaName"
#define COMMON_XML_ATTRIBUTE_RELATEDCLASSNAMES                                        "RelatedClassNames"
#define COMMON_XML_ATTRIBUTE_SCHEMANAME                                               "SchemaName"
#define COMMON_XML_ATTRIBUTE_CLASSNAMES                                               "ClassNames"
#define COMMON_XML_ATTRIBUTE_ONLYIFNOTHANDLED                                         "OnlyIfNotHandled"

#define PRESENTATION_RULE_SET_XML_NODE_NAME                                           "PresentationRuleSet"
#define PRESENTATION_RULE_SET_XML_ATTRIBUTE_RULESETID                                 "RuleSetId"
#define PRESENTATION_RULE_SET_XML_ATTRIBUTE_ISSUPPLEMENTAL                            "IsSupplemental"
#define PRESENTATION_RULE_SET_XML_ATTRIBUTE_VERSION                                   "Version"
#define PRESENTATION_RULE_SET_XML_ATTRIBUTE_PREFERREDIMAGE                            "PreferredImage"

#define PRESENTATION_RULE_XML_ATTRIBUTE_CONDITION                                     "Condition"

#define ROOT_NODE_RULE_XML_NODE_NAME                                                  "RootNodeRule"

#define CHILD_NODE_RULE_XML_NODE_NAME                                                 "ChildNodeRule"
#define CHILD_NODE_RULE_XML_ATTRIBUTE_TARGETTREE                                      "TargetTree"

#define CONTENT_RULE_XML_NODE_NAME                                                    "ContentRule"

#define IMAGE_ID_OVERRIDE_XML_NODE_NAME                                               "ImageIdOverride"
#define IMAGE_ID_OVERRIDE_XML_ATTRIBUTE_IMAGEID                                       "ImageId"

#define LABEL_OVERRIDE_XML_NODE_NAME                                                  "LabelOverride"
#define LABEL_OVERRIDE_XML_ATTRIBUTE_LABEL                                            "Label"
#define LABEL_OVERRIDE_XML_ATTRIBUTE_DESCRIPTION                                      "Description"

#define STYLE_OVERRIDE_XML_NODE_NAME                                                  "StyleOverride"
#define STYLE_OVERRIDE_XML_ATTRIBUTE_FORECOLOR                                        "ForeColor"
#define STYLE_OVERRIDE_XML_ATTRIBUTE_BACKCOLOR                                        "BackColor"
#define STYLE_OVERRIDE_XML_ATTRIBUTE_FONTSTYLE                                        "FontStyle"

#define GROUPING_RULE_XML_NODE_NAME                                                   "GroupingRule"
#define GROUPING_RULE_XML_ATTRIBUTE_SCHEMANAME                                        "SchemaName"
#define GROUPING_RULE_XML_ATTRIBUTE_CLASSNAME                                         "ClassName"
#define GROUPING_RULE_XML_ATTRIBUTE_MENUCONDITION                                     "ContextMenuCondition"
#define GROUPING_RULE_XML_ATTRIBUTE_MENULABEL                                         "ContextMenuLabel"

#define PROPERTY_GROUP_XML_NODE_NAME                                                  "PropertyGroup"
#define PROPERTY_GROUP_XML_ATTRIBUTE_PROPERTYNAME                                     "PropertyName"
#define PROPERTY_GROUP_XML_ATTRIBUTE_IMAGEID                                          "ImageId"
#define PROPERTY_GROUP_XML_ATTRIBUTE_CREATEGROUPFORSINGLEITEM                         "CreateGroupForSingleItem"
#define PROPERTY_GROUP_XML_ATTRIBUTE_MENULABEL                                        "ContextMenuLabel"

#define PROPERTY_RANGE_GROUP_XML_NODE_NAME                                            "Range"
#define PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_LABEL                                      "Label"
#define PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_IMAGEID                                    "ImageId"
#define PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_FROMVALUE                                  "FromValue"
#define PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_TOVALUE                                    "ToValue"

#define LOCALIZATION_DEFINITION_XML_NODE_NAME                                         "LocalizationResourceKeyDefinition"
#define LOCALIZATION_DEFINITION_XML_ATTRIBUTE_ID                                      "Id"
#define LOCALIZATION_DEFINITION_XML_ATTRIBUTE_KEY                                     "Key"
#define LOCALIZATION_DEFINITION_XML_ATTRIBUTE_DEFAULTVALUE                            "DefaultValue"

//ChildNodeSpecifications
#define CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_ALWAYSRETURNSCHILDREN                  "AlwaysReturnsChildren"
#define CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_HIDENODESINHIERARCHY                   "HideNodesInHierarchy"

#define ALL_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME                                "AllInstances"

#define ALL_RELATED_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME                        "AllRelatedInstances"

#define CUSTOM_NODE_SPECIFICATION_XML_NODE_NAME                                       "CustomNode"
#define CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_TYPE                                  "Type"
#define CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_LABEL                                 "Label"
#define CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_DESCRIPTION                           "Description"
#define CUSTOM_NODE_SPECIFICATION_XML_ATTRIBUTE_IMAGEID                               "ImageId"

#define INSTANCE_NODES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME                "InstancesOfSpecificClasses"

#define RELATED_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME                            "RelatedInstances"

#define SEARCH_RESULT_INSTANCE_NODES_SPECIFICATION_XML_NODE_NAME                      "SearchResultInstances"

//ContentSpecifications
#define RELATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME                                "RelatedProperties"

#define CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME             "ContentInstancesOfSpecificClasses"

#define CONTENT_RELATED_INSTANCES_SPECIFICATION_XML_NODE_NAME                         "ContentRelatedInstances"

#define SELECTED_NODE_INSTANCES_SPECIFICATION_XML_NODE_NAME                           "SelectedNodeInstances"
#define SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLESCHEMANAME      "AcceptableSchemaName"
#define SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLECLASSNAMES      "AcceptableClassNames"
