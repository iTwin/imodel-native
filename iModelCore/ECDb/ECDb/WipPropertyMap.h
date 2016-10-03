/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/WipPropertyMap.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "ClassMappingInfo.h"
#include "DbSchema.h"
#include "ClassMap.h"
#include "ECDbSystemSchemaHelper.h"
#include "ECSql/NativeSqlBuilder.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
struct WipPropertyMap;
struct WipVerticalPropertyMap;
struct WipHorizontalPropertyMap;
struct WipCompoundPropertyMap;
struct WipColumnVerticalPropertyMap;
struct WipColumnHorizontalPropertyMap;

struct WipPrimitivePropertyMap; // MapToColumn , StorageType, ColumnType, IsShared
struct WipPrimitiveArrayPropertyMap;
struct WipStructPropertyMap;
struct WipStructArrayPropertyMap;
struct WipNavigationPropertyMap;
struct WipPoint2dPropertyMap;
struct WipPoint3dPropertyMap;
struct WipPropertyMapContainer;
#define EC_ACCESSSTRING_DELIMITER "."

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipPropertyMapContainer final : NonCopyableClass
    {
    private:
        ClassMap const& m_classMap;
        std::vector<WipPropertyMap const*> m_directDecendentList; //! contain direct decedents in order.
        std::map<Utf8CP, RefCountedPtr<WipPropertyMap>, CompareIUtf8Ascii> m_map; //! contain all propertymap owned by the container
        bool m_readonly;
    public:
        WipPropertyMapContainer(ClassMap const& classMap)
            :m_classMap(classMap), m_readonly(false)
            {}
        ~WipPropertyMapContainer() {}

        ClassMap const& GetClassMap() const { return m_classMap; }
        ECN::ECClass const& GetClass() const;
        ECDbCR GetECDb() const;
        BentleyStatus Insert(RefCountedPtr<WipPropertyMap> propertyMap, size_t position = UINT_MAX);
        WipPropertyMap const* Find(Utf8CP accessString, bool recusive = true) const;
        void MakeReadonly() { m_readonly = true; }
        bool IsReadonly() const { return m_readonly; }
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Abstract baseclass of all property map. Do not pollute this class
//+===============+===============+===============+===============+===============+======
struct WipPropertyMap : RefCountedBase, NonCopyableClass
    {
    private:
        ECN::ECPropertyCR m_ecProperty;
        Utf8String m_propertyAccessString;
        WipPropertyMap const* m_parentPropertMap;
        WipPropertyMapContainer const& m_container;
        virtual BentleyStatus _Validate() const = 0;
        bool m_isInEditMode;
    protected:
        WipPropertyMap(WipPropertyMapContainer const& container, ECN::ECPropertyCR ecProperty);
        WipPropertyMap(ECN::ECPropertyCR ecProperty, WipPropertyMap const& parentPropertyMap);
        virtual ~WipPropertyMap() {}
    public:
        //! A property is injected if it does not ECClass but added by ECDb
        bool IsInjected() const;
        bool InEditMode() const { return m_isInEditMode; }
        void FinishEditing() { BeAssert(m_isInEditMode);  m_isInEditMode = false; }
        Utf8String GetName() const { return GetProperty().GetName(); }
        ClassMap const& GetClassMap() const { return m_container.GetClassMap(); }
        ECN::ECPropertyCR GetProperty() const { return m_ecProperty; }
        Utf8StringCR GetAccessString() const { return m_propertyAccessString; }
        WipPropertyMap const* GetParent() const { return m_parentPropertMap; }
        WipPropertyMapContainer const& GetContainer() const { return m_container; }
        BentleyStatus Validate() const { BeAssert(InEditMode() == false); if (InEditMode()) return ERROR;  return _Validate(); }
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Abstract baseclass of all property map that are mapped vertically.
// They must have a table and all the hierarchy of propertymap must hold same table.
//+===============+===============+===============+===============+===============+======
struct WipVerticalPropertyMap : WipPropertyMap
    {
    private:
        virtual DbTable const& _GetTable() const = 0;
    public:
        WipVerticalPropertyMap(WipPropertyMapContainer const& container, ECN::ECPropertyCR ecProperty)
            : WipPropertyMap(container, ecProperty)
            {}
        WipVerticalPropertyMap(ECN::ECPropertyCR ecProperty, WipPropertyMap const& parentPropertyMap)
            : WipPropertyMap(ecProperty, parentPropertyMap)
            {}
        ~WipVerticalPropertyMap() {}
        DbTable const& GetTable() const { return _GetTable(); }
        RefCountedPtr<WipVerticalPropertyMap> CreateCopy(WipPropertyMapContainer const& newContainer) const;
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Abstract baseclass of all property map that are mapped vertically into multiple tables.
// The main class of properties it represent are system properties.
//+===============+===============+===============+===============+===============+======
struct WipHorizontalPropertyMap : WipPropertyMap
    {
    protected:
        WipHorizontalPropertyMap(WipPropertyMapContainer const& container, ECN::ECPropertyCR ecProperty)
            : WipPropertyMap(container, ecProperty)
            {}
        WipHorizontalPropertyMap(ECN::ECPropertyCR ecProperty, WipPropertyMap const& parentPropertyMap)
            : WipPropertyMap(ecProperty, parentPropertyMap)
            {}
        virtual  ~WipHorizontalPropertyMap() {}
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Abstract baseclass of property map that has child property map. Only vertical propertymaps
// is allowed to contain child property map that must be in same table
//+===============+===============+===============+===============+===============+======
struct WipCompoundPropertyMap : WipVerticalPropertyMap
    {
    typedef std::vector<WipVerticalPropertyMap const*>::const_iterator const_iterator;
    private:
        struct Collection
            {
            private:
                std::vector<WipVerticalPropertyMap const*> m_list;
                std::map<Utf8CP, RefCountedPtr<WipVerticalPropertyMap>, CompareIUtf8Ascii> m_map;
            public:
                std::vector<WipVerticalPropertyMap const*> const& GetList() const { return m_list; }
                WipVerticalPropertyMap const* Find(Utf8CP accessString) const;
                WipVerticalPropertyMap const* Front() const { BeAssert(!m_list.empty());  return m_list.empty() ? nullptr : m_list.front(); }
                BentleyStatus Insert(RefCountedPtr<WipVerticalPropertyMap> propertyMap, WipVerticalPropertyMap const& parent, size_t position);
                BentleyStatus Remove(Utf8CP accessString);
                void Clear() { m_list.clear(); m_map.clear(); }
            } m_col;
        bool m_readonly;
        virtual DbTable const& _GetTable() const override;
        BentleyStatus VerifyVerticalIntegerity(WipVerticalPropertyMap const& propertyMap) const;
    protected:
        void MakeWritable() { m_readonly = false; }
        void Clear();
        WipCompoundPropertyMap(WipPropertyMapContainer const& container, ECN::ECPropertyCR ecProperty)
            : WipVerticalPropertyMap(container, ecProperty), m_readonly(false)
            {}
        WipCompoundPropertyMap(ECN::ECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap)
            : WipVerticalPropertyMap(ecProperty, parentPropertyMap), m_readonly(false)
            {}
        virtual ~WipCompoundPropertyMap() {}
        virtual BentleyStatus _Validate() const override;

    public:
        bool IsReadonly() const { return m_readonly; }
        void MakeReadOnly() { m_readonly = true; }
        BentleyStatus Insert(RefCountedPtr<WipVerticalPropertyMap> propertyMap, size_t position = UINT_MAX);
        BentleyStatus Remove(Utf8CP accessString);
        const_iterator begin() const { return m_col.GetList().begin(); }
        const_iterator end() const { return m_col.GetList().end(); }
        size_t size() const { return m_col.GetList().size(); }
        bool empty() const { return m_col.GetList().empty(); }
        WipVerticalPropertyMap const* Find(Utf8CP accessString, bool recusive = true) const;
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Abstract baseclass of all property map that mapped vertically and have single column
//+===============+===============+===============+===============+===============+======
struct WipColumnVerticalPropertyMap: WipVerticalPropertyMap
    {
    private:
        DbColumn const& m_column;
        virtual DbTable const& _GetTable() const override
            {
            return m_column.GetTable();
            }
    protected:
        WipColumnVerticalPropertyMap(WipPropertyMapContainer const& container, ECN::ECPropertyCR ecProperty, DbColumn const& column)
            : WipVerticalPropertyMap(container, ecProperty), m_column(column)
            {}
        WipColumnVerticalPropertyMap(ECN::ECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column)
            : WipVerticalPropertyMap(ecProperty, parentPropertyMap), m_column(column)
            {}
        virtual ~WipColumnVerticalPropertyMap() {}
    public:
        DbColumn const& GetColumn() const { return m_column; }
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Abstract baseclass of all property map horizontally and have one column per table
//+===============+===============+===============+===============+===============+======
struct WipColumnHorizontalPropertyMap : WipHorizontalPropertyMap
    {
    private:
        std::map<Utf8CP, RefCountedPtr<WipColumnVerticalPropertyMap>, CompareIUtf8Ascii> m_vmapsPerTable;
        virtual BentleyStatus _Validate() const override;
    protected:
        WipColumnHorizontalPropertyMap(WipPropertyMapContainer const& container, ECN::ECPropertyCR ecProperty, std::vector<RefCountedPtr<WipColumnVerticalPropertyMap>> const& maps);
        virtual ~WipColumnHorizontalPropertyMap() {}
    public:
        WipColumnVerticalPropertyMap const* GetPropertyMap(Utf8CP tableName) const;
        WipColumnVerticalPropertyMap const* GetPropertyMap(DbTable const& table) const;
        std::vector<WipColumnVerticalPropertyMap const*> GetPropertyMaps() const;
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipSystemPropertyMap : WipColumnHorizontalPropertyMap
    {
    protected:
        WipSystemPropertyMap(WipPropertyMapContainer const& container, ECN::PrimitiveECPropertyCR ecProperty, std::vector<RefCountedPtr<WipPrimitivePropertyMap>> const& maps)
            : WipColumnHorizontalPropertyMap(container, ecProperty, std::vector<RefCountedPtr<WipColumnVerticalPropertyMap>>(maps.begin(), maps.end()))
            {
            BeAssert(ecProperty.GetType() == ECN::PrimitiveType::PRIMITIVETYPE_Long);
            }
        virtual ~WipSystemPropertyMap() {}
        static BentleyStatus TryCreateVerticalMaps(std::vector<RefCountedPtr<WipPrimitivePropertyMap>>& propertyMaps, ECSqlSystemProperty systemProperty, WipPropertyMapContainer const& container, std::vector<DbColumn const*> const& columns);
    };

//==============================Concerte implementations=================================
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipPrimitivePropertyMap final : WipColumnVerticalPropertyMap
    {
    private:
        virtual BentleyStatus _Validate() const override;
    protected:
        WipPrimitivePropertyMap(WipPropertyMapContainer const& container, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column)
            : WipColumnVerticalPropertyMap(container, ecProperty, column)
            {}
        WipPrimitivePropertyMap(ECN::PrimitiveECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column)
            : WipColumnVerticalPropertyMap(ecProperty, parentPropertyMap, column)
            {}
        virtual ~WipPrimitivePropertyMap() {}
    public:
        static RefCountedPtr<WipPrimitivePropertyMap> CreateInstance(WipPropertyMapContainer const& container, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column);
        static RefCountedPtr<WipPrimitivePropertyMap> CreateInstance(ECN::PrimitiveECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column);
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipPrimitiveArrayPropertyMap final : WipColumnVerticalPropertyMap
    {
    private:
        virtual BentleyStatus _Validate() const override { return SUCCESS; }
    protected:
        WipPrimitiveArrayPropertyMap(WipPropertyMapContainer const& container, ECN::ArrayECPropertyCR ecProperty, DbColumn const& column)
            : WipColumnVerticalPropertyMap(container, ecProperty, column)
            {}
        WipPrimitiveArrayPropertyMap(ECN::ArrayECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column)
            : WipColumnVerticalPropertyMap(ecProperty, parentPropertyMap, column)
            {}
        virtual ~WipPrimitiveArrayPropertyMap() {}
    public:
        static RefCountedPtr<WipPrimitiveArrayPropertyMap> CreateInstance(WipPropertyMapContainer const& container, ECN::ArrayECPropertyCR ecProperty, DbColumn const& column);
        static RefCountedPtr<WipPrimitiveArrayPropertyMap> CreateInstance(ECN::ArrayECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column);
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipStructPropertyMap final : WipCompoundPropertyMap
    {
    protected:
        WipStructPropertyMap(WipPropertyMapContainer const& container, ECN::StructECPropertyCR ecProperty)
            : WipCompoundPropertyMap(container, ecProperty)
            {}
        WipStructPropertyMap(ECN::StructECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap)
            : WipCompoundPropertyMap(ecProperty, parentPropertyMap)
            {}
        virtual ~WipStructPropertyMap() {}
    public:
        static RefCountedPtr<WipStructPropertyMap> CreateInstance(WipPropertyMapContainer const& container, ECN::StructECPropertyCR ecProperty);
        static RefCountedPtr<WipStructPropertyMap> CreateInstance(ECN::StructECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap);
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipStructArrayPropertyMap final : WipColumnVerticalPropertyMap
    {
    private:
        virtual BentleyStatus _Validate() const override { return SUCCESS; }

    protected:
        WipStructArrayPropertyMap(WipPropertyMapContainer const& container, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column)
            : WipColumnVerticalPropertyMap(container, ecProperty, column)
            {}
        WipStructArrayPropertyMap(ECN::StructArrayECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column)
            : WipColumnVerticalPropertyMap(ecProperty, parentPropertyMap, column)
            {}
        virtual ~WipStructArrayPropertyMap() {}
    public:
        static RefCountedPtr<WipStructArrayPropertyMap> CreateInstance(WipPropertyMapContainer const& container, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column);
        static RefCountedPtr<WipStructArrayPropertyMap> CreateInstance(ECN::StructArrayECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column);
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipNavigationPropertyMap final : WipCompoundPropertyMap
    {
    struct RelECClassIdPropertyMap final : WipColumnVerticalPropertyMap
        {
        private:
            ECN::ECClassId m_defaultClassId;
            virtual BentleyStatus _Validate() const override { return SUCCESS; }
        protected:
            RelECClassIdPropertyMap(ECN::PrimitiveECPropertyCR ecProperty, WipNavigationPropertyMap const& parentPropertyMap, DbColumn const& column, ECN::ECClassId defaultClassId)
                : WipColumnVerticalPropertyMap(ecProperty, parentPropertyMap, column), m_defaultClassId(defaultClassId)
                {}
            virtual ~RelECClassIdPropertyMap() {}
        public:
            ECN::ECClassId GetDefaultClassId() const { return m_defaultClassId; }
            static RefCountedPtr<RelECClassIdPropertyMap> CreateInstance(WipNavigationPropertyMap const& parentPropertyMap, DbColumn const& column, ECN::ECClassId defaultRelClassId);
        };
    struct IdPropertyMap final : WipColumnVerticalPropertyMap
        {
        private:
            virtual BentleyStatus _Validate() const override { return SUCCESS; }
        protected:
            IdPropertyMap(ECN::PrimitiveECPropertyCR ecProperty, WipNavigationPropertyMap const& parentPropertyMap, DbColumn const& column)
                : WipColumnVerticalPropertyMap(ecProperty, parentPropertyMap, column)
                {}
            virtual ~IdPropertyMap() {}
        public:
            static RefCountedPtr<IdPropertyMap> CreateInstance(WipNavigationPropertyMap const& parentPropertyMap, DbColumn const& column);
        };
    private:
        BentleyStatus Init(DbColumn const& relECClassIdColumn, ECN::ECClassId defaultRelClassId, DbColumn const& idColumn);
        virtual BentleyStatus _Validate() const override;
    protected:
        WipNavigationPropertyMap(WipPropertyMapContainer const& container, ECN::NavigationECPropertyCR ecProperty)
            : WipCompoundPropertyMap(container, ecProperty)
            {}
        virtual ~WipNavigationPropertyMap() {}
    public:
        BentleyStatus Setup(DbColumn const& relECClassIdColumn, ECN::ECClassId defaultRelClassId, DbColumn const& idColumn);
        RelECClassIdPropertyMap const& GetRelECClassId() const { return static_cast<RelECClassIdPropertyMap const&>(*Find(ECDbSystemSchemaHelper::RELECCLASSID_PROPNAME)); }
        IdPropertyMap const& GetId() const { return static_cast<IdPropertyMap const&>(*Find(ECDbSystemSchemaHelper::ID_PROPNAME)); };
        static RefCountedPtr<WipNavigationPropertyMap> CreateInstance(WipPropertyMapContainer const& container, ECN::NavigationECPropertyCR ecProperty);

    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipPoint2dPropertyMap final: WipCompoundPropertyMap
    {
    private:
        BentleyStatus Init(DbColumn const& x, DbColumn const& y);
        virtual BentleyStatus _Validate() const override;
    protected:
        WipPoint2dPropertyMap(WipPropertyMapContainer const& container, ECN::PrimitiveECPropertyCR ecProperty)
            : WipCompoundPropertyMap(container, ecProperty)
            {
            }
        WipPoint2dPropertyMap(ECN::PrimitiveECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap)
            : WipCompoundPropertyMap(ecProperty, parentPropertyMap)
            {
            }
        virtual ~WipPoint2dPropertyMap() {}
    public:
        WipPrimitivePropertyMap const& GetX() const { return static_cast<WipPrimitivePropertyMap const&>(*Find(ECDbSystemSchemaHelper::X_PROPNAME)); }
        WipPrimitivePropertyMap const& GetY() const { return static_cast<WipPrimitivePropertyMap const&>(*Find(ECDbSystemSchemaHelper::Y_PROPNAME)); }
        static RefCountedPtr<WipPoint2dPropertyMap> CreateInstance(WipPropertyMapContainer const& container, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y);
        static RefCountedPtr<WipPoint2dPropertyMap> CreateInstance(ECN::PrimitiveECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& x, DbColumn const& y);
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipPoint3dPropertyMap final : WipCompoundPropertyMap
    {
    private:
        BentleyStatus Init(DbColumn const& x, DbColumn const& y, DbColumn const& z);
        virtual BentleyStatus _Validate() const override;
    protected:
        WipPoint3dPropertyMap(WipPropertyMapContainer const& container, ECN::PrimitiveECPropertyCR ecProperty)
            : WipCompoundPropertyMap(container, ecProperty)
            {
            }
        WipPoint3dPropertyMap(ECN::PrimitiveECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap)
            : WipCompoundPropertyMap(ecProperty, parentPropertyMap)
            {
            }
        virtual ~WipPoint3dPropertyMap() {}
    public:
        WipPrimitivePropertyMap const& GetX() const { return static_cast<WipPrimitivePropertyMap const&>(*Find(ECDbSystemSchemaHelper::X_PROPNAME)); }
        WipPrimitivePropertyMap const& GetY() const { return static_cast<WipPrimitivePropertyMap const&>(*Find(ECDbSystemSchemaHelper::Y_PROPNAME)); }
        WipPrimitivePropertyMap const& GetZ() const { return static_cast<WipPrimitivePropertyMap const&>(*Find(ECDbSystemSchemaHelper::Z_PROPNAME)); }
        static RefCountedPtr<WipPoint3dPropertyMap> CreateInstance(WipPropertyMapContainer const& container, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y, DbColumn const& z);
        static RefCountedPtr<WipPoint3dPropertyMap> CreateInstance(ECN::PrimitiveECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& x, DbColumn const& y, DbColumn const& z);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipECInstanceIdPropertyMap final : WipSystemPropertyMap
    {
    protected:
        WipECInstanceIdPropertyMap(WipPropertyMapContainer const& container, ECN::PrimitiveECPropertyCR ecProperty, std::vector<RefCountedPtr<WipPrimitivePropertyMap>> const& maps)
            : WipSystemPropertyMap(container, ecProperty, maps)
            {}
        virtual ~WipECInstanceIdPropertyMap() {}
    public:
        static RefCountedPtr<WipECInstanceIdPropertyMap> CreateInstance(WipPropertyMapContainer const& container, std::vector<DbColumn const*> const& columns);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipECClassIdPropertyMap final : WipSystemPropertyMap
    {
    private:
        ECN::ECClassId m_defaultECClassId;
    protected:
        WipECClassIdPropertyMap(WipPropertyMapContainer const& container, ECN::PrimitiveECPropertyCR ecProperty, std::vector<RefCountedPtr<WipPrimitivePropertyMap>> const& maps, ECN::ECClassId defaultECClassId)
            : WipSystemPropertyMap(container, ecProperty, maps), m_defaultECClassId(defaultECClassId)
            {}
        virtual ~WipECClassIdPropertyMap() {}
    public:
        ECN::ECClassId GetDefaultECClassId() const { return m_defaultECClassId; }
        static RefCountedPtr<WipECClassIdPropertyMap> CreateInstance(WipPropertyMapContainer const& container, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipConstraintECClassIdPropertyMap final : WipSystemPropertyMap
    {
    enum class ConstraintType
        {
        Source, Target
        };
    private:
        ECN::ECClassId m_defaultECClassId;
        ConstraintType m_constraintType;
    protected:
        WipConstraintECClassIdPropertyMap(WipPropertyMapContainer const& container, ECN::PrimitiveECPropertyCR ecProperty, std::vector<RefCountedPtr<WipPrimitivePropertyMap>> const& maps, ECN::ECClassId defaultECClassId, ConstraintType constraintType)
            : WipSystemPropertyMap(container, ecProperty, maps), m_defaultECClassId(defaultECClassId), m_constraintType(constraintType)
            {}
        virtual ~WipConstraintECClassIdPropertyMap() {}
    public:
        ECN::ECClassId GetDefaultECClassId() const { return m_defaultECClassId; }
        bool IsSource() const { return m_constraintType == ConstraintType::Source; }
        bool IsTarget() const { return m_constraintType == ConstraintType::Target; }
        static RefCountedPtr<WipConstraintECClassIdPropertyMap> CreateInstance(WipPropertyMapContainer const& container, ECN::ECClassId defaultEClassId, ConstraintType constraintType, std::vector<DbColumn const*> const& columns);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipConstraintECInstanceIdIdPropertyMap final : WipSystemPropertyMap
    {
    enum class ConstraintType
        {
        Source, Target
        };
    private:
        ConstraintType m_constraintType;
    protected:
        WipConstraintECInstanceIdIdPropertyMap(WipPropertyMapContainer const& container, ECN::PrimitiveECPropertyCR ecProperty, std::vector<RefCountedPtr<WipPrimitivePropertyMap>> const& maps, ConstraintType constraintType)
            : WipSystemPropertyMap(container, ecProperty, maps)
            {}
        virtual ~WipConstraintECInstanceIdIdPropertyMap() {}
    public:
        bool IsSource() const { return m_constraintType == ConstraintType::Source; }
        bool IsTarget() const { return m_constraintType == ConstraintType::Target; }
        static RefCountedPtr<WipConstraintECInstanceIdIdPropertyMap> CreateInstance(WipPropertyMapContainer const& container, ConstraintType constraintType, std::vector<DbColumn const*> const& columns);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======

struct WipPropertyMapFactory final
    {
    private:
        WipPropertyMapFactory(){}
        static RefCountedPtr<WipVerticalPropertyMap> CreateCopy(WipVerticalPropertyMap const& propertyMap, WipPropertyMapContainer const& newContainer, WipVerticalPropertyMap const* newParent);

    public:
        //! Data Property Maps
        static RefCountedPtr<WipPrimitivePropertyMap> CreatePrimitivePropertyMap(WipPropertyMapContainer const& container, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column);
        static RefCountedPtr<WipPrimitivePropertyMap> CreatePrimitivePropertyMap(ECN::PrimitiveECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column);
        static RefCountedPtr<WipPrimitiveArrayPropertyMap> CreatePrimitiveArrayPropertyMap(WipPropertyMapContainer const& container, ECN::ArrayECPropertyCR ecProperty, DbColumn const& column);
        static RefCountedPtr<WipPrimitiveArrayPropertyMap> CreatePrimitiveArrayPropertyMap(ECN::ArrayECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column);
        static RefCountedPtr<WipStructPropertyMap> CreateStructPropertyMap(WipPropertyMapContainer const& container, ECN::StructECPropertyCR ecProperty);
        static RefCountedPtr<WipStructPropertyMap> CreateStructPropertyMap(ECN::StructECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap);
        static RefCountedPtr<WipStructArrayPropertyMap> CreateStructArrayPropertyMap(WipPropertyMapContainer const& container, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column);
        static RefCountedPtr<WipStructArrayPropertyMap> CreateStructArrayPropertyMap(ECN::StructArrayECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column);
        static RefCountedPtr<WipPoint2dPropertyMap> CreatePoint2dPropertyMap(WipPropertyMapContainer const& container, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y);
        static RefCountedPtr<WipPoint2dPropertyMap> CreatePoint2dPropertyMap(ECN::PrimitiveECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& x, DbColumn const& y);
        static RefCountedPtr<WipPoint3dPropertyMap> CreatePoint3dPropertyMap(WipPropertyMapContainer const& container, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y, DbColumn const& z);
        static RefCountedPtr<WipPoint3dPropertyMap> CreatePoint3dPropertyMap(ECN::PrimitiveECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& x, DbColumn const& y, DbColumn const& z);
        static RefCountedPtr<WipNavigationPropertyMap> CreateNavigationPropertyMap(WipPropertyMapContainer const& container, ECN::NavigationECPropertyCR ecProperty);
        static RefCountedPtr<WipECInstanceIdPropertyMap> CreateECInstanceIdPropertyMap(WipPropertyMapContainer const& container, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<WipECClassIdPropertyMap> CreateECClassIdPropertyMap(WipPropertyMapContainer const& container, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<WipConstraintECClassIdPropertyMap> CreateSourceECClassIdPropertyMap(WipPropertyMapContainer const& container, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<WipConstraintECClassIdPropertyMap> CreateTargetECClassIdPropertyMap(WipPropertyMapContainer const& container, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<WipConstraintECInstanceIdIdPropertyMap> CreateSourceECInstanceIdPropertyMap(WipPropertyMapContainer const& container, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<WipConstraintECInstanceIdIdPropertyMap> CreateTargetECInstanceIdPropertyMap(WipPropertyMapContainer const& container, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<WipVerticalPropertyMap> CreateCopy(WipVerticalPropertyMap const& propertyMap, WipPropertyMapContainer const& newContainer);
    };


END_BENTLEY_SQLITE_EC_NAMESPACE
