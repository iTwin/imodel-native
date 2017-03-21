/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SystemPropertyMap.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "PropertyMap.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
//=======================================================================================
//! Property map for system properties. They are mapped to multiple tables (one column each)
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct SystemPropertyMap : PropertyMap
    {
    public:
        struct PerTablePrimitivePropertyMap final: SingleColumnDataPropertyMap
            {
        private:
            PerTablePrimitivePropertyMap(PropertyMap const& parentPropertyMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column)
                : SingleColumnDataPropertyMap(Type::SystemPerTablePrimitive, parentPropertyMap, ecProperty, column, false)
                {}

            DbColumn::Type _GetColumnDataType() const override { return DbColumn::Type::Integer; }

        public:
            ~PerTablePrimitivePropertyMap() {}

            static RefCountedPtr<PerTablePrimitivePropertyMap> CreateInstance(PropertyMap const& parentPropertyMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column);
            };

    private:
        bmap<Utf8CP, RefCountedPtr<PerTablePrimitivePropertyMap>, CompareIUtf8Ascii> m_dataPropMaps;
        std::vector<PerTablePrimitivePropertyMap const*> m_dataPropMapList;
        std::vector<DbTable const*> m_tables;

        bool _IsMappedToTable(DbTable const& table) const override
            {
            for (DbTable const* t : m_tables)
                if (t == &table)
                    return true;

            return false;
            }

        BentleyStatus _AcceptVisitor(IPropertyMapVisitor const& visitor)  const override { return visitor.Visit(*this); }

    protected:
        SystemPropertyMap(Type, ClassMap const&, ECN::PrimitiveECPropertyCR);
        SystemPropertyMap(Type kind, PropertyMap const& parentPropertyMap, ECN::ECPropertyCR ecProperty, Utf8StringCR accessString) : PropertyMap(kind, parentPropertyMap, ecProperty, accessString) {}

        BentleyStatus Init(std::vector<DbColumn const*> const&);

    public:
        virtual ~SystemPropertyMap() {}

        PerTablePrimitivePropertyMap const* FindDataPropertyMap(Utf8CP tableName) const;
        PerTablePrimitivePropertyMap const* FindDataPropertyMap(DbTable const& table) const { return FindDataPropertyMap(table.GetName().c_str()); }
        std::vector<PerTablePrimitivePropertyMap const*> const& GetDataPropertyMaps() const { return m_dataPropMapList; }
        //! Get list of table to which this property map and its children are mapped to. It is never empty.
        std::vector<DbTable const*> const& GetTables() const { return m_tables; }
        bool IsMappedToSingleTable() const { return GetDataPropertyMaps().size() == 1; }
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct ECInstanceIdPropertyMap final : SystemPropertyMap
    {
    private:
        ECInstanceIdPropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty)
            : SystemPropertyMap(Type::ECInstanceId, classMap, ecProperty)
            {}

    public:
        ~ECInstanceIdPropertyMap() {}
        static RefCountedPtr<ECInstanceIdPropertyMap> CreateInstance(ClassMap const& classMap, std::vector<DbColumn const*> const& columns);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct ECClassIdPropertyMap final : SystemPropertyMap
    {
    private:
        ECN::ECClassId m_defaultECClassId;

        ECClassIdPropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, ECN::ECClassId defaultECClassId)
            : SystemPropertyMap(Type::ECClassId, classMap, ecProperty), m_defaultECClassId(defaultECClassId)
            {}

    public:
        ~ECClassIdPropertyMap() {}
        bool IsVirtual(DbTable const& table) const;
        ECN::ECClassId GetDefaultECClassId() const { return m_defaultECClassId; }
        static RefCountedPtr<ECClassIdPropertyMap> CreateInstance(ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct ConstraintECClassIdPropertyMap final : SystemPropertyMap
    {
    private:
        ECN::ECClassId m_defaultECClassId;
        ECN::ECRelationshipEnd m_end;

        ConstraintECClassIdPropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, ECN::ECClassId defaultECClassId, ECN::ECRelationshipEnd constraintEnd)
            : SystemPropertyMap(Type::ConstraintECClassId, classMap, ecProperty), m_defaultECClassId(defaultECClassId), m_end(constraintEnd)
            {}

    public:
        ~ConstraintECClassIdPropertyMap() {}

        bool IsVirtual(DbTable const& table) const;
        ECN::ECClassId GetDefaultECClassId() const { return m_defaultECClassId; }
        ECN::ECRelationshipEnd GetEnd() const { return m_end; }
     
        static RefCountedPtr<ConstraintECClassIdPropertyMap> CreateInstance(ClassMap const& classMap, ECN::ECClassId defaultEClassId, ECN::ECRelationshipEnd constraintType, std::vector<DbColumn const*> const& columns);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct ConstraintECInstanceIdPropertyMap final : SystemPropertyMap
    {
    private:
        ECN::ECRelationshipEnd m_end;

        ConstraintECInstanceIdPropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, ECN::ECRelationshipEnd constraintEnd)
            : SystemPropertyMap(Type::ConstraintECInstanceId, classMap, ecProperty), m_end(constraintEnd)
            {}

    public:
        ~ConstraintECInstanceIdPropertyMap() {}
        ECN::ECRelationshipEnd GetEnd() const { return m_end; }

        static RefCountedPtr<ConstraintECInstanceIdPropertyMap> CreateInstance(ClassMap const& classMap, ECN::ECRelationshipEnd constraintType, std::vector<DbColumn const*> const& columns);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
