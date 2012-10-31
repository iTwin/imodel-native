/*--------------------------------------------------------------------------------------+
|
|     $Source: src/persistence/IECStatement.cpp $
|
|   $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECObjectsPch.h"

#include "ECPersistence/IECStatement.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

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
BentleyStatus IECStatement::BindDateTime (int parameterIndex, DateTimeCR value, CopyArgumentOptions makeCopy)
    {
    return _BindDateTime (parameterIndex, value, makeCopy);
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
BentleyStatus IECStatement::BindInteger (int parameterIndex, Int32 value)
    {
    return _BindInteger (parameterIndex, value);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IECStatement::BindLong (int parameterIndex, Int64 value)
    {
    return _BindLong (parameterIndex, value);
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
BentleyStatus IECStatement::BindPoint2D (int parameterIndex, DPoint2dCR value, CopyArgumentOptions makeCopy)
    {
    return _BindPoint2D (parameterIndex, value, makeCopy);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus IECStatement::BindPoint3D (int parameterIndex, DPoint3dCR value, CopyArgumentOptions makeCopy)
    {
    return _BindPoint3D (parameterIndex, value, makeCopy);
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
WString IECStatement::GetInstanceId (ValueStatus* valueStatus) const
    {
    WString instanceId;
    ValueStatus status = _GetInstanceId (instanceId);
    AssignValueStatus (valueStatus, status);
    return instanceId;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECStatement::IsNull (int propertyIndex, ValueStatus* valueStatus) const
    {
    bool isNull;
    ValueStatus status = _IsNull (isNull, propertyIndex);
    AssignValueStatus (valueStatus, status);
    return isNull;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
bool IECStatement::GetBooleanValue (int propertyIndex, ValueStatus* valueStatus) const
    {
    bool value;
    ValueStatus status = _GetBooleanValue (value, propertyIndex);
    AssignValueStatus (valueStatus, status);
    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DateTime IECStatement::GetDateTimeValue (int propertyIndex, ValueStatus* valueStatus) const
    {
    DateTime value;
    ValueStatus status = _GetDateTimeValue (value, propertyIndex);
    AssignValueStatus (valueStatus, status);
    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Int32 IECStatement::GetIntegerValue (int propertyIndex, ValueStatus* valueStatus) const
    {
    Int32 value;
    ValueStatus status = _GetIntegerValue (value, propertyIndex);
    AssignValueStatus (valueStatus, status);
    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Int64 IECStatement::GetLongValue (int propertyIndex, ValueStatus* valueStatus) const
    {
    Int64 value;
    ValueStatus status = _GetLongValue (value, propertyIndex);
    AssignValueStatus (valueStatus, status);
    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
double IECStatement::GetDoubleValue (int propertyIndex, ValueStatus* valueStatus) const
    {
    double value;
    ValueStatus status = _GetDoubleValue (value, propertyIndex);
    AssignValueStatus (valueStatus, status);
    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
WCharCP IECStatement::GetStringValue (int propertyIndex, ValueStatus* valueStatus) const
    {
    WCharCP value = NULL;
    ValueStatus status = _GetStringValue (value, propertyIndex);
    AssignValueStatus (valueStatus, status);
    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP IECStatement::GetUtf8Value (int propertyIndex, ValueStatus* valueStatus) const
    {
    Utf8CP value = NULL;
    ValueStatus status = _GetUtf8Value (value, propertyIndex);
    AssignValueStatus (valueStatus, status);
    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
const void* IECStatement::GetBinaryValue (int propertyIndex, int& binarySize, ValueStatus* valueStatus) const
    {
    const void* value = NULL;
    ValueStatus status = _GetBinaryValue (value, binarySize, propertyIndex);
    AssignValueStatus (valueStatus, status);
    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint2d IECStatement::GetPoint2DValue (int propertyIndex, ValueStatus* valueStatus) const
    {
    DPoint2d value;
    ValueStatus status = _GetPoint2DValue (value, propertyIndex);
    AssignValueStatus (valueStatus, status);
    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                08/2012
+---------------+---------------+---------------+---------------+---------------+------*/
DPoint3d IECStatement::GetPoint3DValue (int propertyIndex, ValueStatus* valueStatus) const
    {
    DPoint3d value;
    ValueStatus status = _GetPoint3DValue (value, propertyIndex);
    AssignValueStatus (valueStatus, status);
    return value;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                 Krischan.Eberle                09/2012
+---------------+---------------+---------------+---------------+---------------+------*/
//static
void IECStatement::AssignValueStatus 
(
ValueStatus* valueStatus, 
const ValueStatus& newValue
)
    {
    if (valueStatus != NULL)
        {
        *valueStatus = newValue;
        }
    }

END_BENTLEY_ECOBJECT_NAMESPACE
