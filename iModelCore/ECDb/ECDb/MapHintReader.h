/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/MapHintReader.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ClassMap.h"
#include "ClassMapInfo.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//======================================================================================
// @bsiclass                                                 Krischan.Eberle  02/2014
//+===============+===============+===============+===============+===============+======
struct CustomSchemaMapReader
    {
private:
    CustomSchemaMapReader ();
    ~CustomSchemaMapReader ();

public:
    static ECN::IECInstancePtr Read (ECN::ECSchemaCR);
    static bool TryReadTablePrefix (Utf8String& tablePrefix, ECN::IECInstanceCR customSchemaMap);
    };


//======================================================================================
// @bsiclass                                                 Krischan.Eberle  02/2014
//+===============+===============+===============+===============+===============+======
struct CustomClassMapReader
    {
private:
    CustomClassMapReader ();
    ~CustomClassMapReader ();

public:
    static ECN::IECInstancePtr Read (ECN::ECClassCR);
    static bool TryReadMapStrategy (ECDbMapStrategy& mapStrategy, ECN::IECInstanceCR customClassMap);
    static bool TryReadTableName(Utf8String& tableName, ECN::IECInstanceCR customClassMap);
    static bool TryReadECInstanceIdColumnName(Utf8String& ecInstanceIdColumnName, ECN::IECInstanceCR customClassMap);
    static bool TryReadIndices(bvector<ClassIndexInfoPtr>& indices, ECN::IECInstanceCR customClassMap, ECN::ECClassCR ecClass);
    };

//======================================================================================
// @bsiclass                                                 Krischan.Eberle  02/2014
//+===============+===============+===============+===============+===============+======
struct CustomPropertyMapReader
    {
private:
    CustomPropertyMapReader();
    ~CustomPropertyMapReader();

public:
    static ECN::IECInstancePtr Read(ECN::ECPropertyCR ecProperty);
    static bool TryReadColumnName(Utf8String& columnName, ECN::IECInstanceCR customPropMap);
    static bool TryReadIsNullable(bool& isNullable, ECN::IECInstanceCR customPropMap);
    static bool TryReadIsUnique(bool& isUnique, ECN::IECInstanceCR customPropMap);
    static bool TryReadCollation(ECDbSqlColumn::Constraint::Collation&, ECN::IECInstanceCR customPropMap);
    };

//======================================================================================
// @bsiclass                                                 Krischan.Eberle  02/2014
//+===============+===============+===============+===============+===============+======
struct RelationshipClassHintReader
    {
private:
    RelationshipClassHintReader ();
    ~RelationshipClassHintReader ();

public:
    static ECN::IECInstancePtr ReadHint (ECN::ECClassCR relClass);
    static bool TryReadPreferredDirection (RelationshipClassMapInfo::PreferredDirection& preferredDirection, ECN::IECInstanceCR relClassHint);
    static bool TryReadAllowDuplicateRelationships (RelationshipClassMapInfo::TriState& allowDuplicateRelationships, ECN::IECInstanceCR relClassHint);
    static bool TryReadSourceECInstanceIdColumnName (Utf8String& sourceECInstanceIdColumnName, ECN::IECInstanceCR relClassHint);
    static bool TryReadSourceECClassIdColumnName (Utf8String& sourceECClassIdColumnName, ECN::IECInstanceCR relClassHint);
    static bool TryReadTargetECInstanceIdColumnName (Utf8String& targetECInstanceIdColumnName, ECN::IECInstanceCR relClassHint);
    static bool TryReadTargetECClassIdColumnName (Utf8String& targetECClassIdColumnName, ECN::IECInstanceCR relClassHint);
    };


//======================================================================================
// @bsiclass                                                 Krischan.Eberle  02/2014
//+===============+===============+===============+===============+===============+======
struct CustomAttributeReader
    {
private:
    CustomAttributeReader ();
    ~CustomAttributeReader ();
        
public:
    static ECN::IECInstancePtr Read (ECN::IECCustomAttributeContainer const&, WCharCP customAttributeName);
    static bool TryGetTrimmedValue (Utf8StringR val, ECN::IECInstanceCR ca, WCharCP ecPropertyName);
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
