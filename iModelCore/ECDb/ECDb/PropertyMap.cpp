/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/PropertyMap.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************************************PropertyMap*************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
PropertyMap::PropertyMap(Kind kind, ClassMap const& classMap, ECN::ECPropertyCR ecProperty)
    :m_kind(kind),  m_ecProperty(ecProperty), m_classMap(classMap), m_parentPropertMap(nullptr), 
    m_propertyAccessString(ecProperty.GetName()), m_isInEditMode(true)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
PropertyMap::PropertyMap(Kind kind, PropertyMap const& parentPropertyMap, ECN::ECPropertyCR ecProperty)
    :m_kind(kind), m_ecProperty(ecProperty), m_classMap(parentPropertyMap.GetClassMap()), m_parentPropertMap(&parentPropertyMap), 
    m_propertyAccessString(parentPropertyMap.GetAccessString() + EC_ACCESSSTRING_DELIMITER + ecProperty.GetName()), m_isInEditMode(true)
    {}

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
bool PropertyMap::IsKindOf(Kind kindOfThisOrOneOfItsParent) const
    {
    const PropertyMap *c = this;
    do
        {
        if (Enum::Contains(kindOfThisOrOneOfItsParent, c->GetKind()))
            return true;

        c = c->GetParent();
        } while (c != nullptr);

        return false;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
PropertyMap const& PropertyMap::GetRoot() const
    {
    PropertyMap const* root = this;
    while (root->GetParent() != nullptr)
        root = root->GetParent();

    return *root;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
bool PropertyMap::IsMappedToClassMapTables() const
    {
    for (DbTable const* table : GetClassMap().GetTables())
        {
        if (IsMappedToTable(*table))
            return true;
        }

    return false;
    }

//***************************************************WipPropertyMapContainer*****************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus PropertyMapContainer::Insert(RefCountedPtr<PropertyMap> propertyMap, size_t position)
    {
    if (m_readonly)
        {
        BeAssert(false && "Readonly collection cannot be modified");
        return ERROR;
        }

    if (propertyMap == nullptr)
        {
        BeAssert(false && "PropertyMap passed cannot be null");
        return ERROR;
        }

    if (&propertyMap->GetClassMap() != &GetClassMap())
        {
        BeAssert(false && "propertyMap must belong to same classMap context");
        return ERROR;
        }

    if (Find(propertyMap->GetAccessString().c_str()))
        {
        BeAssert(false && "PropertyMap with same name or may be different case already exist");
        return ERROR;
        }

    auto where = position > m_directDecendentList.size() ? m_directDecendentList.end() : m_directDecendentList.begin() + position;
    m_map[propertyMap->GetAccessString().c_str()] = propertyMap;
    m_directDecendentList.insert(where, propertyMap.get());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
VisitorFeedback PropertyMapContainer::_AcceptVisitor(IPropertyMapVisitor const&  dispatcher)  const
    {
    VisitorFeedback fb = VisitorFeedback::Next;
    for (PropertyMap const* propertyMap : m_directDecendentList)
        {
        fb = propertyMap->AcceptVisitor(dispatcher);
        if (fb == VisitorFeedback::Cancel)
            return fb;
        }

    return fb;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
PropertyMap const* PropertyMapContainer::Find(Utf8CP accessString, bool recusive) const
    {
    auto itor = m_map.find(accessString);
    if (itor != m_map.end())
        return itor->second.get();

    if (recusive)
        for (PropertyMap const* child : m_directDecendentList)
            if (auto collection = dynamic_cast<CompoundDataPropertyMap const*>(child))
                if (auto result = collection->Find(accessString, true))
                    return result;

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
ECN::ECClass const& PropertyMapContainer::GetClass() const { return m_classMap.GetClass(); }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
ECDbCR PropertyMapContainer::GetECDb() const { return m_classMap.GetDbMap().GetECDb(); }

//************************************CompoundDataPropertyMap::Collection********
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
DataPropertyMap const* CompoundDataPropertyMap::Collection::Find(Utf8CP accessString) const
    {
    auto itor = m_map.find(accessString);
    if (itor != m_map.end())
        return itor->second.get();
  
    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
VisitorFeedback CompoundDataPropertyMap::AcceptChildren(IPropertyMapVisitor const&  dispatcher) const
    {
    VisitorFeedback fb = VisitorFeedback::Next;
    for (DataPropertyMap const* pm : *this)
        {
        fb = pm->AcceptVisitor(dispatcher);
        if (fb == VisitorFeedback::Cancel)
            return fb;
        }

    return fb;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
DataPropertyMap const* CompoundDataPropertyMap::Find(Utf8CP accessString, bool recusive) const
    {
    if (auto result = m_children.Find(accessString))
        return result;

    if (recusive)
        for (DataPropertyMap const* child : m_children.GetList())
            if (auto collection = dynamic_cast<CompoundDataPropertyMap const*>(child))
                if (auto result = collection->Find(accessString, true))
                    return result;

    return nullptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus CompoundDataPropertyMap::Collection::Insert(RefCountedPtr<DataPropertyMap> propertyMap, DataPropertyMap const& parent, size_t position)
    {
    if (propertyMap == nullptr)
        {
        BeAssert(false && "propertyMap cannot be null");
        return ERROR;
        }

    if (m_map.find(propertyMap->GetAccessString().c_str()) != m_map.end())
        {
        BeAssert(false && "PropertyMap with same name or may be different case already exist");
        return ERROR;
        }
    auto where = position > m_list.size() ? m_list.end() : m_list.begin() + position;
    m_map[propertyMap->GetAccessString().c_str()] = propertyMap;
    m_list.insert(where, propertyMap.get());
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus CompoundDataPropertyMap::Collection::Remove(Utf8CP accessString)
    {
    if (DataPropertyMap const* r = Find(accessString))
        {
        m_list.erase(std::find(m_list.begin(), m_list.end(), r));
        m_map.erase(accessString);
        return SUCCESS;
        }

    return ERROR;
    }

//************************************CompoundDataPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus CompoundDataPropertyMap::_Validate() const
    {
    if (IsEmpty())
        {
        BeAssert(false);
        return ERROR;
        }
    for (DataPropertyMap const* child : *this)
        {
        if (child->Validate() != BentleyStatus::SUCCESS)
            return ERROR;
        }

    return IsReadonly() ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
DbTable const& CompoundDataPropertyMap::_GetTable() const
    {
    if (IsEmpty())
        {
        if (GetParent() == nullptr)
            {
            BeAssert(false);
            return *static_cast<DbTable const*>(nullptr);
            }
        else
            return static_cast<DataPropertyMap const*>(GetParent())->GetTable();
        }

    return m_children.Front()->GetTable();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus CompoundDataPropertyMap::VerifyVerticalIntegerity(DataPropertyMap const& propertyMap) const
    {
    if (IsEmpty())
        {
        if (GetParent() != nullptr)
            {
            DataPropertyMap const* vp = static_cast<DataPropertyMap const*>(GetParent());
            DbTable const& parentTable = vp->GetTable();
            if (&propertyMap.GetTable() != &parentTable)
                {
                BeAssert(false && "Table must match parent table");
                return ERROR;
                }
            }
        }
    else
        {
        if (&m_children.Front()->GetTable() != &propertyMap.GetTable())
            {
            BeAssert(false && "All property map must have same table");
            return ERROR;
            }
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
void CompoundDataPropertyMap::Clear()
    {
    if (m_readonly)
        {
        BeAssert(false && "PropertyMap is readonly");
        return;
        }

    m_children.Clear();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus CompoundDataPropertyMap::Remove(Utf8CP accessString)
    {
    if (m_readonly)
        {
        BeAssert(false && "property map cannot be removed from this readonly collection");
        return ERROR;
        }
    return m_children.Remove(accessString);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus CompoundDataPropertyMap::Insert(RefCountedPtr<DataPropertyMap> propertyMap, size_t position)
    {
    if (m_readonly)
        {
        BeAssert(false && "New property maps cannot be added to this property map");
        return ERROR;
        }

    if (&propertyMap->GetClassMap() != &GetClassMap())
        {
        BeAssert(false && "propertyMap must belong to same classMap context");
        return ERROR;
        }

    if (propertyMap->GetParent() != this)
        {
        BeAssert(false && "Parent propertymap is incorrect or null");
        return ERROR;
        }

    //Vertical integrity check
    if (m_children.Insert(propertyMap, *this, position) != SUCCESS)
        return ERROR;

    if (VerifyVerticalIntegerity(*propertyMap) != SUCCESS)
        {
        m_children.Remove(propertyMap->GetAccessString().c_str());
        return ERROR;
        }

    return SUCCESS;
    }



//************************************DataPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
RefCountedPtr<DataPropertyMap> DataPropertyMap::CreateCopy(ClassMap const& newClassMapContext) const
    {
    return PropertyMapFactory::CreateCopy(*this, newClassMapContext);
    }



//************************************PrimitivePropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus PrimitivePropertyMap::_Validate() const
    {
    ECN::PrimitiveECPropertyCP property = GetProperty().GetAsPrimitiveProperty();
    return property->GetType() != PRIMITIVETYPE_Point2d && property->GetType() != PRIMITIVETYPE_Point3d ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<PrimitivePropertyMap> PrimitivePropertyMap::CreateInstance(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column)
    {
    if (ecProperty.GetType() == PRIMITIVETYPE_Point2d || ecProperty.GetType() == PRIMITIVETYPE_Point3d)
        {
        BeAssert(false && "Do not support Point2d or Point3d property");
        return nullptr;
        }

    return new PrimitivePropertyMap(classMap, ecProperty, column);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<PrimitivePropertyMap> PrimitivePropertyMap::CreateInstance(ECN::PrimitiveECPropertyCR ecProperty, PropertyMap const& parentPropertyMap, DbColumn const& column)
    {
    if (ecProperty.GetType() == PRIMITIVETYPE_Point2d || ecProperty.GetType() == PRIMITIVETYPE_Point3d)
        {
        BeAssert(false && "Do not support Point2d or Point3d property");
        return nullptr;
        }

    return new PrimitivePropertyMap(parentPropertyMap, ecProperty, column);
    }

//************************************PrimitiveArrayPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<PrimitiveArrayPropertyMap> PrimitiveArrayPropertyMap::CreateInstance(ClassMap const& classMap, ECN::ArrayECPropertyCR ecProperty, DbColumn const& column)
    {
    return new PrimitiveArrayPropertyMap(Kind::PrimitiveArray, classMap, ecProperty, column);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<PrimitiveArrayPropertyMap> PrimitiveArrayPropertyMap::CreateInstance(ECN::ArrayECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& column)
    {
    return new PrimitiveArrayPropertyMap(Kind::PrimitiveArray, parentPropertyMap, ecProperty, column);
    }

//************************************StructArrayPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<StructArrayPropertyMap> StructArrayPropertyMap::CreateInstance(ClassMap const& classMap, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column)
    {
    return new StructArrayPropertyMap(Kind::StructArray, classMap, ecProperty, column);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<StructArrayPropertyMap> StructArrayPropertyMap::CreateInstance(ECN::StructArrayECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& column)
    {
    return new StructArrayPropertyMap(Kind::StructArray, parentPropertyMap, ecProperty, column);
    }


//************************************Point2dPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus Point2dPropertyMap::Init(DbColumn const& x, DbColumn const& y)
    {
    if (&x.GetTable() != &y.GetTable())
        {
        return ERROR;
        }

    ECDbSchemaManagerCR schemaManger = GetClassMap().GetDbMap().GetECDb().Schemas();
    ECPropertyCP propX = ECDbSystemSchemaHelper::GetSystemProperty(schemaManger, ECSqlSystemProperty::X);
    if (propX == nullptr)
        return ERROR;

    RefCountedPtr<PrimitivePropertyMap> xPropertyMap = PrimitivePropertyMap::CreateInstance(*propX->GetAsPrimitiveProperty(), *this, x);
    if (xPropertyMap == nullptr)
        return ERROR;

    if (Insert(xPropertyMap) != SUCCESS)
        return ERROR;

    ECPropertyCP propY = ECDbSystemSchemaHelper::GetSystemProperty(schemaManger, ECSqlSystemProperty::Y);
    if (propY == nullptr)
        return ERROR;

    RefCountedPtr<PrimitivePropertyMap> yPropertyMap = PrimitivePropertyMap::CreateInstance(*propY->GetAsPrimitiveProperty(), *this, y);
    if (yPropertyMap == nullptr)
        return ERROR;

    if (Insert(yPropertyMap) != SUCCESS)
        return ERROR;

    MakeReadOnly();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
VisitorFeedback Point2dPropertyMap::_AcceptVisitor(IPropertyMapVisitor const&  dispatcher)  const
    {
    VisitorFeedback fb = dispatcher.Visit(*this);
    if (fb == VisitorFeedback::Cancel || fb == VisitorFeedback::NextSibling)
        return fb;

    return AcceptChildren(dispatcher);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus Point2dPropertyMap::_Validate() const
    {
    return Size() == 2 && IsReadonly() ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<Point2dPropertyMap> Point2dPropertyMap::CreateInstance(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y)
    {
    if (ecProperty.GetType() != PrimitiveType::PRIMITIVETYPE_Point2d)
        {
        BeAssert(false && "PrimitiveType must be Point2d");
        return nullptr;
        }

    RefCountedPtr<Point2dPropertyMap> ptr = new Point2dPropertyMap(Kind::Point2d, classMap, ecProperty);
    if (ptr->Init(x, y) != SUCCESS)
        return nullptr;

    return ptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<Point2dPropertyMap> Point2dPropertyMap::CreateInstance(ECN::PrimitiveECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& x, DbColumn const& y)
    {
    if (ecProperty.GetType() != PrimitiveType::PRIMITIVETYPE_Point2d)
        {
        BeAssert(false && "PrimitiveType must be Point2d");
        return nullptr;
        }

    RefCountedPtr<Point2dPropertyMap> ptr = new Point2dPropertyMap(Kind::Point3d, parentPropertyMap, ecProperty);
    if (ptr->Init(x, y) != SUCCESS)
        return nullptr;

    return ptr;
    }

//************************************Point3dPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus Point3dPropertyMap::Init(DbColumn const& x, DbColumn const& y, DbColumn const& z)
    {
    ECDbSchemaManagerCR schemaManger = GetClassMap().GetDbMap().GetECDb().Schemas();
    ECPropertyCP propX = ECDbSystemSchemaHelper::GetSystemProperty(schemaManger, ECSqlSystemProperty::X);
    if (propX == nullptr)
        return ERROR;

    RefCountedPtr<PrimitivePropertyMap> xPropertyMap = PrimitivePropertyMap::CreateInstance(*propX->GetAsPrimitiveProperty(), *this, x);
    if (xPropertyMap == nullptr)
        return ERROR;

    if (Insert(xPropertyMap) != SUCCESS)
        return ERROR;

    ECPropertyCP propY = ECDbSystemSchemaHelper::GetSystemProperty(schemaManger, ECSqlSystemProperty::Y);
    if (propY == nullptr)
        return ERROR;

    RefCountedPtr<PrimitivePropertyMap> yPropertyMap = PrimitivePropertyMap::CreateInstance(*propY->GetAsPrimitiveProperty(), *this, y);
    if (yPropertyMap == nullptr)
        return ERROR;

    if (Insert(yPropertyMap) != SUCCESS)
        return ERROR;

    ECPropertyCP propZ = ECDbSystemSchemaHelper::GetSystemProperty(schemaManger, ECSqlSystemProperty::Z);
    if (propZ == nullptr)
        return ERROR;

    RefCountedPtr<PrimitivePropertyMap> zPropertyMap = PrimitivePropertyMap::CreateInstance(*propZ->GetAsPrimitiveProperty(), *this, z);
    if (zPropertyMap == nullptr)
        return ERROR;

    if (Insert(zPropertyMap) != SUCCESS)
        return ERROR;

    MakeReadOnly();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
VisitorFeedback Point3dPropertyMap::_AcceptVisitor(IPropertyMapVisitor const&  dispatcher)  const
    {
    VisitorFeedback fb = dispatcher.Visit(*this);
    if (fb == VisitorFeedback::Cancel || fb == VisitorFeedback::NextSibling)
        return fb;

    return AcceptChildren(dispatcher);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus Point3dPropertyMap::_Validate() const
    {
    return Size() == 3 && IsReadonly() ? SUCCESS : ERROR;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<Point3dPropertyMap> Point3dPropertyMap::CreateInstance(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y, DbColumn const& z)
    {
    if (ecProperty.GetType() != PrimitiveType::PRIMITIVETYPE_Point3d)
        {
        BeAssert(false && "PrimitiveType must be Point3d");
        return nullptr;
        }

    RefCountedPtr<Point3dPropertyMap> ptr = new Point3dPropertyMap(Kind::Point3d, classMap, ecProperty);
    if (ptr->Init(x, y, z) != SUCCESS)
        return nullptr;

    return ptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<Point3dPropertyMap> Point3dPropertyMap::CreateInstance(ECN::PrimitiveECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& x, DbColumn const& y, DbColumn const& z)
    {
    if (ecProperty.GetType() != PrimitiveType::PRIMITIVETYPE_Point3d)
        {
        BeAssert(false && "PrimitiveType must be Point3d");
        return nullptr;
        }

    RefCountedPtr<Point3dPropertyMap> ptr = new Point3dPropertyMap(Kind::Point3d, parentPropertyMap, ecProperty);
    if (ptr->Init(x, y, z) != SUCCESS)
        return nullptr;

    return ptr;
    }

//************************************StructPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<StructPropertyMap> StructPropertyMap::CreateInstance(ClassMap const& classMap, ECN::StructECPropertyCR ecProperty)
    {
    return new StructPropertyMap(Kind::Struct, classMap, ecProperty);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
VisitorFeedback StructPropertyMap::_AcceptVisitor(IPropertyMapVisitor const&  dispatcher)  const
    {
    VisitorFeedback fb = dispatcher.Visit(*this);
    if (fb == VisitorFeedback::Cancel || fb == VisitorFeedback::NextSibling)
        return fb;

    return AcceptChildren(dispatcher);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<StructPropertyMap> StructPropertyMap::CreateInstance(ECN::StructECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap)
    {
    return new StructPropertyMap(Kind::Struct, parentPropertyMap, ecProperty);
    }

//************************************NavigationPropertyMap::RelECClassIdPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<NavigationPropertyMap::RelECClassIdPropertyMap> NavigationPropertyMap::RelECClassIdPropertyMap::CreateInstance(NavigationPropertyMap const& parentPropertyMap, DbColumn const& column, ECN::ECClassId defaultRelClassId)
    {
    ECDbSchemaManagerCR schemaManger = parentPropertyMap.GetClassMap().GetDbMap().GetECDb().Schemas();
    ECPropertyCP relECClassIdProp = ECDbSystemSchemaHelper::GetSystemProperty(schemaManger, ECSqlSystemProperty::RelECClassId);
    if (relECClassIdProp == nullptr)
        return nullptr;

    return  new RelECClassIdPropertyMap(Kind::NavigationRelECClassId, parentPropertyMap, *relECClassIdProp->GetAsPrimitiveProperty(), column, defaultRelClassId);
    }

//************************************NavigationPropertyMap::IdPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static
RefCountedPtr<NavigationPropertyMap::IdPropertyMap> NavigationPropertyMap::IdPropertyMap::CreateInstance(NavigationPropertyMap const& parentPropertyMap, DbColumn const& column)
    {
    ECDbSchemaManagerCR schemaManger = parentPropertyMap.GetClassMap().GetDbMap().GetECDb().Schemas();
    ECPropertyCP idProp = ECDbSystemSchemaHelper::GetSystemProperty(schemaManger, ECSqlSystemProperty::Id);
    if (idProp == nullptr)
        return nullptr;

    return new IdPropertyMap(Kind::NavigationId, parentPropertyMap, *idProp->GetAsPrimitiveProperty(), column);
    }


//************************************NavigationPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus NavigationPropertyMap::Initialize(DbColumn const& relECClassIdColumn, ECN::ECClassId defaultRelClassId, DbColumn const& idColumn)
    {
    if (&relECClassIdColumn.GetTable() != &idColumn.GetTable())
        {
        return ERROR;
        }

    RefCountedPtr<RelECClassIdPropertyMap> relECClassIdPropertyMap = RelECClassIdPropertyMap::CreateInstance(*this, relECClassIdColumn, defaultRelClassId);
    if (relECClassIdPropertyMap == nullptr)
        return ERROR;

    relECClassIdPropertyMap->FinishEditing();
    if(Insert(relECClassIdPropertyMap) != SUCCESS)
        {
        Clear();
        return ERROR;
        }

    RefCountedPtr<IdPropertyMap> idPropertyMap = IdPropertyMap::CreateInstance(*this, idColumn);
    if (idPropertyMap == nullptr)
        return ERROR;

    idPropertyMap->FinishEditing();
    if(Insert(idPropertyMap) != SUCCESS)
        return ERROR;

    MakeReadOnly();
    FinishEditing();
    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
VisitorFeedback NavigationPropertyMap::_AcceptVisitor(IPropertyMapVisitor const&  dispatcher)  const
    {
    VisitorFeedback fb = dispatcher.Visit(*this);
    if (fb == VisitorFeedback::Cancel || fb == VisitorFeedback::NextSibling)
        return fb;

    return AcceptChildren(dispatcher);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus NavigationPropertyMap::_Validate() const
    {
    if (Size() != 2)
        return ERROR;

    return CompoundDataPropertyMap::_Validate();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<NavigationPropertyMap> NavigationPropertyMap::CreateInstance(ClassMap const& classMap, ECN::NavigationECPropertyCR ecProperty)
    {
    return new NavigationPropertyMap(Kind::Navigation, classMap, ecProperty);
    }


//************************************PropertyMapFactory********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<PrimitivePropertyMap> PropertyMapFactory::CreatePrimitivePropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column)
    {
    return PrimitivePropertyMap::CreateInstance(classMap, ecProperty, column);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<PrimitivePropertyMap> PropertyMapFactory::CreatePrimitivePropertyMap(ECN::PrimitiveECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& column)
    {
    return PrimitivePropertyMap::CreateInstance(ecProperty, parentPropertyMap, column);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<PrimitiveArrayPropertyMap> PropertyMapFactory::CreatePrimitiveArrayPropertyMap(ClassMap const& classMap, ECN::ArrayECPropertyCR ecProperty, DbColumn const& column)
    {
    return PrimitiveArrayPropertyMap::CreateInstance(classMap, ecProperty, column);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<PrimitiveArrayPropertyMap> PropertyMapFactory::CreatePrimitiveArrayPropertyMap(ECN::ArrayECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& column)
    {
    return PrimitiveArrayPropertyMap::CreateInstance(ecProperty, parentPropertyMap, column);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<StructPropertyMap> PropertyMapFactory::CreateStructPropertyMap(ClassMap const& classMap, ECN::StructECPropertyCR ecProperty)
    {
    return StructPropertyMap::CreateInstance(classMap, ecProperty);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<StructPropertyMap> PropertyMapFactory::CreateStructPropertyMap(ECN::StructECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap)
    {
    return StructPropertyMap::CreateInstance(ecProperty, parentPropertyMap);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<StructArrayPropertyMap> PropertyMapFactory::CreateStructArrayPropertyMap(ClassMap const& classMap, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column)
    {
    return StructArrayPropertyMap::CreateInstance(classMap, ecProperty, column);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<StructArrayPropertyMap> PropertyMapFactory::CreateStructArrayPropertyMap(ECN::StructArrayECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& column)
    {
    return StructArrayPropertyMap::CreateInstance(ecProperty, parentPropertyMap, column);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<Point2dPropertyMap> PropertyMapFactory::CreatePoint2dPropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y)
    {
    return Point2dPropertyMap::CreateInstance(classMap, ecProperty, x, y);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<Point2dPropertyMap> PropertyMapFactory::CreatePoint2dPropertyMap(ECN::PrimitiveECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& x, DbColumn const& y)
    {
    return Point2dPropertyMap::CreateInstance(ecProperty, parentPropertyMap, x, y);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<Point3dPropertyMap> PropertyMapFactory::CreatePoint3dPropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y, DbColumn const& z)
    {
    return Point3dPropertyMap::CreateInstance(classMap, ecProperty, x, y, z);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+==============+===============+===============+===============+===============+======
RefCountedPtr<Point3dPropertyMap> PropertyMapFactory::CreatePoint3dPropertyMap(ECN::PrimitiveECPropertyCR ecProperty, DataPropertyMap const& parentPropertyMap, DbColumn const& x, DbColumn const& y, DbColumn const& z)
    {
    return Point3dPropertyMap::CreateInstance(ecProperty, parentPropertyMap, x, y, z);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+==============+===============+===============+===============+===============+======
RefCountedPtr<NavigationPropertyMap> PropertyMapFactory::CreateNavigationPropertyMap(ClassMap const& classMap, ECN::NavigationECPropertyCR ecProperty)
    {
    return NavigationPropertyMap::CreateInstance(classMap, ecProperty);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<ECInstanceIdPropertyMap> PropertyMapFactory::CreateECInstanceIdPropertyMap(ClassMap const& classMap, std::vector<DbColumn const*> const& columns)
    {
    return ECInstanceIdPropertyMap::CreateInstance(classMap, columns);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<ECClassIdPropertyMap> PropertyMapFactory::CreateECClassIdPropertyMap(ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns)
    {
    return ECClassIdPropertyMap::CreateInstance(classMap,defaultEClassId, columns);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<ConstraintECClassIdPropertyMap> PropertyMapFactory::CreateSourceECClassIdPropertyMap(ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns)
    {
    return ConstraintECClassIdPropertyMap::CreateInstance(classMap, defaultEClassId , ConstraintECClassIdPropertyMap::ConstraintType::Source, columns);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<ConstraintECClassIdPropertyMap> PropertyMapFactory::CreateTargetECClassIdPropertyMap(ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns)
    {
    return ConstraintECClassIdPropertyMap::CreateInstance(classMap, defaultEClassId, ConstraintECClassIdPropertyMap::ConstraintType::Target, columns);

    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<ConstraintECInstanceIdPropertyMap> PropertyMapFactory::CreateSourceECInstanceIdPropertyMap(ClassMap const& classMap, std::vector<DbColumn const*> const& columns)
    {
    return ConstraintECInstanceIdPropertyMap::CreateInstance(classMap, ConstraintECInstanceIdPropertyMap::ConstraintType::Source, columns);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<ConstraintECInstanceIdPropertyMap> PropertyMapFactory::CreateTargetECInstanceIdPropertyMap(ClassMap const& classMap, std::vector<DbColumn const*> const& columns)
    {
    return ConstraintECInstanceIdPropertyMap::CreateInstance(classMap, ConstraintECInstanceIdPropertyMap::ConstraintType::Target, columns);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<SystemPropertyMap> PropertyMapFactory::CreateCopy(SystemPropertyMap const& propertyMap, ClassMap const& newContext)
    {
    std::vector<DbColumn const*> columns;
    for (SingleColumnDataPropertyMap const* child : propertyMap.GetVerticalPropertyMaps())
        {
        columns.push_back(&child->GetColumn());
        }

    if (auto pm = dynamic_cast<ECInstanceIdPropertyMap const*>(&propertyMap))
        {
        return CreateECInstanceIdPropertyMap(newContext, columns);
        }

    if (auto pm = dynamic_cast<ECClassIdPropertyMap const*>(&propertyMap))
        {
        return CreateECClassIdPropertyMap(newContext, pm->GetDefaultECClassId(), columns);
        }

    if (auto pm = dynamic_cast<ConstraintECInstanceIdPropertyMap const*>(&propertyMap))
        {
        if (pm->IsSource())
            return CreateSourceECInstanceIdPropertyMap(newContext, columns);

        return CreateTargetECInstanceIdPropertyMap(newContext, columns);
        }

    if (auto pm = dynamic_cast<ConstraintECClassIdPropertyMap const*>(&propertyMap))
        {
        if (pm->IsSource())
            return CreateSourceECClassIdPropertyMap(newContext, pm->GetDefaultECClassId(), columns);

        return CreateTargetECClassIdPropertyMap(newContext, pm->GetDefaultECClassId(), columns);
        }

    BeAssert(false && "Unhandled case");
    return nullptr;
    }
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
 RefCountedPtr<ConstraintECInstanceIdPropertyMap> PropertyMapFactory::CreateConstraintECInstanceIdPropertyMap(ECN::ECRelationshipEnd end, ClassMap const& classMap, std::vector<DbColumn const*> const& columns)
     {
     if (end == ECRelationshipEnd_Source)
         return CreateSourceECInstanceIdPropertyMap(classMap, columns);

     return CreateTargetECInstanceIdPropertyMap(classMap, columns);
     }

 //=======================================================================================
 // @bsimethod                                                   Affan.Khan          07/16
 //+===============+===============+===============+===============+===============+======
 RefCountedPtr<ConstraintECClassIdPropertyMap> PropertyMapFactory::CreateConstraintECClassIdPropertyMap(ECN::ECRelationshipEnd end, ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns)
     {
     if (end == ECRelationshipEnd_Source)
         return CreateSourceECClassIdPropertyMap(classMap, defaultEClassId, columns);

     return CreateTargetECClassIdPropertyMap(classMap, defaultEClassId, columns);
     }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<DataPropertyMap> PropertyMapFactory::CreateCopy(DataPropertyMap const& propertyMap, ClassMap const& newContext)
    {
    if (propertyMap.GetParent() != nullptr)
        {
        BeAssert(false && "Expecting a root property map");
        return nullptr;
        }

    return CreateCopy(propertyMap, newContext, nullptr);
    }
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<DataPropertyMap> PropertyMapFactory::CreateCopy(DataPropertyMap const& propertyMap, ClassMap const& newContext, DataPropertyMap const* newParent)
    {
    RefCountedPtr<DataPropertyMap> copy;
    if (Point2dPropertyMap const* typed = dynamic_cast<Point2dPropertyMap const*>(&propertyMap))
        {
        PrimitiveECPropertyCP prop = typed->GetProperty().GetAsPrimitiveProperty();
        if (newParent)
            copy = CreatePoint2dPropertyMap( *prop, *newParent, typed->GetX().GetColumn(), typed->GetY().GetColumn());
        else
            copy = CreatePoint2dPropertyMap(newContext, *prop, typed->GetX().GetColumn(), typed->GetY().GetColumn());

        copy->FinishEditing();
        }
    else if (Point3dPropertyMap const* typed = dynamic_cast<Point3dPropertyMap const*>(&propertyMap))
        {
        PrimitiveECPropertyCP prop = typed->GetProperty().GetAsPrimitiveProperty();
        if (newParent)
            copy = CreatePoint3dPropertyMap(*prop, *newParent, typed->GetX().GetColumn(), typed->GetY().GetColumn(), typed->GetZ().GetColumn());
        else
            copy = CreatePoint3dPropertyMap(newContext, *prop, typed->GetX().GetColumn(), typed->GetY().GetColumn(), typed->GetZ().GetColumn());

        copy->FinishEditing();
        }
    else if (PrimitivePropertyMap const* typed = dynamic_cast<PrimitivePropertyMap const*>(&propertyMap))
        {
        PrimitiveECPropertyCP prop = typed->GetProperty().GetAsPrimitiveProperty();
        if (newParent)
            copy = CreatePrimitivePropertyMap(*prop, *newParent, typed->GetColumn());
        else
            copy = CreatePrimitivePropertyMap(newContext, *prop, typed->GetColumn());

        copy->FinishEditing();
        }
    else if (PrimitiveArrayPropertyMap const* typed = dynamic_cast<PrimitiveArrayPropertyMap const*>(&propertyMap))
        {
        ArrayECPropertyCP prop = typed->GetProperty().GetAsArrayProperty();
        if (newParent)
            copy = CreatePrimitiveArrayPropertyMap(*prop, *newParent, typed->GetColumn());
        else
            copy = CreatePrimitiveArrayPropertyMap(newContext, *prop, typed->GetColumn());

        copy->FinishEditing();
        }
    else if (StructPropertyMap const* typed = dynamic_cast<StructPropertyMap const*>(&propertyMap))
        {
        RefCountedPtr<StructPropertyMap> st;        
        StructECPropertyCP prop = typed->GetProperty().GetAsStructProperty();
        if (newParent)
            st = CreateStructPropertyMap(*prop, *newParent);
        else
            st = CreateStructPropertyMap(newContext, *prop);

        for (DataPropertyMap const* child : *typed)
            {
            RefCountedPtr<DataPropertyMap> childMap = CreateCopy(*child, newContext, st.get());
            if (childMap == nullptr)
                {
                BeAssert(false && "Failed to create copy child map");
                return nullptr;
                }

            if (st->Insert(childMap) != SUCCESS)
                {
                BeAssert(false && "Failed to insert property map");
                return nullptr;
                }
            }

        copy = st;
        copy->FinishEditing();
        }
    else if (StructArrayPropertyMap const* typed = dynamic_cast<StructArrayPropertyMap const*>(&propertyMap))
        {
        StructArrayECPropertyCP prop = typed->GetProperty().GetAsStructArrayProperty();
        if (newParent)
            copy = CreateStructArrayPropertyMap(*prop, *newParent, typed->GetColumn());
        else
            copy = CreateStructArrayPropertyMap(newContext, *prop, typed->GetColumn());

        copy->FinishEditing();
        }
    else if (NavigationPropertyMap const* typed = dynamic_cast<NavigationPropertyMap const*>(&propertyMap))
        {
        RefCountedPtr<NavigationPropertyMap> nav;
        NavigationECPropertyCP prop = typed->GetProperty().GetAsNavigationProperty();
        if (newParent)
            {
            BeAssert(false && "Navigator property map can only be defined on the class directly");
            return nullptr;
            }
        else
            nav = CreateNavigationPropertyMap(newContext, *prop);

        if (typed->Validate() == SUCCESS)
            {
            NavigationPropertyMap::RelECClassIdPropertyMap const& relId = typed->GetRelECClassId();
            NavigationPropertyMap::IdPropertyMap const& id = typed->GetId();

            if (nav->Initialize(relId.GetColumn(), relId.GetDefaultClassId(), id.GetColumn()) != SUCCESS)
                {
                BeAssert(false && "Failed to setup navigation property");
                return nullptr;
                }
            }

        copy = nav;
        }
    else
        {
        BeAssert(false && "Unhandled property map type");
        return nullptr;
        }

    return copy;
    }


END_BENTLEY_SQLITE_EC_NAMESPACE
