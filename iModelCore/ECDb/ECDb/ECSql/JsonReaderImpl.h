
/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/JsonReaderImpl.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include <ECDb/ECSqlStatement.h>
#include <ECDb/JsonAdapter.h>
#include <ECDb/ECRelationshipPath.h> 
#include <ECDb/ECInstanceFinder.h> 

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

    
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

//=======================================================================================
//! Impl of JsonReader
//@bsiclass                                         schan.Eberle                  06/2014
//+===============+===============+===============+===============+===============+======
struct JsonReader::Impl
    {
private:
    ECDbCR m_ecDb;
    ECN::ECClassCP m_ecClass;
    ECSqlStatementCache m_statementCache;

    bool m_isValid;

    BentleyStatus PrepareECSql (CachedECSqlStatementPtr& statement, Utf8StringCR ecSql);
    BentleyStatus PrepareECSql 
        (
        CachedECSqlStatementPtr& statement,
        const ECRelationshipPath& pathFromRelatedClass, 
        const ECInstanceId& ecInstanceId, 
        bool selectInstanceKeyOnly,
        bool isPolymorphic
        );

    BentleyStatus GetRelatedInstanceKeys (ECInstanceKeyMultiMap& instanceKeys, const ECRelationshipPath& pathToClass, const ECInstanceId& ecInstanceId);
    void SetRelationshipPath (JsonValueR addClasses, Utf8StringCR pathToClassStr);

    void AddClasses (JsonValueR allClasses, JsonValueR addClasses);
    void AddCategories (JsonValueR allCategories, JsonValueR addCategories, int currentInstanceIndex);
    void AddInstances (JsonValueR allInstances, JsonValueR addInstances, int currentInstanceIndex);
    BentleyStatus AddInstancesFromPreparedStatement (JsonValueR jsonInstances, JsonValueR jsonDisplayInfo, ECSqlStatement& statement,
        const JsonECSqlSelectAdapter::FormatOptions& formatOptions, Utf8StringCR pathToClassStr);
    BentleyStatus AddInstancesFromSpecifiedClassPath (JsonValueR jsonInstances, JsonValueR jsonDisplayInfo, const ECRelationshipPath& pathToClass,
        const ECInstanceId& ecInstanceId, const JsonECSqlSelectAdapter::FormatOptions& formatOptions);
    BentleyStatus AddInstancesFromRelatedItems (JsonValueR allInstances, JsonValueR allDisplayInfo, ECN::ECClassCR parentClass,
        ECRelationshipPathCR pathFromParent, const ECInstanceId& ecInstanceId, const JsonECSqlSelectAdapter::FormatOptions& formatOptions);
    void SetInstanceIndex (JsonValueR addCategories, int currentInstanceIndex);
    BentleyStatus GetTrivialPathToSelf (ECRelationshipPathR emptyPath, ECN::ECClassCR ecClass);

    bool IsValid () const { return m_isValid; }
public:
    Impl (ECDbCR ecdb, ECN::ECClassId ecClassId);

    BentleyStatus Read (JsonValueR jsonInstances, JsonValueR jsonDisplayInfo, ECInstanceId const& ecInstanceId, JsonECSqlSelectAdapter::FormatOptions formatOptions);
    BentleyStatus ReadInstance (JsonValueR jsonInstance, ECInstanceId const& ecInstanceId, JsonECSqlSelectAdapter::FormatOptions formatOptions);
    };
    
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

//=================================================================================
// Helper utility to work with classes
// @bsiclass                                                 Ramanujam.Raman      01/2014
//+===============+===============+===============+===============+===============+======
struct ECClassHelper
    {
private:
    ECClassHelper ();
    ~ECClassHelper ();

    static void ParseQualifiedName (Utf8StringR schemaPrefixOrName, Utf8StringR className, Utf8StringCR qualifiedClassName, Utf8Char delimiter);

public:
    //! @param schemaName [in] Can be nullptr, in which case the class is resolved in the default schema (assuming that's supplied). 
    static ECN::ECClassCP ResolveClass (Utf8CP schemaName, Utf8StringCR className, ECDbCR ecDb, ECN::ECSchemaCP defaultSchema);
    static ECN::ECClassCP ResolveClass (Utf8StringCR possiblyQualifiedClassName, ECDbCR ecDb, ECN::ECSchemaCP defaultSchema);
    static bool IsAnyClass (ECN::ECClassCR ecClass);
    static Utf8String GetName (ECN::ECClassCR ecClass);
    static Utf8String GetQualifiedECObjectsName (ECN::ECClassCR ecClass);
    static Utf8String GetQualifiedECSqlName (ECN::ECClassCR ecClass);
    static void ParseQualifiedECObjectsName (Utf8StringR schemaPrefixOrName, Utf8StringR className, Utf8StringCR qualifiedClassName);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
