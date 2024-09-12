/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#define INVALID_XML                                                                   "Invalid `%s` xml node: Attibute `%s` is not specified or has incorrect value."

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
#define COMMON_XML_ATTRIBUTE_ISPOLYMORPHIC                                            "IsPolymorphic"
#define COMMON_XML_ATTRIBUTE_STOPFURTHERPROCESSING                                    "StopFurtherProcessing"
#define COMMON_XML_ATTRIBUTE_RELATIONSHIPMEANING                                      "RelationshipMeaning"
#define COMMON_XML_ATTRIBUTE_AUTOEXPAND                                               "AutoExpand"

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

#define INSTANCE_LABEL_OVERRIDE_XML_NODE_NAME                                          "InstanceLabelOverride"
#define INSTANCE_LABEL_OVERRIDE_XML_ATTRIBUTE_CLASSNAME                                "ClassName"
#define INSTANCE_LABEL_OVERRIDE_XML_ATTRIBUTE_PROPERTYNAMES                            "PropertyNames"

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
#define CHECKBOX_RULE_XML_ATTRIBUTE_ISENABLED                                         "IsEnabled"

#define SAME_LABEL_INSTANCE_GROUP_XML_NODE_NAME                                       "SameLabelInstanceGroup"

#define CONTENTMODIFIER_XML_NODE_NAME                                                 "ContentModifier"
#define CONTENTMODIFIER_XML_ATTRIBUTE_SCHEMANAME                                      "SchemaName"
#define CONTENTMODIFIER_XML_ATTRIBUTE_CLASSNAME                                       "ClassName"

#define CLASS_GROUP_XML_NODE_NAME                                                     "ClassGroup"
#define CLASS_GROUP_XML_ATTRIBUTE_SCHEMANAME                                          "SchemaName"
#define CLASS_GROUP_XML_ATTRIBUTE_BASECLASSNAME                                       "BaseClassName"

#define PROPERTY_GROUP_XML_NODE_NAME                                                  "PropertyGroup"

#define GROUP_XML_ATTRIBUTE_IMAGEID                                                   "ImageId"
#define GROUP_XML_ATTRIBUTE_CREATEGROUPFORSINGLEITEM                                  "CreateGroupForSingleItem"
#define GROUP_XML_ATTRIBUTE_CREATEGROUPFORUNSPECIFIEDVALUES                           "CreateGroupForUnspecifiedValues"
#define GROUP_XML_ATTRIBUTE_MENULABEL                                                 "ContextMenuLabel"
#define GROUP_XML_ATTRIBUTE_DEFAULTLABEL                                              "DefaultGroupLabel"

#define PROPERTY_GROUP_XML_ATTRIBUTE_GROUPINGVALUE                                    "GroupingValue"
#define PROPERTY_GROUP_XML_ATTRIBUTE_SORTINGVALUE                                     "SortingValue"
#define PROPERTY_GROUP_XML_ATTRIBUTE_VALUE_GROUPINGVALUE_PROPERTYVALUE                "PropertyValue"
#define PROPERTY_GROUP_XML_ATTRIBUTE_VALUE_GROUPINGVALUE_DISPLAYLABEL                 "DisplayLabel"

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

//ChildNodeSpecifications
#define CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_ALWAYSRETURNSCHILDREN                  "AlwaysReturnsChildren"
#define CHILD_NODE_SPECIFICATION_XML_ATTRIBUTE_HASCHILDREN                            "HasChildren"
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

#define SEARCH_QUERY_SPECIFICATION_XML_ATTRIBUTE_SCHEMA_NAME                          "SchemaName"
#define SEARCH_QUERY_SPECIFICATION_XML_ATTRIBUTE_CLASS_NAME                           "ClassName"
#define STRING_QUERY_SPECIFICATION_XML_NODE_NAME                                      "StringQuery"
#define ECPROPERTY_VALUE_QUERY_SPECIFICATION_XML_NODE_NAME                            "PropertyValueQuery"
#define ECPROPERTY_VALUE_QUERY_SPECIFICATION_XML_ATTRIBUTE_PARENT_PROPERTY_NAME       "ParentPropertyName"

#define RELATED_INSTANCE_SPECIFICATION_XML_NODE_NAME                                  "RelatedInstance"
#define RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_CLASSNAME                        "ClassName"
#define RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPNAME                 "RelationshipName"
#define RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_RELATIONSHIPDIRECTION            "RelationshipDirection"
#define RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_ALIAS                            "Alias"
#define RELATED_INSTANCE_SPECIFICATION_XML_ATTRIBUTE_ISREQUIRED                       "IsRequired"

//ContentSpecifications
#define CONTENT_SPECIFICATION_XML_ATTRIBUTE_SHOWIMAGES                                "ShowImages"

#define RELATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME                                "RelatedProperties"

#define CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_XML_NODE_NAME             "ContentInstancesOfSpecificClasses"

#define CONTENT_RELATED_INSTANCES_SPECIFICATION_XML_NODE_NAME                         "ContentRelatedInstances"
#define CONTENT_RELATED_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ISRECURSIVE             "IsRecursive"

#define SELECTED_NODE_INSTANCES_SPECIFICATION_XML_NODE_NAME                           "SelectedNodeInstances"
#define SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLESCHEMANAME      "AcceptableSchemaName"
#define SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLECLASSNAMES      "AcceptableClassNames"
#define SELECTED_NODE_INSTANCES_SPECIFICATION_XML_ATTRIBUTE_ACCEPTABLEPOLYMORPHICALLY "AcceptablePolymorphically"

#define HIDDEN_PROPERTIES_SPECIFICATION_XML_NODE_NAME                                 "HiddenProperties"
#define DISPLAYED_PROPERTIES_SPECIFICATION_XML_NODE_NAME                              "DisplayedProperties"
#define PROPERTIES_DISPLAY_SPECIFICATION_XML_ATTRIBUTE_CLASSNAME                      "ClassName"
#define PROPERTIES_DISPLAY_SPECIFICATION_XML_ATTRIBUTE_PROPERTYNAMES                  "PropertyNames"
#define PROPERTIES_DISPLAY_SPECIFICATION_XML_ATTRIBUTE_PRIORITY                       "Priority"

#define CALCULATED_PROPERTIES_SPECIFICATION_XML_NODE_NAME                             "CalculatedProperties"
#define CALCULATED_PROPERTIES_SPECIFICATION_XML_CHILD_NAME                            "Property"
#define CALCULATED_PROPERTIES_SPECIFICATION_XML_ATTRIBUTE_LABEL                       "Label"
#define CALCULATED_PROPERTIES_SPECIFICATION_XML_ATTRIBUTE_PRIORITY                    "Priority"

// Content: Property Editors
#define PROPERTY_EDITORS_SPECIFICATION_XML_NODE_NAME                                  "PropertyEditors"
#define PROPERTY_EDITORS_SPECIFICATION_XML_CHILD_NAME                                 "Editor"
#define PROPERTY_EDITORS_SPECIFICATION_XML_ATTRIBUTE_PROPERTYNAME                     "PropertyName"
#define PROPERTY_EDITORS_SPECIFICATION_XML_ATTRIBUTE_EDITORNAME                       "EditorName"

#define PROPERTY_EDITOR_JSON_PARAMETERS_XML_NODE_NAME                                 "JsonParams"

#define PROPERTY_EDITOR_MULTILINE_PARAMETERS_XML_NODE_NAME                            "MultilineParams"
#define PROPERTY_EDITOR_MULTILINE_PARAMETERS_ATTRIBUTE_HEIGHT                         "HeightInRows"

#define PROPERTY_EDITOR_RANGE_PARAMETERS_XML_NODE_NAME                                "RangeParams"
#define PROPERTY_EDITOR_RANGE_PARAMETERS_XML_ATTRIBUTE_MINIMUM                        "Minimum"
#define PROPERTY_EDITOR_RANGE_PARAMETERS_XML_ATTRIBUTE_MAXIMUM                        "Maximum"

#define PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_NODE_NAME                               "SliderParams"
#define PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_MINIMUM                       "Minimum"
#define PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_MAXIMUM                       "Maximum"
#define PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_INTERVALS                     "Intervals"
#define PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_VALUEFACTOR                   "ValueFactor"
#define PROPERTY_EDITOR_SLIDER_PARAMETERS_XML_ATTRIBUTE_VERTICAL                      "Vertical"
