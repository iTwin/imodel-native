/*--------------------------------------------------------------------------------------+
|
|     $Source: src/persistence/IECStatement.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "ECPersistence/IECStatement.h"

BEGIN_BENTLEY_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IECStatement::Prepare (Utf8CP ecsql)
    {
    return _Prepare (ecsql);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECStatement::IsPrepared () const
    {
    return _IsPrepared ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IECStatement::BindBoolean (int parameterIndex, bool value)
    {
    return _BindBoolean (parameterIndex, value);
    }
    
/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IECStatement::BindDateTime (int parameterIndex, const SystemTime& value)
    {
    return _BindDateTime (parameterIndex, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IECStatement::BindDouble (int parameterIndex, double value)
    {
    return _BindDouble (parameterIndex, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IECStatement::BindInt (int parameterIndex, Int32 value)
    {
    return _BindInt32 (parameterIndex, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IECStatement::BindInt (int parameterIndex, Int64 value)
    {
    return _BindInt64 (parameterIndex, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IECStatement::BindString (int parameterIndex, Utf8CP value, int charCount, CopyArgumentOptions makeCopy)
    {
    return _BindUtf8 (parameterIndex, value, charCount, makeCopy);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IECStatement::BindString (int parameterIndex, WCharCP value, CopyArgumentOptions makeCopy)
    {
    return _BindString (parameterIndex, value, makeCopy);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IECStatement::BindBinary (int parameterIndex, const void* value, int binarySize, CopyArgumentOptions makeCopy)
    {
    return _BindBinary (parameterIndex, value, binarySize, makeCopy);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IECStatement::BindPoint (int parameterIndex, DPoint2dCR value)
    {
    return _BindDPoint2d (parameterIndex, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IECStatement::BindPoint (int parameterIndex, DPoint3dCR value)
    {
    return _BindDPoint3d (parameterIndex, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IECStatement::BindNull (int parameterIndex)
    {
    return _BindNull (parameterIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IECStatement::ClearBindings ()
    {
    return _ClearBindings ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
IECStatement::StepStatus IECStatement::Step (int* rowsAffected)
    {
    return _Step (rowsAffected);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IECStatement::Reset ()
    {
    return _Reset ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
int IECStatement::GetPropertyCount () const
    {
    return _GetPropertyCount ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
ECPropertyCP IECStatement::GetProperty (int propertyIndex) const
    {
    return _GetProperty (propertyIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WStringCP IECStatement::GetInstanceId () const
    {
    return _GetInstanceId ();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECStatement::IsNull (int propertyIndex) const
    {
    return _IsNull (propertyIndex);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECStatement::GetBooleanValue (int propertyIndex, ValueStatus* valueStatus) const
    {
    return _GetBooleanValue (propertyIndex, valueStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
SystemTimeCP IECStatement::GetDateTimeValue (int propertyIndex, ValueStatus* valueStatus) const
    {
    return _GetDateTimeValue (propertyIndex, valueStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Int32 IECStatement::GetInt32Value (int propertyIndex, ValueStatus* valueStatus) const
    {
    return _GetInt32Value (propertyIndex, valueStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Int64 IECStatement::GetInt64Value (int propertyIndex, ValueStatus* valueStatus) const
    {
    return _GetInt64Value (propertyIndex, valueStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
double IECStatement::GetDoubleValue (int propertyIndex, ValueStatus* valueStatus) const
    {
    return _GetDoubleValue (propertyIndex, valueStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP IECStatement::GetStringValue (int propertyIndex, ValueStatus* valueStatus) const
    {
    return _GetStringValue (propertyIndex, valueStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP IECStatement::GetUtf8Value (int propertyIndex, ValueStatus* valueStatus) const
    {
    return _GetUtf8Value (propertyIndex, valueStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
const void* IECStatement::GetBinaryValue (int propertyIndex, int& binarySize, ValueStatus* valueStatus) const
    {
    return _GetBinaryValue (propertyIndex, binarySize, valueStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2dCP IECStatement::GetDPoint2dValue (int propertyIndex, ValueStatus* valueStatus) const
    {
    return _GetDPoint2dValue (propertyIndex, valueStatus);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3dCP IECStatement::GetDPoint3dValue (int propertyIndex, ValueStatus* valueStatus) const
    {
    return _GetDPoint3dValue (propertyIndex, valueStatus);
    }


END_BENTLEY_EC_NAMESPACE
