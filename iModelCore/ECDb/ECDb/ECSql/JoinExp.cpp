/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "JoinExp.h"
#include "SelectStatementExp.h"
#include "ExpHelper.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//*************************** CrossJoinExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void CrossJoinExp::_ToJson(BeJsValue val , JsonFormat const& fmt) const  {
    //! ITWINJS_PARSE_TREE: CrossJoinExp
    val.SetEmptyObject();
    val["id"] = "CrossJoinExp";
    GetFromClassRef().ToJson(val["from"], fmt);
    GetToClassRef().ToJson(val["to"], fmt);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void CrossJoinExp::_ToECSql(ECSqlRenderContext& ctx) const {
    ctx.AppendToECSql(GetFromClassRef()).AppendToECSql(" CROSS JOIN ").AppendToECSql(GetToClassRef());
}

//*************************** JoinExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
FromExp const* JoinExp::FindFromExpression() const
    {
    Exp const* parent = GetParent();
    while (parent != nullptr && parent->GetType() != Exp::Type::FromClause)
        parent = parent->GetParent();

    if (parent == nullptr)
        return nullptr;

    return parent->GetAsCP<FromExp>();
    }

//************************* JoinConditionExp *******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
JoinConditionExp::JoinConditionExp(std::unique_ptr<BooleanExp> searchCondition) : JoinSpecExp(Type::JoinCondition)
    {
    AddChild(move(searchCondition));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BooleanExp const* JoinConditionExp::GetSearchCondition() const { return GetChild<BooleanExp>(0); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void JoinConditionExp::_ToJson(BeJsValue val , JsonFormat const& fmt) const  {
    //! ITWINJS_PARSE_TREE: JoinConditionExp
    GetSearchCondition()->ToJson(val, fmt);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void JoinConditionExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql("ON ").AppendToECSql(*GetSearchCondition());
    }

//*************************** NaturalJoinExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void NaturalJoinExp::_ToJson(BeJsValue val , JsonFormat const& fmt) const  {
    //! ITWINJS_PARSE_TREE: NaturalJoinExp
    val.SetEmptyArray();
    val["id"] = "NaturalJoinExp";
    val["type"] = Utf8String(ExpHelper::ToSql(m_appliedJoinType));
    GetFromClassRef().ToJson(val["from"], fmt);
    GetToClassRef().ToJson(val["to"], fmt);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void NaturalJoinExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql(GetFromClassRef()).AppendToECSql(" NATURAL ").AppendToECSql(ExpHelper::ToSql(m_appliedJoinType)).AppendToECSql(" ").AppendToECSql(GetToClassRef());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String NaturalJoinExp::_ToString() const
    {
    Utf8String str("NaturalJoin [Type: ");
    str.append(ExpHelper::ToSql(m_appliedJoinType)).append("]");
    return str;
    }

//*************************** QualifiedJoinExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
QualifiedJoinExp::QualifiedJoinExp(std::unique_ptr<ClassRefExp> from, std::unique_ptr<ClassRefExp> to, ECSqlJoinType joinType, std::unique_ptr<JoinSpecExp> joinSpecExp)
    : JoinExp(Type::QualifiedJoin, joinType, move(from), move(to))
    {
    m_nJoinSpecIndex = AddChild(std::move(joinSpecExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void QualifiedJoinExp::_ToJson(BeJsValue val , JsonFormat const& fmt) const  {
    //! ITWINJS_PARSE_TREE: QualifiedJoinExp
    val.SetEmptyObject();
    val["id"] = "QualifiedJoinExp";
    val["type"] = ExpHelper::ToSql(GetJoinType());
    GetFromClassRef().ToJson(val["from"], fmt);
    GetToClassRef().ToJson(val["to"], fmt);
    GetJoinSpec()->ToJson(val["spec"], fmt);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void QualifiedJoinExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql(GetFromClassRef()).AppendToECSql(" ").AppendToECSql(ExpHelper::ToSql(GetJoinType())).AppendToECSql(" ");
    ctx.AppendToECSql(GetToClassRef()).AppendToECSql(" ").AppendToECSql(*GetJoinSpec());
    }



//*************************** RelationshipJoinExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus UsingRelationshipJoinExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == FinalizeParseMode::AfterFinalizingChildren)
        {
        if (ResolveRelationshipEnds(ctx) != SUCCESS)
            return FinalizeParseStatus::Error;

        return FinalizeParseStatus::Completed;
        }

    return FinalizeParseStatus::NotCompleted;
    }
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
BentleyStatus UsingRelationshipJoinExp::ResolveRelationshipEnds(ECSqlParseContext& ctx)
    {
    FromExp const* fromExpression = FindFromExpression();
    PRECONDITION(fromExpression != nullptr, ERROR);

    std::vector<RangeClassInfo> fromClassRefs;
    fromExpression->FindRangeClassRefs(fromClassRefs);
    PRECONDITION(!fromClassRefs.empty(), ERROR);

    ECRelationshipClassCP relationshipClass = GetRelationshipClassNameExp().GetInfo().GetMap().GetClass().GetRelationshipClassCP();
    PRECONDITION(relationshipClass != nullptr, ERROR);

    ResolvedEndPoint fromEP, toEP;
    toEP.SetClassRef(&GetToClassRef().GetAs<ClassNameExp>());
    // Get flat list of relationship source and target classes.
    // It also consider IsPolymorphic attribute on source and target constraint in ECSchema
    ECSqlParseContext::ClassListById sourceList, targetList;
    if (SUCCESS != ctx.GetConstraintClasses(sourceList, relationshipClass->GetSource()))
        return ERROR;

    if (SUCCESS != ctx.GetConstraintClasses(targetList, relationshipClass->GetTarget()))
        return ERROR;

    //We will attempt to resolve toClassName
    ECClassCR toECClass = toEP.GetClassNameRef()->GetInfo().GetMap().GetClass();
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
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "'%s' class is not related to by the relationship '%s'", toECClass.GetFullName(), relationshipClass->GetFullName());
        return ERROR;
        }

    bmap<ECClassId, ClassNameExp const*> fromClassExistsInSourceList;
    bmap<ECClassId, ClassNameExp const*> fromClassExistsInTargetList;

    for (RangeClassInfo const& classRef : fromClassRefs)
        {
        if (classRef.GetExp().GetType() != Exp::Type::ClassName)
            continue;

        if (&classRef.GetExp() == &GetToClassRef() || &classRef.GetExp() == &GetRelationshipClassNameExp())
            continue;

        ClassNameExp const& fromClassNameExpression = classRef.GetExp().GetAs<ClassNameExp>();
        ECClassId fromClassId = fromClassNameExpression.GetInfo().GetMap().GetClass().GetId();

        //Same ClassNameExp/ECClassId could exist in from SELECT * FROM FOO I, FOO B we need to skip same instance of these classes
        if (fromClassExistsInSourceList.find(fromClassId) == fromClassExistsInSourceList.end())
            {
            auto itor = sourceList.find(fromClassId);
            if (itor != sourceList.end())
                fromClassExistsInSourceList[fromClassId] = &fromClassNameExpression;
            }

        if (fromClassExistsInTargetList.find(fromClassId) == fromClassExistsInTargetList.end())
            {
            auto itor = targetList.find(fromClassId);
            if (itor != targetList.end())
                fromClassExistsInTargetList[fromClassId] = &fromClassNameExpression;
            }
        }

    //ECSQL_TODO: If possible remove child class from 'from class list'. If parent is added do not add child to it.

    if (fromClassExistsInSourceList.empty() && fromClassExistsInTargetList.empty())
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "No ECClass in the FROM and JOIN clauses matches the ends of the relationship '%s'.", relationshipClass->GetFullName());
        return ERROR;
        }

    if (fromClassExistsInSourceList.size() > 1 || fromClassExistsInTargetList.size() > 1)
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "Multiple classes in the FROM and JOIN clauses match an end of the relationship '%s'.", relationshipClass->GetFullName());
        return ERROR;
        }

    if (fromClassExistsInSourceList.size() == 1 && fromClassExistsInTargetList.size() == 1)
        {
        if (fromClassExistsInSourceList.begin()->first != fromClassExistsInTargetList.begin()->first)
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "Multiple classes in the FROM and JOIN clauses match an end of the relationship '%s'.", relationshipClass->GetFullName());
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


    bool isSelf = (fromEP.GetLocation() == ClassLocation::ExistInBoth && toEP.GetLocation() == ClassLocation::ExistInBoth);
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
            ctx.Issues().Report(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "FORWARD or BACKWARD must be specified for joins where source and target cannot be identified unambiguously, e.g. joins between the same class.");
            return ERROR;
            }
        }

    if (fromEP.GetLocation() == ClassLocation::NotResolved || fromEP.GetClassNameRef() == nullptr)
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "Could not find class on one of the ends of relationship %s", relationshipClass->GetFullName());
        return ERROR;
        }

    if (toEP.GetLocation() == ClassLocation::NotResolved || toEP.GetClassNameRef() == nullptr)
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "Could not find class on one of the ends of relationship %s", relationshipClass->GetFullName());
        return ERROR;
        }

    m_resolvedFrom = fromEP;
    m_resolvedTo = toEP;

    return SUCCESS;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void UsingRelationshipJoinExp::_ToJson(BeJsValue val , JsonFormat const& fmt) const  {
    //! ITWINJS_PARSE_TREE: UsingRelationshipJoinExp
    val.SetEmptyObject();
    val["id"] = "UsingRelationshipJoinExp";
    GetFromClassRef().ToJson(val["from"], fmt);
    GetToClassRef().ToJson(val["to"], fmt);
    GetRelationshipClassNameExp().ToJson(val["using"], fmt);
    if (m_direction != JoinDirection::Implied)
        val["direction"] = ExpHelper::ToECSql(m_direction);
}
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void UsingRelationshipJoinExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql(GetFromClassRef()).AppendToECSql(" JOIN ").AppendToECSql(GetToClassRef());
    ctx.AppendToECSql(" USING ").AppendToECSql(GetRelationshipClassNameExp());

    if (m_direction != JoinDirection::Implied)
        ctx.AppendToECSql(" ").AppendToECSql(ExpHelper::ToECSql(m_direction));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String UsingRelationshipJoinExp::_ToString() const
    {
    Utf8String str("RelationshipJoin [Direction: ");
    str.append(ExpHelper::ToECSql(m_direction)).append("]");
    return str;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void NamedPropertiesJoinExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql("USING (");
    bool isFirstProp = true;
    for (Utf8StringCR property : m_properties)
        {
        if (!isFirstProp)
            ctx.AppendToECSql(", ");

        ctx.AppendToECSql(property);
        isFirstProp = false;
        }

    ctx.AppendToECSql(")");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void NamedPropertiesJoinExp::_ToJson(BeJsValue val, JsonFormat const&) const {
    //! ITWINJS_PARSE_TREE: NamedPropertiesJoinExp
    val.SetEmptyArray();
    for(auto& prop : m_properties)
        val.appendValue() = prop;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String NamedPropertiesJoinExp::_ToString() const
    {
    Utf8String str("NamedPropertiesJoin [Properties: ");
    bool isFirstItem = true;
    for (auto const& propertyName : m_properties)
        {
        if (!isFirstItem)
            str.append(", ");

        str.append(propertyName.c_str());
        isFirstItem = false;
        }

    str.append("]");
    return str;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

