/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/JoinExp.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "JoinExp.h"
#include "SelectStatementExp.h"
#include "ExpHelper.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
  
//************************* JoinConditionExp *******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
JoinConditionExp::JoinConditionExp(std::unique_ptr<BooleanExp> searchCondition)
    : JoinSpecExp ()
    {
    AddChild (move (searchCondition));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan                       05/2013
//+---------------+---------------+---------------+---------------+---------------+------
//virtual
Utf8String JoinConditionExp::_ToECSql() const
    {
    return "ON " + GetSearchCondition ()->ToECSql();
    }

//*************************** JoinExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
FromExp const* JoinExp::FindFromExpression() const
    {        
    auto parent = GetParent();
    while (parent != nullptr && parent->GetType() != Exp::Type::FromClause)
        parent = parent->GetParent();

    return static_cast<FromExp const*>(parent);
    }


//*************************** NaturalJoinExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String NaturalJoinExp::_ToECSql() const 
    {
    return GetFromClassRef().ToECSql() + " NATURAL " + ExpHelper::ToString(m_appliedJoinType)+ " " + GetToClassRef().ToECSql();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String NaturalJoinExp::_ToString() const 
    {
    Utf8String str ("NaturalJoin [Type: ");
    str.append (ExpHelper::ToString (m_appliedJoinType)).append ("]");
    return str;
    }

//*************************** QualifiedJoinExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
QualifiedJoinExp::QualifiedJoinExp (std::unique_ptr<ClassRefExp> from, std::unique_ptr<ClassRefExp> to, ECSqlJoinType joinType, std::unique_ptr<JoinSpecExp> joinSpecExp ) 
    : JoinExp(joinType, move (from), move (to))
    {
    m_nJoinSpecIndex = AddChild (std::move (joinSpecExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String QualifiedJoinExp::_ToECSql() const 
    {
    return GetFromClassRef().ToECSql() + " " + ExpHelper::ToString(GetJoinType()) + " " + GetToClassRef().ToECSql() + " "+ GetJoinSpec ()->ToECSql();
    }

//*************************** RelationshipJoinExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus RelationshipJoinExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == FinalizeParseMode::AfterFinalizingChildren)
        {
        if (ResolveRelationshipEnds (ctx) != SUCCESS)
            return FinalizeParseStatus::Error;

        return FinalizeParseStatus::Completed;
        }

    return FinalizeParseStatus::NotCompleted;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus RelationshipJoinExp::ResolveRelationshipEnds (ECSqlParseContext& ctx)
    {
    auto fromExpression = FindFromExpression();
    PRECONDITION(fromExpression != nullptr, ERROR);

    RangeClassRefList fromClassRefs;
    fromExpression->FindRangeClassRefs(fromClassRefs);
    PRECONDITION(!fromClassRefs.empty(), ERROR);

    auto relationshipClass = GetRelationshipClass().GetInfo().GetMap().GetClass().GetRelationshipClassCP();
    PRECONDITION(relationshipClass != nullptr, ERROR);

    ResolvedEndPoint fromEP, toEP;
    toEP.SetClassRef(static_cast<ClassNameExp const*> (&GetToClassRef()));
    // Get flat list of relationship source and target classes. 
    // It also consider IsPolymorphic attribute on source and target constraint in ECSchema
    ECSqlParseContext::ClassListById sourceList, targetList;
    bool sourceContainsAnyClass = false;
    bool targetContainsAnyClass = false;
    ctx.GetConstraintClasses(sourceList, relationshipClass->GetSource(), &sourceContainsAnyClass);
    ctx.GetConstraintClasses(targetList, relationshipClass->GetTarget(), &targetContainsAnyClass);

    //We will attempt to resolve toClassName
    auto& toECClass = toEP.GetClassNameRef()->GetInfo().GetMap().GetClass();
    if (sourceList.find(toECClass.GetId()) != sourceList.end())
        {
        toEP.SetLocation(ClassLocation::ExistInSource, true);
        }

    if (targetList.find(toECClass.GetId()) != targetList.end())
        {
        toEP.SetLocation(ClassLocation::ExistInTarget, true);
        }

    if (toEP.GetLocation() == ClassLocation::NotResolved)
        {
        if (sourceContainsAnyClass && targetContainsAnyClass)
            {
            ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "'%s' class is not related to by the relationship '%s' as both relationship endpoint contain 'AnyClass'.", toECClass.GetFullName(), relationshipClass->GetFullName());
            return ERROR;
            }

        if (sourceContainsAnyClass)
            {
            toEP.SetLocation (ClassLocation::ExistInSource, true);
            toEP.SetAnyClass (true);
            }
        else if (targetContainsAnyClass)
            {
            toEP.SetLocation (ClassLocation::ExistInTarget, true);
            toEP.SetAnyClass (true);
            }
        else
            {
            ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "'%s' class is not related to by the relationship '%s'", toECClass.GetFullName(), relationshipClass->GetFullName());
            return ERROR;
            }
        }

    bmap<ECClassId,ClassNameExp const*> fromClassExistsInSourceList;
    bmap<ECClassId,ClassNameExp const*> fromClassExistsInTargetList;

    for(auto classRef : fromClassRefs)
        {
        if (classRef->GetType() != Exp::Type::ClassName)
            continue;
            
        if (classRef == &GetToClassRef() || classRef == &GetRelationshipClass())
            continue; 

        auto fromClassNameExpression = static_cast<ClassNameExp const*> (classRef);
        auto fromClassId = fromClassNameExpression->GetInfo().GetMap().GetClass ().GetId ();

        //Same ClassNameExp/ECClassId could exist in from SELECT * FROM FOO I, FOO B we need to skip same instance of these classes
        if (fromClassExistsInSourceList.find(fromClassId) == fromClassExistsInSourceList.end())
            {
            auto itor = sourceList.find(fromClassId);
            if (itor != sourceList.end())
                fromClassExistsInSourceList[fromClassId] = fromClassNameExpression;
            else if (sourceContainsAnyClass && fromClassExistsInSourceList.empty()) //only the first class found is use as source
                fromClassExistsInSourceList[fromClassId] = fromClassNameExpression;
            }

        if (fromClassExistsInTargetList.find(fromClassId) == fromClassExistsInTargetList.end())
            {
            auto itor = targetList.find(fromClassId);    
            if (itor != targetList.end())
                fromClassExistsInTargetList[fromClassId] = fromClassNameExpression;
            else if (targetContainsAnyClass && fromClassExistsInTargetList.empty()) //only the first class found is use as target
                fromClassExistsInTargetList[fromClassId] = fromClassNameExpression;
            }
        }

    //ECSQL_TODO: If possible remove child class from 'from class list'. If parent is added do not add child to it. 

    if (fromClassExistsInSourceList.empty() && fromClassExistsInTargetList.empty())
        {
        ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "No ECClass in the FROM and JOIN clauses matches the ends of the relationship '%s'.", relationshipClass->GetFullName());
        return ERROR;
        }

    if (fromClassExistsInSourceList.size() > 1 || fromClassExistsInTargetList.size() > 1)
        {
        ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Multiple classes in the FROM and JOIN clauses match an end of the relationship '%s'.", relationshipClass->GetFullName());
        return ERROR;
        }
        
    if (fromClassExistsInSourceList.size() == 1 && fromClassExistsInTargetList.size() == 1)
        {
        if (fromClassExistsInSourceList.begin()->first != fromClassExistsInTargetList.begin()->first)
            {
            ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Multiple classes in the FROM and JOIN clauses match an end of the relationship '%s'.", relationshipClass->GetFullName());
            return ERROR;
            }
        }

    if (!fromClassExistsInSourceList.empty())
        {
        fromEP.SetClassRef(fromClassExistsInSourceList.begin()->second);
        fromEP.SetLocation(ClassLocation::ExistInSource, true);
        }

    if (!fromClassExistsInTargetList.empty())
        {
        fromEP.SetClassRef(fromClassExistsInTargetList.begin()->second);
        fromEP.SetLocation(ClassLocation::ExistInTarget, true);
        }


    bool isSelf = (fromEP.GetLocation() == ClassLocation::ExistInBoth && toEP.GetLocation() == ClassLocation:: ExistInBoth);
    if (!isSelf)
        {        
        //make other side unambiguous
        if (fromEP.GetLocation() != ClassLocation::ExistInBoth)
            {
            if (fromEP.GetLocation() == ClassLocation::ExistInSource)
                toEP.SetLocation(ClassLocation::ExistInTarget, false);
            else if (fromEP.GetLocation() == ClassLocation::ExistInTarget)
                toEP.SetLocation(ClassLocation::ExistInSource, false);
            }
        else if (toEP.GetLocation() != ClassLocation::ExistInBoth)
            {
            if (toEP.GetLocation() == ClassLocation::ExistInSource)
                fromEP.SetLocation(ClassLocation::ExistInTarget, false);
            else if (toEP.GetLocation() == ClassLocation::ExistInTarget)
                fromEP.SetLocation(ClassLocation::ExistInSource, false);
            }
        }
    else
        {
        //Rule: In self-join direction must be provided
        if (GetDirection() == JoinDirection::Implied)
            {
            ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "FORWARD or REVERSE must be specified for joins where source and target cannot be identified unambiguously, e.g. joins between the same class.");
            return ERROR;
            }
        }

    if (fromEP.GetLocation() == ClassLocation::NotResolved || fromEP.GetClassNameRef() == nullptr)
        {
        ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Could not find class on one of the ends of relationship %s", relationshipClass->GetFullName());
        return ERROR;
        }

    if (toEP.GetLocation() == ClassLocation::NotResolved || toEP.GetClassNameRef() == nullptr)
        {
        ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Could not find class on one of the ends of relationship %s", relationshipClass->GetFullName());
        return ERROR;
        }

    m_resolvedFrom = fromEP;
    m_resolvedTo = toEP;

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String RelationshipJoinExp::_ToECSql() const 
    {
    auto tmp = GetFromClassRef().ToECSql() + " JOIN " + GetToClassRef().ToECSql() + " USING " + GetRelationshipClass ().ToECSql();
    if (m_direction != JoinDirection::Implied)
        tmp += Utf8String(" ") + ExpHelper::ToString(m_direction);

    return tmp;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Affan.Khan       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String RelationshipJoinExp::_ToString() const 
    {
    Utf8String str ("RelationshipJoin [Direction: ");
    str.append (ExpHelper::ToString (m_direction)).append ("]");
    return str;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE

