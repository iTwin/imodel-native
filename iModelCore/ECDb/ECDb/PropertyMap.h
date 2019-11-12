/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "DbSchema.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct ClassMap;
struct PropertyMap;
struct CompoundDataPropertyMap;
struct SingleColumnDataPropertyMap;
struct SystemPropertyMap;

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct IPropertyMapVisitor
    {
    private:
        virtual BentleyStatus _Visit(SingleColumnDataPropertyMap const& propertyMap) const { return SUCCESS; }
        virtual BentleyStatus _Visit(SystemPropertyMap const& propertyMap) const { return SUCCESS; }

    protected:
        virtual BentleyStatus _Visit(CompoundDataPropertyMap const& propertyMap) const;

    public:
        virtual ~IPropertyMapVisitor() {}

        BentleyStatus Visit(SingleColumnDataPropertyMap const& propertyMap) const { return _Visit(propertyMap); }
        BentleyStatus Visit(CompoundDataPropertyMap const& propertyMap) const { return _Visit(propertyMap); }
        BentleyStatus Visit(SystemPropertyMap const& propertyMap) const { return _Visit(propertyMap); }
    };


//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct ISupportsPropertyMapVisitor
    {
    private:
        virtual BentleyStatus _AcceptVisitor(IPropertyMapVisitor const&  visitor) const = 0;
    public:
        BentleyStatus AcceptVisitor(IPropertyMapVisitor const& visitor) const { return _AcceptVisitor(visitor); }
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct PropertyMapContainer final : ISupportsPropertyMapVisitor
    {
    typedef std::vector<PropertyMap const*>::const_iterator const_iterator;
    private:
        ClassMap const& m_classMap;
        std::vector<PropertyMap const*> m_topLevelPropMapsOrdered;
        std::map<Utf8CP, RefCountedPtr<PropertyMap>, CompareIUtf8Ascii> m_byAccessString;

        //not copyable
        PropertyMapContainer(PropertyMapContainer const&) = delete;
        PropertyMapContainer& operator=(PropertyMapContainer const&) = delete;

        BentleyStatus _AcceptVisitor(IPropertyMapVisitor const&)  const override;

    public:
        explicit PropertyMapContainer(ClassMap const& classMap) :m_classMap(classMap) {}
        ~PropertyMapContainer() {}

        BentleyStatus Insert(RefCountedPtr<PropertyMap> propertyMap, size_t position = std::numeric_limits<size_t>::max());
        PropertyMap const* Find(Utf8CP accessString) const;
        size_t Size() const { return m_topLevelPropMapsOrdered.size(); }
        const_iterator begin() const { return m_topLevelPropMapsOrdered.begin(); }
        const_iterator end() const { return m_topLevelPropMapsOrdered.end(); }
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Abstract baseclass of all property map.
//+===============+===============+===============+===============+===============+======
struct PropertyMap : RefCountedBase, ISupportsPropertyMapVisitor
    {
    struct Path
        {
        typedef bvector<PropertyMap const*>::const_iterator const_iterator;
        private:
            bvector<PropertyMap const*> m_vect;
            explicit Path(bvector<PropertyMap const*>& vect):m_vect(std::move(vect)){}
        public:
            Path(Path const&& path) : m_vect(std::move(path.m_vect)) {}
            Path(Path const& path) :m_vect(path.m_vect) {}
            Path& operator = (Path const& path);
            Path& operator = (Path const&& path);
            PropertyMap const& operator [] (size_t i) const;
            size_t size() const { return m_vect.size(); }
            const_iterator begin() const { return m_vect.begin(); }
            const_iterator end() const { return m_vect.end(); }
            PropertyMap const& Front() const { return *m_vect.front(); }
            PropertyMap const& Back() const { return *m_vect.back(); }
            static Path From(PropertyMap const& propertyMap);
        };
    public:
        enum class Type
            {
            ConstraintECClassId = 1,
            ConstraintECInstanceId = 2,
            ECClassId = 4,
            ECInstanceId = 8,

            Navigation = 16,
            NavigationId = 32,
            NavigationRelECClassId = 64,

            Point2d = 128,
            Point3d = 256,
            Primitive = 512,
            PrimitiveArray = 1024,
            Struct = 2048,
            StructArray = 4096,
            SystemPerTableClassId = 8192,
            SystemPerTableId = 16384,

            System = ECInstanceId | ECClassId | ConstraintECClassId | ConstraintECInstanceId,
            SingleColumnData = Primitive | PrimitiveArray | StructArray | NavigationRelECClassId | NavigationId | SystemPerTableId | SystemPerTableClassId,
            CompoundData = Navigation | Point2d | Point3d | Struct,
            Data = SingleColumnData | CompoundData,
            All = System | Data
            };
    private:
        Type m_type;
        ECN::ECPropertyCR m_ecProperty;
        PropertyMap const* m_parentPropertMap = nullptr;
        ClassMap const& m_classMap;
        const Utf8String m_propertyAccessString;

        //not copyable
        PropertyMap(PropertyMap const&) = delete;
        PropertyMap& operator=(PropertyMap const&) = delete;

        virtual bool _IsMappedToTable(DbTable const&) const = 0;

    protected:

        PropertyMap(Type kind, ClassMap const& classMap, ECN::ECPropertyCR ecProperty)
            : m_type(kind), m_classMap(classMap), m_ecProperty(ecProperty), m_parentPropertMap(nullptr), m_propertyAccessString(ecProperty.GetName()) {}

        PropertyMap(Type type, PropertyMap const& parentPropertyMap, ECN::ECPropertyCR ecProperty, Utf8StringCR accessString)
            :m_type(type), m_classMap(parentPropertyMap.GetClassMap()), m_ecProperty(ecProperty), m_parentPropertMap(&parentPropertyMap),
            m_propertyAccessString(accessString)
            {}

    public:
        virtual ~PropertyMap() {}

        Type GetType() const { return m_type; }
        
        template <typename TPropertyMap>
        TPropertyMap const& GetAs() const { BeAssert(dynamic_cast<TPropertyMap const*> (this) != nullptr); return *static_cast<TPropertyMap const*> (this); }
        
        Utf8StringCR GetName() const { return GetProperty().GetName(); }
        ECN::ECPropertyCR GetProperty() const { return m_ecProperty; }
        ECN::ECPropertyId GetRootPropertyId() const;
        //! return full access string from root property to current property.
        Utf8StringCR GetAccessString() const { return m_propertyAccessString; }
        //! return parent property map if any. 
        PropertyMap const* GetParent() const { return m_parentPropertMap; }
        //! return class map that owns this property
        ClassMap const& GetClassMap() const { return m_classMap; }
        
        //! Test if current property is of type system. 
        bool IsSystem() const { return Enum::Contains(Type::System, GetType()); }
        //! Test if current property is of type business. 
        bool IsData() const { return Enum::Contains(Type::Data, GetType()); }
        
        //! Test if current property map mapped to a specific table or not.
        bool IsMappedToTable(DbTable const& table) const { return _IsMappedToTable(table); } //WIP Move to ECSQL
        //! Test if current property map part of class map tables.
        bool IsMappedToClassMapTables() const; //WIP Move to ECSQL
        Path GetPath() const { return Path::From(*this); }
    };

ENUM_IS_FLAGS(PropertyMap::Type);

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Abstract baseclass of all business property maps
// They must have a table and all the hierarchy of property map must hold same table.
//+===============+===============+===============+===============+===============+======
struct DataPropertyMap : PropertyMap
    {
    private:
        virtual DbTable const& _GetTable() const = 0;
        bool _IsMappedToTable(DbTable const& table) const override { return &GetTable() == &table; }

    protected:
        DataPropertyMap(Type kind, ClassMap const& classMap, ECN::ECPropertyCR ecProperty) : PropertyMap(kind, classMap, ecProperty) {}
        DataPropertyMap(Type kind, PropertyMap const& parentPropertyMap, ECN::ECPropertyCR ecProperty, bool appendToAccessString)
            : PropertyMap(kind, parentPropertyMap, ecProperty, appendToAccessString ? 
                          Utf8PrintfString("%s.%s", parentPropertyMap.GetAccessString().c_str(), ecProperty.GetName().c_str()) : 
                          ecProperty.GetName())
            {}

    public:
        virtual ~DataPropertyMap() {}
        DbTable const& GetTable() const { return _GetTable(); }
        //! create copy of the this property map with new context classmap
        RefCountedPtr<DataPropertyMap> CreateCopy(ClassMap const& newClassMapContext) const;
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Abstract baseclass of property map that has child property map. Only property maps
// is allowed to contain child property map that must be in same table
//+===============+===============+===============+===============+===============+======
struct CompoundDataPropertyMap : DataPropertyMap
    {
    typedef bvector<DataPropertyMap const*>::const_iterator const_iterator;
 
    private:
        DbTable const& _GetTable() const override;

    protected:
        bvector<DataPropertyMap const*> m_memberPropertyMaps;

        CompoundDataPropertyMap(Type kind, ClassMap const& classMap, ECN::ECPropertyCR ecProperty)
            : DataPropertyMap(kind, classMap, ecProperty) {}
        CompoundDataPropertyMap(Type kind, CompoundDataPropertyMap const& parentPropertyMap, ECN::ECPropertyCR ecProperty)
            : DataPropertyMap(kind, parentPropertyMap, ecProperty, true) {}

        BentleyStatus _AcceptVisitor(IPropertyMapVisitor const& visitor)  const override { return visitor.Visit(*this); }

        BentleyStatus InsertMember(RefCountedPtr<DataPropertyMap> propertyMap);

    public:
        virtual ~CompoundDataPropertyMap() {}

        DataPropertyMap const* Find(Utf8CP accessString) const;
        const_iterator begin() const { return m_memberPropertyMaps.begin(); }
        const_iterator end() const { return m_memberPropertyMaps.end(); }
        size_t Size() const { return m_memberPropertyMaps.size(); }
        bool IsEmpty() const { return m_memberPropertyMaps.empty(); }
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct SingleColumnDataPropertyMap : DataPropertyMap
    {
    private:
        DbColumn const& m_column;
        DbTable const& _GetTable() const override { return m_column.GetTable(); }
        BentleyStatus _AcceptVisitor(IPropertyMapVisitor const& visitor)  const override { return visitor.Visit(*this); }
        virtual DbColumn::Type _GetColumnDataType() const = 0;

    protected:
        SingleColumnDataPropertyMap(Type kind, ClassMap const& classMap, ECN::ECPropertyCR ecProperty, DbColumn const& column)
            : DataPropertyMap(kind, classMap, ecProperty), m_column(column)
            {}
        SingleColumnDataPropertyMap(Type kind, PropertyMap const& parentPropertyMap, ECN::ECPropertyCR ecProperty, DbColumn const& column, bool appendToAccessString)
            : DataPropertyMap(kind, parentPropertyMap, ecProperty, appendToAccessString), m_column(column)
            {}
    public:       
        virtual ~SingleColumnDataPropertyMap() {}
        DbColumn const& GetColumn() const { return m_column; }
        DbColumn::Type GetColumnDataType() const { return _GetColumnDataType(); }
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct PrimitivePropertyMap final : SingleColumnDataPropertyMap
    {
    private:
        PrimitivePropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column)
            : SingleColumnDataPropertyMap(Type::Primitive, classMap, ecProperty, column)
            {}

        PrimitivePropertyMap(CompoundDataPropertyMap const& parentPropMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column)
            : SingleColumnDataPropertyMap(Type::Primitive, parentPropMap, ecProperty, column, true)
            {}

        DbColumn::Type _GetColumnDataType() const override { return DetermineColumnDataType(GetProperty().GetAsPrimitiveProperty()->GetType()); }

    public:
        ~PrimitivePropertyMap() {}
        static RefCountedPtr<PrimitivePropertyMap> CreateInstance(ClassMap const& classMap, CompoundDataPropertyMap const* parentPropMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column);

        static DbColumn::Type DetermineColumnDataType(ECN::PrimitiveType);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct StructPropertyMap final : CompoundDataPropertyMap
    {
    friend struct StructPropertyMapBuilder;
    private:
        StructPropertyMap(ClassMap const& classMap, ECN::StructECPropertyCR ecProperty) : CompoundDataPropertyMap(Type::Struct, classMap, ecProperty) {}
        StructPropertyMap(CompoundDataPropertyMap const& parentPropMap, ECN::StructECPropertyCR ecProperty) : CompoundDataPropertyMap(Type::Struct, parentPropMap, ecProperty) {}

        static RefCountedPtr<StructPropertyMap> CreateInstance(ClassMap const& classMap, CompoundDataPropertyMap const* parentPropMap, ECN::StructECPropertyCR ecProperty);

    public:
        ~StructPropertyMap() {}

    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct PrimitiveArrayPropertyMap final : SingleColumnDataPropertyMap
    {
    public:
        static const DbColumn::Type COLUMN_DATATYPE = DbColumn::Type::Text;

    private:
        PrimitiveArrayPropertyMap(ClassMap const& classMap, ECN::PrimitiveArrayECPropertyCR ecProperty, DbColumn const& column) : SingleColumnDataPropertyMap(Type::PrimitiveArray, classMap, ecProperty, column) {}
        PrimitiveArrayPropertyMap(CompoundDataPropertyMap const& parentPropMap, ECN::PrimitiveArrayECPropertyCR ecProperty, DbColumn const& column) : SingleColumnDataPropertyMap(Type::PrimitiveArray, parentPropMap, ecProperty, column, true) {}

        DbColumn::Type _GetColumnDataType() const override { return COLUMN_DATATYPE; }

    public:
        ~PrimitiveArrayPropertyMap() {}

        static RefCountedPtr<PrimitiveArrayPropertyMap> CreateInstance(ClassMap const& classMap, CompoundDataPropertyMap const* parentPropMap, ECN::PrimitiveArrayECPropertyCR ecProperty, DbColumn const& column);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct StructArrayPropertyMap final : SingleColumnDataPropertyMap
    {
    public:
        static const DbColumn::Type COLUMN_DATATYPE = DbColumn::Type::Text;

    private:
        StructArrayPropertyMap(ClassMap const& classMap, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column) : SingleColumnDataPropertyMap(Type::StructArray, classMap, ecProperty, column) {}
        StructArrayPropertyMap(CompoundDataPropertyMap const& parentPropMap, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column) : SingleColumnDataPropertyMap(Type::StructArray, parentPropMap, ecProperty, column, true) {}

        DbColumn::Type _GetColumnDataType() const override { return COLUMN_DATATYPE; }

    public:
        ~StructArrayPropertyMap() {}
        static RefCountedPtr<StructArrayPropertyMap> CreateInstance(ClassMap const& classMap, CompoundDataPropertyMap const* parentPropMap, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct Point2dPropertyMap final : CompoundDataPropertyMap
    {
    public:
        static const DbColumn::Type COORDINATE_COLUMN_DATATYPE = DbColumn::Type::Real;

    private:
        Point2dPropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty) : CompoundDataPropertyMap(PropertyMap::Type::Point2d, classMap, ecProperty) {}
        Point2dPropertyMap(CompoundDataPropertyMap const& parentPropMap, ECN::PrimitiveECPropertyCR ecProperty) : CompoundDataPropertyMap(PropertyMap::Type::Point2d, parentPropMap, ecProperty) {}

        BentleyStatus Init(DbColumn const& x, DbColumn const& y);

    public:
        static RefCountedPtr<Point2dPropertyMap> CreateInstance(ClassMap const& classMap, CompoundDataPropertyMap const* parentPropMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y);
        ~Point2dPropertyMap() {}

        PrimitivePropertyMap const& GetX() const;
        PrimitivePropertyMap const& GetY() const;
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct Point3dPropertyMap final : CompoundDataPropertyMap
    {
    public:
        static const DbColumn::Type COORDINATE_COLUMN_DATATYPE = DbColumn::Type::Real;

    private:
        Point3dPropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty)
            : CompoundDataPropertyMap(PropertyMap::Type::Point3d, classMap, ecProperty)
            {}
        Point3dPropertyMap(CompoundDataPropertyMap const& parentPropMap, ECN::PrimitiveECPropertyCR ecProperty)
            : CompoundDataPropertyMap(PropertyMap::Type::Point3d, parentPropMap, ecProperty)
            {}

        BentleyStatus Init(DbColumn const& x, DbColumn const& y, DbColumn const& z);

    public:
        static RefCountedPtr<Point3dPropertyMap> CreateInstance(ClassMap const& classMap, CompoundDataPropertyMap const* parentPropMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y, DbColumn const& z);
        ~Point3dPropertyMap() {}

        PrimitivePropertyMap const& GetX() const;
        PrimitivePropertyMap const& GetY() const;
        PrimitivePropertyMap const& GetZ() const;
    };


//=======================================================================================
// @bsiclass                                                  Krischan.Eberle       10/16
//+===============+===============+===============+===============+===============+======
struct StructPropertyMapBuilder final
    {
    private:
        RefCountedPtr<StructPropertyMap> m_propMap;
        bool m_isFinished;

        //not copyable
        StructPropertyMapBuilder(StructPropertyMapBuilder const&) = delete;
        StructPropertyMapBuilder& operator=(StructPropertyMapBuilder const&) = delete;

    public:
        StructPropertyMapBuilder(ClassMap const& classMap, CompoundDataPropertyMap const* parentPropMap, ECN::StructECPropertyCR prop);
        bool IsValid() const { return m_propMap != nullptr; }

        BentleyStatus AddMember(RefCountedPtr<DataPropertyMap> propertyMap);

        bool IsFinished() const { return m_isFinished; }
        StructPropertyMap const& GetPropertyMapUnderConstruction() { BeAssert(IsValid()); return *m_propMap; }

        BentleyStatus Finish();
        RefCountedPtr<StructPropertyMap> GetPropertyMap() { BeAssert(m_isFinished); return m_propMap; }

    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct NavigationPropertyMap final : CompoundDataPropertyMap
    {
    enum class NavigationEnd
        {
        From,
        To
        };

    struct IdPropertyMap final : SingleColumnDataPropertyMap
        {
        friend NavigationPropertyMap;
        private:
            IdPropertyMap(NavigationPropertyMap const& parentPropertyMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column)
                : SingleColumnDataPropertyMap(Type::NavigationId, parentPropertyMap, ecProperty, column, true)
                {}

            DbColumn::Type _GetColumnDataType() const override { return DbColumn::Type::Integer; }

            static RefCountedPtr<IdPropertyMap> CreateInstance(NavigationPropertyMap const& parentPropertyMap, DbColumn const& column);

        public:
            ~IdPropertyMap() {}
        };

    struct RelECClassIdPropertyMap final : SingleColumnDataPropertyMap
        {
        friend NavigationPropertyMap;
        private:
            ECN::ECClassId m_defaultClassId;

            RelECClassIdPropertyMap(NavigationPropertyMap const& parentPropertyMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column, ECN::ECClassId defaultClassId)
                : SingleColumnDataPropertyMap(Type::NavigationRelECClassId, parentPropertyMap, ecProperty, column, true), m_defaultClassId(defaultClassId)
                {}

            DbColumn::Type _GetColumnDataType() const override { return DbColumn::Type::Integer; }

            static RefCountedPtr<RelECClassIdPropertyMap> CreateInstance(NavigationPropertyMap const& parentPropertyMap, DbColumn const& column, ECN::ECClassId defaultRelClassId);

        public:
            ~RelECClassIdPropertyMap() {}
            ECN::ECClassId GetDefaultClassId() const { return m_defaultClassId; }
        };

    private:
        bool m_isComplete;

        NavigationPropertyMap(ClassMap const& classMap, ECN::NavigationECPropertyCR ecProperty)
            : CompoundDataPropertyMap(Type::Navigation, classMap, ecProperty), m_isComplete(false)
            {}

    public:
        static RefCountedPtr<NavigationPropertyMap> CreateInstance(ClassMap const& classMap, ECN::NavigationECPropertyCR ecProperty) { return new NavigationPropertyMap(classMap, ecProperty); }
        ~NavigationPropertyMap() {}

        //returns true if the prop map is fully set-up and can be used. If false, it is still under construction
        bool IsComplete() const { return m_isComplete; }
        IdPropertyMap const& GetIdPropertyMap() const;
        RelECClassIdPropertyMap const& GetRelECClassIdPropertyMap() const;
        bool HasForeignKeyConstraint() const { return GetProperty().IsDefinedLocal("ECDbMap", "ForeignKeyConstraint"); }
        BentleyStatus SetMembers(DbColumn const& idColumn, DbColumn const& relECClassIdColumn, ECN::ECClassId defaultRelClassId);

        bool CardinalityImpliesNotNull() const { return GetRelationshipConstraint(NavigationPropertyMap::NavigationEnd::To).GetMultiplicity().GetLowerLimit() > 0; }
        bool CardinalityImpliesUnique() const { return GetRelationshipConstraint(NavigationPropertyMap::NavigationEnd::From).GetMultiplicity().GetUpperLimit() <= 1; }

        ECN::ECRelationshipConstraintCR GetRelationshipConstraint(NavigationPropertyMap::NavigationEnd navEnd) const;
        static ECN::ECRelationshipEnd GetRelationshipEnd(ECN::NavigationECPropertyCR, NavigationPropertyMap::NavigationEnd);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct PropertyMapCopier final
    {
    private:
        PropertyMapCopier();
        ~PropertyMapCopier();

        static RefCountedPtr<DataPropertyMap> CreateCopy(DataPropertyMap const&, ClassMap const& newContext, CompoundDataPropertyMap const* parentPropMap);

    public:
        static RefCountedPtr<DataPropertyMap> CreateCopy(DataPropertyMap const&, ClassMap const& newContext);
        static RefCountedPtr<SystemPropertyMap> CreateCopy(SystemPropertyMap const&, ClassMap const& newContext);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
