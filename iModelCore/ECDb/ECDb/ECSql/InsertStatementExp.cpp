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
InsertStatementExp::InsertStatementExp (unique_ptr<ClassNameExp> classNameExp, unique_ptr<PropertyNameListExp> propertyNameListExp, unique_ptr<ValueExpListExp> valuesExp)
    : Exp (), m_isOriginalPropertyNameListUnset (propertyNameListExp == nullptr || propertyNameListExp->GetChildrenCount() == 0)
    {
    m_classNameExpIndex = AddChild (move(classNameExp));
    
    if (propertyNameListExp == nullptr)
        {
        propertyNameListExp = unique_ptr<PropertyNameListExp> (new PropertyNameListExp ());
        }

    m_propertyNameListExpIndex = AddChild (move (propertyNameListExp));
    m_valuesExpIndex = AddChild (move (valuesExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus InsertStatementExp::_FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == Exp::FinalizeParseMode::BeforeFinalizingChildren)
        {
        auto classNameExp = GetClassNameExp();
        auto classList = unique_ptr<RangeClassRefList> (new RangeClassRefList ());
        classList->push_back (classNameExp);
        m_finalizeParsingArgCache = move (classList);
        ctx.PushFinalizeParseArg (m_finalizeParsingArgCache.get ());

        auto propNameListExp = GetPropertyNameListExpP ();
        if (IsOriginalPropertyNameListUnset())
            {
            auto addDelegate = [&propNameListExp] (unique_ptr<PropertyNameExp>& propNameExp)
                {
                //ECInstanceId is treated separately
                if (!propNameExp->GetPropertyMap ().IsECInstanceIdPropertyMap ())
                    propNameListExp->AddPropertyNameExp (propNameExp);
                };

            if (ECSqlStatus::Success != classNameExp->CreatePropertyNameExpList (ctx, addDelegate))
                return FinalizeParseStatus::Error;

            }

        //Assign the respective prop name exp to each parameter exp in the values clause
        ResolveParameters();

        return FinalizeParseStatus::NotCompleted;
        }
    else
        {
        ctx.PopFinalizeParseArg ();
        m_finalizeParsingArgCache = nullptr;

        return Validate (ctx);
        }
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus InsertStatementExp::Validate (ECSqlParseContext& ctx) const
    {
    size_t expectedValueCount = 0;

    auto propertyNameListExp = GetPropertyNameListExp ();
    expectedValueCount = propertyNameListExp->GetChildrenCount();

    auto valuesExp = GetValuesExp();
    if (valuesExp->GetChildrenCount() != expectedValueCount)
        {
        ctx.SetError(ECSqlStatus::InvalidECSql, "Mismatching number of items in VALUES clause.");
        return FinalizeParseStatus::Error;
        }

    size_t index = 0;
    for (auto childExp : valuesExp->GetChildren ())
        {
        switch (childExp->GetType ())
            {
            case Exp::Type::PropertyName:
            case Exp::Type::SetFunctionCall:
                ctx.SetError (ECSqlStatus::InvalidECSql, "Expression '%s' is not allowed in the VALUES clause of the INSERT statement.", childExp->ToECSql ().c_str ());
                return FinalizeParseStatus::Error;
            default:
                break;
            }

        auto valueExp = static_cast<ValueExp const*> (childExp);
        auto propertyNameExp = propertyNameListExp->GetPropertyNameExp (index);

        Utf8String errorMessage;
        if (!propertyNameExp->GetTypeInfo ().Matches (valueExp->GetTypeInfo(), &errorMessage))
            {
            ctx.SetError(ECSqlStatus::InvalidECSql, "Type mismatch in INSERT statement: %s", errorMessage.c_str ());
            return FinalizeParseStatus::Error;
            }

        index++;
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void InsertStatementExp::ResolveParameters () const
    {
    const size_t valueExpCount = GetValuesExp()->GetChildrenCount();
    auto valuesExp = GetValuesExp();
    auto propNameListExp = GetPropertyNameListExp();
    for (size_t i = 0; i < valueExpCount; i++)
        {
        auto parameterExp = valuesExp->TryGetAsParameterExpP (i);
        if (parameterExp != nullptr)
            {
            auto propNameExp = propNameListExp->GetPropertyNameExp (i);
            BeAssert (propNameExp != nullptr);
            parameterExp->SetTargetExpInfo(*propNameExp);
            }
        }
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

