/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/InsertStatementExp.cpp $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//****************** InsertStatementExp *************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/2013
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
// @bsimethod                                    Krischan.Eberle                   11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus InsertStatementExp::_FinalizeParsing(ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    switch (mode)
        {
            case FinalizeParseMode::BeforeFinalizingChildren:
                {
                ClassNameExp const* classNameExp = GetClassNameExp();

                RangeClassInfo::List classList;
                classList.push_back(RangeClassInfo(*classNameExp, RangeClassInfo::Scope::Local));
                m_finalizeParsingArgCache = move(classList);
                ctx.PushArg(std::unique_ptr<ECSqlParseContext::RangeClassArg>(new ECSqlParseContext::RangeClassArg(m_finalizeParsingArgCache)));
                auto propNameListExp = GetPropertyNameListExpP();
                if (IsOriginalPropertyNameListUnset())
                    {
                    auto addDelegate = [&propNameListExp] (std::unique_ptr<PropertyNameExp>& propNameExp)
                        {
                        //ECInstanceId is treated separately
                        const PropertyMap::Type propMapKind = propNameExp->GetPropertyMap().GetType();
                        if (propMapKind != PropertyMap::Type::ECInstanceId && propMapKind != PropertyMap::Type::ECClassId)
                            propNameListExp->AddPropertyNameExp(propNameExp);
                        };

                    if (SUCCESS != classNameExp->CreatePropertyNameExpList(ctx, addDelegate))
                        return FinalizeParseStatus::Error;

                    }

                return FinalizeParseStatus::NotCompleted;
                }

            case Exp::FinalizeParseMode::AfterFinalizingChildren:
                {
                ctx.PopArg();
                m_finalizeParsingArgCache.clear();

                return Validate(ctx);
                }

            default:
                BeAssert(false);
                return FinalizeParseStatus::Error;
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   06/2015
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
// @bsimethod                                    Krischan.Eberle                   11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus InsertStatementExp::Validate(ECSqlParseContext& ctx) const
    {
    PropertyNameListExp const* propertyNameListExp = GetPropertyNameListExp();
    const size_t expectedValueCount = propertyNameListExp->GetChildrenCount();

    ValueExpListExp const* valuesExp = GetValuesExp();
    if (valuesExp->GetChildrenCount() != expectedValueCount)
        {
        ctx.Issues().Report("Mismatching number of items in VALUES clause. For %d properties there are %d values.", expectedValueCount, valuesExp->GetChildrenCount());
        return FinalizeParseStatus::Error;
        }

    for (size_t i = 0; i < expectedValueCount; i++)
        {
        ValueExp const* valueExp = valuesExp->GetValueExp(i);
        switch (valueExp->GetType())
            {
                case Exp::Type::PropertyName:
                    ctx.Issues().Report("Expression '%s' is not allowed in the VALUES clause of the INSERT statement.", valueExp->ToECSql().c_str());
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
            ctx.Issues().Report("Type mismatch in INSERT statement: %s", errorMessage.c_str());
            return FinalizeParseStatus::Error;
            }

        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ClassNameExp const* InsertStatementExp::GetClassNameExp() const { return GetChild<ClassNameExp>(m_classNameExpIndex); }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyNameListExp const* InsertStatementExp::GetPropertyNameListExp() const { return GetChild<PropertyNameListExp>(m_propertyNameListExpIndex); }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyNameListExp* InsertStatementExp::GetPropertyNameListExpP() const { return GetChildP<PropertyNameListExp>(m_propertyNameListExpIndex); }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ValueExpListExp const* InsertStatementExp::GetValuesExp() const { return GetChild<ValueExpListExp>(m_valuesExpIndex); }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void InsertStatementExp::_ToECSql(ECSqlRenderContext& ctx) const
    {
    ctx.AppendToECSql("INSERT INTO ").AppendToECSql(*GetClassNameExp());

    if (!IsOriginalPropertyNameListUnset())
        ctx.AppendToECSql(" ").AppendToECSql(*GetPropertyNameListExp());

    ctx.AppendToECSql(" VALUES (").AppendToECSql(*GetValuesExp()).AppendToECSql(")");
    }


END_BENTLEY_SQLITE_EC_NAMESPACE

