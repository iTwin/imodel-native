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

enum class DispatcherFeedback
    {
    Cancel, //! cancel traversal altogether and return
    NextSibling, //! do not traverse children of current node instead go with next sibling
    Next //! if there is children of current node process them first and then go to next sibling
    };

enum class PropertyMapKind : int
    {
    Nil = 0x0,
    PrimitivePropertyMap = 0x1,
    PrimitiveArrayPropertyMap = 0x2,
    StructPropertyMap = 0x4,
    StructArrayPropertyMap = 0x8,
    Point3dPropertyMap = 0x10,
    Point2dPropertyMap = 0x20,
    NavigationPropertyMap = 0x40,
    ECInstanceIdPropertyMap = 0x80,
    ECClassIdPropertyMap = 0x100,
    ConstraintECClassIdPropertyMap = 0x200,
    ConstraintECInstanceIdIdPropertyMap = 0x400,
    NavRelECClassIdPropertyMap = 0x800,
    NavIdPropertyMap =0x1000,

    System = ECInstanceIdPropertyMap | ECClassIdPropertyMap | ConstraintECClassIdPropertyMap | ConstraintECInstanceIdIdPropertyMap,
    Business = PrimitivePropertyMap
    | PrimitiveArrayPropertyMap
    | StructPropertyMap
    | StructArrayPropertyMap
    | Point3dPropertyMap
    | Point2dPropertyMap
    | NavigationPropertyMap
    | NavRelECClassIdPropertyMap
    | NavIdPropertyMap,
    All = System | Business
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct IPropertyMapDispatcher
    {

    private:
        virtual DispatcherFeedback _Dispatch(WipColumnVerticalPropertyMap const& propertyMap) const { return DispatcherFeedback::Next; }
        virtual DispatcherFeedback _Dispatch(WipCompoundPropertyMap const& propertyMap) const { return DispatcherFeedback::Next; }
        virtual DispatcherFeedback _Dispatch(WipColumnHorizontalPropertyMap const& propertyMap) const { return DispatcherFeedback::Next; }

    public:
        DispatcherFeedback Dispatch(WipColumnVerticalPropertyMap const& propertyMap) const { return _Dispatch(propertyMap); }
        DispatcherFeedback Dispatch(WipCompoundPropertyMap const& propertyMap) const { return _Dispatch(propertyMap); }
        DispatcherFeedback Dispatch(WipColumnHorizontalPropertyMap const& propertyMap) const { return _Dispatch(propertyMap); }
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct ISupportPropertyMapDispatcher
    {
    private:
        virtual DispatcherFeedback _Accept(IPropertyMapDispatcher const&  dispatcher) const = 0;
    public:
        DispatcherFeedback Accept(IPropertyMapDispatcher const& dispatcher) const { return _Accept(dispatcher); }
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipPropertyMapContainer final : NonCopyableClass, ISupportPropertyMapDispatcher
    {
    typedef std::vector<WipPropertyMap const*>::const_iterator const_iterator;
    private:
        ClassMap const& m_classMap;
        std::vector<WipPropertyMap const*> m_directDecendentList; //! contain direct decedents in order.
        std::map<Utf8CP, RefCountedPtr<WipPropertyMap>, CompareIUtf8Ascii> m_map; //! contain all propertymap owned by the container
        bool m_readonly;

    private:
        virtual DispatcherFeedback _Accept(IPropertyMapDispatcher const&  dispatcher)  const override;

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
        const_iterator begin() const { return m_directDecendentList.begin(); }
        const_iterator end() const { return m_directDecendentList.end(); }
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Abstract baseclass of all property map. Do not pollute this classECDbMap const& ecdbMap
//+===============+===============+===============+===============+===============+======
struct WipPropertyMap : RefCountedBase, NonCopyableClass, ISupportPropertyMapDispatcher
    {
    private:
        ECN::ECPropertyCR m_ecProperty;
        Utf8String m_propertyAccessString;
        WipPropertyMap const* m_parentPropertMap;    
        ClassMap const& m_classMap;
        virtual BentleyStatus _Validate() const = 0;
        virtual bool _IsMappedToTable(DbTable const& table) const = 0;
        bool m_isInEditMode;
        PropertyMapKind m_kind;
       
    protected:
        WipPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::ECPropertyCR ecProperty);
        WipPropertyMap(PropertyMapKind kind, ECN::ECPropertyCR ecProperty, WipPropertyMap const& parentPropertyMap);
        virtual ~WipPropertyMap() {}
    public:
        //! A property is injected if it does not ECClass but added by ECDb
        bool InEditMode() const { return m_isInEditMode; }
        void FinishEditing() { BeAssert(m_isInEditMode);  m_isInEditMode = false; }
        Utf8String GetName() const { return GetProperty().GetName(); }
        ECN::ECPropertyCR GetProperty() const { return m_ecProperty; }
        Utf8StringCR GetAccessString() const { return m_propertyAccessString; }
        WipPropertyMap const* GetParent() const { return m_parentPropertMap; }
        ClassMap const& GetClassMap() const { return m_classMap; }
        WipPropertyMap const& GetRoot() const;
        PropertyMapKind GetKind() const { return m_kind; }
        bool IsSystem() const { return Enum::Contains(PropertyMapKind::System, GetKind()); }
        bool IsBusiness () const { return Enum::Contains(PropertyMapKind::Business, GetKind()); }
        bool IsMappedToTable(DbTable const& table) const { return _IsMappedToTable(table); } //WIP Move to ECSQL
        bool IsMappedToClassMapTables() const; //WIP Move to ECSQL
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
        virtual bool _IsMappedToTable(DbTable const& table) const override { return &GetTable() == &table; }
    public:
        WipVerticalPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::ECPropertyCR ecProperty)
            : WipPropertyMap(kind, classMap, ecProperty)
            {}
        WipVerticalPropertyMap(PropertyMapKind kind, ECN::ECPropertyCR ecProperty, WipPropertyMap const& parentPropertyMap)
            : WipPropertyMap(kind, ecProperty, parentPropertyMap)
            {}
        ~WipVerticalPropertyMap() {}
        DbTable const& GetTable() const { return _GetTable(); }
       
        RefCountedPtr<WipVerticalPropertyMap> CreateCopy(ClassMap const& newClassMapContext) const;
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Abstract baseclass of all property map that are mapped vertically into multiple tables.
// The main class of properties it represent are system properties.
//+===============+===============+===============+===============+===============+======
struct WipHorizontalPropertyMap : WipPropertyMap
    {
    private:
        virtual bool _IsMappedToTable(DbTable const& table) const override 
            { 
            for (DbTable const* t : GetTables())
                if (t == &table)
                    return true;

            return false;
            }

    protected:
        WipHorizontalPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::ECPropertyCR ecProperty)
            : WipPropertyMap(kind, classMap, ecProperty)
            {}
        WipHorizontalPropertyMap(PropertyMapKind kind, ECN::ECPropertyCR ecProperty, WipPropertyMap const& parentPropertyMap)
            : WipPropertyMap(kind, ecProperty, parentPropertyMap)
            {}
        virtual  ~WipHorizontalPropertyMap() {}
        virtual std::vector<DbTable const*> const& _GetTables() const = 0;

    public:
        std::vector<DbTable const*> const& GetTables() const { return _GetTables(); }
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
        WipCompoundPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::ECPropertyCR ecProperty)
            : WipVerticalPropertyMap(kind, classMap, ecProperty), m_readonly(false)
            {}
        WipCompoundPropertyMap(PropertyMapKind kind, ECN::ECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap)
            : WipVerticalPropertyMap(kind, ecProperty, parentPropertyMap), m_readonly(false)
            {}
        virtual ~WipCompoundPropertyMap() {}
        virtual BentleyStatus _Validate() const override;
        DispatcherFeedback AcceptChildren(IPropertyMapDispatcher const&  dispatcher) const;
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
        WipColumnVerticalPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::ECPropertyCR ecProperty, DbColumn const& column)
            : WipVerticalPropertyMap(kind, classMap, ecProperty), m_column(column)
            {}
        WipColumnVerticalPropertyMap(PropertyMapKind kind, ECN::ECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column)
            : WipVerticalPropertyMap(kind, ecProperty, parentPropertyMap), m_column(column)
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
        std::vector<WipColumnVerticalPropertyMap const*> m_vmaps;
        virtual BentleyStatus _Validate() const override;
        std::vector<DbTable const*> m_tables;
    protected:
        WipColumnHorizontalPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::ECPropertyCR ecProperty, std::vector<RefCountedPtr<WipColumnVerticalPropertyMap>> const& maps);
        virtual ~WipColumnHorizontalPropertyMap() {}
        virtual std::vector<DbTable const*> const& _GetTables() const override { return m_tables; }
    public:
        WipColumnVerticalPropertyMap const* FindVerticalPropertyMap(Utf8CP tableName) const;
        WipColumnVerticalPropertyMap const* FindVerticalPropertyMap(DbTable const& table) const;
        std::vector<WipColumnVerticalPropertyMap const*> const& GetVerticalPropertyMaps() const;
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipSystemPropertyMap : WipColumnHorizontalPropertyMap
    {
    protected:
        WipSystemPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, std::vector<RefCountedPtr<WipPrimitivePropertyMap>> const& maps)
            : WipColumnHorizontalPropertyMap(kind, classMap, ecProperty, std::vector<RefCountedPtr<WipColumnVerticalPropertyMap>>(maps.begin(), maps.end()))
            {
            BeAssert(ecProperty.GetType() == ECN::PrimitiveType::PRIMITIVETYPE_Long);
            }
        virtual ~WipSystemPropertyMap() {}
        static BentleyStatus TryCreateVerticalMaps(std::vector<RefCountedPtr<WipPrimitivePropertyMap>>& propertyMaps, ECSqlSystemProperty systemProperty, ClassMap const& classMap, std::vector<DbColumn const*> const& columns);
    public:
        bool IsMappedToSingleTable() const { return GetVerticalPropertyMaps().size() == 1LL; }
    };

//==============================Concerte implementations=================================
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipPrimitivePropertyMap final : WipColumnVerticalPropertyMap
    {
    private:
        virtual BentleyStatus _Validate() const override;
        virtual DispatcherFeedback _Accept(IPropertyMapDispatcher const&  dispatcher)  const override { return dispatcher.Dispatch(*this); }
    protected:
        WipPrimitivePropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column)
            : WipColumnVerticalPropertyMap(kind, classMap, ecProperty, column)
            {}
        WipPrimitivePropertyMap(PropertyMapKind kind, ECN::PrimitiveECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column)
            : WipColumnVerticalPropertyMap(kind, ecProperty, parentPropertyMap, column)
            {}
        virtual ~WipPrimitivePropertyMap() {}
    public:
        static RefCountedPtr<WipPrimitivePropertyMap> CreateInstance(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column);
        static RefCountedPtr<WipPrimitivePropertyMap> CreateInstance(ECN::PrimitiveECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column);
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipPrimitiveArrayPropertyMap final : WipColumnVerticalPropertyMap
    {
    private:
        virtual BentleyStatus _Validate() const override { return SUCCESS; }
        virtual DispatcherFeedback _Accept(IPropertyMapDispatcher const&  dispatcher)  const override { return dispatcher.Dispatch(*this); }
    protected:
        WipPrimitiveArrayPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::ArrayECPropertyCR ecProperty, DbColumn const& column)
            : WipColumnVerticalPropertyMap(kind, classMap, ecProperty, column)
            {}
        WipPrimitiveArrayPropertyMap(PropertyMapKind kind, ECN::ArrayECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column)
            : WipColumnVerticalPropertyMap(kind, ecProperty, parentPropertyMap, column)
            {}
        virtual ~WipPrimitiveArrayPropertyMap() {}
    public:
        static RefCountedPtr<WipPrimitiveArrayPropertyMap> CreateInstance(ClassMap const& classMap, ECN::ArrayECPropertyCR ecProperty, DbColumn const& column);
        static RefCountedPtr<WipPrimitiveArrayPropertyMap> CreateInstance(ECN::ArrayECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column);
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipStructPropertyMap final : WipCompoundPropertyMap
    {
    private:
        virtual DispatcherFeedback _Accept(IPropertyMapDispatcher const&  dispatcher)  const override;

    protected:
        WipStructPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::StructECPropertyCR ecProperty)
            : WipCompoundPropertyMap(kind, classMap, ecProperty)
            {}
        WipStructPropertyMap(PropertyMapKind kind, ECN::StructECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap)
            : WipCompoundPropertyMap(kind, ecProperty, parentPropertyMap)
            {}
        virtual ~WipStructPropertyMap() {}
    public:
        static RefCountedPtr<WipStructPropertyMap> CreateInstance(ClassMap const& classMap, ECN::StructECPropertyCR ecProperty);
        static RefCountedPtr<WipStructPropertyMap> CreateInstance(ECN::StructECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap);
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipStructArrayPropertyMap final : WipColumnVerticalPropertyMap
    {
    private:
        virtual BentleyStatus _Validate() const override { return SUCCESS; }
        virtual DispatcherFeedback _Accept(IPropertyMapDispatcher const&  dispatcher)  const override { return dispatcher.Dispatch(*this); }

    protected:
        WipStructArrayPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column)
            : WipColumnVerticalPropertyMap(kind, classMap, ecProperty, column)
            {}
        WipStructArrayPropertyMap(PropertyMapKind kind, ECN::StructArrayECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column)
            : WipColumnVerticalPropertyMap(kind, ecProperty, parentPropertyMap, column)
            {}
        virtual ~WipStructArrayPropertyMap() {}
    public:
        static RefCountedPtr<WipStructArrayPropertyMap> CreateInstance(ClassMap const& classMap, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column);
        static RefCountedPtr<WipStructArrayPropertyMap> CreateInstance(ECN::StructArrayECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column);
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipNavigationPropertyMap final : WipCompoundPropertyMap
    {
    enum class NavigationEnd
        {
        From,
        To
        };

    struct RelECClassIdPropertyMap final : WipColumnVerticalPropertyMap
        {
        private:
            ECN::ECClassId m_defaultClassId;
            virtual BentleyStatus _Validate() const override { return SUCCESS; }
            virtual DispatcherFeedback _Accept(IPropertyMapDispatcher const&  dispatcher)  const override { return  dispatcher.Dispatch(*this); }

        protected:
            RelECClassIdPropertyMap(PropertyMapKind kind, ECN::PrimitiveECPropertyCR ecProperty, WipNavigationPropertyMap const& parentPropertyMap, DbColumn const& column, ECN::ECClassId defaultClassId)
                : WipColumnVerticalPropertyMap(kind, ecProperty, parentPropertyMap, column), m_defaultClassId(defaultClassId)
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
            virtual DispatcherFeedback _Accept(IPropertyMapDispatcher const&  dispatcher)  const override { return  dispatcher.Dispatch(*this); }

        protected:
            IdPropertyMap(PropertyMapKind kind, ECN::PrimitiveECPropertyCR ecProperty, WipNavigationPropertyMap const& parentPropertyMap, DbColumn const& column)
                : WipColumnVerticalPropertyMap(kind, ecProperty, parentPropertyMap, column)
                {}
            virtual ~IdPropertyMap() {}
        public:
            static RefCountedPtr<IdPropertyMap> CreateInstance(WipNavigationPropertyMap const& parentPropertyMap, DbColumn const& column);
        };
    private:
        BentleyStatus Init(DbColumn const& relECClassIdColumn, ECN::ECClassId defaultRelClassId, DbColumn const& idColumn);
        virtual BentleyStatus _Validate() const override;
        virtual DispatcherFeedback _Accept(IPropertyMapDispatcher const&  dispatcher)  const override;
    protected:
        WipNavigationPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::NavigationECPropertyCR ecProperty)
            : WipCompoundPropertyMap(kind, classMap, ecProperty)
            {}
        virtual ~WipNavigationPropertyMap() {}
    public:
        BentleyStatus Setup(DbColumn const& relECClassIdColumn, ECN::ECClassId defaultRelClassId, DbColumn const& idColumn);
        RelECClassIdPropertyMap const& GetRelECClassId() const { return static_cast<RelECClassIdPropertyMap const&>(*Find(ECDbSystemSchemaHelper::RELECCLASSID_PROPNAME)); }
        IdPropertyMap const& GetId() const { return static_cast<IdPropertyMap const&>(*Find(ECDbSystemSchemaHelper::ID_PROPNAME)); };
        bool IsSupportedInECSql(bool logIfNotSupported = false) const;
        static RefCountedPtr<WipNavigationPropertyMap> CreateInstance(ClassMap const& classMap, ECN::NavigationECPropertyCR ecProperty);
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipPoint2dPropertyMap final: WipCompoundPropertyMap
    {
    private:
        BentleyStatus Init(DbColumn const& x, DbColumn const& y);
        virtual BentleyStatus _Validate() const override;
        virtual DispatcherFeedback _Accept(IPropertyMapDispatcher const&  dispatcher)  const override;
    protected:
        WipPoint2dPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty)
            : WipCompoundPropertyMap(kind, classMap, ecProperty)
            {
            }
        WipPoint2dPropertyMap(PropertyMapKind kind, ECN::PrimitiveECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap)
            : WipCompoundPropertyMap(kind, ecProperty, parentPropertyMap)
            {
            }
        virtual ~WipPoint2dPropertyMap() {}
    public:
        WipPrimitivePropertyMap const& GetX() const { return static_cast<WipPrimitivePropertyMap const&>(*Find(ECDbSystemSchemaHelper::X_PROPNAME)); }
        WipPrimitivePropertyMap const& GetY() const { return static_cast<WipPrimitivePropertyMap const&>(*Find(ECDbSystemSchemaHelper::Y_PROPNAME)); }
        static RefCountedPtr<WipPoint2dPropertyMap> CreateInstance(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y);
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
        virtual DispatcherFeedback _Accept(IPropertyMapDispatcher const&  dispatcher)  const override;

    protected:
        WipPoint3dPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty)
            : WipCompoundPropertyMap(kind, classMap, ecProperty)
            {
            }
        WipPoint3dPropertyMap(PropertyMapKind kind, ECN::PrimitiveECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap)
            : WipCompoundPropertyMap(kind, ecProperty, parentPropertyMap)
            {
            }
        virtual ~WipPoint3dPropertyMap() {}
    public:
        WipPrimitivePropertyMap const& GetX() const { return static_cast<WipPrimitivePropertyMap const&>(*Find(ECDbSystemSchemaHelper::X_PROPNAME)); }
        WipPrimitivePropertyMap const& GetY() const { return static_cast<WipPrimitivePropertyMap const&>(*Find(ECDbSystemSchemaHelper::Y_PROPNAME)); }
        WipPrimitivePropertyMap const& GetZ() const { return static_cast<WipPrimitivePropertyMap const&>(*Find(ECDbSystemSchemaHelper::Z_PROPNAME)); }
        static RefCountedPtr<WipPoint3dPropertyMap> CreateInstance(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y, DbColumn const& z);
        static RefCountedPtr<WipPoint3dPropertyMap> CreateInstance(ECN::PrimitiveECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& x, DbColumn const& y, DbColumn const& z);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipECInstanceIdPropertyMap final : WipSystemPropertyMap
    {
    private:
        virtual DispatcherFeedback _Accept(IPropertyMapDispatcher const&  dispatcher)  const override { return dispatcher.Dispatch(*this); }

    protected:
        WipECInstanceIdPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, std::vector<RefCountedPtr<WipPrimitivePropertyMap>> const& maps)
            : WipSystemPropertyMap(kind, classMap, ecProperty, maps)
            {}
        virtual ~WipECInstanceIdPropertyMap() {}
    public:
        static RefCountedPtr<WipECInstanceIdPropertyMap> CreateInstance(ClassMap const& classMap, std::vector<DbColumn const*> const& columns);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipECClassIdPropertyMap final : WipSystemPropertyMap
    {
    private:
        ECN::ECClassId m_defaultECClassId;
        virtual DispatcherFeedback _Accept(IPropertyMapDispatcher const&  dispatcher)  const override { return dispatcher.Dispatch(*this); }

    protected:
        WipECClassIdPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, std::vector<RefCountedPtr<WipPrimitivePropertyMap>> const& maps, ECN::ECClassId defaultECClassId)
            : WipSystemPropertyMap(kind, classMap, ecProperty, maps), m_defaultECClassId(defaultECClassId)
            {}
        virtual ~WipECClassIdPropertyMap() {}
    public:
        ECN::ECClassId GetDefaultECClassId() const { return m_defaultECClassId; }
        static RefCountedPtr<WipECClassIdPropertyMap> CreateInstance(ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns);
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
        virtual DispatcherFeedback _Accept(IPropertyMapDispatcher const&  dispatcher)  const override { return dispatcher.Dispatch(*this); }

    protected:
        WipConstraintECClassIdPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, std::vector<RefCountedPtr<WipPrimitivePropertyMap>> const& maps, ECN::ECClassId defaultECClassId, ConstraintType constraintType)
            : WipSystemPropertyMap(kind, classMap, ecProperty, maps), m_defaultECClassId(defaultECClassId), m_constraintType(constraintType)
            {}
        virtual ~WipConstraintECClassIdPropertyMap() {}
    public:
        ECN::ECClassId GetDefaultECClassId() const { return m_defaultECClassId; }
        bool IsSource() const { return m_constraintType == ConstraintType::Source; }
        bool IsTarget() const { return m_constraintType == ConstraintType::Target; }
        static RefCountedPtr<WipConstraintECClassIdPropertyMap> CreateInstance(ClassMap const& classMap, ECN::ECClassId defaultEClassId, ConstraintType constraintType, std::vector<DbColumn const*> const& columns);
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
        virtual DispatcherFeedback _Accept(IPropertyMapDispatcher const&  dispatcher)  const override { return dispatcher.Dispatch(*this); }

    protected:
        WipConstraintECInstanceIdIdPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, std::vector<RefCountedPtr<WipPrimitivePropertyMap>> const& maps, ConstraintType constraintType)
            : WipSystemPropertyMap(kind, classMap, ecProperty, maps)
            {}
        virtual ~WipConstraintECInstanceIdIdPropertyMap() {}
    public:
        bool IsSource() const { return m_constraintType == ConstraintType::Source; }
        bool IsTarget() const { return m_constraintType == ConstraintType::Target; }
        static RefCountedPtr<WipConstraintECInstanceIdIdPropertyMap> CreateInstance(ClassMap const& classMap, ConstraintType constraintType, std::vector<DbColumn const*> const& columns);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipPropertyMapFactory final
    {
    private:
        WipPropertyMapFactory(){}
        static RefCountedPtr<WipVerticalPropertyMap> CreateCopy(WipVerticalPropertyMap const& propertyMap, ClassMap const& newContext, WipVerticalPropertyMap const* newParent);

    public:
        //! Data Property Maps
        static RefCountedPtr<WipPrimitivePropertyMap> CreatePrimitivePropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column);
        static RefCountedPtr<WipPrimitivePropertyMap> CreatePrimitivePropertyMap(ECN::PrimitiveECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column);
        static RefCountedPtr<WipPrimitiveArrayPropertyMap> CreatePrimitiveArrayPropertyMap(ClassMap const& classMap, ECN::ArrayECPropertyCR ecProperty, DbColumn const& column);
        static RefCountedPtr<WipPrimitiveArrayPropertyMap> CreatePrimitiveArrayPropertyMap(ECN::ArrayECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column);
        static RefCountedPtr<WipStructPropertyMap> CreateStructPropertyMap(ClassMap const& classMap, ECN::StructECPropertyCR ecProperty);
        static RefCountedPtr<WipStructPropertyMap> CreateStructPropertyMap(ECN::StructECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap);
        static RefCountedPtr<WipStructArrayPropertyMap> CreateStructArrayPropertyMap(ClassMap const& classMap, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column);
        static RefCountedPtr<WipStructArrayPropertyMap> CreateStructArrayPropertyMap(ECN::StructArrayECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column);
        static RefCountedPtr<WipPoint2dPropertyMap> CreatePoint2dPropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y);
        static RefCountedPtr<WipPoint2dPropertyMap> CreatePoint2dPropertyMap(ECN::PrimitiveECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& x, DbColumn const& y);
        static RefCountedPtr<WipPoint3dPropertyMap> CreatePoint3dPropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y, DbColumn const& z);
        static RefCountedPtr<WipPoint3dPropertyMap> CreatePoint3dPropertyMap(ECN::PrimitiveECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& x, DbColumn const& y, DbColumn const& z);
        static RefCountedPtr<WipNavigationPropertyMap> CreateNavigationPropertyMap(ClassMap const& classMap, ECN::NavigationECPropertyCR ecProperty);
        static RefCountedPtr<WipECInstanceIdPropertyMap> CreateECInstanceIdPropertyMap(ClassMap const& classMap, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<WipECClassIdPropertyMap> CreateECClassIdPropertyMap(ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<WipConstraintECClassIdPropertyMap> CreateSourceECClassIdPropertyMap(ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<WipConstraintECClassIdPropertyMap> CreateTargetECClassIdPropertyMap(ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<WipConstraintECInstanceIdIdPropertyMap> CreateSourceECInstanceIdPropertyMap(ClassMap const& classMap, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<WipConstraintECInstanceIdIdPropertyMap> CreateTargetECInstanceIdPropertyMap(ClassMap const& classMap, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<WipVerticalPropertyMap> CreateCopy(WipVerticalPropertyMap const& propertyMap, ClassMap const& newContext);
        static RefCountedPtr<WipHorizontalPropertyMap> CreateCopy(WipHorizontalPropertyMap const& propertyMap, ClassMap const& newContext);
        static RefCountedPtr<WipConstraintECInstanceIdIdPropertyMap> CreateConstraintECInstanceIdPropertyMap(ECN::ECRelationshipEnd end, ClassMap const& classMap, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<WipConstraintECClassIdPropertyMap> CreateConstraintECClassIdPropertyMap(ECN::ECRelationshipEnd end, ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Allow to collect columns from property maps
//+===============+===============+===============+===============+===============+======
struct WipPropertyMapColumnDispatcher final: IPropertyMapDispatcher
    {
    private:
        mutable std::vector<DbColumn const*> m_columns;
        DbTable const* m_table;
        PropertyMapKind m_filter;
        bool m_doNotSkipHorizontalPropertyMaps;
    private:

        virtual DispatcherFeedback _Dispatch(WipColumnVerticalPropertyMap const& propertyMap) const override;
        virtual DispatcherFeedback _Dispatch(WipCompoundPropertyMap const& propertyMap) const override;
        virtual DispatcherFeedback _Dispatch(WipColumnHorizontalPropertyMap const& propertyMap) const override;

    public:
        WipPropertyMapColumnDispatcher(DbTable const& table, PropertyMapKind filter = PropertyMapKind::All)
            :m_table(&table), m_filter(filter), m_doNotSkipHorizontalPropertyMaps(false)
            {}
        WipPropertyMapColumnDispatcher(PropertyMapKind filter = PropertyMapKind::All, bool doNotSkipHorizontalPropertyMaps = false)
            :m_table(nullptr), m_filter(filter), m_doNotSkipHorizontalPropertyMaps(doNotSkipHorizontalPropertyMaps)
            {}
        ~WipPropertyMapColumnDispatcher(){}
        void Reset() { m_columns.clear(); }
        std::vector<DbColumn const*> const& GetColumns() const { return m_columns; }
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Allow to collect columns from property maps
//+===============+===============+===============+===============+===============+======
struct WipPropertyMapTableDispatcher final : IPropertyMapDispatcher
    {
    private:
        mutable std::set<DbTable const*> m_tables;
        PropertyMapKind m_filter;
    private:

        virtual DispatcherFeedback _Dispatch(WipColumnVerticalPropertyMap const& propertyMap) const override
            {
            if (Enum::Contains(m_filter, propertyMap.GetKind()))
                m_tables.insert(&propertyMap.GetTable());
            }
        virtual DispatcherFeedback _Dispatch(WipCompoundPropertyMap const& propertyMap) const override
            {
            return DispatcherFeedback::NextSibling;
            }
        virtual DispatcherFeedback _Dispatch(WipColumnHorizontalPropertyMap const& propertyMap) const override
            {
            if (Enum::Contains(m_filter, propertyMap.GetKind()))
                m_tables.insert(propertyMap.GetTables().begin(), propertyMap.GetTables().end());
            }

    public:
        WipPropertyMapTableDispatcher(PropertyMapKind filter = PropertyMapKind::All)
            : m_filter(filter)
            {}
        std::set<DbTable const*> GetTables() const { return m_tables; }
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Search PropertyMap with a given type
//+===============+===============+===============+===============+===============+======
struct WipPropertyMapTypeDispatcher final : IPropertyMapDispatcher
    {

    private:
        mutable std::vector<WipPropertyMap const*> m_propertyMaps;
        PropertyMapKind m_filter;
        bool m_recordCompondProperties;
    private:
        DispatcherFeedback Record(WipPropertyMap const& propertyMap) const
            {
            if (Enum::Contains(m_filter, propertyMap.GetKind()))
                m_propertyMaps.push_back(&propertyMap);

            return DispatcherFeedback::Next;
            }
        virtual DispatcherFeedback _Dispatch(WipColumnVerticalPropertyMap const& propertyMap) const override
            {
            if (m_recordCompondProperties)
                return Record(propertyMap);

            return DispatcherFeedback::Next;
            }
        virtual DispatcherFeedback _Dispatch(WipCompoundPropertyMap const& propertyMap) const override
            {
            return Record(propertyMap);
            }
        virtual DispatcherFeedback _Dispatch(WipColumnHorizontalPropertyMap const& propertyMap) const override
            {
            return Record(propertyMap);
            }

    public:
        WipPropertyMapTypeDispatcher(PropertyMapKind filter = PropertyMapKind::All, bool recordCompoundProperties = false)
            :m_filter(filter), m_recordCompondProperties(recordCompoundProperties)
            {}
        ~WipPropertyMapTypeDispatcher() {}
        void Reset() { m_propertyMaps.clear(); }
        std::vector<WipPropertyMap const*> const& ResultSet() const { return m_propertyMaps; }
      
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Allow to collect columns from property maps
//+===============+===============+===============+===============+===============+======
struct WipPropertyMapSqlDispatcher final : IPropertyMapDispatcher
    {
    enum SqlTarget
        {
        View, //!Inline view is in from. Normally it happen only in SELECT statement where view has a contract.
        Table //!Direct query against a table
        };

    struct Result
        {
        private:
            WipColumnVerticalPropertyMap const* m_propertyMap;
            NativeSqlBuilder m_sql;
        public:
            Result(WipColumnVerticalPropertyMap const& propertyMap)
                :m_propertyMap(&propertyMap)
                {}
            Result()
                :m_propertyMap(nullptr)
                {}
            ~Result(){}
            Utf8CP GetAccessString() const { return GetPropertyMap().GetAccessString().c_str(); }
            WipColumnVerticalPropertyMap const& GetPropertyMap() const { BeAssert(m_propertyMap != nullptr); return *m_propertyMap; }
            NativeSqlBuilder& GetSqlBuilderR() { return m_sql; }
            NativeSqlBuilder const& GetSqlBuilder() const{ return m_sql; }

            Utf8CP GetSql() const { return m_sql.ToString(); }
            DbColumn const& GetColumn() const { return GetPropertyMap().GetColumn(); }
            DbTable const& GetTable() const { return GetColumn().GetTable(); }
            bool  IsColumnPersisted() const { return GetColumn().GetPersistenceType() == PersistenceType::Persisted; }
            bool  IsTablePersisted() const { return GetTable().GetPersistenceType() == PersistenceType::Persisted; }
        };

    private:
        mutable bmap<Utf8CP, size_t, CompareIUtf8Ascii> m_resultSetByAccessString;
        mutable std::vector<Result> m_resultSet;
        mutable BentleyStatus m_status;
        SqlTarget m_target;
        Utf8CP m_classIdentifier;
        DbTable const& m_tableFilter;

    private:
        Result& Record(WipColumnVerticalPropertyMap const& propertyMap) const;
        bool IsAlienTable(DbTable const& table) const
            {
            if (&table != &m_tableFilter)
                {
                BeAssert(false && "PropertyMap table does not match the table filter specified.");
                m_status = ERROR;
                return true;
                }

            return false;
            }
        WipColumnVerticalPropertyMap const* FindSystemPropertyMapForTable(WipSystemPropertyMap const& systemPropertyMap) const;
        DispatcherFeedback ToNativeSql(WipColumnVerticalPropertyMap const& propertyMap) const;
        DispatcherFeedback ToNativeSql(WipConstraintECInstanceIdIdPropertyMap const& propertyMap) const;
        DispatcherFeedback ToNativeSql(WipConstraintECClassIdPropertyMap const& propertyMap) const;
        DispatcherFeedback ToNativeSql(WipECClassIdPropertyMap const& propertyMap) const;
        DispatcherFeedback ToNativeSql(WipECInstanceIdPropertyMap const& propertyMap) const;

    private:
        virtual DispatcherFeedback _Dispatch(WipColumnVerticalPropertyMap const& propertyMap) const override;
        virtual DispatcherFeedback _Dispatch(WipCompoundPropertyMap const& propertyMap) const override;
        virtual DispatcherFeedback _Dispatch(WipColumnHorizontalPropertyMap const& propertyMap) const override;

    public:
        WipPropertyMapSqlDispatcher(DbTable const& tableFilter, SqlTarget target, Utf8CP classIdentifier)
            :m_tableFilter(tableFilter), m_target(target), m_classIdentifier(classIdentifier)
            {}
        ~WipPropertyMapSqlDispatcher() {}

        BentleyStatus GetStatus() const { return m_status; }
        std::vector<Result> const& GetResultSet() const { return m_resultSet; }
        const Result* Find(Utf8CP accessString) const;
        void Reset() const { m_resultSetByAccessString.clear(); m_resultSet.clear(); m_status = SUCCESS; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
