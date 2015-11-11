/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ColumnInfo.cpp $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ColumnInfo ColumnInfo::Create(ECN::ECPropertyCR ecProperty, Utf8CP propertyAccessString)
    {
    ColumnInfo info;
    if (SUCCESS != info.Initialize(ecProperty, propertyAccessString))
        return ColumnInfo();

    return std::move(info);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ColumnInfo::ColumnInfo () 
: m_isValid(false), m_nullable (true), m_unique (false), m_columnType(PRIMITIVETYPE_Binary), //default column type
m_collation (ECDbSqlColumn::Constraint::Collation::Default)
    {}

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
BentleyStatus ColumnInfo::Initialize(ECPropertyCR ecProperty, Utf8CP propertyAccessString)
    {
    // Assumes that if it is mapped to a column and is not primitive, it is mapped as a blob. Needswork if we add a JSON mapping hint
    PrimitiveECPropertyCP primitiveProperty = ecProperty.GetAsPrimitiveProperty();
    if (primitiveProperty)
        m_columnType = primitiveProperty->GetType();
    else
        m_columnType = PRIMITIVETYPE_Binary; //In this case, it is the type of the column, not of the property


    ECDbPropertyMap customPropMap;
    if (ECDbMapCustomAttributeHelper::TryGetPropertyMap(customPropMap, ecProperty))
        {
        if (ECObjectsStatus::Success != customPropMap.TryGetColumnName(m_columnName))
            return ERROR;

        if (ECObjectsStatus::Success != customPropMap.TryGetIsNullable(m_nullable))
            return ERROR;

        if (ECObjectsStatus::Success != customPropMap.TryGetIsUnique(m_unique))
            return ERROR;

        Utf8String collationStr;
        if (ECObjectsStatus::Success != customPropMap.TryGetCollation(collationStr))
            return ERROR;

        if (!ECDbSqlColumn::Constraint::TryParseCollationString(m_collation, collationStr.c_str()))
            {
            LOG.errorv("Custom attribute PropertyMap on ECProperty %s:%s has an invalid value for the property 'Collation': %s",
                       ecProperty.GetClass().GetFullName(), ecProperty.GetName().c_str(),
                       collationStr.c_str());
            return ERROR;
            }
        }

    // PropertyMappingRule: if custom attribute PropertyMap does not supply a column name for an ECProperty, 
    // we use the ECProperty's propertyAccessString (and replace . by _)
    if (Utf8String::IsNullOrEmpty(m_columnName.c_str()))
        {
        m_columnName.assign (propertyAccessString);
        m_columnName.ReplaceAll(".", "_");
        BeAssert(m_columnName.find(".") == Utf8String::npos);
        }

    //WIP_ECDB: needswork: handle DoNotMap
    m_isValid = true;
    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
