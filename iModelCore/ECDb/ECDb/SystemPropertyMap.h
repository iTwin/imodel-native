/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
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
        struct PerTableIdPropertyMap : SingleColumnDataPropertyMap
            {
        private:
            DbColumn::Type _GetColumnDataType() const override { return DbColumn::Type::Integer; }

            PerTableIdPropertyMap(SystemPropertyMap const& parentPropertyMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column)
                : PerTableIdPropertyMap(Type::SystemPerTableId, parentPropertyMap, ecProperty, column)
                {}

        protected:
            PerTableIdPropertyMap(Type type, SystemPropertyMap const& parentPropertyMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column)
                : SingleColumnDataPropertyMap(type, parentPropertyMap, ecProperty, column, false)
                {}

        public:
            virtual ~PerTableIdPropertyMap() {}

            static RefCountedPtr<PerTableIdPropertyMap> CreateInstance(SystemPropertyMap const& parentPropertyMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column);
            };

        struct PerTableClassIdPropertyMap final : PerTableIdPropertyMap
            {
            private:
                ECN::ECClassId m_defaultClassId;

                PerTableClassIdPropertyMap(SystemPropertyMap const& parentPropertyMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column, ECN::ECClassId defaultClassId)
                    : PerTableIdPropertyMap(Type::SystemPerTableClassId, parentPropertyMap, ecProperty, column), m_defaultClassId(defaultClassId) {}
            public:
                ~PerTableClassIdPropertyMap() {}

                static RefCountedPtr<PerTableClassIdPropertyMap> CreateInstance(SystemPropertyMap const& parentPropertyMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column, ECN::ECClassId defaultClassId);

                //!@returns invalid id if the property map doesn't have a default classid
                ECN::ECClassId GetDefaultECClassId() const { return m_defaultClassId; }
            };

    private:
        bmap<Utf8CP, RefCountedPtr<PerTableIdPropertyMap>, CompareIUtf8Ascii> m_dataPropMaps;
        std::vector<PerTableIdPropertyMap const*> m_dataPropMapList;
        std::vector<DbTable const*> m_tables;

        bool _IsMappedToTable(DbTable const& table) const override;
        BentleyStatus _AcceptVisitor(IPropertyMapVisitor const& visitor)  const override { return visitor.Visit(*this); }
        virtual RefCountedPtr<PerTableIdPropertyMap> _CreatePerTablePropertyMap(SystemPropertyMap&, ECN::PrimitiveECPropertyCR, DbColumn const&) const = 0;

    protected:
        SystemPropertyMap(Type, ClassMap const&, ECN::PrimitiveECPropertyCR);
        SystemPropertyMap(Type kind, PropertyMap const& parentPropertyMap, ECN::ECPropertyCR ecProperty, Utf8StringCR accessString) : PropertyMap(kind, parentPropertyMap, ecProperty, accessString) {}

        BentleyStatus Init(std::vector<DbColumn const*> const&, bool appendMode=false);

    public:
        virtual ~SystemPropertyMap() {}

        PerTableIdPropertyMap const* FindDataPropertyMap(Utf8CP tableName) const;
        PerTableIdPropertyMap const* FindDataPropertyMap(DbTable const& table) const { return FindDataPropertyMap(table.GetName().c_str()); }
        PerTableIdPropertyMap const* FindDataPropertyMap(ClassMap const&) const;
        std::vector<PerTableIdPropertyMap const*> const& GetDataPropertyMaps() const { return m_dataPropMapList; }
        //! Get list of table to which this property map and its children are mapped to. It is never empty.
        std::vector<DbTable const*> const& GetTables() const { return m_tables; }
        bool IsMappedToSingleTable() const { return GetDataPropertyMaps().size() == 1; }
        static BentleyStatus AppendSystemColumnFromNewlyAddedDataTable(SystemPropertyMap& propertyMap, DbColumn const& column);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct ECInstanceIdPropertyMap final : SystemPropertyMap
    {
    private:
        ECInstanceIdPropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty) : SystemPropertyMap(Type::ECInstanceId, classMap, ecProperty) {}

        RefCountedPtr<PerTableIdPropertyMap> _CreatePerTablePropertyMap(SystemPropertyMap& parentPropMap, ECN::PrimitiveECPropertyCR prop, DbColumn const& col) const override { return PerTableIdPropertyMap::CreateInstance(parentPropMap, prop, col); }

    public:
        ~ECInstanceIdPropertyMap() {}
        static RefCountedPtr<ECInstanceIdPropertyMap> CreateInstance(ClassMap const&, std::vector<DbColumn const*> const&);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct ECClassIdPropertyMap final : SystemPropertyMap
    {
    private:
        ECClassIdPropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty) : SystemPropertyMap(Type::ECClassId, classMap, ecProperty) {}

        RefCountedPtr<PerTableIdPropertyMap> _CreatePerTablePropertyMap(SystemPropertyMap& parentPropMap, ECN::PrimitiveECPropertyCR, DbColumn const&) const override;

    public:
        ~ECClassIdPropertyMap() {}
        static RefCountedPtr<ECClassIdPropertyMap> CreateInstance(ClassMap const&, std::vector<DbColumn const*> const&);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct ConstraintECClassIdPropertyMap final : SystemPropertyMap
    {
    private:
        ECN::ECRelationshipEnd m_end;

        ConstraintECClassIdPropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, ECN::ECRelationshipEnd constraintEnd)
            : SystemPropertyMap(Type::ConstraintECClassId, classMap, ecProperty), m_end(constraintEnd)
            {}

        RefCountedPtr<PerTableIdPropertyMap> _CreatePerTablePropertyMap(SystemPropertyMap& parentPropMap, ECN::PrimitiveECPropertyCR, DbColumn const&) const override;

    public:
        ~ConstraintECClassIdPropertyMap() {}

        ECN::ECRelationshipEnd GetEnd() const { return m_end; }
     
        static RefCountedPtr<ConstraintECClassIdPropertyMap> CreateInstance(ClassMap const&, ECN::ECRelationshipEnd, std::vector<DbColumn const*> const&);
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

        RefCountedPtr<PerTableIdPropertyMap> _CreatePerTablePropertyMap(SystemPropertyMap& parentPropMap, ECN::PrimitiveECPropertyCR prop, DbColumn const& col) const override { return PerTableIdPropertyMap::CreateInstance(parentPropMap, prop, col); }

    public:
        ~ConstraintECInstanceIdPropertyMap() {}
        ECN::ECRelationshipEnd GetEnd() const { return m_end; }

        static RefCountedPtr<ConstraintECInstanceIdPropertyMap> CreateInstance(ClassMap const&, ECN::ECRelationshipEnd, std::vector<DbColumn const*> const&);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
