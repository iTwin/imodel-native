/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/RelationshipClassMap.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
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
        ECN::ECRelationshipConstraintCR m_constraint;
        ConstraintECInstanceIdPropertyMap const* m_ecInstanceIdPropMap;
        ConstraintECClassIdPropertyMap const* m_ecClassIdPropMap;

    public:
        explicit RelationshipConstraintMap(ECN::ECRelationshipConstraintCR constraint)
            :  m_constraint(constraint), m_ecInstanceIdPropMap(nullptr), m_ecClassIdPropMap(nullptr)
            {}
        ConstraintECInstanceIdPropertyMap const* GetECInstanceIdPropMap() const { return m_ecInstanceIdPropMap; }
        void SetECInstanceIdPropMap(ConstraintECInstanceIdPropertyMap const* ecinstanceIdPropMap) { m_ecInstanceIdPropMap = ecinstanceIdPropMap; }
        ConstraintECClassIdPropertyMap const* GetECClassIdPropMap() const { return m_ecClassIdPropMap; }
        void SetECClassIdPropMap(ConstraintECClassIdPropertyMap const* ecClassIdPropMap) { m_ecClassIdPropMap = ecClassIdPropMap; }
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

        RelationshipClassMap(ECDb const&, Type, ECN::ECClassCR, MapStrategyExtendedInfo const&);
        RelationshipClassMap(ECDb const&, Type, ECN::ECClassCR, MapStrategyExtendedInfo const&, UpdatableViewInfo const&);
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
        static bool ConstraintIncludesAnyClass(ECN::ECRelationshipConstraintClassList const&);
    };

typedef RelationshipClassMap const& RelationshipClassMapCR;

/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      06/2012
+===============+===============+===============+===============+===============+======*/
struct RelationshipClassEndTableMap final : RelationshipClassMap
    {
    friend struct ClassMapFactory;

    private:
        static Utf8CP DEFAULT_FK_COL_PREFIX;
        static Utf8CP RELECCLASSID_COLNAME_TOKEN;
        bool m_mapping;
        struct ForeignKeyColumnInfo final: NonCopyableClass
            {
            private:
                bool m_canImplyFromNavigationProperty = false;
                Utf8String m_impliedFkColName;
                Utf8String m_impliedRelClassIdColName;

            public:
                ForeignKeyColumnInfo() {}

                void Assign(Utf8StringCR impliedFkColName, Utf8StringCR impliedRelClassIdColName)
                    {
                    m_canImplyFromNavigationProperty = true;
                    m_impliedFkColName.assign(impliedFkColName);
                    m_impliedRelClassIdColName.assign(impliedRelClassIdColName);
                    }

                bool CanImplyFromNavigationProperty() const { return m_canImplyFromNavigationProperty; }
                Utf8StringCR GetImpliedFkColumnName() const { return m_impliedFkColName; }
                Utf8StringCR GetImpliedRelClassIdColumnName() const { return m_impliedRelClassIdColName; }
            };
        RelationshipClassEndTableMap(ECDb const&, ECN::ECClassCR, MapStrategyExtendedInfo const&);
        RelationshipClassEndTableMap(ECDb const&, ECN::ECClassCR, MapStrategyExtendedInfo const&, UpdatableViewInfo const&);
        void AddIndexToRelationshipEnd(RelationshipMappingInfo const&);
        ClassMappingStatus _Map(ClassMappingContext&) override;
        BentleyStatus _Load(ClassMapLoadContext&, DbClassMapLoadContext const&) override;
        BentleyStatus ValidateForeignKeyColumn(DbColumn const& fkColumn, bool cardinalityImpliesNotNullOnFkCol, DbColumn::Kind) const;
        void GetForeignKeyColumnInfo(ForeignKeyColumnInfo& fkColInfo, NavigationPropertyMap const& navProp) const;
        DbColumn* CreateForeignColumn(RelationshipMappingInfo const& classMappingInfo, DbTable&  fkTable, NavigationPropertyMap const& navPropMap, ForeignKeyColumnInfo &fkColInfo);
        DbColumn* CreateReferencedClassIdColumn(DbTable& fkTable) const;
        DbColumn* CreateRelECClassIdColumn(DbTable& fkTable, ForeignKeyColumnInfo const& fkColInfo, DbColumn const& fkCol, NavigationPropertyMap const& navPropMap) const;
        ClassMappingStatus CreateForiegnKeyConstraint(DbTable const& referencedTable, RelationshipMappingInfo const& classMappingInfo);
        ClassMappingStatus UpdatePersistedEndForChild(SchemaImportContext& ctx, NavigationPropertyMap& navPropMap);
        RelationshipClassEndTableMap* GetRootRelationshipMap(SchemaImportContext& ctx);
        ClassMappingStatus FinishMappingForChild(SchemaImportContext& ctx);
    public:

        ~RelationshipClassEndTableMap() {}
        ClassMappingStatus UpdatePersistedEnd(SchemaImportContext& ctx, NavigationPropertyMap& navPropMap);
        //!Gets the end in which the ForeignKey is persisted
        ECN::ECRelationshipEnd GetForeignEnd() const;
        //!Gets the end the ForeignKey end references
        ECN::ECRelationshipEnd GetReferencedEnd() const;
        ClassMappingStatus FinishMapping(SchemaImportContext& ctx);
        ConstraintECInstanceIdPropertyMap const* GetReferencedEndECInstanceIdPropMap() const;
        //WIP: This code must go elsewhere. It is only used by the column factory
        Utf8String GetAccessStringForId() const { return Utf8String(GetClass().GetFullName()) + Utf8String("." ECDBSYS_PROP_NavPropId); }
        Utf8String GetAccessStringForRelClassId() const { return Utf8String(GetClass().GetFullName()) + Utf8String("." ECDBSYS_PROP_NavPropRelECClassId); }
    };

/*==========================================================================
* @bsiclass                                                 Ramanujam.Raman      06/2012
+===============+===============+===============+===============+===============+======*/
struct RelationshipClassLinkTableMap final : RelationshipClassMap
    {
    friend struct ClassMapFactory;

    private:
        enum class RelationshipIndexSpec
            {
            Source,
            Target,
            SourceAndTarget
            };

    private:
        RelationshipClassLinkTableMap(ECDb const&, ECN::ECClassCR, MapStrategyExtendedInfo const&);
        RelationshipClassLinkTableMap(ECDb const&, ECN::ECClassCR, MapStrategyExtendedInfo const&, UpdatableViewInfo const&);

        ClassMappingStatus _Map(ClassMappingContext&) override;
        ClassMappingStatus MapSubClass(ClassMappingContext&, RelationshipMappingInfo const&);

        ClassMappingStatus CreateConstraintPropMaps(ClassMappingContext&, RelationshipMappingInfo const&, bool addSourceECClassIdColumnToTable, bool addTargetECClassIdColumnToTable);

        void AddIndices(ClassMappingContext&, bool allowDuplicateRelationship);
        void AddIndex(SchemaImportContext&, RelationshipIndexSpec, bool addUniqueIndex);
        DbColumn* CreateConstraintColumn(Utf8CP columnName, DbColumn::Kind, PersistenceType);
        void DetermineConstraintClassIdColumnHandling(bool& addConstraintClassIdColumnNeeded, ECN::ECRelationshipConstraintCR) const;


        BentleyStatus _Load(ClassMapLoadContext&, DbClassMapLoadContext const&) override;
        DbColumn* ConfigureForeignECClassIdKey(ClassMappingContext&, RelationshipMappingInfo const&, ECN::ECRelationshipEnd);

        static void GenerateIndexColumnList(std::vector<DbColumn const*>&, DbColumn const* col1, DbColumn const* col2, DbColumn const* col3, DbColumn const* col4);
        static Utf8String DetermineConstraintECInstanceIdColumnName(RelationshipMappingInfo::LinkTableMappingInfo const&, ECN::ECRelationshipEnd);
        static Utf8String DetermineConstraintECClassIdColumnName(RelationshipMappingInfo::LinkTableMappingInfo const&, ECN::ECRelationshipEnd);
        static bool DetermineAllowDuplicateRelationshipsFlagFromRoot(ECN::ECRelationshipClassCR baseRelClass);

    public:
        ~RelationshipClassLinkTableMap() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
