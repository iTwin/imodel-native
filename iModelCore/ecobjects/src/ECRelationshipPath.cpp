/*--------------------------------------------------------------------------------------+
|
|     $Source: src/ECRelationshipPath.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01 / 2014
//+---------------+---------------+---------------+---------------+---------------+------
ECClassCP ResolveClass(Utf8StringCR possiblyQualifiedClassName, IECClassLocaterR classLocater, ECSchemaCP defaultSchema)
    {
    Utf8String schemaName, className;
    if (ECObjectsStatus::Success != ECClass::ParseClassName(schemaName, className, possiblyQualifiedClassName))
        return nullptr;

    if (!schemaName.empty())
        return classLocater.LocateClass(schemaName.c_str(), className.c_str());

    return classLocater.LocateClass(defaultSchema->GetName().c_str(), className.c_str());
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01/2014
//---------------------------------------------------------------------------------------
ECRelatedClassSpecifier::ECRelatedClassSpecifier() : m_relationshipClass(nullptr), m_relatedClass(nullptr), m_direction(ECRelatedInstanceDirection::Forward) {}

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01/2014
//---------------------------------------------------------------------------------------
ECRelatedClassSpecifier::ECRelatedClassSpecifier(ECRelationshipClassCR relationshipClass, ECClassCR relatedClass, ECRelatedInstanceDirection direction)
    : m_relationshipClass(&relationshipClass), m_relatedClass(&relatedClass), m_direction(direction) {}

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01/2014
//---------------------------------------------------------------------------------------
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

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01/2014
//---------------------------------------------------------------------------------------
Utf8String ECRelatedClassSpecifier::ToString() const
    {
    Utf8String relatedClassString(m_relationshipClass->GetFullName());
    relatedClassString.append(":");
    relatedClassString.append((m_direction == ECRelatedInstanceDirection::Forward) ? "0" : "1");
    relatedClassString.append(":");
    relatedClassString.append(m_relatedClass->GetFullName());
    return relatedClassString;
    }
    
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01/2014
//---------------------------------------------------------------------------------------
ECRelationshipPath::ECRelationshipPath (ECRelationshipPath const& other)
    {
    CopyFrom (other);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01/2014
//---------------------------------------------------------------------------------------
ECRelationshipPath& ECRelationshipPath::operator= (ECRelationshipPath const& other)
    {
    if (this != &other)
        CopyFrom(other);
    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01/2014
//---------------------------------------------------------------------------------------
void ECRelationshipPath::CopyFrom (ECRelationshipPath const& other)
    {
    m_rootClass = other.m_rootClass;
    m_relatedClassSpecifiers = other.m_relatedClassSpecifiers;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01/2014
//---------------------------------------------------------------------------------------
void ECRelationshipPath::Clear() 
    {
    m_rootClass = nullptr;
    m_relatedClassSpecifiers.clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01/2014
//---------------------------------------------------------------------------------------
bool ECRelationshipPath::IsEmpty() const
    {
    return m_rootClass == nullptr && m_relatedClassSpecifiers.empty();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01/2014
//---------------------------------------------------------------------------------------
ECClassCP ECRelationshipPath::GetEndClass (End end) const
    {
    if (end == End::Root)
        return m_rootClass;
    
    const size_t len =  m_relatedClassSpecifiers.size();
    return (len > 0) ?  m_relatedClassSpecifiers[len-1].GetRelatedClass() : m_rootClass;
    }
    

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01/2014
//---------------------------------------------------------------------------------------
//static
bool ECRelationshipPath::IsAnyClass(ECClassCR ecClass)
    {
    return ecClass.GetSchema().IsStandardSchema() && ecClass.GetName().Equals("AnyClass");
    }
    
//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 10/2014
//---------------------------------------------------------------------------------------
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

//---------------------------------------------------------------------------------------
// Setup aliases for all the classes in case there are self-joins
// @bsimethod                                    Ramanujam.Raman                 10/2014
//---------------------------------------------------------------------------------------
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

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01/2014
//---------------------------------------------------------------------------------------
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
    
//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 05/2014
//---------------------------------------------------------------------------------------
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

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 05/2014
//---------------------------------------------------------------------------------------
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

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01/2014
//---------------------------------------------------------------------------------------
Utf8String ECRelationshipPath::ToString() const
    {
    Utf8String relationshipPathString(m_rootClass->GetFullName());
    for (ECRelatedClassSpecifier const& relatedSpecifier : m_relatedClassSpecifiers)
        {
        relationshipPathString.append(".").append(relatedSpecifier.ToString());
        }

    return relationshipPathString;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01/2014
//---------------------------------------------------------------------------------------
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

//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01/2014
//---------------------------------------------------------------------------------------
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
    m_rootClass = ResolveClass(*it, classLocater, defaultSchema);
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
    
//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01/2014
//---------------------------------------------------------------------------------------
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
    
//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01/2014
//---------------------------------------------------------------------------------------
// static
bool ECRelationshipPath::AreInSameHierarchy(ECClassCP& moreDerivedClass, ECClassCP inClass1, ECClassCP inClass2)
    {
    if (inClass1 == inClass2)
        {
        moreDerivedClass = inClass1;
        return true;
        }

    if (inClass1->Is(inClass2))
        {
        moreDerivedClass = inClass1;
        return true;
        }

    if (inClass2->Is(inClass1))
        {
        moreDerivedClass = inClass2;
        return true;
        }

    return false;
    }

//---------------------------------------------------------------------------------------
// Finds the last location in the the search path that matches the supplied search class - a match is found when the classes are the same or in the same hierarchy.
// @param foundClass [out] The class that was matched - if the classes are not the same, but in the same hierarchy, this returns the more derived class. 
// @param foundIter [out] The iterator location in the searchPath *next* to the location that was found
// @param searchPath [in] The path to be searched
// @param searchClass [in] The class to be searched
// @return true if a match was found. false otherwise. 
// @bsimethod                                    Ramanujam.Raman                 05/2014
//---------------------------------------------------------------------------------------
// static
bool ECRelationshipPath::FindLastMatchingClass(ECClassCP& foundClass, bvector<ECRelatedClassSpecifier>::const_iterator& foundIter, ECRelationshipPath const& searchPath, ECClassCP searchClass)
    {
    // Note: Attempted to use const_reverse_iterator here, but for some reason couldn't get that to work - 
    // perhaps it's an issue with bvector (as opposed to std::vector). 
    bvector<ECRelatedClassSpecifier>::const_iterator combineFromIter = searchPath.m_relatedClassSpecifiers.end();
    bvector<ECRelatedClassSpecifier>::const_iterator it = searchPath.m_relatedClassSpecifiers.begin();
    while (it != searchPath.m_relatedClassSpecifiers.end())
        {
        if (AreInSameHierarchy(foundClass, it->GetRelatedClass(), searchClass))
            foundIter = it;
        it++;
        }

    if (combineFromIter != searchPath.m_relatedClassSpecifiers.end())
        {
        foundIter++;
        return true;
        }

    if (AreInSameHierarchy(foundClass, searchPath.m_rootClass, searchClass))
        {
        foundIter = searchPath.m_relatedClassSpecifiers.begin();
        return true;
        }

    return false;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                    Ramanujam.Raman                 01/2014
//---------------------------------------------------------------------------------------
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

    ECClassCP relatedClass1, foundClass;
    bvector<ECRelatedClassSpecifier>::iterator it1;
    bvector<ECRelatedClassSpecifier>::const_iterator it2;

    /*
     * Start with the root class in this path, and find the last matching related class in the other path
     * If there's a match, i.e., the classes are in the same hierarchy, combine the paths right after that match, 
     * replacing the class at the match with the "more" derived class. 
     */

    relatedClass1 = m_rootClass;
    if (FindLastMatchingClass(foundClass, it2, pathToCombine, relatedClass1))
        {
        m_rootClass = foundClass;
        m_relatedClassSpecifiers.clear();
        m_relatedClassSpecifiers.insert(m_relatedClassSpecifiers.end(), it2, pathToCombine.m_relatedClassSpecifiers.end());
        return SUCCESS;
        }

    it1 = m_relatedClassSpecifiers.begin();
    while (it1 != m_relatedClassSpecifiers.end())
        {
        relatedClass1 = it1->GetRelatedClass();
        if (FindLastMatchingClass(foundClass, it2, pathToCombine, relatedClass1))
            {
            ECRelatedClassSpecifier replaceSpecifier(*it1->GetRelationshipClass(), *foundClass, it1->GetDirection());
            m_relatedClassSpecifiers.erase(it1, m_relatedClassSpecifiers.end());
            m_relatedClassSpecifiers.push_back(replaceSpecifier);
            m_relatedClassSpecifiers.insert(m_relatedClassSpecifiers.end(), it2, pathToCombine.m_relatedClassSpecifiers.end());
            return SUCCESS;
            }

        it1++;
        }

    EXPECTED_CONDITION (false && "The root end of the pathToCombine must match the leaf end of this path");
    return ERROR;
    }

END_BENTLEY_ECOBJECT_NAMESPACE


