/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/DbColumn.cpp $
|
|  $Copyright: (c) 2014 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ColumnInfo::ColumnInfo (ECN::ECPropertyCR ecProperty, WCharCP propertyAccessString, IECInstanceCP hint) 
    : m_nullable(true), m_unique(false), m_columnType(PRIMITIVETYPE_Unknown), m_collate(Collate::Default), m_virtual(false)
    {
    // Assumes that if it is mapped to a column and is not primitive, it is mapped as a blob. Needswork if we add a JSON mapping hint
    PrimitiveECPropertyCP primitiveProperty = ecProperty.GetAsPrimitiveProperty();
    if (primitiveProperty)
        m_columnType = primitiveProperty->GetType ();
    else
        m_columnType = PRIMITIVETYPE_Binary; //In this case, it is the type of the column, not of the property

    InitializeFromHint(hint, ecProperty);

    // PropertyMappingRule: if ECDbPropertyHint does not supply a column name for an ECProperty, we use the ECProperty's propertyAccessString
    if (Utf8String::IsNullOrEmpty (m_columnName.c_str ()))
        BeStringUtilities::WCharToUtf8(m_columnName, propertyAccessString);
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
ColumnInfo::ColumnInfo (Utf8CP columnName, PrimitiveType primitiveType, bool nullable, bool unique, Collate collate)
: m_columnName (columnName), m_columnType (primitiveType), m_nullable (nullable), m_unique (unique), m_collate (collate), m_virtual (false)
    {
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ColumnInfo::InitializeFromHint(IECInstanceCP hint, ECPropertyCR ecProperty)
    {
    if (hint == nullptr)
        return;

    Utf8String columnName;
    if (PropertyHintReader::TryReadColumnName (columnName, *hint))
        m_columnName = columnName;

    bool isNullable = false;
    if (PropertyHintReader::TryReadIsNullable (isNullable, *hint))
        m_nullable = isNullable;

    bool isUnique = false;
    if (PropertyHintReader::TryReadIsUnique (isUnique, *hint))
        m_unique = isUnique;

    Collate collate;
    if (PropertyHintReader::TryReadCollate (collate, *hint))
        m_collate = collate;

    //WIP_ECDB: needswork: handle DoNotMap
    }


/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Affan.Khan      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
BeSQLite::DbResult ColumnInfo::LoadMappingInformationFromDb (ColumnInfoR columnInfo, ECPropertyCR ecProperty, BeSQLite::Db& db)
    {
    Utf8String mapColumnName;
    DbResult r;
    ECPropertyId propertyId;
    if (ecProperty.HasId())
        propertyId = ecProperty.GetId();
    else
        propertyId = ECDbSchemaManager::GetPropertyIdForECPropertyFromDuplicateECSchema (db, ecProperty);

    if (BE_SQLITE_ROW == (r = ECDbSchemaPersistence::GetECPropertyMapColumnName(mapColumnName, propertyId, db)))
        {
        if (Utf8String::IsNullOrEmpty (mapColumnName.c_str ()))
            {
            LOG.warningv (L"Property map MappedColumnName has empty string for %ls.%ls", ecProperty.GetClass().GetFullName(), ecProperty.GetName().c_str());
            BeAssert(Utf8String::IsNullOrEmpty (mapColumnName.c_str ()));
            }
        columnInfo.m_columnName = mapColumnName;
        }
    return r;
    }


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
Utf8CP ColumnInfo::GetName() const
    {
    return m_columnName.c_str();
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ColumnInfo::GetNullable() const
    {
    return m_nullable;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool ColumnInfo::GetUnique() const
    {
    return m_unique;
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
PrimitiveType ColumnInfo::GetColumnType() const
    {
    return m_columnType;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                    casey.mullen      12/2012
//---------------------------------------------------------------------------------------
void ColumnInfo::SetColumnName (Utf8CP columnName)
    {
    m_columnName = columnName;
    }

/*---------------------------------------------------------------------------------**//**
* @bsimethod                                                    Casey.Mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DbColumn::DbColumn(Utf8CP columnName, PrimitiveType primitiveType, bool nullable, bool unique, Collate collate) 
    : ColumnInfo(columnName, primitiveType, nullable, unique, collate) 
    {
    }

DbColumnPtr DbColumn::Create(Utf8CP columnName, PrimitiveType primitiveType, bool nullable, bool unique, Collate collate) 
    {
    return new DbColumn(columnName, primitiveType, nullable, unique, collate);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DbColumnPtr DbColumn::CreatePrimaryKey()
    {
    return new DbColumn(ECDB_COL_ECInstanceId, PRIMITIVETYPE_DbKey, false, true, Collate::Default); // specify a value generator on the column?
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DbColumnPtr DbColumn::CreatePrimaryKey(Utf8CP column)
    {
    return new DbColumn(column, PRIMITIVETYPE_DbKey, false, true, Collate::Default); // specify a value generator on the column?
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
DbColumnPtr DbColumn::CreateClassId()
    {
    return new DbColumn(ECDB_COL_ECClassId, PRIMITIVETYPE_DbKey, false, false, Collate::Default);
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2011
+---------------+---------------+---------------+---------------+---------------+------*/
bool DbColumn::IsCompatibleWith (PrimitiveType columnType) const
    {
    return (GetColumnType () == columnType);
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
