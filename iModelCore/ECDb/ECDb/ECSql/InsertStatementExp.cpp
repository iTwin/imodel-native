/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//****************** InsertStatementExp *************************
//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
InsertStatementExp::InsertStatementExp(std::unique_ptr<ClassNameExp>& classNameExp, std::unique_ptr<PropertyNameListExp>& propertyNameListExp, std::vector<std::unique_ptr<ValueExp>>& valueExpList)
    : Exp(Type::Insert), m_isOriginalPropertyNameListUnset(propertyNameListExp == nullptr || propertyNameListExp->GetChildrenCount() == 0)
    {
    m_classNameExpIndex = AddChild(std::move(classNameExp));

    if (propertyNameListExp == nullptr)
        propertyNameListExp = std::make_unique<PropertyNameListExp>();

    m_propertyNameListExpIndex = AddChild(std::move(propertyNameListExp));
    m_valuesExpIndex = AddChild(std::make_unique<ValueExpListExp>(valueExpList));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus InsertStatementExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    switch (mode)
        {
            case FinalizeParseMode::BeforeFinalizingChildren:
            {
            ClassNameExp const* classNameExp = GetClassNameExp();
            if (classNameExp == nullptr)
                {
                BeAssert(false && "ClassNameExp expected to be not null for InsertStatementExp");
                return FinalizeParseStatus::Error;
                }

            if (classNameExp->GetMemberFunctionCallExp() != nullptr)
                {
                ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "May not call function on class in a INSERT statement: %s", ToECSql().c_str());
                return FinalizeParseStatus::Error;
                }


            std::vector<RangeClassInfo> classList;
            classList.push_back(RangeClassInfo(*classNameExp, RangeClassInfo::Scope::Local));
            m_rangeClassRefExpCache = std::move(classList);
            ctx.PushArg(std::make_unique<ECSqlParseContext::RangeClassArg>(m_rangeClassRefExpCache));

            PropertyNameListExp* propNameListExp = GetPropertyNameListExpP();
            if (IsOriginalPropertyNameListUnset())
                {
                if (!classNameExp->HasMetaInfo())
                    {
                    BeAssert(false && "ClassNameExp has not been assigned the ClassMap yet.");
                    return FinalizeParseStatus::Error;
                    }

                ClassMap const& classMap = classNameExp->GetInfo().GetMap();
                for (PropertyMap const* propertyMap : classMap.GetPropertyMaps())
                    {
                    //ECInstanceId, ECClassId are treated separately, SourceECClassId/TargetECClassId is never persisted
                    const PropertyMap::Type propMapKind = propertyMap->GetType();
                    if (propMapKind == PropertyMap::Type::ECInstanceId || propMapKind == PropertyMap::Type::ECClassId || propMapKind == PropertyMap::Type::ConstraintECClassId)
                        continue;

                    std::unique_ptr<PropertyNameExp> exp = std::make_unique<PropertyNameExp>(ctx, propertyMap->GetAccessString(), *classNameExp, classMap);
                    propNameListExp->AddPropertyNameExp(exp);
                    }
                }

            return FinalizeParseStatus::NotCompleted;
            }

            case Exp::FinalizeParseMode::AfterFinalizingChildren:
                {
                ctx.PopArg();
                m_rangeClassRefExpCache.clear();

                return Validate(ctx);
                }

            default:
                BeAssert(false);
                return FinalizeParseStatus::Error;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
bool InsertStatementExp::_TryDetermineParameterExpType(ECSqlParseContext& ctx, ParameterExp& parameterExp) const
    {
    const size_t valueExpCount = GetValuesExp()->GetChildrenCount();
    ValueExpListExp const* valuesExp = GetValuesExp();
    PropertyNameListExp const* propNameListExp = GetPropertyNameListExp();
    for (size_t i = 0; i < valueExpCount; i++)
        {
        ValueExp const* valueExp = valuesExp->GetValueExp(i);
        if (&parameterExp == valueExp)
            {
            BeAssert(valueExp->IsParameterExp());
            PropertyNameExp const* propNameExp = propNameListExp->GetPropertyNameExp(i);
            BeAssert(propNameExp != nullptr);
            parameterExp.SetTargetExpInfo(*propNameExp);
            return true;
            }
        }

    return false;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus InsertStatementExp::Validate(ECSqlParseContext& ctx) const
    {
    PropertyNameListExp const* propertyNameListExp = GetPropertyNameListExp();
    const size_t expectedValueCount = propertyNameListExp->GetChildrenCount();

    ValueExpListExp const* valuesExp = GetValuesExp();
    if (valuesExp->GetChildrenCount() != expectedValueCount)
        {
        ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "Mismatching number of items in VALUES clause. For %d properties there are %d values.", expectedValueCount, valuesExp->GetChildrenCount());
        return FinalizeParseStatus::Error;
        }

    for (size_t i = 0; i < expectedValueCount; i++)
        {
        ValueExp const* valueExp = valuesExp->GetValueExp(i);
        switch (valueExp->GetType())
            {
                case Exp::Type::PropertyName:
                    ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "Expression '%s' is not allowed in the VALUES clause of the INSERT statement.", valueExp->ToECSql().c_str());
                    return FinalizeParseStatus::Error;

                case Exp::Type::Parameter:
                    continue; //parameters are handled in the last step
                default:
                    break;
            }

        PropertyNameExp const* propertyNameExp = propertyNameListExp->GetPropertyNameExp(i);

        Utf8String errorMessage;
        if (!propertyNameExp->GetTypeInfo().CanCompare(valueExp->GetTypeInfo(), &errorMessage))
            {
            ctx.Issues().ReportV(IssueSeverity::Error, IssueCategory::BusinessProperties, IssueType::ECSQL, "Type mismatch in INSERT statement: %s", errorMessage.c_str());
            return FinalizeParseStatus::Error;
            }

        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
ClassNameExp const* InsertStatementExp::GetClassNameExp() const { return GetChild<ClassNameExp>(m_classNameExpIndex); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyNameListExp const* InsertStatementExp::GetPropertyNameListExp() const { return GetChild<PropertyNameListExp>(m_propertyNameListExpIndex); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyNameListExp* InsertStatementExp::GetPropertyNameListExpP() const { return GetChildP<PropertyNameListExp>(m_propertyNameListExpIndex); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
ValueExpListExp const* InsertStatementExp::GetValuesExp() const { return GetChild<ValueExpListExp>(m_valuesExpIndex); }

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void InsertStatementExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql("INSERT INTO ").AppendToECSql(*GetClassNameExp());

    if (!IsOriginalPropertyNameListUnset())
        ctx.AppendToECSql(" ").AppendToECSql(*GetPropertyNameListExp());

    ctx.AppendToECSql(" VALUES (").AppendToECSql(*GetValuesExp()).AppendToECSql(")");
    }


END_BENTLEY_SQLITE_EC_NAMESPACE

