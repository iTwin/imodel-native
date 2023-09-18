/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

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
    ctx.AppendToECSql(" OVER ");
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
        ctx.AppendToECSql("(ORDER BY ECInstanceId) ");
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

void WindowPartitionExp::_ToECSql(ECSqlRenderContext &ctx) const
    {
    }

void WindowPartitionExp::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

Exp::FinalizeParseStatus WindowPartitionExp::_FinalizeParsing(ECSqlParseContext &, FinalizeParseMode)
    {
    return FinalizeParseStatus::Completed;
    }

void WindowPartitionColumnReferenceListExp::_ToECSql(ECSqlRenderContext & ctx) const
    {
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
    }

void WindowPartitionColumnReferenceExp::_ToJson(BeJsValue, JsonFormat const &) const
    {
    }

Exp::FinalizeParseStatus WindowPartitionColumnReferenceExp::_FinalizeParsing(ECSqlParseContext &, FinalizeParseMode)
    {
    return FinalizeParseStatus::Completed;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE