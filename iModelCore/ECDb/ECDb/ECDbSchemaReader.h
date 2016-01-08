/*--------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECDbSchemaReader.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+-------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
USING_NAMESPACE_BENTLEY_EC
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbECSchemaEntry
    {
public:
    ECN::ECSchemaPtr m_cachedECSchema;    //Contain ECSchema which might be not complete
    int m_typeCountInSchema; //This is read from Db
    uint32_t m_loadedTypeCount; //Every time a class or enum is loaded from db for this schema it is incremented

    explicit DbECSchemaEntry(ECN::ECSchemaR schema) : m_cachedECSchema(&schema), m_typeCountInSchema(-1), m_loadedTypeCount(0)
        {}

    DbECSchemaEntry(ECN::ECSchemaPtr& schema, int typeCountInSchema) : m_cachedECSchema(schema), m_typeCountInSchema(typeCountInSchema), m_loadedTypeCount(0)
        {}

    ECN::ECSchemaId GetId() const { return m_cachedECSchema->GetId(); }
    bool IsFullyLoaded() const { return m_typeCountInSchema == m_loadedTypeCount; }
    };

/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        06/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct DbECClassEntry
    {
    public:
        ECClassId m_ecClassId;
        ECN::ECClassP m_cachedECClass;

        explicit DbECClassEntry(ECN::ECClassCR ecClass) : m_ecClassId(ecClass.GetId())
            {
            m_cachedECClass = const_cast<ECN::ECClassP> (&ecClass);
            }
    };

//=======================================================================================
// @bsiclass                                                Krischan.Eberle      12/2015
//+===============+===============+===============+===============+===============+======
struct DbECEnumEntry
    {
    public:
        uint64_t m_enumId;
        ECN::ECEnumerationP m_cachedECEnum;

        DbECEnumEntry(uint64_t enumId, ECN::ECEnumerationCR ecEnum) : m_enumId(enumId)
            {
            m_cachedECEnum = const_cast<ECN::ECEnumerationP> (&ecEnum);
            }
    };


/*---------------------------------------------------------------------------------------
* @bsimethod                                                    Affan.Khan        05/2012
+---------------+---------------+---------------+---------------+---------------+------*/
struct ECDbSchemaReader: public RefCountedBase
    {
    typedef std::map<ECSchemaId, std::unique_ptr<DbECSchemaEntry>> DbECSchemaMap;
    typedef std::map<ECClassId, std::unique_ptr<DbECClassEntry>> DbECClassEntryMap;
    typedef std::map<uint64_t, std::unique_ptr<DbECEnumEntry>> DbECEnumEntryMap;

private:
    struct Context : NonCopyableClass
        {
        private:
            std::vector<ECN::NavigationECProperty*> m_navProps;

        public:
            Context() {}

            void AddNavigationProperty(ECN::NavigationECProperty& navProp) { m_navProps.push_back(&navProp); }

            BentleyStatus Postprocess() const
                {
                /*for (ECN::NavigationECProperty* navProp : m_navProps)
                {
                if (ECObjectsStatus::Success != navProp->Validate())
                return ERROR;
                }*/
                return SUCCESS;
                }
        };

    ECDbCR m_db;
    mutable ECSchemaCache m_cache;
    mutable DbECSchemaMap m_ecSchemaCache;
    mutable DbECClassEntryMap m_ecClassCache;
    mutable DbECEnumEntryMap m_ecEnumCache;
    mutable BeMutex m_criticalSection;

    explicit ECDbSchemaReader(ECDbCR db) :m_db(db) {}

    ECSchemaCP            GetECSchema(Context&, ECSchemaId ecSchemaId, bool loadClasses) const;
    ECClassP              GetECClass(Context&, ECClassId ecClassId) const;
    ECEnumerationCP       GetECEnumeration(Context&, Utf8CP schemaName, Utf8CP enumName) const;

    BentleyStatus         LoadClassesAndEnumsFromDb(DbECSchemaEntry* ecSchemaKey, Context&, std::set<DbECSchemaEntry*>& fullyLoadedSchemas) const;
    BentleyStatus         LoadECSchemaFromDb(DbECSchemaEntry*&, ECSchemaId) const;
    BentleyStatus         LoadECPropertiesFromDb(ECClassP& ecClass, Context&, ECClassId ecClassId) const;
    BentleyStatus         LoadBaseClassesFromDb(ECClassP& ecClass, Context&, ECClassId ecClassId) const;
    BentleyStatus         LoadCAFromDb(ECN::IECCustomAttributeContainerR  caConstainer, Context&, ECContainerId containerId, ECContainerType containerType) const;
    BentleyStatus         LoadECRelationshipConstraintFromDb(ECRelationshipClassP&, Context&, ECClassId constraintClassId, ECRelationshipEnd) const;
    BentleyStatus         LoadECRelationshipConstraintClassesFromDb(ECRelationshipConstraintR, Context&, ECClassId relationshipClassId, ECRelationshipEnd) const;
    BentleyStatus         LoadECSchemaDefinition(DbECSchemaEntry*&, bvector<DbECSchemaEntry*>& newlyLoadedSchemas, ECSchemaId ctxECSchemaId) const;

    BentleyStatus         ReadECSchema(DbECSchemaEntry*&, Context&, ECSchemaId ctxECSchemaId, bool loadClasses) const;
    BentleyStatus         ReadECEnumeration(ECEnumerationP&, Context&, uint64_t ecenumId) const;

    BentleyStatus         EnsureDerivedClassesExist(Context&, ECClassId baseClassId) const;

public:
    ~ECDbSchemaReader() {}

    ECSchemaCP            GetECSchema(ECSchemaId ecSchemaId, bool loadClasses) const;
    ECClassCP              GetECClass (ECClassId ecClassId) const;
    ECEnumerationCP       GetECEnumeration(Utf8CP schemaName, Utf8CP enumName) const;

    BentleyStatus         EnsureDerivedClassesExist(ECClassId baseClassId) const;
    bool                  TryGetECClassId(ECN::ECClassId& id, Utf8CP schemaNameOrPrefix, Utf8CP className, ResolveSchema) const;
    void                  ClearCache ();

    static ECDbSchemaReaderPtr Create(ECDbCR);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
