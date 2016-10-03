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

    if (&propertyMap->GetClassMap() == &GetClassMap())
        {
        BeAssert(false && "PropertyMap classMap does not match");
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
ECDbCR WipPropertyMapContainer::GetECDb() const { return m_classMap.GetECDbMap().GetECDb(); }

//************************************WipPropertyMap*************************************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
WipPropertyMap::WipPropertyMap(WipPropertyMapContainer const& container, ECN::ECPropertyCR ecProperty)
    : m_ecProperty(ecProperty), m_container(container), m_parentPropertMap(nullptr), m_propertyAccessString(ecProperty.GetName()), m_isInEditMode(true)
    {}

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
WipPropertyMap::WipPropertyMap(ECN::ECPropertyCR ecProperty, WipPropertyMap const& parentPropertyMap)
    : m_ecProperty(ecProperty), m_container(parentPropertyMap.GetContainer()), m_parentPropertMap(&parentPropertyMap), m_propertyAccessString(parentPropertyMap.GetAccessString() + EC_ACCESSSTRING_DELIMITER + ecProperty.GetName()), m_isInEditMode(true)
    {}

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
bool WipPropertyMap::IsInjected() const
    {
    for (ECN::ECPropertyCP property : GetContainer().GetClass().GetProperties(true))
        if (property == &m_ecProperty)
            return false;

    return true;
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
RefCountedPtr<WipVerticalPropertyMap> WipVerticalPropertyMap::CreateCopy(WipPropertyMapContainer const& newContainer) const
    {
    return WipPropertyMapFactory::CreateCopy(*this, newContainer);
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
    if (&propertyMap->GetContainer() == &GetContainer())
        {
        BeAssert(false && "All nested property map must belong to same container");
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
WipColumnHorizontalPropertyMap::WipColumnHorizontalPropertyMap(WipPropertyMapContainer const& container, ECN::ECPropertyCR ecProperty, std::vector<RefCountedPtr<WipColumnVerticalPropertyMap>> const& maps)
    : WipHorizontalPropertyMap(container, ecProperty)
    {
    for (RefCountedPtr<WipColumnVerticalPropertyMap> const& map : maps)
        {
        if (m_vmapsPerTable.find(map->GetTable().GetName().c_str()) != m_vmapsPerTable.end())
            {
            BeAssert(false && "PropertyMap must be one per table");
            m_vmapsPerTable.clear();
            return;
            }
        m_vmapsPerTable[map->GetTable().GetName().c_str()] = map;
        }
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
WipColumnVerticalPropertyMap const* WipColumnHorizontalPropertyMap::GetPropertyMap(Utf8CP tableName) const
    {
    auto itor = m_vmapsPerTable.find(tableName);
    if (itor != m_vmapsPerTable.end())
        return itor->second.get();

    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
WipColumnVerticalPropertyMap const* WipColumnHorizontalPropertyMap::GetPropertyMap(DbTable const& table) const
    {
    return GetPropertyMap(table.GetName().c_str());
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
std::vector<WipColumnVerticalPropertyMap const*> WipColumnHorizontalPropertyMap::GetPropertyMaps() const
    {
    std::vector<WipColumnVerticalPropertyMap const*> maps;
    for (auto const& map : m_vmapsPerTable)
        maps.push_back(map.second.get());

    return maps;
    }

//************************************WipColumnHorizontalPropertyMap********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
BentleyStatus WipSystemPropertyMap::TryCreateVerticalMaps(std::vector<RefCountedPtr<WipPrimitivePropertyMap>>& propertyMaps, ECSqlSystemProperty systemProperty, WipPropertyMapContainer const& container, std::vector<DbColumn const*> const& columns)
    {
    ECDbSchemaManagerCR schemaManger = container.GetECDb().Schemas();
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
        auto prop = WipPrimitivePropertyMap::CreateInstance(container, *primtiveECProp, *column);
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
RefCountedPtr<WipECInstanceIdPropertyMap> WipECInstanceIdPropertyMap::CreateInstance(WipPropertyMapContainer const& container, std::vector<DbColumn const*> const& columns)
    {
    std::vector<RefCountedPtr<WipPrimitivePropertyMap>> propertyMaps;
    if (TryCreateVerticalMaps(propertyMaps, ECSqlSystemProperty::ECInstanceId, container, columns) != SUCCESS)
        return nullptr;

    PrimitiveECPropertyCP systemProperty = propertyMaps.front()->GetProperty().GetAsPrimitiveProperty();
    return new WipECInstanceIdPropertyMap(container, *systemProperty, propertyMaps);
    }
//************************************WipECClassIdPropertyMap********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipECClassIdPropertyMap> WipECClassIdPropertyMap::CreateInstance(WipPropertyMapContainer const& container, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns)
    {
    std::vector<RefCountedPtr<WipPrimitivePropertyMap>> propertyMaps;
    if (TryCreateVerticalMaps(propertyMaps, ECSqlSystemProperty::ECClassId, container, columns) != SUCCESS)
        return nullptr;

    PrimitiveECPropertyCP systemProperty = propertyMaps.front()->GetProperty().GetAsPrimitiveProperty();
    return new WipECClassIdPropertyMap(container, *systemProperty, propertyMaps, defaultEClassId);
    }

//************************************WipConstraintECClassIdPropertyMap********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipConstraintECClassIdPropertyMap> WipConstraintECClassIdPropertyMap::CreateInstance(WipPropertyMapContainer const& container, ECN::ECClassId defaultEClassId, ConstraintType constraintType, std::vector<DbColumn const*> const& columns)
    {
    std::vector<RefCountedPtr<WipPrimitivePropertyMap>> propertyMaps;

    if (TryCreateVerticalMaps(propertyMaps, constraintType == ConstraintType::Source ? ECSqlSystemProperty::SourceECClassId : ECSqlSystemProperty::TargetECClassId, container, columns) != SUCCESS)
        return nullptr;

    PrimitiveECPropertyCP systemProperty = propertyMaps.front()->GetProperty().GetAsPrimitiveProperty();
    return new WipConstraintECClassIdPropertyMap(container, *systemProperty, propertyMaps, defaultEClassId, constraintType);
    }

//************************************WipConstraintECClassIdPropertyMap********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static
RefCountedPtr<WipConstraintECInstanceIdIdPropertyMap> WipConstraintECInstanceIdIdPropertyMap::CreateInstance(WipPropertyMapContainer const& container, ConstraintType constraintType, std::vector<DbColumn const*> const& columns)
    {
    std::vector<RefCountedPtr<WipPrimitivePropertyMap>> propertyMaps;
    if (TryCreateVerticalMaps(propertyMaps, constraintType == ConstraintType::Source ? ECSqlSystemProperty::SourceECInstanceId : ECSqlSystemProperty::TargetECInstanceId, container, columns) != SUCCESS)
        return nullptr;

    PrimitiveECPropertyCP systemProperty = propertyMaps.front()->GetProperty().GetAsPrimitiveProperty();
    return new WipConstraintECInstanceIdIdPropertyMap(container, *systemProperty, propertyMaps, constraintType);
    }
//************************************WipPrimitivePropertyMap********************
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
BentleyStatus WipPrimitivePropertyMap::_Validate() const
    {
    ECN::PrimitiveECPropertyCP property = GetProperty().GetAsPrimitiveProperty();
    return property->GetType() != PRIMITIVETYPE_Point2D && property->GetType() != PRIMITIVETYPE_Point3D ? SUCCESS : ERROR;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipPrimitivePropertyMap> WipPrimitivePropertyMap::CreateInstance(WipPropertyMapContainer const& container, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column)
    {
    if (ecProperty.GetType() == PRIMITIVETYPE_Point2D || ecProperty.GetType() == PRIMITIVETYPE_Point3D)
        {
        BeAssert(false && "Do not support Point2d or Point3d property");
        return nullptr;
        }

    return new WipPrimitivePropertyMap(container, ecProperty, column);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipPrimitivePropertyMap> WipPrimitivePropertyMap::CreateInstance(ECN::PrimitiveECPropertyCR ecProperty, WipVerticalPropertyMap const& parentPropertyMap, DbColumn const& column)
    {
    if (ecProperty.GetType() == PRIMITIVETYPE_Point2D || ecProperty.GetType() == PRIMITIVETYPE_Point3D)
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
RefCountedPtr<WipPrimitiveArrayPropertyMap> WipPrimitiveArrayPropertyMap::CreateInstance(WipPropertyMapContainer const& container, ECN::ArrayECPropertyCR ecProperty, DbColumn const& column)
    {
    return new WipPrimitiveArrayPropertyMap(container, ecProperty, column);
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
RefCountedPtr<WipStructPropertyMap> WipStructPropertyMap::CreateInstance(WipPropertyMapContainer const& container, ECN::StructECPropertyCR ecProperty)
    {
    return new WipStructPropertyMap(container, ecProperty);
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
RefCountedPtr<WipStructArrayPropertyMap> WipStructArrayPropertyMap::CreateInstance(WipPropertyMapContainer const& container, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column)
    {
    return new WipStructArrayPropertyMap(container, ecProperty, column);
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
    ECDbSchemaManagerCR schemaManger = parentPropertyMap.GetContainer().GetECDb().Schemas();
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
    ECDbSchemaManagerCR schemaManger = parentPropertyMap.GetContainer().GetECDb().Schemas();
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
RefCountedPtr<WipNavigationPropertyMap> WipNavigationPropertyMap::CreateInstance(WipPropertyMapContainer const& container, ECN::NavigationECPropertyCR ecProperty)
    {
    return new WipNavigationPropertyMap(container, ecProperty);
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

    ECDbSchemaManagerCR schemaManger = GetContainer().GetECDb().Schemas();
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
BentleyStatus WipPoint2dPropertyMap::_Validate() const
    {
    return size() == 2 && IsReadonly() ? SUCCESS : ERROR;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipPoint2dPropertyMap> WipPoint2dPropertyMap::CreateInstance(WipPropertyMapContainer const& container, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y)
    {
    if (ecProperty.GetType() != PrimitiveType::PRIMITIVETYPE_Point2D)
        {
        BeAssert(false && "PrimitiveType must be Point2d");
        return nullptr;
        }

    RefCountedPtr<WipPoint2dPropertyMap> ptr = new WipPoint2dPropertyMap(container, ecProperty);
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
    if (ecProperty.GetType() != PrimitiveType::PRIMITIVETYPE_Point2D)
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
    ECDbSchemaManagerCR schemaManger = GetContainer().GetECDb().Schemas();
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
BentleyStatus WipPoint3dPropertyMap::_Validate() const
    {
    return size() == 3 && IsReadonly() ? SUCCESS : ERROR;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipPoint3dPropertyMap> WipPoint3dPropertyMap::CreateInstance(WipPropertyMapContainer const& container, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y, DbColumn const& z)
    {
    if (ecProperty.GetType() != PrimitiveType::PRIMITIVETYPE_Point3D)
        {
        BeAssert(false && "PrimitiveType must be Point3d");
        return nullptr;
        }

    RefCountedPtr<WipPoint3dPropertyMap> ptr = new WipPoint3dPropertyMap(container, ecProperty);
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
    if (ecProperty.GetType() != PrimitiveType::PRIMITIVETYPE_Point3D)
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
RefCountedPtr<WipPrimitivePropertyMap> WipPropertyMapFactory::CreatePrimitivePropertyMap(WipPropertyMapContainer const& container, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column)
    {
    return WipPrimitivePropertyMap::CreateInstance(container, ecProperty, column);
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
RefCountedPtr<WipPrimitiveArrayPropertyMap> WipPropertyMapFactory::CreatePrimitiveArrayPropertyMap(WipPropertyMapContainer const& container, ECN::ArrayECPropertyCR ecProperty, DbColumn const& column)
    {
    return WipPrimitiveArrayPropertyMap::CreateInstance(container, ecProperty, column);
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
RefCountedPtr<WipStructPropertyMap> WipPropertyMapFactory::CreateStructPropertyMap(WipPropertyMapContainer const& container, ECN::StructECPropertyCR ecProperty)
    {
    return WipStructPropertyMap::CreateInstance(container, ecProperty);
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
RefCountedPtr<WipStructArrayPropertyMap> WipPropertyMapFactory::CreateStructArrayPropertyMap(WipPropertyMapContainer const& container, ECN::StructArrayECPropertyCR ecProperty, DbColumn const& column)
    {
    return WipStructArrayPropertyMap::CreateInstance(container, ecProperty, column);
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
RefCountedPtr<WipPoint2dPropertyMap> WipPropertyMapFactory::CreatePoint2dPropertyMap(WipPropertyMapContainer const& container, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y)
    {
    return WipPoint2dPropertyMap::CreateInstance(container, ecProperty, x, y);
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
RefCountedPtr<WipPoint3dPropertyMap> WipPropertyMapFactory::CreatePoint3dPropertyMap(WipPropertyMapContainer const& container, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y, DbColumn const& z)
    {
    return WipPoint3dPropertyMap::CreateInstance(container, ecProperty, x, y, z);
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
RefCountedPtr<WipNavigationPropertyMap> WipPropertyMapFactory::CreateNavigationPropertyMap(WipPropertyMapContainer const& container, ECN::NavigationECPropertyCR ecProperty)
    {
    return WipNavigationPropertyMap::CreateInstance(container, ecProperty);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipECInstanceIdPropertyMap> WipPropertyMapFactory::CreateECInstanceIdPropertyMap(WipPropertyMapContainer const& container, std::vector<DbColumn const*> const& columns)
    {
    return WipECInstanceIdPropertyMap::CreateInstance(container, columns);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipECClassIdPropertyMap> WipPropertyMapFactory::CreateECClassIdPropertyMap(WipPropertyMapContainer const& container, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns)
    {
    return WipECClassIdPropertyMap::CreateInstance(container,defaultEClassId, columns);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipConstraintECClassIdPropertyMap> WipPropertyMapFactory::CreateSourceECClassIdPropertyMap(WipPropertyMapContainer const& container, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns)
    {
    return WipConstraintECClassIdPropertyMap::CreateInstance(container, defaultEClassId , WipConstraintECClassIdPropertyMap::ConstraintType::Source, columns);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipConstraintECClassIdPropertyMap> WipPropertyMapFactory::CreateTargetECClassIdPropertyMap(WipPropertyMapContainer const& container, ECN::ECClassId defaultEClassId, std::vector<DbColumn const*> const& columns)
    {
    return WipConstraintECClassIdPropertyMap::CreateInstance(container, defaultEClassId, WipConstraintECClassIdPropertyMap::ConstraintType::Target, columns);

    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipConstraintECInstanceIdIdPropertyMap> WipPropertyMapFactory::CreateSourceECInstanceIdPropertyMap(WipPropertyMapContainer const& container, std::vector<DbColumn const*> const& columns)
    {
    return WipConstraintECInstanceIdIdPropertyMap::CreateInstance(container, WipConstraintECInstanceIdIdPropertyMap::ConstraintType::Source, columns);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<WipConstraintECInstanceIdIdPropertyMap> WipPropertyMapFactory::CreateTargetECInstanceIdPropertyMap(WipPropertyMapContainer const& container, std::vector<DbColumn const*> const& columns)
    {
    return WipConstraintECInstanceIdIdPropertyMap::CreateInstance(container, WipConstraintECInstanceIdIdPropertyMap::ConstraintType::Target, columns);
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipVerticalPropertyMap> WipPropertyMapFactory::CreateCopy(WipVerticalPropertyMap const& propertyMap, WipPropertyMapContainer const& newContainer)
    {
    if (propertyMap.GetParent() != nullptr)
        {
        BeAssert(false && "Expecting a root property map");
        return nullptr;
        }

    if (&propertyMap.GetContainer() == &newContainer)
        {
        BeAssert(false && "Expecting new container to be different from existing");
        return nullptr;
        }

    return CreateCopy(propertyMap, newContainer, nullptr);
    }
//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<WipVerticalPropertyMap> WipPropertyMapFactory::CreateCopy(WipVerticalPropertyMap const& propertyMap, WipPropertyMapContainer const& newContainer, WipVerticalPropertyMap const* newParent)
    {
    RefCountedPtr<WipVerticalPropertyMap> copy;
    if (WipPoint2dPropertyMap const* typed = dynamic_cast<WipPoint2dPropertyMap const*>(&propertyMap))
        {
        PrimitiveECPropertyCP prop = typed->GetProperty().GetAsPrimitiveProperty();
        if (newParent)
            copy = CreatePoint2dPropertyMap( *prop, *newParent, typed->GetX().GetColumn(), typed->GetY().GetColumn());
        else
            copy = CreatePoint2dPropertyMap(newContainer, *prop, typed->GetX().GetColumn(), typed->GetY().GetColumn());

        copy->FinishEditing();
        }
    else if (WipPoint3dPropertyMap const* typed = dynamic_cast<WipPoint3dPropertyMap const*>(&propertyMap))
        {
        PrimitiveECPropertyCP prop = typed->GetProperty().GetAsPrimitiveProperty();
        if (newParent)
            copy = CreatePoint3dPropertyMap(*prop, *newParent, typed->GetX().GetColumn(), typed->GetY().GetColumn(), typed->GetZ().GetColumn());
        else
            copy = CreatePoint3dPropertyMap(newContainer, *prop, typed->GetX().GetColumn(), typed->GetY().GetColumn(), typed->GetZ().GetColumn());

        copy->FinishEditing();
        }
    else if (WipPrimitivePropertyMap const* typed = dynamic_cast<WipPrimitivePropertyMap const*>(&propertyMap))
        {
        PrimitiveECPropertyCP prop = typed->GetProperty().GetAsPrimitiveProperty();
        if (newParent)
            copy = CreatePrimitivePropertyMap(*prop, *newParent, typed->GetColumn());
        else
            copy = CreatePrimitivePropertyMap(newContainer, *prop, typed->GetColumn());

        copy->FinishEditing();
        }
    else if (WipPrimitiveArrayPropertyMap const* typed = dynamic_cast<WipPrimitiveArrayPropertyMap const*>(&propertyMap))
        {
        ArrayECPropertyCP prop = typed->GetProperty().GetAsArrayProperty();
        if (newParent)
            copy = CreatePrimitiveArrayPropertyMap(*prop, *newParent, typed->GetColumn());
        else
            copy = CreatePrimitiveArrayPropertyMap(newContainer, *prop, typed->GetColumn());

        copy->FinishEditing();
        }
    else if (WipStructPropertyMap const* typed = dynamic_cast<WipStructPropertyMap const*>(&propertyMap))
        {
        RefCountedPtr<WipStructPropertyMap> st;        
        StructECPropertyCP prop = typed->GetProperty().GetAsStructProperty();
        if (newParent)
            st = CreateStructPropertyMap(*prop, *newParent);
        else
            st = CreateStructPropertyMap(newContainer, *prop);

        for (WipVerticalPropertyMap const* child : *typed)
            {
            RefCountedPtr<WipVerticalPropertyMap> childMap = CreateCopy(*child, newContainer, st.get());
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
            copy = CreateStructArrayPropertyMap(newContainer, *prop, typed->GetColumn());

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
            nav = CreateNavigationPropertyMap(newContainer, *prop);

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


END_BENTLEY_SQLITE_EC_NAMESPACE
