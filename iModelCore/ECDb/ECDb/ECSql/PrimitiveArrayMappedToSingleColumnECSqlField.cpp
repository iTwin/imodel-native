/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/PrimitiveArrayMappedToSingleColumnECSqlField.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

using namespace std;

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//*********************** PrimitiveArrayMappedToSingleColumnECSqlField *******************

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      07/2013
//---------------------------------------------------------------------------------------
PrimitiveArrayMappedToSingleColumnECSqlField::PrimitiveArrayMappedToSingleColumnECSqlField (ECSqlStatementBase& ecsqlStatement, ECSqlColumnInfo&& ecsqlColumnInfo, int sqliteColumnIndex, ECClassCR primitiveArraySystemClass)
: ECSqlField (ecsqlStatement, move (ecsqlColumnInfo), true, true), m_primitiveArraySystemClass (primitiveArraySystemClass), m_sqliteColumnIndex (sqliteColumnIndex), m_arrayElement(*ecsqlStatement.GetECDb())
    {
    //for empty arrays we cache some information so that we don't have to compute it for each step
    m_emptyArrayValueECInstance = m_primitiveArraySystemClass.GetDefaultStandaloneEnabler()->CreateInstance();
    ECValue arrayMetaInfo;
    if (m_emptyArrayValueECInstance->GetValue(arrayMetaInfo, 1) != ECObjectsStatus::ECOBJECTS_STATUS_Success)
        {
        ReportError(ECSqlStatus::Error, "Could not retrieve array information from array ECInstance.");
        BeAssert(false && "Could not retrieve array information from array ECInstance.");
        return;
        }

    m_emptyArrayInfo = arrayMetaInfo.GetArrayInfo();

    if (m_ecsqlColumnInfo.GetDataType().GetPrimitiveType() == PRIMITIVETYPE_DateTime)
        {
        auto property = m_ecsqlColumnInfo.GetProperty();
        BeAssert(property != nullptr && "ColumnInfo::GetProperty can return null. Please double-check");
        DateTimeInfo dateTimeInfo;
        if (StandardCustomAttributeHelper::GetDateTimeInfo(dateTimeInfo, *property) != ECOBJECTS_STATUS_Success)
            {
            ReportError(ECSqlStatus::Error, "Retrieving DateTimeInfo custom attribute from corresponding ECProperty failed.");
            BeAssert(false && "Retrieving DateTimeInfo custom attribute from corresponding ECProperty failed.");
            return;
            }

        m_datetimeMetadata = dateTimeInfo.GetInfo(true);
        }

    m_arrayElement.Init (m_ecsqlColumnInfo);
    Reset ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      07/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayMappedToSingleColumnECSqlField::_Init()
    {
    Reset();

    Byte* arrayBlob = (Byte*) GetSqliteStatement().GetValueBlob(m_sqliteColumnIndex);
    const int arrayBlobSize = GetSqliteStatement().GetColumnBytes(m_sqliteColumnIndex);

    const bool isEmptyArray = arrayBlob == nullptr;
    if (!isEmptyArray)
        {
        if (!ECDBuffer::IsCompatibleVersion(nullptr, arrayBlob))
            {
            BeAssert(false && "BLOB is from a future version that thinks it is not compatible with us");
            return ReportError(ECSqlStatus::Error, "BLOB is from a future version that thinks it is not compatible with us");
            }

        //Initialize ECInstance from blob
        m_arrayValueECInstance = m_primitiveArraySystemClass.GetDefaultStandaloneEnabler()->CreateSharedInstance(arrayBlob, arrayBlobSize);
        if (!m_arrayValueECInstance.IsValid())
            {
            BeAssert(false && "Shared ECInstance created from array BLOB is nullptr.");
            return ReportError(ECSqlStatus::Error, "Shared ECInstance created from array BLOB is nullptr.");
            }

        //Get array information 
        ECValue arrayMetaInfo;
        if (m_arrayValueECInstance->GetValue(arrayMetaInfo, 1) != ECObjectsStatus::ECOBJECTS_STATUS_Success)
            {
            BeAssert(false && "Could not retrieve array information from array ECInstance.");
            return ReportError(ECSqlStatus::Error, "Could not retrieve array information from array ECInstance.");
            }

        m_arrayInfo = arrayMetaInfo.GetArrayInfo();
        }
    else
        {
        //array is empty.
        m_arrayValueECInstance = m_emptyArrayValueECInstance;
        m_arrayInfo = m_emptyArrayInfo;
        }

    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      10/2013
//---------------------------------------------------------------------------------------
ECSqlStatus PrimitiveArrayMappedToSingleColumnECSqlField::_Reset ()
    {
    DoReset ();
    return ECSqlStatus::Success;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
void PrimitiveArrayMappedToSingleColumnECSqlField::DoReset () const
    {
    m_arrayElement.Reset ();
    m_currentArrayIndex = -1;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      07/2013
//---------------------------------------------------------------------------------------
bool PrimitiveArrayMappedToSingleColumnECSqlField::_IsNull () const
    {
    return false; //arrays are always considered to be not-null (seems to be EC contract)
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      07/2013
//---------------------------------------------------------------------------------------
void PrimitiveArrayMappedToSingleColumnECSqlField::_MoveNext (bool onInitializingIterator) const
    {
    if (onInitializingIterator)
        DoReset ();

    m_arrayElement.Reset ();

    m_currentArrayIndex++;

    if (_IsAtEnd ())
        return;

    BeAssert (m_currentArrayIndex >= 0);

    auto ecInstance = GetArrayValueECInstance ();
    BeAssert (ecInstance != nullptr);
    m_arrayElement.SetValue (*ecInstance, (uint32_t) m_currentArrayIndex, m_datetimeMetadata);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
bool PrimitiveArrayMappedToSingleColumnECSqlField::_IsAtEnd () const
    {
    return m_currentArrayIndex >= _GetArrayLength ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlValue const* PrimitiveArrayMappedToSingleColumnECSqlField::_GetCurrent () const
    {
    BeAssert (m_currentArrayIndex >= 0 && m_currentArrayIndex < _GetArrayLength ());
    return &m_arrayElement;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      07/2013
//---------------------------------------------------------------------------------------
int PrimitiveArrayMappedToSingleColumnECSqlField::_GetArrayLength () const
    {
    return m_arrayInfo.GetCount ();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlPrimitiveValue const& PrimitiveArrayMappedToSingleColumnECSqlField::_GetPrimitive () const
    {
    ReportError (ECSqlStatus::Error, "GetPrimitive cannot be called for array value. Call GetArray instead.");
    return NoopECSqlValue::GetSingleton ().GetPrimitive ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
IECSqlStructValue const& PrimitiveArrayMappedToSingleColumnECSqlField::_GetStruct () const
    {
    ReportError (ECSqlStatus::Error, "GetStruct cannot be called for array value. Call GetArray instead.");
    return NoopECSqlValue::GetSingleton ().GetStruct ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      07/2013
//---------------------------------------------------------------------------------------
IECInstanceCP PrimitiveArrayMappedToSingleColumnECSqlField::GetArrayValueECInstance () const
    {
    return m_arrayValueECInstance.get ();
    }


//*********************** PrimitiveArrayMappedToSingleColumnECSqlField::ArrayElementValue *******************
//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
void PrimitiveArrayMappedToSingleColumnECSqlField::ArrayElementValue::Init (ECSqlColumnInfoCR parentColumnInfo)
    {
    m_columnInfo = ECSqlColumnInfo::CreateForArrayElement (parentColumnInfo, -1);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
BentleyStatus PrimitiveArrayMappedToSingleColumnECSqlField::ArrayElementValue::SetValue 
(
IECInstanceCR instance, 
uint32_t arrayIndex, 
DateTime::Info const& dateTimeMetadata
)
    {
    auto status = instance.GetValue (m_value, 1, arrayIndex);
    if (status != ECOBJECTS_STATUS_Success)
        {
        BeAssert (false && "PrimitiveArrayMappedToSingleColumnECSqlField::ArrayElementValue::SetValue> Failed to read array value from ECInstance.");
        return ERROR;
        }

    m_columnInfo.GetPropertyPathR ().SetLeafArrayIndex (arrayIndex);

    //As the ECInstance refers to a helper ECClass for primitive arrays which is not aware of DateTimeInfo CA on the actual
    //target ECProperty, the ECValues don't have DateTime metadata (in case they are of the DateTime type).
    //The metadata will therefore be added in a separate step further down.
    if (!m_value.IsNull () && m_value.IsDateTime ())
        {
        if (SUCCESS != m_value.SetDateTimeTicks (m_value.GetDateTimeTicks (), dateTimeMetadata))
            {
            BeAssert (false && "PrimitiveArrayMappedToSingleColumnECSqlField::ArrayElementValue::SetValue> Failed to set date time ticks with date time metadata in ECValue");
            return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
bool PrimitiveArrayMappedToSingleColumnECSqlField::ArrayElementValue::_IsNull () const
    {
    return m_value.IsNull ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      07/2013
//---------------------------------------------------------------------------------------
void const* PrimitiveArrayMappedToSingleColumnECSqlField::ArrayElementValue::_GetBinary (int* binarySize) const
    {
    if (!CanRead(PRIMITIVETYPE_Binary))
        return NoopECSqlValue::GetSingleton ().GetBinary (binarySize);

    size_t size;
    auto data = m_value.GetBinary(size);
    if (binarySize)
        *binarySize = (int)size;
    return data;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      07/2013
//---------------------------------------------------------------------------------------
bool PrimitiveArrayMappedToSingleColumnECSqlField::ArrayElementValue::_GetBoolean () const
    {
    if (!CanRead(PRIMITIVETYPE_Boolean))
        return NoopECSqlValue::GetSingleton ().GetBoolean ();

    return m_value.GetBoolean ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      07/2013
//---------------------------------------------------------------------------------------
uint64_t PrimitiveArrayMappedToSingleColumnECSqlField::ArrayElementValue::_GetDateTimeJulianDaysHns (DateTime::Info& metadata) const
    {
    if (!CanRead(PRIMITIVETYPE_DateTime))
        return NoopECSqlValue::GetSingleton ().GetDateTimeJulianDaysHns(metadata);

    bool hasMetadata = false;
    const int64_t ceTicks = m_value.GetDateTimeTicks (hasMetadata, metadata);
    return DateTime::CommonEraTicksToJulianDay (ceTicks);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      09/2015
//---------------------------------------------------------------------------------------
double PrimitiveArrayMappedToSingleColumnECSqlField::ArrayElementValue::_GetDateTimeJulianDays(DateTime::Info& metadata) const
    {
    const uint64_t jdHns = _GetDateTimeJulianDaysHns(metadata);
    return DateTime::HnsToRationalDay(jdHns);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      07/2013
//---------------------------------------------------------------------------------------
double PrimitiveArrayMappedToSingleColumnECSqlField::ArrayElementValue::_GetDouble () const
    {
    if (!CanRead(PRIMITIVETYPE_Double))
        return NoopECSqlValue::GetSingleton ().GetDouble();

    return m_value.GetDouble ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      07/2013
//---------------------------------------------------------------------------------------
int PrimitiveArrayMappedToSingleColumnECSqlField::ArrayElementValue::_GetInt () const
    {
    if (!CanRead(PRIMITIVETYPE_Integer))
        return NoopECSqlValue::GetSingleton ().GetInt();

    return m_value.GetInteger ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      07/2013
//---------------------------------------------------------------------------------------
int64_t PrimitiveArrayMappedToSingleColumnECSqlField::ArrayElementValue::_GetInt64 () const
    {
    if (!CanRead(PRIMITIVETYPE_Long))
        return NoopECSqlValue::GetSingleton ().GetInt64();

    return m_value.GetLong ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      07/2013
//---------------------------------------------------------------------------------------
Utf8CP PrimitiveArrayMappedToSingleColumnECSqlField::ArrayElementValue::_GetText () const
    {
    if (!CanRead(PRIMITIVETYPE_String))
        return NoopECSqlValue::GetSingleton ().GetText();

    return m_value.GetUtf8CP ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      07/2013
//---------------------------------------------------------------------------------------
DPoint2d PrimitiveArrayMappedToSingleColumnECSqlField::ArrayElementValue::_GetPoint2D () const
    {
    if (!CanRead(PRIMITIVETYPE_Point2D))
        return NoopECSqlValue::GetSingleton ().GetPoint2D();

    return m_value.GetPoint2D();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      07/2013
//---------------------------------------------------------------------------------------
DPoint3d PrimitiveArrayMappedToSingleColumnECSqlField::ArrayElementValue::_GetPoint3D () const
    {
    if (!CanRead(PRIMITIVETYPE_Point3D))
        return NoopECSqlValue::GetSingleton ().GetPoint3D();

    return m_value.GetPoint3D ();
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2014
//---------------------------------------------------------------------------------------
IGeometryPtr PrimitiveArrayMappedToSingleColumnECSqlField::ArrayElementValue::_GetGeometry() const
    {
    if (!CanRead(PRIMITIVETYPE_IGeometry))
        return NoopECSqlValue::GetSingleton().GetGeometry();

    return m_value.GetIGeometry();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      11/2014
//---------------------------------------------------------------------------------------
void const* PrimitiveArrayMappedToSingleColumnECSqlField::ArrayElementValue::_GetGeometryBlob (int* blobSize) const
    {
    if (!CanRead (PRIMITIVETYPE_IGeometry))
        return NoopECSqlValue::GetSingleton ().GetGeometryBlob (blobSize);

    //ECObjects handles geometries as blobs, and as primitive arrays in ECDb are stored as an ECInstance,
    //we can call _GetBinary to retrieve the geometry
    size_t size;
    auto data = m_value.GetBinary (size);
    if (blobSize)
        *blobSize = (int) size;
    return data;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      03/2014
//---------------------------------------------------------------------------------------
ECSqlColumnInfoCR PrimitiveArrayMappedToSingleColumnECSqlField::ArrayElementValue::_GetColumnInfo () const
    {
    return m_columnInfo;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      07/2013
//---------------------------------------------------------------------------------------
IECSqlStructValue const& PrimitiveArrayMappedToSingleColumnECSqlField::ArrayElementValue::_GetStruct () const
    {
    m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "GetStruct cannot be called for array element. Call GetPrimitive instead.");
    return NoopECSqlValue::GetSingleton ().GetStruct ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      07/2013
//---------------------------------------------------------------------------------------
IECSqlArrayValue const& PrimitiveArrayMappedToSingleColumnECSqlField::ArrayElementValue::_GetArray () const
    {
    m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "GetArray cannot be called for array element. Call GetPrimitive instead.");
    return NoopECSqlValue::GetSingleton ().GetArray ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Affan.Khan      07/2013
//---------------------------------------------------------------------------------------
void PrimitiveArrayMappedToSingleColumnECSqlField::ArrayElementValue::Reset ()
    {
    m_value.Clear ();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                Krischan.Eberle      10/2013
//---------------------------------------------------------------------------------------
bool PrimitiveArrayMappedToSingleColumnECSqlField::ArrayElementValue::CanRead (PrimitiveType requestedType) const
    {
    if (requestedType != m_columnInfo.GetDataType ().GetPrimitiveType ())
        {
        m_ecdb.GetECDbImplR().GetIssueReporter().Report(ECDbIssueSeverity::Error, "For primitive array elements only the GetXXX method which directly matches the array element type can be called.");
        return false;
        }

    return true;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
