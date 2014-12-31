/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ListExp.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ListExp.h"
#include "UpdateStatementExp.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

using namespace std;

//****************** AssignmentListExp *************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus AssignmentListExp::_FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    const size_t childrenCount = GetChildrenCount ();
    for (size_t i = 0; i < childrenCount; i++)
        {
        auto assignmentExp = GetAssignmentExp (i);
        auto const& prop = assignmentExp->GetPropertyNameExp ()->GetPropertyMap ().GetProperty ();
        auto systemPropKind = ECSqlSystemProperty::ECInstanceId;
        if (ECDbSystemSchemaHelper::TryGetSystemPropertyKind (systemPropKind, prop))
            m_specialTokenExpIndexMap.AddIndex (systemPropKind, i);
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
void AssignmentListExp::AddAssignmentExp (std::unique_ptr<AssignmentExp> assignmentExp)
    {
    AddChild (move (assignmentExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
AssignmentExp const* AssignmentListExp::GetAssignmentExp (size_t index) const
    {
    return GetChild<AssignmentExp> (index);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       01/2014
//+---------------+---------------+---------------+---------------+---------------+--------
//virtual
Utf8String AssignmentListExp::ToECSql () const
    {
    Utf8String ecsql;

    bool isFirstItem = true;
    for (auto childExp : GetChildren ())
        {
        if (!isFirstItem)
            ecsql.append (", ");

        ecsql.append (childExp->ToECSql ());
        isFirstItem = false;
        }

    return ecsql;
    }




//****************** PropertyNameListExp *************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       12/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus PropertyNameListExp::_FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == FinalizeParseMode::BeforeFinalizingChildren)
        return FinalizeParseStatus::NotCompleted;

    const size_t childrenCount = GetChildrenCount ();
    for (size_t i = 0; i < childrenCount; i++)
        {
        auto propNameExp = GetPropertyNameExp (i);
        ECSqlSystemProperty systemPropKind;
        if (propNameExp->TryGetSystemProperty (systemPropKind))
            m_specialTokenExpIndexMap.AddIndex (systemPropKind, i);
        }

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void PropertyNameListExp::AddPropertyNameExp (std::unique_ptr<PropertyNameExp>& propertyNameExp)
    {
    AddChild (move (propertyNameExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
PropertyNameExp const* PropertyNameListExp::GetPropertyNameExp (size_t index) const
    {
    return GetChild<PropertyNameExp> (index);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//virtual
Utf8String PropertyNameListExp::ToECSql () const
    {
    Utf8String tmp ("(");

    bool isFirstItem = true;
    for(auto childExp : GetChildren ())
        {
        if (!isFirstItem)
            tmp.append(", ");

        tmp.append(childExp->ToECSql ());
        isFirstItem = false;
        }
    tmp.append(")");
    return tmp;
    }


//****************** RowValueConstructorListExp *************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
RowValueConstructorListExp::RowValueConstructorListExp () 
    : Exp ()
    {}

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle                   11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void RowValueConstructorListExp::AddRowValueConstructorExp (std::unique_ptr<ValueExp>& valueExp)
    {
    AddChild (move (valueExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       11/2013
//+---------------+---------------+---------------+---------------+---------------+--------
ParameterExp* RowValueConstructorListExp::TryGetAsParameterExpP (size_t index) const
    {
    auto exp = GetChildP<Exp> (index);
    if (exp->GetType () != Exp::Type::Parameter)
        return nullptr;
        
    return static_cast<ParameterExp*> (exp);
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//virtual
Utf8String RowValueConstructorListExp::ToECSql () const
    {
    Utf8String ecsql;
    bool isFirstItem = true;
    for(auto childExp : GetChildren ())
        {
        if (!isFirstItem)
            ecsql.append(", ");

        ecsql.append(childExp->ToECSql ());
        isFirstItem = false;
        }

    return ecsql;
    }


//*************************** ValueListExp ******************************************
//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
void ValueListExp::AddValueExp (unique_ptr<ValueExp>& valueExp)
    {
    AddChild (move (valueExp));
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
Exp::FinalizeParseStatus ValueListExp::_FinalizeParsing (ECSqlParseContext& ctx, FinalizeParseMode mode)
    {
    if (mode == FinalizeParseMode::BeforeFinalizingChildren)
        //Indicate that the exp per se doesn't have a single type info, because in can vary across it children
        SetTypeInfo (ECSqlTypeInfo (ECSqlTypeInfo::Kind::Varies));

    return FinalizeParseStatus::Completed;
    }

//-----------------------------------------------------------------------------------------
// @bsimethod                                    Krischan.Eberle       08/2013
//+---------------+---------------+---------------+---------------+---------------+--------
//virtual
Utf8String ValueListExp::ToECSql () const
    {
    Utf8String ecsql ("(");

    bool isFirstItem = true;
    for(auto childExp : GetChildren ())
        {
        if (!isFirstItem)
            ecsql.append(", ");

        ecsql.append(childExp->ToECSql ());
        isFirstItem = false;
        }

    ecsql.append(")");
    return ecsql;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
