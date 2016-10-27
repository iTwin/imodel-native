/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/SystemPropertyMap.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
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
    private:
        bmap<Utf8CP, RefCountedPtr<PrimitivePropertyMap>, CompareIUtf8Ascii> m_dataPropMaps;
        std::vector<PrimitivePropertyMap const*> m_dataPropMapList;
        std::vector<DbTable const*> m_tables;

        virtual BentleyStatus _Validate() const override;
        virtual bool _IsMappedToTable(DbTable const& table) const override
            {
            for (DbTable const* t : m_tables)
                if (t == &table)
                    return true;

            return false;
            }

    protected:
        SystemPropertyMap(Kind, ClassMap const&, ECN::PrimitiveECPropertyCR);
        SystemPropertyMap(Kind kind, PropertyMap const& parentPropertyMap, ECN::ECPropertyCR ecProperty) : PropertyMap(kind, parentPropertyMap, ecProperty) {}

        BentleyStatus Init(std::vector<DbColumn const*> const& columns);

    public:
        virtual ~SystemPropertyMap() {}

        PrimitivePropertyMap const* FindDataPropertyMap(Utf8CP tableName) const;
        PrimitivePropertyMap const* FindDataPropertyMap(DbTable const& table) const { return FindDataPropertyMap(table.GetName().c_str()); }
        std::vector<PrimitivePropertyMap const*> const& GetDataPropertyMaps() const { return m_dataPropMapList; }

        //! Get list of table to which this property map and its children are mapped to. It is never empty.
        std::vector<DbTable const*> const& GetTables() const { return m_tables; }
        bool IsMappedToSingleTable() const { return GetDataPropertyMaps().size() == 1; }
        BentleyStatus Add(DbColumn const& column)
            {

            }
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct ECInstanceIdPropertyMap final : SystemPropertyMap
    {
    private:
        ECInstanceIdPropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty)
            : SystemPropertyMap(Kind::ECInstanceId, classMap, ecProperty)
            {}

        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const& visitor)  const override { return visitor.Visit(*this); }

    public:
        virtual ~ECInstanceIdPropertyMap() {}
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
            : SystemPropertyMap(Kind::ECClassId, classMap, ecProperty), m_defaultECClassId(defaultECClassId)
            {}

        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  visitor)  const override { return visitor.Visit(*this); }

    public:
        virtual ~ECClassIdPropertyMap() {}
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
            : SystemPropertyMap(Kind::ConstraintECClassId, classMap, ecProperty), m_defaultECClassId(defaultECClassId), m_end(constraintEnd)
            {}

        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  visitor)  const override { return visitor.Visit(*this); }

    public:
        virtual ~ConstraintECClassIdPropertyMap() {}
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
            : SystemPropertyMap(Kind::ConstraintECInstanceId, classMap, ecProperty), m_end(constraintEnd)
            {}

        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  visitor)  const override { return visitor.Visit(*this); }

    public:
        virtual ~ConstraintECInstanceIdPropertyMap() {}
        ECN::ECRelationshipEnd GetEnd() const { return m_end; }

        static RefCountedPtr<ConstraintECInstanceIdPropertyMap> CreateInstance(ClassMap const& classMap, ECN::ECRelationshipEnd constraintType, std::vector<DbColumn const*> const& columns);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
