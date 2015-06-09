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
ColumnInfo::ColumnInfo (ECN::ECPropertyCR ecProperty, WCharCP propertyAccessString, IECInstanceCP hint) 
: m_nullable (true), m_unique (false), m_columnType (PRIMITIVETYPE_Unknown), m_collation (ECDbSqlColumn::Constraint::Collation::Default), m_virtual (false), m_priority (0)
    {
    // Assumes that if it is mapped to a column and is not primitive, it is mapped as a blob. Needswork if we add a JSON mapping hint
    PrimitiveECPropertyCP primitiveProperty = ecProperty.GetAsPrimitiveProperty();
    if (primitiveProperty)
        m_columnType = primitiveProperty->GetType ();
    else
        m_columnType = PRIMITIVETYPE_Binary; //In this case, it is the type of the column, not of the property

    InitializeFromHint(hint, ecProperty);

    // PropertyMappingRule: if custom attribute PropertyMap does not supply a column name for an ECProperty, 
    // we use the ECProperty's propertyAccessString (and replace . by _)
    if (Utf8String::IsNullOrEmpty (m_columnName.c_str ()))
        {
        m_columnName = Utf8String (propertyAccessString);
        m_columnName.ReplaceAll (".", "_");
        BeAssert (m_columnName.find (".") == Utf8String::npos);
        }
    }

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    casey.mullen      11/2012
+---------------+---------------+---------------+---------------+---------------+------*/
void ColumnInfo::InitializeFromHint(IECInstanceCP hint, ECPropertyCR ecProperty)
    {
    if (hint == nullptr)
        return;

    Utf8String columnName;
    if (CustomPropertyMapReader::TryReadColumnName (columnName, *hint))
        m_columnName = columnName;

    bool isNullable = false;
    if (CustomPropertyMapReader::TryReadIsNullable (isNullable, *hint))
        m_nullable = isNullable;

    bool isUnique = false;
    if (CustomPropertyMapReader::TryReadIsUnique (isUnique, *hint))
        m_unique = isUnique;

    ECDbSqlColumn::Constraint::Collation collation;
    if (CustomPropertyMapReader::TryReadCollation (collation, *hint))
        m_collation = collation;

    //WIP_ECDB: needswork: handle DoNotMap
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


//
END_BENTLEY_SQLITE_EC_NAMESPACE
