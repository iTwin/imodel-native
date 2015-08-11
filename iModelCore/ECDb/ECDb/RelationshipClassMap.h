/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/RelationshipClassMap.h $
|
|  $Copyright: (c) 2015 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "ClassMap.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      06/2012
+===============+===============+===============+===============+===============+======*/
struct RelationshipClassMap : ClassMap
{
protected:
    //=======================================================================================
    // @bsiclass                                                 Krischan.Eberle      07/2014
    //+===============+===============+===============+===============+===============+======
    struct ConstraintMap : NonCopyableClass
        {
    private:
        ECDbSchemaManager const& m_schemaManager;
        ECN::ECRelationshipConstraintCR m_constraint;
        PropertyMapCP m_ecInstanceIdPropMap;
        PropertyMapCP m_ecClassIdPropMap;
        mutable bool m_anyClassMatches;
        mutable bool m_isCacheSetup;
        mutable std::set<ECN::ECClassId> m_ecClassIdCache;

        void CacheClassIds () const;
        void CacheClassIds (bvector<ECN::ECClassP> const& constraintClassList, bool constraintIsPolymorphic) const;

        void CacheClassId (ECN::ECClassId classId) const;
        void SetAnyClassMatches () const;

    public:
        ConstraintMap (ECDbSchemaManager const& schemaManager, ECN::ECRelationshipConstraintCR constraint)
            : m_schemaManager (schemaManager), m_constraint (constraint), m_ecInstanceIdPropMap (nullptr), m_ecClassIdPropMap (nullptr), m_anyClassMatches (false), m_isCacheSetup (false)
            {}

        PropertyMapCP GetECInstanceIdPropMap () const { return m_ecInstanceIdPropMap; }
        void SetECInstanceIdPropMap (PropertyMapCP ecinstanceIdPropMap) { m_ecInstanceIdPropMap = ecinstanceIdPropMap; }
        PropertyMapCP GetECClassIdPropMap () const { return m_ecClassIdPropMap; }
        void SetECClassIdPropMap (PropertyMapCP ecClassIdPropMap) { m_ecClassIdPropMap = ecClassIdPropMap; }

        bool ClassIdMatchesConstraint (ECN::ECClassId candidateClassId) const;
        bool TryGetSingleClassIdFromConstraint (ECN::ECClassId& classId) const;
        ECN::ECRelationshipConstraintCR GetRelationshipConstraint()const;
        };

    static Utf8CP const DEFAULT_SOURCEECINSTANCEID_COLUMNNAME;
    static Utf8CP const DEFAULT_SOURCEECCLASSID_COLUMNNAME;
    static Utf8CP const DEFAULT_TARGETECINSTANCEID_COLUMNNAME;
    static Utf8CP const DEFAULT_TARGETECCLASSID_COLUMNNAME;

    ConstraintMap m_sourceConstraintMap;
    ConstraintMap m_targetConstraintMap;

    RelationshipClassMap (ECN::ECRelationshipClassCR ecRelClass, ECDbMapCR ecDbMap, ECDbMapStrategy mapStrategy, bool setIsDirty);

    ECDbSqlColumn* CreateConstraintColumn (Utf8CP columnName, bool addToTable = false);
    std::unique_ptr<ClassDbView> CreateClassDbView ();

    void DetermineConstraintClassIdColumnHandling (bool& addConstraintClassIdColumnNeeded, ECN::ECClassId& defaultConstraintClassId, ECN::ECRelationshipConstraintCR constraint) const;
    static bool ConstraintIncludesAnyClass (ECN::ECConstraintClassesList const& constraintClasses);
    static RelationshipEndColumns const& GetEndColumnsMapping(RelationshipMapInfo const&, ECN::ECRelationshipEnd);

public:
    virtual ~RelationshipClassMap () {}

    ECN::ECRelationshipClassCR GetRelationshipClass () const { return *(GetClass ().GetRelationshipClassCP ()); }

    ConstraintMap const& GetConstraintMap (ECN::ECRelationshipEnd constraintEnd) const;

    PropertyMapCP GetConstraintECInstanceIdPropMap (ECN::ECRelationshipEnd constraintEnd) const;
    PropertyMapCP GetConstraintECClassIdPropMap (ECN::ECRelationshipEnd constraintEnd) const;

    PropertyMapCP GetSourceECInstanceIdPropMap () const { return m_sourceConstraintMap.GetECInstanceIdPropMap (); }
    PropertyMapCP GetSourceECClassIdPropMap () const { return m_sourceConstraintMap.GetECClassIdPropMap (); }
    PropertyMapCP GetTargetECInstanceIdPropMap () const { return m_targetConstraintMap.GetECInstanceIdPropMap (); }
    PropertyMapCP GetTargetECClassIdPropMap () const { return m_targetConstraintMap.GetECClassIdPropMap (); }

    };

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      06/2012
+===============+===============+===============+===============+===============+======*/
struct RelationshipClassEndTableMap : RelationshipClassMap
    {
private:
    RelationshipClassEndTableMap (ECN::ECRelationshipClassCR ecRelClass, ECDbMapCR ecDbMap, ECDbMapStrategy mapStrategy, bool setIsDirty);
    virtual Type _GetClassMapType () const override { return Type::RelationshipEndTable; };

    bool GetRelationshipColumnName (Utf8StringR columnName, ECDbSqlTable const& table, Utf8CP prefix, bool mappingInProgress) const;

    void AddIndices (ClassMapInfo const& mapInfo);
    void AddIndexToRelationshipEnd (bool isUniqueIndex);

    RelationshipEndColumns const& GetEndColumnsMapping(RelationshipMapInfo const&) const;

    bool GetOtherEndKeyColumnName (Utf8StringR columnName, ECDbSqlTable const& table, bool mappingInProgress) const;
    bool GetOtherEndECClassIdColumnName (Utf8StringR columnName, ECDbSqlTable const& table, bool mappingInProgress) const;
    virtual MapStatus _InitializePart1 (ClassMapInfo const& classMapInfo, IClassMap const* parentClassMap) override;
    virtual MapStatus _InitializePart2 (ClassMapInfo const& classMapInfo, IClassMap const* parentClassMap) override;

    //! Tries to retrieve the column to which the key property on the specified constraint is mapped to.
    //! @param[out] keyPropertyColumn found column or nullptr if no key property was defined on the constraint.
    //! @param[in] constraint Constraint
    //! @return SUCCESS if key property was found or no key property exists on the constraint. ERROR if constraint has more
    //! than one class or more than one key properties.
    BentleyStatus TryGetKeyPropertyColumn(ECDbSqlColumn const*& keyPropertyColumn, ECN::ECRelationshipConstraintCR, ECN::ECRelationshipClassCR, ECN::ECRelationshipEnd constraintEnd) const;

    MapStatus CreateConstraintColumns(ECDbSqlColumn*& otherEndECInstanceIdColumn, RelationshipMapInfo const&, ECN::ECRelationshipEnd constraintEnd, ECN::ECRelationshipConstraintCR);
    MapStatus CreateConstraintPropMaps (ECN::ECRelationshipEnd thisEnd, ECN::ECClassId defaultThisEndClassId, ECDbSqlColumn* const& otherEndECInstanceIdColumn, ECDbSqlColumn* const& otherEndECClassIdColumn, ECN::ECClassId defaultOtherEndClassId);
    ECDbSqlColumn* ConfigureForeignECClassIdKey(RelationshipMapInfo const&, ECN::ECRelationshipConstraintCR otherEndConstraint, IClassMap const& otheEndClassMap, size_t otherEndTableCount);
    ECN::ECRelationshipEnd GetOtherEnd () const;

    virtual BentleyStatus _Load (std::set<ClassMap const*>& loadGraph, ECDbClassMapInfo const& mapInfo, IClassMap const* parentClassMap) override;
    virtual BentleyStatus _EvaluateDMLPolicy () override
        {
        GetDMLPolicyR ().Set (DMLPolicy::Operation::Insert, DMLPolicy::Target::View);
        GetDMLPolicyR ().Set (DMLPolicy::Operation::Update, DMLPolicy::Target::View);
        GetDMLPolicyR ().Set (DMLPolicy::Operation::Delete, DMLPolicy::Target::View);
        GetDMLPolicyR ().Set (DMLPolicy::Operation::Select, DMLPolicy::Target::View);
        return BentleyStatus::SUCCESS;
        }

public:
    ~RelationshipClassEndTableMap () {}

    ECN::ECRelationshipEnd GetThisEnd () const;

    PropertyMapCP GetThisEndECInstanceIdPropMap () const;
    PropertyMapCP GetThisEndECClassIdPropMap () const;
    PropertyMapCP GetOtherEndECInstanceIdPropMap () const;
    PropertyMapCP GetOtherEndECClassIdPropMap () const;

    bool GetOtherEndECClassIdColumnName (Utf8StringR columnName, ECDbSqlTable const& table) const { return GetOtherEndECClassIdColumnName (columnName, table, false);}

    static ClassMapPtr Create (ECN::ECRelationshipClassCR ecRelClass, ECDbMapCR ecDbMap, ECDbMapStrategy mapStrategy, bool setIsDirty) { return new RelationshipClassEndTableMap (ecRelClass, ecDbMap, mapStrategy, setIsDirty); }
    };

/*==========================================================================
* @bsiclass                                                 Ramanujam.Raman      06/2012
+===============+===============+===============+===============+===============+======*/
struct RelationshipClassLinkTableMap : RelationshipClassMap
{
private:
    enum class RelationshipIndexSpec
        {
        Source,
        Target,
        SourceAndTarget
        };

private:
    RelationshipClassLinkTableMap (ECN::ECRelationshipClassCR ecRelClass, ECDbMapCR ecDbMap, ECDbMapStrategy mapStrategy, bool setIsDirty);
    virtual Type  _GetClassMapType () const override { return Type::RelationshipLinkTable; };

    virtual MapStatus   _InitializePart1 (ClassMapInfo const& classMapInfo, IClassMap const* parentClassMap) override;
    virtual MapStatus   _InitializePart2 (ClassMapInfo const& classMapInfo, IClassMap const* parentClassMap) override;

    MapStatus           CreateConstraintPropMaps (RelationshipMapInfo const&, bool addSourceECClassIdColumnToTable, ECN::ECClassId defaultSourceECClassid, bool addTargetECClassIdColumnToTable, ECN::ECClassId defaultTargetECClassId);

    void                AddIndices (ClassMapInfo const& mapInfo);
    void                AddIndicesToRelationshipEnds (RelationshipIndexSpec spec, bool addUniqueIndex);
    ECDbSqlIndex*       CreateIndex (RelationshipIndexSpec spec, bool uniqueIndex);
    static void         AddColumnsToIndex (ECDbSqlIndex& index, ECDbSqlColumn const* col1, ECDbSqlColumn const* col2, ECDbSqlColumn const* col3, ECDbSqlColumn const* col4);

    bool                GetConstraintECInstanceIdColumnName (Utf8StringR columnName, ECN::ECRelationshipEnd relationshipEnd, ECDbSqlTable const& table) const;
    virtual BentleyStatus _Load (std::set<ClassMap const*>& loadGraph, ECDbClassMapInfo const&, IClassMap const* parentClassMap) override;
    ECDbSqlColumn*      ConfigureForeignECClassIdKey(RelationshipMapInfo const&, ECN::ECRelationshipEnd relationshipEnd);
public:
    ~RelationshipClassLinkTableMap () {}
    static ClassMapPtr  Create (ECN::ECRelationshipClassCR ecRelClass, ECDbMapCR ecDbMap, ECDbMapStrategy mapStrategy, bool setIsDirty) { return new RelationshipClassLinkTableMap (ecRelClass, ecDbMap, mapStrategy, setIsDirty); }

    bool                GetConstraintECClassIdColumnName (Utf8StringR columnName, ECN::ECRelationshipEnd relationshipEnd, ECDbSqlTable const& table) const;
};

END_BENTLEY_SQLITE_EC_NAMESPACE
