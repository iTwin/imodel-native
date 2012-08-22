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

void IECStatement::Dispose ()
    {
    _Dispose ();
    }

BentleyStatus IECStatement::Prepare (Utf8CP ecsql)
    {
    return _Prepare (ecsql);
    }

bool IECStatement::IsPrepared () const
    {
    return _IsPrepared ();
    }

BentleyStatus IECStatement::BindBoolean (int parameterIndex, bool value)
    {
    return _BindBoolean (parameterIndex, value);
    }
    
BentleyStatus IECStatement::BindDateTime (int parameterIndex, const SystemTime& value)
    {
    return _BindDateTime (parameterIndex, value);
    }

BentleyStatus IECStatement::BindInt (int parameterIndex, Int32 value)
    {
    return _BindInt32 (parameterIndex, value);
    }

BentleyStatus IECStatement::BindInt (int parameterIndex, Int64 value)
    {
    return _BindInt64 (parameterIndex, value);
    }

BentleyStatus IECStatement::BindDouble (int parameterIndex, double value)
    {
    return _BindDouble (parameterIndex, value);
    }

BentleyStatus IECStatement::BindString (int parameterIndex, Utf8StringCR value, bool makeCopy)
    {
    return _BindUtf8String (parameterIndex, value, makeCopy);
    }

BentleyStatus IECStatement::BindString (int parameterIndex, Utf8CP value, int length, bool makeCopy)
    {
    return _BindUtf8 (parameterIndex, value, length, makeCopy);
    }

BentleyStatus IECStatement::BindBinary (int parameterIndex, const void* value, int binarySize, bool makeCopy)
    {
    return _BindBinary (parameterIndex, value, binarySize, makeCopy);
    }

BentleyStatus IECStatement::BindPoint (int parameterIndex, DPoint2dCR value)
    {
    return _BindDPoint2d (parameterIndex, value);
    }

BentleyStatus IECStatement::BindPoint (int parameterIndex, DPoint3dCR value)
    {
    return _BindDPoint3d (parameterIndex, value);
    }

BentleyStatus IECStatement::BindStruct (int parameterIndex, const void* value)
    {
    return _BindStruct (parameterIndex, value);
    }

BentleyStatus IECStatement::BindArray (int parameterIndex, const void* value)
    {
    return _BindArray (parameterIndex, value);
    }

BentleyStatus IECStatement::BindNull (int parameterIndex)
    {
    return _BindNull (parameterIndex);
    }

BentleyStatus IECStatement::ClearBindings ()
    {
    return _ClearBindings ();
    }

ECStepStatus IECStatement::Step (int* rowsAffected)
    {
    return _Step (rowsAffected);
    }

BentleyStatus IECStatement::Reset ()
    {
    return _Reset ();
    }

int IECStatement::GetPropertyCount () const
    {
    return _GetPropertyCount ();
    }

BentleyStatus IECStatement::GetType (int propertyIndex, ECTypeDescriptor*& ecType) const
    {
    return _GetType (propertyIndex, ecType);
    }

BentleyStatus IECStatement::GetProperty (int propertyIndex, ECPropertyP& ecProperty) const
    {
    return _GetProperty (propertyIndex, ecProperty);
    }

BentleyStatus IECStatement::GetClass (int propertyIndex, ECClassP& ecClass) const
    {
    return _GetClass (propertyIndex, ecClass);
    }

WStringCR IECStatement::GetInstanceId () const
    {
    return _GetInstanceId ();
    }

bool IECStatement::IsNull (int propertyIndex) const
    {
    return _IsNull (propertyIndex);
    }

bool IECStatement::GetBooleanValue (int propertyIndex) const
    {
    return _GetBooleanValue (propertyIndex);
    }

SystemTimeCR IECStatement::GetDateTimeValue (int propertyIndex) const
    {
    return _GetDateTimeValue (propertyIndex);
    }

Int32 IECStatement::GetInt32Value (int propertyIndex) const
    {
    return _GetInt32Value (propertyIndex);
    }

Int64 IECStatement::GetInt64Value (int propertyIndex) const
    {
    return _GetInt64Value (propertyIndex);
    }

double IECStatement::GetDoubleValue (int propertyIndex) const
    {
    return _GetDoubleValue (propertyIndex);
    }

Utf8StringCR IECStatement::GetUtf8StringValue (int propertyIndex) const
    {
    return _GetUtf8StringValue (propertyIndex);
    }

Utf8CP IECStatement::GetUtf8Value (int propertyIndex) const
    {
    return _GetUtf8Value (propertyIndex);
    }

const void* IECStatement::GetBinaryValue (int propertyIndex, int& binarySize) const
    {
    return _GetBinaryValue (propertyIndex, binarySize);
    }

DPoint2dCR IECStatement::GetDPoint2dValue (int propertyIndex) const
    {
    return _GetDPoint2dValue (propertyIndex);
    }

DPoint3dCR IECStatement::GetDPoint3dValue (int propertyIndex) const
    {
    return _GetDPoint3dValue (propertyIndex);
    }


END_BENTLEY_EC_NAMESPACE
