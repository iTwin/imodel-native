/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECInstanceECSqlSelectAdapter::ECInstanceECSqlSelectAdapter(ECSqlStatement const &ecSqlStatement) : m_ecSqlStatement(ecSqlStatement)
    {
    if (!m_ecSqlStatement.IsPrepared())
        return;

    m_isValid = CreateColumnHandlers() == SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceECSqlSelectAdapter::GetInstanceId(ECInstanceId& id) const
    {
    if (!m_isValid)
        return ERROR;

    ECDbSystemSchemaHelper const& systemSchemaHelper = m_ecSqlStatement.GetECDb()->Schemas().Main().GetSystemSchemaHelper();
    for (int i = 0; i < m_ecSqlStatement.GetColumnCount(); i++)
        {
        ECPropertyCP prop = m_ecSqlStatement.GetColumnInfo(i).GetProperty();
        if (systemSchemaHelper.GetSystemPropertyInfo(*prop) == ECSqlSystemPropertyInfo::ECInstanceId())
            {
            id = m_ecSqlStatement.GetValueId<ECInstanceId>(i);
            return SUCCESS;
            }
        }
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstancePtr ECInstanceECSqlSelectAdapter::GetInstance(ECN::ECClassId ecClassid) const
    {
    if (!m_isValid)
        return nullptr;

    /* Create instance */
    ECClassCP ecClass = m_ecSqlStatement.GetECDb()->Schemas().GetClass(ecClassid);
    if (ecClass == nullptr)
        return nullptr;

    ECN::IECInstancePtr instance = ECInstanceAdapterHelper::CreateECInstance(*ecClass);
    if (SUCCESS != SetInstanceData(*instance, true))
        return nullptr;

    return instance;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
ECN::IECInstancePtr ECInstanceECSqlSelectAdapter::GetInstance() const
    {
    if (!m_isValid)
        return nullptr;

    if (!m_isSingleClassSelectClause)
        {
        LOG.error("Can only call ECInstanceECSqlSelectAdapter::GetInstance() for an ECSQL select clause made up properties from a single ECClass.");
        BeAssert(false && "Can only call ECInstanceECSqlSelectAdapter::GetInstance() for an ECSQL select clause made up properties from a single ECClass.");
        return nullptr;
        }

    ECN::ECClassCP ecClass = nullptr;
    if (-1 != m_ecClassIdColumnIndex)
        {
        IECSqlValue const& value = m_ecSqlStatement.GetValue(m_ecClassIdColumnIndex);
        ecClass = m_ecSqlStatement.GetECDb()->Schemas().GetClass(value.GetId<ECClassId>());
        }
    else
        {
        for (int i = 0; i < m_ecSqlStatement.GetColumnCount(); i++)
            {
            ECSqlColumnInfo const& columnInfo = m_ecSqlStatement.GetColumnInfo(i);
            if (columnInfo.IsGeneratedProperty())
                continue;

            ecClass = &columnInfo.GetRootClass().GetClass();
            break;
            }
        }

    if (nullptr == ecClass)
        {
        LOG.debugv("ECInstanceECSqlSelectAdapter::GetInstance - failed to get ecClass from %s", m_ecSqlStatement.GetECSql());
        return nullptr;
        }

    ECN::IECInstancePtr instance = ECInstanceAdapterHelper::CreateECInstance(*ecClass);
    if (SUCCESS != SetInstanceData(*instance, false))
        return nullptr;

    return instance;
    }

//----------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-
 BentleyStatus ECInstanceECSqlSelectAdapter::GetInstance(ECN::IECInstanceR ecInstance) const
    {
    if (!m_isValid)
        return ERROR;

    return SetInstanceData(ecInstance, true);
    }
 
/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceECSqlSelectAdapter::SetInstanceData(IECInstanceR instance, bool usesClassIdFilter) const
    {
    ECClassCR ecClass = instance.GetClass();
    for (int i = 0; i < m_ecSqlStatement.GetColumnCount(); i++)
        {
        IECSqlValue const& value = m_ecSqlStatement.GetValue(i);
        if (usesClassIdFilter && ecClass.GetPropertyP(value.GetColumnInfo().GetProperty()->GetName().c_str()) == nullptr)
            continue;

        if (nullptr != m_columnHandlers[i])
            {
            ColumnHandler handler = m_columnHandlers[i];
            if (SUCCESS != (this->*handler)(instance, value))
                return ERROR;
            }
        }

    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECInstanceECSqlSelectAdapter::SetPropertyData(IECInstanceR instance, Utf8CP parentPropertyAccessString, IECSqlValue const& value) const
    {
    ECSqlColumnInfo const& columnInfo = value.GetColumnInfo();
    ECPropertyCP prop = columnInfo.GetProperty();
    BeAssert(prop != nullptr && "TODO: ECInstanceECSqlSelectAdapter expects that GetColumnInfo().GetProperty is never null. This is not the case for prim arrays. Please double-check the code.");

    if (columnInfo.IsGeneratedProperty())
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
        ECObjectsStatus ecStatus = instance.SetInternalValue(accessString.c_str(), ecVal);
        if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
            {
            BeAssert(false);
            LOG.debugv("ECInstanceECSqlSelectAdapter::SetPropertyData - failed to set primitive value for property %s.  Error (%d)", accessString.c_str(), ecStatus);
            return ERROR;
            }

        return SUCCESS;
        }

    if (prop->GetIsStruct())
        {
        for (IECSqlValue const& memberVal : value.GetStructIterable())
            {
            if (SUCCESS != SetPropertyData(instance, accessString.c_str(), memberVal))
                {
                LOG.debugv("ECInstanceECSqlSelectAdapter::SetPropertyData - failed to set struct property %s", accessString.c_str());
                return ERROR;
                }
            }

        return SUCCESS;
        }

    if (prop->GetIsArray())
        {
        const int arrayLength = value.GetArrayLength();
        if (arrayLength <= 0)
            return SUCCESS;

        instance.AddArrayElements(accessString.c_str(), arrayLength);
        int arrayIndex = 0;
        for (IECSqlValue const& arrayElementValue : value.GetArrayIterable())
            {
            if (prop->GetIsStructArray())
                {
                if (SUCCESS != SetStructArrayElement(ecVal, prop->GetAsStructArrayProperty()->GetStructElementType(), arrayElementValue))
                    {
                    LOG.debugv("ECInstanceECSqlSelectAdapter::SetPropertyData - failed to set struct array element (%d) on property %s", arrayIndex, accessString.c_str());
                    return ERROR;
                    }
                }
            else if (prop->GetIsPrimitiveArray())
                {
                if (SUCCESS != SetPrimitiveValue(ecVal, prop->GetAsPrimitiveArrayProperty()->GetPrimitiveElementType(), arrayElementValue))
                    {
                    LOG.debugv("ECInstanceECSqlSelectAdapter::SetPropertyData - failed to set primitive array element (%d) on property %s", arrayIndex, accessString.c_str());
                    return ERROR;
                    }
                }

            ECObjectsStatus ecStatus = instance.SetValue(accessString.c_str(), ecVal, arrayIndex);
            if (ecStatus != ECObjectsStatus::Success && ecStatus != ECObjectsStatus::PropertyValueMatchesNoChange)
                {
                BeAssert(false);
                LOG.debugv("ECInstanceECSqlSelectAdapter::SetPropertyData - failed to set array value (%d) for property %s.  Error (%d)", arrayIndex, accessString.c_str(), ecStatus);
                return ERROR;
                }
            arrayIndex++;
            }

        return SUCCESS;
        }

    BeAssert(false);
    return ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
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
            case ECN::PRIMITIVETYPE_IGeometry:
            {
            int size = 0;
            const Byte* b = (const Byte *) value.GetBlob(&size);

            if (primitiveType == PRIMITIVETYPE_Binary)
                val.SetBinary(b, (size_t) size, false);
            else
                val.SetIGeometry(b, (size_t) size, false);

            break;
            }
            case ECN::PRIMITIVETYPE_Point2d:
            {
            DPoint2d d = value.GetPoint2d();
            val.SetPoint2d(d);
            break;
            }
            case ECN::PRIMITIVETYPE_Point3d:
            {
            DPoint3d d = value.GetPoint3d();
            val.SetPoint3d(d);
            break;
            }
            case ECN::PRIMITIVETYPE_DateTime:
            {
            DateTime::Info metadata;
            const uint64_t jdMsec = value.GetDateTimeJulianDaysMsec(metadata);
            const int64_t ceTicks = DateTime::JulianDayToCommonEraMilliseconds(jdMsec) * 10000;
            val.SetDateTimeTicks(ceTicks, metadata);
            break;
            }
        }

    return SUCCESS;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceECSqlSelectAdapter::SetStructArrayElement(ECValueR val, ECClassCR structType, IECSqlValue const& value) const
    {
    val.Clear();
    if (value.IsNull())
        {
        val.SetStruct(nullptr); // ECValue API seems to need this call to know that this is a struct value. Just Clear doesn't seem to suffice
        return SUCCESS;
        }

    IECInstancePtr structInstance = structType.GetDefaultStandaloneEnabler()->CreateInstance();
    for (IECSqlValue const& memberVal : value.GetStructIterable())
        {
        if (SUCCESS != SetPropertyData(*structInstance, memberVal))
            return ERROR;
        }

    val.SetStruct(structInstance.get());
    return SUCCESS;
    }

//--------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BentleyStatus ECInstanceECSqlSelectAdapter::SetNavigationValue(IECInstanceR instance, IECSqlValue const& value) const
    {
    if (value.IsNull())
        return SUCCESS;

    ECSqlColumnInfo const& columnInfo = value.GetColumnInfo();
    if (!columnInfo.GetDataType().IsNavigation())
        {
        BeAssert(false);
        return ERROR;
        }

    ECPropertyCP prop = columnInfo.GetProperty();
    BeAssert(prop != nullptr && prop->GetIsNavigation() && "TODO: ECInstanceECSqlSelectAdapter expects that GetColumnInfo().GetProperty is never null. This is not the case for prim arrays. Please double-check the code.");

    ECClassId relClassId;
    ECInstanceId navId = value.GetNavigation<ECInstanceId>(&relClassId);
    ECValue navValue;
    if (relClassId.IsValid())
        navValue = ECValue(navId, relClassId);
    else
        navValue = ECValue(navId);

    return instance.SetValue(prop->GetName().c_str(), navValue) == ECObjectsStatus::Success ? SUCCESS : ERROR;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
IECInstancePtr ECInstanceECSqlSelectAdapter::FindRelationshipEndpoint(ECInstanceId endpointInstanceId, ECN::ECClassId endpointClassId,
                                                                      ECN::StandaloneECRelationshipInstance* relationshipInstance, bool isSource) const
    {
    IECInstancePtr instance;

    ECClassCP endpointClass = m_ecSqlStatement.GetECDb()->Schemas().GetClass(endpointClassId);
    if (nullptr == endpointClass)
        return instance;

    Utf8String ecsql("SELECT * FROM ");
    ecsql.append(endpointClass->GetECSqlName()).append(" WHERE ECInstanceId=?");
    ECSqlStatement statement;
    ECSqlStatus status = statement.Prepare(*(m_ecSqlStatement.GetECDb()), ecsql.c_str());
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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceECSqlSelectAdapter::SetInstanceId(ECN::IECInstanceR instance, IECSqlValue const& value) const
    {
    return ECInstanceAdapterHelper::SetECInstanceId(instance, value.GetId<ECInstanceId>());
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceECSqlSelectAdapter::SetRelationshipSource(ECN::IECInstanceR instance, IECSqlValue const& value) const
    {
    if (m_sourceECClassIdColumnIndex < 0)
        {
        LOG.errorv("ECInstanceECSqlSelectAdapter cannot return ECRelationship instances because the SELECT clause does not include SourceECClassId. ECSQL: %s", m_ecSqlStatement.GetECSql());
        return ERROR;
        }

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
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceECSqlSelectAdapter::SetRelationshipTarget(ECN::IECInstanceR instance, IECSqlValue const& value) const
    {
    if (m_targetECClassIdColumnIndex < 0)
        {
        LOG.errorv("ECInstanceECSqlSelectAdapter cannot return ECRelationship instances because the SELECT clause does not include TargetECClassId. ECSQL: %s", m_ecSqlStatement.GetECSql());
        return ERROR;
        }

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

/*---------------------------------------------------------------------------------**//**
* @bsimethod
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ECInstanceECSqlSelectAdapter::CreateColumnHandlers()
    {
    if (!m_ecSqlStatement.IsPrepared())
        return ERROR;

    bool isSingleClassSelectClause = true;
    ECClassCP targetClassInSelectClause = nullptr;
    ECDbSystemSchemaHelper const& systemSchemaHelper = m_ecSqlStatement.GetECDb()->Schemas().Main().GetSystemSchemaHelper();
    for (int i = 0; i < m_ecSqlStatement.GetColumnCount(); i++)
        {
        ECSqlColumnInfo const& columnInfo = m_ecSqlStatement.GetColumnInfo(i);
        ECPropertyCP prop = columnInfo.GetProperty();
        ECSqlSystemPropertyInfo const& sysPropInfo = systemSchemaHelper.GetSystemPropertyInfo(*prop);
        if (!sysPropInfo.IsSystemProperty())
            {
            if (prop->GetIsNavigation())
                m_columnHandlers.push_back(&ECInstanceECSqlSelectAdapter::SetNavigationValue);
            else
                {
                m_columnHandlers.push_back(&ECInstanceECSqlSelectAdapter::SetPropertyData);

                if (isSingleClassSelectClause)
                    {
                    if (!columnInfo.IsGeneratedProperty())
                        {
                        if (targetClassInSelectClause == nullptr)
                            targetClassInSelectClause = &columnInfo.GetRootClass().GetClass();
                        else
                            isSingleClassSelectClause = ECInstanceAdapterHelper::Equals(*targetClassInSelectClause, columnInfo.GetRootClass().GetClass());
                        }
                    else
                        isSingleClassSelectClause = false;
                    }
                }
            continue;
            }

        switch (sysPropInfo.GetType())
            {
                case ECSqlSystemPropertyInfo::Type::Class:
                {
                switch (sysPropInfo.GetClass())
                    {
                        case ECSqlSystemPropertyInfo::Class::ECInstanceId:
                        {
                        m_columnHandlers.push_back(&ECInstanceECSqlSelectAdapter::SetInstanceId);
                        continue;
                        }

                        case ECSqlSystemPropertyInfo::Class::ECClassId:
                        {
                        m_columnHandlers.push_back(nullptr);
                        m_ecClassIdColumnIndex = i;
                        continue;
                        }

                        default:
                            BeAssert(false);
                            return ERROR;
                    }
                }

                case ECSqlSystemPropertyInfo::Type::Relationship:
                {
                switch (sysPropInfo.GetRelationship())
                    {
                        case ECSqlSystemPropertyInfo::Relationship::SourceECInstanceId:
                        {
                        m_columnHandlers.push_back(&ECInstanceECSqlSelectAdapter::SetRelationshipSource);
                        continue;
                        }
                        case ECSqlSystemPropertyInfo::Relationship::SourceECClassId:
                        {
                        m_columnHandlers.push_back(nullptr);
                        m_sourceECClassIdColumnIndex = i;
                        continue;
                        }
                        case ECSqlSystemPropertyInfo::Relationship::TargetECInstanceId:
                        {
                        m_columnHandlers.push_back(&ECInstanceECSqlSelectAdapter::SetRelationshipTarget);
                        continue;
                        }
                        case ECSqlSystemPropertyInfo::Relationship::TargetECClassId:
                        {
                        m_columnHandlers.push_back(nullptr);
                        m_targetECClassIdColumnIndex = i;
                        continue;
                        }

                        default:
                            BeAssert(false);
                            return ERROR;


                    }
                case ECSqlSystemPropertyInfo::Type::Navigation:
                {
                LOG.error("ECInstanceECSqlSelectAdapter does not support members of a NavigationECProperty in the SELECT clause. Only the NavigationECProperty itself may be specified in the SELECT clause.");
                return ERROR;
                }
                }
            }
        }

    m_isSingleClassSelectClause = isSingleClassSelectClause;
    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
