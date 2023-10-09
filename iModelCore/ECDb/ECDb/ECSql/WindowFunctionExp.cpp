/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "WindowFunctionExp.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Exp::FinalizeParseStatus WindowFunctionExp::_FinalizeParsing(ECSqlParseContext & ctx, FinalizeParseMode mode)
    {
    if (mode == FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    SetTypeInfo(GetWindowFunctionType()->GetAsCP<FunctionCallExp>()->GetTypeInfo());
    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowFunctionExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql(*GetWindowFunctionType());
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
void WindowFunctionExp::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String WindowFunctionExp::_ToString() const
    {
    return Utf8String();
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowFunctionType::_ToECSql(ECSqlRenderContext &) const
    {
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowFunctionType::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String WindowFunctionType::_ToString() const
    {
    return "?";
    }

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
void WindowSpecification::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String WindowSpecification::_ToString() const
    {
    return "?";
    }

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
void WindowPartitionColumnReferenceListExp::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
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

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowPartitionColumnReferenceExp::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

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
void FilterClauseExp::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowFrameClauseExp::_ToECSql(ECSqlRenderContext &ctx) const
    {
    switch (GetWindowFrameUnit())
        {
        case WindowFrameClauseExp::WindowFrameUnit::Groups:
            ctx.AppendToECSql("GROUPS ");
            break;
        case WindowFrameClauseExp::WindowFrameUnit::Range:
            ctx.AppendToECSql("RANGE ");
            break;
        case WindowFrameClauseExp::WindowFrameUnit::Rows:
            ctx.AppendToECSql("ROWS ");
            break;
        }
    
    if (WindowFrameBetweenExp const * e = GetWindowFrameBetweenExp())
        ctx.AppendToECSql(*e);
    else if (WindowFrameStartExp const * e = GetWindowFrameStartExp())
        ctx.AppendToECSql(*e);
    
    switch (GetWindowFrameExclusionType())
        {
        case WindowFrameClauseExp::WindowFrameExclusionType::ExcludeCurrentRow:
            ctx.AppendToECSql(" EXCLUDE CURRENT ROW");
            break;
        case WindowFrameClauseExp::WindowFrameExclusionType::ExcludeGroup:
            ctx.AppendToECSql(" EXCLUDE GROUP");
            break;
        case WindowFrameClauseExp::WindowFrameExclusionType::ExcludeNoOthers:
            ctx.AppendToECSql(" EXCLUDE NO OTHERS");
            break;
        case WindowFrameClauseExp::WindowFrameExclusionType::ExcludeTies:
            ctx.AppendToECSql(" EXCLUDE TIES");
            break;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowFrameClauseExp::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String WindowFrameClauseExp::_ToString() const
    {
    return "";
    }

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
void WindowFrameStartExp::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String WindowFrameStartExp::_ToString() const
    {
    return "";
    }

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
void WindowFrameBetweenExp::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String WindowFrameBetweenExp::_ToString() const
    {
    return "";
    }

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
void FirstWindowFrameBoundExp::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String FirstWindowFrameBoundExp::_ToString() const
    {
    return "";
    }

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
void SecondWindowFrameBoundExp::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String SecondWindowFrameBoundExp::_ToString() const
    {
    return "";
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowFunctionClauseExp::_ToECSql(ECSqlRenderContext &) const
    {
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowFunctionClauseExp::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String WindowFunctionClauseExp::_ToString() const
    {
    return "";
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowDefinitionExp::_ToECSql(ECSqlRenderContext &) const
    {
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowDefinitionExp::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String WindowDefinitionExp::_ToString() const
    {
    return "";
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowDefinitionListExp::_ToECSql(ECSqlRenderContext &) const
    {
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void WindowDefinitionListExp::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Utf8String WindowDefinitionListExp::_ToString() const
    {
    return "";
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
