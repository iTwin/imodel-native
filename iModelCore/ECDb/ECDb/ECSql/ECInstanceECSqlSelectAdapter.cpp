/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ECInstanceECSqlSelectAdapter.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"
#include "ECInstanceAdapterHelper.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Carole.MacDonald                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceECSqlSelectAdapter::ECInstanceECSqlSelectAdapter(ECSqlStatement const &ecSqlStatement)
: m_ecSqlStatement (ecSqlStatement), m_initialized (false), m_isSingleClassSelectClause (false)
    {
    m_initialized = Initialize();
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Carole.MacDonald                   08/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECInstanceECSqlSelectAdapter::Initialize()
    {
    if (!m_ecSqlStatement.IsPrepared())
        return false;
    m_ecClassIdColumnIndex = -1;
    CreateColumnHandlers();
    return true;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Carole.MacDonald                   10/13
+---------------+---------------+---------------+---------------+---------------+------*/
bool ECInstanceECSqlSelectAdapter::GetInstanceId(ECInstanceId& id) const
    {
    if (!m_initialized)
        return false;
    for (int i = 0; i < m_ecSqlStatement.GetColumnCount(); i++)
        {
        auto prop = m_ecSqlStatement.GetColumnInfo(i).GetProperty();
        if (prop->GetName().Equals("ECInstanceId"))
            {
            id = m_ecSqlStatement.GetValueId<ECInstanceId>(i);
            return true;
            }
        }
    return false;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Carole.MacDonald                   02/14
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstancePtr ECInstanceECSqlSelectAdapter::GetInstance(ECN::ECClassId ecClassid) const
    {
    if (!m_initialized)
        return nullptr;

    /* Create instance */
    ECClassCP ecClass = m_ecSqlStatement.GetECDb()->Schemas().GetECClass(ecClassid);
    if (ecClass == nullptr)
        return nullptr;

    ECN::IECInstancePtr instance = ECInstanceAdapterHelper::CreateECInstance(*ecClass);
    if (SUCCESS != SetInstanceData(*instance, true))
        return nullptr;

    return instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Carole.MacDonald                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstancePtr ECInstanceECSqlSelectAdapter::GetInstance() const
    {
    if (!m_initialized)
        return nullptr;

    if (!m_isSingleClassSelectClause)
        {
        LOG.error ("Can only call ECInstanceECSqlSelectAdapter::GetInstance() for an ECSQL select clause made up properties from a single ECClass.");
        BeAssert (false && "Can only call ECInstanceECSqlSelectAdapter::GetInstance() for an ECSQL select clause made up properties from a single ECClass.");
        return nullptr;
        }

    ECN::ECClassCP ecClass = nullptr;
    if (-1 != m_ecClassIdColumnIndex)
        {
        IECSqlValue const& value = m_ecSqlStatement.GetValue (m_ecClassIdColumnIndex);
        ecClass = m_ecSqlStatement.GetECDb()->Schemas().GetECClass(value.GetId<ECClassId>());
        }
    else
        {
        for(int i=0; i < m_ecSqlStatement.GetColumnCount(); i++)
            {
            auto const& columnInfo = m_ecSqlStatement.GetColumnInfo (i);
            if (columnInfo.IsGeneratedProperty())
                continue;
            ecClass = &(columnInfo.GetRootClass());
            break;
            }
        }

    if (nullptr == ecClass)
        return nullptr;

    ECN::IECInstancePtr instance = ECInstanceAdapterHelper::CreateECInstance(*ecClass);
    if (SUCCESS != SetInstanceData (*instance, false))
        return nullptr;

    return instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Carole.MacDonald                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
void ECInstanceECSqlSelectAdapter::CreateColumnHandlers()
    {
    bool isSingleClassSelectClause = true;
    ECClassCP targetClassInSelectClause = nullptr;
    for (int i = 0; i < m_ecSqlStatement.GetColumnCount (); i++)
        {
        IECSqlValue const& value = m_ecSqlStatement.GetValue (i);
        auto const& columnInfo = value.GetColumnInfo ();
        auto prop = columnInfo.GetProperty ();
        if (prop->GetName().Equals("ECInstanceId"))
            {
            m_columnHandlers.push_back(&ECInstanceECSqlSelectAdapter::SetInstanceId);
            }
        else if (prop->GetName().Equals("SourceECInstanceId"))
            {
            m_columnHandlers.push_back(&ECInstanceECSqlSelectAdapter::SetRelationshipSource);
            }
        else if (prop->GetName().Equals("SourceECClassId"))
            {
            m_columnHandlers.push_back(nullptr);
            m_sourceECClassIdColumnIndex = i;
            }
        else if (prop->GetName().Equals("TargetECInstanceId"))
            {
            m_columnHandlers.push_back(&ECInstanceECSqlSelectAdapter::SetRelationshipTarget);
            }
        else if (prop->GetName().Equals("TargetECClassId"))
            {
            m_columnHandlers.push_back(nullptr);
            m_targetECClassIdColumnIndex = i;
            }
        else if (prop->GetName().Equals("ECClassId"))
            {
            m_columnHandlers.push_back(nullptr);
            m_ecClassIdColumnIndex = i;
            }
        else
            {
            m_columnHandlers.push_back (&ECInstanceECSqlSelectAdapter::SetPropertyData);

            if (isSingleClassSelectClause)
                {
                if (!columnInfo.IsGeneratedProperty ())
                    {
                    if (targetClassInSelectClause == nullptr)
                        targetClassInSelectClause = &columnInfo.GetRootClass ();
                    else
                        isSingleClassSelectClause = ECInstanceAdapterHelper::Equals(*targetClassInSelectClause, columnInfo.GetRootClass());
                    }
                else
                    isSingleClassSelectClause = false;
                }
            }
        }

    m_isSingleClassSelectClause = isSingleClassSelectClause;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Carole.MacDonald                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceECSqlSelectAdapter::SetInstanceData(IECInstanceR instance, bool usesClassIdFilter) const
    {
    auto const& ecClass = instance.GetClass();
    for (int i = 0; i < m_ecSqlStatement.GetColumnCount(); i++)
        {
        IECSqlValue const& value = m_ecSqlStatement.GetValue(i);
        if (usesClassIdFilter && ecClass.GetPropertyP(value.GetColumnInfo().GetProperty()->GetName().c_str()) == nullptr)
            continue;

        if (nullptr != m_columnHandlers[i])
            {
            ColumnHandler handler = m_columnHandlers[i];
            const auto stat = (this->*handler)(instance, value);
            if (stat != SUCCESS)
                return stat;
            }
        }

    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Krischan.Eberle                   08/14
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECInstanceECSqlSelectAdapter::SetPropertyData(IECInstanceR instance, IECSqlValue const& value) const
    {
    return SetPropertyData(instance, nullptr, value);
    }

//--------------------------------------------------------------------------------------
// @bsimethod                                   Carole.MacDonald                 03/14
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECInstanceECSqlSelectAdapter::SetPropertyData(IECInstanceR instance, Utf8CP parentPropertyAccessString, IECSqlValue const& value) const
    {
    auto const& columnInfo = value.GetColumnInfo();
    auto prop = columnInfo.GetProperty();
    BeAssert(prop != nullptr && "TODO: ECInstanceECSqlSelectAdapter expects that GetColumnInfo ().GetProperty is never null. This is not the case for prim arrays. Please double-check the code.");

    if (columnInfo.IsGeneratedProperty() || prop->IsCalculated()) // WIP_ECSQL: is this true? do we need to set for last calculated and other scenarios?
        return SUCCESS;

    Utf8String accessString;
    if (!Utf8String::IsNullOrEmpty(parentPropertyAccessString))
        {
        accessString = parentPropertyAccessString;
        accessString.append(".");
        }

    accessString.append(prop->GetName());

    ECN::ECValue ecVal;
    if (prop->GetIsPrimitive())
        {
        auto primitiveProp = prop->GetAsPrimitiveProperty();
        SetPrimitiveValue(ecVal, primitiveProp->GetType(), value);
        ECObjectsStatus ecStatus = instance.SetValue(accessString.c_str(), ecVal);
        if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
            {
            BeAssert(false);
            return ERROR;
            }
        }
    else if (prop->GetIsStruct())
        {
        IECSqlStructValue const& structValue = value.GetStruct();
        int memberCount = structValue.GetMemberCount();
        for (int i = 0; i < memberCount; i++)
            {
            if (SUCCESS != SetPropertyData(instance, accessString.c_str(), structValue.GetValue(i)))
                return ERROR;
            }
        }
    else if (prop->GetIsArray())
        {
        auto arrayProperty = prop->GetAsArrayProperty();
        auto structArrayProperty = prop->GetAsStructArrayProperty();
        IECSqlArrayValue const& arrayValue = value.GetArray();
        int arrayLength = arrayValue.GetArrayLength();
        if (arrayLength <= 0)
            return SUCCESS;

        instance.AddArrayElements(accessString.c_str(), arrayLength);
        int arrayIndex = 0;
        for (IECSqlValue const* arrayElementValue : arrayValue)
            {
            if (nullptr != structArrayProperty)
                {
                if (SUCCESS != SetStructArrayElement(ecVal, *structArrayProperty->GetStructElementType(), *arrayElementValue))
                    return ERROR;
                }
            else if (arrayProperty->GetKind() == ARRAYKIND_Primitive)
                {
                if (SUCCESS != SetPrimitiveValue(ecVal, arrayProperty->GetPrimitiveElementType(), *arrayElementValue))
                    return ERROR;
                }

            ECObjectsStatus ecStatus = instance.SetValue(accessString.c_str(), ecVal, arrayIndex);
            if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
                {
                BeAssert(false);
                return ERROR;
                }
            arrayIndex++;
            }
        }
    else if (prop->GetIsNavigation())
        {
        NavigationECPropertyCP navProp = prop->GetAsNavigationProperty();
        PrimitiveType navPropIdType = navProp->GetType();
        if (!navProp->IsMultiple())
            {
            SetPrimitiveValue(ecVal, navPropIdType, value);
            ECObjectsStatus ecStatus = instance.SetValue(accessString.c_str(), ecVal);
            return (ecStatus == ECObjectsStatus::Success || ecStatus == ECObjectsStatus::PropertyValueMatchesNoChange) ? SUCCESS : ERROR;
            }

        IECSqlArrayValue const& arrayValue = value.GetArray();
        int arrayLength = arrayValue.GetArrayLength();
        if (arrayLength <= 0)
            return SUCCESS;

        instance.AddArrayElements(accessString.c_str(), arrayLength);
        int arrayIndex = 0;
        for (IECSqlValue const* arrayElementValue : arrayValue)
            {
            if (SUCCESS != SetPrimitiveValue(ecVal, navPropIdType, *arrayElementValue))
                return ERROR;

            ECObjectsStatus ecStatus = instance.SetValue(accessString.c_str(), ecVal, arrayIndex);
            if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
                {
                BeAssert(false);
                return ERROR;
                }
            arrayIndex++;
            }

        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Carole.MacDonald                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceECSqlSelectAdapter::SetPrimitiveValue(ECValueR val, ECN::PrimitiveType primitiveType, IECSqlValue const& value) const
    {
    if (value.IsNull())
        {
        val.SetToNull();
        return SUCCESS;
        }

    switch (primitiveType)
        {
            case ECN::PRIMITIVETYPE_Integer:
            {
            auto intValue = value.GetInt();
            val.SetInteger(intValue);
            break;
            }
            case ECN::PRIMITIVETYPE_String:
            {
            auto str = value.GetText();
            val.SetUtf8CP(str);
            break;
            }
            case ECN::PRIMITIVETYPE_Long:
            {
            auto intValue = value.GetInt64();
            val.SetLong(intValue);
            break;
            }
            case ECN::PRIMITIVETYPE_Double:
            {
            auto doubleValue = value.GetDouble();
            val.SetDouble(doubleValue);
            break;
            }
            case ECN::PRIMITIVETYPE_Boolean:
            {
            auto boolValue = value.GetBoolean();
            val.SetBoolean(boolValue);
            break;
            }
            case ECN::PRIMITIVETYPE_Binary:
            {
            int size = 0;
            const Byte* b = (const Byte *) value.GetBinary(&size);
            val.SetBinary(b, size, false);
            break;
            }
            case ECN::PRIMITIVETYPE_Point2D:
            {
            auto d = value.GetPoint2D();
            val.SetPoint2D(d);
            break;
            }
            case ECN::PRIMITIVETYPE_Point3D:
            {
            auto d = value.GetPoint3D();
            val.SetPoint3D(d);
            break;
            }
            case ECN::PRIMITIVETYPE_DateTime:
            {
            DateTime::Info metadata;
            const uint64_t jdHns = value.GetDateTimeJulianDaysHns(metadata);
            const int64_t ceTicks = DateTime::JulianDayToCommonEraTicks(jdHns);
            val.SetDateTimeTicks(ceTicks, metadata);
            break;
            }
            case ECN::PRIMITIVETYPE_IGeometry:
            {
            int bgfbSize = -1;
            void const* bgfb = value.GetGeometryBlob(&bgfbSize);
            val.SetBinary(static_cast<Byte const*> (bgfb), (size_t) bgfbSize, false);
            break;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Carole.MacDonald                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceECSqlSelectAdapter::SetStructArrayElement(ECValueR val, ECClassCR structType, IECSqlValue const& value) const
    {
    val.Clear();
    auto structInstance = structType.GetDefaultStandaloneEnabler()->CreateInstance();

    IECSqlStructValue const& structValue = value.GetStruct();
    int memberCount = structValue.GetMemberCount();
    for (int i = 0; i < memberCount; i++)
        {
        if (SUCCESS != SetPropertyData(*structInstance, structValue.GetValue(i)))
            return ERROR;
        }

    val.SetStruct(structInstance.get());
    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Carole.MacDonald                   09/13
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr ECInstanceECSqlSelectAdapter::FindRelationshipEndpoint(ECInstanceId endpointInstanceId, ECN::ECClassId endpointClassId,
                        ECN::StandaloneECRelationshipInstance* relationshipInstance, bool isSource) const
    {
    IECInstancePtr instance;

    ECClassCP endpointClass = m_ecSqlStatement.GetECDb()->Schemas().GetECClass(endpointClassId);
    if (nullptr == endpointClass)
        return instance;

    Utf8String ecsql ("SELECT * FROM ");
    ecsql.append (endpointClass->GetECSqlName()).append (" WHERE ECInstanceId=?");
    ECSqlStatement statement;
    ECSqlStatus status = statement.Prepare(*(m_ecSqlStatement.GetECDb ()), ecsql.c_str ());
    if (!status.IsSuccess())
        return instance;

    statement.BindId(1, endpointInstanceId);
    ECInstanceECSqlSelectAdapter endpointAdapter(statement);
    while (BE_SQLITE_ROW == statement.Step())
        {
        instance = endpointAdapter.GetInstance();
        if (instance.IsValid())
            return instance;
        }

    return instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Carole.MacDonald                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceECSqlSelectAdapter::SetInstanceId(ECN::IECInstanceR instance, IECSqlValue const& value) const
    {
    return ECInstanceAdapterHelper::SetECInstanceId (instance, value.GetId<ECInstanceId>());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Carole.MacDonald                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceECSqlSelectAdapter::SetRelationshipSource(ECN::IECInstanceR instance, IECSqlValue const& value) const
    {
    ECN::StandaloneECRelationshipInstance* standaloneRelationship = dynamic_cast<ECN::StandaloneECRelationshipInstance*>(&instance);
    if (nullptr == standaloneRelationship)
        return ERROR;

    IECInstancePtr endpoint = FindRelationshipEndpoint(value.GetId<ECInstanceId>(), m_ecSqlStatement.GetValueId<ECClassId>(m_sourceECClassIdColumnIndex), standaloneRelationship, true);
    if (endpoint.IsValid())
        {
        standaloneRelationship->SetSource(&(*endpoint));
        return SUCCESS;
        }

    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                   Carole.MacDonald                   04/14
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceECSqlSelectAdapter::SetRelationshipTarget(ECN::IECInstanceR instance, IECSqlValue const& value) const
    {
    ECN::StandaloneECRelationshipInstance* standaloneRelationship = dynamic_cast<ECN::StandaloneECRelationshipInstance*>(&instance);
    if (nullptr == standaloneRelationship)
        return ERROR;

    IECInstancePtr endpoint = FindRelationshipEndpoint(value.GetId<ECInstanceId>(), m_ecSqlStatement.GetValueId<ECClassId>(m_targetECClassIdColumnIndex), standaloneRelationship, false);
    if (endpoint.IsValid())
        {
        standaloneRelationship->SetTarget(&(*endpoint));
        return SUCCESS;
        }

    return ERROR;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
