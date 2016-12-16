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
        ECDb const& m_ecdb;
        ECN::ECClassId m_relClassId;
        ECN::ECRelationshipEnd m_constraintEnd;
        ECN::ECRelationshipConstraintCR m_constraint;
        ConstraintECInstanceIdPropertyMap const* m_ecInstanceIdPropMap;
        ConstraintECClassIdPropertyMap const* m_ecClassIdPropMap;
        bool m_anyClassMatches;

    public:
        RelationshipConstraintMap(ECDb const&, ECN::ECClassId relClassId, ECN::ECRelationshipEnd, ECN::ECRelationshipConstraintCR);

        ConstraintECInstanceIdPropertyMap const* GetECInstanceIdPropMap() const { return m_ecInstanceIdPropMap; }
        void SetECInstanceIdPropMap(ConstraintECInstanceIdPropertyMap const* ecinstanceIdPropMap) { m_ecInstanceIdPropMap = ecinstanceIdPropMap; }
        ConstraintECClassIdPropertyMap const* GetECClassIdPropMap() const { return m_ecClassIdPropMap; }
        void SetECClassIdPropMap(ConstraintECClassIdPropertyMap const* ecClassIdPropMap) { m_ecClassIdPropMap = ecClassIdPropMap; }

        bool TryGetSingleClassIdFromConstraint(ECN::ECClassId&) const;
        bool IsSingleAbstractClass() const { return m_constraint.GetClasses().size() == 1 && m_constraint.GetClasses().front()->GetClassModifier() == ECN::ECClassModifier::Abstract; }
        ECN::ECRelationshipConstraintCR GetRelationshipConstraint() const { return m_constraint; }
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

        RelationshipClassMap(ECDb const&, Type, ECN::ECRelationshipClassCR, MapStrategyExtendedInfo const&, bool setIsDirty);
        DbColumn* CreateConstraintColumn(Utf8CP columnName, DbColumn::Kind columnId, PersistenceType);
        void DetermineConstraintClassIdColumnHandling(bool& addConstraintClassIdColumnNeeded, ECN::ECClassId& defaultConstraintClassId, ECN::ECRelationshipConstraintCR constraint) const;

        RelationshipConstraintMap& GetConstraintMapR(ECN::ECRelationshipEnd constraintEnd);

    public:
        virtual ~RelationshipClassMap() {}

        ECN::ECRelationshipClassCR GetRelationshipClass() const { return *(GetClass().GetRelationshipClassCP()); }
        RelationshipConstraintMap const& GetConstraintMap(ECN::ECRelationshipEnd constraintEnd) const;
        ConstraintECInstanceIdPropertyMap const* GetConstraintECInstanceIdPropMap(ECN::ECRelationshipEnd constraintEnd) const;
        ConstraintECClassIdPropertyMap const* GetConstraintECClassIdPropMap(ECN::ECRelationshipEnd) const;

        ConstraintECInstanceIdPropertyMap const* GetSourceECInstanceIdPropMap() const { return m_sourceConstraintMap.GetECInstanceIdPropMap(); }
        ConstraintECClassIdPropertyMap const* GetSourceECClassIdPropMap() const { return m_sourceConstraintMap.GetECClassIdPropMap(); }
        ConstraintECInstanceIdPropertyMap const* GetTargetECInstanceIdPropMap() const { return m_targetConstraintMap.GetECInstanceIdPropMap(); }
        ConstraintECClassIdPropertyMap const* GetTargetECClassIdPropMap() const { return m_targetConstraintMap.GetECClassIdPropMap(); }

        virtual ReferentialIntegrityMethod _GetDataIntegrityEnforcementMethod() const = 0;
        virtual bool _RequiresJoin(ECN::ECRelationshipEnd) const;

        static bool ConstraintIncludesAnyClass(ECN::ECRelationshipConstraintClassList const&);
    };

typedef RelationshipClassMap const& RelationshipClassMapCR;

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      06/2012
+===============+===============+===============+===============+===============+======*/
struct RelationshipClassEndTableMap : RelationshipClassMap
    {
    private:
        struct ColumnLists : NonCopyableClass
            {
            std::vector<DbColumn const*> m_relECClassIdColumnsPerFkTable; //Rel ECClassId
            std::vector<DbColumn const*> m_ecInstanceIdColumnsPerFkTable; //ForeignEnd ECInstanceId
            std::vector<DbColumn const*> m_classIdColumnsPerFkTable; //ForeignEnd ECClassId
            std::vector<DbColumn const*> m_relClassIdColumnsPerFkTable; //ForeignEnd ECClassId
            std::vector<DbColumn const*> m_fkColumnsPerFkTable; //ReferencedEnd ECInstanceId
            //The referenced end class id cols are either from the FK table, or if the referenced table has its own class id column, that one is taken.
            //WIP_FOR_AFFAN: Is this safe enough? Does consuming code know that the prop map has columns to another table??
            std::vector<DbColumn const*> m_referencedEndECClassIdColumns; //ReferencedEnd ECClassId in the referenced table or fk table
            static void push_back(std::vector<DbColumn const*>& list, DbColumn const* column)
                {
                BeAssert(column != nullptr);
                if (std::find(list.begin(), list.end(), column) == list.end())
                    list.push_back(column);
                }
            };

        struct ForeignKeyColumnInfo : NonCopyableClass
            {
            private:
                bool m_canImplyFromNavigationProperty;
                Utf8String m_impliedColumnName;
                bool m_appendToEnd;
                PropertyMap const* m_propMapBeforeNavProp;
                PropertyMap const* m_propMapAfterNavProp;

            public:
                ForeignKeyColumnInfo() : m_canImplyFromNavigationProperty(false), m_appendToEnd(true), m_propMapBeforeNavProp(nullptr), m_propMapAfterNavProp(nullptr) {}

                void Assign(Utf8CP impliedColName, bool appendToEnd, PropertyMap const* propMapBeforeNavProp, PropertyMap const* propMapAfterNavProp)
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
                PropertyMap const* GetPropertyMapBeforeNavProp() const { return m_propMapBeforeNavProp; }
                PropertyMap const* GetPropertyMapAfterNavProp() const { return m_propMapAfterNavProp; }
            };

        RelationshipClassEndTableMap(ECDb const&, ECN::ECRelationshipClassCR, MapStrategyExtendedInfo const&, bool setIsDirty);

        void AddIndexToRelationshipEnd(ClassMappingContext&);

        virtual ClassMappingStatus _Map(ClassMappingContext&) override;
        DbColumn* CreateRelECClassIdColumn(DbTable&, Utf8StringCR colName, bool makeNotNull) const;

        BentleyStatus DetermineKeyAndConstraintColumns(ColumnLists&, RelationshipMappingInfo const&);
        BentleyStatus DetermineFkColumns(ColumnLists&, RelationshipMappingInfo const&);
        Utf8String DetermineFkColumnName(RelationshipMappingInfo const&, ForeignKeyColumnInfo const&) const;
        static Utf8String DetermineRelECClassIdColumnName(ECN::ECRelationshipClassCR, Utf8StringCR fkColumnName);
        BentleyStatus MapSubClass(RelationshipMappingInfo const&);

        BentleyStatus TryGetForeignKeyColumnInfoFromNavigationProperty(ForeignKeyColumnInfo&, ECN::ECRelationshipConstraintCR, ECN::ECRelationshipClassCR, ECN::ECRelationshipEnd) const;
        BentleyStatus TryDetermineForeignKeyColumnPosition(int& position, DbTable const&, ForeignKeyColumnInfo const&) const;

        virtual BentleyStatus _Load(ClassMapLoadContext&, DbClassMapLoadContext const&) override;

        BentleyStatus ValidateForeignKeyColumn(DbColumn const& fkColumn, bool cardinalityImpliesNotNullOnFkCol, DbColumn::Kind) const;

    public:
        ~RelationshipClassEndTableMap() {}

        //!Gets the end in which the ForeignKey is persisted
        ECN::ECRelationshipEnd GetForeignEnd() const;
        //!Gets the end the ForeignKey end references
        ECN::ECRelationshipEnd GetReferencedEnd() const;

        ConstraintECInstanceIdPropertyMap const* GetForeignEndECInstanceIdPropMap() const;
        ConstraintECInstanceIdPropertyMap const* GetReferencedEndECInstanceIdPropMap() const;
        ConstraintECClassIdPropertyMap const* GetReferencedEndECClassIdPropMap() const;
        ConstraintECClassIdPropertyMap const* GetForeignEndECClassIdPropMap() const;
        static ClassMapPtr Create(ECDb const& ecdb, ECN::ECRelationshipClassCR ecRelClass, MapStrategyExtendedInfo const& mapStrategy, bool setIsDirty) { return new RelationshipClassEndTableMap(ecdb, ecRelClass, mapStrategy, setIsDirty); }
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
        RelationshipClassLinkTableMap(ECDb const&, ECN::ECRelationshipClassCR, MapStrategyExtendedInfo const&, bool setIsDirty);

        virtual ClassMappingStatus _Map(ClassMappingContext&) override;

        ClassMappingStatus CreateConstraintPropMaps(RelationshipMappingInfo const&, bool addSourceECClassIdColumnToTable, ECN::ECClassId defaultSourceECClassid, bool addTargetECClassIdColumnToTable, ECN::ECClassId defaultTargetECClassId);

        void AddIndices(ClassMappingContext&);
        void AddIndex(SchemaImportContext&, RelationshipIndexSpec, bool addUniqueIndex);

        virtual BentleyStatus _Load(ClassMapLoadContext&, DbClassMapLoadContext const&) override;
        DbColumn* ConfigureForeignECClassIdKey(RelationshipMappingInfo const&, ECN::ECRelationshipEnd relationshipEnd);

        static void GenerateIndexColumnList(std::vector<DbColumn const*>&, DbColumn const* col1, DbColumn const* col2, DbColumn const* col3, DbColumn const* col4);

        static Utf8String DetermineConstraintECInstanceIdColumnName(RelationshipMappingInfo::LinkTableMappingInfo const&, ECN::ECRelationshipEnd);
        static Utf8String DetermineConstraintECClassIdColumnName(RelationshipMappingInfo::LinkTableMappingInfo const&, ECN::ECRelationshipEnd);

    public:
        ~RelationshipClassLinkTableMap() {}
        static ClassMapPtr Create(ECDb const& ecdb, ECN::ECRelationshipClassCR relClass, MapStrategyExtendedInfo const& mapStrategy, bool setIsDirty) { return new RelationshipClassLinkTableMap(ecdb, relClass, mapStrategy, setIsDirty); }

        virtual ReferentialIntegrityMethod _GetDataIntegrityEnforcementMethod() const override;
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
