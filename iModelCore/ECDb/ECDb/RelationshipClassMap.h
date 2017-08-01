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
#include "SystemPropertyMap.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
// @bsiclass                                                 Krischan.Eberle      07/2014
//+===============+===============+===============+===============+===============+======
struct RelationshipConstraintMap : NonCopyableClass
    {
    private:
        ECN::ECRelationshipConstraintCR m_constraint;
        ConstraintECInstanceIdPropertyMap const* m_ecInstanceIdPropMap = nullptr;
        ConstraintECClassIdPropertyMap const* m_ecClassIdPropMap = nullptr;

    public:
        explicit RelationshipConstraintMap(ECN::ECRelationshipConstraintCR constraint) :  m_constraint(constraint) {}
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
    protected:
        static Utf8CP const DEFAULT_SOURCEECINSTANCEID_COLUMNNAME;
        static Utf8CP const DEFAULT_SOURCEECCLASSID_COLUMNNAME;
        static Utf8CP const DEFAULT_TARGETECINSTANCEID_COLUMNNAME;
        static Utf8CP const DEFAULT_TARGETECCLASSID_COLUMNNAME;

        RelationshipConstraintMap m_sourceConstraintMap;
        RelationshipConstraintMap m_targetConstraintMap;

        RelationshipClassMap(ECDb const&, Type, ECN::ECClassCR, MapStrategyExtendedInfo const&);
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
    };

typedef RelationshipClassMap const& RelationshipClassMapCR;
/*=================================================================================**//**
* @bsiclass                                                 Ramanujam.Raman      06/2012
+===============+===============+===============+===============+===============+======*/
struct RelationshipClassEndTableMap final : RelationshipClassMap
    {
    friend struct ClassMapFactory;
    private:
        RelationshipClassEndTableMap(ECDb const& ecdb, ECN::ECClassCR relClass, MapStrategyExtendedInfo const& mapStrategy) : RelationshipClassMap(ecdb, Type::RelationshipEndTable, relClass, mapStrategy) {}
        ClassMappingStatus _Map(ClassMappingContext&) override;
        BentleyStatus _Load(ClassMapLoadContext&, DbClassMapLoadContext const&) override;
        RelationshipClassEndTableMap const* GetBaseClassMap(SchemaImportContext* ctx = nullptr) const;
        ClassMappingStatus MapSubClass(RelationshipClassEndTableMap const& baseClassMap);

    public:
        ~RelationshipClassEndTableMap() {}
        ECN::ECRelationshipEnd GetForeignEnd() const;
        ECN::ECRelationshipEnd GetReferencedEnd() const;
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
        ClassMappingStatus _Map(ClassMappingContext&) override;
        ClassMappingStatus MapSubClass(ClassMappingContext&);
        ClassMappingStatus CreateConstraintPropMaps(SchemaImportContext&, LinkTableRelationshipMapCustomAttribute const&, bool addSourceECClassIdColumnToTable, bool addTargetECClassIdColumnToTable);
        void AddIndices(SchemaImportContext&, bool allowDuplicateRelationship);
        void AddIndex(SchemaImportContext&, RelationshipIndexSpec, bool addUniqueIndex);
        DbColumn* CreateConstraintColumn(Utf8CP columnName, PersistenceType);
        void DetermineConstraintClassIdColumnHandling(bool& addConstraintClassIdColumnNeeded, ECN::ECRelationshipConstraintCR) const;
        BentleyStatus _Load(ClassMapLoadContext&, DbClassMapLoadContext const&) override;
        DbColumn* ConfigureForeignECClassIdKey(SchemaImportContext&, LinkTableRelationshipMapCustomAttribute const&, ECN::ECRelationshipEnd);
        static void GenerateIndexColumnList(std::vector<DbColumn const*>&, DbColumn const* col1, DbColumn const* col2, DbColumn const* col3, DbColumn const* col4);
        
        static Utf8String DetermineConstraintECInstanceIdColumnName(LinkTableRelationshipMapCustomAttribute const&, ECN::ECRelationshipEnd);
        static Utf8String DetermineConstraintECClassIdColumnName(LinkTableRelationshipMapCustomAttribute const&, ECN::ECRelationshipEnd);
        //AllowDuplicateRelationships flag is inherited from root rel class to actual rel class
        static bool DetermineAllowDuplicateRelationshipsFlagFromRoot(ECN::ECRelationshipClassCR baseRelClass);

        static bool GetAllowDuplicateRelationshipsFlag(Nullable<bool> const& allowDuplicateRelationshipFlag) { return allowDuplicateRelationshipFlag.IsNull() ? false : allowDuplicateRelationshipFlag.Value(); }

    public:
        ~RelationshipClassLinkTableMap() {}
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
