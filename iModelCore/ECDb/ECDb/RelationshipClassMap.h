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
        RelConstraintECClassIdPropertyMap const* m_ecClassIdPropMap;
        mutable bool m_anyClassMatches;
        mutable bool m_isCacheSetup;
        mutable std::set<ECN::ECClassId> m_ecClassIdCache;

        void CacheClassIds() const;
        void CacheClassIds(ECN::ECRelationshipConstraintClassList const&, bool constraintIsPolymorphic) const;
        void CacheClassId(ECN::ECClassCR, bool constraintIsPolymorphic) const;
        void SetAnyClassMatches() const;

    public:
        RelationshipConstraintMap(ECDbSchemaManager const& schemaManager, ECN::ECRelationshipConstraintCR constraint)
            : m_schemaManager(schemaManager), m_constraint(constraint), m_ecInstanceIdPropMap(nullptr), m_ecClassIdPropMap(nullptr), m_anyClassMatches(false), m_isCacheSetup(false)
            {}

        PropertyMapCP GetECInstanceIdPropMap() const { return m_ecInstanceIdPropMap; }
        void SetECInstanceIdPropMap(PropertyMapCP ecinstanceIdPropMap) { m_ecInstanceIdPropMap = ecinstanceIdPropMap; }
        RelConstraintECClassIdPropertyMap const* GetECClassIdPropMap() const { return m_ecClassIdPropMap; }
        void SetECClassIdPropMap(RelConstraintECClassIdPropertyMap const* ecClassIdPropMap) { m_ecClassIdPropMap = ecClassIdPropMap; }

        bool ClassIdMatchesConstraint(ECN::ECClassId const& candidateClassId) const;
        bool TryGetSingleClassIdFromConstraint(ECN::ECClassId&) const;
        ECN::ECRelationshipConstraintCR GetRelationshipConstraint() const { return m_constraint; }
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

        RelationshipClassMap(Type, ECN::ECRelationshipClassCR, ECDbMap const&, ECDbMapStrategy const&, bool setIsDirty);
        DbColumn* CreateConstraintColumn(Utf8CP columnName, DbColumn::Kind columnId, PersistenceType);
        void DetermineConstraintClassIdColumnHandling(bool& addConstraintClassIdColumnNeeded, ECN::ECClassId& defaultConstraintClassId, ECN::ECRelationshipConstraintCR constraint) const;

        RelationshipConstraintMap& GetConstraintMapR(ECN::ECRelationshipEnd);

        static bool ConstraintIncludesAnyClass(ECN::ECRelationshipConstraintClassList const&);
        static RelationshipEndColumns const& GetEndColumnsMapping(RelationshipMappingInfo const&, ECN::ECRelationshipEnd);

    public:
        virtual ~RelationshipClassMap() {}

        ECN::ECRelationshipClassCR GetRelationshipClass() const { return *(GetClass().GetRelationshipClassCP()); }
        RelationshipConstraintMap const& GetConstraintMap(ECN::ECRelationshipEnd) const;
        PropertyMapCP GetConstraintECInstanceIdPropMap(ECN::ECRelationshipEnd) const;
        RelConstraintECClassIdPropertyMap const* GetConstraintECClassIdPropMap(ECN::ECRelationshipEnd) const;

        PropertyMapCP GetSourceECInstanceIdPropMap() const { return m_sourceConstraintMap.GetECInstanceIdPropMap(); }
        RelConstraintECClassIdPropertyMap const* GetSourceECClassIdPropMap() const { return m_sourceConstraintMap.GetECClassIdPropMap(); }
        PropertyMapCP GetTargetECInstanceIdPropMap() const { return m_targetConstraintMap.GetECInstanceIdPropMap(); }
        RelConstraintECClassIdPropertyMap const* GetTargetECClassIdPropMap() const { return m_targetConstraintMap.GetECClassIdPropMap(); }

        virtual ReferentialIntegrityMethod _GetDataIntegrityEnforcementMethod() const = 0;
        virtual bool _RequiresJoin(ECN::ECRelationshipEnd) const;
    };

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      06/2012
+===============+===============+===============+===============+===============+======*/
struct RelationshipClassEndTableMap : RelationshipClassMap
    {
    private:
        struct ColumnLists : NonCopyableClass
            {
            std::vector<DbColumn const*> m_fkTablesECInstanceIdColumns;
            std::vector<DbColumn const*> m_fkTablesECClassIdColumns;
            std::vector<DbColumn const*> m_fkTablesFkColumns;
            //The referenced end class id cols are either from the FK table, or if the referenced table has its own class id column, that one is taken.
            //WIP_FOR_AFFAN: Is this safe enough? Does consuming code know that the prop map has columns to another table??
            std::vector<DbColumn const*> m_referencedEndECClassIdColumns;
            };

        struct ForeignKeyColumnInfo : NonCopyableClass
            {
            private:
                bool m_canImplyFromNavigationProperty;
                Utf8String m_impliedColumnName;
                bool m_appendToEnd;
                PropertyMapCP m_propMapBeforeNavProp;
                PropertyMapCP m_propMapAfterNavProp;

            public:
                ForeignKeyColumnInfo() : m_canImplyFromNavigationProperty(false), m_appendToEnd(true), m_propMapBeforeNavProp(nullptr), m_propMapAfterNavProp(nullptr) {}

                void Assign(Utf8CP impliedColName, bool appendToEnd, PropertyMapCP propMapBeforeNavProp, PropertyMapCP propMapAfterNavProp)
                    {
                    m_canImplyFromNavigationProperty = true;
                    m_impliedColumnName.assign(impliedColName);
                    m_appendToEnd = appendToEnd;
                    m_propMapBeforeNavProp = propMapBeforeNavProp;
                    m_propMapAfterNavProp = propMapAfterNavProp;
                    }

                void Clear()
                    {
                    m_canImplyFromNavigationProperty = false;
                    m_impliedColumnName.clear();
                    m_appendToEnd = true;
                    m_propMapBeforeNavProp = nullptr;
                    m_propMapAfterNavProp = nullptr;
                    }

                bool CanImplyFromNavigationProperty() const { return m_canImplyFromNavigationProperty; }
                Utf8StringCR GetImpliedColumnName() const { return m_impliedColumnName; }
                bool AppendToEnd() const { return m_appendToEnd; }
                PropertyMapCP GetPropertyMapBeforeNavProp() const { return m_propMapBeforeNavProp; }
                PropertyMapCP GetPropertyMapAfterNavProp() const { return m_propMapAfterNavProp; }
            };

        static Utf8CP const DEFAULT_FKCOLUMNNAME_PREFIX;

        bool m_autogenerateForeignKeyColumns;

        RelationshipClassEndTableMap(ECN::ECRelationshipClassCR, ECDbMap const&, ECDbMapStrategy const&, bool setIsDirty);

        void AddIndexToRelationshipEnd(SchemaImportContext&, ClassMappingInfo const&);

        virtual MappingStatus _MapPart1(SchemaImportContext&, ClassMappingInfo const&) override;
        virtual MappingStatus _MapPart2(SchemaImportContext&, ClassMappingInfo const&) override;

        BentleyStatus DetermineKeyAndConstraintColumns(ColumnLists&, RelationshipMappingInfo const&);
        BentleyStatus MapSubClass(RelationshipMappingInfo const&, ClassMap const& baseClassMap);

        //! Tries to retrieve the column to which the key property on the specified constraint is mapped to.
        //! @param[out] keyPropertyColumn found column or nullptr if no key property was defined on the constraint.
        //! @param[in] constraint Constraint
        //! @return SUCCESS if key property was found or no key property exists on the constraint. ERROR if constraint has more
        //! than one class or more than one key properties.
        BentleyStatus TryGetKeyPropertyColumn(std::set<DbColumn const*>& keyPropertyColumns, ECN::ECRelationshipConstraintCR, ECN::ECRelationshipClassCR, ECN::ECRelationshipEnd) const;
        BentleyStatus TryGetForeignKeyColumnInfoFromNavigationProperty(ForeignKeyColumnInfo&, ECN::ECRelationshipConstraintCR, ECN::ECRelationshipClassCR, ECN::ECRelationshipEnd) const;
        BentleyStatus TryDetermineForeignKeyColumnPosition(int& position, DbTable const&, ForeignKeyColumnInfo const&) const;

        virtual BentleyStatus _Load(std::set<ClassMap const*>& loadGraph, ClassMapLoadContext&, ClassDbMapping const&, ClassMap const* parentClassMap) override;

        BentleyStatus ValidateForeignKeyColumn(DbColumn const& fkColumn, bool cardinalityImpliesNotNullOnFkCol, DbColumn::Kind) const;

    public:
        ~RelationshipClassEndTableMap() {}

        //!Gets the end in which the ForeignKey is persisted
        ECN::ECRelationshipEnd GetForeignEnd() const;
        //!Gets the end the ForeignKey end references
        ECN::ECRelationshipEnd GetReferencedEnd() const;

        PropertyMapCP GetForeignEndECInstanceIdPropMap() const;
        PropertyMapCP GetReferencedEndECInstanceIdPropMap() const;
        RelConstraintECClassIdPropertyMap const* GetReferencedEndECClassIdPropMap() const;

        static ClassMapPtr Create(ECN::ECRelationshipClassCR ecRelClass, ECDbMap const& ecDbMap, ECDbMapStrategy const& mapStrategy, bool setIsDirty) { return new RelationshipClassEndTableMap(ecRelClass, ecDbMap, mapStrategy, setIsDirty); }
        virtual ReferentialIntegrityMethod _GetDataIntegrityEnforcementMethod() const override;
        virtual bool _RequiresJoin(ECN::ECRelationshipEnd) const override;
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
        RelationshipClassLinkTableMap(ECN::ECRelationshipClassCR, ECDbMap const&, ECDbMapStrategy const&, bool setIsDirty);

        virtual MappingStatus _MapPart1(SchemaImportContext&, ClassMappingInfo const&) override;
        virtual MappingStatus _MapPart2(SchemaImportContext&, ClassMappingInfo const&) override;

        MappingStatus CreateConstraintPropMaps(RelationshipMappingInfo const&, bool addSourceECClassIdColumnToTable, ECN::ECClassId defaultSourceECClassid, bool addTargetECClassIdColumnToTable, ECN::ECClassId defaultTargetECClassId);

        void AddIndices(SchemaImportContext&, ClassMappingInfo const&);
        void AddIndex(SchemaImportContext&, RelationshipIndexSpec, bool addUniqueIndex);

        bool GetConstraintECInstanceIdColumnName(Utf8StringR columnName, ECN::ECRelationshipEnd relationshipEnd, DbTable const& table) const;
        virtual BentleyStatus _Load(std::set<ClassMap const*>& loadGraph, ClassMapLoadContext&, ClassDbMapping const&, ClassMap const* parentClassMap) override;
        DbColumn* ConfigureForeignECClassIdKey(RelationshipMappingInfo const&, ECN::ECRelationshipEnd relationshipEnd);

        static void GenerateIndexColumnList(std::vector<DbColumn const*>&, DbColumn const* col1, DbColumn const* col2, DbColumn const* col3, DbColumn const* col4);
    public:
        ~RelationshipClassLinkTableMap() {}
        static ClassMapPtr Create(ECN::ECRelationshipClassCR relClass, ECDbMap const& ecdbMap, ECDbMapStrategy const& mapStrategy, bool setIsDirty) { return new RelationshipClassLinkTableMap(relClass, ecdbMap, mapStrategy, setIsDirty); }

        bool GetConstraintECClassIdColumnName(Utf8StringR columnName, ECN::ECRelationshipEnd, DbTable const&) const;
        virtual ReferentialIntegrityMethod _GetDataIntegrityEnforcementMethod() const override;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
