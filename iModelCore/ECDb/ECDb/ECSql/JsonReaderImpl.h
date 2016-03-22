
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
#include <ECObjects/ECRelationshipPath.h> 
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
    struct ECRelatedItemsDisplaySpecCacheAppData : public Db::AppData
        {
    private:
        ECDbCR m_ecDb;
        ECN::ECRelatedItemsDisplaySpecificationsCache m_cache;

        ECRelatedItemsDisplaySpecCacheAppData(ECDbCR ecDb) : Db::AppData(), m_ecDb(ecDb) {}

        static BeSQLite::Db::AppData::Key const& GetKey() { static BeSQLite::Db::AppData::Key s_key; return s_key; }

    public:
        ~ECRelatedItemsDisplaySpecCacheAppData() {}

        ECN::ECRelatedItemsDisplaySpecificationsCache const& GetCache() const { return m_cache; }

        static ECRelatedItemsDisplaySpecCacheAppData* Get(ECDbCR);
        };

    ECDbCR m_ecDb;
    ECN::ECClassCP m_ecClass;
    ECSqlStatementCache m_statementCache;

    bool m_isValid;

    BentleyStatus PrepareECSql (CachedECSqlStatementPtr& statement, Utf8StringCR ecSql) const;
    BentleyStatus PrepareECSql (CachedECSqlStatementPtr& statement, ECN::ECRelationshipPath const& pathFromRelatedClass, ECInstanceId, bool selectInstanceKeyOnly, bool isPolymorphic) const;

    BentleyStatus GetRelatedInstanceKeys (ECInstanceKeyMultiMap& instanceKeys, ECN::ECRelationshipPath const& pathToClass, ECInstanceId);
    void SetRelationshipPath (JsonValueR addClasses, Utf8StringCR pathToClassStr);

    void AddClasses (JsonValueR allClasses, JsonValueR addClasses);
    void AddCategories (JsonValueR allCategories, JsonValueR addCategories, int currentInstanceIndex);
    void AddInstances (JsonValueR allInstances, JsonValueR addInstances, int currentInstanceIndex);
    BentleyStatus AddInstancesFromPreparedStatement (JsonValueR jsonInstances, JsonValueR jsonDisplayInfo, ECSqlStatement&, JsonECSqlSelectAdapter::FormatOptions const&, Utf8StringCR pathToClassStr);
    BentleyStatus AddInstancesFromSpecifiedClassPath (JsonValueR jsonInstances, JsonValueR jsonDisplayInfo, const ECRelationshipPath& pathToClass, ECInstanceId, JsonECSqlSelectAdapter::FormatOptions const&);
    BentleyStatus AddInstancesFromRelatedItems(JsonValueR allInstances, JsonValueR allDisplayInfo, ECClassCR parentClass, ECRelationshipPath const& pathFromParent, ECInstanceId, JsonECSqlSelectAdapter::FormatOptions const&);

    void SetInstanceIndex (JsonValueR addCategories, int currentInstanceIndex);
    BentleyStatus GetTrivialPathToSelf (ECN::ECRelationshipPath& emptyPath, ECN::ECClassCR);

    bool IsValid () const { return m_isValid; }
public:
    Impl (ECDbCR, ECN::ECClassId);

    BentleyStatus Read (JsonValueR jsonInstances, JsonValueR jsonDisplayInfo, ECInstanceId, JsonECSqlSelectAdapter::FormatOptions);
    BentleyStatus ReadInstance (JsonValueR jsonInstance, ECInstanceId, JsonECSqlSelectAdapter::FormatOptions);
    };
    

END_BENTLEY_SQLITE_EC_NAMESPACE
