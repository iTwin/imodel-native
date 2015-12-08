/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ClassMapInfo.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "MapStrategy.h"

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
    static std::unique_ptr<ClassMapInfo> Create(MapStatus&, SchemaImportContext const&, ECN::ECClassCR, ECDbMapCR);
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
    bool m_isECInstanceIdAutogenerationDisabled;
    ECN::ECPropertyCP m_classHasCurrentTimeStampProperty;

protected:
    ECDbMapCR m_ecdbMap;
    ECN::ECClassCR m_ecClass;
    ECDbMapStrategy m_resolvedStrategy;

private:
    BentleyStatus InitializeFromClassMapCA ();
    BentleyStatus InitializeDisableECInstanceIdAutogeneration();
    BentleyStatus InitializeFromClassHasCurrentTimeStampProperty();

    BentleyStatus DoEvaluateMapStrategy(bool& baseClassesNotMappedYet, UserECDbMapStrategy&);

    bool GatherBaseClassMaps (bvector<IClassMap const*>& baseClassMaps, bvector<IClassMap const*>& tphMaps, bvector<IClassMap const*>& tpcMaps, bvector<IClassMap const*>& nmhMaps, ECN::ECClassCR ecClass) const;

    bool ValidateChildStrategy(ECDbMapStrategy const& parentStrategy, UserECDbMapStrategy const& childStrategy) const;

    BentleyStatus ProcessStandardKeys(ECN::ECClassCR ecClass, Utf8CP customAttributeName);

protected:
    virtual BentleyStatus _InitializeFromSchema();
    virtual MapStatus _EvaluateMapStrategy();

    static void LogClassNotMapped (NativeLogging::SEVERITY severity, ECN::ECClassCR ecClass, Utf8CP explanation);

public:
    ClassMapInfo(ECN::ECClassCR, ECDbMapCR);
    virtual ~ClassMapInfo() {}

    MapStatus Initialize();

    ECN::ECPropertyCP GetClassHasCurrentTimeStampProperty() const { return m_classHasCurrentTimeStampProperty; }
    bool IsECInstanceIdAutogenerationDisabled() const { return m_isECInstanceIdAutogenerationDisabled; }

    ECDbMapStrategy const& GetMapStrategy () const{ return m_resolvedStrategy; }

    ECDbMapCR GetECDbMap() const {return m_ecdbMap;}
    ECN::ECClassCR GetECClass() const {return m_ecClass;}
    bvector<ClassIndexInfoPtr> const& GetIndexInfo() const { return m_dbIndexes;}
    Utf8CP GetTableName() const {return m_tableName.c_str();}
    Utf8CP GetECInstanceIdColumnName() const {return m_ecInstanceIdColumnName.c_str();}
    IClassMap const* GetParentClassMap () const { return m_parentClassMap; }
    bvector<StandardKeySpecificationPtr>const& GetStandardKeys() const {return m_standardKeys;}

    //! Virtual tables are not persisted   
    bool IsMapToVirtualTable () const { return m_isMapToVirtualTable; }
    void RestoreSaveSettings (ECDbMapStrategy mapStrategy, Utf8CP tableName){ m_resolvedStrategy = mapStrategy; if (tableName != nullptr) m_tableName = tableName; }
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
    bool m_createForeignKeyConstraint;
    ForeignKeyActionType m_onDeleteAction;
    ForeignKeyActionType m_onUpdateAction;
    bool m_createIndexOnForeignKey;
    CustomMapType m_customMapType;

    virtual BentleyStatus _InitializeFromSchema() override;
    virtual MapStatus _EvaluateMapStrategy();
    void DetermineCardinality(ECN::ECRelationshipConstraintCR source, ECN::ECRelationshipConstraintCR target);

public:
    RelationshipMapInfo(ECN::ECRelationshipClassCR relationshipClass, ECDbMapCR ecdbMap)
        : ClassMapInfo(relationshipClass, ecdbMap), m_sourceColumnsMappingIsNull(true), m_targetColumnsMappingIsNull(true), 
        m_allowDuplicateRelationships(false), m_createForeignKeyConstraint(false), m_onDeleteAction(ForeignKeyActionType::NotSpecified),
        m_onUpdateAction(ForeignKeyActionType::NotSpecified), m_createIndexOnForeignKey(true), m_customMapType(CustomMapType::None)
        {}

    virtual ~RelationshipMapInfo() {}

    Cardinality GetCardinality() const { return m_cardinality; }
    RelationshipEndColumns const& GetSourceColumnsMapping() const { BeAssert(m_customMapType != CustomMapType::ForeignKeyOnTarget && m_resolvedStrategy.GetStrategy() != ECDbMapStrategy::Strategy::ForeignKeyRelationshipInTargetTable); return m_sourceColumnsMapping; }
    RelationshipEndColumns const& GetTargetColumnsMapping() const { BeAssert(m_customMapType != CustomMapType::ForeignKeyOnSource && m_resolvedStrategy.GetStrategy() != ECDbMapStrategy::Strategy::ForeignKeyRelationshipInSourceTable); return m_targetColumnsMapping; }
    bool AllowDuplicateRelationships() const { BeAssert((m_customMapType == CustomMapType::LinkTable || m_customMapType == CustomMapType::None) && !m_resolvedStrategy.IsForeignKeyMapping()); return m_allowDuplicateRelationships; }

    bool CreateForeignKeyConstraint() const { BeAssert(m_customMapType != CustomMapType::LinkTable && m_resolvedStrategy.IsForeignKeyMapping()); return m_createForeignKeyConstraint;}
    ForeignKeyActionType GetOnDeleteAction() const { BeAssert(CreateForeignKeyConstraint()); return m_onDeleteAction; }
    ForeignKeyActionType GetOnUpdateAction() const { BeAssert(CreateForeignKeyConstraint()); return m_onUpdateAction; }

    bool CreateIndexOnForeignKey() const { BeAssert(m_customMapType != CustomMapType::LinkTable && m_resolvedStrategy.IsForeignKeyMapping()); return m_createIndexOnForeignKey; }
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
        static Type GetTypeFromString(Utf8CP customAttributeName)
            {
            Type keyType = Type::None;
            if (BeStringUtilities::Stricmp(customAttributeName, "SyncIDSpecification") == 0)
                keyType = Type::SyncIDSpecification;
            else if (BeStringUtilities::Stricmp(customAttributeName, "GlobalIdSpecification") == 0)
                keyType = Type::GlobalIdSpecification;
            else if (BeStringUtilities::Stricmp(customAttributeName, "BusinessKeySpecification") == 0)
                keyType = Type::BusinessKeySpecification;

            return keyType;
            }
        static Utf8String TypeToString(Type keyType)
            {
            if (keyType == Type::SyncIDSpecification)
                return "SyncIDSpecification";
            if (keyType == Type::GlobalIdSpecification)
                return "GlobalIdSpecification";
            if (keyType == Type::BusinessKeySpecification)
                return "BusinessKeySpecification";

            return "";
            }
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
