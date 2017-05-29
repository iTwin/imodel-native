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
        mutable Nullable<Utf8String>  m_idAccessString;
        mutable Nullable<Utf8String>  m_relClassIdAccessString;

        //======================================================================================
        // @bsiclass                                                     Affan.Khan      01/2015
        //===============+===============+===============+===============+===============+======
        struct ColumnFactory final : NonCopyableClass
            {
            private:
                RelationshipClassEndTableMap const& m_relMap;
                RelationshipMappingInfo const& m_relInfo;
                std::map<DbTable const*, std::unique_ptr<EndTableRelationshipColumnResolutionScope>> m_scopes;
                bset<ClassMapCP> m_sharedBlock;

                void Initialize();

            public:
                ColumnFactory(RelationshipClassEndTableMap const&, RelationshipMappingInfo const&);
                ~ColumnFactory(){}

                DbColumn* AllocateForeignKeyECInstanceId(DbTable&, Utf8StringCR colName, int position);
                DbColumn* AllocateForeignKeyRelECClassId(DbTable&, Utf8StringCR colName, PersistenceType, int position);
        
            };

        struct ColumnLists final : NonCopyableClass
            {
            private:
                ColumnFactory m_columnFactory;

                //Following is not created
                std::vector<DbColumn const*> m_secondaryTableECInstanceIdColumns; //secondary table primary key
                std::vector<DbColumn const*> m_secondaryTableECClassIdColumns;  //secondary table classId

                //Following are actually create for each secondary table.
                std::vector<DbColumn const*> m_secondaryTableFkRelECClassIdColumns; //Point to relationship classid associated with following
                std::vector<DbColumn const*> m_secondaryTableFkECInstanceIdColumns; //Point to primary table but created in secondary 

                //Following is not really created but just referenced 
                std::vector<DbColumn const*> m_primaryTableFkECClassIdColumns;     //Point to primary table ECClassId and we just store reference to it to act as FkECClassId.
                
                static void Add(std::vector<DbColumn const*>& list, DbColumn const* column)
                    {
                    BeAssert(column != nullptr);
                    if (std::find(list.begin(), list.end(), column) == list.end())
                        list.push_back(column);
                    }

            public:
                explicit ColumnLists(RelationshipClassEndTableMap const& relMap, RelationshipMappingInfo const& relInfo) : m_columnFactory(relMap, relInfo) {}

                void AddECInstanceIdColumn(DbColumn const& column) { Add(m_secondaryTableECInstanceIdColumns, &column); }
                void AddECClassIdColumn(DbColumn const& column) { Add(m_secondaryTableECClassIdColumns, &column); }
                void AddFkECInstanceIdColumn(DbColumn const& column) { Add(m_secondaryTableFkECInstanceIdColumns, &column); }
                void AddFkRelECClassIdColumn(DbColumn const& column) { Add(m_secondaryTableFkRelECClassIdColumns, &column); }
                void AddFkECClassIdColumn(DbColumn const& column) { Add(m_primaryTableFkECClassIdColumns, &column); }
                std::vector<DbColumn const*> const& GetECInstanceIdColumns() const { return m_secondaryTableECInstanceIdColumns; }
                std::vector<DbColumn const*> const& GetECClassIdColumns() const { return m_secondaryTableECClassIdColumns; }
                std::vector<DbColumn const*> const& GetFkECInstanceIdColumns() const { return m_secondaryTableFkECInstanceIdColumns; }
                std::vector<DbColumn const*> const& GetFkRelECClassIdColumns() const { return m_secondaryTableFkRelECClassIdColumns; }
                std::vector<DbColumn const*> const& GetFkECClassIdColumns() const { return m_primaryTableFkECClassIdColumns; }
                ColumnFactory& GetColumnFactory() { return m_columnFactory; }
            };

        struct ForeignKeyColumnInfo final: NonCopyableClass
            {
            private:
                bool m_canImplyFromNavigationProperty = false;
                Utf8String m_impliedFkColName;
                Utf8String m_impliedRelClassIdColName;
                bool m_appendToEnd = true;
                PropertyMap const* m_propMapBeforeNavProp = nullptr;
                PropertyMap const* m_propMapAfterNavProp = nullptr;

            public:
                ForeignKeyColumnInfo() {}

                void Assign(Utf8StringCR impliedFkColName, Utf8StringCR impliedRelClassIdColName, bool appendToEnd, PropertyMap const* propMapBeforeNavProp, PropertyMap const* propMapAfterNavProp)
                    {
                    m_canImplyFromNavigationProperty = true;
                    m_impliedFkColName.assign(impliedFkColName);
                    m_impliedRelClassIdColName.assign(impliedRelClassIdColName);
                    m_appendToEnd = appendToEnd;
                    m_propMapBeforeNavProp = propMapBeforeNavProp;
                    m_propMapAfterNavProp = propMapAfterNavProp;
                    }

                bool CanImplyFromNavigationProperty() const { return m_canImplyFromNavigationProperty; }
                Utf8StringCR GetImpliedFkColumnName() const { return m_impliedFkColName; }
                Utf8StringCR GetImpliedRelClassIdColumnName() const { return m_impliedRelClassIdColName; }
                bool AppendToEnd() const { return m_appendToEnd; }
                PropertyMap const* GetPropertyMapBeforeNavProp() const { return m_propMapBeforeNavProp; }
                PropertyMap const* GetPropertyMapAfterNavProp() const { return m_propMapAfterNavProp; }
            };

        RelationshipClassEndTableMap(ECDb const&, ECN::ECClassCR, MapStrategyExtendedInfo const&);
        RelationshipClassEndTableMap(ECDb const&, ECN::ECClassCR, MapStrategyExtendedInfo const&, UpdatableViewInfo const&);

        void AddIndexToRelationshipEnd(RelationshipMappingInfo const&);

        ClassMappingStatus _Map(ClassMappingContext&) override;
        DbColumn* CreateRelECClassIdColumn(ColumnFactory&, DbTable&, ForeignKeyColumnInfo const&, DbColumn const& fkCol) const;

        BentleyStatus DetermineKeyAndConstraintColumns(ColumnLists&, RelationshipMappingInfo const&);
        BentleyStatus DetermineFkColumns(ColumnLists&, ForeignKeyColumnInfo&, RelationshipMappingInfo const&);
        BentleyStatus MapSubClass(RelationshipMappingInfo const&);

        BentleyStatus TryGetForeignKeyColumnInfoFromNavigationProperty(ForeignKeyColumnInfo&, ECN::ECRelationshipConstraintCR, ECN::ECRelationshipClassCR, ECN::ECRelationshipEnd) const;
        BentleyStatus TryDetermineForeignKeyColumnPosition(int& position, DbTable const&, ForeignKeyColumnInfo const&) const;

        BentleyStatus _Load(ClassMapLoadContext&, DbClassMapLoadContext const&) override;

        BentleyStatus ValidateForeignKeyColumn(DbColumn const& fkColumn, bool cardinalityImpliesNotNullOnFkCol, DbColumn::Kind) const;
    public:
        ~RelationshipClassEndTableMap() {}

        //!Gets the end in which the ForeignKey is persisted
        ECN::ECRelationshipEnd GetForeignEnd() const;
        //!Gets the end the ForeignKey end references
        ECN::ECRelationshipEnd GetReferencedEnd() const;
        Utf8CP GetAccessStringForId() const
            {
            if (m_idAccessString.IsNull())
                m_idAccessString.ValueR().assign(GetClass().GetFullName()).append("." ECDBSYS_PROP_NavPropId);

            return m_idAccessString.Value().c_str();
            }
        Utf8CP GetAccessStringForRelClassId() const
            {
            if (m_relClassIdAccessString.IsNull())
                m_relClassIdAccessString.ValueR().assign(GetClass().GetFullName()).append("." ECDBSYS_PROP_NavPropRelECClassId);

            return m_relClassIdAccessString.Value().c_str();
            }

        ConstraintECInstanceIdPropertyMap const* GetReferencedEndECInstanceIdPropMap() const;
        Utf8String BuildQualifiedAccessString(Utf8StringCR accessString) const  
            {
            Utf8String temp = GetRelationshipClass().GetFullName();
            temp.append(".").append(accessString);
            return temp;
            }
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
