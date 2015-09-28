/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/InsertStatementExp.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "InsertStatementExp.h"

using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//****************** InsertStatementExp *************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
InsertStatementExp::InsertStatementExp (unique_ptr<ClassNameExp>& classNameExp, unique_ptr<PropertyNameListExp>& propertyNameListExp, unique_ptr<ValueExpListExp>& valuesExp)
    : Exp (), m_isOriginalPropertyNameListUnset (propertyNameListExp == nullptr || propertyNameListExp->GetChildrenCount() == 0)
    {
    m_classNameExpIndex = AddChild (move(classNameExp));
    
    if (propertyNameListExp == nullptr)
        {
        propertyNameListExp = unique_ptr<PropertyNameListExp> (new PropertyNameListExp());
        }

    m_propertyNameListExpIndex = AddChild (move(propertyNameListExp));
    m_valuesExpIndex = AddChild (move(valuesExp));
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

                unique_ptr<RangeClassRefList> classList = unique_ptr<RangeClassRefList>(new RangeClassRefList());
                classList->push_back(classNameExp);
                m_finalizeParsingArgCache = move(classList);
                ctx.PushFinalizeParseArg(m_finalizeParsingArgCache.get());

                auto propNameListExp = GetPropertyNameListExpP();
                if (IsOriginalPropertyNameListUnset())
                    {
                    auto addDelegate = [&propNameListExp] (unique_ptr<PropertyNameExp>& propNameExp)
                        {
                        //ECInstanceId is treated separately
                        if (!propNameExp->GetPropertyMap().IsECInstanceIdPropertyMap())
                            propNameListExp->AddPropertyNameExp(propNameExp);
                        };

                    if (SUCCESS != classNameExp->CreatePropertyNameExpList(addDelegate))
                        return FinalizeParseStatus::Error;

                    }

                return FinalizeParseStatus::NotCompleted;
                }

            case Exp::FinalizeParseMode::AfterFinalizingChildren:
                {
                ctx.PopFinalizeParseArg();
                m_finalizeParsingArgCache = nullptr;

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
            auto propNameExp = propNameListExp->GetPropertyNameExp(i);
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
Exp::FinalizeParseStatus InsertStatementExp::Validate (ECSqlParseContext& ctx) const
    {
    PropertyNameListExp const* propertyNameListExp = GetPropertyNameListExp ();
    const size_t expectedValueCount = propertyNameListExp->GetChildrenCount();

    ValueExpListExp const* valuesExp = GetValuesExp();
    if (valuesExp->GetChildrenCount() != expectedValueCount)
        {
        ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Mismatching number of items in VALUES clause.");
        return FinalizeParseStatus::Error;
        }

    if (propertyNameListExp->GetSpecialTokenExpIndexMap().IsUnset(ECSqlSystemProperty::ECInstanceId))
        {
        ClassNameExp const* classNameExp = GetClassNameExp();
        if (classNameExp->GetInfo().GetMap().IsECInstanceIdAutogenerationDisabled())
            {
            ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "ECSQL INSERT must always specify the ECInstanceId as ECInstanceId auto-generation is disabled for the ECClass (via custom attribute): %s", ToECSql().c_str());
            return FinalizeParseStatus::Error;
            }
        }

    for (size_t i = 0; i < expectedValueCount; i++)
        {
        ValueExp const* valueExp = valuesExp->GetValueExp(i);
        switch (valueExp->GetType())
            {
                case Exp::Type::PropertyName:
                case Exp::Type::SetFunctionCall:
                    ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Expression '%s' is not allowed in the VALUES clause of the INSERT statement.", valueExp->ToECSql().c_str());
                    return FinalizeParseStatus::Error;

                case Exp::Type::Parameter:
                    continue; //parameters are handled in the last step
                default:
                    break;
            }

        PropertyNameExp const* propertyNameExp = propertyNameListExp->GetPropertyNameExp(i);

        Utf8String errorMessage;
        if (!propertyNameExp->GetTypeInfo().Matches(valueExp->GetTypeInfo(), &errorMessage))
            {
            ctx.GetIssueReporter().Report(ECDbIssueSeverity::Error, "Type mismatch in INSERT statement: %s", errorMessage.c_str());
            return FinalizeParseStatus::Error;
            }

        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ClassNameExp const* InsertStatementExp::GetClassNameExp () const
    {
    return GetChild<ClassNameExp> (m_classNameExpIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyNameListExp const* InsertStatementExp::GetPropertyNameListExp() const
    {
    return GetChild<PropertyNameListExp> (m_propertyNameListExpIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyNameListExp* InsertStatementExp::GetPropertyNameListExpP() const
    {
    return GetChildP<PropertyNameListExp> (m_propertyNameListExpIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ValueExpListExp const* InsertStatementExp::GetValuesExp() const
    {
    return GetChild<ValueExpListExp>(m_valuesExpIndex);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8String InsertStatementExp::_ToECSql() const 
    {
    Utf8String ecsql ("INSERT INTO ");

    ecsql.append (GetClassNameExp ()->ToECSql());

    if (!IsOriginalPropertyNameListUnset())
        {
        auto propertyNameListExp = GetPropertyNameListExp();
        ecsql.append (" ").append (propertyNameListExp->ToECSql());
        }

    ecsql.append (" VALUES (").append (GetValuesExp()->ToECSql()).append (")");

    return ecsql;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE

