/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapInfo.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "MapStrategy.h"
#include "ECDbSql.h"
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct IClassMap;
struct ClassMapInfo;
struct SchemaImportContext;

//======================================================================================
// @bsiclass                                                 Krischan.Eberle  02/2014
//+===============+===============+===============+===============+===============+======
struct ClassMapInfoFactory
    {
private:
    ClassMapInfoFactory ();
    ~ClassMapInfoFactory ();

public:
    static std::unique_ptr<ClassMapInfo> Create(MappingStatus&, SchemaImportContext const&, ECN::ECClassCR, ECDbMapCR);
    };

//======================================================================================
//!This class grabs information from ClassMap ECCustomAttribute and evaluates
//! it along with other standard metadata on the ECClass... building standard table name, etc.
// @bsiclass                                                     Casey.Mullen      11/2011
//+===============+===============+===============+===============+===============+======
struct ClassMapInfo : NonCopyableClass
{
private:
    Utf8String m_tableName;
    Utf8String m_ecInstanceIdColumnName;
    bvector<ClassIndexInfoPtr> m_dbIndexes; 
    IClassMap const* m_parentClassMap;
    bool m_isMapToVirtualTable;
    ECN::ECPropertyCP m_classHasCurrentTimeStampProperty;

protected:
    ECDbMapCR m_ecdbMap;
    ECN::ECClassCR m_ecClass;
    ECDbMapStrategy m_resolvedStrategy;

private:
    BentleyStatus DoEvaluateMapStrategy(bool& baseClassesNotMappedYet, UserECDbMapStrategy&);

    bool GatherBaseClassMaps (bvector<IClassMap const*>& baseClassMaps, bvector<IClassMap const*>& tphMaps, bvector<IClassMap const*>& tpcMaps, bvector<IClassMap const*>& nmhMaps, ECN::ECClassCR ecClass) const;
    bool ValidateChildStrategy(ECDbMapStrategy const& parentStrategy, UserECDbMapStrategy const& childStrategy) const;
    BentleyStatus InitializeClassHasCurrentTimeStampProperty();

protected:
    virtual BentleyStatus _InitializeFromSchema();
    virtual MappingStatus _EvaluateMapStrategy();

    static void LogClassNotMapped (NativeLogging::SEVERITY severity, ECN::ECClassCR ecClass, Utf8CP explanation);
public:
    ClassMapInfo(ECN::ECClassCR, ECDbMapCR);
    virtual ~ClassMapInfo() {}

    MappingStatus Initialize();

    ECDbMapStrategy const& GetMapStrategy () const{ return m_resolvedStrategy; }
    ECDbMapCR GetECDbMap() const {return m_ecdbMap;}
    ECN::ECClassCR GetECClass() const {return m_ecClass;}
    bvector<ClassIndexInfoPtr> const& GetIndexInfos() const { return m_dbIndexes;}
    Utf8CP GetTableName() const {return m_tableName.c_str();}
    Utf8CP GetECInstanceIdColumnName() const {return m_ecInstanceIdColumnName.c_str();}
    ECN::ECPropertyCP GetClassHasCurrentTimeStampProperty() const { return m_classHasCurrentTimeStampProperty; }
    IClassMap const* GetParentClassMap () const { return m_parentClassMap; }

    //! Virtual tables are not persisted   
    bool IsMapToVirtualTable () const { return m_isMapToVirtualTable; }
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
    RelationshipEndColumns(Utf8CP ecInstanceIdColumnName, Utf8CP ecClassIdColumnName = nullptr) : m_ecInstanceIdColumnName(ecInstanceIdColumnName), m_ecClassIdColumnName(ecClassIdColumnName) {}
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
    CustomMapType m_customMapType;
    bool m_allowDuplicateRelationships;
    ForeignKeyActionType m_onDeleteAction;
    ForeignKeyActionType m_onUpdateAction;
    bool m_createIndexOnForeignKey;
    std::set<ECDbSqlTable const*>  m_sourceTables;
    std::set<ECDbSqlTable const*>  m_targetTables;

    virtual BentleyStatus _InitializeFromSchema() override;
    virtual MappingStatus _EvaluateMapStrategy();
    void DetermineCardinality(ECN::ECRelationshipConstraintCR source, ECN::ECRelationshipConstraintCR target);
    BentleyStatus ResolveEndTables(EndTablesOptimizationOptions source, EndTablesOptimizationOptions target);
    MappingStatus Validate(ECDbMapStrategy::Strategy strategy, RelationshipMapInfo::Cardinality cardinality);

public:
    RelationshipMapInfo(ECN::ECRelationshipClassCR relationshipClass, ECDbMapCR ecdbMap) : ClassMapInfo(relationshipClass, ecdbMap), m_sourceColumnsMappingIsNull(true), m_targetColumnsMappingIsNull(true),
        m_customMapType(CustomMapType::None), m_allowDuplicateRelationships(false), 
        m_onDeleteAction(ForeignKeyActionType::NotSpecified), m_onUpdateAction(ForeignKeyActionType::NotSpecified), m_createIndexOnForeignKey(true)
        {}

    virtual ~RelationshipMapInfo() {}

    Cardinality GetCardinality() const { return m_cardinality; }

    CustomMapType GetCustomMapType() const { return m_customMapType; }
    bool AllowDuplicateRelationships() const { BeAssert((m_customMapType == CustomMapType::LinkTable || m_customMapType == CustomMapType::None) && !m_resolvedStrategy.IsForeignKeyMapping()); return m_allowDuplicateRelationships; }
    ForeignKeyActionType GetOnDeleteAction() const { BeAssert(m_customMapType != CustomMapType::LinkTable && m_resolvedStrategy.IsForeignKeyMapping());  return m_onDeleteAction; }
    ForeignKeyActionType GetOnUpdateAction() const { BeAssert(m_customMapType != CustomMapType::LinkTable && m_resolvedStrategy.IsForeignKeyMapping()); return m_onUpdateAction; }
    bool CreateIndexOnForeignKey() const { BeAssert(m_customMapType != CustomMapType::LinkTable && m_resolvedStrategy.IsForeignKeyMapping()); return m_createIndexOnForeignKey; }

    RelationshipEndColumns const& GetColumnsMapping(ECN::ECRelationshipEnd end) const;
    std::set<ECDbSqlTable const*> const& GetSourceTables() const {return m_sourceTables;}
    std::set<ECDbSqlTable const*> const& GetTargetTables() const {return m_targetTables;}
    };


//======================================================================================
// @bsiclass                                                Affan.Khan  02/2012
//+===============+===============+===============+===============+===============+======
struct ClassIndexInfo : RefCountedBase
    {
private:
    Utf8String m_name;
    bool m_isUnique;
    bvector<Utf8String> m_properties;
    bool m_addPropsAreNotNullWhereExp;
    static std::vector<std::pair<Utf8String, Utf8String>> s_idSpecCustomAttributeNames;

    ClassIndexInfo(Utf8CP name, bool isUnique, bvector<Utf8String> const& properties, bool addPropsAreNotNullWhereExp)
        : m_name(name), m_isUnique(isUnique), m_properties(properties), m_addPropsAreNotNullWhereExp(addPropsAreNotNullWhereExp)
        {}

    static BentleyStatus CreateFromIdSpecificationCAs(bvector<ClassIndexInfoPtr>& indexInfos, ECDbCR, ECN::ECClassCR);

    static ClassIndexInfoPtr Create(ECDbCR, ECN::ECDbClassMap::DbIndex const&);

    static std::vector<std::pair<Utf8String, Utf8String>> const& GetIdSpecCustomAttributeNames();

public:
    static ClassIndexInfoPtr Clone(ClassIndexInfoCR, Utf8CP newIndexName);
    //!@param customClassMap pass nullptr if @p ecClass doesn't have the ClassMap CA. 
    static BentleyStatus CreateFromECClass(bvector<ClassIndexInfoPtr>& indexInfos, ECDbCR, ECN::ECClassCR ecClass, ECN::ECDbClassMap const* customClassMap);

    Utf8CP GetName() const { return m_name.c_str();}
    bool GetIsUnique() const { return m_isUnique;}
    bvector<Utf8String> const& GetProperties() const{ return m_properties;}
    bool IsAddPropsAreNotNullWhereExp() const { return m_addPropsAreNotNullWhereExp; }
    };

//======================================================================================
// @bsiclass                                                Krischan.Eberle  02/2016
//+===============+===============+===============+===============+===============+======
struct ClassIndexInfoCache : NonCopyableClass
    {
private:
    ECDbCR m_ecdb;
    SchemaImportContext const& m_schemaImportContext;
    mutable bmap<ClassMap const*, bvector<ClassIndexInfoPtr>> m_indexInfoCache;

public:
    ClassIndexInfoCache(ECDbCR ecdb, SchemaImportContext const& ctx) : m_ecdb(ecdb), m_schemaImportContext(ctx) {}
    BentleyStatus TryGetIndexInfos(bvector<ClassIndexInfoPtr> const*& indexInfos, ClassMapCR) const;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
