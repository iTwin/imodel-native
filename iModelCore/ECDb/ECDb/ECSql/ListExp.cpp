
/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ListExp.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ListExp.h"
#include "UpdateStatementExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//****************** AssignmentListExp *************************

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
void AssignmentListExp::AddAssignmentExp(std::unique_ptr<AssignmentExp> assignmentExp) { AddChild(std::move(assignmentExp)); }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       01/2014
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
// @bsimethod                                    Krischan.Eberle       01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
AssignmentExp const* AssignmentListExp::GetAssignmentExp(size_t index) const { return GetChild<AssignmentExp>(index); }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       01/2014
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
// @bsimethod                                    Krischan.Eberle       12/2013
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
// @bsimethod                                    Krischan.Eberle       11/2013
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
// @bsimethod                                    Krischan.Eberle       07/2017
//+---------------+---------------+---------------+---------------+---------------+--------
ValueExpListExp::ValueExpListExp(std::vector<std::unique_ptr<ValueExp>>& valueExpList) : ValueExpListExp()
    {
    for (std::unique_ptr<ValueExp>& valueExp : valueExpList)
        {
        AddValueExp(valueExp);
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus ValueExpListExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == FinalizeParseMode::BeforeFinalizingChildren)
        //Indicate that the exp per se doesn't have a single type info, because it can vary across it children
        SetTypeInfo(ECSqlTypeInfo(ECSqlTypeInfo::Kind::Varies));

    return FinalizeParseStatus::Completed;
    }


//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       08/2013
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
// @bsimethod                                    Krischan.Eberle       12/2016
//+---------------+---------------+---------------+---------------+---------------+--------
void SystemPropertyExpIndexMap::AddIfSystemProperty(PropertyNameExp const& exp, size_t index)
    {
    if (exp.GetSystemPropertyInfo().IsSystemProperty())
        m_sysPropIndexMap[&exp.GetSystemPropertyInfo()] = index;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       12/2016
//+---------------+---------------+---------------+---------------+---------------+--------
void SystemPropertyExpIndexMap::AddIfSystemProperty(SchemaManager const& schemaManager, ECN::ECPropertyCR prop, size_t index)
    {
    ECSqlSystemPropertyInfo const& sysPropInfo = schemaManager.Main().GetSystemSchemaHelper().GetSystemPropertyInfo(prop);
    if (sysPropInfo.IsSystemProperty())
        m_sysPropIndexMap[&sysPropInfo] = index;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE

