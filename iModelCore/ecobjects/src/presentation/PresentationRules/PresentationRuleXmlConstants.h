/*--------------------------------------------------------------------------------------+
|
|     $Source: src/presentation/PresentationRules/PresentationRuleXmlConstants.h $
|
|   $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/

#define COMMON_XML_ATTRIBUTE_VALUE_TARGET_TREE_MAINTREE                              "MainTree"
#define COMMON_XML_ATTRIBUTE_VALUE_TARGET_TREE_SELECTIONTREE                         "SelectionTree"
#define COMMON_XML_ATTRIBUTE_VALUE_TARGET_TREE_BOTH                                  "Both"

#define COMMON_XML_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BOTH                           "Both"
#define COMMON_XML_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_FORWARD                        "Forward"
#define COMMON_XML_ATTRIBUTE_VALUE_REQUIRED_DIRECTION_BACKWARD                       "Backward"

#define COMMON_XML_ATTRIBUTE_PRIORITY                                                 "Priority"
#define COMMON_XML_ATTRIBUTE_GROUPBYCLASS                                             "GroupByClass"
#define COMMON_XML_ATTRIBUTE_GROUPBYRELATIONSHIP                                      "GroupByRelationship"
#define COMMON_XML_ATTRIBUTE_GROUPBYLABEL                                             "GroupByLabel"
#define COMMON_XML_ATTRIBUTE_SUPPORTEDSCHEMAS                                         "SupportedSchemas"
#define COMMON_XML_ATTRIBUTE_SKIPRELATEDLEVEL                                         "SkipRelatedLevel"
#define COMMON_XML_ATTRIBUTE_SHOWEMPTYGROUPS                                          "ShowEmptyGroups"
#define COMMON_XML_ATTRIBUTE_INSTANCEFILTER                                           "InstanceFilter"
#define COMMON_XML_ATTRIBUTE_REQUIREDDIRECTION                                        "RequiredDirection"
#define COMMON_XML_ATTRIBUTE_RELATIONSHIPCLASSNAMES                                   "RelationshipClassNames"
#define COMMON_XML_ATTRIBUTE_RELATEDCLASSNAMES                                        "RelatedClassNames"
#define COMMON_XML_ATTRIBUTE_SCHEMANAME                                               "SchemaName"
#define COMMON_XML_ATTRIBUTE_CLASSNAMES                                               "ClassNames"
#define COMMON_XML_ATTRIBUTE_CLASSNAME                                                "ClassName"
#define COMMON_XML_ATTRIBUTE_PROPERTYNAMES                                            "PropertyNames"
#define COMMON_XML_ATTRIBUTE_PROPERTYNAME                                             "PropertyName"
#define COMMON_XML_ATTRIBUTE_ONLYIFNOTHANDLED                                         "OnlyIfNotHandled"
#define COMMON_XML_ATTRIBUTE_AREPOLYMORPHIC                                           "ArePolymorphic"
#define COMMON_XML_ATTRIBUTE_STOPFURTHERPROCESSING                                    "StopFurtherProcessing"

#define PRESENTATION_RULE_SET_XML_NODE_NAME                                           "PresentationRuleSet"
#define PRESENTATION_RULE_SET_XML_ATTRIBUTE_RULESETID                                 "RuleSetId"
#define PRESENTATION_RULE_SET_XML_ATTRIBUTE_ISSUPPLEMENTAL                            "IsSupplemental"
#define PRESENTATION_RULE_SET_XML_ATTRIBUTE_SUPPLEMENTATIONPURPOSE                    "SupplementationPurpose"
#define PRESENTATION_RULE_SET_XML_ATTRIBUTE_VERSIONMAJOR                              "VersionMajor"
#define PRESENTATION_RULE_SET_XML_ATTRIBUTE_VERSIONMINOR                              "VersionMinor"
#define PRESENTATION_RULE_SET_XML_ATTRIBUTE_PREFERREDIMAGE                            "PreferredImage"
#define PRESENTATION_RULE_SET_XML_ATTRIBUTE_ISSEARCHENABLED                           "IsSearchEnabled"
#define PRESENTATION_RULE_SET_XML_ATTRIBUTE_SEARCHCLASSES                             "SearchClasses"
#define PRESENTATION_RULE_SET_XML_ATTRIBUTE_EXTENDEDDATA                              "ExtendedData"

#define PRESENTATION_RULE_XML_ATTRIBUTE_CONDITION                                     "Condition"

#define ROOT_NODE_RULE_XML_NODE_NAME                                                  "RootNodeRule"
#define ROOT_NODE_RULE_XML_ATTRIBUTE_AUTOEXPAND                                       "AutoExpand"

#define SUB_CONDITION_XML_NODE_NAME                                                   "SubCondition"

#define CHILD_NODE_RULE_XML_NODE_NAME                                                 "ChildNodeRule"
#define CHILD_NODE_RULE_XML_ATTRIBUTE_TARGETTREE                                      "TargetTree"

#define CONTENT_RULE_XML_NODE_NAME                                                    "ContentRule"
#define CONTENT_RULE_XML_ATTRIBUTE_CUSTOMCONTROL                                      "CustomControl"

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
#define GROUPING_RULE_XML_ATTRIBUTE_SETTINGSID                                        "SettingsId"

#define CHECKBOX_RULE_XML_NODE_NAME                                                   "CheckBoxRule"
#define CHECKBOX_RULE_XML_ATTRIBUTE_USEINVERSEDPROPERTYVALUE                          "UseInversedPropertyValue"
#define CHECKBOX_RULE_XML_ATTRIBUTE_DEFAULTVALUE                                      "DefaultValue"

#define RENAMENODE_RULE_XML_NODE_NAME                                                 "RenameNodeRule"

#define SAMEL_LABEL_INSTANCE_GROUP_XML_NODE_NAME                                      "SameLabelInstanceGroup"

#define DISPLAYRELATEDITEMS_SPECIFICATION_XML_NODE_NAME                               "DisplayRelatedItems"
#define DISPLAYRELATEDITEMS_SPECIFICATION_XML_ATTRIBUTE_LOGICALCHILDREN               "LogicalChildren"
#define DISPLAYRELATEDITEMS_SPECIFICATION_XML_ATTRIBUTE_NESTINGDEPTH                  "NestingDepth"
#define DISPLAYRELATEDITEMS_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPCLASSES           "RelationshipClasses"

#define CLASS_GROUP_XML_NODE_NAME                                                     "ClassGroup"
#define CLASS_GROUP_XML_ATTRIBUTE_SCHEMANAME                                          "SchemaName"
#define CLASS_GROUP_XML_ATTRIBUTE_BASECLASSNAME                                       "BaseClassName"

#define PROPERTY_GROUP_XML_NODE_NAME                                                  "PropertyGroup"

#define GROUP_XML_ATTRIBUTE_IMAGEID                                                   "ImageId"
#define GROUP_XML_ATTRIBUTE_CREATEGROUPFORSINGLEITEM                                  "CreateGroupForSingleItem"
#define GROUP_XML_ATTRIBUTE_CREATEGROUPFORUNSPECIFIEDVALUES                           "CreateGroupForUnspecifiedValues"
#define GROUP_XML_ATTRIBUTE_MENULABEL                                                 "ContextMenuLabel"
#define GROUP_XML_ATTRIBUTE_DEFAULTLABEL                                              "DefaultGroupLabel"

#define PROPERTY_RANGE_GROUP_XML_NODE_NAME                                            "Range"
#define PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_LABEL                                      "Label"
#define PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_IMAGEID                                    "ImageId"
#define PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_FROMVALUE                                  "FromValue"
#define PROPERTY_RANGE_GROUP_XML_ATTRIBUTE_TOVALUE                                    "ToValue"

#define LOCALIZATION_DEFINITION_XML_NODE_NAME                                         "LocalizationResourceKeyDefinition"
#define LOCALIZATION_DEFINITION_XML_ATTRIBUTE_ID                                      "Id"
#define LOCALIZATION_DEFINITION_XML_ATTRIBUTE_KEY                                     "Key"
#define LOCALIZATION_DEFINITION_XML_ATTRIBUTE_DEFAULTVALUE                            "DefaultValue"

#define USER_SETTINGS_XML_NODE_NAME                                                   "UserSettings"
#define USER_SETTINGS_XML_ATTRIBUTE_CATEGORY_LABEL                                    "CategoryLabel"

#define USER_SETTINGS_ITEM_XML_NODE_NAME                                              "UserSettingsItem"
#define USER_SETTINGS_ITEM_XML_ATTRIBUTE_ID                                           "Id"
#define USER_SETTINGS_ITEM_XML_ATTRIBUTE_LABEL                                        "Label"
#define USER_SETTINGS_ITEM_XML_ATTRIBUTE_OPTIONS                                      "Options"
#define USER_SETTINGS_ITEM_XML_ATTRIBUTE_DEFAULT_VALUE                                "DefaultValue"

#define SORTING_RULE_XML_NODE_NAME                                                    "SortingRule"
#define SORTING_RULE_XML_ATTRIBUTE_SORTASCENDING                                      "SortAscending"
#define SORTING_RULE_XML_ATTRIBUTE_DONOTSORT                                          "DoNotSort"
#define SORTING_RULE_XML_ATTRIBUTE_ISPOLYMORPHIC                                      "IsPolymorphic"

//ChildNodeSpecifications
#define CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_ALWAYSRETURNSCHILDREN                  "AlwaysReturnsChildren"
#define CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_HIDENODESINHIERARCHY                   "HideNodesInHierarchy"
#define CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_HIDEIFNOCHILDREN                       "HideIfNoChildren"
#define CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_EXTENDEDDATA                           "ExtendedData"

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

#define SEARCH_QUERY_SPECIFICATION_XML_NODE_NAME                                      "Query"
#define SEARCH_QUERY_SPECIFICATION_XML_ATTRIBUTE_SCHEMA_NAME                          "SchemaName"
#define SEARCH_QUERY_SPECIFICATION_XML_ATTRIBUTE_CLASS_NAME                           "ClassName"

//ContentSpecifications
#define CONTENT_SPECIFICATION_XML_ATTRIBUTE_SHOWIMAGES                                "ShowImages"

#define RELATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME                                "RelatedProperties"

#define CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME             "ContentInstancesOfSpecificClasses"

#define CONTENT_RELATED_INSTANCES_SPECIFICATION_XML_NODE_NAME                         "ContentRelatedInstances"

#define SELECTED_NODE_INSTANCES_SPECIFICATION_XML_NODE_NAME                           "SelectedNodeInstances"
#define SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLESCHEMANAME      "AcceptableSchemaName"
#define SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLECLASSNAMES      "AcceptableClassNames"
#define SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLEPOLYMORPHICALLY "AcceptablePolymorphically"

#define HIDDEN_PROPERTIES_SPECIFICATION_XML_NODE_NAME                                   "HiddenProperties"
#define HIDDEN_PROPERTIES_SPECIFICATION_XML_ATTRIBUTE_CLASSNAME                         "ClassName"
#define HIDDEN_PROPERTIES_SPECIFICATION_XML_ATTRIBUTE_PROPERTYNAMES                     "PropertyNames"