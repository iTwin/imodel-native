/*--------------------------------------------------------------------------------------+
|
|     $Source: PublicApi/ECPersistence/IECStatement.h $
|
|  $Copyright: (c) 2012 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
/*__PUBLISH_SECTION_START__*/

#include "Bentley/RefCounted.h"

#include <Geom/GeomApi.h>

#include "ECObjects/ECSchema.h"

#include "ECPersistence/ECPersistence.h"

BEGIN_BENTLEY_ECOBJECT_NAMESPACE

enum CopyArgumentOptions
    {
    COPYARGUMENT_No = 0,
    COPYARGUMENT_Yes = 1
    };

typedef RefCountedPtr<IECStatement> IECStatementPtr;

//=======================================================================================    
//! @ingroup ECPersistence
//! IECStatement is used to execute an ECSQL statement. It is also used to read and iterate
//! over the results of the ECSQL execution.
//! Clients create an IECStatement from the corresponding IECConnection instance.
//! @see IECConnection::CreateStatement
//! @bsiclass                                                 Krischan.Eberle      08/2012
//=======================================================================================    
struct IECStatement : RefCountedBase
    {
    public:
    //=======================================================================================    
    //! @ingroup ECPersistence
    //! Contains the possible values returned when executing an IECStatement or when
    //! stepping through the results of an IECStatement.
    //! @see IECStatement::Step
    //! @bsiclass                                                 Krischan.Eberle      08/2012
    //=======================================================================================    
    enum StepStatus
        {
        STEPSTATUS_HasRow = 0,
        STEPSTATUS_Done = 1,
        STEPSTATUS_Error = 2
        };

    //=======================================================================================    
    //! @ingroup ECPersistence
    //! Contains the possible values returned from the GetXXXValue methods of an IECStatement
    //! @bsiclass                                                 Krischan.Eberle      09/2012
    //=======================================================================================    
    enum ValueStatus
        {
        VALUESTATUS_Success = SUCCESS,
        VALUESTATUS_Error = ERROR,
        VALUESTATUS_IsNull = 100,
        VALUESTATUS_Unknown = 101,
        };

private:
    ECPERSISTENCE_EXPORT virtual BentleyStatus _Prepare (Utf8CP ecsql) = 0;
    ECPERSISTENCE_EXPORT virtual bool _IsPrepared () const = 0;

    ECPERSISTENCE_EXPORT virtual BentleyStatus _BindBoolean (int parameterIndex, bool value) = 0;
    ECPERSISTENCE_EXPORT virtual BentleyStatus _BindDateTime (int parameterIndex, const SystemTime& value, CopyArgumentOptions makeCopy) = 0;
    ECPERSISTENCE_EXPORT virtual BentleyStatus _BindDouble (int parameterIndex, double value) = 0;
    ECPERSISTENCE_EXPORT virtual BentleyStatus _BindInteger (int parameterIndex, Int32 value) = 0;
    ECPERSISTENCE_EXPORT virtual BentleyStatus _BindLong (int parameterIndex, Int64 value) = 0;
    ECPERSISTENCE_EXPORT virtual BentleyStatus _BindUtf8 (int parameterIndex, Utf8CP value, int charCount, CopyArgumentOptions makeCopy) = 0;
    ECPERSISTENCE_EXPORT virtual BentleyStatus _BindString (int parameterIndex, WCharCP value, CopyArgumentOptions makeCopy) = 0;
    ECPERSISTENCE_EXPORT virtual BentleyStatus _BindBinary (int parameterIndex, const void* value, int binarySize, CopyArgumentOptions makeCopy) = 0;
    ECPERSISTENCE_EXPORT virtual BentleyStatus _BindPoint2D (int parameterIndex, DPoint2dCR value, CopyArgumentOptions makeCopy) = 0;
    ECPERSISTENCE_EXPORT virtual BentleyStatus _BindPoint3D (int parameterIndex, DPoint3dCR value, CopyArgumentOptions makeCopy) = 0;
    ECPERSISTENCE_EXPORT virtual BentleyStatus _BindNull (int parameterIndex) = 0;
    ECPERSISTENCE_EXPORT virtual BentleyStatus _ClearBindings () = 0;

    ECPERSISTENCE_EXPORT virtual StepStatus _Step (int* rowsAffected) = 0;

    ECPERSISTENCE_EXPORT virtual BentleyStatus _Reset () = 0;

    ECPERSISTENCE_EXPORT virtual int _GetPropertyCount () const = 0;
    ECPERSISTENCE_EXPORT virtual ECPropertyCP _GetProperty (int propertyIndex) const = 0;

    ECPERSISTENCE_EXPORT virtual ValueStatus _GetInstanceId (WStringR instanceId) const = 0;
    ECPERSISTENCE_EXPORT virtual ValueStatus _IsNull (bool& isNull, int propertyIndex) const = 0;
    ECPERSISTENCE_EXPORT virtual ValueStatus _GetBooleanValue (bool& value, int propertyIndex) const = 0;
    ECPERSISTENCE_EXPORT virtual ValueStatus _GetDateTimeValue (SystemTimeR value, int propertyIndex) const = 0;
    ECPERSISTENCE_EXPORT virtual ValueStatus _GetDoubleValue (double& value, int propertyIndex) const = 0;
    ECPERSISTENCE_EXPORT virtual ValueStatus _GetIntegerValue (Int32& value, int propertyIndex) const = 0;
    ECPERSISTENCE_EXPORT virtual ValueStatus _GetLongValue (Int64& value, int propertyIndex) const = 0;
    ECPERSISTENCE_EXPORT virtual ValueStatus _GetStringValue (WCharCP& value, int propertyIndex) const = 0;
    ECPERSISTENCE_EXPORT virtual ValueStatus _GetUtf8Value (Utf8CP& value, int propertyIndex) const = 0;
    ECPERSISTENCE_EXPORT virtual ValueStatus _GetBinaryValue (const void*& value, int& binarySize, int propertyIndex) const = 0;
    ECPERSISTENCE_EXPORT virtual ValueStatus _GetPoint2DValue (DPoint2dR value, int propertyIndex) const = 0;
    ECPERSISTENCE_EXPORT virtual ValueStatus _GetPoint3DValue (DPoint3dR value, int propertyIndex) const = 0;

    static void AssignValueStatus (ValueStatus* valueStatus, const ValueStatus& newValue);
public: 
    ECPERSISTENCE_EXPORT virtual ~IECStatement () {};

    //*** Prepare ***
    ECPERSISTENCE_EXPORT BentleyStatus Prepare (Utf8CP ecsql);
    ECPERSISTENCE_EXPORT bool IsPrepared () const;

    //*** Bind ***
    ECPERSISTENCE_EXPORT BentleyStatus BindBoolean (int parameterIndex, bool value);
    
    //! Bind a date time value to a parameter of this statement
    //! @param[in] parameterIndex Index of the parameter to bind a value to
    //! @param[in] value The value to bind.
    //! @return SUCCESS or ERROR
    //**TODO**: Is SystemTime acceptable as type? It seems independent of anything else in ECObjects. But how can we
    //ensure that it remains independent in future?
    ECPERSISTENCE_EXPORT BentleyStatus BindDateTime (int parameterIndex, const SystemTime& value, CopyArgumentOptions makeCopy);
    ECPERSISTENCE_EXPORT BentleyStatus BindDouble (int parameterIndex, double value);
    ECPERSISTENCE_EXPORT BentleyStatus BindInteger (int parameterIndex, Int32 value);
    ECPERSISTENCE_EXPORT BentleyStatus BindLong (int parameterIndex, Int64 value);
    //TODO: also allow to bind WStrings and WCharP?
    ECPERSISTENCE_EXPORT BentleyStatus BindString (int parameterIndex, Utf8CP value, int charCount, CopyArgumentOptions makeCopy);
    ECPERSISTENCE_EXPORT BentleyStatus BindString (int parameterIndex, WCharCP value, CopyArgumentOptions makeCopy);
    
    //! Bind a binary value to a parameter of this statement
    //! @param[in] parameterIndex Index of the parameter to bind a value to
    //! @param[in] value The value to bind.
    //! @param[in] binarySize The number of bytes in the binary value
    //! @param[in] makeCopy Make a private copy of the binary value. Only pass false if value will remain valid until the statement's bindings are cleared.
    //! @return SUCCESS or ERROR
    ECPERSISTENCE_EXPORT BentleyStatus BindBinary (int parameterIndex, const void* value, int binarySize, CopyArgumentOptions makeCopy);

    ECPERSISTENCE_EXPORT BentleyStatus BindPoint2D (int parameterIndex, DPoint2dCR value, CopyArgumentOptions makeCopy);
    ECPERSISTENCE_EXPORT BentleyStatus BindPoint3D (int parameterIndex, DPoint3dCR value, CopyArgumentOptions makeCopy);

    ECPERSISTENCE_EXPORT BentleyStatus BindNull (int parameterIndex);

    //TODO: Support for geometries? How does the geometry type look like?

    //TODO: With ECPersistence not using ECInstances, we need to find a way to allow clients
    //to bind structs and arrays.

    ECPERSISTENCE_EXPORT BentleyStatus ClearBindings ();

    //*** Step ***
    //! Executes a prepared statement and if the statement returns any results iterates over the result set.
    //! @param[out] rowsAffected Number of rows affected by the statement (only for INSERT, UPDATE, DELETE)
    //! @return STEPSTATUS_HASROW each time a new row of data is ready for processing by the caller (only if statement returned any data (SELECT statements).
    //!         STEPSTATUS_DONE if no further rows exist in the result or if a non-query statement (INSERT, UPDATE, DELETE) was successfully executed.
    //!         STEPSTATUS_ERROR if any error occurred while executing the statement or stepping through the results.
    ECPERSISTENCE_EXPORT StepStatus Step (int* rowsAffected = NULL);

    ECPERSISTENCE_EXPORT BentleyStatus Reset ();

    //*** Read meta data ***
    ECPERSISTENCE_EXPORT int GetPropertyCount () const;

    //Note / TODO: In the future we might need a GetType method that returns the EC type of a returned prop for cases
    //where the returned data does not match an existing ECClass (e.g. select count (*) from Foo). That means in those
    //cases there is no ECProperty (GetProperty should return null). But there still is an ECType.
    //ECPERSISTENCE_EXPORT BentleyStatus GetType (int propertyIndex, ECTypeDescriptor*& ecType) const;

    ECPERSISTENCE_EXPORT ECPropertyCP GetProperty (int propertyIndex) const;


    //Read values
    ECPERSISTENCE_EXPORT WString GetInstanceId (ValueStatus* valueStatus = NULL) const;

    ECPERSISTENCE_EXPORT bool IsNull (int propertyIndex, ValueStatus* valueStatus = NULL) const;

    ECPERSISTENCE_EXPORT bool GetBooleanValue (int propertyIndex, ValueStatus* valueStatus = NULL) const;
    ECPERSISTENCE_EXPORT SystemTime GetDateTimeValue (int propertyIndex, ValueStatus* valueStatus = NULL) const;
    ECPERSISTENCE_EXPORT double GetDoubleValue (int propertyIndex, ValueStatus* valueStatus = NULL) const;
    ECPERSISTENCE_EXPORT Int32 GetIntegerValue (int propertyIndex, ValueStatus* valueStatus = NULL) const;
    ECPERSISTENCE_EXPORT Int64 GetLongValue (int propertyIndex, ValueStatus* valueStatus = NULL) const;
    ECPERSISTENCE_EXPORT WCharCP GetStringValue (int propertyIndex, ValueStatus* valueStatus = NULL) const;
    ECPERSISTENCE_EXPORT Utf8CP GetUtf8Value (int propertyIndex, ValueStatus* valueStatus = NULL) const;
    ECPERSISTENCE_EXPORT const void* GetBinaryValue (int propertyIndex, int& binarySize, ValueStatus* valueStatus = NULL) const;
    ECPERSISTENCE_EXPORT DPoint2d GetPoint2DValue (int propertyIndex, ValueStatus* valueStatus = NULL) const;
    ECPERSISTENCE_EXPORT DPoint3d GetPoint3DValue (int propertyIndex, ValueStatus* valueStatus = NULL) const;

    //TODO: return geometries, structs and arrays
    };

END_BENTLEY_ECOBJECT_NAMESPACE

