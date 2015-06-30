/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapInfo.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "SchemaImportContext.h"
#include "MapStrategy.h"
#include "ECDbSql.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct IClassMap;
struct ClassMapInfo;

//======================================================================================
// @bsiclass                                                 Krischan.Eberle  02/2014
//+===============+===============+===============+===============+===============+======
struct ClassMapInfoFactory
    {
private:
    ClassMapInfoFactory ();
    ~ClassMapInfoFactory ();

    static std::shared_ptr<ClassMapInfo> Create(MapStatus&, ECN::ECClassCR, ECDbMapCR);

public:
    static std::shared_ptr<ClassMapInfo> Create(MapStatus&, SchemaImportContext const&, ECN::ECClassCR, ECDbMapCR);
    };

//======================================================================================
//!This class grabs information from ClassMap ECCustomAttribute and evaluates
//! it along with other standard metadata on the ECClass... applying default 
//! rules like checking if isDomainClass=False, building standard table name, etc.
// @bsiclass                                                     Casey.Mullen      11/2011
//+===============+===============+===============+===============+===============+======
struct ClassMapInfo : NonCopyableClass
{
private:
    Utf8String m_tableName;
    Utf8String m_ecInstanceIdColumnName;
    bvector<StandardKeySpecificationPtr> m_standardKeys;
    bvector<ClassIndexInfoPtr> m_dbIndexes; 
    IClassMap const* m_parentClassMap;
    bool m_isMapToVirtualTable;
    ECN::ECPropertyCP m_classHasCurrentTimeStampProperty;
    ECDbMapStrategy m_strategy;

protected:
    ECDbMapCR m_ecdbMap;
    ECN::ECClassCR m_ecClass;

private:
    BentleyStatus InitializeFromClassMapCA ();
    BentleyStatus InitializeFromClassHasCurrentTimeStampProperty();

    bool ValidateBaseClasses () const;
    MapStatus EvaluateInheritedMapStrategy ();

    bool GatherBaseClassMaps (bvector<IClassMap const*>& baseClassMaps, bvector<IClassMap const*>& tphMaps, bvector<IClassMap const*>& tpcMaps, bvector<IClassMap const*>& nmhMaps, ECN::ECClassCR ecClass) const;

    BentleyStatus ProcessStandardKeys(ECN::ECClassCR ecClass, WCharCP customAttributeName);
    static Utf8String ResolveTablePrefix (ECN::ECClassCR ecClass);

    MapStatus ReportError_OneClassMappedByTableInHierarchyFromTwoDifferentAncestors (ECN::ECClassCR ecClass, bvector<IClassMap const*> tphMaps) const;

protected:
    virtual BentleyStatus _InitializeFromSchema();
    virtual MapStatus _EvaluateMapStrategy();

    static void LogClassNotMapped (NativeLogging::SEVERITY severity, ECN::ECClassCR ecClass, Utf8CP explanation);

public:
    ClassMapInfo(ECN::ECClassCR, ECDbMapCR);
    virtual ~ClassMapInfo() {}

    ECN::ECPropertyCP GetClassHasCurrentTimeStampProperty() const { return m_classHasCurrentTimeStampProperty; }
    virtual MapStatus _Initialize();

    ECDbMapStrategy const& GetMapStrategy () const{ return m_strategy; }
    ECDbMapStrategy& GetMapStrategyR (){ return m_strategy; }

    ECDbMapCR GetECDbMap() const {return m_ecdbMap;}
    ECN::ECClassCR GetECClass() const {return m_ecClass;}
    bvector<ClassIndexInfoPtr> const& GetIndexInfo() const { return m_dbIndexes;}
    Utf8CP GetTableName() const {return m_tableName.c_str();}
    Utf8CP GetECInstanceIdColumnName() const {return m_ecInstanceIdColumnName.c_str();}
    IClassMap const* GetParentClassMap () const { return m_parentClassMap; }
    void SetParentClassMap (IClassMap const* parentClassMap) { m_parentClassMap = parentClassMap; }
    bvector<StandardKeySpecificationPtr>const& GetStandardKeys() const {return m_standardKeys;}

    //! Virtual tables are not persisted   
    bool IsMapToVirtualTable () const { return m_isMapToVirtualTable; }
    void RestoreSaveSettings (ECDbMapStrategy mapStrategy, Utf8CP tableName){ m_strategy = mapStrategy; if (tableName != nullptr) m_tableName = tableName; }
    };


//======================================================================================
// @bsiclass                                                     Krischan.Eberle     06/2015
//+===============+===============+===============+===============+===============+======
struct RelationshipEndColumns
    {
private:
    Utf8String m_ecInstanceIdColumnName;
    Utf8String m_ecClassIdColumnName;

public:
    RelationshipEndColumns() {}
    RelationshipEndColumns(Utf8CP ecInstanceIdColumnName, Utf8CP ecClassIdColumnName) : m_ecInstanceIdColumnName(ecInstanceIdColumnName), m_ecClassIdColumnName(ecClassIdColumnName) {}
    Utf8CP GetECInstanceIdColumnName() const { return m_ecInstanceIdColumnName.c_str(); }
    Utf8CP GetECClassIdColumnName() const { return m_ecClassIdColumnName.c_str(); }
    };

//======================================================================================
//!This class grabs information from ForeignKeyRelationshipMap or LinkTableRelationshipMap ECCustomAttribute and evaluates
//! it along with other standard metadata on the ECRelationshipClass
// @bsiclass                                                     Krischan.Eberle     06/2015
//+===============+===============+===============+===============+===============+======
struct RelationshipMapInfo : public ClassMapInfo
    {
public:
    enum class Cardinality
        {
        ManyToMany,
        OneToMany,
        ManyToOne,
        OneToOne
        };

    enum class CustomMapType
        {
        None,
        ForeignKeyOnTarget,
        ForeignKeyOnSource,
        LinkTable
        };

private:
    Cardinality m_cardinality;
    bool m_sourceColumnsMappingIsNull;
    RelationshipEndColumns m_sourceColumnsMapping;
    bool m_targetColumnsMappingIsNull;
    RelationshipEndColumns m_targetColumnsMapping;
    bool m_allowDuplicateRelationships;
    ECDbSqlForeignKeyConstraint::ActionType m_onDeleteAction;
    ECDbSqlForeignKeyConstraint::ActionType m_onUpdateAction;
    CustomMapType m_customMapType;

    virtual BentleyStatus _InitializeFromSchema() override;
    virtual MapStatus _EvaluateMapStrategy();
    MapStatus DetermineImpliedMapStrategy(bool& mapStrategyAlreadyEvaluated);
    MapStatus EvaluateCustomMapping(bool mapStrategyAlreadyEvaluated);

    void DetermineCardinality();
    bool VerifyRelatedClasses() const;

    bool TryDetermine11RelationshipMapStrategy(MapStrategy&, ECN::ECRelationshipConstraintR source, ECN::ECRelationshipConstraintR target, ECN::ECRelationshipClassCR relationshipClass) const;
    bool TryDetermine1MRelationshipMapStrategy(MapStrategy&, ECN::ECRelationshipConstraintR source, ECN::ECRelationshipConstraintR target, ECN::ECRelationshipClassCR relationshipClass) const;
    static bool ContainsRelationshipClass(std::vector<ECN::ECClassCP> const& endClasses);

public:
    RelationshipMapInfo(ECN::ECRelationshipClassCR relationshipClass, ECDbMapCR ecdbMap)
        : ClassMapInfo(relationshipClass, ecdbMap), m_sourceColumnsMappingIsNull(true), m_targetColumnsMappingIsNull(true), 
        m_allowDuplicateRelationships(false), m_onDeleteAction(ECDbSqlForeignKeyConstraint::ActionType::NotSpecified), 
        m_onUpdateAction(ECDbSqlForeignKeyConstraint::ActionType::NotSpecified), m_customMapType(CustomMapType::None)
        {}

    virtual ~RelationshipMapInfo() {}
    virtual MapStatus _Initialize() override;

    Cardinality GetCardinality() const { return m_cardinality; }
    RelationshipEndColumns const& GetSourceColumnsMapping() const { BeAssert(m_customMapType != CustomMapType::ForeignKeyOnTarget && GetMapStrategy().GetStrategy() != MapStrategy::RelationshipTargetTable); return m_sourceColumnsMapping; }
    RelationshipEndColumns const& GetTargetColumnsMapping() const { BeAssert(m_customMapType != CustomMapType::ForeignKeyOnSource && GetMapStrategy().GetStrategy() != MapStrategy::RelationshipSourceTable); return m_targetColumnsMapping; }
    bool AllowDuplicateRelationships() const { BeAssert((m_customMapType == CustomMapType::LinkTable || m_customMapType == CustomMapType::None) && GetMapStrategy().IsLinkTableStrategy()); return m_allowDuplicateRelationships; }
    bool IsCreateForeignKeyConstraint() const { BeAssert(m_customMapType != CustomMapType::LinkTable && GetMapStrategy().IsEndTableMapping()); return m_onDeleteAction != ECDbSqlForeignKeyConstraint::ActionType::NotSpecified || m_onUpdateAction != ECDbSqlForeignKeyConstraint::ActionType::NotSpecified; }
    ECDbSqlForeignKeyConstraint::ActionType GetOnDeleteAction() const { BeAssert(IsCreateForeignKeyConstraint()); return m_onDeleteAction; }
    ECDbSqlForeignKeyConstraint::ActionType GetOnUpdateAction() const { BeAssert(IsCreateForeignKeyConstraint()); return m_onUpdateAction; }
    };


//======================================================================================
// @bsiclass                                                Affan.Khan  02/2012
//+===============+===============+===============+===============+===============+======
struct ClassIndexInfo : RefCountedBase
    {
    public:
        enum class WhereConstraint
            {
            None,
            NotNull
            };
private:
    Utf8String m_name;
    bool m_isUnique;
    bvector<Utf8String> m_properties;
    WhereConstraint m_where;
private:
    ClassIndexInfo(Utf8CP name, bool isUnique, bvector<Utf8String> const& properties, WhereConstraint whereConstraint)
        : m_name(name), m_isUnique(isUnique), m_properties(properties), m_where(whereConstraint)
        {}

public:
    Utf8CP GetName() const { return m_name.c_str();}
    bool GetIsUnique() const { return m_isUnique;}
    bvector<Utf8String>& GetProperties(){ return m_properties;}
    WhereConstraint GetWhere() const { return m_where; }
    static ClassIndexInfoPtr Create(ECN::ECDbClassMap::DbIndex const& dbIndex)
        {
        WhereConstraint whereConstraint = WhereConstraint::None;

        Utf8CP whereClause = dbIndex.GetWhereClause();
        if (!Utf8String::IsNullOrEmpty(whereClause))
            {
            if (BeStringUtilities::Stricmp(whereClause, "ECDB_NOTNULL") == 0)
                whereConstraint = WhereConstraint::NotNull;
            else
                {
                LOG.errorv("Invalid where clause in ClassMap::DbIndex: %s. Only ECDB_NOTNULL supported by ECDb.", dbIndex.GetWhereClause());
                return nullptr;
                }
            }

        return new ClassIndexInfo(dbIndex.GetName(), dbIndex.IsUnique(), dbIndex.GetProperties(), whereConstraint);
        }
    };

/*=================================================================================**//**
* This class hold key specification as describe by standard custom attributes
* @bsiclass                                                     Affan.Khan      09/2012
+===============+===============+===============+===============+===============+======*/
struct StandardKeySpecification : RefCountedBase
    {
    public:
        enum class Type
            {
            None = 0,
            SyncIDSpecification,
            GlobalIdSpecification,
            BusinessKeySpecification
            };
    private:
        bvector<Utf8String> m_keyProperties;
        Type m_type;

        StandardKeySpecification(Type type) : m_type(type){}

    public:
        bvector<Utf8String>& GetKeyProperties() { return m_keyProperties; }
        Type GetType() const { return m_type; }
        static StandardKeySpecificationPtr Create(Type type)
            {
            return new StandardKeySpecification(type);
            }
        static Type GetTypeFromString(WCharCP customAttributeName)
            {
            Type keyType = Type::None;
            if (BeStringUtilities::Wcsicmp(customAttributeName, L"SyncIDSpecification") == 0)
                keyType = Type::SyncIDSpecification;
            else if (BeStringUtilities::Wcsicmp(customAttributeName, L"GlobalIdSpecification") == 0)
                keyType = Type::GlobalIdSpecification;
            else if (BeStringUtilities::Wcsicmp(customAttributeName, L"BusinessKeySpecification") == 0)
                keyType = Type::BusinessKeySpecification;

            return keyType;
            }
        static WString TypeToString(Type keyType)
            {
            if (keyType == Type::SyncIDSpecification)
                return L"SyncIDSpecification";
            if (keyType == Type::GlobalIdSpecification)
                return L"GlobalIdSpecification";
            if (keyType == Type::BusinessKeySpecification)
                return L"BusinessKeySpecification";

            return L"";
            }
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
