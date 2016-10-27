/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/PropertyMap.h $
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
struct PropertyMap;
struct DataPropertyMap;
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
    Data = PrimitivePropertyMap
    | PrimitiveArrayPropertyMap
    | StructPropertyMap
    | StructArrayPropertyMap
    | Point3dPropertyMap
    | Point2dPropertyMap
    | NavigationPropertyMap
    | NavRelECClassIdPropertyMap
    | NavIdPropertyMap,
    All = System | Data
    };

enum class VisitorFeedback
    {
    Cancel, //! cancel traversal altogether and return
    NextSibling, //! do not traverse children of current node instead go with next sibling
    Next //! if there is children of current node process them first and then go to next sibling
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct IPropertyMapVisitor
    {

    private:
        virtual VisitorFeedback _Visit(WipColumnVerticalPropertyMap const& propertyMap) const { return VisitorFeedback::Next; }
        virtual VisitorFeedback _Visit(WipCompoundPropertyMap const& propertyMap) const { return VisitorFeedback::Next; }
        virtual VisitorFeedback _Visit(WipColumnHorizontalPropertyMap const& propertyMap) const { return VisitorFeedback::Next; }

    public:
        VisitorFeedback Visit(WipColumnVerticalPropertyMap const& propertyMap) const { return _Visit(propertyMap); }
        VisitorFeedback Visit(WipCompoundPropertyMap const& propertyMap) const { return _Visit(propertyMap); }
        VisitorFeedback Visit(WipColumnHorizontalPropertyMap const& propertyMap) const { return _Visit(propertyMap); }
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct ISupportsPropertyMapVisitor
    {
    private:
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  dispatcher) const = 0;
    public:
        VisitorFeedback AcceptVisitor(IPropertyMapVisitor const& dispatcher) const { return _AcceptVisitor(dispatcher); }
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipPropertyMapContainer final : NonCopyableClass, ISupportsPropertyMapVisitor
    {
    typedef std::vector<PropertyMap const*>::const_iterator const_iterator;
    private:
        ClassMap const& m_classMap;
        std::vector<PropertyMap const*> m_directDecendentList; //! contain direct decedents in order.
        std::map<Utf8CP, RefCountedPtr<PropertyMap>, CompareIUtf8Ascii> m_map; //! contain all propertymap owned by the container
        bool m_readonly;

    private:
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  dispatcher)  const override;

    public:
        WipPropertyMapContainer(ClassMap const& classMap)
            :m_classMap(classMap), m_readonly(false)
            {}
        ~WipPropertyMapContainer() {}

        ClassMap const& GetClassMap() const { return m_classMap; }
        ECN::ECClass const& GetClass() const;
        ECDbCR GetECDb() const;
        BentleyStatus Insert(RefCountedPtr<PropertyMap> propertyMap, size_t position = UINT_MAX);
        PropertyMap const* Find(Utf8CP accessString, bool recusive = true) const;
        void MakeReadonly() { m_readonly = true; }
        bool IsReadonly() const { return m_readonly; }
        const_iterator begin() const { return m_directDecendentList.begin(); }
        const_iterator end() const { return m_directDecendentList.end(); }
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Abstract baseclass of all property map.
//+===============+===============+===============+===============+===============+======
struct PropertyMap : RefCountedBase, NonCopyableClass, ISupportsPropertyMapVisitor
    {
    private:
        PropertyMapKind m_kind;
        ECN::ECPropertyCR m_ecProperty;
        Utf8String m_propertyAccessString;
        PropertyMap const* m_parentPropertMap;    
        ClassMap const& m_classMap;
        bool m_isInEditMode;

        virtual BentleyStatus _Validate() const = 0;
        virtual bool _IsMappedToTable(DbTable const&) const = 0;
       
    protected:
        PropertyMap(PropertyMapKind, ClassMap const&, ECN::ECPropertyCR);
        PropertyMap(PropertyMapKind, PropertyMap const&, ECN::ECPropertyCR);
        
    public:
        virtual ~PropertyMap() {}

        //! return kind for this property.
        PropertyMapKind GetKind() const { return m_kind; }

        //! Test for inherited type/
        bool IsKindOf(PropertyMapKind kindOfThisOrOneOfItsParent) const;

        Utf8StringCR GetName() const { return GetProperty().GetName(); }
        ECN::ECPropertyCR GetProperty() const { return m_ecProperty; }
        //! return full access string from root property to current property.
        Utf8StringCR GetAccessString() const { return m_propertyAccessString; }
        //! return parent propertymap if any. 
        PropertyMap const* GetParent() const { return m_parentPropertMap; }
        //! return classmap that owns this property
        ClassMap const& GetClassMap() const { return m_classMap; }
        //! return root propertymap.
        PropertyMap const& GetRoot() const;
        
        //! Test if currrent property is of type system. 
        bool IsSystem() const { return Enum::Contains(PropertyMapKind::System, GetKind()); }
        //! Test if currrent property is of type business. 
        bool IsBusiness () const { return Enum::Contains(PropertyMapKind::Data, GetKind()); }
        
        //! Test if current properyt map mapped to a specific table or not.
        bool IsMappedToTable(DbTable const& table) const { return _IsMappedToTable(table); } //WIP Move to ECSQL
        //! Test if current property map part of classmap tables.
        bool IsMappedToClassMapTables() const; //WIP Move to ECSQL
        
        //! Test if property map is constructed correctly.
        BentleyStatus Validate() const { BeAssert(InEditMode() == false); if (InEditMode()) return ERROR;  return _Validate(); }
   
        //! A property is injected if it does not ECClass but added by ECDb
        bool InEditMode() const { return m_isInEditMode; }
        void FinishEditing() { BeAssert(m_isInEditMode);  m_isInEditMode = false; }
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Abstract baseclass of all property map that are mapped vertically.
// They must have a table and all the hierarchy of propertymap must hold same table.
//+===============+===============+===============+===============+===============+======
struct DataPropertyMap : PropertyMap
    {
    private:
        virtual DbTable const& _GetTable() const = 0;
        virtual bool _IsMappedToTable(DbTable const& table) const override { return &GetTable() == &table; }
    public:
        DataPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::ECPropertyCR ecProperty)
            : PropertyMap(kind, classMap, ecProperty)
            {}
        DataPropertyMap(PropertyMapKind kind, PropertyMap const& parentPropertyMap, ECN::ECPropertyCR ecProperty)
            : PropertyMap(kind, parentPropertyMap, ecProperty)
            {}
        ~DataPropertyMap() {}

        DbTable const& GetTable() const { return _GetTable(); }
        //! create copy of the this property map with new context classmap
        RefCountedPtr<DataPropertyMap> CreateCopy(ClassMap const& newClassMapContext) const;
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Abstract baseclass of all property map that are mapped vertically into multiple tables.
// The main class of properties it represent are system properties.
//+===============+===============+===============+===============+===============+======
struct WipHorizontalPropertyMap : PropertyMap
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
            : PropertyMap(kind, classMap, ecProperty)
            {}
        WipHorizontalPropertyMap(PropertyMapKind kind, PropertyMap const& parentPropertyMap, ECN::ECPropertyCR ecProperty)
            : PropertyMap(kind, parentPropertyMap, ecProperty)
            {}
        virtual  ~WipHorizontalPropertyMap() {}
        virtual std::vector<DbTable const*> const& _GetTables() const = 0;

    public:
        //! Get list of table to which this property map and its children are mapped to. It is never empty.
        std::vector<DbTable const*> const& GetTables() const { return _GetTables(); }
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Abstract baseclass of property map that has child property map. Only vertical propertymaps
// is allowed to contain child property map that must be in same table
//+===============+===============+===============+===============+===============+======
struct WipCompoundPropertyMap : DataPropertyMap
    {
    typedef std::vector<DataPropertyMap const*>::const_iterator const_iterator;
    private:
        struct Collection
            {
            private:
                std::vector<DataPropertyMap const*> m_list;
                std::map<Utf8CP, RefCountedPtr<DataPropertyMap>, CompareIUtf8Ascii> m_map;
            public:
                std::vector<DataPropertyMap const*> const& GetList() const { return m_list; }
                DataPropertyMap const* Find(Utf8CP accessString) const;
                DataPropertyMap const* Front() const { BeAssert(!m_list.empty());  return m_list.empty() ? nullptr : m_list.front(); }
                BentleyStatus Insert(RefCountedPtr<DataPropertyMap> propertyMap, DataPropertyMap const& parent, size_t position);
                BentleyStatus Remove(Utf8CP accessString);
                void Clear() { m_list.clear(); m_map.clear(); }
            } m_col;
        bool m_readonly;
        virtual DbTable const& _GetTable() const override;
        BentleyStatus VerifyVerticalIntegerity(DataPropertyMap const& propertyMap) const;
    protected:
        void MakeWritable() { m_readonly = false; }
        void Clear();
        WipCompoundPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::ECPropertyCR ecProperty)
            : DataPropertyMap(kind, classMap, ecProperty), m_readonly(false)
            {}
        WipCompoundPropertyMap(PropertyMapKind kind, DataPropertyMap const& parentPropertyMap, ECN::ECPropertyCR ecProperty)
            : DataPropertyMap(kind, parentPropertyMap, ecProperty), m_readonly(false)
            {}
        virtual ~WipCompoundPropertyMap() {}
        virtual BentleyStatus _Validate() const override;
        VisitorFeedback AcceptChildren(IPropertyMapVisitor const&  dispatcher) const;
    public:
        bool IsReadonly() const { return m_readonly; }
        void MakeReadOnly() { m_readonly = true; }
        BentleyStatus Insert(RefCountedPtr<DataPropertyMap> propertyMap, size_t position = UINT_MAX);
        BentleyStatus Remove(Utf8CP accessString);
        const_iterator begin() const { return m_col.GetList().begin(); }
        const_iterator end() const { return m_col.GetList().end(); }
        size_t size() const { return m_col.GetList().size(); }
        bool empty() const { return m_col.GetList().empty(); }
        DataPropertyMap const* Find(Utf8CP accessString, bool recusive = true) const;
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Abstract baseclass of all property map that mapped vertically and have single column
//+===============+===============+===============+===============+===============+======
struct WipColumnVerticalPropertyMap: DataPropertyMap
    {
    private:
        DbColumn const& m_column;
        virtual DbTable const& _GetTable() const override
            {
            return m_column.GetTable();
            }
    protected:
        WipColumnVerticalPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::ECPropertyCR ecProperty, DbColumn const& column)
            : DataPropertyMap(kind, classMap, ecProperty), m_column(column)
            {}
        WipColumnVerticalPropertyMap(PropertyMapKind kind, DataPropertyMap const& parentPropertyMap, ECN::ECPropertyCR ecProperty, DbColumn const& column)
            : DataPropertyMap(kind, parentPropertyMap, ecProperty), m_column(column)
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
        static PropertyMapKind ToPropertyMapKind(ECSqlSystemProperty systemProperty);
    };

//==============================Concerte implementations=================================
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipPrimitivePropertyMap final : WipColumnVerticalPropertyMap
    {
    private:
        virtual BentleyStatus _Validate() const override;
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  dispatcher)  const override { return dispatcher.Visit(*this); }
    protected:
        WipPrimitivePropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column)
            : WipColumnVerticalPropertyMap(kind, classMap, ecProperty, column)
            {}
        WipPrimitivePropertyMap(PropertyMapKind kind, DataPropertyMap const& parentPropertyMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column)
            : WipColumnVerticalPropertyMap(kind, parentPropertyMap, ecProperty, column)
            {}
        virtual ~WipPrimitivePropertyMap() {}
    public:
        static RefCountedPtr<WipPrimitivePropertyMap> CreateInstance(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column);
        static RefCountedPtr<WipPrimitivePropertyMap> CreateInstance(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column, PropertyMapKind kind);

        static RefCountedPtr<WipPrimitivePropertyMap> CreateInstance(ECN::PrimitiveECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& column);
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipPrimitiveArrayPropertyMap final : WipColumnVerticalPropertyMap
    {
    private:
        virtual BentleyStatus _Validate() const override { return SUCCESS; }
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  dispatcher)  const override { return dispatcher.Visit(*this); }
    protected:
        WipPrimitiveArrayPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::ArrayECPropertyCR ecProperty, DbColumn const& column)
            : WipColumnVerticalPropertyMap(kind, classMap, ecProperty, column)
            {}
        WipPrimitiveArrayPropertyMap(PropertyMapKind kind, DataPropertyMap const& parentPropertyMap, ECN::ArrayECPropertyCR ecProperty, DbColumn const& column)
            : WipColumnVerticalPropertyMap(kind, parentPropertyMap, ecProperty, column)
            {}
        virtual ~WipPrimitiveArrayPropertyMap() {}
    public:
        static RefCountedPtr<WipPrimitiveArrayPropertyMap> CreateInstance(ClassMap const& classMap, ECN::ArrayECPropertyCR ecProperty, DbColumn const& column);
        static RefCountedPtr<WipPrimitiveArrayPropertyMap> CreateInstance(ECN::ArrayECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& column);
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipStructPropertyMap final : WipCompoundPropertyMap
    {
    private:
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  dispatcher)  const override;

    protected:
        WipStructPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::StructECPropertyCR ecProperty)
            : WipCompoundPropertyMap(kind, classMap, ecProperty)
            {}
        WipStructPropertyMap(PropertyMapKind kind, DataPropertyMap const& parentPropertyMap, ECN::StructECPropertyCR ecProperty)
            : WipCompoundPropertyMap(kind, parentPropertyMap, ecProperty)
            {}
        virtual ~WipStructPropertyMap() {}
    public:
        static RefCountedPtr<WipStructPropertyMap> CreateInstance(ClassMap const& classMap, ECN::StructECPropertyCR ecProperty);
        static RefCountedPtr<WipStructPropertyMap> CreateInstance(ECN::StructECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap);
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipStructArrayPropertyMap final : WipColumnVerticalPropertyMap
    {
    private:
        virtual BentleyStatus _Validate() const override { return SUCCESS; }
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  dispatcher)  const override { return dispatcher.Visit(*this); }

    protected:
        WipStructArrayPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column)
            : WipColumnVerticalPropertyMap(kind, classMap, ecProperty, column)
            {}
        WipStructArrayPropertyMap(PropertyMapKind kind, DataPropertyMap const& parentPropertyMap, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column)
            : WipColumnVerticalPropertyMap(kind, parentPropertyMap, ecProperty, column)
            {}
        virtual ~WipStructArrayPropertyMap() {}
    public:
        static RefCountedPtr<WipStructArrayPropertyMap> CreateInstance(ClassMap const& classMap, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column);
        static RefCountedPtr<WipStructArrayPropertyMap> CreateInstance(ECN::StructArrayECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& column);
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
            virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  dispatcher)  const override { return  dispatcher.Visit(*this); }

        protected:
            RelECClassIdPropertyMap(PropertyMapKind kind, WipNavigationPropertyMap const& parentPropertyMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column, ECN::ECClassId defaultClassId)
                : WipColumnVerticalPropertyMap(kind, parentPropertyMap, ecProperty, column), m_defaultClassId(defaultClassId)
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
            virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  dispatcher)  const override { return  dispatcher.Visit(*this); }

        protected:
            IdPropertyMap(PropertyMapKind kind, WipNavigationPropertyMap const& parentPropertyMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column)
                : WipColumnVerticalPropertyMap(kind, parentPropertyMap, ecProperty, column)
                {}
            virtual ~IdPropertyMap() {}
        public:
            static RefCountedPtr<IdPropertyMap> CreateInstance(WipNavigationPropertyMap const& parentPropertyMap, DbColumn const& column);
        };
    private:
        BentleyStatus Init(DbColumn const& relECClassIdColumn, ECN::ECClassId defaultRelClassId, DbColumn const& idColumn);
        virtual BentleyStatus _Validate() const override;
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  dispatcher)  const override;
    protected:
        WipNavigationPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::NavigationECPropertyCR ecProperty)
            : WipCompoundPropertyMap(kind, classMap, ecProperty)
            {}
        virtual ~WipNavigationPropertyMap() {}
    public:
        BentleyStatus Setup(DbColumn const& relECClassIdColumn, ECN::ECClassId defaultRelClassId, DbColumn const& idColumn);
        RelECClassIdPropertyMap const& GetRelECClassId() const { return static_cast<RelECClassIdPropertyMap const&>(*Find(ECDbSystemSchemaHelper::RELECCLASSID_PROPNAME)); }
        IdPropertyMap const& GetId() const { return static_cast<IdPropertyMap const&>(*Find(ECDbSystemSchemaHelper::ID_PROPNAME)); };
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
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  dispatcher)  const override;
    protected:
        WipPoint2dPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty)
            : WipCompoundPropertyMap(kind, classMap, ecProperty)
            {}

        WipPoint2dPropertyMap(PropertyMapKind kind, DataPropertyMap const& parentPropertyMap, ECN::PrimitiveECPropertyCR ecProperty)
            : WipCompoundPropertyMap(kind, parentPropertyMap, ecProperty)
            {}

        virtual ~WipPoint2dPropertyMap() {}
    public:
        WipPrimitivePropertyMap const& GetX() const { return static_cast<WipPrimitivePropertyMap const&>(*Find(ECDbSystemSchemaHelper::X_PROPNAME)); }
        WipPrimitivePropertyMap const& GetY() const { return static_cast<WipPrimitivePropertyMap const&>(*Find(ECDbSystemSchemaHelper::Y_PROPNAME)); }
        static RefCountedPtr<WipPoint2dPropertyMap> CreateInstance(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y);
        static RefCountedPtr<WipPoint2dPropertyMap> CreateInstance(ECN::PrimitiveECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& x, DbColumn const& y);
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipPoint3dPropertyMap final : WipCompoundPropertyMap
    {
    private:
        BentleyStatus Init(DbColumn const& x, DbColumn const& y, DbColumn const& z);
        virtual BentleyStatus _Validate() const override;
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  dispatcher)  const override;

    protected:
        WipPoint3dPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty)
            : WipCompoundPropertyMap(kind, classMap, ecProperty)
            {
            }
        WipPoint3dPropertyMap(PropertyMapKind kind, DataPropertyMap const& parentPropertyMap, ECN::PrimitiveECPropertyCR ecProperty)
            : WipCompoundPropertyMap(kind, parentPropertyMap, ecProperty)
            {
            }
        virtual ~WipPoint3dPropertyMap() {}
    public:
        WipPrimitivePropertyMap const& GetX() const { return static_cast<WipPrimitivePropertyMap const&>(*Find(ECDbSystemSchemaHelper::X_PROPNAME)); }
        WipPrimitivePropertyMap const& GetY() const { return static_cast<WipPrimitivePropertyMap const&>(*Find(ECDbSystemSchemaHelper::Y_PROPNAME)); }
        WipPrimitivePropertyMap const& GetZ() const { return static_cast<WipPrimitivePropertyMap const&>(*Find(ECDbSystemSchemaHelper::Z_PROPNAME)); }
        static RefCountedPtr<WipPoint3dPropertyMap> CreateInstance(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y, DbColumn const& z);
        static RefCountedPtr<WipPoint3dPropertyMap> CreateInstance(ECN::PrimitiveECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& x, DbColumn const& y, DbColumn const& z);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipECInstanceIdPropertyMap final : WipSystemPropertyMap
    {
    private:
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const& dispatcher)  const override { return dispatcher.Visit(*this); }

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
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  dispatcher)  const override { return dispatcher.Visit(*this); }

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
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  dispatcher)  const override { return dispatcher.Visit(*this); }

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
struct WipConstraintECInstanceIdPropertyMap final : WipSystemPropertyMap
    {
    enum class ConstraintType
        {
        Source, Target
        };
    private:
        ConstraintType m_constraintType;
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  dispatcher)  const override { return dispatcher.Visit(*this); }

    protected:
        WipConstraintECInstanceIdPropertyMap(PropertyMapKind kind, ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, std::vector<RefCountedPtr<WipPrimitivePropertyMap>> const& maps, ConstraintType constraintType)
            : WipSystemPropertyMap(kind, classMap, ecProperty, maps)
            {}
        virtual ~WipConstraintECInstanceIdPropertyMap() {}
    public:
        bool IsSource() const { return m_constraintType == ConstraintType::Source; }
        bool IsTarget() const { return m_constraintType == ConstraintType::Target; }
        static RefCountedPtr<WipConstraintECInstanceIdPropertyMap> CreateInstance(ClassMap const& classMap, ConstraintType constraintType, std::vector<DbColumn const*> const& columns);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct WipPropertyMapFactory final
    {
    private:
        WipPropertyMapFactory(){}
        static RefCountedPtr<DataPropertyMap> CreateCopy(DataPropertyMap const& propertyMap, ClassMap const& newContext, DataPropertyMap const* newParent);

    public:
        //! Data Property Maps
        static RefCountedPtr<WipPrimitivePropertyMap> CreatePrimitivePropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column);
        static RefCountedPtr<WipPrimitivePropertyMap> CreatePrimitivePropertyMap(ECN::PrimitiveECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& column);
        static RefCountedPtr<WipPrimitiveArrayPropertyMap> CreatePrimitiveArrayPropertyMap(ClassMap const& classMap, ECN::ArrayECPropertyCR ecProperty, DbColumn const& column);
        static RefCountedPtr<WipPrimitiveArrayPropertyMap> CreatePrimitiveArrayPropertyMap(ECN::ArrayECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& column);
        static RefCountedPtr<WipStructPropertyMap> CreateStructPropertyMap(ClassMap const& classMap, ECN::StructECPropertyCR ecProperty);
        static RefCountedPtr<WipStructPropertyMap> CreateStructPropertyMap(ECN::StructECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap);
        static RefCountedPtr<WipStructArrayPropertyMap> CreateStructArrayPropertyMap(ClassMap const& classMap, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column);
        static RefCountedPtr<WipStructArrayPropertyMap> CreateStructArrayPropertyMap(ECN::StructArrayECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& column);
        static RefCountedPtr<WipPoint2dPropertyMap> CreatePoint2dPropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y);
        static RefCountedPtr<WipPoint2dPropertyMap> CreatePoint2dPropertyMap(ECN::PrimitiveECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& x, DbColumn const& y);
        static RefCountedPtr<WipPoint3dPropertyMap> CreatePoint3dPropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y, DbColumn const& z);
        static RefCountedPtr<WipPoint3dPropertyMap> CreatePoint3dPropertyMap(ECN::PrimitiveECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& x, DbColumn const& y, DbColumn const& z);
        static RefCountedPtr<WipNavigationPropertyMap> CreateNavigationPropertyMap(ClassMap const& classMap, ECN::NavigationECPropertyCR ecProperty);
        static RefCountedPtr<WipECInstanceIdPropertyMap> CreateECInstanceIdPropertyMap(ClassMap const& classMap, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<WipECClassIdPropertyMap> CreateECClassIdPropertyMap(ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<WipConstraintECClassIdPropertyMap> CreateSourceECClassIdPropertyMap(ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<WipConstraintECClassIdPropertyMap> CreateTargetECClassIdPropertyMap(ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<WipConstraintECInstanceIdPropertyMap> CreateSourceECInstanceIdPropertyMap(ClassMap const& classMap, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<WipConstraintECInstanceIdPropertyMap> CreateTargetECInstanceIdPropertyMap(ClassMap const& classMap, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<DataPropertyMap> CreateCopy(DataPropertyMap const& propertyMap, ClassMap const& newContext);
        static RefCountedPtr<WipHorizontalPropertyMap> CreateCopy(WipHorizontalPropertyMap const& propertyMap, ClassMap const& newContext);
        static RefCountedPtr<WipConstraintECInstanceIdPropertyMap> CreateConstraintECInstanceIdPropertyMap(ECN::ECRelationshipEnd end, ClassMap const& classMap, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<WipConstraintECClassIdPropertyMap> CreateConstraintECClassIdPropertyMap(ECN::ECRelationshipEnd end, ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Allow to collect columns from property maps
//+===============+===============+===============+===============+===============+======
struct WipPropertyMapColumnDispatcher final: IPropertyMapVisitor
    {
    private:
        mutable std::vector<DbColumn const*> m_columns;
        DbTable const* m_table;
        PropertyMapKind m_filter;
        bool m_doNotSkipHorizontalPropertyMaps;
    private:

        virtual VisitorFeedback _Visit(WipColumnVerticalPropertyMap const& propertyMap) const override;
        virtual VisitorFeedback _Visit(WipCompoundPropertyMap const& propertyMap) const override;
        virtual VisitorFeedback _Visit(WipColumnHorizontalPropertyMap const& propertyMap) const override;

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
        bool AreResultingColumnsAreVirtual() const 
            {
            BeAssert(!GetColumns().empty());
            bool isVirtual = true;
            for (DbColumn const* column : GetColumns())
                {
                isVirtual &= column->GetPersistenceType() == PersistenceType::Virtual;
                if (!isVirtual)
                    break;
                }

            return isVirtual;
            }
        DbColumn const* GetSingleColumn() const 
            { 
            BeAssert(GetColumns().size() == 1); 
            if (GetColumns().size() != 1)
                {
                return nullptr;
                }

            return GetColumns().front();
            }
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Allow to collect columns from property maps
//+===============+===============+===============+===============+===============+======
struct WipPropertyMapTableDispatcher final : IPropertyMapVisitor
    {
    private:
        mutable std::set<DbTable const*> m_tables;
        PropertyMapKind m_filter;
    private:

        virtual VisitorFeedback _Visit(WipColumnVerticalPropertyMap const& propertyMap) const override
            {
            if (Enum::Contains(m_filter, propertyMap.GetKind()))
                m_tables.insert(&propertyMap.GetTable());

            return VisitorFeedback::Cancel;
            }
        virtual VisitorFeedback _Visit(WipCompoundPropertyMap const& propertyMap) const override
            {
            if (Enum::Contains(m_filter, propertyMap.GetKind()))
                m_tables.insert(&propertyMap.GetTable());

            return VisitorFeedback::NextSibling;
            }
        virtual VisitorFeedback _Visit(WipColumnHorizontalPropertyMap const& propertyMap) const override
            {
            if (Enum::Contains(m_filter, propertyMap.GetKind()))
                m_tables.insert(propertyMap.GetTables().begin(), propertyMap.GetTables().end());

            return VisitorFeedback::Cancel;
            }

    public:
        WipPropertyMapTableDispatcher(PropertyMapKind filter = PropertyMapKind::All)
            : m_filter(filter)
            {}
        std::set<DbTable const*> GetTables() const { return m_tables; }
        DbTable const* GetSingleTable() const 
            { 
            BeAssert(!m_tables.empty()); 
            if (m_tables.size() != 1) 
                return nullptr; 

            return *(m_tables.begin()); 
            }
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Search PropertyMap with a given type
//+===============+===============+===============+===============+===============+======
struct WipPropertyMapTypeDispatcher final : IPropertyMapVisitor
    {

    private:
        mutable std::vector<PropertyMap const*> m_propertyMaps;
        PropertyMapKind m_filter;
        bool m_traverseCompoundProperties;
    private:

        virtual VisitorFeedback _Visit(WipColumnVerticalPropertyMap const& propertyMap) const override
            {
            if (Enum::Contains(m_filter, propertyMap.GetKind()))
                m_propertyMaps.push_back(&propertyMap);

            return VisitorFeedback::Next;
            }
        virtual VisitorFeedback _Visit(WipCompoundPropertyMap const& propertyMap) const override
            {
            if (Enum::Contains(m_filter, propertyMap.GetKind()))
                {
                if (m_traverseCompoundProperties)
                    return VisitorFeedback::Next;

                m_propertyMaps.push_back(&propertyMap);
                return VisitorFeedback::NextSibling;
                }

            return VisitorFeedback::Next;
            }
        virtual VisitorFeedback _Visit(WipColumnHorizontalPropertyMap const& propertyMap) const override
            {
            if (Enum::Contains(m_filter, propertyMap.GetKind()))
                m_propertyMaps.push_back(&propertyMap);

            return VisitorFeedback::Next;
            }

    public:
        WipPropertyMapTypeDispatcher(PropertyMapKind filter = PropertyMapKind::All, bool traverseCompoundProperties = false)
            :m_filter(filter), m_traverseCompoundProperties(traverseCompoundProperties)
            {}
        ~WipPropertyMapTypeDispatcher() {}
        void Reset() { m_propertyMaps.clear(); }
        std::vector<PropertyMap const*> const& ResultSet() const { return m_propertyMaps; }
        static std::vector<PropertyMap const*> Accept(PropertyMap const& propertyMap, PropertyMapKind filter = PropertyMapKind::All, bool traverseCompoundProperties = false)
            {
            WipPropertyMapTypeDispatcher typeDispatcher(filter, traverseCompoundProperties);
            propertyMap.AcceptVisitor(typeDispatcher);
            return typeDispatcher.ResultSet();
            }
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Allow to collect columns from property maps
//+===============+===============+===============+===============+===============+======
struct WipPropertyMapSqlDispatcher final : IPropertyMapVisitor
    {
    enum SqlTarget
        {
        View, //!Inline view is in from. Normally it happen only in SELECT statement where view has a contract.
        Table, //!Direct query against a table
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
        bool m_wrapInParentheses;
        bool m_usePropertyNameAsAliasForSystemPropertyMaps;
    private:
        Result& Record(WipColumnVerticalPropertyMap const& propertyMap) const;
        bool IsAlienTable(DbTable const& table) const;
        DbTable const* RequiresJoinTo(WipConstraintECClassIdPropertyMap const& propertyMap) const;
        Utf8CP GetECClassIdPrimaryTableAlias(WipConstraintECClassIdPropertyMap const& propertyMap) const;

        WipColumnVerticalPropertyMap const* FindSystemPropertyMapForTable(WipSystemPropertyMap const& systemPropertyMap) const;
        VisitorFeedback ToNativeSql(WipColumnVerticalPropertyMap const& propertyMap) const;
        VisitorFeedback ToNativeSql(WipConstraintECInstanceIdPropertyMap const& propertyMap) const;
        VisitorFeedback ToNativeSql(WipConstraintECClassIdPropertyMap const& propertyMap) const;
        VisitorFeedback ToNativeSql(WipECClassIdPropertyMap const& propertyMap) const;
        VisitorFeedback ToNativeSql(WipECInstanceIdPropertyMap const& propertyMap) const;

    private:
        virtual VisitorFeedback _Visit(WipColumnVerticalPropertyMap const& propertyMap) const override;
        virtual VisitorFeedback _Visit(WipCompoundPropertyMap const& propertyMap) const override;
        virtual VisitorFeedback _Visit(WipColumnHorizontalPropertyMap const& propertyMap) const override;

    public:
        WipPropertyMapSqlDispatcher(DbTable const& tableFilter, SqlTarget target, Utf8CP classIdentifier, bool wrapInParentheses = false, bool usePropertyNameAsAliasForSystemPropertyMaps = false)
            :m_tableFilter(tableFilter), m_target(target), m_classIdentifier(classIdentifier), m_wrapInParentheses(wrapInParentheses), m_status(SUCCESS), m_usePropertyNameAsAliasForSystemPropertyMaps(usePropertyNameAsAliasForSystemPropertyMaps)
            {
            if (m_usePropertyNameAsAliasForSystemPropertyMaps)
                {
                BeAssert(target == SqlTarget::Table);
                BeAssert(wrapInParentheses == false);
                }
            }

        ~WipPropertyMapSqlDispatcher() {}

        BentleyStatus GetStatus() const { return m_status; }
        std::vector<Result> const& GetResultSet() const { return m_resultSet; }
        const Result* Find(Utf8CP accessString) const;
        NativeSqlBuilder::List ToList() const 
            {
            NativeSqlBuilder::List list;
            for (Result const& r : m_resultSet)
                list.push_back(r.GetSqlBuilder());

            return list;
            }
        void Reset() const { m_resultSetByAccessString.clear(); m_resultSet.clear(); m_status = SUCCESS; }
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
