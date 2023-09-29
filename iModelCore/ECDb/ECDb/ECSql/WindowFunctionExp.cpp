/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "WindowFunctionExp.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

Exp::FinalizeParseStatus WindowFunctionExp::_FinalizeParsing(ECSqlParseContext &, FinalizeParseMode)
    {
    SetTypeInfo(ECSqlTypeInfo::CreatePrimitive(PRIMITIVETYPE_Double));
    return FinalizeParseStatus::Completed;
    }

bool WindowFunctionExp::_TryDetermineParameterExpType(ECSqlParseContext& ctx, ParameterExp& parameterExp) const
    {
    parameterExp.SetTargetExpInfo(GetTypeInfo());
    return true;
    }

void WindowFunctionExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql(*GetWindowFunctionType());
    ctx.AppendToECSql(" OVER");
    ctx.AppendToECSql(*GetWindowSpecification());
    }

void WindowFunctionExp::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

Utf8String WindowFunctionExp::_ToString() const
    {
    return Utf8String();
    }

Exp::FinalizeParseStatus WindowFunctionType::_FinalizeParsing(ECSqlParseContext &, FinalizeParseMode)
    {
    return FinalizeParseStatus::Completed;
    }

bool WindowFunctionType::_TryDetermineParameterExpType(ECSqlParseContext& ctx, ParameterExp& parameterExp) const
    {
    parameterExp.SetTargetExpInfo(GetTypeInfo());
    return true;
    }

void WindowFunctionType::_ToECSql(ECSqlRenderContext &) const
    {
    }

void WindowFunctionType::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

Utf8String WindowFunctionType::_ToString() const
    {
    return "?";
    }

bool WindowSpecification::_TryDetermineParameterExpType(ECSqlParseContext& ctx, ParameterExp& parameterExp) const
    {
    return true;
    }

void WindowSpecification::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql("(");
    bool isFirstWindowSpecificationClause = true;
    if (WindowPartitionColumnReferenceListExp const* e = GetPartitionBy())
        {
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

    ctx.AppendToECSql(")");
    }

void WindowSpecification::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

Utf8String WindowSpecification::_ToString() const
    {
    return "?";
    }

Exp::FinalizeParseStatus WindowSpecification::_FinalizeParsing(ECSqlParseContext &, FinalizeParseMode)
    {
    return FinalizeParseStatus::Completed;
    }

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

void WindowPartitionColumnReferenceListExp::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

Exp::FinalizeParseStatus WindowPartitionColumnReferenceListExp::_FinalizeParsing(ECSqlParseContext &, FinalizeParseMode)
    {
    return FinalizeParseStatus::Completed;
    }


void WindowPartitionColumnReferenceExp::_ToECSql(ECSqlRenderContext &ctx) const
    {
    ctx.AppendToECSql(*GetColumnRef());
    if (GetCollateClauseFunction() != WindowPartitionColumnReferenceExp::CollateClauseFunction::NotSpecified)
        {
        ctx.AppendToECSql(" COLLATE ");
        switch(GetCollateClauseFunction())
            {
            case WindowPartitionColumnReferenceExp::CollateClauseFunction::Binary:
                ctx.AppendToECSql("BINARY");
                break;
            case WindowPartitionColumnReferenceExp::CollateClauseFunction::NoCase:
                ctx.AppendToECSql("NOCASE");
                break;
            case WindowPartitionColumnReferenceExp::CollateClauseFunction::Rtrim:
                ctx.AppendToECSql("RTRIM");
                break;
            }
        }
    }

void WindowPartitionColumnReferenceExp::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

Exp::FinalizeParseStatus WindowPartitionColumnReferenceExp::_FinalizeParsing(ECSqlParseContext &, FinalizeParseMode)
    {
    return FinalizeParseStatus::Completed;
    }

void FilterClauseExp::_ToECSql(ECSqlRenderContext &ctx) const
    {
    }

void FilterClauseExp::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

Exp::FinalizeParseStatus FilterClauseExp::_FinalizeParsing(ECSqlParseContext &, FinalizeParseMode)
    {
    return FinalizeParseStatus::Completed;
    }

Exp::FinalizeParseStatus WindowFrameClauseExp::_FinalizeParsing(ECSqlParseContext &, FinalizeParseMode)
    {
    return FinalizeParseStatus::Completed;
    }

bool WindowFrameClauseExp::_TryDetermineParameterExpType(ECSqlParseContext &, ParameterExp &) const
    {
    return false;
    }

void WindowFrameClauseExp::_ToECSql(ECSqlRenderContext &) const
    {
    }

void WindowFrameClauseExp::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

Utf8String WindowFrameClauseExp::_ToString() const
    {
    return "";
    }

Exp::FinalizeParseStatus WindowFrameStartExp::_FinalizeParsing(ECSqlParseContext &, FinalizeParseMode)
    {
    return FinalizeParseStatus::Completed;
    }

bool WindowFrameStartExp::_TryDetermineParameterExpType(ECSqlParseContext &, ParameterExp &) const
    {
    return false;
    }

void WindowFrameStartExp::_ToECSql(ECSqlRenderContext &) const
    {
    }

void WindowFrameStartExp::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

Utf8String WindowFrameStartExp::_ToString() const
    {
    return "";
    }

Exp::FinalizeParseStatus WindowFrameBetweenExp::_FinalizeParsing(ECSqlParseContext &, FinalizeParseMode)
    {
    return FinalizeParseStatus::Completed;
    }

bool WindowFrameBetweenExp::_TryDetermineParameterExpType(ECSqlParseContext &, ParameterExp &) const
    {
    return false;
    }

void WindowFrameBetweenExp::_ToECSql(ECSqlRenderContext &) const
    {
    }

void WindowFrameBetweenExp::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

Utf8String WindowFrameBetweenExp::_ToString() const
    {
    return "";
    }

Exp::FinalizeParseStatus FirstWindowFrameBoundExp::_FinalizeParsing(ECSqlParseContext &, FinalizeParseMode)
    {
    return FinalizeParseStatus::Completed;
    }

bool FirstWindowFrameBoundExp::_TryDetermineParameterExpType(ECSqlParseContext &, ParameterExp &) const
    {
    return false;
    }

void FirstWindowFrameBoundExp::_ToECSql(ECSqlRenderContext &) const
    {
    }

void FirstWindowFrameBoundExp::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

Utf8String FirstWindowFrameBoundExp::_ToString() const
    {
    return "";
    }

Exp::FinalizeParseStatus SecondWindowFrameBoundExp::_FinalizeParsing(ECSqlParseContext &, FinalizeParseMode)
    {
    return FinalizeParseStatus::Completed;
    }

bool SecondWindowFrameBoundExp::_TryDetermineParameterExpType(ECSqlParseContext &, ParameterExp &) const
    {
    return false;
    }

void SecondWindowFrameBoundExp::_ToECSql(ECSqlRenderContext &) const
    {
    }

void SecondWindowFrameBoundExp::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

Utf8String SecondWindowFrameBoundExp::_ToString() const
    {
    return "";
    }


END_BENTLEY_SQLITE_EC_NAMESPACE