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
struct SchemaHintReader
    {
private:
    SchemaHintReader ();
    ~SchemaHintReader ();

public:
    static ECN::IECInstancePtr ReadHint (ECN::ECSchemaCR ecschema);
    static bool TryReadTablePrefix (Utf8String& tablePrefix, ECN::IECInstanceCR hint);
    };


//======================================================================================
// @bsiclass                                                 Krischan.Eberle  02/2014
//+===============+===============+===============+===============+===============+======
struct ClassHintReader
    {
private:
    ClassHintReader ();
    ~ClassHintReader ();

    static bool TryConvertToMapStrategy (MapStrategy& mapStrategy, WCharCP mapStrategyName);
public:
    static ECN::IECInstancePtr ReadHint (ECN::ECClassCR ecClass);
    static bool TryReadMapStrategy (MapStrategy& mapStrategy, ECN::IECInstanceCR classHint);
    static bool TryReadTableName (Utf8String& tableName, ECN::IECInstanceCR classHint);
    static bool TryReadECInstanceIdColumnName (Utf8String& ecInstanceIdColumnName, ECN::IECInstanceCR classHint);
    static bool TryReadMapToExistingTable (bool& mapToExistingTable, ECN::IECInstanceCR classHint);
    static bool TryReadReplaceEmptyTableWithEmptyView (bool& replaceEmptyTableWithEmptyView, ECN::IECInstanceCR classHint);
    static bool TryReadUseSharedColumnStrategy (bool& useSharedColumnStrategy, ECN::IECInstanceCR classHint);
    static ECN::IECInstancePtr ReadClassHasTimeStamp(ECN::ECClassCR ecClass);
    static bool TryReadIndices (bvector<ClassIndexInfoPtr>& indices, ECN::IECInstanceCR classHint, ECN::ECClassCR ecClass);
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
struct PropertyHintReader
    {
private:
    PropertyHintReader ();
    ~PropertyHintReader ();

public:
    static ECN::IECInstancePtr ReadHint (ECN::ECPropertyCR ecProperty);
    static bool TryReadColumnName (Utf8String& columnName, ECN::IECInstanceCR hint);
    static bool TryReadIsNullable (bool& isNullable, ECN::IECInstanceCR hint);
    static bool TryReadIsUnique (bool& isUnique, ECN::IECInstanceCR hint);
    static bool TryReadCollate (ECDbSqlColumn::Constraint::Collate& collate, ECN::IECInstanceCR hint);
    };

//======================================================================================
// @bsiclass                                                 Krischan.Eberle  02/2014
//+===============+===============+===============+===============+===============+======
struct HintReaderHelper
    {
private:
    HintReaderHelper ();
    ~HintReaderHelper ();
        
public:
    static ECN::IECInstancePtr ReadHint (ECN::IECCustomAttributeContainer const& caContainer, WCharCP hintCustomAttributeName);
    static bool TryGetTrimmedValue (Utf8StringR val, ECN::IECInstanceCR ca, WCharCP ecPropertyName);
    };
END_BENTLEY_SQLITE_EC_NAMESPACE
