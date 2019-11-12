/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//************************************PropertyMap*************************************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
ECPropertyId PropertyMap::GetRootPropertyId() const
    {
    PropertyMap const* root = this;
    while (root->GetParent() != nullptr)
        root = root->GetParent();

    return root->GetProperty().GetId();
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

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
PropertyMap::Path& PropertyMap::Path::operator=(PropertyMap::Path const& path)
    {
    if (this != &path)
        m_vect = path.m_vect;

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
PropertyMap::Path& PropertyMap::Path::operator=(PropertyMap::Path const&& path)
    {
    if (this != &path)
        m_vect = std::move(path.m_vect);

    return *this;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//--------------------------------------------------------------------------------------
PropertyMap const& PropertyMap::Path::operator[](size_t i) const { return *m_vect[i]; }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
PropertyMap::Path PropertyMap::Path::From(PropertyMap const& propertyMap)
    {
    bvector<PropertyMap const*> vect;
    PropertyMap const* mp = &propertyMap;
    while (mp != nullptr)
        {
        vect.insert(vect.begin(), mp);
        mp = mp->GetParent();
        }

    return Path(vect);
    }

//***************************************************PropertyMapContainer*****************************************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus PropertyMapContainer::Insert(RefCountedPtr<PropertyMap> propertyMap, size_t position)
    {
    if (propertyMap == nullptr)
        {
        BeAssert(false && "PropertyMap passed cannot be null");
        return ERROR;
        }

    if (&propertyMap->GetClassMap() != &m_classMap)
        {
        BeAssert(false && "propertyMap must belong to same classMap context");
        return ERROR;
        }

    if (Find(propertyMap->GetAccessString().c_str()))
        {
        BeAssert(false && "PropertyMap with same name or may be different case already exist");
        return ERROR;
        }

#ifndef NDEBUG
    //Ensure ClassMap::GetTables() point it.
    if (!m_classMap.IsRelationshipClassMap())
        {
        GetTablesPropertyMapVisitor visitor;
        propertyMap->AcceptVisitor(visitor);
        std::set<DbTable const*> classMapTables(m_classMap.GetTables().begin(), m_classMap.GetTables().end());
        for (DbTable const* table : visitor.GetTables())
            {
            if (classMapTables.find(table) == classMapTables.end())
                {
                BeAssert(false && "PropertyMap pointing to table that is not register in ClassMap::GetTables()");
                return ERROR;
                }
            }
        }
#endif

    auto positionIt = position > m_topLevelPropMapsOrdered.size() ? m_topLevelPropMapsOrdered.end() : m_topLevelPropMapsOrdered.begin() + position;
    m_byAccessString[propertyMap->GetAccessString().c_str()] = propertyMap;
    if (propertyMap->GetParent() == nullptr)
        m_topLevelPropMapsOrdered.insert(positionIt, propertyMap.get());

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus PropertyMapContainer::_AcceptVisitor(IPropertyMapVisitor const&  visitor)  const
    {
    for (PropertyMap const* propertyMap : m_topLevelPropMapsOrdered)
        {
        if (SUCCESS != propertyMap->AcceptVisitor(visitor))
            return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
PropertyMap const* PropertyMapContainer::Find(Utf8CP accessString) const
    {
    auto itor = m_byAccessString.find(accessString);
    if (itor != m_byAccessString.end())
        return itor->second.get();

    return nullptr;
    }

//************************************CompoundDataPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
DbTable const& CompoundDataPropertyMap::_GetTable() const
    {
    if (m_memberPropertyMaps.empty())
        {
        BeAssert(!m_memberPropertyMaps.empty());
        }
    return m_memberPropertyMaps[0]->GetTable();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
DataPropertyMap const* CompoundDataPropertyMap::Find(Utf8CP accessString) const
    {
    Utf8String resolveAccessString(GetAccessString());
    resolveAccessString.append(".").append(accessString);

    PropertyMap const* propMap = GetClassMap().GetPropertyMaps().Find(resolveAccessString.c_str());
    if (propMap == nullptr)
        return nullptr;

    return &propMap->GetAs<DataPropertyMap>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus CompoundDataPropertyMap::InsertMember(RefCountedPtr<DataPropertyMap> propertyMap)
    {
    if (propertyMap->GetParent() != this)
        {
        BeAssert(false);
        return ERROR;
        }

    if (const_cast<ClassMap&>(GetClassMap()).GetPropertyMapsR().Insert(propertyMap) != SUCCESS)
        return ERROR;

    m_memberPropertyMaps.push_back(propertyMap.get());
    return SUCCESS;
    }




//************************************PrimitivePropertyMap********************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<PrimitivePropertyMap> PrimitivePropertyMap::CreateInstance(ClassMap const& classMap, CompoundDataPropertyMap const* parentPropMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& column)
    {
    if (ecProperty.GetType() == PRIMITIVETYPE_Point2d || ecProperty.GetType() == PRIMITIVETYPE_Point3d)
        {
        BeAssert(false && "Do not support Point2d or Point3d property");
        return nullptr;
        }

    if (parentPropMap != nullptr)
        return new PrimitivePropertyMap(*parentPropMap, ecProperty, column);

    return new PrimitivePropertyMap(classMap, ecProperty, column);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Krischan.Eberle     01/17
//---------------------------------------------------------------------------------------
//static 
DbColumn::Type PrimitivePropertyMap::DetermineColumnDataType(ECN::PrimitiveType primType)
    {
    switch (primType)
        {
            case ECN::PrimitiveType::PRIMITIVETYPE_Binary:
            case ECN::PrimitiveType::PRIMITIVETYPE_IGeometry:
                return DbColumn::Type::Blob;

            case ECN::PrimitiveType::PRIMITIVETYPE_Boolean:
                return DbColumn::Type::Boolean;

            case ECN::PrimitiveType::PRIMITIVETYPE_DateTime:
                return DbColumn::Type::TimeStamp;

            case ECN::PrimitiveType::PRIMITIVETYPE_Double:
                return DbColumn::Type::Real;

            case ECN::PrimitiveType::PRIMITIVETYPE_Integer:
            case ECN::PrimitiveType::PRIMITIVETYPE_Long:
                return DbColumn::Type::Integer;

            case ECN::PrimitiveType::PRIMITIVETYPE_String:
                return DbColumn::Type::Text;
        }

    BeAssert(false && "Type not supported");
    return DbColumn::Type::Any;
    }

//************************************Point2dPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<Point2dPropertyMap> Point2dPropertyMap::CreateInstance(ClassMap const& classMap, CompoundDataPropertyMap const* parentPropMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y)
    {
    if (ecProperty.GetType() != PrimitiveType::PRIMITIVETYPE_Point2d)
        {
        BeAssert(false && "PrimitiveType must be Point2d");
        return nullptr;
        }

    RefCountedPtr<Point2dPropertyMap> ptr = nullptr;
    if (parentPropMap != nullptr)
        ptr = new Point2dPropertyMap(*parentPropMap, ecProperty);
    else
        ptr = new Point2dPropertyMap(classMap, ecProperty);

    if (ptr->Init(x, y) != SUCCESS)
        return nullptr;

    return ptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus Point2dPropertyMap::Init(DbColumn const& x, DbColumn const& y)
    {
    if (&x.GetTable() != &y.GetTable())
        {
        BeAssert(false && "Point2d property must be mapped to columns in the same table.");
        return ERROR;
        }

    ECDbSystemSchemaHelper const& systemSchemaHelper = GetClassMap().GetECDb().Schemas().Main().GetSystemSchemaHelper();
    ECPropertyCP propX = systemSchemaHelper.GetSystemProperty(ECSqlSystemPropertyInfo::PointX());
    if (propX == nullptr)
        return ERROR;

    RefCountedPtr<PrimitivePropertyMap> xPropertyMap = PrimitivePropertyMap::CreateInstance(GetClassMap(), this, *propX->GetAsPrimitiveProperty(), x);
    if (xPropertyMap == nullptr)
        return ERROR;

    if (InsertMember(xPropertyMap) != SUCCESS)
        return ERROR;

    ECPropertyCP propY = systemSchemaHelper.GetSystemProperty(ECSqlSystemPropertyInfo::PointY());
    if (propY == nullptr)
        return ERROR;

    RefCountedPtr<PrimitivePropertyMap> yPropertyMap = PrimitivePropertyMap::CreateInstance(GetClassMap(), this, *propY->GetAsPrimitiveProperty(), y);
    if (yPropertyMap == nullptr)
        return ERROR;

    if (InsertMember(yPropertyMap) != SUCCESS)
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
PrimitivePropertyMap const& Point2dPropertyMap::GetX() const
    {
    PropertyMap const* propMap = Find(ECDBSYS_PROP_PointX);
    BeAssert(propMap != nullptr);
    return propMap->GetAs<PrimitivePropertyMap>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
PrimitivePropertyMap const& Point2dPropertyMap::GetY() const
    {
    PropertyMap const* propMap = Find(ECDBSYS_PROP_PointY);
    BeAssert(propMap != nullptr);
    return propMap->GetAs<PrimitivePropertyMap>();
    }

//************************************Point3dPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<Point3dPropertyMap> Point3dPropertyMap::CreateInstance(ClassMap const& classMap, CompoundDataPropertyMap const* parentPropMap, ECN::PrimitiveECPropertyCR ecProperty, DbColumn const& x, DbColumn const& y, DbColumn const& z)
    {
    if (ecProperty.GetType() != PrimitiveType::PRIMITIVETYPE_Point3d)
        {
        BeAssert(false && "PrimitiveType must be Point3d");
        return nullptr;
        }

    RefCountedPtr<Point3dPropertyMap> ptr = nullptr;
    if (parentPropMap != nullptr)
        ptr = new Point3dPropertyMap(*parentPropMap, ecProperty);
    else
        ptr = new Point3dPropertyMap(classMap, ecProperty);

    if (ptr->Init(x, y, z) != SUCCESS)
        return nullptr;

    return ptr;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus Point3dPropertyMap::Init(DbColumn const& x, DbColumn const& y, DbColumn const& z)
    {
    if (!(&x.GetTable() == &y.GetTable() && &x.GetTable() == &z.GetTable()))
        {
        BeAssert(false && "Point3d property must be mapped to columns in the same table.");
        return ERROR;
        }

    ECDbSystemSchemaHelper const& systemSchemaHelper = GetClassMap().GetECDb().Schemas().Main().GetSystemSchemaHelper();
    ECPropertyCP propX = systemSchemaHelper.GetSystemProperty(ECSqlSystemPropertyInfo::PointX());
    if (propX == nullptr)
        return ERROR;

    RefCountedPtr<PrimitivePropertyMap> xPropertyMap = PrimitivePropertyMap::CreateInstance(GetClassMap(), this, *propX->GetAsPrimitiveProperty(), x);
    if (xPropertyMap == nullptr)
        return ERROR;

    if (InsertMember(xPropertyMap) != SUCCESS)
        return ERROR;

    ECPropertyCP propY = systemSchemaHelper.GetSystemProperty(ECSqlSystemPropertyInfo::PointY());
    if (propY == nullptr)
        return ERROR;

    RefCountedPtr<PrimitivePropertyMap> yPropertyMap = PrimitivePropertyMap::CreateInstance(GetClassMap(), this, *propY->GetAsPrimitiveProperty(), y);
    if (yPropertyMap == nullptr)
        return ERROR;

    if (InsertMember(yPropertyMap) != SUCCESS)
        return ERROR;

    ECPropertyCP propZ = systemSchemaHelper.GetSystemProperty(ECSqlSystemPropertyInfo::PointZ());
    if (propZ == nullptr)
        return ERROR;

    RefCountedPtr<PrimitivePropertyMap> zPropertyMap = PrimitivePropertyMap::CreateInstance(GetClassMap(), this, *propZ->GetAsPrimitiveProperty(), z);
    if (zPropertyMap == nullptr)
        return ERROR;

    if (InsertMember(zPropertyMap) != SUCCESS)
        return ERROR;

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
PrimitivePropertyMap const& Point3dPropertyMap::GetX() const
    {
    PropertyMap const* propMap = Find(ECDBSYS_PROP_PointX);
    BeAssert(propMap != nullptr);
    return propMap->GetAs<PrimitivePropertyMap>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
PrimitivePropertyMap const& Point3dPropertyMap::GetY() const
    {
    PropertyMap const* propMap = Find(ECDBSYS_PROP_PointY);
    BeAssert(propMap != nullptr);
    return propMap->GetAs<PrimitivePropertyMap>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
PrimitivePropertyMap const& Point3dPropertyMap::GetZ() const
    {
    PropertyMap const* propMap = Find(ECDBSYS_PROP_PointZ);
    BeAssert(propMap != nullptr);
    return propMap->GetAs<PrimitivePropertyMap>();
    }

//************************************StructPropertyMap********************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<StructPropertyMap> StructPropertyMap::CreateInstance(ClassMap const& classMap, CompoundDataPropertyMap const* parentPropMap, ECN::StructECPropertyCR ecProperty)
    {
    if (parentPropMap != nullptr)
        return new StructPropertyMap(*parentPropMap, ecProperty);

    return new StructPropertyMap(classMap, ecProperty);
    }


//************************************PrimitiveArrayPropertyMap********************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<PrimitiveArrayPropertyMap> PrimitiveArrayPropertyMap::CreateInstance(ClassMap const& classMap, CompoundDataPropertyMap const* parentPropMap, ECN::PrimitiveArrayECPropertyCR prop, DbColumn const& column)
    {
    if (parentPropMap != nullptr)
        return new PrimitiveArrayPropertyMap(*parentPropMap, prop, column);

    return new PrimitiveArrayPropertyMap(classMap, prop, column);
    }

//************************************StructArrayPropertyMap********************

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<StructArrayPropertyMap> StructArrayPropertyMap::CreateInstance(ClassMap const& classMap, CompoundDataPropertyMap const* parentPropMap, ECN::StructArrayECPropertyCR prop, DbColumn const& column)
    {
    if (parentPropMap != nullptr)
        return new StructArrayPropertyMap(*parentPropMap, prop, column);

    return new StructArrayPropertyMap(classMap, prop, column);
    }



//************************************NavigationPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
NavigationPropertyMap::IdPropertyMap const& NavigationPropertyMap::GetIdPropertyMap() const
    {
    BeAssert(m_isComplete);
    PropertyMap const* propMap = Find(ECDBSYS_PROP_NavPropId);
    BeAssert(propMap != nullptr);
    return propMap->GetAs<IdPropertyMap>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
NavigationPropertyMap::RelECClassIdPropertyMap const& NavigationPropertyMap::GetRelECClassIdPropertyMap() const
    {
    BeAssert(m_isComplete);
    PropertyMap const* propMap = Find(ECDBSYS_PROP_NavPropRelECClassId);
    BeAssert(propMap != nullptr);
    return propMap->GetAs<RelECClassIdPropertyMap>();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle         10/16
//---------------------------------------------------------------------------------------
BentleyStatus NavigationPropertyMap::SetMembers(DbColumn const& idColumn, DbColumn const& relECClassIdColumn, ECN::ECClassId defaultRelClassId)
    {
    if (m_isComplete)
        {
        BeAssert(false && "NavigationPropertyMap::SetMembers can only be called on NavigationPropertyMap which is being built");
        return ERROR;
        }

    if (&relECClassIdColumn.GetTable() != &idColumn.GetTable())
        {
        BeAssert(false && "Navigation property must be mapped to columns in the same table.");
        return ERROR;
        }

    RefCountedPtr<NavigationPropertyMap::IdPropertyMap> idPropertyMap = IdPropertyMap::CreateInstance(*this, idColumn);
    if (idPropertyMap == nullptr)
        return ERROR;

    if (InsertMember(idPropertyMap) != SUCCESS)
        return ERROR;

    RefCountedPtr<NavigationPropertyMap::RelECClassIdPropertyMap> relECClassIdPropertyMap = RelECClassIdPropertyMap::CreateInstance(*this, relECClassIdColumn, defaultRelClassId);
    if (relECClassIdPropertyMap == nullptr)
        return ERROR;

    if (SUCCESS != InsertMember(relECClassIdPropertyMap))
        return ERROR;

    m_isComplete = true;
    return SUCCESS;
    }


//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
ECRelationshipConstraintCR NavigationPropertyMap::GetRelationshipConstraint(NavigationPropertyMap::NavigationEnd navEnd) const
    {
    BeAssert(GetProperty().GetIsNavigation());
    NavigationECPropertyCR navProp = *GetProperty().GetAsNavigationProperty();
    ECRelationshipEnd constraintEnd = GetRelationshipEnd(navProp, navEnd);
    return constraintEnd == ECRelationshipEnd::ECRelationshipEnd_Source ? navProp.GetRelationshipClass()->GetSource() : navProp.GetRelationshipClass()->GetTarget();
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                 Krischan.Eberle                      01/2016
//---------------------------------------------------------------------------------------
//static
ECN::ECRelationshipEnd NavigationPropertyMap::GetRelationshipEnd(ECN::NavigationECPropertyCR prop, NavigationPropertyMap::NavigationEnd end)
    {
    const ECRelatedInstanceDirection navPropDir = prop.GetDirection();
    if (navPropDir == ECRelatedInstanceDirection::Forward && end == NavigationPropertyMap::NavigationEnd::From ||
        navPropDir == ECRelatedInstanceDirection::Backward && end == NavigationPropertyMap::NavigationEnd::To)
        return ECRelationshipEnd_Source;

    return ECRelationshipEnd_Target;
    }

//************************************NavigationPropertyMap::RelECClassIdPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static 
RefCountedPtr<NavigationPropertyMap::RelECClassIdPropertyMap> NavigationPropertyMap::RelECClassIdPropertyMap::CreateInstance(NavigationPropertyMap const& parentPropertyMap, DbColumn const& column, ECN::ECClassId defaultRelClassId)
    {
    ECPropertyCP relECClassIdProp = parentPropertyMap.GetClassMap().GetECDb().Schemas().Main().GetSystemSchemaHelper().GetSystemProperty(ECSqlSystemPropertyInfo::NavigationRelECClassId());
    if (relECClassIdProp == nullptr)
        return nullptr;

    return new RelECClassIdPropertyMap(parentPropertyMap, *relECClassIdProp->GetAsPrimitiveProperty(), column, defaultRelClassId);
    }

//************************************NavigationPropertyMap::IdPropertyMap********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
//static
RefCountedPtr<NavigationPropertyMap::IdPropertyMap> NavigationPropertyMap::IdPropertyMap::CreateInstance(NavigationPropertyMap const& parentPropertyMap, DbColumn const& column)
    {
    ECPropertyCP idProp = parentPropertyMap.GetClassMap().GetECDb().Schemas().Main().GetSystemSchemaHelper().GetSystemProperty(ECSqlSystemPropertyInfo::NavigationId());
    if (idProp == nullptr)
        return nullptr;

    return new IdPropertyMap(parentPropertyMap, *idProp->GetAsPrimitiveProperty(), column);
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle         10/16
//---------------------------------------------------------------------------------------
StructPropertyMapBuilder::StructPropertyMapBuilder(ClassMap const& classMap, CompoundDataPropertyMap const* parentPropMap, ECN::StructECPropertyCR prop)
    : m_propMap(StructPropertyMap::CreateInstance(classMap, parentPropMap, prop)), m_isFinished(false)
    {}


//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle         10/16
//---------------------------------------------------------------------------------------
BentleyStatus StructPropertyMapBuilder::AddMember(RefCountedPtr<DataPropertyMap> propertyMap)
    {
    if (!IsValid())
        {
        BeAssert(false);
        return ERROR;
        }

    if (propertyMap == nullptr || &m_propMap->GetClassMap() != &propertyMap->GetClassMap() || m_propMap.get() != propertyMap->GetParent())
        {
        BeAssert(false);
        return ERROR;
        }

    if (SUCCESS != m_propMap->InsertMember(propertyMap))
        {
        m_propMap = nullptr;
        return ERROR;
        }

    return SUCCESS;
    }

//---------------------------------------------------------------------------------------
// @bsimethod                                               Krischan.Eberle         10/16
//---------------------------------------------------------------------------------------
BentleyStatus StructPropertyMapBuilder::Finish()
    {
    if (!IsValid() || m_isFinished)
        {
        BeAssert(false && "StructPropertyMapBuilder was never initialized or is already finished");
        return ERROR;
        }

    if (m_propMap->IsEmpty())
        {
        BeAssert(false && "Struct properties without members is not supported");
        return ERROR;
        }

    DbTable const* table = nullptr;
    for (DataPropertyMap const* member : *m_propMap)
        {
        if (table == nullptr)
            table = &member->GetTable();
        else
            {
            if (table != &member->GetTable())
                {
                BeAssert(false && "All members of the struct property map must map to the same table");
                return ERROR;
                }
            }
        }

    m_isFinished = true;
    return SUCCESS;
    }


//************************************PropertyMapCopier********************

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
RefCountedPtr<SystemPropertyMap> PropertyMapCopier::CreateCopy(SystemPropertyMap const& propertyMap, ClassMap const& newContext)
    {
    std::vector<DbColumn const*> columns;
    for (SingleColumnDataPropertyMap const* child : propertyMap.GetDataPropertyMaps())
        {
        columns.push_back(&child->GetColumn());
        }

    if (propertyMap.GetType() == PropertyMap::Type::ECInstanceId)
        return ECInstanceIdPropertyMap::CreateInstance(newContext, columns);

    if (propertyMap.GetType() == PropertyMap::Type::ECClassId)
        return ECClassIdPropertyMap::CreateInstance(newContext, columns);

    if (propertyMap.GetType() == PropertyMap::Type::ConstraintECInstanceId)
        return ConstraintECInstanceIdPropertyMap::CreateInstance(newContext, propertyMap.GetAs<ConstraintECInstanceIdPropertyMap>().GetEnd(), columns);

    if (propertyMap.GetType() == PropertyMap::Type::ConstraintECClassId)
        {
        ConstraintECClassIdPropertyMap const& constraintECClassIdPropMap = propertyMap.GetAs<ConstraintECClassIdPropertyMap>();
        return ConstraintECClassIdPropertyMap::CreateInstance(newContext, constraintECClassIdPropMap.GetEnd(), columns);
        }

    BeAssert(false && "Unhandled case");
    return nullptr;
    }

//=======================================================================================
// @bsimethod                                                   Affan.Khan          07/16
//+===============+===============+===============+===============+===============+======
//static 
RefCountedPtr<DataPropertyMap> PropertyMapCopier::CreateCopy(DataPropertyMap const& propertyMap, ClassMap const& newContext)
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
RefCountedPtr<DataPropertyMap> PropertyMapCopier::CreateCopy(DataPropertyMap const& propertyMap, ClassMap const& newContext, CompoundDataPropertyMap const* parentPropMap)
    {
    switch (propertyMap.GetType())
        {
            case PropertyMap::Type::Primitive:
            {
            PrimitivePropertyMap const& primPropMap = propertyMap.GetAs<PrimitivePropertyMap>();
            PrimitiveECPropertyCP prop = primPropMap.GetProperty().GetAsPrimitiveProperty();
            return PrimitivePropertyMap::CreateInstance(newContext, parentPropMap, *prop, primPropMap.GetColumn());
            }

            case PropertyMap::Type::Point2d:
            {
            Point2dPropertyMap const& ptPropMap = propertyMap.GetAs<Point2dPropertyMap>();
            PrimitiveECPropertyCP prop = ptPropMap.GetProperty().GetAsPrimitiveProperty();
            return Point2dPropertyMap::CreateInstance(newContext, parentPropMap, *prop, ptPropMap.GetX().GetColumn(), ptPropMap.GetY().GetColumn());
            }

            case PropertyMap::Type::Point3d:
            {
            Point3dPropertyMap const& ptPropMap = propertyMap.GetAs<Point3dPropertyMap>();
            PrimitiveECPropertyCP prop = ptPropMap.GetProperty().GetAsPrimitiveProperty();
            return Point3dPropertyMap::CreateInstance(newContext, parentPropMap, *prop, ptPropMap.GetX().GetColumn(), ptPropMap.GetY().GetColumn(), ptPropMap.GetZ().GetColumn());
            }

            case PropertyMap::Type::PrimitiveArray:
            {
            PrimitiveArrayPropertyMap const& arrayPropMap = propertyMap.GetAs<PrimitiveArrayPropertyMap>();
            PrimitiveArrayECPropertyCP prop = arrayPropMap.GetProperty().GetAsPrimitiveArrayProperty();
            return PrimitiveArrayPropertyMap::CreateInstance(newContext, parentPropMap, *prop, arrayPropMap.GetColumn());
            }

            case PropertyMap::Type::StructArray:
            {
            StructArrayPropertyMap const& arrayPropMap = propertyMap.GetAs<StructArrayPropertyMap>();
            StructArrayECPropertyCP prop = arrayPropMap.GetProperty().GetAsStructArrayProperty();
            return StructArrayPropertyMap::CreateInstance(newContext, parentPropMap, *prop, arrayPropMap.GetColumn());
            }

            case PropertyMap::Type::Struct:
            {
            StructPropertyMapBuilder builder(newContext, parentPropMap, *propertyMap.GetProperty().GetAsStructProperty());
            StructPropertyMap const& structPropMap = propertyMap.GetAs<StructPropertyMap>();
            for (DataPropertyMap const* memberPropMap : structPropMap)
                {
                RefCountedPtr<DataPropertyMap> childMap = CreateCopy(*memberPropMap, newContext, &builder.GetPropertyMapUnderConstruction());
                if (childMap == nullptr)
                    {
                    BeAssert(false);
                    return nullptr;
                    }

                if (SUCCESS != builder.AddMember(childMap))
                    {
                    BeAssert(false);
                    return nullptr;
                    }
                }

            if (SUCCESS != builder.Finish())
                {
                BeAssert(false);
                return nullptr;
                }

            return builder.GetPropertyMap();
            }

            case PropertyMap::Type::Navigation:
            {
            //nav props can be copied even when under construction. 
            RefCountedPtr<NavigationPropertyMap> clonedPropMap = NavigationPropertyMap::CreateInstance(newContext, *propertyMap.GetProperty().GetAsNavigationProperty());
            if (clonedPropMap == nullptr)
                {
                BeAssert(false);
                return nullptr;
                }

            NavigationPropertyMap const& navPropMap = propertyMap.GetAs<NavigationPropertyMap>();
            if (navPropMap.IsComplete())
                {
                NavigationPropertyMap::RelECClassIdPropertyMap const& relClassIdPropMap = navPropMap.GetRelECClassIdPropertyMap();
                if (SUCCESS != clonedPropMap->SetMembers(navPropMap.GetIdPropertyMap().GetColumn(), relClassIdPropMap.GetColumn(), relClassIdPropMap.GetDefaultClassId()))
                    return nullptr;

                BeAssert(clonedPropMap->IsComplete());
                }

            return clonedPropMap;
            }

            default:
            {
            BeAssert(false && "Unhandled property map type");
            return nullptr;
            }
        }
    }

//************************************IPropertyMapVisitor********************
//---------------------------------------------------------------------------------------
// @bsimethod                                                   Affan.Khan          07/16
//---------------------------------------------------------------------------------------
BentleyStatus IPropertyMapVisitor::_Visit(CompoundDataPropertyMap const& propertyMap) const
    {
    for (DataPropertyMap const* childPropertyMap : propertyMap)
        {
        if (SUCCESS != childPropertyMap->AcceptVisitor(*this))
            return ERROR;
        }

    return SUCCESS;
    }

END_BENTLEY_SQLITE_EC_NAMESPACE
