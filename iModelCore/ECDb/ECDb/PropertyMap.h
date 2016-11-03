/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/PropertyMap.h $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
#include "ECDbInternalTypes.h"
#include "DbSchema.h"
#include "ClassMap.h"

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct PropertyMap;
struct CompoundDataPropertyMap;
struct SingleColumnDataPropertyMap;
struct SystemPropertyMap;

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
        virtual VisitorFeedback _Visit(SingleColumnDataPropertyMap const& propertyMap) const { return VisitorFeedback::Next; }
        virtual VisitorFeedback _Visit(CompoundDataPropertyMap const& propertyMap) const { return VisitorFeedback::Next; }
        virtual VisitorFeedback _Visit(SystemPropertyMap const& propertyMap) const { return VisitorFeedback::Next; }

    public:
        VisitorFeedback Visit(SingleColumnDataPropertyMap const& propertyMap) const { return _Visit(propertyMap); }
        VisitorFeedback Visit(CompoundDataPropertyMap const& propertyMap) const { return _Visit(propertyMap); }
        VisitorFeedback Visit(SystemPropertyMap const& propertyMap) const { return _Visit(propertyMap); }
    };


//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct ISupportsPropertyMapVisitor
    {
    private:
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  visitor) const = 0;
    public:
        VisitorFeedback AcceptVisitor(IPropertyMapVisitor const& visitor) const { return _AcceptVisitor(visitor); }
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct PropertyMapContainer final : NonCopyableClass, ISupportsPropertyMapVisitor
    {
    typedef std::vector<PropertyMap const*>::const_iterator const_iterator;
    private:
        ClassMap const& m_classMap;
        std::vector<PropertyMap const*> m_directDecendentList; //! contain direct decedents in order.
        std::map<Utf8CP, RefCountedPtr<PropertyMap>, CompareIUtf8Ascii> m_map; //! contain all propertymap owned by the container
        bool m_readonly;

    private:
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&)  const override;

    public:
        PropertyMapContainer(ClassMap const& classMap)
            :m_classMap(classMap), m_readonly(false)
            {}
        ~PropertyMapContainer() {}

        ClassMap const& GetClassMap() const { return m_classMap; }
        ECN::ECClass const& GetClass() const;
        ECDbCR GetECDb() const;
        BentleyStatus Insert(RefCountedPtr<PropertyMap> propertyMap, size_t position = std::numeric_limits<size_t>::max());
        PropertyMap const* Find(Utf8CP accessString) const;
        const_iterator begin() const { return m_directDecendentList.begin(); }
        const_iterator end() const { return m_directDecendentList.end(); }
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Abstract baseclass of all property map.
//+===============+===============+===============+===============+===============+======
struct PropertyMap : RefCountedBase, NonCopyableClass, ISupportsPropertyMapVisitor
    {
    public:
        enum class Type
            {
            Primitive = 1,
            PrimitiveArray = 2,
            Point3d = 4,
            Point2d = 8,
            Struct = 16,
            StructArray = 32,
            Navigation = 64,
            NavigationRelECClassId = 128,
            NavigationId = 256,
            ECInstanceId = 512,
            ECClassId = 1024,
            ConstraintECInstanceId = 2048,
            ConstraintECClassId = 4096,
            SystemPerTablePrimitive = 8192,

            System = ECInstanceId | ECClassId | ConstraintECClassId | ConstraintECInstanceId,
            Data = Primitive | Point3d | Point2d | PrimitiveArray | Struct | StructArray | Navigation | NavigationRelECClassId | NavigationId | SystemPerTablePrimitive,
            Entity = ECInstanceId | ECClassId | Data,
            Relationship = Entity | ConstraintECClassId | ConstraintECInstanceId,
            All = System | Data
            };
    private:
        Type m_type;
        ECN::ECPropertyCR m_ecProperty;
        PropertyMap const* m_parentPropertMap;
        ClassMap const& m_classMap;

        virtual bool _IsMappedToTable(DbTable const&) const = 0;

    protected:
        const Utf8String m_propertyAccessString;

        PropertyMap(Type, ClassMap const&, ECN::ECPropertyCR);
        PropertyMap(Type, PropertyMap const& parent, ECN::ECPropertyCR, Utf8StringCR accessString);

    public:
        virtual ~PropertyMap() {}

        Type GetType() const { return m_type; }

        //! Test for inherited type/
        bool IsKindOf(Type kindOfThisOrOneOfItsParent) const;

        Utf8StringCR GetName() const { return GetProperty().GetName(); }
        ECN::ECPropertyCR GetProperty() const { return m_ecProperty; }
        //! return full access string from root property to current property.
        Utf8StringCR GetAccessString() const { return m_propertyAccessString; }
        //! return parent property map if any. 
        PropertyMap const* GetParent() const { return m_parentPropertMap; }
        //! return class map that owns this property
        ClassMap const& GetClassMap() const { return m_classMap; }
        //! return root property map.
        PropertyMap const& GetRoot() const;
        
        //! Test if current property is of type system. 
        bool IsSystem() const { return Enum::Contains(Type::System, GetType()); }
        //! Test if current property is of type business. 
        bool IsData () const { return Enum::Contains(Type::Data, GetType()); }
        
        //! Test if current property map mapped to a specific table or not.
        bool IsMappedToTable(DbTable const& table) const { return _IsMappedToTable(table); } //WIP Move to ECSQL
        //! Test if current property map part of class map tables.
        bool IsMappedToClassMapTables() const; //WIP Move to ECSQL
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Abstract baseclass of all business property maps
// They must have a table and all the hierarchy of property map must hold same table.
//+===============+===============+===============+===============+===============+======
struct DataPropertyMap : PropertyMap
    {
    private:
        virtual DbTable const& _GetTable() const = 0;
        virtual bool _IsMappedToTable(DbTable const& table) const override { return &GetTable() == &table; }

    protected:
        DataPropertyMap(Type kind, ClassMap const& classMap, ECN::ECPropertyCR ecProperty)
            : PropertyMap(kind, classMap, ecProperty)
            {}
        DataPropertyMap(Type kind, PropertyMap const& parentPropertyMap, ECN::ECPropertyCR ecProperty, bool appendToAccessString)
            : PropertyMap(kind, parentPropertyMap, ecProperty, appendToAccessString ? parentPropertyMap.GetAccessString() + "." + ecProperty.GetName() : ecProperty.GetName())
            {}

    public:
        ~DataPropertyMap() {}

        DbTable const& GetTable() const { return _GetTable(); }
        //! create copy of the this property map with new context classmap
        RefCountedPtr<DataPropertyMap> CreateCopy(ClassMap const& newClassMapContext) const;
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Abstract baseclass of property map that has child property map. Only vertical property maps
// is allowed to contain child property map that must be in same table
//+===============+===============+===============+===============+===============+======
struct CompoundDataPropertyMap : DataPropertyMap
    {
    typedef bvector<DataPropertyMap const*>::const_iterator const_iterator;
 
    private:
        virtual DbTable const& _GetTable() const override;

    protected:
        bvector<DataPropertyMap const*> m_list;
        CompoundDataPropertyMap(Type kind, ClassMap const& classMap, ECN::ECPropertyCR ecProperty)
            : DataPropertyMap(kind, classMap, ecProperty) {}
        CompoundDataPropertyMap(Type kind, DataPropertyMap const& parentPropertyMap, ECN::ECPropertyCR ecProperty)
            : DataPropertyMap(kind, parentPropertyMap, ecProperty, true) {}

        VisitorFeedback AcceptChildren(IPropertyMapVisitor const&) const;
        BentleyStatus InsertMember(RefCountedPtr<DataPropertyMap> propertyMap);

    public:
        virtual ~CompoundDataPropertyMap() {}

        DataPropertyMap const* Find(Utf8CP accessString) const;
        const_iterator begin() const { return m_list.begin(); }
        const_iterator end() const { return m_list.end(); }
        size_t Size() const { return m_list.size(); }
        bool IsEmpty() const { return m_list.empty(); }
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct SingleColumnDataPropertyMap : DataPropertyMap
    {
    private:
        DbColumn const& m_column;
        virtual DbTable const& _GetTable() const override { return m_column.GetTable(); }

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

        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  visitor)  const override { return visitor.Visit(*this); }

    public:
        virtual ~PrimitivePropertyMap() {}
        static RefCountedPtr<PrimitivePropertyMap> CreateInstance(ClassMap const& classMap, CompoundDataPropertyMap const* parentPropMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct StructPropertyMap final : CompoundDataPropertyMap
    {
    friend struct StructPropertyMapBuilder;
    private:
        StructPropertyMap(ClassMap const& classMap, ECN::StructECPropertyCR ecProperty) : CompoundDataPropertyMap(Type::Struct, classMap, ecProperty) {}
        StructPropertyMap(StructPropertyMap const& parentStructPropMap, ECN::StructECPropertyCR ecProperty) : CompoundDataPropertyMap(Type::Struct, parentStructPropMap, ecProperty) {}

        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  visitor) const override;

        static RefCountedPtr<StructPropertyMap> CreateInstance(ClassMap const& classMap, StructPropertyMap const* parentStructPropMap, ECN::StructECPropertyCR ecProperty);

    public:
        virtual ~StructPropertyMap() {}

    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct PrimitiveArrayPropertyMap final : SingleColumnDataPropertyMap
    {
    private:
        PrimitiveArrayPropertyMap(ClassMap const& classMap, ECN::PrimitiveArrayECPropertyCR ecProperty, DbColumn const& column) : SingleColumnDataPropertyMap(Type::PrimitiveArray, classMap, ecProperty, column) {}
        PrimitiveArrayPropertyMap(StructPropertyMap const& parentStructPropMap, ECN::PrimitiveArrayECPropertyCR ecProperty, DbColumn const& column) : SingleColumnDataPropertyMap(Type::PrimitiveArray, parentStructPropMap, ecProperty, column, true) {}

        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  visitor)  const override { return visitor.Visit(*this); }

    public:
        virtual ~PrimitiveArrayPropertyMap() {}

        static RefCountedPtr<PrimitiveArrayPropertyMap> CreateInstance(ClassMap const& classMap, StructPropertyMap const* parentStructPropMap, ECN::PrimitiveArrayECPropertyCR ecProperty, DbColumn const& column);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct StructArrayPropertyMap final : SingleColumnDataPropertyMap
    {
    private:
        StructArrayPropertyMap(ClassMap const& classMap, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column) : SingleColumnDataPropertyMap(Type::StructArray, classMap, ecProperty, column) {}
        StructArrayPropertyMap(StructPropertyMap const& parentStructPropMap, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column) : SingleColumnDataPropertyMap(Type::StructArray, parentStructPropMap, ecProperty, column, true) {}

        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  visitor)  const override { return visitor.Visit(*this); }

    public:
        virtual ~StructArrayPropertyMap() {}
        static RefCountedPtr<StructArrayPropertyMap> CreateInstance(ClassMap const& classMap, StructPropertyMap const* parentStructPropMap, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct Point2dPropertyMap final : CompoundDataPropertyMap
    {
    private:
        Point2dPropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty) : CompoundDataPropertyMap(PropertyMap::Type::Point2d, classMap, ecProperty) {}
        Point2dPropertyMap(StructPropertyMap const& parentStructPropMap, ECN::PrimitiveECPropertyCR ecProperty) : CompoundDataPropertyMap(PropertyMap::Type::Point2d, parentStructPropMap, ecProperty) {}

        BentleyStatus Init(DbColumn const& x, DbColumn const& y);

        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  visitor)  const override;

    public:
        static RefCountedPtr<Point2dPropertyMap> CreateInstance(ClassMap const& classMap, StructPropertyMap const* parentStructPropMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y);
        virtual ~Point2dPropertyMap() {}

        PrimitivePropertyMap const& GetX() const;
        PrimitivePropertyMap const& GetY() const;
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct Point3dPropertyMap final : CompoundDataPropertyMap
    {
    private:
        Point3dPropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty)
            : CompoundDataPropertyMap(PropertyMap::Type::Point3d, classMap, ecProperty)
            {}
        Point3dPropertyMap(StructPropertyMap const& parentStructPropMap, ECN::PrimitiveECPropertyCR ecProperty)
            : CompoundDataPropertyMap(PropertyMap::Type::Point3d, parentStructPropMap, ecProperty)
            {}

        BentleyStatus Init(DbColumn const& x, DbColumn const& y, DbColumn const& z);
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  visitor)  const override;

    public:
        static RefCountedPtr<Point3dPropertyMap> CreateInstance(ClassMap const& classMap, StructPropertyMap const* parentStructPropMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y, DbColumn const& z);
        virtual ~Point3dPropertyMap() {}

        PrimitivePropertyMap const& GetX() const;
        PrimitivePropertyMap const& GetY() const;
        PrimitivePropertyMap const& GetZ() const;
    };


//=======================================================================================
// @bsiclass                                                  Krischan.Eberle       10/16
//+===============+===============+===============+===============+===============+======
struct StructPropertyMapBuilder final : NonCopyableClass
    {
    private:
        RefCountedPtr<StructPropertyMap> m_propMap;
        bmap<StructPropertyMap const*, StructPropertyMapBuilder*> m_childStructBuilderCache;

        bool m_isFinished;

    public:
        StructPropertyMapBuilder(ClassMap const& classMap, StructPropertyMapBuilder* parentBuilder, ECN::StructECPropertyCR prop);
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

            static RefCountedPtr<IdPropertyMap> CreateInstance(NavigationPropertyMap const& parentPropertyMap, DbColumn const& column);

            virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  visitor)  const override { return  visitor.Visit(*this); }

        public:
            virtual ~IdPropertyMap() {}
        };

    struct RelECClassIdPropertyMap final : SingleColumnDataPropertyMap
        {
        friend NavigationPropertyMap;
        private:
            ECN::ECClassId m_defaultClassId;

            RelECClassIdPropertyMap(NavigationPropertyMap const& parentPropertyMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column, ECN::ECClassId defaultClassId)
                : SingleColumnDataPropertyMap(Type::NavigationRelECClassId, parentPropertyMap, ecProperty, column, true), m_defaultClassId(defaultClassId)
                {}

            static RefCountedPtr<RelECClassIdPropertyMap> CreateInstance(NavigationPropertyMap const& parentPropertyMap, DbColumn const& column, ECN::ECClassId defaultRelClassId);

            virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  visitor)  const override { return  visitor.Visit(*this); }

        public:
            virtual ~RelECClassIdPropertyMap() {}
            ECN::ECClassId GetDefaultClassId() const { return m_defaultClassId; }
        };

    private:
        bool m_isComplete;

        NavigationPropertyMap(ClassMap const& classMap, ECN::NavigationECPropertyCR ecProperty)
            : CompoundDataPropertyMap(Type::Navigation, classMap, ecProperty), m_isComplete(false)
            {}

        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&) const override;

    public:
        static RefCountedPtr<NavigationPropertyMap> CreateInstance(ClassMap const& classMap, ECN::NavigationECPropertyCR ecProperty) { return new NavigationPropertyMap(classMap, ecProperty); }
        virtual ~NavigationPropertyMap() {}

        //returns true if the prop map is fully set-up and can be used. If false, it is still under construction
        bool IsComplete() const { return m_isComplete; }
        IdPropertyMap const& GetIdPropertyMap() const;
        RelECClassIdPropertyMap const& GetRelECClassIdPropertyMap() const;

        BentleyStatus SetMembers(DbColumn const& idColumn, DbColumn const& relECClassIdColumn, ECN::ECClassId defaultRelClassId);

    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct PropertyMapCopier
    {
    private:
        PropertyMapCopier();
        ~PropertyMapCopier();

        static RefCountedPtr<DataPropertyMap> CreateCopy(DataPropertyMap const&, ClassMap const& newContext, StructPropertyMapBuilder* parentStructPropMapBuilder);

    public:
        static RefCountedPtr<DataPropertyMap> CreateCopy(DataPropertyMap const&, ClassMap const& newContext);
        static RefCountedPtr<SystemPropertyMap> CreateCopy(SystemPropertyMap const&, ClassMap const& newContext);
    };

END_BENTLEY_SQLITE_EC_NAMESPACE
