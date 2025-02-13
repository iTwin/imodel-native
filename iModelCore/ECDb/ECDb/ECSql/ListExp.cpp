
/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ListExp.h"
#include "UpdateStatementExp.h"
#include "SelectStatementExp.h"

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
        ECN::ECPropertyCR prop = assignmentExp->GetPropertyNameExp()->GetPropertyMap()->GetProperty();
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
//+---------------+---------------+---------------+---------------+---------------+------
void AssignmentListExp::_ToJson(BeJsValue val, JsonFormat const& fmt ) const {
    //! ITWINJS_PARSE_TREE: AssignmentListExp
    val.SetEmptyArray();
    for (Exp const* childExp : GetChildren())
        childExp->ToJson(val.appendValue(), fmt);
}

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
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyNameListExp::_ToJson(BeJsValue val, JsonFormat const& fmt ) const {
    //! ITWINJS_PARSE_TREE: PropertyNameListExp
    val.SetEmptyArray();
    for (Exp const* childExp : GetChildren())
        childExp->ToJson(val.appendValue(), fmt);
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
//+---------------+---------------+---------------+---------------+---------------+------
void ValueExpListExp::_ToJson(BeJsValue val, JsonFormat const& fmt ) const {
    //! ITWINJS_PARSE_TREE: ValueExpListExp
    val.SetEmptyArray();
    for (Exp const* childExp : GetChildren())
        childExp->ToJson(val.appendValue(), fmt);
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

// ******************* RowValueConstructorListExp *************************

RowValueConstructorListExp::RowValueConstructorListExp(std::vector<std::unique_ptr<ValueExpListExp>>& rowValueList)
    : RangeClassRefExp(Exp::Type::RowValueConstructorList, PolymorphicInfo::NotSpecified())
    {
    std::unique_ptr<SelectClauseExp> selectStmtExp = std::make_unique<SelectClauseExp>(); 

    // BeAssert(!rowValueList.empty() && rowValueList[0] != nullptr);
    size_t childrenCount = rowValueList[0]->GetChildrenCount();

    for (size_t i = 1; i <= childrenCount; i++)
        {
        Utf8String columnName = "column" + std::to_string(i);
        
        selectStmtExp->AddProperty(std::make_unique<DerivedPropertyExp>(
            std::make_unique<SqlColumnNameExp>(columnName),
                columnName.c_str()
        ));
        }
    
    AddChild(std::move(selectStmtExp));

    for (std::unique_ptr<ValueExpListExp>& valueExpListExp : rowValueList)
        { 
        AddChild(std::move(valueExpListExp));
        }
    }

Exp::FinalizeParseStatus RowValueConstructorListExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
        if (mode == FinalizeParseMode::BeforeFinalizingChildren)
            SetTypeInfo(ECSqlTypeInfo(ECSqlTypeInfo::Kind::Varies)); 
        return FinalizeParseStatus::Completed;
    }

void RowValueConstructorListExp::_ToECSql(ECSqlRenderContext& ctx) const
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

        ctx.AppendToECSql("(");
    }

void RowValueConstructorListExp::_ToJson(BeJsValue val, JsonFormat const& fmt) const
    {
        val.SetEmptyArray();
        for (Exp const* childExp : GetChildren())
            childExp->ToJson(val.appendValue(), fmt);
    }

// @todo - Naron: this doesn't get triggered 
void RowValueConstructorListExp::_ExpandSelectAsterisk(std::vector<std::unique_ptr<DerivedPropertyExp>>& expandedSelectClauseItemList, ECSqlParseContext const& ctx) const
    {
        for (size_t i = 0; i < GetSelection()->GetChildrenCount(); i++)
            {
            auto derivedPropertyExp = GetSelection()->GetChildren().Get<DerivedPropertyExp>(i);
            expandedSelectClauseItemList.push_back(std::make_unique<DerivedPropertyExp>(
                std::make_unique<SqlColumnNameExp>(derivedPropertyExp->GetColumnAlias()),
                derivedPropertyExp->GetColumnAlias().c_str()
            ));
            }
    }

PropertyMatchResult RowValueConstructorListExp::_FindProperty(ECSqlParseContext& ctx, PropertyPath const& propertyPath, const PropertyMatchOptions& options) const
    {
    if (propertyPath.IsEmpty())
        return PropertyMatchResult::NotFound();
    
    Utf8String subqueryAlias = GetParent()->GetParent()->GetAs<SubqueryRefExp>().GetAlias();

    // if subquery dooesn't have alias
    if (subqueryAlias.empty())
        {
        for (size_t i = 0; i <  GetSelection()->GetChildrenCount(); i++)
            {
            auto derivedPropertyExp = GetSelection()->GetChildren().Get<DerivedPropertyExp>(i);
            if (propertyPath.First().GetName() == derivedPropertyExp->GetColumnAlias())
                return PropertyMatchResult(options, propertyPath, propertyPath, *derivedPropertyExp, 0);
            }
        }
    // if subquery has alias
    else{
        if (propertyPath.First().GetName() == subqueryAlias && propertyPath.Size() == 2)
            {
            for (size_t i = 0; i <  GetSelection()->GetChildrenCount(); i++)
                {
                auto derivedPropertyExp = GetSelection()->GetChildren().Get<DerivedPropertyExp>(i);
                if (propertyPath[1].GetName() == derivedPropertyExp->GetColumnAlias())
                    return PropertyMatchResult(options, propertyPath, propertyPath, *derivedPropertyExp, 0);
                }
            }
        }

    
    return PropertyMatchResult::NotFound();
    }

std::vector<ValueExpListExp const*> RowValueConstructorListExp::GetRowValues() const
    {
        std::vector<ValueExpListExp const*> rowValues;
        for (size_t i = 1; i < GetChildrenCount(); i++)
            rowValues.push_back(GetChild<ValueExpListExp>(i));
        return rowValues;
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

