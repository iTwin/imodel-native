/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECSqlStatementNoopImpls.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECSqlStatementNoopImpls.h"
#include "ECSqlStatementImpl.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//****************** NoopECSqlBinderFactory **************************
//static member initialization
std::map<ECSqlStatus, std::unique_ptr<NoopECSqlBinder>> NoopECSqlBinderFactory::s_flyweightBinderMap;

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      05/2013
//---------------------------------------------------------------------------------------
//static
NoopECSqlBinder& NoopECSqlBinderFactory::GetBinder (ECSqlStatus status)
    {
    //insert returns a pair where the first element is an iterator pointing to the inserted / existing element in the map.
    //the second element indicates whether the pair was newly inserted or whether it already existed.
    s_flyweightBinderMap[status] = std::unique_ptr<NoopECSqlBinder>(new NoopECSqlBinder(status));
    //auto ret = s_flyweightBinderMap.insert (kvPair);
    return *s_flyweightBinderMap[status];//*ret.first->second;
    }


//****************** NoopECSqlValue **************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
//static member initialization
NoopECSqlValue NoopECSqlValue::s_singleton;

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
NoopECSqlValue::NoopECSqlValue ()
: IECSqlValue (), IECSqlPrimitiveValue (), IECSqlStructValue (), IECSqlArrayValue ()
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      12/2013
//---------------------------------------------------------------------------------------
NoopECSqlValue::~NoopECSqlValue ()
    {}

END_BENTLEY_SQLITE_EC_NAMESPACE
