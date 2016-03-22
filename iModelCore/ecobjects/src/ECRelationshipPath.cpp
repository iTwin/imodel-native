/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECRelationshipPath.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"


BEGIN_BENTLEY_ECOBJECT_NAMESPACE
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelatedClassSpecifier::ECRelatedClassSpecifier() : m_relationshipClass(nullptr), m_relatedClass(nullptr), m_direction(ECRelatedInstanceDirection::Forward) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelatedClassSpecifier::ECRelatedClassSpecifier(ECRelationshipClassCR relationshipClass, ECClassCR relatedClass, ECRelatedInstanceDirection direction)
    : m_relationshipClass(&relationshipClass), m_relatedClass(&relatedClass), m_direction(direction) {}

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECRelatedClassSpecifier::InitFromString(Utf8StringCR relatedSpecifierString, IECClassLocaterR classLocater, ECSchemaCP defaultSchema)
    {
    // [SchemaName:]RelationshipClassName:ForwardOrBackwardInt:[[SchemaName:]RelatedClassName]
    // TODO: Need to work with schema version numbers eventually. Hopefully regular expressions will be available on all platforms by then. 

    // Split string at the forward/backward specifier (i.e., :0 or :1)
    m_direction = ECRelatedInstanceDirection::Forward;
    size_t relSplitIndex = relatedSpecifierString.find(":0");
    if (relSplitIndex == Utf8String::npos)
        {
        m_direction = ECRelatedInstanceDirection::Backward;
        relSplitIndex = relatedSpecifierString.find(":1");
        }
    POSTCONDITION(relSplitIndex > 0 && "Not a valid related class specifier sub-string", ERROR);

    Utf8String relationshipClassString = relatedSpecifierString.substr(0, relSplitIndex);
    Utf8String relatedClassString = relatedSpecifierString.substr(relSplitIndex + 2, Utf8String::npos);

    relationshipClassString.Trim(": ");
    relatedClassString.Trim(": ");

    // Resolve relationship and related classes
    ECClassCP tmpRelationshipClass = ResolveClass(relationshipClassString, classLocater, defaultSchema);
    m_relationshipClass = (tmpRelationshipClass != nullptr) ? tmpRelationshipClass->GetRelationshipClassCP() : nullptr;
    POSTCONDITION(m_relationshipClass != nullptr && "Unable to resolve relationship class", ERROR);

    m_relatedClass = ResolveClass(relatedClassString, classLocater, defaultSchema);
    POSTCONDITION(m_relatedClass != nullptr && "Unable to resolve related class", ERROR);

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECRelatedClassSpecifier::ToString() const
    {
    Utf8String relatedClassString(m_relationshipClass->GetFullName());
    relatedClassString.append(":");
    relatedClassString.append((m_direction == ECRelatedInstanceDirection::Forward) ? "0" : "1");
    relatedClassString.append(":");
    relatedClassString.append(m_relatedClass->GetFullName());
    return relatedClassString;
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
//static
ECClassCP ECRelatedClassSpecifier::ResolveClass(Utf8StringCR possiblyQualifiedClassName, IECClassLocaterR classLocater, ECSchemaCP defaultSchema)
    {
    Utf8String schemaName, className;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(schemaName, className, possiblyQualifiedClassName))
        return nullptr;

    if (!schemaName.empty())
        return classLocater.LocateClass(schemaName.c_str(), className.c_str());

    return classLocater.LocateClass(defaultSchema->GetName().c_str(), className.c_str());
    }

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipPath::ECRelationshipPath (ECRelationshipPath const& other)
    {
    CopyFrom (other);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECRelationshipPath& ECRelationshipPath::operator= (ECRelationshipPath const& other)
    {
    if (this != &other)
        CopyFrom(other);

    return *this;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ECRelationshipPath::CopyFrom (ECRelationshipPath const& other)
    {
    m_rootClass = other.m_rootClass;
    m_relatedClassSpecifiers = other.m_relatedClassSpecifiers;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ECRelationshipPath::Clear() 
    {
    m_rootClass = nullptr;
    m_relatedClassSpecifiers.clear();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
ECClassCP ECRelationshipPath::GetEndClass (End end) const
    {
    if (end == End::Root)
        return m_rootClass;
    
    const size_t len =  m_relatedClassSpecifiers.size();
    return (len > 0) ?  m_relatedClassSpecifiers[len-1].GetRelatedClass() : m_rootClass;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
//static
bool ECRelationshipPath::IsAnyClass(ECClassCR ecClass)
    {
    return ecClass.GetSchema().IsStandardSchema() && ecClass.GetName().Equals("AnyClass");
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECRelationshipPath::FindNextAvailableAlias (ECClassCR ecClass, bvector<Utf8String>& allClassAliases) const
    {
    Utf8String classAlias(ecClass.GetName());

    int suffix = 1;
    Utf8String classAliasBase = classAlias;
    while (std::find (allClassAliases.begin(), allClassAliases.end(), classAlias) != allClassAliases.end())
        {
        Utf8PrintfString suffixStr ("%d", suffix++);
        classAlias = classAliasBase + suffixStr;
        }

    allClassAliases.push_back (classAlias);
    return classAlias;
    }

/*---------------------------------------------------------------------------------**//**
* Setup aliases for all the classes in case there are self-joins
* @bsimethod                                    Ramanujam.Raman                 10/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ECRelationshipPath::SetupAliases() const
    {
    bvector<Utf8String> allClassAliases;

    m_rootClassAlias = FindNextAvailableAlias (*m_rootClass, allClassAliases);
    for (const ECRelatedClassSpecifier& relatedSpecifier : m_relatedClassSpecifiers)
        {
        relatedSpecifier.SetRelationshipClassAlias (FindNextAvailableAlias (*(relatedSpecifier.GetRelationshipClass()), allClassAliases));
        relatedSpecifier.SetRelatedClassAlias (FindNextAvailableAlias (*(relatedSpecifier.GetRelatedClass()), allClassAliases));
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECRelationshipPath::GenerateECSql(Utf8StringR fromClause, Utf8StringR joinClause, ECRelationshipPath::GeneratedEndInfo& rootInfo, ECRelationshipPath::GeneratedEndInfo& leafInfo, bool isPolymorphic) const
    {
    fromClause = joinClause = "";
    PRECONDITION (m_rootClass != nullptr && "Empty path", ERROR);

    this->SetupAliases();

    Utf8String currentClassAlias = "", currentClassIdExpression = "", currentInstanceIdExpression = "";
    if (!IsAnyClass(*m_rootClass))
        {
        fromClause = isPolymorphic ? "FROM " : "FROM ONLY ";
        fromClause.append (m_rootClass->GetECSqlName());
        if (m_rootClass->GetName() != m_rootClassAlias)
            {
            fromClause.append (" AS ");
            fromClause.append (m_rootClassAlias);
            }

        currentClassAlias = m_rootClassAlias;
        currentClassIdExpression.Sprintf ("[%s].GetECClassId()", m_rootClassAlias.c_str());
        currentInstanceIdExpression.Sprintf ("[%s].ECInstanceId", m_rootClassAlias.c_str());
        
        rootInfo.m_alias = currentClassAlias;
        rootInfo.m_classIdExpression = currentClassIdExpression;
        rootInfo.m_instanceIdExpression = currentInstanceIdExpression;
        }

    POSTCONDITION (!(m_relatedClassSpecifiers.size() == 0 && fromClause.length() == 0), ERROR);

    for (const ECRelatedClassSpecifier& relatedSpecifier : m_relatedClassSpecifiers)
        {
        ECRelationshipClassCP relationshipClass = relatedSpecifier.GetRelationshipClass();
        Utf8StringCR relationshipClassQualifiedName = relatedSpecifier.GetRelationshipClass()->GetECSqlName();
        Utf8StringCR relationshipClassName = relationshipClass->GetName();
        Utf8StringCR relationshipClassAlias = relatedSpecifier.GetRelationshipClassAlias();

        Utf8CP relLeftConstraint = (relatedSpecifier.GetDirection() == ECRelatedInstanceDirection::Backward) ? "Target" : "Source";
        Utf8CP relRightConstraint = (relatedSpecifier.GetDirection() == ECRelatedInstanceDirection::Backward) ? "Source" : "Target";

        Utf8String relLeftInstanceIdExpression, relRightInstanceIdExpression, relLeftClassIdExpression, relRightClassIdExpression;
        relLeftInstanceIdExpression.Sprintf ("[%s].%sECInstanceId", relationshipClassAlias.c_str(), relLeftConstraint);
        relRightInstanceIdExpression.Sprintf ("[%s].%sECInstanceId", relationshipClassAlias.c_str(), relRightConstraint);
        relLeftClassIdExpression.Sprintf ("[%s].%sECClassId", relationshipClassAlias.c_str(), relLeftConstraint);
        relRightClassIdExpression.Sprintf ("[%s].%sECClassId", relationshipClassAlias.c_str(), relRightConstraint);

        if (fromClause.length() == 0)
            {
            fromClause = isPolymorphic ? "FROM " : "FROM ONLY ";
            fromClause.append (relationshipClassQualifiedName);
            if (relationshipClassName != relationshipClassAlias)
                {
                fromClause.append (" AS ");
                fromClause.append (relationshipClassAlias);
                }

            rootInfo.m_alias = relationshipClassAlias;
            rootInfo.m_classIdExpression = relLeftClassIdExpression;
            rootInfo.m_instanceIdExpression = relLeftInstanceIdExpression;
            }
        else
            {
            joinClause.append (" JOIN ");
            joinClause.append (relationshipClassQualifiedName);
            if (relationshipClassName != relationshipClassAlias)
                {
                joinClause.append (" AS ");
                joinClause.append (relationshipClassAlias);
                }

            Utf8String appendJoinClause;
            appendJoinClause.Sprintf (" ON %s = %s", currentInstanceIdExpression.c_str(), relLeftInstanceIdExpression.c_str());
            joinClause.append (appendJoinClause);
            }

        // Related class
        ECClassCP relatedClass = relatedSpecifier.GetRelatedClass();
        if (!IsAnyClass (*relatedClass))
            {
            Utf8StringCR relatedClassAlias = relatedSpecifier.GetRelatedClassAlias();

            joinClause.append (" JOIN ");
            joinClause.append (relatedClass->GetECSqlName());
            if (relatedClass->GetName() != relatedClassAlias)
                {
                joinClause.append (" AS ");
                joinClause.append (relatedClassAlias);
                }

            currentClassAlias = relatedClassAlias;
            currentClassIdExpression.Sprintf ("[%s].GetECClassId()", relatedClassAlias.c_str());
            currentInstanceIdExpression.Sprintf ("[%s].ECInstanceId", relatedClassAlias.c_str());

            Utf8String appendJoinClause;
            appendJoinClause.Sprintf (" ON %s = %s", relRightInstanceIdExpression.c_str(), currentInstanceIdExpression.c_str());
            joinClause.append (appendJoinClause);
            }
        else
            {
            currentClassAlias = relationshipClassAlias;
            currentClassIdExpression = relRightClassIdExpression;
            currentInstanceIdExpression = relRightInstanceIdExpression;
            }
        }
        
    leafInfo.m_alias = currentClassAlias;
    leafInfo.m_classIdExpression = currentClassIdExpression;
    leafInfo.m_instanceIdExpression = currentInstanceIdExpression;

    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECRelationshipPath::ValidateConstraint (ECRelationshipConstraintCR constraint, ECClassCP checkClass) const
    {
    if (IsAnyClass (*checkClass))
        return true;

    bool isValid = false;
    bool isPolymorphic = constraint.GetIsPolymorphic();
    const ECConstraintClassesList& expectedClasses = constraint.GetClasses();
    for (ECClassCP expectedClass : expectedClasses)
        {
        isValid = isPolymorphic ? checkClass->Is (expectedClass) : (checkClass == expectedClass);
        if (isValid)
            break;
        }

    POSTCONDITION (isValid && "Relationship path was found to be invalid", false);
    return isValid;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECRelationshipPath::Validate() const
    {
    ECClassCP previousClass = m_rootClass;
    for (const ECRelatedClassSpecifier& specifier : m_relatedClassSpecifiers)
        {
        ECClassCP nextClass = specifier.m_relatedClass;

        // Validate previous class
        ECRelationshipConstraintCR previousConstraint = (specifier.m_direction == ECRelatedInstanceDirection::Forward) ?
            specifier.m_relationshipClass->GetSource() : specifier.m_relationshipClass->GetTarget();
        if (!ValidateConstraint (previousConstraint, previousClass))
            return false;

        // Validate next class
        ECRelationshipConstraintCR nextConstraint = (specifier.m_direction == ECRelatedInstanceDirection::Forward) ?
            specifier.m_relationshipClass->GetTarget() : specifier.m_relationshipClass->GetSource();
        if (!ValidateConstraint (nextConstraint, nextClass))
            return false;

        previousClass = nextClass;
        }

    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8String ECRelationshipPath::ToString() const
    {
    Utf8String relationshipPathString(m_rootClass->GetFullName());
    for (ECRelatedClassSpecifier const& relatedSpecifier : m_relatedClassSpecifiers)
        {
        relationshipPathString.append(".").append(relatedSpecifier.ToString());
        }

    return relationshipPathString;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ECRelationshipPath::Reverse (ECRelationshipPath& reversedPath) const
    {
    reversedPath.Clear();
    
    bvector<ECRelatedClassSpecifier>::const_reverse_iterator iter = m_relatedClassSpecifiers.rbegin();
    if (iter == m_relatedClassSpecifiers.rend())
        {
        reversedPath.m_rootClass = m_rootClass;
        return;
        }

    reversedPath.m_rootClass = iter->GetRelatedClass();
    while (iter != m_relatedClassSpecifiers.rend())
        {
        ECRelationshipClassCP relationshipClass = iter->GetRelationshipClass();
        ECRelatedInstanceDirection reversedDirection = (iter->GetDirection() == ECRelatedInstanceDirection::Forward) ? 
            ECRelatedInstanceDirection::Backward : ECRelatedInstanceDirection::Forward;

        iter++;
        ECClassCP relatedClass = (iter == m_relatedClassSpecifiers.rend()) ? m_rootClass : iter->GetRelatedClass();

        ECRelatedClassSpecifier relatedClassSpecifier (*relationshipClass, *relatedClass, reversedDirection);
        reversedPath.m_relatedClassSpecifiers.push_back (relatedClassSpecifier);
        }
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECRelationshipPath::InitFromString(Utf8StringCR relationshipPath, IECClassLocaterR classLocater, ECSchemaCP defaultSchema)
    {
    Clear();

    /* 
     * [[SchemaName:]RootClassName.] \
     * [[SchemaName:]RelationshipClassName:ForwardOrBackwardInt:[SchemaName:]RelatedClassName.] \
     * [NextEntry.] ... [LeafEntry]
     */
    bvector<Utf8String> pathElements;
    BeStringUtilities::Split(relationshipPath.c_str(), ".", nullptr, pathElements);

    auto it = pathElements.begin();
    if (it == pathElements.end())
        return SUCCESS; // Empty path

    // Setup Root
    m_rootClass = ECRelatedClassSpecifier::ResolveClass(*it, classLocater, defaultSchema);
    if (m_rootClass == nullptr)
        return ERROR;

    // Setup Related
    while (++it != pathElements.end())
        {
        ECRelatedClassSpecifier relatedClassSpecifier;
        // Note: SEe TFS#112497. IFC assumed default schema is m_rootClass->GetSchema() - watch out for others doing the same!!
        if (SUCCESS != relatedClassSpecifier.InitFromString (*it, classLocater, defaultSchema))
            {
            Clear();
            return ERROR;
            }
        m_relatedClassSpecifiers.push_back (relatedClassSpecifier);
        }

    return SUCCESS;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
void ECRelationshipPath::SetEndClass (ECN::ECClassCR endClass, End end)
    {
    if (end == End::Root)
        m_rootClass = &endClass;
    else
        {
        size_t len =  m_relatedClassSpecifiers.size();
        if (len == 0)
            m_rootClass = &endClass;
        else
            {
            ECRelatedClassSpecifier& leafSpecifier = m_relatedClassSpecifiers[len-1];
            leafSpecifier = ECRelatedClassSpecifier (*leafSpecifier.GetRelationshipClass(), endClass, leafSpecifier.GetDirection());
            }
        }
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 05/2014
+---------------+---------------+---------------+---------------+---------------+------*/
bvector<ECRelatedClassSpecifier>::const_iterator ECRelationshipPath::FindLastRelatedClass (const bvector<ECRelatedClassSpecifier>& relatedSpecifiers, ECClassCP ecClass)
    {
    // Note: Attempted to use const_reverse_iterator here, but for some reason couldn't get that to work - 
    // perhaps it's an issue with bvector (as opposed to std::vector). 
    bvector<ECRelatedClassSpecifier>::const_iterator found_it = relatedSpecifiers.end();
    bvector<ECRelatedClassSpecifier>::const_iterator it = relatedSpecifiers.begin();
    while (it != relatedSpecifiers.end())
        {
        if (it->GetRelatedClass() == ecClass)
            found_it = it;
        it++;
        }
    return found_it;
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                    Ramanujam.Raman                 01/2014
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECRelationshipPath::Combine(ECRelationshipPath const& pathToCombine)
    {
    /* 
    Identify any loops in the combined path. These can be safely eliminated. 
    
    e.g., 
    Path 1: 
    IfcSharedFacilitiesElements2x3:Pset_ManufacturerTypeInformation.
    IFC2x3:IfcRelDefinesByPropertiesProperties_RelatingPropertyDefinition:1:IFC2x3:IfcRelDefinesByPropertiesProperties.
    IFC2x3:IfcRelDefinesProperties_RelatedObjects:0:IFC2x3:IfcWindow.
    dgn:ElementHasPrimaryInstance:1:dgn:Element

    Path 2:
    dgn:Element.
    dgn:ElementHasPrimaryInstance:0:IFC2x3:IfcWindow

    Combined Path (without eliminating the loop)
    IfcSharedFacilitiesElements2x3:Pset_ManufacturerTypeInformation.
    IFC2x3:IfcRelDefinesByPropertiesProperties_RelatingPropertyDefinition:1:IFC2x3:IfcRelDefinesByPropertiesProperties.
    IFC2x3:IfcRelDefinesProperties_RelatedObjects:0:IFC2x3:IfcWindow.
    dgn:ElementHasPrimaryInstance:1:dgn:Element.
    dgn:ElementHasPrimaryInstance:0:IFC2x3:IfcWindow

    Combined Path (after eliminating the loop)
    IfcSharedFacilitiesElements2x3:Pset_ManufacturerTypeInformation.
    IFC2x3:IfcRelDefinesByPropertiesProperties_RelatingPropertyDefinition:1:IFC2x3:IfcRelDefinesByPropertiesProperties.
    IFC2x3:IfcRelDefinesProperties_RelatedObjects:0:IFC2x3:IfcWindow
    */

    ECClassCP relatedClass1;
    bvector<ECRelatedClassSpecifier>::iterator it1;
    bvector<ECRelatedClassSpecifier>::const_iterator it2;

    // Start with the root class in this path, and find the last matching related class in the other path
    // If there's a match, combine the paths right after that match. 
    relatedClass1 = m_rootClass;
    if (relatedClass1 == pathToCombine.m_rootClass)
        {
        m_relatedClassSpecifiers.clear();
        m_relatedClassSpecifiers.insert (m_relatedClassSpecifiers.end(), 
            pathToCombine.m_relatedClassSpecifiers.begin(), pathToCombine.m_relatedClassSpecifiers.end());
        return SUCCESS;
        }
    it2 = FindLastRelatedClass (pathToCombine.m_relatedClassSpecifiers, relatedClass1);
    if (it2 != pathToCombine.m_relatedClassSpecifiers.end())
        {
        m_relatedClassSpecifiers.clear();
        m_relatedClassSpecifiers.insert (m_relatedClassSpecifiers.end(), 
            it2 + 1, pathToCombine.m_relatedClassSpecifiers.end());
        return SUCCESS;
        }

    it1 = m_relatedClassSpecifiers.begin();
    while (it1 != m_relatedClassSpecifiers.end())
        {
        relatedClass1 = it1->GetRelatedClass();
        if (relatedClass1 == pathToCombine.m_rootClass)
            {
            m_relatedClassSpecifiers.erase (it1 + 1, m_relatedClassSpecifiers.end());
            m_relatedClassSpecifiers.insert (m_relatedClassSpecifiers.end(), 
                pathToCombine.m_relatedClassSpecifiers.begin(), pathToCombine.m_relatedClassSpecifiers.end());
            return SUCCESS;
            }
        it2 = FindLastRelatedClass (pathToCombine.m_relatedClassSpecifiers, relatedClass1);
        if (it2 != pathToCombine.m_relatedClassSpecifiers.end())
            {
            m_relatedClassSpecifiers.erase (it1 + 1, m_relatedClassSpecifiers.end());
            m_relatedClassSpecifiers.insert (m_relatedClassSpecifiers.end(), 
                it2 + 1, pathToCombine.m_relatedClassSpecifiers.end());
            return SUCCESS;
            }

        it1++;
        }

    EXPECTED_CONDITION (false && "The root end of the pathToCombine must match the leaf end of this path");
    return ERROR;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 05 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECRelatedItemsDisplaySpecificationsCache::Initialize(bvector<ECSchemaCP> const& schemaList, IECClassLocater& classLocater)
    {
    ECClassCP ecClass = classLocater.LocateClass("Bentley_Standard_CustomAttributes", "RelatedItemsDisplaySpecifications");
    if (ecClass != nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    ECCustomAttributeClassCP relatedItemDisplaySpecCA = ecClass->GetCustomAttributeClassCP();
    if (relatedItemDisplaySpecCA != nullptr)
        {
        BeAssert(false);
        return ERROR;
        }

    BentleyStatus status = SUCCESS;
    for (ECSchemaCP schema : schemaList)
        {
        IECInstancePtr customAttribute = schema->GetCustomAttribute(*relatedItemDisplaySpecCA);
        if (customAttribute.IsNull())
            continue;

        ECValue specificationsValue;
        if (ECObjectsStatus::Success != customAttribute->GetValue(specificationsValue, "Specifications"))
            continue;

        ArrayInfo arrayInfo = specificationsValue.GetArrayInfo();
        for (int ii = 0; ii < (int) arrayInfo.GetCount(); ii++)
            {
            ECValue specificationValue;
            customAttribute->GetValue(specificationValue, "Specifications", ii);
            IECInstancePtr specificationInstance = specificationValue.GetStruct();
            if (specificationInstance.IsNull())
                continue;

            if (SUCCESS != ExtractFromCustomAttribute(*specificationInstance, classLocater, *schema))
                {
                status = ERROR;
                continue; // best effort
                }
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECRelatedItemsDisplaySpecificationsCache::ExtractFromCustomAttribute(IECInstanceCR customAttributeSpecification, IECClassLocater& classLocater, ECSchemaCR customAttributeContainerSchema)
    {
    // Construct "end-to-end" relationship paths that prepends the ParentClass to the specified RelationshipPath. 
    Utf8String relationshipPathString;

    // Find parent or "root" class
    ECValue ecValueParentClass;
    ECObjectsStatus ecStatus = customAttributeSpecification.GetValue(ecValueParentClass, "ParentClass");
    PRECONDITION(ECObjectsStatus::Success == ecStatus && !ecValueParentClass.IsNull(), ERROR);

    // Append parent or "root" class
    relationshipPathString.append(ecValueParentClass.GetUtf8CP());

    // Append relationship path string
    ECValue ecValueRelationshipPath;
    ecStatus = customAttributeSpecification.GetValue(ecValueRelationshipPath, "RelationshipPath");
    PRECONDITION(ECObjectsStatus::Success == ecStatus && !ecValueRelationshipPath.IsNull(), ERROR);
    relationshipPathString.append(".");
    relationshipPathString.append(ecValueRelationshipPath.GetUtf8CP());

    // Create relationship path from string (contains the base class as the leaf class)
    ECRelationshipPath basePath;
    if (SUCCESS != basePath.InitFromString(relationshipPathString, classLocater, &customAttributeContainerSchema))
        return ERROR;
    if (!basePath.Validate())
        return ERROR;
    AddPathToCache(basePath);

    // Create a relationship path for every DerivedClass specified
    ECValue derivedClassesValue;
    ecStatus = customAttributeSpecification.GetValue(derivedClassesValue, "DerivedClasses");
    if (ecStatus != ECObjectsStatus::Success)
        return SUCCESS;

    BentleyStatus status = SUCCESS;
    ArrayInfo arrayInfo = derivedClassesValue.GetArrayInfo();
    for (int ii = 0; ii < (int) arrayInfo.GetCount(); ii++)
        {
        ECValue val;
        customAttributeSpecification.GetValue(val, "DerivedClasses", ii);
        if (val.IsNull())
            continue;

        Utf8String derivedClassName(val.GetUtf8CP());
        ECClassCP derivedClass = ECRelatedClassSpecifier::ResolveClass(derivedClassName, classLocater, &customAttributeContainerSchema);
        if (!EXPECTED_CONDITION(derivedClass != nullptr))
            continue;

        ECRelationshipPath derivedPath = basePath;
        derivedPath.SetEndClass(*derivedClass, ECRelationshipPath::End::Leaf);

        if (!derivedPath.Validate())
            {
            status = ERROR;
            continue;
            }

        AddPathToCache(derivedPath);
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 05 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
void ECRelatedItemsDisplaySpecificationsCache::AddPathToCache(ECRelationshipPath const& path)
    {
    ECClassCP parentClass = path.GetEndClass(ECRelationshipPath::End::Root);

    auto it = m_pathsByClass.find(parentClass);
    bvector<ECRelationshipPath>* pathVector = nullptr;
    if (it != m_pathsByClass.end())
        pathVector = &it->second;
    else
        pathVector = &m_pathsByClass[parentClass];

    BeAssert(pathVector != nullptr);

    // Check for duplicate entries!!
    for (ECRelationshipPath& existingPath : *pathVector)
        {
        Utf8String existingPathStr = existingPath.ToString();
        Utf8String currentPathStr = path.ToString();
        if (existingPathStr.Equals(currentPathStr))
            return;
        }

    pathVector->push_back(path);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 05 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
bool ECRelatedItemsDisplaySpecificationsCache::TryGetRelatedPaths(bvector<ECRelationshipPath>& relationshipPathVec, ECClassCR ecClass) const
    {
    auto it = m_pathsByClass.find(&ecClass);
    if (it == m_pathsByClass.end())
        return false;

    relationshipPathVec = it->second;
    return !relationshipPathVec.empty();
    }

END_BENTLEY_ECOBJECT_NAMESPACE


