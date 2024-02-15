/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#define INVALID_JSON                                                                    "Invalid %s json: Attribute %s is not specified or has incorrect value."

#define COMMON_JSON_ATTRIBUTE_RULETYPE                                                  "ruleType"
#define COMMON_JSON_ATTRIBUTE_SPECTYPE                                                  "specType"
#define COMMON_JSON_ATTRIBUTE_PARAMSTYPE                                                "paramsType"
#define COMMON_JSON_ATTRIBUTE_PRIORITY                                                  "priority"
#define COMMON_JSON_ATTRIBUTE_GROUPBYCLASS                                              "groupByClass"
#define COMMON_JSON_ATTRIBUTE_GROUPBYLABEL                                              "groupByLabel"
#define COMMON_JSON_ATTRIBUTE_SUPPORTEDSCHEMAS                                          "supportedSchemas"
#define COMMON_JSON_ATTRIBUTE_SKIPRELATEDLEVEL                                          "skipRelatedLevel"
#define COMMON_JSON_ATTRIBUTE_INSTANCEFILTER                                            "instanceFilter"
#define COMMON_JSON_ATTRIBUTE_REQUIREDDIRECTION                                         "requiredDirection"
#define COMMON_JSON_ATTRIBUTE_RELATIONSHIPS                                             "relationships"
#define COMMON_JSON_ATTRIBUTE_RELATEDCLASSES                                            "relatedClasses"
#define COMMON_JSON_ATTRIBUTE_CLASSES                                                   "classes"
#define COMMON_JSON_ATTRIBUTE_EXCLUDEDCLASSES                                           "excludedClasses"
#define COMMON_JSON_ATTRIBUTE_CLASS                                                     "class"
#define COMMON_JSON_ATTRIBUTE_PROPERTYNAMES                                             "propertyNames"
#define COMMON_JSON_ATTRIBUTE_PROPERTYNAME                                              "propertyName"
#define COMMON_JSON_ATTRIBUTE_ONLYIFNOTHANDLED                                          "onlyIfNotHandled"
#define COMMON_JSON_ATTRIBUTE_AREPOLYMORPHIC                                            "arePolymorphic"
#define COMMON_JSON_ATTRIBUTE_ISPOLYMORPHIC                                             "isPolymorphic"
#define COMMON_JSON_ATTRIBUTE_STOPFURTHERPROCESSING                                     "stopFurtherProcessing"
#define COMMON_JSON_ATTRIBUTE_RELATIONSHIPMEANING                                       "relationshipMeaning"
#define COMMON_JSON_ATTRIBUTE_RELATIONSHIPPATHS                                         "relationshipPaths"
#define COMMON_JSON_ATTRIBUTE_AUTOEXPAND                                                "autoExpand"
#define COMMON_JSON_ATTRIBUTE_REQUIREDSCHEMAS                                           "requiredSchemas"

#define SCHEMAS_SPECIFICATION_SCHEMANAMES                                               "schemaNames"
#define SCHEMAS_SPECIFICATION_ISEXCLUDE                                                 "isExclude"

#define SCHEMA_CLASS_SPECIFICATION_SCHEMANAME                                           "schemaName"
#define SINGLE_SCHEMA_CLASS_SPECIFICATION_CLASSNAME                                     "className"
#define MULTI_SCHEMA_CLASSES_SPECIFICATION_CLASSNAMES                                   "classNames"

#define CHILD_NODE_RULE_JSON_ATTRIBUTE_SUBCONDITIONS                                    "subConditions"
#define CHILD_NODE_RULE_JSON_ATTRIBUTE_SPECIFICATIONS                                   "specifications"
#define CHILD_NODE_RULE_JSON_ATTRIBUTE_CUSTOMIZATIONRULES                               "customizationRules"

#define PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULESETID                                  "id"
#define PRESENTATION_RULE_SET_JSON_ATTRIBUTE_SUPPORTEDSCHEMAS                           "supportedSchemas"
#define PRESENTATION_RULE_SET_JSON_ATTRIBUTE_SUPPLEMENTATIONINFO                        "supplementationInfo"
#define PRESENTATION_RULE_SET_JSON_ATTRIBUTE_SUPPLEMENTATIONINFO_PURPOSE                "supplementationPurpose"
#define PRESENTATION_RULE_SET_JSON_ATTRIBUTE_USERSETTINGS                               "vars"
#define PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULES                                      "rules"
#define PRESENTATION_RULE_SET_JSON_ATTRIBUTE_SCHEMA_VERSION                             "schemaVersion"
#define PRESENTATION_RULE_SET_JSON_ATTRIBUTE_RULESET_VERSION                            "version"

#define PRESENTATION_RULE_JSON_ATTRIBUTE_CONDITION                                      "condition"

#define ROOT_NODE_RULE_JSON_TYPE                                                        "RootNodes"

#define SUB_CONDITION_JSON_ATTRIBUTE_SUBCONDITIONS                                      "subConditions"
#define SUB_CONDITION_JSON_ATTRIBUTE_SPECIFICATIONS                                     "specifications"

#define CHILD_NODE_RULE_JSON_TYPE                                                       "ChildNodes"

#define CONTENT_RULE_JSON_TYPE                                                          "Content"
#define CONTENT_RULE_JSON_ATTRIBUTE_SPECIFICATIONS                                      "specifications"

#define IMAGE_ID_OVERRIDE_JSON_TYPE                                                     "ImageIdOverride"
#define IMAGE_ID_OVERRIDE_JSON_ATTRIBUTE_IMAGEID                                        "imageIdExpression"

#define LABEL_OVERRIDE_JSON_TYPE                                                        "LabelOverride"
#define LABEL_OVERRIDE_JSON_ATTRIBUTE_LABEL                                             "label"
#define LABEL_OVERRIDE_JSON_ATTRIBUTE_DESCRIPTION                                       "description"

#define INSTANCE_LABEL_OVERRIDE_JSON_TYPE                                               "InstanceLabelOverride"
#define INSTANCE_LABEL_OVERRIDE_JSON_ATTRIBUTE_CLASS                                    "class"
#define INSTANCE_LABEL_OVERRIDE_JSON_ATTRIBUTE_VALUES                                   "values"
#define INSTANCE_LABEL_OVERRIDE_VALUE_SPECIFICATION_BASE_JSON_ATTRIBUTE_TYPE            "specType"
#define INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_JSON_TYPE                 "Composite"
#define INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_JSON_ATTRIBUTE_SEPARATOR  "separator"
#define INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PARTS      "parts"
#define INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_SPEC  "spec"
#define INSTANCE_LABEL_OVERRIDE_COMPOSITE_VALUE_SPECIFICATION_PART_JSON_ATTRIBUTE_ISREQUIRED "isRequired"
#define INSTANCE_LABEL_OVERRIDE_PROPERTY_VALUE_SPECIFICATION_JSON_TYPE                  "Property"
#define INSTANCE_LABEL_OVERRIDE_PROPERTY_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAME "propertyName"
#define INSTANCE_LABEL_OVERRIDE_PROPERTY_VALUE_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYSOURCE "propertySource"
#define INSTANCE_LABEL_OVERRIDE_CLASSNAME_VALUE_SPECIFICATION_JSON_TYPE                 "ClassName"
#define INSTANCE_LABEL_OVERRIDE_CLASSNAME_VALUE_SPECIFICATION_JSON_ATTRIBUTE_FULL       "full"
#define INSTANCE_LABEL_OVERRIDE_CLASSLABEL_VALUE_SPECIFICATION_JSON_TYPE                "ClassLabel"
#define INSTANCE_LABEL_OVERRIDE_BRIEFCASEID_VALUE_SPECIFICATION_JSON_TYPE               "BriefcaseId"
#define INSTANCE_LABEL_OVERRIDE_LOCALID_VALUE_SPECIFICATION_JSON_TYPE                   "LocalId"
#define INSTANCE_LABEL_OVERRIDE_STRING_VALUE_SPECIFICATION_JSON_TYPE                    "String"
#define INSTANCE_LABEL_OVERRIDE_STRING_VALUE_SPECIFICATION_JSON_ATTRIBUTE_VALUE         "value"
#define INSTANCE_LABEL_OVERRIDE_RELATED_INSTANCE_LABEL_SPECIFICATION_JSON_TYPE          "RelatedInstanceLabel"
#define INSTANCE_LABEL_OVERRIDE_RELATED_INSTANCE_LABEL_SPECIFICATION_JSON_ATTRIBUTE_PATHTORELATEDINSTANCE "pathToRelatedInstance"

#define STYLE_OVERRIDE_JSON_TYPE                                                        "StyleOverride"
#define STYLE_OVERRIDE_JSON_ATTRIBUTE_FORECOLOR                                         "foreColor"
#define STYLE_OVERRIDE_JSON_ATTRIBUTE_BACKCOLOR                                         "backColor"
#define STYLE_OVERRIDE_JSON_ATTRIBUTE_FONTSTYLE                                         "fontStyle"

#define GROUPING_RULE_JSON_TYPE                                                         "Grouping"
#define GROUPING_RULE_JSON_ATTRIBUTE_GROUPS                                             "groups"

#define EXTENDED_DATA_RULE_JSON_TYPE                                                    "ExtendedData"
#define EXTENDED_DATA_RULE_JSON_ATTRIBUTE_ITEMS                                         "items"

#define NODE_ARTIFACTS_RULE_JSON_TYPE                                                   "NodeArtifacts"
#define NODE_ARTIFACTS_RULE_JSON_ATTRIBUTE_ITEMS                                        "items"

#define CHECKBOX_RULE_JSON_TYPE                                                         "CheckBox"
#define CHECKBOX_RULE_JSON_ATTRIBUTE_USEINVERSEDPROPERTYVALUE                           "useInversedPropertyValue"
#define CHECKBOX_RULE_JSON_ATTRIBUTE_DEFAULTVALUE                                       "defaultValue"
#define CHECKBOX_RULE_JSON_ATTRIBUTE_ISENABLED                                          "isEnabled"
#define CHECKBOX_RULE_JSON_ATTRIBUTE_PROPERTYNAME                                       "propertyName"

#define SAME_LABEL_INSTANCE_GROUP_JSON_TYPE                                             "SameLabelInstance"
#define SAME_LABEL_INSTANCE_GROUP_JSON_ATTRIBUTE_APPLICATIONSTAGE                       "applicationStage"
#define SAME_LABEL_INSTANCE_GROUP_JSON_ATTRIBUTE_APPLICATIONSTAGE_VALUES_QUERY          "Query"
#define SAME_LABEL_INSTANCE_GROUP_JSON_ATTRIBUTE_APPLICATIONSTAGE_VALUES_POSTPROCESS    "PostProcess"

#define CONTENTMODIFIER_RULE_JSON_TYPE                                                  "ContentModifier"
#define CONTENTMODIFIER_JSON_ATTRIBUTE_CALCULATEDPROPERTIES                             "calculatedProperties"
#define CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYDISPLAYSPECIFICATIONS                    "propertiesDisplay"
#define CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYEDITORS                                  "propertyEditors"
#define CONTENTMODIFIER_JSON_ATTRIBUTE_RELATEDPROPERTIES                                "relatedProperties"
#define CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYCATEGORIES                               "propertyCategories"
#define CONTENTMODIFIER_JSON_ATTRIBUTE_PROPERTYOVERRIDES                                "propertyOverrides"
#define CONTENTMODIFIER_JSON_ATTRIBUTE_APPLYONNESTEDCONTENT                             "applyOnNestedContent"

#define DEFAULT_PROPERTY_CATEGORY_OVERRIDE_JSON_TYPE                                    "DefaultPropertyCategoryOverride"
#define DEFAULT_PROPERTY_CATEGORY_OVERRIDE_JSON_ATTRIBUTE_SPECIFICATION                 "specification"

#define CLASS_GROUP_JSON_TYPE                                                           "Class"
#define CLASS_GROUP_JSON_ATTRIBUTE_BASECLASS                                            "baseClass"

#define PROPERTY_GROUP_JSON_TYPE                                                        "Property"

#define GROUP_JSON_ATTRIBUTE_IMAGEID                                                    "imageId"
#define GROUP_JSON_ATTRIBUTE_CREATEGROUPFORSINGLEITEM                                   "createGroupForSingleItem"
#define GROUP_JSON_ATTRIBUTE_CREATEGROUPFORUNSPECIFIEDVALUES                            "createGroupForUnspecifiedValues"
#define GROUP_JSON_ATTRIBUTE_DEFAULTLABEL                                               "defaultLabel"

#define PROPERTY_GROUP_JSON_ATTRIBUTE_GROUPINGVALUE                                     "groupingValue"
#define PROPERTY_GROUP_JSON_ATTRIBUTE_SORTINGVALUE                                      "sortingValue"
#define PROPERTY_GROUP_JSON_ATTRIBUTE_RANGES                                            "ranges"
#define PROPERTY_GROUP_JSON_ATTRIBUTE_VALUE_GROUPINGVALUE_PROPERTYVALUE                 "propertyValue"
#define PROPERTY_GROUP_JSON_ATTRIBUTE_VALUE_GROUPINGVALUE_DISPLAYLABEL                  "displayLabel"

#define PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_LABEL                                       "label"
#define PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_IMAGEID                                     "imageId"
#define PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_FROMVALUE                                   "fromValue"
#define PROPERTY_RANGE_GROUP_JSON_ATTRIBUTE_TOVALUE                                     "toValue"

#define USER_SETTINGS_JSON_TYPE                                                         "vars"
#define USER_SETTINGS_JSON_ATTRIBUTE_CATEGORY_LABEL                                     "label"
#define USER_SETTINGS_JSON_ATTRIBUTE_NESTED_SETTINGS                                    "nestedGroups"
#define USER_SETTINGS_JSON_ATTRIBUTE_SETTINGS_ITEMS                                     "vars"

#define USER_SETTINGS_ITEM_JSON_ATTRIBUTE_ID                                            "id"
#define USER_SETTINGS_ITEM_JSON_ATTRIBUTE_LABEL                                         "label"
#define USER_SETTINGS_ITEM_JSON_ATTRIBUTE_OPTIONS                                       "type"
#define USER_SETTINGS_ITEM_JSON_ATTRIBUTE_DEFAULT_VALUE                                 "defaultValue"

#define SORTING_RULE_PROPERTYSORTING_JSON_TYPE                                          "PropertySorting"
#define SORTING_RULE_DISABLEDSORTING_JSON_TYPE                                          "DisabledSorting"
#define SORTING_RULE_JSON_ATTRIBUTE_SORTASCENDING                                       "sortAscending"

#define RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_CLASS                             "class"
#define RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP                      "relationship"
#define RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPDIRECTION             "relationshipDirection"
#define RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPATH                  "relationshipPath"
#define RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_TARGETINSTANCES                   "targetInstances"
#define RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ALIAS                             "alias"
#define RELATED_INSTANCE_SPECIFICATION_JSON_ATTRIBUTE_ISREQUIRED                        "isRequired"
#define RELATED_INSTANCE_BY_IDS_SPECIFICATION_JSON_ATTRIBUTE_CLASS                      "class"
#define RELATED_INSTANCE_BY_IDS_SPECIFICATION_JSON_ATTRIBUTE_INSTANCEIDS                "instanceIds"

#define RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIP                     "relationship"
#define RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_DIRECTION                        "direction"
#define RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_TARGETCLASS                      "targetClass"
#define RELATIONSHIP_STEP_SPECIFICATION_JSON_ATTRIBUTE_COUNT                            "count"

#define REQUIRED_SCHEMA_SPECIFICATION_JSON_ATTRIBUTE_NAME                               "name"
#define REQUIRED_SCHEMA_SPECIFICATION_JSON_ATTRIBUTE_MINVERSION                         "minVersion"
#define REQUIRED_SCHEMA_SPECIFICATION_JSON_ATTRIBUTE_MAXVERSION                         "maxVersion"

//ChildNodeSpecifications
#define CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_DONOTSORT                               "doNotSort"
#define CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_ALWAYSRETURNSCHILDREN                   "alwaysReturnsChildren"
#define CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_HASCHILDREN                             "hasChildren"
#define CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_HIDENODESINHIERARCHY                    "hideNodesInHierarchy"
#define CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_HIDEIFNOCHILDREN                        "hideIfNoChildren"
#define CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_HIDEEXPRESSION                          "hideExpression"
#define CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_RELATEDINSTANCES                        "relatedInstances"
#define CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_NESTEDRULES                             "nestedRules"
#define CHILD_NODE_SPECIFICATION_JSON_ATTRIBUTE_SUPPRESSSIMILARANCESTORSCHECK           "suppressSimilarAncestorsCheck"

#define ALL_INSTANCE_NODES_SPECIFICATION_JSON_TYPE                                      "AllInstanceNodes"

#define ALL_RELATED_INSTANCE_NODES_SPECIFICATION_JSON_TYPE                              "AllRelatedInstanceNodes"

#define CUSTOM_NODE_SPECIFICATION_JSON_TYPE                                             "CustomNode"
#define CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_TYPE                                   "type"
#define CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_LABEL                                  "label"
#define CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_DESCRIPTION                            "description"
#define CUSTOM_NODE_SPECIFICATION_JSON_ATTRIBUTE_IMAGEID                                "imageId"

#define INSTANCE_NODES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_TYPE                      "InstanceNodesOfSpecificClasses"

#define RELATED_INSTANCE_NODES_SPECIFICATION_JSON_TYPE                                  "RelatedInstanceNodes"

#define SEARCH_RESULT_INSTANCE_NODES_SPECIFICATION_JSON_TYPE                            "CustomQueryInstanceNodes"
#define SEARCH_RESULT_INSTANCE_NODES_SPECIFICATION_JSON_ATTRIBUTE_QUERIES               "queries"

#define SEARCH_QUERY_SPECIFICATION_JSON_ATTRIBUTE_CLASS                                 "class"

#define STRING_QUERY_SPECIFICATION_JSON_TYPE                                            "String"
#define STRING_QUERY_SPECIFICATION_JSON_ATTRIBUTE_QUERY                                 "query"

#define ECPROPERTY_VALUE_QUERY_SPECIFICATION_JSON_TYPE                                  "ECPropertyValue"
#define ECPROPERTY_VALUE_QUERY_SPECIFICATION_JSON_ATTRIBUTE_PARENT_PROPERTY_NAME        "parentPropertyName"

//ContentSpecifications
#define CONTENT_SPECIFICATION_JSON_ATTRIBUTE_SHOWIMAGES                                 "showImages"
#define CONTENT_SPECIFICATION_JSON_ATTRIBUTE_RELATEDINSTANCESSPECIFICATION              "relatedInstances"

#define RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIESSOURCE                "propertiesSource"
#define RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTIES                      "properties"
#define RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAMES                   "propertyNames"
#define RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_NESTEDRELATEDPROPERTIES         "nestedRelatedProperties"
#define RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_HANDLETARGETCLASSPOLYMORPHICALLY "handleTargetClassPolymorphically"
#define RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_SKIPIFDUPLICATE                 "skipIfDuplicate"
#define RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_RELATIONSHIPPROPERTIES          "relationshipProperties"
#define RELATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_FORCECREATERELATIONSHIPCATEGORY "forceCreateRelationshipCategory"

#define CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_TYPE                   "ContentInstancesOfSpecificClasses"
#define CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_ATTRIBUTE_HANDLEINSTANCESPOLYMORPHICALLY "handleInstancesPolymorphically"
#define CONTENT_INSTANCES_OF_SPECIFIC_CLASSES_SPECIFICATION_JSON_ATTRIBUTE_HANDLEPROPERTIESPOLYMORPHICALLY "handlePropertiesPolymorphically"

#define CONTENT_RELATED_INSTANCES_SPECIFICATION_JSON_TYPE                               "ContentRelatedInstances"
#define CONTENT_RELATED_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ISRECURSIVE              "isRecursive"

#define SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_TYPE                                 "SelectedNodeInstances"
#define SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ACCEPTABLESCHEMANAME       "acceptableSchemaName"
#define SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ACCEPTABLECLASSNAMES       "acceptableClassNames"
#define SELECTED_NODE_INSTANCES_SPECIFICATION_JSON_ATTRIBUTE_ACCEPTABLEPOLYMORPHICALLY  "acceptablePolymorphically"

#define PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_ISDISPLAYED                     "isDisplayed"
#define PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_CLASSNAME                       "className"
#define PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAMES                   "propertyNames"
#define PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_PRIORITY                        "priority"
#define PROPERTIES_DISPLAY_SPECIFICATION_JSON_ATTRIBUTE_DONOTHIDEOTHERPROPERTIESONDISPLAYOVERRIDE "doNotHideOtherPropertiesOnDisplayOverride"

#define CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_LABEL                        "label"
#define CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_PRIORITY                     "priority"
#define CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_VALUE                        "value"
#define CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_RENDERER                     "renderer"
#define CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_EDITOR                       "editor"
#define CALCULATED_PROPERTIES_SPECIFICATION_JSON_ATTRIBUTE_CATEGORYID                   "categoryId"

#define PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_NAME                                      "name"
#define PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_OVERRIDESPRIORITY                         "overridesPriority"
#define PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_LABELOVERRIDE                             "labelOverride"
#define PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_ISDISPLAYED                               "isDisplayed"
#define PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_RENDERER                                  "renderer"
#define PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_EDITOR                                    "editor"
#define PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_CATEGORYID                                "categoryId"
#define PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_DONOTHIDEOTHERPROPERTIESONDISPLAYOVERRIDE "doNotHideOtherPropertiesOnDisplayOverride"
#define PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_ISREADONLY                                "isReadOnly"
#define PROPERTY_SPECIFICATION_JSON_ATTRIBUTE_PRIORITY                                  "priority"

#define PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_ID                               "id"
#define PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_PARENTID                         "parentId"
#define PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_LABEL                            "label"
#define PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_DESCRIPTION                      "description"
#define PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_PRIORITY                         "priority"
#define PROPERTY_CATEGORY_SPECIFICATION_JSON_ATTRIBUTE_AUTOEXPAND                       "autoExpand"

#define PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_JSON_ATTRIBUTE_TYPE                  "type"
#define PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_JSON_ATTRIBUTE_CATEGORYID            "categoryId"
#define PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_JSON_ATTRIBUTE_CREATECLASSCATEGORY   "createClassCategory"
#define PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_TYPE_ROOT                            "Root"
#define PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_TYPE_DEFAULTPARENT                   "DefaultParent"
#define PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_TYPE_ID                              "Id"
#define PROPERTY_CATEGORY_IDENTIFIER_SPECIFICATION_TYPE_NONE                            "None"

// Content: Property Renderers
#define PROPERTY_RENDERERS_SPECIFICATION_JSON_ATTRIBUTE_RENDERERNAME                    "rendererName"

// Content: Property Editors
#define PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_PROPERTYNAME                      "propertyName"
#define PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_EDITORNAME                        "editorName"
#define PROPERTY_EDITORS_SPECIFICATION_JSON_ATTRIBUTE_PARAMETERS                        "parameters"

#define PROPERTY_EDITOR_RANGE_PARAMETERS_JSON_TYPE                                      "Range"
#define PROPERTY_EDITOR_RANGE_PARAMETERS_JSON_ATTRIBUTE_MINIMUM                         "min"
#define PROPERTY_EDITOR_RANGE_PARAMETERS_JSON_ATTRIBUTE_MAXIMUM                         "max"

#define PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_TYPE                                     "Slider"
#define PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_MINIMUM                        "min"
#define PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_MAXIMUM                        "max"
#define PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_INTERVALS                      "intervalsCount"
#define PROPERTY_EDITOR_SLIDER_PARAMETERS_JSON_ATTRIBUTE_VERTICAL                       "isVertical"

#define PROPERTY_EDITOR_JSON_PARAMETERS_JSON_TYPE                                       "Json"
#define PROPERTY_EDITOR_JSON_PARAMETERS_JSON_ATTRIBUTE_JSON                             "json"

#define PROPERTY_EDITOR_MULTILINE_PARAMETERS_JSON_TYPE                                  "Multiline"
#define PROPERTY_EDITOR_MULTILINE_PARAMETERS_JSON_ATTRIBUTE_HEIGHT                      "height"
