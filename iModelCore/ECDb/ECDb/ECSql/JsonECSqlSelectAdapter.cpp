/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/JsonECSqlSelectAdapter.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include <BeJsonCpp/BeJsonUtilities.h>

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool IsInstanceIdProperty(Utf8StringCR propertyName)
    {
    return (0 == propertyName.CompareTo("ECInstanceId") || 0 == propertyName.CompareTo("SourceECInstanceId") || 
        0 == propertyName.CompareTo("TargetECInstanceId"));
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonECSqlSelectAdapter::PropertyTree::AddInlinedStructNodes(PropertyTreeNodeR parentNode, IECSqlValue const& ecSqlValue, int instanceIndex)
    {
    BeAssert(&parentNode == &m_rootNode); // Inlined structs should only be specified at the root (or top level of a ECSQL query). 
    ECSqlColumnInfoCR columnInfo = ecSqlValue.GetColumnInfo();
    ECSqlPropertyPathCR propertyPath = columnInfo.GetPropertyPath();
    ECN::ECClassCR rootClass = columnInfo.GetRootClass();

    // Visit all the property path entries from the root struct property to the leaf primitive property
    size_t pathLength = propertyPath.Size();
    BeAssert(pathLength > 1 && "The method should only be called for inlined structs");
    PropertyTreeNodeP currentNode = &parentNode;
    for (size_t ii = 0; ii < pathLength; ii++)
        {
        ECPropertyCP ecProperty = propertyPath.At(ii).GetProperty();
        BeAssert(ecProperty != nullptr && "With the current ECSQL implementation, an in-lined struct member cannot have array entries (with NULL properties) in it's path!!!");
        PropertyTreeNodeByName& childNodes = currentNode->m_childNodes;
        PropertyTreeNodeByName::iterator it = childNodes.find(ecProperty->GetName());
        if (it != childNodes.end())
            currentNode = it->second;
        else
            {
            ECClassCR ecClass = (ii == 0) ? rootClass : ecProperty->GetClass();
            childNodes[ecProperty->GetName()] = new PropertyTreeNode(ecProperty, &ecClass, instanceIndex);
            currentNode = childNodes[ecProperty->GetName()];
            }
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP JsonECSqlSelectAdapter::GetClassFromStructOrStructArray(ECPropertyCR ecProperty)
    {
    if (ecProperty.GetIsStruct())
        {
        StructECPropertyCP structProperty = ecProperty.GetAsStructProperty();
        return &(structProperty->GetType());
        }
    else if (ecProperty.GetIsArray())
        {
        ArrayECPropertyCP arrayProperty = ecProperty.GetAsArrayProperty();
        BeAssert(arrayProperty != nullptr && arrayProperty->GetKind() == ARRAYKIND_Struct);
        return arrayProperty->GetStructElementType();
        }

    /* else */
    BeAssert(false && "Properyt has to be a struct or a struct array");
    return nullptr;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonECSqlSelectAdapter::PropertyTree::AddChildNodes(PropertyTreeNodeR parentNode, bvector<ECClassCP>& rootClasses, IECSqlValue const& ecsqlValue)
    {
    bool isRootReader = (&parentNode == &m_rootNode);

    ECSqlColumnInfoCR columnInfo = ecsqlValue.GetColumnInfo();
    ECPropertyCP ecProperty = columnInfo.GetProperty();
    BeAssert(ecProperty != nullptr && "This iteration cannot be on an array reader, and the column cannot represent an array value");

    // Skip instance id node
    if (isRootReader && IsInstanceIdProperty(ecProperty->GetName()))
        return;

    // If reading the root (top level) columns, determine the index of the instance that the property belongs in
    int instanceIndex = parentNode.m_instanceIndex;
    if (isRootReader)
        {
        ECN::ECClassCR rootClass = columnInfo.GetRootClass();
        bvector<ECClassCP>::const_iterator iter = std::find(rootClasses.begin(), rootClasses.end(), &rootClass);
        if (iter == rootClasses.end())
            {
            // Setup a new instance (index)
            instanceIndex = (int)rootClasses.size();
            rootClasses.push_back(&rootClass);
            }
        else
            {
            // Use an existing instance index
            instanceIndex = (int)(iter - rootClasses.begin());
            }
        }
    BeAssert(instanceIndex >= 0);

    // Special processing of inline structs - visit the entire property path to hierarchically add nodes
    bool isInlinedStructMember = (isRootReader && ecProperty->GetIsStruct() && (columnInfo.GetPropertyPath().Size() > 1));
    if (isInlinedStructMember)
        {
        AddInlinedStructNodes(parentNode, ecsqlValue, instanceIndex);
        return;
        }

    // Add node for the property
    PropertyTreeNodeByName& childNodes = parentNode.m_childNodes;
    if (childNodes.find(ecProperty->GetName()) == childNodes.end())
        {
        ECPropertyCP ecProperty = columnInfo.GetProperty();
        BeAssert(ecProperty != nullptr);
        ECClassCP ecClass;
        if (parentNode.m_property == nullptr) // i.e., IsRootReader
            ecClass = &(columnInfo.GetRootClass());
        else
            ecClass = GetClassFromStructOrStructArray(*(parentNode.m_property));
        childNodes[ecProperty->GetName()] = new PropertyTreeNode(ecProperty, ecClass, instanceIndex);
        }

    // Process nested children
    IECSqlStructValue const* structValue = nullptr;
    if (ecProperty->GetIsStruct())
        {
        bool isInlinedStructMember = (isRootReader && (columnInfo.GetPropertyPath().Size() > 1));
        if (!isInlinedStructMember)
            structValue = &ecsqlValue.GetStruct();
        }
    else if (ecProperty->GetIsArray())
        {
        ArrayECPropertyCP arrayLeafProperty = ecProperty->GetAsArrayProperty();
        BeAssert(arrayLeafProperty != nullptr);
        if (arrayLeafProperty->GetKind() == ARRAYKIND_Struct)
            {
            IECSqlArrayValue const& arrayValue = ecsqlValue.GetArray();
            auto arrayIt = arrayValue.begin();
            if (arrayIt != arrayValue.end())
                {
                IECSqlValue const* arrayElementValue = *arrayIt;
                structValue = &arrayElementValue->GetStruct();
                }
            }
        }

    if (structValue != nullptr)
        {
        //TODO: root class handling should not be necessary for struct members, right? Krischan.
        bvector<ECClassCP> structRootClasses;
        int memberCount = structValue->GetMemberCount();
        for (int i = 0; i < memberCount; i++)
            AddChildNodes(*childNodes[ecProperty->GetName()], structRootClasses, structValue->GetValue(i));
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
JsonECSqlSelectAdapter::FormatOptions::FormatOptions 
    (
    ECValueFormat format /* = ECValueFormat::FormattedStrings */,
    ECPropertyFormatterP propertyFormatter /* = nullptr */
    ) : m_format(format), m_propertyFormatter(propertyFormatter) 
    {
    if (m_format == ECValueFormat::FormattedStrings && m_propertyFormatter.IsNull())
        m_propertyFormatter = ECPropertyFormatter::Create();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
JsonECSqlSelectAdapter::JsonECSqlSelectAdapter 
(
ECSqlStatementCR ecsqlStatement, 
FormatOptions formatOptions /* = FormatOptions (ECValueFormat::FormattedStrings) */
) : m_ecsqlStatement(ecsqlStatement), m_formatOptions(formatOptions)
    {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::GetIntegerValue(int& value, IECInstanceCR instance, Utf8CP propertyName)
    {
    ECValue v;
    instance.GetValue(v, propertyName);
    if (v.IsNull())
        return false;
    value = v.GetInteger();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::GetStringValue(Utf8String& value, IECInstanceCR instance, Utf8CP propertyName)
    {
    ECValue v;
    instance.GetValue(v, propertyName);
    if (v.IsNull())
        return false;
    value = v.GetUtf8CP();
    return true;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::GetBooleanValue(bool& value, IECInstanceCR instance, Utf8CP propertyName)
    {
    ECValue v;
    instance.GetValue(v, propertyName);
    if (v.IsNull())
        return false;
    value = v.GetBoolean();
    return true;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::GetPriorityFromCustomAttribute(int& priority, IECInstancePtr priorityCA)
    {
    if (priorityCA.IsNull())
        return false;
    return GetIntegerValue(priority, *priorityCA, "Priority");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::GetPriorityFromProperty(int& priority, ECPropertyCR ecProperty)
    {
    IECInstancePtr priorityCA = ecProperty.GetCustomAttribute("PropertyPriority");
    return GetPriorityFromCustomAttribute(priority, priorityCA);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::PrioritySortPredicate(const IECInstancePtr& priorityCA1, const IECInstancePtr& priorityCA2)
    {
    if (priorityCA1 == priorityCA2)
        return false;

    int priority1 = INT_MIN;
    GetPriorityFromCustomAttribute(priority1, priorityCA1);

    int priority2 = INT_MIN;
    GetPriorityFromCustomAttribute(priority2, priorityCA2);

    return priority1 > priority2;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::PropertySortPredicate(PropertyTreeNodeCP propertyNode1, PropertyTreeNodeCP propertyNode2)
    {
    /* Return true if property1 is before property2. False otherwise */

    // Sort by priority
    int priority1 = INT_MIN;
    GetPriorityFromProperty(priority1, *propertyNode1->m_property);
    int priority2 = INT_MIN;
    GetPriorityFromProperty(priority2, *propertyNode2->m_property);
    if (priority1 != priority2)
        return priority1 > priority2; // Return true if property1 has higher priority than priority2

    // Sort alphabetically by display label
    Utf8StringCR displayLabel1 = propertyNode1->m_property->GetDisplayLabel();
    Utf8StringCR displayLabel2 = propertyNode2->m_property->GetDisplayLabel();
    return displayLabel1.CompareTo(displayLabel2) < 0; // Return true if property1 is alphabetically before property2
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   08/12
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonECSqlSelectAdapter::GetRowDisplayInfo(JsonValueR rowDisplayInfo) const
    {
    PropertyTree propertyTree;
    const int columnCount = m_ecsqlStatement.GetColumnCount();
    bvector<ECClassCP> rootClasses; // Root classes found in order of their first appearance in a column
    for (int i = 0; i < columnCount; i++)
        {
        propertyTree.AddChildNodes(propertyTree.m_rootNode, rootClasses, m_ecsqlStatement.GetValue(i));
        }

    JsonFromPropertyTree(rowDisplayInfo, propertyTree);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonECSqlSelectAdapter::JsonFromPropertyTree(JsonValueR jsonValue, PropertyTreeCR propertyTree) const
    {
    // TODO: Need a hierarchical visitor pattern on the tree
    JsonFromPropertyRecursive(jsonValue, propertyTree.m_rootNode);

    // TODO: Wish we had the unordered_set here. 
    bset<ECClassCP> processedECClasses;
    JsonFromClassesRecursive(jsonValue["Classes"], processedECClasses, propertyTree.m_rootNode);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonECSqlSelectAdapter::JsonFromClassesRecursive(JsonValueR jsonValue, bset<ECClassCP>& classes, PropertyTreeNodeCR propertyTreeNode) const
    {
    ECClassCP ecClass = propertyTreeNode.m_class;
    if (ecClass != nullptr && classes.end() == std::find(classes.begin(), classes.end(), ecClass))
        {
        classes.insert(ecClass);
        JsonFromClass(jsonValue, *ecClass);
        }

    for (PropertyTreeNodeByName::const_iterator nodeIter = propertyTreeNode.m_childNodes.begin(); nodeIter != propertyTreeNode.m_childNodes.end(); nodeIter++)
        {
        PropertyTreeNodeCP childNode = nodeIter->second;
        JsonFromClassesRecursive(jsonValue, classes, *childNode);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String JsonECSqlSelectAdapter::GetClassKey(ECClassCR ecClass)
    {
    Utf8PrintfString classKey("%s.%s", Utf8String(ecClass.GetSchema().GetName().c_str()).c_str(), Utf8String(ecClass.GetName().c_str()).c_str());
    return classKey;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   01/14
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonECSqlSelectAdapter::JsonFromClass(JsonValueR jsonValue, ECClassCR ecClass) const
    {
    Utf8String classKey = GetClassKey(ecClass);
    BeAssert(!jsonValue.isMember(classKey.c_str()));

    JsonValueR jsonClass = jsonValue[classKey.c_str()];

    jsonClass["Name"] = Utf8String(ecClass.GetName());
    jsonClass["DisplayLabel"] = Utf8String(ecClass.GetDisplayLabel());
    jsonClass["SchemaName"] = Utf8String(ecClass.GetSchema().GetName());
    jsonClass["RelationshipPath"] = Json::nullValue;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonECSqlSelectAdapter::JsonFromPropertyRecursive(JsonValueR jsonValue, PropertyTreeNodeCR propertyTreeNode) const
    {
    // TODO: Consider implementing a hierarchical visitor pattern here. 
    ECPropertyCP ecProperty = propertyTreeNode.m_property;
    ECClassCP ecClass = propertyTreeNode.m_class;
    int instanceIndex = propertyTreeNode.m_instanceIndex;

    /* Process node */
    if (ecProperty != nullptr)
        JsonFromProperty(jsonValue, *ecProperty, *ecClass, instanceIndex);

    /* Process any child nodes */
    if (propertyTreeNode.m_childNodes.size() > 0)
        {
        // Categorize, filter and sort child nodes
        bvector<IECInstancePtr> categories;
        PropertyTreeNodesByCategory childNodesByCategory;
        CategorizeProperties(categories, childNodesByCategory, propertyTreeNode.m_childNodes);
        SortProperties(categories, childNodesByCategory);

        // Create Json
        int categoryIndex = 0;
        jsonValue["Categories"] = Json::Value(Json::arrayValue);
        for (IECInstancePtr& categoryCustomAttribute : categories)
            JsonFromCategory(jsonValue["Categories"][categoryIndex++], categoryCustomAttribute, childNodesByCategory);

        return;
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonECSqlSelectAdapter::CategorizeProperties(bvector<IECInstancePtr>& categories, PropertyTreeNodesByCategory& nodesByCategory, const PropertyTreeNodeByName& nodes) const
    {
    // Process all children of node - filter/hide and organize by category
    for (PropertyTreeNodeByName::const_iterator nodeIter = nodes.begin(); nodeIter != nodes.end(); nodeIter++)
        {
        PropertyTreeNodeCP node = nodeIter->second;
        ECPropertyCP ecProperty = node->m_property;

        // Skip property if hidden
        IECInstancePtr hide = ecProperty->GetCustomAttribute("HideProperty");
        if (hide.IsValid())
            continue;
            
        // Determine category
        Utf8String categoryName;
        IECInstancePtr category = m_formatOptions.m_propertyFormatter->GetPropertyCategory(*ecProperty);
        if (!category.IsValid() || !GetStringValue(categoryName, *category, "Name"))
            categoryName = "Miscellaneous";

        // Organize by category
        bvector<PropertyTreeNodeCP>* categoryProperties = nullptr;
        PropertyTreeNodesByCategory::iterator categoryIter = nodesByCategory.find(categoryName);
        if (categoryIter != nodesByCategory.end())
            categoryProperties = &(categoryIter->second);
        else
            {
            categories.push_back(category);
            nodesByCategory[categoryName] = bvector<PropertyTreeNodeCP>();
            categoryProperties = &nodesByCategory[categoryName];
            }
        categoryProperties->push_back(node);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonECSqlSelectAdapter::SortProperties(bvector<IECInstancePtr>& categories, PropertyTreeNodesByCategory& nodesByCategory) const
    {
    // Sort categories
    std::sort(categories.begin(), categories.end(), JsonECSqlSelectAdapter::PrioritySortPredicate);

    // Sort properties within each category
    for (PropertyTreeNodesByCategory::iterator it = nodesByCategory.begin(); it != nodesByCategory.end(); ++it)
        {
        bvector<PropertyTreeNodeCP>& categoryProperties = it->second;
        std::sort(categoryProperties.begin(), categoryProperties.end(), JsonECSqlSelectAdapter::PropertySortPredicate);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   10/12
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonECSqlSelectAdapter::JsonFromProperty(JsonValueR propertyJson, ECPropertyCR ecProperty, ECClassCR ecClass, int instanceIndex) const
    {
    // PropertyJson = ObjectValue (SchemaName, ClassName, PropertyName, PropertyDisplayLabel, PropertyDescription, PropertyType, PropertyPriority)
    propertyJson = Json::Value(Json::objectValue);
    propertyJson["Name"] = ecProperty.GetName();

    propertyJson["DisplayLabel"] = ecProperty.GetDisplayLabel();

    if (!Utf8String::IsNullOrEmpty(ecProperty.GetDescription().c_str()))
        propertyJson["Description"] = ecProperty.GetDescription();

    propertyJson["PrimitiveType"] = ecProperty.GetTypeName();

    int priority;
    if (GetPriorityFromProperty(priority, ecProperty))
        propertyJson["Priority"] = priority;

    // Struct array properties are processed through a separate cursor
    if (ecProperty.GetIsPrimitive())
        propertyJson["IsPrimitive"] = true;
    else if (ecProperty.GetIsStruct())
        propertyJson["IsStruct"] = true;
    else /* if (ecProperty.GetIsArray()) */
        propertyJson["IsArray"] = true;

    propertyJson["ClassKey"] = GetClassKey(ecClass);
    propertyJson["InstanceIndex"] = (instanceIndex >= 0) ? instanceIndex : Json::nullValue;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Ramanujam.Raman                   12/12
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonECSqlSelectAdapter::JsonFromCategory(JsonValueR jsonValue, IECInstancePtr& categoryCustomAttribute, const PropertyTreeNodesByCategory& nodesByCategory) const
    {
    // CategoryJson = ObjectValue (CategoryName, CategoryStandard, CategoryDisplayLabel, CategoryDescription, CategoryExpand, CategoryPriority, Properties)
    jsonValue = Json::Value(Json::objectValue);

    Utf8String categoryName;
    if (categoryCustomAttribute.IsValid())
        {
        bool status = GetStringValue(categoryName, *categoryCustomAttribute, "Name");
        if (!EXPECTED_CONDITION (status))
            categoryName = "Miscellaneous";
        jsonValue["CategoryName"] = categoryName;
        
        int standard;
        if (GetIntegerValue(standard, *categoryCustomAttribute, "Standard"))
            jsonValue["Standard"] = standard;

        Utf8String displayLabel;
        if (GetStringValue(displayLabel, *categoryCustomAttribute, "DisplayLabel"))
            jsonValue["DisplayLabel"] = displayLabel;
        else
            jsonValue["DisplayLabel"] = categoryName;

        Utf8String description;
        if (GetStringValue(description, *categoryCustomAttribute, "Description"))
            jsonValue["Description"] = description;

        bool expand;
        if (GetBooleanValue(expand, *categoryCustomAttribute, "Expand"))
            jsonValue["Expand"] = expand;

        int priority;
        if (GetIntegerValue(priority, *categoryCustomAttribute, "Priority"))
            jsonValue["Priority"] = priority;
        }
    else
        {
        categoryName = "Miscellaneous";
        jsonValue["CategoryName"] = "Miscellaneous";
        jsonValue["DisplayLabel"] = "Miscellaneous";
        jsonValue["Description"] = "Miscellaneous";
        }

    // Construct the JSON for the properties within the category
    jsonValue["Properties"] = Json::Value(Json::arrayValue);
    PropertyTreeNodesByCategory::const_iterator categoryIter = nodesByCategory.find(categoryName);
    BeAssert(categoryIter != nodesByCategory.end());
    const bvector<PropertyTreeNodeCP>& categoryProperties = categoryIter->second;
    JsonValueR categoryPropertiesJson = jsonValue["Properties"];
    for (PropertyTreeNodeCP propertyTreeNode : categoryProperties)
        {
        // Create property JSON (at the end of the array)
        JsonFromPropertyRecursive(categoryPropertiesJson[categoryPropertiesJson.size()], *propertyTreeNode);
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstancePtr ECPropertyFormatter::_GetPropertyCategory(ECN::ECPropertyCR ecProperty)
    {
    return ecProperty.GetCustomAttribute("Category");
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECPropertyFormatter::_FormattedStringFromECValue 
    (
    Utf8StringR strVal, 
    ECN::ECValueCR ecValue, 
    ECN::ECPropertyCR ecProperty, 
    bool isArrayMember
    ) const
    {
    /* 
     * Note: We could conceivably use the IECTypeAdapter and IECTypeAdapterContext
     * to come up with the equivalent string value here. 
     * However, these constructs are unusable in the ECDb layer at the moment. 
     * For example, IECTypeAdapterContext::Create() takes the entire ECInstance 
     * containing the property value. There are some type adapters that require the 
     * entire instance to format a single property value. 
     * Then there are adapters that use the (DgnEC) implementation of IECInstance to pass
     * context like DgnModel, DgnElement, etc. 
     * In ECDb, we simply don't have the entire instance loaded into memory. 
     * 
     * Fortunately, all the type adapters are still at the DgnPlatform layer, and the
     * application that wants to use the ITypeAdapter/ITypeAdapterContext can still pass in 
     * an appropriate implementation of ECPropertyFormatter. 
     * For an example of this use, see DgnECPropertyFormatter in DgnPlatform. 
     */

    strVal = ecValue.ToString(); 
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::FormattedJsonFromECValue(JsonValueR jsonValue, ECValueCR ecValue, ECPropertyCR ecProperty, bool isArrayMember) const
    {
    Utf8String strValue;
    BeAssert(m_formatOptions.m_propertyFormatter.IsValid());
    if (!m_formatOptions.m_propertyFormatter->FormattedStringFromECValue(strValue, ecValue, ecProperty, isArrayMember))
        return false;
    jsonValue = strValue;
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::JsonFromBinary(JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECPropertyCR ecProperty, bool isArrayMember) const
    {
    int size;
    const Byte* data = (const Byte *)ecsqlValue.GetBinary(&size);
    ECValue ecValue;
    ecValue.SetBinary(data, (size_t) size, false);
    return FormattedJsonFromECValue(jsonValue, ecValue, ecProperty, isArrayMember);
    // TODO: Raw binary values needs to be Base64 encoded here
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::JsonFromBoolean(JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECPropertyCR ecProperty, bool isArrayMember) const
    {
    bool value = ecsqlValue.GetBoolean();
    if (m_formatOptions.m_format == ECValueFormat::RawNativeValues)
        {
        jsonValue = value;
        return true;
        }
    else
        {
        ECValue ecValue;
        ecValue.SetBoolean(value);
        return FormattedJsonFromECValue(jsonValue, ecValue, ecProperty, isArrayMember);
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::JsonFromDateTime(JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECPropertyCR ecProperty, bool isArrayMember) const
    {
    DateTime dateTime = ecsqlValue.GetDateTime();
    if (m_formatOptions.m_format == ECValueFormat::RawNativeValues)
        {
        jsonValue = dateTime.ToUtf8String();
        return true;
        }
    else
        {
        // TODO: Need to make the formatting locale aware. 
        ECValue ecValue;
        ecValue.SetDateTime(dateTime);
        return FormattedJsonFromECValue(jsonValue, ecValue, ecProperty, isArrayMember);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::JsonFromDouble(JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECPropertyCR ecProperty, bool isArrayMember) const
    {
    double value = ecsqlValue.GetDouble();
    if (m_formatOptions.m_format == ECValueFormat::RawNativeValues)
        {
        jsonValue = value;
        return true;
        }
    else
        {
        ECValue ecValue;
        ecValue.SetDouble(value);
        return FormattedJsonFromECValue(jsonValue, ecValue, ecProperty, isArrayMember);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::JsonFromInt(JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECPropertyCR ecProperty, bool isArrayMember) const
    {
    int value = ecsqlValue.GetInt();
    if (m_formatOptions.m_format == ECValueFormat::RawNativeValues)
        {
        jsonValue = value;
        return true;
        }
    else
        {
        ECValue ecValue;
        ecValue.SetInteger(value);
        return FormattedJsonFromECValue(jsonValue, ecValue, ecProperty, isArrayMember);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::JsonFromInt64(JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECPropertyCR ecProperty, bool isArrayMember) const
    {
    int64_t value = ecsqlValue.GetInt64();
    if (m_formatOptions.m_format == ECValueFormat::RawNativeValues)
        {
        jsonValue = BeJsonUtilities::StringValueFromInt64(value); // Javascript has issues with holding Int64 values!!!
        return true;
        }
    else
        {
        ECValue ecValue;
        ecValue.SetLong(value);
        return FormattedJsonFromECValue(jsonValue, ecValue, ecProperty, isArrayMember);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::JsonFromPoint2D (JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECPropertyCR ecProperty, bool isArrayMember) const
    {
    DPoint2d value = ecsqlValue.GetPoint2D ();
    if (m_formatOptions.m_format == ECValueFormat::RawNativeValues)
        {
        jsonValue["x"] = value.x;
        jsonValue["y"] = value.y;
        return true;
        }
    else
        {
        ECValue ecValue;
        ecValue.SetPoint2D (value);
        return FormattedJsonFromECValue(jsonValue, ecValue, ecProperty, isArrayMember);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::JsonFromPoint3D (JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECPropertyCR ecProperty, bool isArrayMember) const
    {
    DPoint3d value = ecsqlValue.GetPoint3D ();
    if (m_formatOptions.m_format == ECValueFormat::RawNativeValues)
        {
        jsonValue["x"] = value.x;
        jsonValue["y"] = value.y;
        jsonValue["z"] = value.z;
        return true;
        }
    else
        {
        ECValue ecValue;
        ecValue.SetPoint3D (value);
        return FormattedJsonFromECValue(jsonValue, ecValue, ecProperty, isArrayMember);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::JsonFromString(JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECPropertyCR ecProperty, bool isArrayMember) const
    {
    Utf8CP strValue = ecsqlValue.GetText();
    if (m_formatOptions.m_format == ECValueFormat::RawNativeValues)
        {
        jsonValue = strValue;
        return true;
        }
    else
        {
        ECValue ecValue;
        ecValue.SetUtf8CP (strValue, false);
        return FormattedJsonFromECValue(jsonValue, ecValue, ecProperty, isArrayMember);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::JsonFromPrimitive(JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ECPropertyCR ecProperty, bool isArrayMember) const
    {
    bool status = false;
    ECSqlColumnInfoCR columnInfo = ecsqlValue.GetColumnInfo();
    const ECTypeDescriptor& typeDescriptor = columnInfo.GetDataType();
    BeAssert(typeDescriptor.IsPrimitive());
    PrimitiveType primitiveType = typeDescriptor.GetPrimitiveType();
    // Note: The incoming property can be an array, so it's just simpler to use the column to get the primitive type. 

    switch (primitiveType)
        {
        case PRIMITIVETYPE_Binary:
            {
            status = JsonFromBinary(jsonValue, ecsqlValue, ecProperty, isArrayMember);
            break;
            }
        case PRIMITIVETYPE_Boolean:
            {
            status = JsonFromBoolean(jsonValue, ecsqlValue, ecProperty, isArrayMember);
            break;
            }
        case PRIMITIVETYPE_DateTime:
            {
            status = JsonFromDateTime(jsonValue, ecsqlValue, ecProperty, isArrayMember);
            break;
            }
        case PRIMITIVETYPE_Double:
            {
            status = JsonFromDouble(jsonValue, ecsqlValue, ecProperty, isArrayMember);
            break;
            }
        case PRIMITIVETYPE_Integer:
            {
            status = JsonFromInt(jsonValue, ecsqlValue, ecProperty, isArrayMember);
            break;
            }
        case PRIMITIVETYPE_Long:
            {
            status = JsonFromInt64(jsonValue, ecsqlValue, ecProperty, isArrayMember);
            break;
            }
        case PRIMITIVETYPE_Point2D:
            {
            status = JsonFromPoint2D (jsonValue, ecsqlValue, ecProperty, isArrayMember);
            break;
            }
        case PRIMITIVETYPE_Point3D:
            {
            status = JsonFromPoint3D (jsonValue, ecsqlValue, ecProperty, isArrayMember);
            break;
            }
        case PRIMITIVETYPE_String:
            {
            status = JsonFromString(jsonValue, ecsqlValue, ecProperty, isArrayMember);
            break;
            }
        case PRIMITIVETYPE_IGeometry:
            {
            // TODO: Unhandled for now. Not asserting due to ATPs. 
            ECClassCR ecClass = ecProperty.GetClass();
            ECSchemaCR ecSchema = ecClass.GetSchema();
            LOG.errorv("Cannot handle IGeometry primitive types. Property %s:%s:%s", ecSchema.GetName().c_str(), ecClass.GetName().c_str(), ecProperty.GetName().c_str());
            status = true;
            break;
            }
        default:
            BeAssert(false && "Unknown type");
            break;
        }

    POSTCONDITION (status, false);
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::JsonFromArray(JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ArrayECPropertyCR arrayProperty) const
    {
    if (ARRAYKIND_Struct == arrayProperty.GetKind())
        return JsonFromStructArray(jsonValue, ecsqlValue);
    /* else if (ARRAYKIND_Primitive == arrayProperty.GetKind()) */
    return JsonFromPrimitiveArray(jsonValue, ecsqlValue, arrayProperty);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::JsonFromStructArray(JsonValueR jsonValue, IECSqlValue const& ecsqlValue) const
    {
    bool status = true;
    IECSqlArrayValue const&  structArrayValue = ecsqlValue.GetArray();

    int ii = 0;
    jsonValue = Json::Value(Json::arrayValue);
    for (IECSqlValue const* arrayElementValue : structArrayValue)
        {
        if (!JsonFromStruct(jsonValue[ii++], *arrayElementValue))
            status = false;
        }

    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::JsonFromPrimitiveArray(JsonValueR jsonValue, IECSqlValue const& ecsqlValue, ArrayECPropertyCR arrayProperty) const
    {
    bool status = true;
    IECSqlArrayValue const&  primArrayValue = ecsqlValue.GetArray();

    int ii = 0;
    jsonValue = Json::Value(Json::arrayValue);
    for (IECSqlValue const* arrayElementValue : primArrayValue)
        {
        if (!JsonFromPrimitive(jsonValue[ii++], *arrayElementValue, arrayProperty, true))
            status = false;
        }

    return status;
    }
       
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 09/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::JsonFromStruct(JsonValueR jsonValue, IECSqlValue const& ecsqlValue) const
    {
    IECSqlStructValue const& structECSqlValue = ecsqlValue.GetStruct();

    bool rStatus = true;
    jsonValue = Json::Value(Json::objectValue);
    int count = structECSqlValue.GetMemberCount();
    for (int memberIndex = 0; memberIndex < count; memberIndex++)
        {
        IECSqlValue const& structMemberValue = structECSqlValue.GetValue(memberIndex);
        ECPropertyCP ecLeafProperty = structMemberValue.GetColumnInfo().GetProperty();
        BeAssert(ecLeafProperty != nullptr && "TODO: Adjust code as ColumnInfo::GetProperty can be null.");

        Utf8String ecLeafPropertyName = Utf8String(ecLeafProperty->GetName());
        if (!JsonFromPropertyValue(jsonValue[ecLeafPropertyName.c_str()], structMemberValue))
            rStatus = false;
        }
    return rStatus;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonECSqlSelectAdapter::JsonFromClassKey(JsonValueR jsonValue, ECClassCR ecClass) const
    {
    jsonValue = GetClassKey(ecClass);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonECSqlSelectAdapter::JsonFromClassLabel(JsonValueR jsonValue, ECClassCR ecClass) const
    {
    jsonValue = Utf8String(ecClass.GetDisplayLabel());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void JsonECSqlSelectAdapter::JsonFromInstanceLabel(JsonValueR jsonValue, ECClassCR ecClass) const
    {
    jsonValue = Json::Value(Json::nullValue);

    ECPropertyP ecLabelProperty = ecClass.GetInstanceLabelProperty();
    if (nullptr != ecLabelProperty && ecLabelProperty->GetIsPrimitive())
        {
        // Find the appropriate column, and retrieve the value from the Db
        int columnCount = m_ecsqlStatement.GetColumnCount();
        for (int columnIndex = 0; columnIndex < columnCount; columnIndex++)
            {
            IECSqlValue const& ecsqlValue = m_ecsqlStatement.GetValue(columnIndex);
            ECPropertyCP ecProperty = ecsqlValue.GetColumnInfo().GetProperty();
            BeAssert(ecProperty != nullptr && "TODO: Adjust code as ColumnInfo::GetProperty can be null.");

            if (ecProperty->GetId() == ecLabelProperty->GetId())
                {
                if (!ecsqlValue.IsNull() && 
                    JsonFromPrimitive(jsonValue, ecsqlValue, *ecProperty, false))
                    return;
                break;
                }
            }
        }

    JsonFromClassLabel(jsonValue, ecClass);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::JsonFromInstanceId(JsonValueR jsonValue, IECSqlValue const& ecsqlValue) const
    {
    ECInstanceId ecInstanceId = ecsqlValue.GetId<ECInstanceId>();
    //TODO: If ECInstanceId is invalid, shouldn't something else than 0 be returned?
    const int64_t ecInstanceIdVal = ecInstanceId.IsValid() ? ecInstanceId.GetValue() : 0LL;
    jsonValue = BeJsonUtilities::StringValueFromInt64(ecInstanceIdVal); // Javascript has issues with holding Int64 values!!!
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::JsonFromPropertyValue(JsonValueR jsonValue, IECSqlValue const& ecsqlValue) const
    {
    if (ecsqlValue.IsNull())
        {
        jsonValue = Json::nullValue;
        return true;
        }

    ECN::ECPropertyCP ecProperty = ecsqlValue.GetColumnInfo().GetProperty();
    BeAssert(ecProperty != nullptr && "According to the ECSqlStatement API, this can happen only for array readers, where this method should never have been called");
    if (ecProperty->GetIsPrimitive())
        return JsonFromPrimitive(jsonValue, ecsqlValue, *ecProperty, false);
    else if (ecProperty->GetIsStruct())
        return JsonFromStruct(jsonValue, ecsqlValue);
    else
        {
        ArrayECPropertyCP arrayProperty = ecProperty->GetAsArrayProperty();
        BeAssert(arrayProperty != nullptr);
        return JsonFromArray(jsonValue, ecsqlValue, *arrayProperty);
        }
    }
   
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2013
+---------------+---------------+---------------+---------------+---------------+------*/
 bool JsonECSqlSelectAdapter::JsonFromCell(JsonValueR jsonValue, IECSqlValue const& ecsqlValue) const
    {
    // Create an empty JSON cell (create hierarchy in the case of structs)
    ECSqlColumnInfoCR columnInfo = ecsqlValue.GetColumnInfo();
    ECSqlPropertyPathCR propertyPath = columnInfo.GetPropertyPath();
    size_t pathLength = propertyPath.Size();
    BeAssert(pathLength >= 1 && "Invalid path");
    bool isInstanceIdColumn = false;
    Json::Value* currentCell = &jsonValue;
    for (size_t ii = 0; ii < pathLength; ii++)
        {
        ECPropertyCP ecProperty = propertyPath.At(ii).GetProperty();
        BeAssert(ecProperty != nullptr && "According to the ECSqlStatement API, this can happen only for array readers, where this method should never have been called");
        Utf8String ecPropertyName = Utf8String(ecProperty->GetName());
        if (IsInstanceIdProperty(ecPropertyName))
            {
            ecPropertyName = "$" + ecPropertyName;
            isInstanceIdColumn = true;
            BeAssert(pathLength == 1 && "Cannot have a instance id field as a member of a struct");
            }

        currentCell = &((*currentCell)[ecPropertyName.c_str()]); 
        }
 
    if (isInstanceIdColumn)
        return JsonFromInstanceId(*currentCell, ecsqlValue);
    else
        return JsonFromPropertyValue(*currentCell, ecsqlValue);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 07/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::GetRowInstance(JsonValueR ecJsonInstance) const
    {
    PRECONDITION (m_ecsqlStatement.GetColumnCount() > 0 && "No columns to create an instance", false);

    // Pick the first column's class to get the instance
    IECSqlValue const& ecsqlValue = m_ecsqlStatement.GetValue(0); 
    ECClassCR rootClass = ecsqlValue.GetColumnInfo().GetRootClass();
    return GetRowInstance(ecJsonInstance, rootClass.GetId());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 03/2013
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::GetRowInstance(JsonValueR ecJsonInstance, ECClassId ecClassId) const
    {
    bool status = true;
    ecJsonInstance = Json::Value(Json::objectValue);

    ecJsonInstance["$ECClassKey"]      = Json::nullValue;
    ecJsonInstance["$ECClassLabel"]    = Json::nullValue;
    ecJsonInstance["$ECInstanceLabel"] = Json::nullValue;

    ECClassCP foundClass = nullptr;
    int count = m_ecsqlStatement.GetColumnCount();
    for (int columnIndex=0; columnIndex < count; columnIndex++)
        {
        IECSqlValue const& ecsqlValue = m_ecsqlStatement.GetValue(columnIndex);
        auto const& columnInfo = ecsqlValue.GetColumnInfo();
        auto const& rootClass = columnInfo.GetRootClass();
        if (columnInfo.IsGeneratedProperty() || rootClass.GetId() != ecClassId)
            continue;

        foundClass = &rootClass;
        if (!JsonFromCell(ecJsonInstance, ecsqlValue))
            status = false;
        }

    POSTCONDITION (foundClass != nullptr && "Could not find supplied class in row", false); 

    JsonFromClassKey(ecJsonInstance["$ECClassKey"], *foundClass);
    JsonFromClassLabel(ecJsonInstance["$ECClassLabel"], *foundClass);
    JsonFromInstanceLabel(ecJsonInstance["$ECInstanceLabel"], *foundClass);

    return status;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 07/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter:: GetRowAsIs(JsonValueR ecJsonRow) const
    {
    bool status = true;
    ecJsonRow = Json::Value(Json::arrayValue);

    int count = m_ecsqlStatement.GetColumnCount();
    for (int columnIndex=0; columnIndex < count; columnIndex++)
        {
        IECSqlValue const& ecsqlValue = m_ecsqlStatement.GetValue(columnIndex);
        if (!JsonFromPropertyValue(ecJsonRow[columnIndex], ecsqlValue))
            status = false;
        }

    return status;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool JsonECSqlSelectAdapter::GetRow(JsonValueR currentRow) const
    {
    bool status = true;
    currentRow = Json::Value(Json::arrayValue);

    bvector<ECClassCP> classes; // Classes found in order of their first appearance in a column

    int count = m_ecsqlStatement.GetColumnCount();
    for (int columnIndex=0; columnIndex < count; columnIndex++)
        {
        IECSqlValue const& ecsqlValue = m_ecsqlStatement.GetValue(columnIndex);
        ECClassCR rootClass = ecsqlValue.GetColumnInfo().GetRootClass();

        // Setup a new instance (if necessary)
        int instanceIndex = 0;
        bvector<ECClassCP>::const_iterator iter = std::find(classes.begin(), classes.end(), &rootClass);
        if (iter == classes.end())
            { 
            // Setup a new instance
            instanceIndex = (int) classes.size();
            classes.push_back(&rootClass);

            Json::Value& currentInstance = currentRow[instanceIndex];
            JsonFromClassKey(currentInstance["$ECClassKey"], rootClass);
            JsonFromClassLabel(currentInstance["$ECClassLabel"], rootClass);
            JsonFromInstanceLabel(currentInstance["$ECInstanceLabel"], rootClass);
            }
        else
            {
            // Update an existing instance
            instanceIndex = (int) (iter - classes.begin());
            }

        // Setup node for the property
        if (!JsonFromCell(currentRow[instanceIndex], ecsqlValue))
            status = false;
        }

    return status;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

