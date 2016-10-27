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
        bmap<Utf8CP, RefCountedPtr<PrimitivePropertyMap>, CompareIUtf8Ascii> m_vmapsPerTable;
        std::vector<PrimitivePropertyMap const*> m_vmaps;
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
        SystemPropertyMap(Kind, ClassMap const&, ECN::PrimitiveECPropertyCR, std::vector<RefCountedPtr<PrimitivePropertyMap>> const&);
        SystemPropertyMap(Kind kind, PropertyMap const& parentPropertyMap, ECN::ECPropertyCR ecProperty) : PropertyMap(kind, parentPropertyMap, ecProperty) {}

        static BentleyStatus TryCreateVerticalMaps(std::vector<RefCountedPtr<PrimitivePropertyMap>>& propertyMaps, ECSqlSystemProperty systemProperty, ClassMap const& classMap, std::vector<DbColumn const*> const& columns);

    public:
        virtual ~SystemPropertyMap() {}

        PrimitivePropertyMap const* FindVerticalPropertyMap(Utf8CP tableName) const;
        PrimitivePropertyMap const* FindVerticalPropertyMap(DbTable const& table) const { return FindVerticalPropertyMap(table.GetName().c_str()); }
        std::vector<PrimitivePropertyMap const*> const& GetVerticalPropertyMaps() const { return m_vmaps; }

        //! Get list of table to which this property map and its children are mapped to. It is never empty.
        std::vector<DbTable const*> const& GetTables() const { return m_tables; }
        bool IsMappedToSingleTable() const { return GetVerticalPropertyMaps().size() == 1; }
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct ECInstanceIdPropertyMap final : SystemPropertyMap
    {
    private:
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const& dispatcher)  const override { return dispatcher.Visit(*this); }

    protected:
        ECInstanceIdPropertyMap(Kind kind, ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, std::vector<RefCountedPtr<PrimitivePropertyMap>> const& maps)
            : SystemPropertyMap(kind, classMap, ecProperty, maps)
            {}
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
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  dispatcher)  const override { return dispatcher.Visit(*this); }

    protected:
        ECClassIdPropertyMap(Kind kind, ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, std::vector<RefCountedPtr<PrimitivePropertyMap>> const& maps, ECN::ECClassId defaultECClassId)
            : SystemPropertyMap(kind, classMap, ecProperty, maps), m_defaultECClassId(defaultECClassId)
            {}
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
    enum class ConstraintType
        {
        Source, Target
        };
    private:
        ECN::ECClassId m_defaultECClassId;
        ConstraintType m_constraintType;
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  dispatcher)  const override { return dispatcher.Visit(*this); }

    protected:
        ConstraintECClassIdPropertyMap(Kind kind, ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, std::vector<RefCountedPtr<PrimitivePropertyMap>> const& maps, ECN::ECClassId defaultECClassId, ConstraintType constraintType)
            : SystemPropertyMap(kind, classMap, ecProperty, maps), m_defaultECClassId(defaultECClassId), m_constraintType(constraintType)
            {}
    public:
        virtual ~ConstraintECClassIdPropertyMap() {}

        ECN::ECClassId GetDefaultECClassId() const { return m_defaultECClassId; }
        bool IsSource() const { return m_constraintType == ConstraintType::Source; }
        bool IsTarget() const { return m_constraintType == ConstraintType::Target; }
        
        static RefCountedPtr<ConstraintECClassIdPropertyMap> CreateInstance(ClassMap const& classMap, ECN::ECClassId defaultEClassId, ConstraintType constraintType, std::vector<DbColumn const*> const& columns);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct ConstraintECInstanceIdPropertyMap final : SystemPropertyMap
    {
    enum class ConstraintType
        {
        Source, Target
        };
    private:
        ConstraintType m_constraintType;
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  dispatcher)  const override { return dispatcher.Visit(*this); }

    protected:
        ConstraintECInstanceIdPropertyMap(Kind kind, ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, std::vector<RefCountedPtr<PrimitivePropertyMap>> const& maps, ConstraintType constraintType)
            : SystemPropertyMap(kind, classMap, ecProperty, maps)
            {}
    public:
        virtual ~ConstraintECInstanceIdPropertyMap() {}
        bool IsSource() const { return m_constraintType == ConstraintType::Source; }
        bool IsTarget() const { return m_constraintType == ConstraintType::Target; }
        static RefCountedPtr<ConstraintECInstanceIdPropertyMap> CreateInstance(ClassMap const& classMap, ConstraintType constraintType, std::vector<DbColumn const*> const& columns);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
