/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/StructArrayToSecondaryTableECSqlBinder.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "StructArrayToSecondaryTableECSqlBinder.h"

using namespace std;

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
StructArrayToSecondaryTableECSqlBinder::StructArrayToSecondaryTableECSqlBinder (ECSqlStatementBase& ecsqlStatement, ECSqlTypeInfo const& typeInfo) 
: ECSqlBinder (ecsqlStatement, typeInfo, 0, true, true), IECSqlArrayBinder ()
    {
    BeAssert (GetTypeInfo ().GetKind () == ECSqlTypeInfo::Kind::StructArray);
    m_value = ECSqlParameterValueFactory::CreateArray (*ecsqlStatement.GetECDb(), typeInfo);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
void StructArrayToSecondaryTableECSqlBinder::_SetSqliteIndex (int ecsqlParameterComponentIndex, size_t sqliteIndex)
    {
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlBinder& StructArrayToSecondaryTableECSqlBinder::_AddArrayElement ()
    {
    return m_value->AddArrayElement ();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      08/2013
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayToSecondaryTableECSqlBinder::_BindNull ()
    {
    return m_value->BindNull ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlPrimitiveBinder& StructArrayToSecondaryTableECSqlBinder::_BindPrimitive ()
    {
    return m_value->BindPrimitive ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlStructBinder& StructArrayToSecondaryTableECSqlBinder::_BindStruct ()
    {
    return m_value->BindStruct ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      01/2014
//---------------------------------------------------------------------------------------
IECSqlArrayBinder& StructArrayToSecondaryTableECSqlBinder::_BindArray (uint32_t initialCapacity)
    {
    return m_value->BindArray (initialCapacity);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
void StructArrayToSecondaryTableECSqlBinder::_OnClearBindings ()
    {
    m_value->Clear ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlStatus StructArrayToSecondaryTableECSqlBinder::_OnBeforeStep ()
    {
    //GetArrayLength is cheap on ECSqlParameterValue
    return ArrayConstraintValidator::Validate (GetECDb(), GetTypeInfo (), m_value->GetArray ().GetArrayLength ());
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
