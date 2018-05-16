/*--------------------------------------------------------------------------------------+
|
|     $Source: Source/RulesDriven/Rules/PresentationRuleJsonConstants.h $
|
|   $Copyright: (c) 2018 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__BENTLEY_INTERNAL_ONLY__*/
#define INVALID_JSON                                                                   "Invalid %s json: Attribute %s is not specified or has incorrect value."

#define COMMON_JSON_ATTRIBUTE_PRIORITY                                                 "priority"
#define COMMON_JSON_ATTRIBUTE_GROUPBYCLASS                                             "groupByClass"
#define COMMON_JSON_ATTRIBUTE_GROUPBYRELATIONSHIP                                      "groupByRelationship"
#define COMMON_JSON_ATTRIBUTE_GROUPBYLABEL                                             "groupByLabel"
#define COMMON_JSON_ATTRIBUTE_SUPPORTEDSCHEMAS                                         "supportedSchemas"
#define COMMON_JSON_ATTRIBUTE_SKIPRELATEDLEVEL                                         "skipRelatedLevel"
#define COMMON_JSON_ATTRIBUTE_SHOWEMPTYGROUPS                                          "showEmptyGroups"
#define COMMON_JSON_ATTRIBUTE_INSTANCEFILTER                                           "instanceFilter"
#define COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION                                        "requiredDirection"
#define COMMON_JSON_ATTRIBUTE_RELATIONSHIPCLASSNAMES                                   "relationshipClassNames"
#define COMMON_JSON_ATTRIBUTE_RELATEDCLASSNAMES                                        "relatedClassNames"
#define COMMON_JSON_ATTRIBUTE_SCHEMANAME                                               "schemaName"
#define COMMON_JSON_ATTRIBUTE_CLASSNAMES                                               "classNames"
#define COMMON_JSON_ATTRIBUTE_CLASSNAME                                                "className"
#define COMMON_JSON_ATTRIBUTE_PROPERTYNAMES                                            "propertyNames"
#define COMMON_JSON_ATTRIBUTE_PROPERTYNAME                                             "propertyName"
#define COMMON_JSON_ATTRIBUTE_ONLYIFNOTHANDLED                                         "onlyIfNotHandled"
#define COMMON_JSON_ATTRIBUTE_AREPOLYMORPHIC                                           "arePolymorphic"
#define COMMON_JSON_ATTRIBUTE_ISPOLYMORPHIC                                            "isPolymorphic"
#define COMMON_JSON_ATTRIBUTE_STOPFURTHERPROCESSING                                    "stopFurtherProcessing"
#define COMMON_JSON_ATTRIBUTE_RELATIONSHIPMEANING                                      "relationshipMeaning"

#define CHILD_NODE_RULE_JSON_ATTRIBUTE_SUBCONDITIONS                                   "subConditions"
#define CHILD_NODE_RULE_JSON_ATTRIBUTE_SPECIFICATIONS                                  "specifications"
#define CHILD_NODE_RULE_JSON_ATTRIBUTE_CUSTOMIZATIONRULES                              "customizationRules"

#define PRESENTATION_RULE_SET_JSON_NAME                                                "PresentationRuleSet"
#define PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULESETID                                 "ruleSetId"
#define PRESENTATION_RULE_SET_JSON_ATTRIBUTE_SUPPORTEDSCHEMAS                          "supportedSchemas"
#define PRESENTATION_RULE_SET_JSON_ATTRIBUTE_ISSUPPLEMENTAL                            "isSupplemental"
#define PRESENTATION_RULE_SET_JSON_ATTRIBUTE_SUPPLEMENTATIONPURPOSE                    "supplementalPurpose"
#define PRESENTATION_RULE_SET_JSON_ATTRIBUTE_VERSIONMAJOR                              "versionMajor"
#define PRESENTATION_RULE_SET_JSON_ATTRIBUTE_VERSIONMINOR                              "versionMinor"
#define PRESENTATION_RULE_SET_JSON_ATTRIBUTE_CONTENTMODIFIERS                          "contentModifiers"
#define PRESENTATION_RULE_SET_JSON_ATTRIBUTE_USERSETTINGS                              "userSettings"
#define PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULES                                     "rules"

#define PRESENTATION_RULE_JSON_ATTRIBUTE_CONDITION                                     "condition"

#define ROOT_NODE_RULE_JSON_TYPE                                                       "RootNode"
#define ROOT_NODE_RULE_JSON_ATTRIBUTE_AUTOEXPAND                                       "autoExpand"

#define SUB_CONDITION_JSON_ATTRIBUTE_SUBCONDITIONS                                     "subConditions"
#define SUB_CONDITION_JSON_ATTRIBUTE_SPECIFICATIONS                                    "specifications"

#define CHILD_NODE_RULE_JSON_TYPE                                                      "ChildNode"
#define CHILD_NODE_RULE_JSON_ATTRIBUTE_TARGETTREE                                      "targetTree"

#define CONTENT_RULE_JSON_TYPE                                                         "Content"
#define CONTENT_RULE_JSON_ATTRIBUTE_CUSTOMCONTROL                                      "customControl"
#define CONTENT_RULE_JSON_ATTRIBUTE_SPECIFICATIONS                                     "specifications"

#define IMAGE_ID_OVERRIDE_JSON_TYPE                                                    "ImageId"
#define IMAGE_ID_OVERRIDE_JSON_ATTRIBUTE_IMAGEID                                       "imageIdExpression"

#define LABEL_OVERRIDE_JSON_TYPE                                                       "Label"
#define LABEL_OVERRIDE_JSON_ATTRIBUTE_LABEL                                            "label"
#define LABEL_OVERRIDE_JSON_ATTRIBUTE_DESCRIPTION                                      "description"

#define INSTANCE_LABEL_OVERRIDE_JSON_TYPE                                              "InstanceLabel"
#define INSTANCE_LABEL_OVERRIDE_JSON_ATTRIBUTE_CLASSNAME                               "className"
#define INSTANCE_LABEL_OVERRIDE_JSON_ATTRIBUTE_PROPERTYNAMES                           "properties"

#define STYLE_OVERRIDE_JSON_TYPE                                                       "Style"
#define STYLE_OVERRIDE_JSON_ATTRIBUTE_FORECOLOR                                        "foreColor"
#define STYLE_OVERRIDE_JSON_ATTRIBUTE_BACKCOLOR                                        "backColor"
#define STYLE_OVERRIDE_JSON_ATTRIBUTE_FONTSTYLE                                        "fontStyle"

#define GROUPING_RULE_JSON_NAME                                                        "GroupingRule"
#define GROUPING_RULE_JSON_TYPE                                                        "Grouping"
#define GROUPING_RULE_JSON_ATTRIBUTE_SCHEMANAME                                        "schemaName"
#define GROUPING_RULE_JSON_ATTRIBUTE_CLASSNAME                                         "className"
#define GROUPING_RULE_JSON_ATTRIBUTE_MENUCONDITION                                     "contextMenuCondition"
#define GROUPING_RULE_JSON_ATTRIBUTE_MENULABEL                                         "contextMenuLabel"
#define GROUPING_RULE_JSON_ATTRIBUTE_SETTINGSID                                        "settingsId"
#define GROUPING_RULE_JSON_ATTRIBUTE_GROUPS                                            "groups"

#define CHECKBOX_RULE_JSON_TYPE                                                        "CheckBox"
#define CHECKBOX_RULE_JSON_ATTRIBUTE_USEINVERSEDPROPERTYVALUE                          "useInversedPropertyValue"
#define CHECKBOX_RULE_JSON_ATTRIBUTE_DEFAULTVALUE                                      "defaultValue"
#define CHECKBOX_RULE_JSON_ATTRIBUTE_ISENABLED                                         "isEnabled"
#define CHECKBOX_RULE_JSON_ATTRIBUTE_PROPERTYNAME                                      "propertyName"


#define SAMEL_LABEL_INSTANCE_GROUP_JSON_TYPE                                           "SameLabelInstance"

#define DISPLAYRELATEDITEMS_SPECIFICATION_JSON_ATTRIBUTE_LOGICALCHILDREN               "logicalChildren"
#define DISPLAYRELATEDITEMS_SPECIFICATION_JSON_ATTRIBUTE_NESTINGDEPTH                  "nestingDepth"
#define DISPLAYRELATEDITEMS_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPCLASSES           "relationshipClasses"

#define CONTENTMODIEFIER_JSON_ATTRIBUTE_CALCULATEDPROPERTIES                           "calculatedProperties"
#define CONTENTMODIEFIER_JSON_ATTRIBUTE_CLASSNAME                                      "className"
#define CONTENTMODIEFIER_JSON_ATTRIBUTE_PROPERTYDISPLAYSPECIFICATIONS                  "propertyDisplaySpecifications"
#define CONTENTMODIEFIER_JSON_ATTRIBUTE_PROPERTYEDITORS                                "propertyEditors"
#define CONTENTMODIEFIER_JSON_ATTRIBUTE_RELATEDPROPERTIES                              "relatedProperties"
#define CONTENTMODIEFIER_JSON_ATTRIBUTE_SCHEMANAME                                     "schemaName"


#define CLASS_GROUP_JSON_TYPE                                                          "Class"
#define CLASS_GROUP_JSON_ATTRIBUTE_SCHEMANAME                                          "schemaName"
#define CLASS_GROUP_JSON_ATTRIBUTE_BASECLASSNAME                                       "baseClassName"

#define PROPERTY_GROUP_JSON_NAME                                                       "PropertyGroup"
#define PROPERTY_GROUP_JSON_TYPE                                                       "Property"

#define GROUP_JSON_ATTRIBUTE_IMAGEID                                                   "imageId"
#define GROUP_JSON_ATTRIBUTE_CREATEGROUPFORSINGLEITEM                                  "createGroupForSingleItem"
#define GROUP_JSON_ATTRIBUTE_CREATEGROUPFORUNSPECIFIEDVALUES                           "createGroupForUnspecifiedValues"
#define GROUP_JSON_ATTRIBUTE_MENULABEL                                                 "contextMenuLabel"
#define GROUP_JSON_ATTRIBUTE_DEFAULTLABEL                                              "defaultLabel"

#define PROPERTY_GROUP_JSON_ATTRIBUTE_GROUPINGVALUE                                    "groupingValue"
#define PROPERTY_GROUP_JSON_ATTRIBUTE_SORTINGVALUE                                     "sortingValue"
#define PROPERTY_GROUP_JSON_ATTRIBUTE_RANGES                                           "ranges"
#define PROPERTY_GROUP_JSON_ATTRIBUTE_VALUE_GROUPINGVALUE_PROPERTYVALUE                "propertyValue"
#define PROPERTY_GROUP_JSON_ATTRIBUTE_VALUE_GROUPINGVALUE_DISPLAYLABEL                 "displayLabel"

#define PROPERTY_RANGE_GROUP_JSON_NAME                                                 "PropertyRangeGroupSpecification"
#define PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_LABEL                                      "label"
#define PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_IMAGEID                                    "imageId"
#define PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_FROMVALUE                                  "fromValue"
#define PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_TOVALUE                                    "toValue"

#define LOCALIZATION_DEFINITION_JSON_ATTRIBUTE_ID                                      "id"
#define LOCALIZATION_DEFINITION_JSON_ATTRIBUTE_KEY                                     "key"
#define LOCALIZATION_DEFINITION_JSON_ATTRIBUTE_DEFAULTVALUE                            "defaultValue"

#define USER_SETTINGS_JSON_ATTRIBUTE_CATEGORY_LABEL                                    "categoryLabel"
#define USER_SETTINGS_JSON_ATTRIBUTE_NESTED_SETTINGS                                   "nestedSettings"
#define USER_SETTINGS_JSON_ATTRIBUTE_SETTINGS_ITEMS                                    "settingsItems"

#define USER_SETTINGS_ITEM_JSON_NAME                                                   "UserSettingsItem"
#define USER_SETTINGS_ITEM_JSON_ATTRIBUTE_ID                                           "id"
#define USER_SETTINGS_ITEM_JSON_ATTRIBUTE_LABEL                                        "label"
#define USER_SETTINGS_ITEM_JSON_ATTRIBUTE_OPTIONS                                      "options"
#define USER_SETTINGS_ITEM_JSON_ATTRIBUTE_DEFAULT_VALUE                                "defaultValue"

#define SORTING_RULE_JSON_TYPE                                                         "Sorting"
#define SORTING_RULE_JSON_ATTRIBUTE_SORTASCENDING                                      "sortAscending"
#define SORTING_RULE_JSON_ATTRIBUTE_DONOTSORT                                          "doNotSort"

//ChildNodeSpecifications
#define CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_ALWAYSRETURNSCHILDREN                  "alwaysReturnsChildren"
#define CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_HIDENODESINHIERARCHY                   "hideNodesInHierarchy"
#define CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_HIDEIFNOCHILDREN                       "hideIfNoChildren"
#define CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_EXTENDEDDATA                           "extendedData"
#define CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_RELATEDINSTANCES                       "relatedInstances"
#define CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_NESTEDRULES                            "nestedRules"

#define ALL_INSTANCE_NODES_SPECIFICATION_JSON_TYPE                                     "AllInstanceNodes"

#define ALL_RELATED_INSTANCE_NODES_SPECIFICATION_JSON_TYPE                             "AllRelatedInstanceNodes"

#define CUSTOM_NODE_SPECIFICATION_JSON_TYPE                                            "CustomNode"
#define CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_TYPE                                  "nodeType"
#define CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_LABEL                                 "label"
#define CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_DESCRIPTION                           "description"
#define CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_IMAGEID                               "imageId"

#define INSTANCE_NODES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_NAME                     "InstanceNodesOfSpecificClassesSpecification"
#define INSTANCE_NODES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_TYPE                     "InstanceNodesOfSpecificClasses"

#define RELATED_INSTANCE_NODES_SPECIFICATION_JSON_TYPE                                 "RelatedInstanceNodes"

#define SEARCH_RESULT_INSTANCE_NODES_SPECIFICATION_JSON_TYPE                           "SearchResultInstanceNodes"
#define SEARCH_RESULT_INSTANCE_NODES_SPECIFICATION_JSON_ATTRIBUTE_QUERIES              "queries"

#define SEARCH_QUERY_SPECIFICATION_JSON_NAME                                           "QuerySpecification"
#define SEARCH_QUERY_SPECIFICATION_JSON_ATTRIBUTE_SCHEMA_NAME                          "schemaName"
#define SEARCH_QUERY_SPECIFICATION_JSON_ATTRIBUTE_CLASS_NAME                           "className"

#define STRING_QUERY_SPECIFICATION_JSON_NAME                                           "StringQuerySpecification"
#define STRING_QUERY_SPECIFICATION_JSON_TYPE                                           "StringQuery"
#define STRING_QUERY_SPECIFICATION_JSON_ATTRIBUTE_QUERY                                "query"

#define ECPROPERTY_VALUE_QUERY_SPECIFICATION_JSON_NAME                                 "ECPropertyValueQuerySpecification"
#define ECPROPERTY_VALUE_QUERY_SPECIFICATION_JSON_TYPE                                 "ECPropertyQuery"
#define ECPROPERTY_VALUE_QUERY_SPECIFICATION_JSON_ATTRIBUTE_PARENT_PROPERTY_NAME       "parentPropertyName"

#define RELATED_INSTANCE_SPECIFICATION_JSON_NAME                                       "RelatedInstanceSpecification"
#define RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_CLASSNAME                        "className"
#define RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPNAME                 "relationshipName"
#define RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPDIRECTION            "relationshipDirection"
#define RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ALIAS                            "alias"
#define RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ISREQUIRED                       "isRequired"

//ContentSpecifications
#define CONTENT_SPECIFICATION_JSON_ATTRIBUTE_SHOWIMAGES                                "showImages"
#define CONTENT_SPECIFICATION_JSON_ATTRIBUTE_RELATEDPROPERTIESSPECIFICATION            "relatedPropertiesSpecification"
#define CONTENT_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIESDISPLAYSPECIFICATION            "propertiesDisplaySpecification"
#define CONTENT_SPECIFICATION_JSON_ATTRIBUTE_DISPLAYRELATEDITEMSSPECIFICATION          "displayRelatedItemsSpecification"
#define CONTENT_SPECIFICATION_JSON_ATTRIBUTE_CALCULATEDPROPERTIESSPECIFICATION         "calculatedPropertiesSpecification"
#define CONTENT_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYEDITORSSPECIFICATION              "propertyEditorsSpecification"
#define CONTENT_SPECIFICATION_JSON_ATTRIBUTE_RELATEDINSTANCESSPECIFICATION             "relatedInstancesSpecification"


#define RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_NESTEDRELATEDPROPERTIES        "nestedRelatedProperties"

#define CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_NAME                  "ContentInstancesOfSpecificClassesSpecification"
#define CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_TYPE                  "ContentInstancesOfSpecificClasses"


#define CONTENT_RELATED_INSTANCES_SPECIFICATION_JSON_TYPE                              "ContentRelatedInstances"
#define CONTENT_RELATED_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ISRECURSIVE             "isRecursive"

#define SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_TYPE                                "SelectedNodeInstances"
#define SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ACCEPTABLESCHEMANAME      "acceptableSchemaName"
#define SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ACCEPTABLECLASSNAMES      "acceptableClassNames"
#define SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ACCEPTABLEPOLYMORPHICALLY "acceptablePolymorphically"

#define PROPERTIES_DISPLAY_SPECIFICATION_JSON_NAME                                     "PropertiesDisplaySpecification"
#define PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_ISDISPLAYED                    "isDisplayed"
#define PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_CLASSNAME                      "className"
#define PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAMES                  "propertyNames"
#define PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_PRIORITY                       "priority"

#define CALCULATED_PROPERTIES_SPECIFICATION_JSON_NAME                                  "CalculatedPropertiesSpecification"
#define CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_LABEL                       "label"
#define CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PRIORITY                    "priority"
#define CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_VALUE                       "value"


// Content: Property Editors
#define PROPERTY_EDITORS_SPECIFICATION_JSON_NAME                                       "PropertyEditorsSpecification"
#define PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAME                     "propertyName"
#define PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_EDITORNAME                       "editorName"
#define PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_PARAMETERS                       "parameters"

#define PROPERTY_EDITOR_RANGE_PARAMETERS_JSON_TYPE                                     "Range"
#define PROPERTY_EDITOR_RANGE_PARAMETERS_JSON_ATTRIBUTE_MINIMUM                        "min"
#define PROPERTY_EDITOR_RANGE_PARAMETERS_JSON_ATTRIBUTE_MAXIMUM                        "max"

#define PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_NAME                                    "PropertyEditorSliderParameters"
#define PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_TYPE                                    "Slider"
#define PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_MINIMUM                       "min"
#define PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_MAXIMUM                       "max"
#define PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_INTERVALS                     "intervalsCount"
#define PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_VALUEFACTOR                   "valueFactor"
#define PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_VERTICAL                      "isVertical"

#define PROPERTY_EDITOR_JSON_PARAMETERS_JSON_TYPE                                      "Json"
#define PROPERTY_EDITOR_JSON_PARAMETERS_JSON_ATTRIBUTE_JSON                            "json"

#define PROPERTY_EDITOR_MULTILINE_PARAMETERS_JSON_TYPE                                 "Multiline"
#define PROPERTY_EDITOR_MULTILINE_PARAMETERS_JSON_ATTRIBUTE_HEIGHT                     "height"