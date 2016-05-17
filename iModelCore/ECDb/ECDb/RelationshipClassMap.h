/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/RelationshipClassMap.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "ClassMap.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      07/2014
//+===============+===============+===============+===============+===============+======
struct RelationshipConstraintMap : NonCopyableClass
    {
    private:
        ECDbSchemaManager const& m_schemaManager;
        ECN::ECRelationshipConstraintCR m_constraint;
        PropertyMapCP m_ecInstanceIdPropMap;
        PropertyMapRelationshipConstraintClassId const* m_ecClassIdPropMap;
        mutable bool m_anyClassMatches;
        mutable bool m_isCacheSetup;
        mutable std::set<ECN::ECClassId> m_ecClassIdCache;

        void CacheClassIds() const;
        void CacheClassIds(ECN::ECConstraintClassesList const& constraintClassList, bool constraintIsPolymorphic) const;

        void CacheClassId(ECN::ECClassId classId) const;
        void SetAnyClassMatches() const;

    public:
        RelationshipConstraintMap(ECDbSchemaManager const& schemaManager, ECN::ECRelationshipConstraintCR constraint)
            : m_schemaManager(schemaManager), m_constraint(constraint), m_ecInstanceIdPropMap(nullptr), m_ecClassIdPropMap(nullptr), m_anyClassMatches(false), m_isCacheSetup(false)
            {}

        PropertyMapCP GetECInstanceIdPropMap() const { return m_ecInstanceIdPropMap; }
        void SetECInstanceIdPropMap(PropertyMapCP ecinstanceIdPropMap) { m_ecInstanceIdPropMap = ecinstanceIdPropMap; }
        PropertyMapRelationshipConstraintClassId const* GetECClassIdPropMap() const { return m_ecClassIdPropMap; }
        void SetECClassIdPropMap(PropertyMapRelationshipConstraintClassId const* ecClassIdPropMap) { m_ecClassIdPropMap = ecClassIdPropMap; }

        bool ClassIdMatchesConstraint(ECN::ECClassId candidateClassId) const;
        bool TryGetSingleClassIdFromConstraint(ECN::ECClassId& classId) const;
        ECN::ECRelationshipConstraintCR GetRelationshipConstraint()const;
        bool IsSingleAbstractClass() const { return m_constraint.GetClasses().size() == 1 && m_constraint.GetClasses().front()->GetClassModifier() == ECN::ECClassModifier::Abstract; }
    };

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      06/2012
+===============+===============+===============+===============+===============+======*/
struct RelationshipClassMap : ClassMap
{
public:
    enum class ReferentialIntegrityMethod
        {
        None,
        ForeignKey,
        Trigger
        };

protected:
    static Utf8CP const DEFAULT_SOURCEECINSTANCEID_COLUMNNAME;
    static Utf8CP const DEFAULT_SOURCEECCLASSID_COLUMNNAME;
    static Utf8CP const DEFAULT_TARGETECINSTANCEID_COLUMNNAME;
    static Utf8CP const DEFAULT_TARGETECCLASSID_COLUMNNAME;

    RelationshipConstraintMap m_sourceConstraintMap;
    RelationshipConstraintMap m_targetConstraintMap;

    RelationshipClassMap (ECN::ECRelationshipClassCR ecRelClass, ECDbMapCR ecDbMap, ECDbMapStrategy mapStrategy, bool setIsDirty);
    ECDbSqlColumn* CreateConstraintColumn (Utf8CP columnName, ColumnKind columnId, PersistenceType);
    std::unique_ptr<ClassDbView> CreateClassDbView ();
    void DetermineConstraintClassIdColumnHandling (bool& addConstraintClassIdColumnNeeded, ECN::ECClassId& defaultConstraintClassId, ECN::ECRelationshipConstraintCR constraint) const;

    RelationshipConstraintMap& GetConstraintMapR(ECN::ECRelationshipEnd constraintEnd);

    static bool ConstraintIncludesAnyClass (ECN::ECConstraintClassesList const& constraintClasses);
    static RelationshipEndColumns const& GetEndColumnsMapping(RelationshipMapInfo const&, ECN::ECRelationshipEnd);

public:
    virtual ~RelationshipClassMap () {}

    ECN::ECRelationshipClassCR GetRelationshipClass () const { return *(GetClass ().GetRelationshipClassCP ()); }
    RelationshipConstraintMap const& GetConstraintMap (ECN::ECRelationshipEnd constraintEnd) const;
    PropertyMapCP GetConstraintECInstanceIdPropMap (ECN::ECRelationshipEnd constraintEnd) const;
    PropertyMapRelationshipConstraintClassId const* GetConstraintECClassIdPropMap (ECN::ECRelationshipEnd constraintEnd) const;

    PropertyMapCP GetSourceECInstanceIdPropMap () const { return m_sourceConstraintMap.GetECInstanceIdPropMap (); }
    PropertyMapRelationshipConstraintClassId const* GetSourceECClassIdPropMap () const { return m_sourceConstraintMap.GetECClassIdPropMap (); }
    PropertyMapCP GetTargetECInstanceIdPropMap () const { return m_targetConstraintMap.GetECInstanceIdPropMap (); }
    PropertyMapRelationshipConstraintClassId const* GetTargetECClassIdPropMap () const { return m_targetConstraintMap.GetECClassIdPropMap (); }
    
    virtual ReferentialIntegrityMethod _GetDataIntegrityEnforcementMethod() const = 0;
    virtual bool _RequiresJoin(ECN::ECRelationshipEnd) const;
    virtual bool _IsReadonly() const { return false; }
    };

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      06/2012
+===============+===============+===============+===============+===============+======*/
struct RelationshipClassEndTableMap : RelationshipClassMap
    {
private:
    bool m_autogenerateForeignKeyColumns;

    RelationshipClassEndTableMap (ECN::ECRelationshipClassCR ecRelClass, ECDbMapCR ecDbMap, ECDbMapStrategy mapStrategy, bool setIsDirty);
    virtual Type _GetClassMapType () const override { return Type::RelationshipEndTable; };

    bool GetRelationshipColumnName (Utf8StringR columnName, ECDbSqlTable const& table, Utf8CP prefix, bool mappingInProgress) const;

    void AddIndexToRelationshipEnd (SchemaImportContext&, ClassMapInfo const& mapInfo);

    bool GetReferencedEndKeyColumnName (Utf8StringR columnName, ECDbSqlTable const& table, bool mappingInProgress) const;
    virtual MappingStatus _MapPart1 (SchemaImportContext&, ClassMapInfo const& classMapInfo, IClassMap const* parentClassMap) override;
    virtual MappingStatus _MapPart2 (SchemaImportContext&, ClassMapInfo const& classMapInfo, IClassMap const* parentClassMap) override;

    //! Tries to retrieve the column to which the key property on the specified constraint is mapped to.
    //! @param[out] keyPropertyColumn found column or nullptr if no key property was defined on the constraint.
    //! @param[in] constraint Constraint
    //! @return SUCCESS if key property was found or no key property exists on the constraint. ERROR if constraint has more
    //! than one class or more than one key properties.
    BentleyStatus TryGetKeyPropertyColumn(std::set<ECDbSqlColumn const*>& keyPropertyColumns, ECN::ECRelationshipConstraintCR, ECN::ECRelationshipClassCR, ECN::ECRelationshipEnd constraintEnd) const;
    BentleyStatus TryGetConstraintIdColumnNameFromNavigationProperty(Utf8StringR, ECDbCR, ECN::ECRelationshipConstraintCR, ECN::ECRelationshipClassCR, ECN::ECRelationshipEnd constraintEnd) const;

    virtual BentleyStatus _Load (std::set<ClassMap const*>& loadGraph, ClassMapLoadContext&, ECDbClassMapInfo const&, IClassMap const* parentClassMap) override;

public:
    ~RelationshipClassEndTableMap () {}

    //!Gets the end in which the ForeignKey is persisted
    ECN::ECRelationshipEnd GetForeignEnd() const;
    //!Gets the end the ForeignKey end references
    ECN::ECRelationshipEnd GetReferencedEnd() const;

    PropertyMapCP GetForeignEndECInstanceIdPropMap () const;
    PropertyMapCP GetReferencedEndECInstanceIdPropMap () const;
    PropertyMapRelationshipConstraintClassId const* GetReferencedEndECClassIdPropMap () const;

    static ClassMapPtr Create (ECN::ECRelationshipClassCR ecRelClass, ECDbMapCR ecDbMap, ECDbMapStrategy mapStrategy, bool setIsDirty) { return new RelationshipClassEndTableMap (ecRelClass, ecDbMap, mapStrategy, setIsDirty); }
    virtual ReferentialIntegrityMethod _GetDataIntegrityEnforcementMethod() const override;
    virtual bool _RequiresJoin(ECN::ECRelationshipEnd) const override;
    virtual bool _IsReadonly() const override  { return !IsMappedToSingleTable(); }
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
    virtual Type _GetClassMapType () const override { return Type::RelationshipLinkTable; };

    virtual MappingStatus _MapPart1 (SchemaImportContext&, ClassMapInfo const& classMapInfo, IClassMap const* parentClassMap) override;
    virtual MappingStatus _MapPart2 (SchemaImportContext&, ClassMapInfo const& classMapInfo, IClassMap const* parentClassMap) override;

    MappingStatus CreateConstraintPropMaps (RelationshipMapInfo const&, bool addSourceECClassIdColumnToTable, ECN::ECClassId defaultSourceECClassid, bool addTargetECClassIdColumnToTable, ECN::ECClassId defaultTargetECClassId);

    void AddIndices (SchemaImportContext&, ClassMapInfo const&);
    void AddIndex(SchemaImportContext&, RelationshipIndexSpec, bool addUniqueIndex);

    bool GetConstraintECInstanceIdColumnName (Utf8StringR columnName, ECN::ECRelationshipEnd relationshipEnd, ECDbSqlTable const& table) const;
    virtual BentleyStatus _Load (std::set<ClassMap const*>& loadGraph, ClassMapLoadContext&, ECDbClassMapInfo const&, IClassMap const* parentClassMap) override;
    ECDbSqlColumn* ConfigureForeignECClassIdKey(RelationshipMapInfo const&, ECN::ECRelationshipEnd relationshipEnd);

    static void GenerateIndexColumnList(std::vector<ECDbSqlColumn const*>&, ECDbSqlColumn const* col1, ECDbSqlColumn const* col2, ECDbSqlColumn const* col3, ECDbSqlColumn const* col4);
    static bool HasKeyProperties(ECN::ECRelationshipConstraint const&);
public:
    ~RelationshipClassLinkTableMap () {}
    static ClassMapPtr Create (ECN::ECRelationshipClassCR ecRelClass, ECDbMapCR ecDbMap, ECDbMapStrategy mapStrategy, bool setIsDirty) { return new RelationshipClassLinkTableMap (ecRelClass, ecDbMap, mapStrategy, setIsDirty); }

    bool GetConstraintECClassIdColumnName (Utf8StringR columnName, ECN::ECRelationshipEnd relationshipEnd, ECDbSqlTable const& table) const;
    virtual ReferentialIntegrityMethod _GetDataIntegrityEnforcementMethod() const override;
};

END_BENTLEY_SQLITE_EC_NAMESPACE
