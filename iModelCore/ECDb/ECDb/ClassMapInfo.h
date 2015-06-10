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
BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//======================================================================================
// @bsiclass                                                 Krischan.Eberle  02/2014
//+===============+===============+===============+===============+===============+======
struct ClassMapInfoFactory
    {
private:
    ClassMapInfoFactory ();
    ~ClassMapInfoFactory ();

    static ClassMapInfoPtr CreateInstance(ECN::ECClassCR ecClass, ECDbMapCR ecDbMap, Utf8CP tableName, Utf8CP primaryKeyColumnName, ECDbMapStrategy mapStrategy);

public:
    static ClassMapInfoPtr CreateFromECDbMapCustomAttributes (MapStatus& mapStatus, SchemaImportContext const& schemaImportContext, ECN::ECClassCR ecClass, ECDbMapCR ecDbMap);
    };

//======================================================================================
//!This class grabs information from ClassMap ECCustomAttribute and evaluates
//! it along with other standard metadata on the ECClass... applying default 
//! rules like checking if isDomainClass=False, building standard table name, etc.
// @bsiclass                                                     Casey.Mullen      11/2011
//+===============+===============+===============+===============+===============+======
struct ClassMapInfo : public RefCountedBase, NonCopyableClass
{
private:
    Utf8String m_tableName;
    Utf8String m_ecInstanceIdColumnName;
    bvector<StandardKeySpecificationPtr> m_standardKeys;
    bvector<ClassIndexInfoPtr> m_dbIndexes; 
    IClassMap const* m_parentClassMap;
    bool m_isMapToVirtualTable;
    ECN::ECPropertyP m_ClassHasCurrentTimeStampProperty;
    ECDbMapStrategy m_strategy;
protected:

    ECDbMapCR m_ecDbMap;
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
    ClassMapInfo (ECN::ECClassCR ecClass, ECDbMapCR ecDbMap, Utf8CP tableName, Utf8CP primaryKeyColumnName, ECDbMapStrategy mapStrategy);
    virtual ~ClassMapInfo() {}
    virtual BentleyStatus _InitializeFromSchema();
    virtual MapStatus _EvaluateMapStrategy ();

    static void LogClassNotMapped (NativeLogging::SEVERITY severity, ECN::ECClassCR ecClass, Utf8CP explanation);

public:
    ECN::ECPropertyP GetClassHasCurrentTimeStampProperty() const { return m_ClassHasCurrentTimeStampProperty; }
    static ClassMapInfoPtr Create (ECN::ECClassCR ecClass, ECDbMapCR ecDbMap, Utf8CP tableName, Utf8CP primaryKeyColumnName, ECDbMapStrategy mapStrategy);
    virtual BentleyStatus _Initialize();

    ECDbMapStrategy const& GetMapStrategy () const{ return m_strategy; }
    ECDbMapStrategy& GetMapStrategyR (){ return m_strategy; }
    //! Evaluates the MapStrategy for the ECClass represented by this ClassMapInfo based on ClassMap ECCustomAttribute and
    //! default mapping rules. The results are stored in ClassMapInfo
    //! @remarks Assumes all base classes have been mapped already. 
    MapStatus EvaluateMapStrategy ();

    ECDbMapCR GetECDbMap() const {return m_ecDbMap;}
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

/*=================================================================================**//**
* This class grabs information from ECDbRelationshipConstraintHint ECCustomAttribute
* @bsiclass                                                     Affan.Khan       09/2014
+===============+===============+===============+===============+===============+======*/
struct RelationshipConstraintInfo
    {
    private:
        Utf8String m_ecClassIdColumn;
        Utf8String m_ecInstanceIdColumn;
        bool       m_enforceIntegrityCheck;
        ECDbSqlForeignKeyConstraint::ActionType m_onDeleteAction;
        ECDbSqlForeignKeyConstraint::ActionType m_onUpdateAction;
        ECDbSqlForeignKeyConstraint::MatchType  m_matchType;
        bool       m_generateDefaultIndex;
        bool       m_isEmpty;

    public:
        RelationshipConstraintInfo ()
            : m_enforceIntegrityCheck (false), m_onDeleteAction (ECDbSqlForeignKeyConstraint::ActionType::NotSpecified), m_onUpdateAction (ECDbSqlForeignKeyConstraint::ActionType::NotSpecified), m_isEmpty (true), m_generateDefaultIndex (true), m_matchType (ECDbSqlForeignKeyConstraint::MatchType::NotSpecified)
            {}

        Utf8StringCR GetECClassIdColumn () const { return m_ecClassIdColumn; }
        Utf8StringCR GetECInstanceIdColumn () const { return m_ecInstanceIdColumn; }
        bool DoEnforceIntegrityCheck () const { return m_enforceIntegrityCheck; }
        ECDbSqlForeignKeyConstraint::ActionType GetOnDeleteAction () const { return m_onDeleteAction; }
        ECDbSqlForeignKeyConstraint::ActionType GetOnUpdateAction () const { return m_onUpdateAction; }
        ECDbSqlForeignKeyConstraint::MatchType  GetMatchType () const { return m_matchType; }
        bool IsEmpty () const { return m_isEmpty; }
        bool GenerateDefaultIndex () const { return m_generateDefaultIndex; }

        static void ReadFromConstraint (RelationshipConstraintInfo& info, ECN::ECRelationshipConstraintCR constraint);
    };

/*=================================================================================**//**
* This class grabs information from ECDbRelationshipClassHint ECCustomAttribute and evaluates
* it along with other standard metadata on the ECClass... applying default 
* rules like checking if isDomainClass=False, building standard table name, etc.
* @bsiclass                                                     Casey.Mullen      11/2011
+===============+===============+===============+===============+===============+======*/
struct RelationshipClassMapInfo : public ClassMapInfo
{
public:
    enum class PreferredDirection
        {
        SourceToTarget,
        Bidirectional,
        TargetToSource,
        Unspecified
        };

    enum class CardinalityType
        {
        ManyToMany,
        OneToMany,
        ManyToOne,
        OneToOne
        };

    enum class TriState
        {
        True,
        False,
        Default,
        };

private:
    PreferredDirection m_userPreferredDirection;
    TriState m_allowDuplicateRelationships;
    CardinalityType m_relationshipCardinality;
    Utf8String m_sourceECInstanceIdColumn;
    Utf8String m_sourceECClassIdColumn;
    Utf8String m_targetECInstanceIdColumn;
    Utf8String m_targetECClassIdColumn;
    RelationshipConstraintInfo m_sourceInfo, m_targetInfo;

    virtual BentleyStatus _InitializeFromSchema () override;
    virtual MapStatus _EvaluateMapStrategy ();

    MapStatus DetermineRelationshipMapStrategy ();
    void DetermineCardinality ();
    bool ValidateRelatedClasses () const;

    Strategy Get11RelationshipMapStrategy (ECN::ECRelationshipConstraintR source, ECN::ECRelationshipConstraintR target, ECN::ECRelationshipClassCR relationshipClass) const;
    Strategy Get1MRelationshipMapStrategy (ECN::ECRelationshipConstraintR source, ECN::ECRelationshipConstraintR target, ECN::ECRelationshipClassCR relationshipClass) const;
    static CardinalityType CalculateRelationshipCardinality (ECN::ECRelationshipConstraintCR source, ECN::ECRelationshipConstraintCR target);
    static bool ContainsRelationshipClass (std::vector<ECN::ECClassCP> const& endClasses);

protected:
    RelationshipClassMapInfo (ECN::ECRelationshipClassCR relationshipClass, ECDbMapCR ecDbMap, Utf8CP tableName, Utf8CP primaryKeyColumnName, ECDbMapStrategy mapStrategy);
    ~RelationshipClassMapInfo () {} // RefCounted pattern


public:
    static ClassMapInfoPtr Create (ECN::ECRelationshipClassCR relationshipClass, ECDbMapCR ecDbMap, Utf8CP tableName, Utf8CP primaryKeyColumnName, ECDbMapStrategy mapStrategy);
    virtual BentleyStatus _Initialize() override;

    PreferredDirection GetUserPreferredDirection () const {return m_userPreferredDirection;}
    TriState GetAllowDuplicateRelationships() const {return m_allowDuplicateRelationships;}
    Utf8StringCR GetSourceECInstanceIdColumn() const {return m_sourceECInstanceIdColumn;}
    Utf8StringCR GetSourceECClassIdColumn() const {return m_sourceECClassIdColumn;}
    Utf8StringCR GetTargetECInstanceIdColumn() const {return m_targetECInstanceIdColumn;}
    Utf8StringCR GetTargetECClassIdColumn() const {return m_targetECClassIdColumn;}

    CardinalityType GetCardinality() const {return m_relationshipCardinality;}

    RelationshipConstraintInfo const& GetSourceInfo () const { return m_sourceInfo; }
    RelationshipConstraintInfo const& GetTargetInfo () const { return m_targetInfo; }
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
                LOG.errorv("Invalid where clause in ECDbClassMap::DbIndex: %s. Only ECDB_NOTNULL supported by ECDb.", dbIndex.GetWhereClause());
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
