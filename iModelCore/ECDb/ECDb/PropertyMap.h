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

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

struct PropertyMap;
struct DataPropertyMap;
struct CompoundDataPropertyMap;
struct SingleColumnDataPropertyMap;
struct PrimitivePropertyMap;
struct PrimitiveArrayPropertyMap;
struct StructPropertyMap;
struct StructArrayPropertyMap;
struct NavigationPropertyMap;
struct Point2dPropertyMap;
struct Point3dPropertyMap;
struct PropertyMapContainer;
struct SystemPropertyMap;
struct ECInstanceIdPropertyMap;
struct ECClassIdPropertyMap;
struct ConstraintECInstanceIdPropertyMap;
struct ConstraintECClassIdPropertyMap;

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
    public:
        enum class Kind
            {
            Primitive = 1,
            PrimitiveArray = 2,
            Point3d = 4,
            Point2d = 8,
            Struct = 16,
            StructArray = 32,
            Navigation = 64,
            ECInstanceId = 128,
            ECClassId = 256,
            ConstraintECInstanceId = 512,
            ConstraintECClassId = 1024,
            NavigationRelECClassId = 2048,
            NavigationId = 4096,

            System = ECInstanceId | ECClassId | ConstraintECClassId | ConstraintECInstanceId,
            Data = Primitive | Point3d | Point2d | PrimitiveArray | Struct | StructArray | Navigation | NavigationRelECClassId | NavigationId,
            All = System | Data
            };
    private:
        Kind m_kind;
        ECN::ECPropertyCR m_ecProperty;
        Utf8String m_propertyAccessString;
        PropertyMap const* m_parentPropertMap;    
        ClassMap const& m_classMap;
        bool m_isInEditMode;

        virtual BentleyStatus _Validate() const = 0;
        virtual bool _IsMappedToTable(DbTable const&) const = 0;
       
    protected:
        PropertyMap(Kind, ClassMap const&, ECN::ECPropertyCR);
        PropertyMap(Kind, PropertyMap const&, ECN::ECPropertyCR);
        
    public:
        virtual ~PropertyMap() {}

        //! return kind for this property.
        Kind GetKind() const { return m_kind; }

        //! Test for inherited type/
        bool IsKindOf(Kind kindOfThisOrOneOfItsParent) const;

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
        bool IsSystem() const { return Enum::Contains(Kind::System, GetKind()); }
        //! Test if current property is of type business. 
        bool IsData () const { return Enum::Contains(Kind::Data, GetKind()); }
        
        //! Test if current property map mapped to a specific table or not.
        bool IsMappedToTable(DbTable const& table) const { return _IsMappedToTable(table); } //WIP Move to ECSQL
        //! Test if current property map part of class map tables.
        bool IsMappedToClassMapTables() const; //WIP Move to ECSQL
        
        //! Test if property map is constructed correctly.
        BentleyStatus Validate() const { BeAssert(InEditMode() == false); if (InEditMode()) return ERROR;  return _Validate(); }
   
        //! A property is injected if it does not ECClass but added by ECDb
        bool InEditMode() const { return m_isInEditMode; }
        void FinishEditing() { BeAssert(m_isInEditMode);  m_isInEditMode = false; }
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
// Abstract baseclass of all business property maps
// They must have a table and all the hierarchy of propertymap must hold same table.
//+===============+===============+===============+===============+===============+======
struct DataPropertyMap : PropertyMap
    {
    private:
        virtual DbTable const& _GetTable() const = 0;
        virtual bool _IsMappedToTable(DbTable const& table) const override { return &GetTable() == &table; }

    protected:
        DataPropertyMap(Kind kind, ClassMap const& classMap, ECN::ECPropertyCR ecProperty)
            : PropertyMap(kind, classMap, ecProperty)
            {}
        DataPropertyMap(Kind kind, PropertyMap const& parentPropertyMap, ECN::ECPropertyCR ecProperty)
            : PropertyMap(kind, parentPropertyMap, ecProperty)
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
    struct Collection
        {
    private:
        bmap<Utf8CP, RefCountedPtr<DataPropertyMap>, CompareIUtf8Ascii> m_map;
        bvector<DataPropertyMap const*> m_list;

    public:
        DataPropertyMap const* Find(Utf8CP accessString) const;
        DataPropertyMap const* Front() const { BeAssert(!m_list.empty());  return m_list.empty() ? nullptr : m_list.front(); }
        bvector<DataPropertyMap const*> const& GetList() const { return m_list; }
        BentleyStatus Insert(RefCountedPtr<DataPropertyMap> propertyMap, DataPropertyMap const& parent, size_t position);
        BentleyStatus Remove(Utf8CP accessString);
        void Clear() { m_list.clear(); m_map.clear(); }
        } ;

    private:
        Collection m_children;
        bool m_readonly;

        virtual DbTable const& _GetTable() const override;
        BentleyStatus VerifyVerticalIntegerity(DataPropertyMap const& propertyMap) const;

    protected:
        CompoundDataPropertyMap(Kind kind, ClassMap const& classMap, ECN::ECPropertyCR ecProperty)
            : DataPropertyMap(kind, classMap, ecProperty), m_readonly(false)
            {}
        CompoundDataPropertyMap(Kind kind, DataPropertyMap const& parentPropertyMap, ECN::ECPropertyCR ecProperty)
            : DataPropertyMap(kind, parentPropertyMap, ecProperty), m_readonly(false)
            {}

        void MakeWritable() { m_readonly = false; }
        void Clear();
        virtual BentleyStatus _Validate() const override;
        VisitorFeedback AcceptChildren(IPropertyMapVisitor const&) const;

    public:
        virtual ~CompoundDataPropertyMap() {}

        bool IsReadonly() const { return m_readonly; }
        void MakeReadOnly() { m_readonly = true; }
        DataPropertyMap const* Find(Utf8CP accessString, bool recusive = true) const;
        BentleyStatus Insert(RefCountedPtr<DataPropertyMap> propertyMap, size_t position = std::numeric_limits<size_t>::max());
        BentleyStatus Remove(Utf8CP accessString);
        const_iterator begin() const { return m_children.GetList().begin(); }
        const_iterator end() const { return m_children.GetList().end(); }
        size_t Size() const { return m_children.GetList().size(); }
        bool IsEmpty() const { return m_children.GetList().empty(); }
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
        SingleColumnDataPropertyMap(Kind kind, ClassMap const& classMap, ECN::ECPropertyCR ecProperty, DbColumn const& column)
            : DataPropertyMap(kind, classMap, ecProperty), m_column(column)
            {}
        SingleColumnDataPropertyMap(Kind kind, DataPropertyMap const& parentPropertyMap, ECN::ECPropertyCR ecProperty, DbColumn const& column)
            : DataPropertyMap(kind, parentPropertyMap, ecProperty), m_column(column)
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
            : SingleColumnDataPropertyMap(Kind::Primitive, classMap, ecProperty, column)
            {}
        PrimitivePropertyMap(DataPropertyMap const& parentPropertyMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column)
            : SingleColumnDataPropertyMap(Kind::Primitive, parentPropertyMap, ecProperty, column)
            {}

        virtual BentleyStatus _Validate() const override;
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  visitor)  const override { return visitor.Visit(*this); }

    public:
        virtual ~PrimitivePropertyMap() {}
        static RefCountedPtr<PrimitivePropertyMap> CreateInstance(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column);
        static RefCountedPtr<PrimitivePropertyMap> CreateInstance(ECN::PrimitiveECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& column);
    };
//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct PrimitiveArrayPropertyMap final : SingleColumnDataPropertyMap
    {
    private:
        PrimitiveArrayPropertyMap(ClassMap const& classMap, ECN::ArrayECPropertyCR ecProperty, DbColumn const& column) : SingleColumnDataPropertyMap(Kind::PrimitiveArray, classMap, ecProperty, column) {}
        PrimitiveArrayPropertyMap(DataPropertyMap const& parentPropertyMap, ECN::ArrayECPropertyCR ecProperty, DbColumn const& column) : SingleColumnDataPropertyMap(Kind::PrimitiveArray, parentPropertyMap, ecProperty, column) {}

        virtual BentleyStatus _Validate() const override { return SUCCESS; }
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  visitor)  const override { return visitor.Visit(*this); }

    public:
        virtual ~PrimitiveArrayPropertyMap() {}

        static RefCountedPtr<PrimitiveArrayPropertyMap> CreateInstance(ClassMap const& classMap, ECN::ArrayECPropertyCR ecProperty, DbColumn const& column) { return new PrimitiveArrayPropertyMap(classMap, ecProperty, column); }
        static RefCountedPtr<PrimitiveArrayPropertyMap> CreateInstance(ECN::ArrayECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& column) { return new PrimitiveArrayPropertyMap(parentPropertyMap, ecProperty, column); }
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct StructArrayPropertyMap final : SingleColumnDataPropertyMap
    {
    private:
        StructArrayPropertyMap(ClassMap const& classMap, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column) : SingleColumnDataPropertyMap(Kind::StructArray, classMap, ecProperty, column) {}
        StructArrayPropertyMap(DataPropertyMap const& parentPropertyMap, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column) : SingleColumnDataPropertyMap(Kind::StructArray, parentPropertyMap, ecProperty, column) {}

        virtual BentleyStatus _Validate() const override { return SUCCESS; }
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  visitor)  const override { return visitor.Visit(*this); }

    public:
        virtual ~StructArrayPropertyMap() {}
        static RefCountedPtr<StructArrayPropertyMap> CreateInstance(ClassMap const& classMap, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column) { return new StructArrayPropertyMap(classMap, ecProperty, column); }
        static RefCountedPtr<StructArrayPropertyMap> CreateInstance(ECN::StructArrayECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& column) { return new StructArrayPropertyMap(parentPropertyMap, ecProperty, column); }
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct Point2dPropertyMap final : CompoundDataPropertyMap
    {
    private:
        Point2dPropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty) : CompoundDataPropertyMap(PropertyMap::Kind::Point2d, classMap, ecProperty) {}
        Point2dPropertyMap(DataPropertyMap const& parentPropertyMap, ECN::PrimitiveECPropertyCR ecProperty) : CompoundDataPropertyMap(PropertyMap::Kind::Point2d, parentPropertyMap, ecProperty) {}

        BentleyStatus Init(DbColumn const& x, DbColumn const& y);

        virtual BentleyStatus _Validate() const override;
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  visitor)  const override;

    public:
        virtual ~Point2dPropertyMap() {}

        PrimitivePropertyMap const& GetX() const { return static_cast<PrimitivePropertyMap const&>(*Find(ECDbSystemSchemaHelper::X_PROPNAME)); }
        PrimitivePropertyMap const& GetY() const { return static_cast<PrimitivePropertyMap const&>(*Find(ECDbSystemSchemaHelper::Y_PROPNAME)); }
        static RefCountedPtr<Point2dPropertyMap> CreateInstance(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y);
        static RefCountedPtr<Point2dPropertyMap> CreateInstance(ECN::PrimitiveECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& x, DbColumn const& y);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct Point3dPropertyMap final : CompoundDataPropertyMap
    {
    private:
        Point3dPropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty)
            : CompoundDataPropertyMap(PropertyMap::Kind::Point3d, classMap, ecProperty)
            {}
        Point3dPropertyMap(DataPropertyMap const& parentPropertyMap, ECN::PrimitiveECPropertyCR ecProperty)
            : CompoundDataPropertyMap(PropertyMap::Kind::Point3d, parentPropertyMap, ecProperty)
            {}

        BentleyStatus Init(DbColumn const& x, DbColumn const& y, DbColumn const& z);
        virtual BentleyStatus _Validate() const override;
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  visitor)  const override;

    public:
        virtual ~Point3dPropertyMap() {}
        PrimitivePropertyMap const& GetX() const { return static_cast<PrimitivePropertyMap const&>(*Find(ECDbSystemSchemaHelper::X_PROPNAME)); }
        PrimitivePropertyMap const& GetY() const { return static_cast<PrimitivePropertyMap const&>(*Find(ECDbSystemSchemaHelper::Y_PROPNAME)); }
        PrimitivePropertyMap const& GetZ() const { return static_cast<PrimitivePropertyMap const&>(*Find(ECDbSystemSchemaHelper::Z_PROPNAME)); }
        static RefCountedPtr<Point3dPropertyMap> CreateInstance(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y, DbColumn const& z);
        static RefCountedPtr<Point3dPropertyMap> CreateInstance(ECN::PrimitiveECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& x, DbColumn const& y, DbColumn const& z);
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct StructPropertyMap final : CompoundDataPropertyMap
    {
    private:
        StructPropertyMap(ClassMap const& classMap, ECN::StructECPropertyCR ecProperty) : CompoundDataPropertyMap(Kind::Struct, classMap, ecProperty) {}
        StructPropertyMap(DataPropertyMap const& parentPropertyMap, ECN::StructECPropertyCR ecProperty) : CompoundDataPropertyMap(Kind::Struct, parentPropertyMap, ecProperty) {}

        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  visitor)  const override;

    public:
        virtual ~StructPropertyMap() {}

        static RefCountedPtr<StructPropertyMap> CreateInstance(ClassMap const& classMap, ECN::StructECPropertyCR ecProperty) { return new StructPropertyMap(classMap, ecProperty); }
        static RefCountedPtr<StructPropertyMap> CreateInstance(ECN::StructECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap) { return new StructPropertyMap(parentPropertyMap, ecProperty); }
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
        private:
            IdPropertyMap(NavigationPropertyMap const& parentPropertyMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column)
                : SingleColumnDataPropertyMap(Kind::NavigationId, parentPropertyMap, ecProperty, column)
                {}
            virtual BentleyStatus _Validate() const override { return SUCCESS; }
            virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  visitor)  const override { return  visitor.Visit(*this); }

        public:
            virtual ~IdPropertyMap() {}
            static RefCountedPtr<IdPropertyMap> CreateInstance(NavigationPropertyMap const& parentPropertyMap, DbColumn const& column);
        };

    struct RelECClassIdPropertyMap final : SingleColumnDataPropertyMap
        {
        private:
            ECN::ECClassId m_defaultClassId;

            RelECClassIdPropertyMap(NavigationPropertyMap const& parentPropertyMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column, ECN::ECClassId defaultClassId)
                : SingleColumnDataPropertyMap(Kind::NavigationRelECClassId, parentPropertyMap, ecProperty, column), m_defaultClassId(defaultClassId)
                {}

            virtual BentleyStatus _Validate() const override { return SUCCESS; }
            virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&  visitor)  const override { return  visitor.Visit(*this); }

        public:
            virtual ~RelECClassIdPropertyMap() {}
            ECN::ECClassId GetDefaultClassId() const { return m_defaultClassId; }
            static RefCountedPtr<RelECClassIdPropertyMap> CreateInstance(NavigationPropertyMap const& parentPropertyMap, DbColumn const& column, ECN::ECClassId defaultRelClassId);
        };

    private:
        NavigationPropertyMap(ClassMap const& classMap, ECN::NavigationECPropertyCR ecProperty)
            : CompoundDataPropertyMap(Kind::Navigation, classMap, ecProperty)
            {}

        virtual BentleyStatus _Validate() const override;
        virtual VisitorFeedback _AcceptVisitor(IPropertyMapVisitor const&)  const override;

    public:
        virtual ~NavigationPropertyMap() {}

        BentleyStatus Initialize(DbColumn const& relECClassIdColumn, ECN::ECClassId defaultRelClassId, DbColumn const& idColumn);
        RelECClassIdPropertyMap const& GetRelECClassId() const { return static_cast<RelECClassIdPropertyMap const&>(*Find(ECDbSystemSchemaHelper::RELECCLASSID_PROPNAME)); }
        IdPropertyMap const& GetId() const { return static_cast<IdPropertyMap const&>(*Find(ECDbSystemSchemaHelper::ID_PROPNAME)); };
        static RefCountedPtr<NavigationPropertyMap> CreateInstance(ClassMap const& classMap, ECN::NavigationECPropertyCR ecProperty) { return new NavigationPropertyMap(classMap, ecProperty); }
    };

//=======================================================================================
// @bsiclass                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
struct PropertyMapFactory final
    {
    private:
        PropertyMapFactory();
        ~PropertyMapFactory();

        static RefCountedPtr<DataPropertyMap> CreateCopy(DataPropertyMap const& propertyMap, ClassMap const& newContext, DataPropertyMap const* newParent);

    public:
        //! Data Property Maps
        static RefCountedPtr<PrimitivePropertyMap> CreatePrimitivePropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column);
        static RefCountedPtr<PrimitivePropertyMap> CreatePrimitivePropertyMap(ECN::PrimitiveECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& column);
        static RefCountedPtr<PrimitiveArrayPropertyMap> CreatePrimitiveArrayPropertyMap(ClassMap const& classMap, ECN::ArrayECPropertyCR ecProperty, DbColumn const& column);
        static RefCountedPtr<PrimitiveArrayPropertyMap> CreatePrimitiveArrayPropertyMap(ECN::ArrayECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& column);
        static RefCountedPtr<StructPropertyMap> CreateStructPropertyMap(ClassMap const& classMap, ECN::StructECPropertyCR ecProperty);
        static RefCountedPtr<StructPropertyMap> CreateStructPropertyMap(ECN::StructECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap);
        static RefCountedPtr<StructArrayPropertyMap> CreateStructArrayPropertyMap(ClassMap const& classMap, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column);
        static RefCountedPtr<StructArrayPropertyMap> CreateStructArrayPropertyMap(ECN::StructArrayECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& column);
        static RefCountedPtr<Point2dPropertyMap> CreatePoint2dPropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y);
        static RefCountedPtr<Point2dPropertyMap> CreatePoint2dPropertyMap(ECN::PrimitiveECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& x, DbColumn const& y);
        static RefCountedPtr<Point3dPropertyMap> CreatePoint3dPropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y, DbColumn const& z);
        static RefCountedPtr<Point3dPropertyMap> CreatePoint3dPropertyMap(ECN::PrimitiveECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& x, DbColumn const& y, DbColumn const& z);
        static RefCountedPtr<NavigationPropertyMap> CreateNavigationPropertyMap(ClassMap const& classMap, ECN::NavigationECPropertyCR ecProperty);
        static RefCountedPtr<ECInstanceIdPropertyMap> CreateECInstanceIdPropertyMap(ClassMap const& classMap, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<ECClassIdPropertyMap> CreateECClassIdPropertyMap(ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<ConstraintECClassIdPropertyMap> CreateSourceECClassIdPropertyMap(ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<ConstraintECClassIdPropertyMap> CreateTargetECClassIdPropertyMap(ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<ConstraintECInstanceIdPropertyMap> CreateSourceECInstanceIdPropertyMap(ClassMap const& classMap, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<ConstraintECInstanceIdPropertyMap> CreateTargetECInstanceIdPropertyMap(ClassMap const& classMap, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<DataPropertyMap> CreateCopy(DataPropertyMap const& propertyMap, ClassMap const& newContext);
        static RefCountedPtr<SystemPropertyMap> CreateCopy(SystemPropertyMap const& propertyMap, ClassMap const& newContext);
        static RefCountedPtr<ConstraintECInstanceIdPropertyMap> CreateConstraintECInstanceIdPropertyMap(ECN::ECRelationshipEnd end, ClassMap const& classMap, std::vector<DbColumn const*> const& columns);
        static RefCountedPtr<ConstraintECClassIdPropertyMap> CreateConstraintECClassIdPropertyMap(ECN::ECRelationshipEnd end, ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns);
    };


        //DbTable const* RequiresJoinTo(WipConstraintECClassIdPropertyMap const& propertyMap) const;
        //Utf8CP GetECClassIdPrimaryTableAlias(WipConstraintECClassIdPropertyMap const& propertyMap) const;

END_BENTLEY_SQLITE_EC_NAMESPACE
