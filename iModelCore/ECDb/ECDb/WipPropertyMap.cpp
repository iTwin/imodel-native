/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/WipPropertyMap.cpp $
|
|  $Copyright: (c) 2016 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

using namespace std;
USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//***************************************************WipECDbPropertyMapper*****************************************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
BentleyStatus WipPropertyMapContainer::Insert(RefCountedPtr<WipPropertyMap> propertyMap, size_t position)
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

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
DispatcherFeedback WipPropertyMapContainer::_Accept(IPropertyMapDispatcher const&  dispatcher)  const
    {
    DispatcherFeedback fb = DispatcherFeedback::Next;
    for (WipPropertyMap const* propertyMap : m_directDecendentList)
        {
        fb = propertyMap->Accept(dispatcher);
        if (fb == DispatcherFeedback::Cancel)
            return fb;
        }

    return fb;
    }
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
WipPropertyMap const* WipPropertyMapContainer::Find(Utf8CP accessString, bool recusive) const
    {
    auto itor = m_map.find(accessString);
    if (itor != m_map.end())
        return itor->second.get();

    if (recusive)
        for (WipPropertyMap const* child : m_directDecendentList)
            if (auto collection = dynamic_cast<WipCompoundPropertyMap const*>(child))
                if (auto result = collection->Find(accessString, true))
                    return result;

    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
ECN::ECClass const& WipPropertyMapContainer::GetClass() const { return m_classMap.GetClass(); }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
ECDbCR WipPropertyMapContainer::GetECDb() const { return m_classMap.GetDbMap().GetECDb(); }

//************************************WipPropertyMap*************************************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
WipPropertyMap::WipPropertyMap(ClassMap const& classMap, ECN::ECPropertyCR ecProperty)
    : m_ecProperty(ecProperty), m_classMap(classMap), m_parentPropertMap(nullptr), m_propertyAccessString(ecProperty.GetName()), m_isInEditMode(true)
    {}

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
WipPropertyMap::WipPropertyMap(ECN::ECPropertyCR ecProperty, WipPropertyMap const& parentPropertyMap)
    : m_ecProperty(ecProperty), m_classMap(parentPropertyMap.GetClassMap()), m_parentPropertMap(&parentPropertyMap), m_propertyAccessString(parentPropertyMap.GetAccessString() + EC_ACCESSSTRING_DELIMITER + ecProperty.GetName()), m_isInEditMode(true)
    {}

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
WipPropertyMap const& WipPropertyMap::GetRoot() const
    {
    WipPropertyMap const* root = this;
    while (this->GetParent() != nullptr)
        root = this->GetParent();

    return *root;
    }

//************************************WipCompoundPropertyMap::Collection********
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
WipVerticalPropertyMap const* WipCompoundPropertyMap::Collection::Find(Utf8CP accessString) const
    {
    auto itor = m_map.find(accessString);
    if (itor != m_map.end())
        return itor->second.get();
  
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
DispatcherFeedback WipCompoundPropertyMap::AcceptChildren(IPropertyMapDispatcher const&  dispatcher) const
    {
    DispatcherFeedback fb = DispatcherFeedback::Next;
    for (WipVerticalPropertyMap const* pm : *this)
        {
        fb = pm->Accept(dispatcher);
        if (fb == DispatcherFeedback::Cancel)
            return fb;
        }

    return fb;
    }
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
WipVerticalPropertyMap const* WipCompoundPropertyMap::Find(Utf8CP accessString, bool recusive) const
    {
    if (auto result = m_col.Find(accessString))
        return result;

    if (recusive)
        for (WipVerticalPropertyMap const* child : m_col.GetList())
            if (auto collection = dynamic_cast<WipCompoundPropertyMap const*>(child))
                if (auto result = collection->Find(accessString, true))
                    return result;

    return nullptr;
    }
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
BentleyStatus WipCompoundPropertyMap::Collection::Insert(RefCountedPtr<WipVerticalPropertyMap> propertyMap, WipVerticalPropertyMap const& parent, size_t position)
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

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
BentleyStatus WipCompoundPropertyMap::Collection::Remove(Utf8CP accessString)
    {
    if (WipVerticalPropertyMap const* r = Find(accessString))
        {
        m_list.erase(std::find(m_list.begin(), m_list.end(), r));
        m_map.erase(accessString);
        return SUCCESS;
        }

    return ERROR;
    }
//************************************WipVerticalPropertyMap********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipVerticalPropertyMap> WipVerticalPropertyMap::CreateCopy(ClassMap const& newClassMapContext) const
    {
    return WipPropertyMapFactory::CreateCopy(*this, newClassMapContext);
    }

//************************************WipCompoundPropertyMap********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
BentleyStatus WipCompoundPropertyMap::_Validate() const 
    {
    if (empty())
        {
        BeAssert(false);
        return ERROR;
        }
    for (WipVerticalPropertyMap const* child : *this)
        {
        if (child->Validate() != BentleyStatus::SUCCESS)
            return ERROR;
        }

    return IsReadonly() ? SUCCESS : ERROR;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
DbTable const& WipCompoundPropertyMap::_GetTable() const 
    {
    if (empty())
        {
        if (GetParent() == nullptr)
            {
            BeAssert(false);
            return *static_cast<DbTable const*>(nullptr);
            }
        else
            return static_cast<WipVerticalPropertyMap const*>(GetParent())->GetTable();
        }

    return m_col.Front()->GetTable();
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
BentleyStatus WipCompoundPropertyMap::VerifyVerticalIntegerity(WipVerticalPropertyMap const& propertyMap) const
    {
    if (empty())
        {
        if (GetParent() != nullptr)
            {
            DbTable const& parentTable = static_cast<WipVerticalPropertyMap const*>(GetParent())->GetTable();
            if (&propertyMap.GetTable() != &parentTable)
                {
                BeAssert(false && "Table must match parent table");
                return ERROR;
                }
            }
        }
    else
        {
        if (&m_col.Front()->GetTable() != &propertyMap.GetTable())
            {
            BeAssert(false && "All property map must have same table");
            return ERROR;
            }
        }

    return SUCCESS;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
void WipCompoundPropertyMap::Clear()
    {
    if (m_readonly)
        {
        BeAssert(false && "PropertyMap is readonly");
        return;
        }

    m_col.Clear();
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
BentleyStatus WipCompoundPropertyMap::Remove(Utf8CP accessString)
    {
    if (m_readonly)
        {
        BeAssert(false && "property map cannot be removed from this readonly collection");
        return ERROR;
        }
    return m_col.Remove(accessString);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
BentleyStatus WipCompoundPropertyMap::Insert(RefCountedPtr<WipVerticalPropertyMap> propertyMap, size_t position)
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
    if (VerifyVerticalIntegerity(*propertyMap) != SUCCESS)
        return ERROR;

    return m_col.Insert(propertyMap, *this, position);
    }
//************************************WipColumnHorizontalPropertyMap********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
BentleyStatus WipColumnHorizontalPropertyMap::_Validate() const
    {
    BeAssert(!m_vmapsPerTable.empty());
    if (m_vmapsPerTable.empty())
        return ERROR;

    return SUCCESS;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
WipColumnHorizontalPropertyMap::WipColumnHorizontalPropertyMap(ClassMap const& classMap, ECN::ECPropertyCR ecProperty, std::vector<RefCountedPtr<WipColumnVerticalPropertyMap>> const& maps)
    : WipHorizontalPropertyMap(classMap, ecProperty)
    {
    for (RefCountedPtr<WipColumnVerticalPropertyMap> const& map : maps)
        {
        if (m_vmapsPerTable.find(map->GetTable().GetName().c_str()) != m_vmapsPerTable.end())
            {
            BeAssert(false && "PropertyMap must be one per table");
            m_vmapsPerTable.clear();
            m_vmaps.clear();
            return;
            }

        m_vmapsPerTable[map->GetTable().GetName().c_str()] = map;
        m_vmaps.push_back(map.get());
        }
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
WipColumnVerticalPropertyMap const* WipColumnHorizontalPropertyMap::FindVerticalPropertyMap(Utf8CP tableName) const
    {
    auto itor = m_vmapsPerTable.find(tableName);
    if (itor != m_vmapsPerTable.end())
        return itor->second.get();

    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
WipColumnVerticalPropertyMap const* WipColumnHorizontalPropertyMap::FindVerticalPropertyMap(DbTable const& table) const
    {
    return FindVerticalPropertyMap(table.GetName().c_str());
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
std::vector<WipColumnVerticalPropertyMap const*> const& WipColumnHorizontalPropertyMap::GetVerticalPropertyMaps() const
    {
    return m_vmaps;
    }

//************************************WipColumnHorizontalPropertyMap********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
BentleyStatus WipSystemPropertyMap::TryCreateVerticalMaps(std::vector<RefCountedPtr<WipPrimitivePropertyMap>>& propertyMaps, ECSqlSystemProperty systemProperty, ClassMap const& classMap, std::vector<DbColumn const*> const& columns)
    {
    ECDbSchemaManagerCR schemaManger = classMap.GetDbMap().GetECDb().Schemas();
    ECPropertyCP ecProperty = ECDbSystemSchemaHelper::GetSystemProperty(schemaManger, systemProperty);
    if (ecProperty == nullptr)
        {
        BeAssert(false && "Failed to find system property");
        return ERROR;
        }
    PrimitiveECPropertyCP primtiveECProp = ecProperty->GetAsPrimitiveProperty();
    if (primtiveECProp == nullptr || primtiveECProp->GetType() != PRIMITIVETYPE_Long)
        {
        BeAssert(false && "System property must correspond to a primitive property of type long");
        return ERROR;
        }

    if (columns.empty())
        {
        BeAssert(false && "Columns cannot be empty");
        return ERROR;
        }

    std::set<DbTable const*> doneList;
    for (DbColumn const* column : columns)
        {
        if (column->GetType() != DbColumn::Type::Integer)
            {
            BeAssert(false && "System column must have type integer");
            return ERROR;
            }

        if (doneList.find(&column->GetTable()) != doneList.end())
            {
            BeAssert(false && "Column must unique per table");
            return ERROR;
            }

        doneList.insert(&column->GetTable());
        auto prop = WipPrimitivePropertyMap::CreateInstance(classMap, *primtiveECProp, *column);
        if (prop == nullptr)
            {
            BeAssert(false && "Failed to create property Map");
            return ERROR;
            }

        propertyMaps.push_back(prop);
        }

    return SUCCESS;
    }
//************************************WipECInstanceIdPropertyMap********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipECInstanceIdPropertyMap> WipECInstanceIdPropertyMap::CreateInstance(ClassMap const& classMap, std::vector<DbColumn const*> const& columns)
    {
    std::vector<RefCountedPtr<WipPrimitivePropertyMap>> propertyMaps;
    if (TryCreateVerticalMaps(propertyMaps, ECSqlSystemProperty::ECInstanceId, classMap, columns) != SUCCESS)
        return nullptr;

    PrimitiveECPropertyCP systemProperty = propertyMaps.front()->GetProperty().GetAsPrimitiveProperty();
    return new WipECInstanceIdPropertyMap(classMap, *systemProperty, propertyMaps);
    }
//************************************WipECClassIdPropertyMap********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipECClassIdPropertyMap> WipECClassIdPropertyMap::CreateInstance(ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns)
    {
    std::vector<RefCountedPtr<WipPrimitivePropertyMap>> propertyMaps;
    if (TryCreateVerticalMaps(propertyMaps, ECSqlSystemProperty::ECClassId, classMap, columns) != SUCCESS)
        return nullptr;

    PrimitiveECPropertyCP systemProperty = propertyMaps.front()->GetProperty().GetAsPrimitiveProperty();
    return new WipECClassIdPropertyMap(classMap, *systemProperty, propertyMaps, defaultEClassId);
    }

//************************************WipConstraintECClassIdPropertyMap********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipConstraintECClassIdPropertyMap> WipConstraintECClassIdPropertyMap::CreateInstance(ClassMap const& classMap, ECN::ECClassId defaultEClassId, ConstraintType constraintType, std::vector<DbColumn const*> const& columns)
    {
    std::vector<RefCountedPtr<WipPrimitivePropertyMap>> propertyMaps;

    if (TryCreateVerticalMaps(propertyMaps, constraintType == ConstraintType::Source ? ECSqlSystemProperty::SourceECClassId : ECSqlSystemProperty::TargetECClassId, classMap, columns) != SUCCESS)
        return nullptr;

    PrimitiveECPropertyCP systemProperty = propertyMaps.front()->GetProperty().GetAsPrimitiveProperty();
    return new WipConstraintECClassIdPropertyMap(classMap, *systemProperty, propertyMaps, defaultEClassId, constraintType);
    }

//************************************WipConstraintECClassIdPropertyMap********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static
RefCountedPtr<WipConstraintECInstanceIdIdPropertyMap> WipConstraintECInstanceIdIdPropertyMap::CreateInstance(ClassMap const& classMap, ConstraintType constraintType, std::vector<DbColumn const*> const& columns)
    {
    std::vector<RefCountedPtr<WipPrimitivePropertyMap>> propertyMaps;
    if (TryCreateVerticalMaps(propertyMaps, constraintType == ConstraintType::Source ? ECSqlSystemProperty::SourceECInstanceId : ECSqlSystemProperty::TargetECInstanceId, classMap, columns) != SUCCESS)
        return nullptr;

    PrimitiveECPropertyCP systemProperty = propertyMaps.front()->GetProperty().GetAsPrimitiveProperty();
    return new WipConstraintECInstanceIdIdPropertyMap(classMap, *systemProperty, propertyMaps, constraintType);
    }
//************************************WipPrimitivePropertyMap********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
BentleyStatus WipPrimitivePropertyMap::_Validate() const
    {
    ECN::PrimitiveECPropertyCP property = GetProperty().GetAsPrimitiveProperty();
    return property->GetType() != PRIMITIVETYPE_Point2d && property->GetType() != PRIMITIVETYPE_Point3d ? SUCCESS : ERROR;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipPrimitivePropertyMap> WipPrimitivePropertyMap::CreateInstance(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column)
    {
    if (ecProperty.GetType() == PRIMITIVETYPE_Point2d || ecProperty.GetType() == PRIMITIVETYPE_Point3d)
        {
        BeAssert(false && "Do not support Point2d or Point3d property");
        return nullptr;
        }

    return new WipPrimitivePropertyMap(classMap, ecProperty, column);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipPrimitivePropertyMap> WipPrimitivePropertyMap::CreateInstance(ECN::PrimitiveECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column)
    {
    if (ecProperty.GetType() == PRIMITIVETYPE_Point2d || ecProperty.GetType() == PRIMITIVETYPE_Point3d)
        {
        BeAssert(false && "Do not support Point2d or Point3d property");
        return nullptr;
        }

    return new WipPrimitivePropertyMap(ecProperty, parentPropertyMap, column);
    }
//************************************WipPrimitivePropertyMap********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipPrimitiveArrayPropertyMap> WipPrimitiveArrayPropertyMap::CreateInstance(ClassMap const& classMap, ECN::ArrayECPropertyCR ecProperty, DbColumn const& column)
    {
    return new WipPrimitiveArrayPropertyMap(classMap, ecProperty, column);
    }
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipPrimitiveArrayPropertyMap> WipPrimitiveArrayPropertyMap::CreateInstance(ECN::ArrayECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column)
    {
    return new WipPrimitiveArrayPropertyMap(ecProperty, parentPropertyMap, column);
    }

//************************************WipStructPropertyMap********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipStructPropertyMap> WipStructPropertyMap::CreateInstance(ClassMap const& classMap, ECN::StructECPropertyCR ecProperty)
    {
    return new WipStructPropertyMap(classMap, ecProperty);
    }
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
DispatcherFeedback WipStructPropertyMap::_Accept(IPropertyMapDispatcher const&  dispatcher)  const
    {
    DispatcherFeedback fb = dispatcher.Dispatch(PropertyMapType::StructPropertyMap, *this);
    if (fb == DispatcherFeedback::Cancel || fb == DispatcherFeedback::NextSibling)
        return fb;

    return AcceptChildren(dispatcher);
    }
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipStructPropertyMap> WipStructPropertyMap::CreateInstance(ECN::StructECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap)
    {
    return new WipStructPropertyMap(ecProperty, parentPropertyMap);
    }
//************************************WipStructArrayPropertyMap********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipStructArrayPropertyMap> WipStructArrayPropertyMap::CreateInstance(ClassMap const& classMap, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column)
    {
    return new WipStructArrayPropertyMap(classMap, ecProperty, column);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipStructArrayPropertyMap> WipStructArrayPropertyMap::CreateInstance(ECN::StructArrayECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column)
    {
    return new WipStructArrayPropertyMap(ecProperty, parentPropertyMap, column);
    }

//************************************WipNavigationPropertyMap::RelECClassIdPropertyMap********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipNavigationPropertyMap::RelECClassIdPropertyMap> WipNavigationPropertyMap::RelECClassIdPropertyMap::CreateInstance(WipNavigationPropertyMap const& parentPropertyMap, DbColumn const& column, ECN::ECClassId defaultRelClassId)
    {
    ECDbSchemaManagerCR schemaManger = parentPropertyMap.GetClassMap().GetDbMap().GetECDb().Schemas();
    ECPropertyCP relECClassIdProp = ECDbSystemSchemaHelper::GetSystemProperty(schemaManger, ECSqlSystemProperty::RelECClassId);
    if (relECClassIdProp == nullptr)
        return nullptr;

    return  new RelECClassIdPropertyMap(*relECClassIdProp->GetAsPrimitiveProperty(), parentPropertyMap, column, defaultRelClassId);
    }

//************************************WipNavigationPropertyMap::IdPropertyMap********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static
RefCountedPtr<WipNavigationPropertyMap::IdPropertyMap> WipNavigationPropertyMap::IdPropertyMap::CreateInstance(WipNavigationPropertyMap const& parentPropertyMap, DbColumn const& column)
    {
    ECDbSchemaManagerCR schemaManger = parentPropertyMap.GetClassMap().GetDbMap().GetECDb().Schemas();
    ECPropertyCP idProp = ECDbSystemSchemaHelper::GetSystemProperty(schemaManger, ECSqlSystemProperty::Id);
    if (idProp == nullptr)
        return nullptr;

    return new IdPropertyMap(*idProp->GetAsPrimitiveProperty(), parentPropertyMap, column);
    }


//************************************WipNavigationPropertyMap********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
BentleyStatus WipNavigationPropertyMap::Init(DbColumn const& relECClassIdColumn, ECN::ECClassId defaultRelClassId, DbColumn const& idColumn)
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
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
bool WipNavigationPropertyMap::IsSupportedInECSql(bool logIfNotSupported = false) const { return ClassMapper::IsNavigationPropertySupportedInECSql(*this, logIfNotSupported); }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
DispatcherFeedback WipNavigationPropertyMap::_Accept(IPropertyMapDispatcher const&  dispatcher)  const
    {
    DispatcherFeedback fb = dispatcher.Dispatch(PropertyMapType::NavigationPropertyMap, *this);
    if (fb == DispatcherFeedback::Cancel || fb == DispatcherFeedback::NextSibling)
        return fb;

    return AcceptChildren(dispatcher);
    }
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
BentleyStatus WipNavigationPropertyMap::_Validate() const
    {
    if (size() != 2)
        return ERROR;

    return WipCompoundPropertyMap::_Validate();
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
BentleyStatus WipNavigationPropertyMap::Setup(DbColumn const& relECClassIdColumn, ECN::ECClassId defaultRelClassId, DbColumn const& idColumn)
    {
    return Init(relECClassIdColumn, defaultRelClassId, idColumn);
    }


//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipNavigationPropertyMap> WipNavigationPropertyMap::CreateInstance(ClassMap const& classMap, ECN::NavigationECPropertyCR ecProperty)
    {
    return new WipNavigationPropertyMap(classMap, ecProperty);
    }


//************************************WipPoint2dPropertyMap********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
BentleyStatus WipPoint2dPropertyMap::Init(DbColumn const& x, DbColumn const& y)
    {
    if (&x.GetTable() != &y.GetTable())
        {
        return ERROR;
        }

    ECDbSchemaManagerCR schemaManger = GetClassMap().GetDbMap().GetECDb().Schemas();
    ECPropertyCP propX = ECDbSystemSchemaHelper::GetSystemProperty(schemaManger, ECSqlSystemProperty::X);
    if (propX == nullptr)
        return ERROR;

    RefCountedPtr<WipPrimitivePropertyMap> xPropertyMap = WipPrimitivePropertyMap::CreateInstance(*propX->GetAsPrimitiveProperty(), *this, x);
    if (xPropertyMap == nullptr)
        return ERROR;

    if(Insert(xPropertyMap) != SUCCESS)
        return ERROR;

    ECPropertyCP propY = ECDbSystemSchemaHelper::GetSystemProperty(schemaManger, ECSqlSystemProperty::Y);
    if (propY == nullptr)
        return ERROR;

    RefCountedPtr<WipPrimitivePropertyMap> yPropertyMap = WipPrimitivePropertyMap::CreateInstance(*propY->GetAsPrimitiveProperty(), *this, y);
    if (yPropertyMap == nullptr)
        return ERROR;

    if(Insert(yPropertyMap) != SUCCESS)
        return ERROR;

    MakeReadOnly();
    return SUCCESS;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
DispatcherFeedback WipPoint2dPropertyMap::_Accept(IPropertyMapDispatcher const&  dispatcher)  const
    {
    DispatcherFeedback fb = dispatcher.Dispatch(PropertyMapType::Point2dPropertyMap, *this);
    if (fb == DispatcherFeedback::Cancel || fb == DispatcherFeedback::NextSibling)
        return fb;

    return AcceptChildren(dispatcher);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
BentleyStatus WipPoint2dPropertyMap::_Validate() const
    {
    return size() == 2 && IsReadonly() ? SUCCESS : ERROR;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipPoint2dPropertyMap> WipPoint2dPropertyMap::CreateInstance(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y)
    {
    if (ecProperty.GetType() != PrimitiveType::PRIMITIVETYPE_Point2d)
        {
        BeAssert(false && "PrimitiveType must be Point2d");
        return nullptr;
        }

    RefCountedPtr<WipPoint2dPropertyMap> ptr = new WipPoint2dPropertyMap(classMap, ecProperty);
    if (ptr->Init(x, y) != SUCCESS)
        return nullptr;

    return ptr;
    }
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipPoint2dPropertyMap> WipPoint2dPropertyMap::CreateInstance(ECN::PrimitiveECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& x, DbColumn const& y)
    {
    if (ecProperty.GetType() != PrimitiveType::PRIMITIVETYPE_Point2d)
        {
        BeAssert(false && "PrimitiveType must be Point2d");
        return nullptr;
        }

    RefCountedPtr<WipPoint2dPropertyMap> ptr = new WipPoint2dPropertyMap(ecProperty, parentPropertyMap);
    if (ptr->Init(x, y) != SUCCESS)
        return nullptr;

    return ptr;
    }
//************************************WipPoint3dPropertyMap********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
BentleyStatus WipPoint3dPropertyMap::Init(DbColumn const& x, DbColumn const& y, DbColumn const& z)
    {
    ECDbSchemaManagerCR schemaManger = GetClassMap().GetDbMap().GetECDb().Schemas();
    ECPropertyCP propX = ECDbSystemSchemaHelper::GetSystemProperty(schemaManger, ECSqlSystemProperty::X);
    if (propX == nullptr)
        return ERROR;

    RefCountedPtr<WipPrimitivePropertyMap> xPropertyMap = WipPrimitivePropertyMap::CreateInstance(*propX->GetAsPrimitiveProperty(), *this, x);
    if (xPropertyMap == nullptr)
        return ERROR;

    if(Insert(xPropertyMap) != SUCCESS)
        return ERROR;

    ECPropertyCP propY = ECDbSystemSchemaHelper::GetSystemProperty(schemaManger, ECSqlSystemProperty::Y);
    if (propY == nullptr)
        return ERROR;

    RefCountedPtr<WipPrimitivePropertyMap> yPropertyMap = WipPrimitivePropertyMap::CreateInstance(*propY->GetAsPrimitiveProperty(), *this, y);
    if (yPropertyMap == nullptr)
        return ERROR;

    if(Insert(yPropertyMap) != SUCCESS)
        return ERROR;

    ECPropertyCP propZ = ECDbSystemSchemaHelper::GetSystemProperty(schemaManger, ECSqlSystemProperty::Z);
    if (propZ == nullptr)
        return ERROR;

    RefCountedPtr<WipPrimitivePropertyMap> zPropertyMap = WipPrimitivePropertyMap::CreateInstance(*propZ->GetAsPrimitiveProperty(), *this, z);
    if (zPropertyMap == nullptr)
        return ERROR;

    if(Insert(zPropertyMap) != SUCCESS)
        return ERROR;

    MakeReadOnly();
    return SUCCESS;
    }
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
DispatcherFeedback WipPoint3dPropertyMap::_Accept(IPropertyMapDispatcher const&  dispatcher)  const
    {
    DispatcherFeedback fb = dispatcher.Dispatch(PropertyMapType::Point3dPropertyMap, *this);
    if (fb == DispatcherFeedback::Cancel || fb == DispatcherFeedback::NextSibling)
        return fb;

    return AcceptChildren(dispatcher);
    }
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
BentleyStatus WipPoint3dPropertyMap::_Validate() const
    {
    return size() == 3 && IsReadonly() ? SUCCESS : ERROR;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipPoint3dPropertyMap> WipPoint3dPropertyMap::CreateInstance(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y, DbColumn const& z)
    {
    if (ecProperty.GetType() != PrimitiveType::PRIMITIVETYPE_Point3d)
        {
        BeAssert(false && "PrimitiveType must be Point3d");
        return nullptr;
        }

    RefCountedPtr<WipPoint3dPropertyMap> ptr = new WipPoint3dPropertyMap(classMap, ecProperty);
    if (ptr->Init(x, y, z) != SUCCESS)
        return nullptr;

    return ptr;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipPoint3dPropertyMap> WipPoint3dPropertyMap::CreateInstance(ECN::PrimitiveECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& x, DbColumn const& y, DbColumn const& z)
    {
    if (ecProperty.GetType() != PrimitiveType::PRIMITIVETYPE_Point3d)
        {
        BeAssert(false && "PrimitiveType must be Point3d");
        return nullptr;
        }

    RefCountedPtr<WipPoint3dPropertyMap> ptr = new WipPoint3dPropertyMap(ecProperty, parentPropertyMap);
    if (ptr->Init(x, y, z) != SUCCESS)
        return nullptr;

    return ptr;
    }

//************************************WipPropertyMapFactory********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipPrimitivePropertyMap> WipPropertyMapFactory::CreatePrimitivePropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column)
    {
    return WipPrimitivePropertyMap::CreateInstance(classMap, ecProperty, column);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipPrimitivePropertyMap> WipPropertyMapFactory::CreatePrimitivePropertyMap(ECN::PrimitiveECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column)
    {
    return WipPrimitivePropertyMap::CreateInstance(ecProperty, parentPropertyMap, column);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipPrimitiveArrayPropertyMap> WipPropertyMapFactory::CreatePrimitiveArrayPropertyMap(ClassMap const& classMap, ECN::ArrayECPropertyCR ecProperty, DbColumn const& column)
    {
    return WipPrimitiveArrayPropertyMap::CreateInstance(classMap, ecProperty, column);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipPrimitiveArrayPropertyMap> WipPropertyMapFactory::CreatePrimitiveArrayPropertyMap(ECN::ArrayECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column)
    {
    return WipPrimitiveArrayPropertyMap::CreateInstance(ecProperty, parentPropertyMap, column);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipStructPropertyMap> WipPropertyMapFactory::CreateStructPropertyMap(ClassMap const& classMap, ECN::StructECPropertyCR ecProperty)
    {
    return WipStructPropertyMap::CreateInstance(classMap, ecProperty);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipStructPropertyMap> WipPropertyMapFactory::CreateStructPropertyMap(ECN::StructECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap)
    {
    return WipStructPropertyMap::CreateInstance(ecProperty, parentPropertyMap);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipStructArrayPropertyMap> WipPropertyMapFactory::CreateStructArrayPropertyMap(ClassMap const& classMap, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column)
    {
    return WipStructArrayPropertyMap::CreateInstance(classMap, ecProperty, column);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipStructArrayPropertyMap> WipPropertyMapFactory::CreateStructArrayPropertyMap(ECN::StructArrayECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column)
    {
    return WipStructArrayPropertyMap::CreateInstance(ecProperty, parentPropertyMap, column);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipPoint2dPropertyMap> WipPropertyMapFactory::CreatePoint2dPropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y)
    {
    return WipPoint2dPropertyMap::CreateInstance(classMap, ecProperty, x, y);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipPoint2dPropertyMap> WipPropertyMapFactory::CreatePoint2dPropertyMap(ECN::PrimitiveECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& x, DbColumn const& y)
    {
    return WipPoint2dPropertyMap::CreateInstance(ecProperty, parentPropertyMap, x, y);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipPoint3dPropertyMap> WipPropertyMapFactory::CreatePoint3dPropertyMap(ClassMap const& classMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y, DbColumn const& z)
    {
    return WipPoint3dPropertyMap::CreateInstance(classMap, ecProperty, x, y, z);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+==============+===============+===============+===============+===============+======
RefCountedPtr<WipPoint3dPropertyMap> WipPropertyMapFactory::CreatePoint3dPropertyMap(ECN::PrimitiveECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& x, DbColumn const& y, DbColumn const& z)
    {
    return WipPoint3dPropertyMap::CreateInstance(ecProperty, parentPropertyMap, x, y, z);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+==============+===============+===============+===============+===============+======
RefCountedPtr<WipNavigationPropertyMap> WipPropertyMapFactory::CreateNavigationPropertyMap(ClassMap const& classMap, ECN::NavigationECPropertyCR ecProperty)
    {
    return WipNavigationPropertyMap::CreateInstance(classMap, ecProperty);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipECInstanceIdPropertyMap> WipPropertyMapFactory::CreateECInstanceIdPropertyMap(ClassMap const& classMap, std::vector<DbColumn const*> const& columns)
    {
    return WipECInstanceIdPropertyMap::CreateInstance(classMap, columns);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipECClassIdPropertyMap> WipPropertyMapFactory::CreateECClassIdPropertyMap(ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns)
    {
    return WipECClassIdPropertyMap::CreateInstance(classMap,defaultEClassId, columns);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipConstraintECClassIdPropertyMap> WipPropertyMapFactory::CreateSourceECClassIdPropertyMap(ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns)
    {
    return WipConstraintECClassIdPropertyMap::CreateInstance(classMap, defaultEClassId , WipConstraintECClassIdPropertyMap::ConstraintType::Source, columns);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipConstraintECClassIdPropertyMap> WipPropertyMapFactory::CreateTargetECClassIdPropertyMap(ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns)
    {
    return WipConstraintECClassIdPropertyMap::CreateInstance(classMap, defaultEClassId, WipConstraintECClassIdPropertyMap::ConstraintType::Target, columns);

    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipConstraintECInstanceIdIdPropertyMap> WipPropertyMapFactory::CreateSourceECInstanceIdPropertyMap(ClassMap const& classMap, std::vector<DbColumn const*> const& columns)
    {
    return WipConstraintECInstanceIdIdPropertyMap::CreateInstance(classMap, WipConstraintECInstanceIdIdPropertyMap::ConstraintType::Source, columns);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipConstraintECInstanceIdIdPropertyMap> WipPropertyMapFactory::CreateTargetECInstanceIdPropertyMap(ClassMap const& classMap, std::vector<DbColumn const*> const& columns)
    {
    return WipConstraintECInstanceIdIdPropertyMap::CreateInstance(classMap, WipConstraintECInstanceIdIdPropertyMap::ConstraintType::Target, columns);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipHorizontalPropertyMap> WipPropertyMapFactory::CreateCopy(WipHorizontalPropertyMap const& propertyMap, ClassMap const& newContext)
    {
    WipColumnHorizontalPropertyMap const* pm = dynamic_cast<WipColumnHorizontalPropertyMap const*>(&propertyMap);
    if (pm == nullptr)
        return nullptr;

    std::vector<DbColumn const*> columns;
    for (WipColumnVerticalPropertyMap const* child : pm->GetVerticalPropertyMaps())
        {
        columns.push_back(&child->GetColumn());
        }

    if (auto pm = dynamic_cast<WipECInstanceIdPropertyMap const*>(&propertyMap))
        {
        return CreateECInstanceIdPropertyMap(newContext, columns);
        }

    if (auto pm = dynamic_cast<WipECClassIdPropertyMap const*>(&propertyMap))
        {
        return CreateECClassIdPropertyMap(newContext, pm->GetDefaultECClassId(), columns);
        }

    if (auto pm = dynamic_cast<WipConstraintECInstanceIdIdPropertyMap const*>(&propertyMap))
        {
        if (pm->IsSource())
            return CreateSourceECInstanceIdPropertyMap(newContext, columns);

        return CreateTargetECInstanceIdPropertyMap(newContext, columns);
        }

    if (auto pm = dynamic_cast<WipConstraintECClassIdPropertyMap const*>(&propertyMap))
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
 RefCountedPtr<WipConstraintECInstanceIdIdPropertyMap> WipPropertyMapFactory::CreateConstraintECInstanceIdPropertyMap(ECN::ECRelationshipEnd end, ClassMap const& classMap, std::vector<DbColumn const*> const& columns)
     {
     if (end == ECRelationshipEnd_Source)
         return CreateSourceECInstanceIdPropertyMap(classMap, columns);

     return CreateTargetECInstanceIdPropertyMap(classMap, columns);
     }

 //=======================================================================================
 // @bsimethod                                                   Affan.Khan          07/16
 //+===============+===============+===============+===============+===============+======
 RefCountedPtr<WipConstraintECClassIdPropertyMap> WipPropertyMapFactory::CreateConstraintECClassIdPropertyMap(ECN::ECRelationshipEnd end, ClassMap const& classMap, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns)
     {
     if (end == ECRelationshipEnd_Source)
         return CreateSourceECClassIdPropertyMap(classMap, defaultEClassId, columns);

     return CreateTargetECClassIdPropertyMap(classMap, defaultEClassId, columns);
     }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipVerticalPropertyMap> WipPropertyMapFactory::CreateCopy(WipVerticalPropertyMap const& propertyMap, ClassMap const& newContext)
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
RefCountedPtr<WipVerticalPropertyMap> WipPropertyMapFactory::CreateCopy(WipVerticalPropertyMap const& propertyMap, ClassMap const& newContext, WipVerticalPropertyMap const* newParent)
    {
    RefCountedPtr<WipVerticalPropertyMap> copy;
    if (WipPoint2dPropertyMap const* typed = dynamic_cast<WipPoint2dPropertyMap const*>(&propertyMap))
        {
        PrimitiveECPropertyCP prop = typed->GetProperty().GetAsPrimitiveProperty();
        if (newParent)
            copy = CreatePoint2dPropertyMap( *prop, *newParent, typed->GetX().GetColumn(), typed->GetY().GetColumn());
        else
            copy = CreatePoint2dPropertyMap(newContext, *prop, typed->GetX().GetColumn(), typed->GetY().GetColumn());

        copy->FinishEditing();
        }
    else if (WipPoint3dPropertyMap const* typed = dynamic_cast<WipPoint3dPropertyMap const*>(&propertyMap))
        {
        PrimitiveECPropertyCP prop = typed->GetProperty().GetAsPrimitiveProperty();
        if (newParent)
            copy = CreatePoint3dPropertyMap(*prop, *newParent, typed->GetX().GetColumn(), typed->GetY().GetColumn(), typed->GetZ().GetColumn());
        else
            copy = CreatePoint3dPropertyMap(newContext, *prop, typed->GetX().GetColumn(), typed->GetY().GetColumn(), typed->GetZ().GetColumn());

        copy->FinishEditing();
        }
    else if (WipPrimitivePropertyMap const* typed = dynamic_cast<WipPrimitivePropertyMap const*>(&propertyMap))
        {
        PrimitiveECPropertyCP prop = typed->GetProperty().GetAsPrimitiveProperty();
        if (newParent)
            copy = CreatePrimitivePropertyMap(*prop, *newParent, typed->GetColumn());
        else
            copy = CreatePrimitivePropertyMap(newContext, *prop, typed->GetColumn());

        copy->FinishEditing();
        }
    else if (WipPrimitiveArrayPropertyMap const* typed = dynamic_cast<WipPrimitiveArrayPropertyMap const*>(&propertyMap))
        {
        ArrayECPropertyCP prop = typed->GetProperty().GetAsArrayProperty();
        if (newParent)
            copy = CreatePrimitiveArrayPropertyMap(*prop, *newParent, typed->GetColumn());
        else
            copy = CreatePrimitiveArrayPropertyMap(newContext, *prop, typed->GetColumn());

        copy->FinishEditing();
        }
    else if (WipStructPropertyMap const* typed = dynamic_cast<WipStructPropertyMap const*>(&propertyMap))
        {
        RefCountedPtr<WipStructPropertyMap> st;        
        StructECPropertyCP prop = typed->GetProperty().GetAsStructProperty();
        if (newParent)
            st = CreateStructPropertyMap(*prop, *newParent);
        else
            st = CreateStructPropertyMap(newContext, *prop);

        for (WipVerticalPropertyMap const* child : *typed)
            {
            RefCountedPtr<WipVerticalPropertyMap> childMap = CreateCopy(*child, newContext, st.get());
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
    else if (WipStructArrayPropertyMap const* typed = dynamic_cast<WipStructArrayPropertyMap const*>(&propertyMap))
        {
        StructArrayECPropertyCP prop = typed->GetProperty().GetAsStructArrayProperty();
        if (newParent)
            copy = CreateStructArrayPropertyMap(*prop, *newParent, typed->GetColumn());
        else
            copy = CreateStructArrayPropertyMap(newContext, *prop, typed->GetColumn());

        copy->FinishEditing();
        }
    else if (WipNavigationPropertyMap const* typed = dynamic_cast<WipNavigationPropertyMap const*>(&propertyMap))
        {
        RefCountedPtr<WipNavigationPropertyMap> nav;
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
            WipNavigationPropertyMap::RelECClassIdPropertyMap const& relId = typed->GetRelECClassId();
            WipNavigationPropertyMap::IdPropertyMap const& id = typed->GetId();

            if (nav->Setup(relId.GetColumn(), relId.GetDefaultClassId(), id.GetColumn()) != SUCCESS)
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

//************************************WipPropertyMapColumnDispatcher********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
DispatcherFeedback WipPropertyMapColumnDispatcher::_Dispatch(PropertyMapType mapType, WipColumnVerticalPropertyMap const& propertyMap) const
    {
    if (Enum::Contains(m_filter, mapType))
        m_columns.push_back(&propertyMap.GetColumn());

    return DispatcherFeedback::Next;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
DispatcherFeedback WipPropertyMapColumnDispatcher::_Dispatch(PropertyMapType mapType, WipCompoundPropertyMap const& propertyMap) const
    {
    if (Enum::Contains(m_filter, mapType))
        return DispatcherFeedback::Next;

    return DispatcherFeedback::NextSibling;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
DispatcherFeedback WipPropertyMapColumnDispatcher::_Dispatch(PropertyMapType mapType, WipColumnHorizontalPropertyMap const& propertyMap) const
    {
    if (m_table && Enum::Contains(m_filter, mapType))
        {
        if (WipColumnVerticalPropertyMap const* m = propertyMap.FindVerticalPropertyMap(*m_table))
            {
            m_columns.push_back(&m->GetColumn());
            }
        }

    return DispatcherFeedback::Next;
    }

//************************************WipPropertyMapSaveDispatcher*************************************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
DispatcherFeedback WipPropertyMapSaveDispatcher::_Dispatch(PropertyMapType mapType, WipColumnVerticalPropertyMap const& propertyMap) const
    {
    const ECN::ECPropertyId rootPropertyId = propertyMap.GetRoot().GetProperty().GetId();
    Utf8StringCR accessString = propertyMap.GetAccessString();
    if (m_context.InsertPropertyMap(rootPropertyId, accessString.c_str(), propertyMap.GetColumn().GetId()) != SUCCESS)
        {
        BeAssert(false);
        m_status = ERROR;
        m_failedMap = &propertyMap;
        return DispatcherFeedback::Cancel;
        }

    return DispatcherFeedback::Next;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
DispatcherFeedback WipPropertyMapSaveDispatcher::_Dispatch(PropertyMapType mapType, WipCompoundPropertyMap const& propertyMap) const
    {
    return DispatcherFeedback::Next;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
DispatcherFeedback WipPropertyMapSaveDispatcher::_Dispatch(PropertyMapType mapType, WipColumnHorizontalPropertyMap const& propertyMap) const
    {
    const ECN::ECPropertyId rootPropertyId = propertyMap.GetRoot().GetProperty().GetId();
    Utf8StringCR accessString = propertyMap.GetAccessString();
    for (WipColumnVerticalPropertyMap const* childMap : propertyMap.GetVerticalPropertyMaps())
        {
        if (m_context.InsertPropertyMap(rootPropertyId, accessString.c_str(), childMap->GetColumn().GetId()) != SUCCESS)
            {
            BeAssert(false);
            m_status = ERROR;
            m_failedMap = &propertyMap;
            return DispatcherFeedback::Cancel;
            }
        }

    return DispatcherFeedback::Next;
    }
END_BENTLEY_SQLITE_EC_NAMESPACE
