
/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ListExp.h"
#include "UpdateStatementExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//****************** AssignmentListExp *************************

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void AssignmentListExp::AddAssignmentExp(std::unique_ptr<AssignmentExp> assignmentExp) { AddChild(std::move(assignmentExp)); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus AssignmentListExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    const size_t childrenCount = GetChildrenCount();
    for (size_t i = 0; i < childrenCount; i++)
        {
        AssignmentExp const* assignmentExp = GetAssignmentExp(i);
        ECN::ECPropertyCR prop = assignmentExp->GetPropertyNameExp()->GetPropertyMap().GetProperty();
        m_specialTokenExpIndexMap.AddIfSystemProperty(ctx.GetECDb().Schemas(), prop, i);
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
AssignmentExp const* AssignmentListExp::GetAssignmentExp(size_t index) const { return GetChild<AssignmentExp>(index); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void AssignmentListExp::_ToECSql(ECSqlRenderContext& ctx) const
{
    bool isFirstItem = true;
    for (Exp const* childExp : GetChildren())
        {
        if (!isFirstItem)
            ctx.AppendToECSql(", ");

        ctx.AppendToECSql(*childExp);
        isFirstItem = false;
        }
    }


//****************** PropertyNameListExp *************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus PropertyNameListExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    const size_t childrenCount = GetChildrenCount();
    for (size_t i = 0; i < childrenCount; i++)
        {
        PropertyNameExp const* propNameExp = GetPropertyNameExp(i);
        m_specialTokenExpIndexMap.AddIfSystemProperty(*propNameExp, i);
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void PropertyNameListExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql("(");

    bool isFirstItem = true;
    for (Exp const* childExp : GetChildren())
        {
        if (!isFirstItem)
            ctx.AppendToECSql(", ");

        ctx.AppendToECSql(*childExp);
        isFirstItem = false;
        }

    ctx.AppendToECSql(")");
    }


//*************************** ValueListExp ******************************************

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
ValueExpListExp::ValueExpListExp(std::vector<std::unique_ptr<ValueExp>>& valueExpList) : ValueExpListExp()
    {
    for (std::unique_ptr<ValueExp>& valueExp : valueExpList)
        {
        AddValueExp(valueExp);
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus ValueExpListExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == FinalizeParseMode::BeforeFinalizingChildren)
        //Indicate that the exp per se doesn't have a single type info, because it can vary across it children
        SetTypeInfo(ECSqlTypeInfo(ECSqlTypeInfo::Kind::Varies));

    return FinalizeParseStatus::Completed;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void ValueExpListExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    bool isFirstItem = true;
    for (Exp const* childExp : GetChildren())
        {
        if (!isFirstItem)
            ctx.AppendToECSql(", ");

        ctx.AppendToECSql(*childExp);
        isFirstItem = false;
        }
    }


//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void SystemPropertyExpIndexMap::AddIfSystemProperty(PropertyNameExp const& exp, size_t index)
    {
    if (exp.GetSystemPropertyInfo().IsSystemProperty())
        m_sysPropIndexMap[&exp.GetSystemPropertyInfo()] = index;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void SystemPropertyExpIndexMap::AddIfSystemProperty(SchemaManager const& schemaManager, ECN::ECPropertyCR prop, size_t index)
    {
    ECSqlSystemPropertyInfo const& sysPropInfo = schemaManager.Main().GetSystemSchemaHelper().GetSystemPropertyInfo(prop);
    if (sysPropInfo.IsSystemProperty())
        m_sysPropIndexMap[&sysPropInfo] = index;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

