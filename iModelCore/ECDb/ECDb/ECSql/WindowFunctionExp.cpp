/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//*************************** WindowFunctionExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus WindowFunctionExp::_FinalizeParsing(ECSqlParseContext & ctx, FinalizeParseMode mode)
    {
    if (mode == FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    SetTypeInfo(GetWindowFunctionCallExp()->GetAsCP<FunctionCallExp>()->GetTypeInfo());
    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowFunctionExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql(*GetWindowFunctionCallExp());
    if (FilterClauseExp const * e = GetFilterClauseExp())
        ctx.AppendToECSql(*e);

    ctx.AppendToECSql(" OVER");
    if (WindowSpecification const * e = GetWindowSpecification())
        ctx.AppendToECSql(*e);
    else
        {
        ctx.AppendToECSql(" ");
        ctx.AppendToECSql(GetWindowName());
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowFunctionExp::_ToJson(BeJsValue val, JsonFormat const & fmt) const
    {
    val.SetEmptyObject();
    val["id"] = "WindowFunctionExp";
    GetWindowFunctionCallExp()->ToJson(val["WindowFunctionCallExp"], fmt);
    if (GetFilterClauseExp() != nullptr)
        GetFilterClauseExp()->ToJson(val["FilterExp"], fmt);
    if (GetWindowSpecification() != nullptr)
        GetWindowSpecification()->ToJson(val["WindowSpecificationExp"], fmt);
    if (GetWindowName().size() != 0)
        val["WindowName"] = GetWindowName();
    }

//*************************** WindowSpecification ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowSpecification::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql("(");
    bool isFirstWindowSpecificationClause = true;
    if (GetWindowName().size() != 0)
        {
        ctx.AppendToECSql(GetWindowName());
        isFirstWindowSpecificationClause = false;
        }
    if (WindowPartitionColumnReferenceListExp const* e = GetPartitionBy())
        {
        if (!isFirstWindowSpecificationClause)
            ctx.AppendToECSql(" ");

        ctx.AppendToECSql(*e);
        isFirstWindowSpecificationClause = false;
        }
    if (OrderByExp const * e = GetOrderBy())
        {
        if (!isFirstWindowSpecificationClause)
            ctx.AppendToECSql(" ");

        ctx.AppendToECSql(*e);
        isFirstWindowSpecificationClause = false;
        }
    if (WindowFrameClauseExp const * e = GetWindowFrameClause())
        {
        if (!isFirstWindowSpecificationClause)
            ctx.AppendToECSql(" ");

        ctx.AppendToECSql(*e);
        }
    ctx.AppendToECSql(")");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowSpecification::_ToJson(BeJsValue val, JsonFormat const & fmt) const
    {
    val.SetEmptyObject();
    val["id"] = "WindowSpecificationExp";
    if (GetPartitionBy() != nullptr)
        GetPartitionBy()->ToJson(val["PartitionByExp"], fmt);
    if (GetOrderBy() != nullptr)
        GetOrderBy()->ToJson(val["OrderBy"], fmt);
    if (GetWindowFrameClause() != nullptr)
        GetWindowFrameClause()->ToJson(val["WindowFrameClauseExp"], fmt);
    if (GetWindowName().size() != 0)
        val["WindowName"] = GetWindowName();
    }

//*************************** WindowPartitionColumnReferenceListExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowPartitionColumnReferenceListExp::_ToECSql(ECSqlRenderContext & ctx) const
    {
    ctx.AppendToECSql("PARTITION BY");
    bool isFirstItem = true;
     for (size_t nPos = 0; nPos < GetChildrenCount(); nPos++)
        {
        if (!isFirstItem)
            ctx.AppendToECSql(",");

        ctx.AppendToECSql(" ");
        ctx.AppendToECSql(GetChildren()[nPos]->GetAs<WindowPartitionColumnReferenceExp>());
        
        isFirstItem = false;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowPartitionColumnReferenceListExp::_ToJson(BeJsValue val, JsonFormat const & fmt) const
    {
    val.SetEmptyArray();
    for (Exp const* childExp : GetChildren())
        childExp->ToJson(val.appendValue(), fmt);
    }

//*************************** WindowPartitionColumnReferenceExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowPartitionColumnReferenceExp::_ToECSql(ECSqlRenderContext &ctx) const
    {
    ctx.AppendToECSql(*GetColumnRef());
    if (GetCollateClauseFunction() != WindowPartitionColumnReferenceExp::CollateClauseFunction::NotSpecified)
        ctx.AppendToECSql(" COLLATE ").AppendToECSql(ExpHelper::ToSql(GetCollateClauseFunction()));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowPartitionColumnReferenceExp::_ToJson(BeJsValue val, JsonFormat const & fmt) const
    {
    val.SetEmptyObject();
    val["id"] = "WindowPartitionColumnReferenceExp";
    GetColumnRef()->ToJson(val["ColumnRef"], fmt);
    if (GetCollateClauseFunction() != CollateClauseFunction::NotSpecified)
        val["CollateClauseFunction"] = ExpHelper::ToSql(GetCollateClauseFunction());
    }

//*************************** FilterClauseExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void FilterClauseExp::_ToECSql(ECSqlRenderContext &ctx) const
    {
    ctx.AppendToECSql(" FILTER(").AppendToECSql(*GetWhereExp()).AppendToECSql(")");
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void FilterClauseExp::_ToJson(BeJsValue val, JsonFormat const & fmt) const
    {
    val.SetEmptyObject();
    val["id"] = "FilterExp";
    GetWhereExp()->ToJson(val["WhereExp"], fmt);
    }

//*************************** WindowFrameClauseExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowFrameClauseExp::_ToECSql(ECSqlRenderContext &ctx) const
    {
    ctx.AppendToECSql(ExpHelper::ToSql(GetWindowFrameUnit())).AppendToECSql(" ");

    if (WindowFrameBetweenExp const * betweenExp = GetWindowFrameBetweenExp())
        ctx.AppendToECSql(*betweenExp);
    else if (WindowFrameStartExp const * startExp = GetWindowFrameStartExp())
        ctx.AppendToECSql(*startExp);
    
    if (GetWindowFrameExclusionType() != WindowFrameClauseExp::WindowFrameExclusionType::NotSpecified)
        ctx.AppendToECSql(" ").AppendToECSql(ExpHelper::ToSql(GetWindowFrameExclusionType()));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowFrameClauseExp::_ToJson(BeJsValue val, JsonFormat const & fmt) const
    {
    val.SetEmptyObject();
    val["id"] = "WindowFrameClauseExp";
    val["WindowFrameUnit"] = ExpHelper::ToSql(GetWindowFrameUnit());
    if (GetWindowFrameBetweenExp() != nullptr)
        GetWindowFrameBetweenExp()->ToJson(val["WindowFrameBetweenExp"], fmt);
    else
        GetWindowFrameStartExp()->ToJson(val["WindowFrameStartExp"], fmt);
    
    if (GetWindowFrameExclusionType() != WindowFrameClauseExp::WindowFrameExclusionType::NotSpecified)
        GetWindowFrameStartExp()->ToJson(val["WindowFrameExclusion"], fmt);
    }

//*************************** WindowFrameStartExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowFrameStartExp::_ToECSql(ECSqlRenderContext &ctx) const
    {
    switch (GetWindowFrameStartType())
        {
        case WindowFrameStartExp::WindowFrameStartType::CurrentRow:
            ctx.AppendToECSql("CURRENT ROW");
            break;
        case WindowFrameStartExp::WindowFrameStartType::UnboundedPreceding:
            ctx.AppendToECSql("UNBOUNDED PRECEDING");
            break;
        case WindowFrameStartExp::WindowFrameStartType::ValuePreceding:
            ctx.AppendToECSql(*GetValueExp());
            ctx.AppendToECSql(" PRECEDING");
            break;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowFrameStartExp::_ToJson(BeJsValue val, JsonFormat const &fmt) const
    {
    val.SetEmptyObject();
    val["id"] = "WindowFrameStartExp";
    switch (GetWindowFrameStartType())
        {
        case WindowFrameStartExp::WindowFrameStartType::CurrentRow:
            val["WindowFrameStartType"] = "CURRENT ROW";
            break;
        case WindowFrameStartExp::WindowFrameStartType::UnboundedPreceding:
            val["WindowFrameStartType"] = "UNBOUNDED PRECEDING";
            break;
        case WindowFrameStartExp::WindowFrameStartType::ValuePreceding:
            val["WindowFrameStartType"] = "VALUE PRECEDING";
            GetValueExp()->ToJson(val["WindowFrameStartValue"], fmt);
            break;
        }
    }

//*************************** WindowFrameBetweenExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowFrameBetweenExp::_ToECSql(ECSqlRenderContext &ctx) const
    {
    ctx.AppendToECSql("BETWEEN ");
    ctx.AppendToECSql(*GetFirstWindowFrameBoundExp());
    ctx.AppendToECSql(" AND ");
    ctx.AppendToECSql(*GetSecondWindowFrameBoundExp());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowFrameBetweenExp::_ToJson(BeJsValue val, JsonFormat const & fmt) const
    {
    val.SetEmptyObject();
    GetFirstWindowFrameBoundExp()->ToJson(val["FirstWindowFrameBoundExp"], fmt);
    GetSecondWindowFrameBoundExp()->ToJson(val["SecondWindowFrameBoundExp"], fmt);
    }

//*************************** FirstWindowFrameBoundExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void FirstWindowFrameBoundExp::_ToECSql(ECSqlRenderContext &ctx) const
    {
    switch (GetWindowFrameBoundType())
        {
        case FirstWindowFrameBoundExp::WindowFrameBoundType::CurrentRow:
            ctx.AppendToECSql("CURRENT ROW");
            break;
        case FirstWindowFrameBoundExp::WindowFrameBoundType::UnboundedPreceding:
            ctx.AppendToECSql("UNBOUNDED PRECEDING");
            break;
        case FirstWindowFrameBoundExp::WindowFrameBoundType::ValueFollowing:
            ctx.AppendToECSql(*GetValueExp());
            ctx.AppendToECSql(" FOLLOWING");
            break;
        case FirstWindowFrameBoundExp::WindowFrameBoundType::ValuePreceding:
            ctx.AppendToECSql(*GetValueExp());
            ctx.AppendToECSql(" PRECEDING");
            break;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void FirstWindowFrameBoundExp::_ToJson(BeJsValue val, JsonFormat const & fmt) const
    {
    val.SetEmptyObject();
    switch (GetWindowFrameBoundType())
        {
        case FirstWindowFrameBoundExp::WindowFrameBoundType::CurrentRow:
            val["FirstWindowFrameBoundType"] = "CURRENT ROW";
            break;
        case FirstWindowFrameBoundExp::WindowFrameBoundType::UnboundedPreceding:
            val["FirstWindowFrameBoundType"] = "UNBOUNDED PRECEDING";
            break;
        case FirstWindowFrameBoundExp::WindowFrameBoundType::ValueFollowing:
            val["FirstWindowFrameBoundType"] = "VALUE FOLLOWING";
            GetValueExp()->ToJson(val["FirstWindowFrameBoundValueExp"], fmt);
            break;
        case FirstWindowFrameBoundExp::WindowFrameBoundType::ValuePreceding:
            val["FirstWindowFrameBoundType"] = "VALUE PRECEDING";
            GetValueExp()->ToJson(val["FirstWindowFrameBoundValueExp"], fmt);
            break;
        }
    }

//*************************** SecondWindowFrameBoundExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SecondWindowFrameBoundExp::_ToECSql(ECSqlRenderContext &ctx) const
    {
    switch (GetWindowFrameBoundType())
        {
        case SecondWindowFrameBoundExp::WindowFrameBoundType::CurrentRow:
            ctx.AppendToECSql("CURRENT ROW");
            break;
        case SecondWindowFrameBoundExp::WindowFrameBoundType::UnboundedFollowing:
            ctx.AppendToECSql("UNBOUNDED FOLLOWING");
            break;
        case SecondWindowFrameBoundExp::WindowFrameBoundType::ValueFollowing:
            ctx.AppendToECSql(*GetValueExp());
            ctx.AppendToECSql(" FOLLOWING");
            break;
        case SecondWindowFrameBoundExp::WindowFrameBoundType::ValuePreceding:
            ctx.AppendToECSql(*GetValueExp());
            ctx.AppendToECSql(" PRECEDING");
            break;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SecondWindowFrameBoundExp::_ToJson(BeJsValue val, JsonFormat const & fmt) const
    {
    val.SetEmptyObject();
    val["id"] = "SecondWindowFrameBoundExp";
    switch (GetWindowFrameBoundType())
        {
        case SecondWindowFrameBoundExp::WindowFrameBoundType::CurrentRow:
            val["SecondWindowFrameBoundType"] = "CURRENT ROW";
            break;
        case SecondWindowFrameBoundExp::WindowFrameBoundType::UnboundedFollowing:
            val["SecondWindowFrameBoundType"] = "UNBOUNDED FOLLOWING";
            break;
        case SecondWindowFrameBoundExp::WindowFrameBoundType::ValueFollowing:
            val["SecondWindowFrameBoundType"] = "VALUE FOLLOWING";
            GetValueExp()->ToJson(val["SecondWindowFrameBoundValueExp"], fmt);
            break;
        case SecondWindowFrameBoundExp::WindowFrameBoundType::ValuePreceding:
            val["SecondWindowFrameBoundType"] = "VALUE PRECEDING";
            GetValueExp()->ToJson(val["SecondWindowFrameBoundValueExp"], fmt);
            break;
        }
    }

//*************************** WindowFunctionClauseExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowFunctionClauseExp::_ToECSql(ECSqlRenderContext & ctx) const
    {
    ctx.AppendToECSql("WINDOW ").AppendToECSql(*GetWindowDefinitionListExp());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowFunctionClauseExp::_ToJson(BeJsValue val, JsonFormat const & fmt) const
    {
    val["id"] = "WindowFunctionClauseExp";
    GetWindowDefinitionListExp()->ToJson(val["WindowDefinitionListExp"], fmt);
    }

//*************************** WindowDefinitionExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowDefinitionExp::_ToECSql(ECSqlRenderContext & ctx) const
    {
    ctx.AppendToECSql(GetWindowName()).AppendToECSql(" AS ").AppendToECSql(*GetWindowSpecification());
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowDefinitionExp::_ToJson(BeJsValue val, JsonFormat const & fmt) const
    {
    val.SetEmptyObject();
    val["id"] = "WindowDefinitionExp";
    GetWindowSpecification()->ToJson(val["WindowSpecificationExp"], fmt);
    }

//*************************** WindowDefinitionListExp *****************************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowDefinitionListExp::_ToECSql(ECSqlRenderContext & ctx) const
    {
    bool isFirstWindowDefinition = false;
    for (Exp const* childExp : GetChildren())
        {
        if (!isFirstWindowDefinition)
            ctx.AppendToECSql(", ");
        ctx.AppendToECSql(*childExp);
        isFirstWindowDefinition = true;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowDefinitionListExp::_ToJson(BeJsValue val, JsonFormat const & fmt) const
    {
    val.SetEmptyArray();
    for (Exp const* childExp : GetChildren())
        childExp->ToJson(val.appendValue(), fmt);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
