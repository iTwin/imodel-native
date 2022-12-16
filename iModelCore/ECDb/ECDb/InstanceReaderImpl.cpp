/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See COPYRIGHT.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE
// This class replace virtual ClassId/SourceClassId/TargetClassId with constant ClassIdField
struct ClassIdECSqlField : ECSqlField {
    private:
        ECN::ECClassId m_classId;
        mutable Utf8String m_idStr;
    private:
        virtual bool _IsNull() const override { return !m_classId.IsValid(); }
        virtual void const* _GetBlob(int* blobSize) const override { return NoopECSqlValue::GetSingleton().GetBlob(blobSize);}
        virtual bool _GetBoolean() const override { return NoopECSqlValue::GetSingleton().GetBoolean();}
        virtual double _GetDateTimeJulianDays(DateTime::Info& metadata) const override { return NoopECSqlValue::GetSingleton().GetDateTimeJulianDays(metadata);}
        virtual uint64_t _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const override { return NoopECSqlValue::GetSingleton().GetDateTimeJulianDaysMsec(metadata);}
        virtual double _GetDouble() const override { return NoopECSqlValue::GetSingleton().GetDouble();}
        virtual int _GetInt() const override { return NoopECSqlValue::GetSingleton().GetInt();}
        virtual IGeometryPtr _GetGeometry() const override { return NoopECSqlValue::GetSingleton().GetGeometry();}
        virtual DPoint2d _GetPoint2d() const override { return NoopECSqlValue::GetSingleton().GetPoint2d();}
        virtual DPoint3d _GetPoint3d() const override { return NoopECSqlValue::GetSingleton().GetPoint3d();}
        virtual IECSqlValue const& _GetStructMemberValue(Utf8CP structMemberName) const  override { return NoopECSqlValue::GetSingleton();}
        virtual IECSqlValueIterable const& _GetStructIterable() const  override { return NoopECSqlValue::GetSingleton().GetStructIterable();}
        virtual int _GetArrayLength() const override  { return NoopECSqlValue::GetSingleton().GetArrayLength();}
        virtual IECSqlValueIterable const& _GetArrayIterable() const override { return NoopECSqlValue::GetSingleton().GetArrayIterable();}
        virtual Utf8CP _GetText() const override {
            if (m_idStr.empty()) {
                m_idStr = m_classId.ToHexStr();
            }
            return m_idStr.c_str();
        }
        virtual int64_t _GetInt64() const override {
            return static_cast<int64_t>(m_classId.GetValueUnchecked());
        }

    public:
        ClassIdECSqlField(ECSqlSelectPreparedStatement& ecsqlStatement, ECSqlColumnInfo const& ecsqlColumnInfo, ECN::ECClassId classId)
            :ECSqlField(ecsqlStatement, ecsqlColumnInfo, false, false), m_classId(classId){}
};

// ======================================================================================
using Class=InstanceReader::Impl::Class;
using Property=InstanceReader::Impl::Property;
using Reader=InstanceReader::Impl::Reader;
using RowRender = InstanceReader::Impl::RowRender;
using SeekPos=InstanceReader::Impl::SeekPos;
using Table=InstanceReader::Impl::Table;
using PropertyExists=InstanceReader::Impl::PropertyExists;
// ======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyExists::Clear() const {
    m_propMap.clear();
    m_props.clear();
    m_classIds.clear();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool PropertyExists::Exists(ECN::ECClassId classId, Utf8CP accessString) const {
    if(!m_classIds.insert(classId).second){
        const auto it = m_propMap.find(accessString);
        if (it == m_propMap.end()) {
            return false;
        }
        return it->second.find(classId) != it->second.end();
    }

    // load class access string
    auto stmt = m_conn.GetCachedStatement("SELECT pp.AccessString FROM ec_PropertyMap pm JOIN ec_PropertyPath pp ON pp.Id = pm.PropertyPathId WHERE pm.ClassId = ?");
    stmt->BindId(1, classId);
    while(stmt->Step() == BE_SQLITE_ROW) {
        const auto accessString = stmt->GetValueText(0);
        if (accessString == nullptr) {
            continue;
        }
        auto it = m_propMap.find(accessString);
        if (it != m_propMap.end()) {
            it->second.insert(classId);
        } else {
            m_props.push_back(std::make_unique<std::string>(accessString));
            const auto cachedAccessStr = m_props.back().get()->c_str();
            m_propMap.insert(std::make_pair(cachedAccessStr, std::set<ECN::ECClassId>()))
                .first->second.insert(classId);
        }
    }
    const auto it = m_propMap.find(accessString);
    if (it == m_propMap.end()) {
        return false;
    }
    return it->second.find(classId) != it->second.end();
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BeJsValue SeekPos::GetJson(InstanceReader::JsonParams const& param) const {
    if (m_prop == nullptr) {
        return m_rowRender.GetInstanceJsonObject(ECInstanceKey(m_class->GetClassId(), m_rowId),*this, param);
    }
    return m_rowRender.GetPropertyJsonValue(ECInstanceKey(m_class->GetClassId(), m_rowId), m_accessString, m_prop->GetValue(), param);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Property const* Class::FindProperty(Utf8CP propertyName) const {
    for (const auto& prop : m_properties) {
        if (prop->GetName().EqualsIAscii(propertyName)) {
            return prop.get();
        }
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& Class::GetValue(int index) const {
    if (index < 0 ||  index >=m_properties.size()) {
        return NoopECSqlValue::GetSingleton();
    }
    return m_properties[index]->GetValue();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Property::Property(Table const& table, std::unique_ptr<ECSqlField> field):
    m_table(&table), m_field(std::move(field)){}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
InstanceReader::Impl::Impl(InstanceReader& owner, ECDbCR ecdb): m_reader(ecdb), m_owner(owner){}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
InstanceReader::Impl::~Impl() {}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
RowRender::Document& RowRender::ClearAndGetCachedXmlDocument() const {
    m_cachedXmlDoc.RemoveAllMembers();
    return m_cachedXmlDoc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BeJsValue RowRender::GetInstanceJsonObject(ECInstanceKeyCR instanceKey, IECSqlRow const& ecsqlRow, InstanceReader::JsonParams const& param ) const  {
    if (instanceKey == m_instanceKey && param == m_jsonParam && m_accessString.empty()) {
        return BeJsValue(m_cachedXmlDoc);
    }
    auto& rowsDoc = ClearAndGetCachedXmlDocument();
    BeJsValue row(rowsDoc);
    QueryJsonAdaptor adaptor(m_conn);
    adaptor.UseJsNames(param.GetUseJsName());
    adaptor.SetAbbreviateBlobs(param.GetAbbreviateBlobs());
    adaptor.SetConvertClassIdsToClassNames(param.GetClassIdToClassNames());
    adaptor.RenderRow(row, ecsqlRow, false);
    m_instanceKey = instanceKey;
    m_jsonParam = param;
    m_accessString.clear();
    return row;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BeJsValue RowRender::GetPropertyJsonValue(ECInstanceKeyCR instanceKey, Utf8StringCR  accessString, IECSqlValue const& ecsqlValue, InstanceReader::JsonParams const& param) const  {
    if (instanceKey == m_instanceKey && param == m_jsonParam && m_accessString.Equals(accessString)) {
        return BeJsValue(m_cachedXmlDoc)["$"];
    }
    auto& rowsDoc = ClearAndGetCachedXmlDocument();
    BeJsValue row(rowsDoc);
    auto out = row["$"];
    QueryJsonAdaptor adaptor(m_conn);
    adaptor.UseJsNames(param.GetUseJsName());
    adaptor.SetAbbreviateBlobs(param.GetAbbreviateBlobs());
    adaptor.SetConvertClassIdsToClassNames(param.GetClassIdToClassNames());
    adaptor.RenderValue(out, ecsqlValue);
    m_instanceKey = instanceKey;
    m_jsonParam = param;
    m_accessString.assign(accessString);
    return out;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void Reader::Clear() const {
    BeMutexHolder holder(m_mutex);
    m_queryTableMap.clear();
    m_queryClassMap.clear();
    m_seekPos.Reset();
    m_propExists.Clear();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool Reader::Seek(InstanceReader::Position const& pos, InstanceReader::RowCallback callback) const {
    BeMutexHolder holder(m_mutex);
    Position rsPos = pos;
    const auto thisClassName = pos.GetClassFullName();
    if (thisClassName != nullptr) {
        auto& lastClassName = m_lastClassResolved.m_className;
        auto& lastClassId = m_lastClassResolved.m_classId;
        if (lastClassName.Equals(thisClassName)) {
            rsPos = pos.Resolve(lastClassId);
        } else {
            auto classDef = m_conn.Schemas().FindClass(thisClassName);
            if (classDef == nullptr) {
                return false;
            }
            const auto classId = classDef->GetId();
            rsPos = pos.Resolve(classId);
            lastClassName.assign(thisClassName);
            lastClassId = classId;
        }
    }

    const auto whatChanged = m_seekPos.Compare(rsPos);
    bool hasRow = false;
    if (whatChanged == SeekPos::CompareResult::SameRowAndSchema) {
        hasRow = m_seekPos.HasRow();
    } else if (whatChanged == SeekPos::CompareResult::SameSchema) {
        hasRow =  m_seekPos.Seek(rsPos.GetInstanceId());
    } else {
        if (!PrepareRowSchema(rsPos.GetClassId(), rsPos.GetAccessString())) {
            return false;
        }
        hasRow = m_seekPos.Seek(rsPos.GetInstanceId());
    }
    if (hasRow) {
        callback(m_seekPos);
    }
    return hasRow;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool Reader::PrepareRowSchema(ECN::ECClassId classId, Utf8CP accessString) const {
    /*
        // Following may not be required as the FindProperty is also good enough
        if (accessString != nullptr ){
            if (!m_propExists.Exists(classId, accessString)) {
                return false;
            }
        }
    */
    const auto queryClass = GetOrAddClass(classId);
    if (queryClass == nullptr) {
        return false;
    }
    if (accessString != nullptr) {
        const auto queryProp = queryClass->FindProperty(accessString);
        if (queryProp == nullptr) {
            return false;
        }
        m_seekPos.Reset(*queryClass, *queryProp, accessString);
    } else {
        m_seekPos.Reset(*queryClass);
    }
    return true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool SeekPos::Seek(ECInstanceId rowId) {
    if (m_class == nullptr) {
        return false;
    }
    m_rowClassId = m_class->GetClassId();
    bool seekResult = false;;
    if (m_prop != nullptr) {
        seekResult =  m_prop->Seek(rowId, m_rowClassId);
    } else {
        seekResult = m_class->Seek(rowId, m_rowClassId);
    }
    m_rowId = seekResult ? rowId : ECInstanceId();
    return seekResult;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
SeekPos::CompareResult SeekPos::Compare(InstanceReader::Position pos) {
    if (m_class == nullptr) {
        return CompareResult::None;
    }
    if (m_class->GetClassId() !=  pos.GetClassId()) {
        return CompareResult::None;
    }

    const auto lhsIsNull = m_prop == nullptr;
    const auto rhsIsNull = pos.GetAccessString() == nullptr;
    if (lhsIsNull && rhsIsNull) {
        return m_rowId == pos.GetInstanceId() ? CompareResult::SameRowAndSchema : CompareResult::SameSchema;
    }
    if (lhsIsNull != rhsIsNull) {
        return CompareResult::None;
    }
    if (!m_accessString.EqualsIAscii(pos.GetAccessString())) {
        return CompareResult::None;
    }
    return m_rowId == pos.GetInstanceId() ? CompareResult::SameRowAndSchema : CompareResult::SameSchema;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& SeekPos::GetValue(int columnIndex) const {
    if (m_class == nullptr || columnIndex < 0 || columnIndex >= m_class->GetPropertyCount() || !m_rowId.IsValid()) {
        return NoopECSqlValue::GetSingleton();
    }

    return m_prop == nullptr ? m_class->GetValue(columnIndex): m_prop->GetValue();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int SeekPos::GetColumnCount() const {
    if (m_class == nullptr || !m_rowId.IsValid()) {
        return 0;
    }
    return m_prop == nullptr ?  (int)m_class->GetPropertyCount() : 1;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SeekPos::Reset(Class const& queryClass) const{
    Reset();
    m_class = &queryClass;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SeekPos::Reset() const {
    m_class =nullptr;
    m_accessString.clear();
    m_prop = nullptr;
    m_rowId=ECInstanceId();
    m_rowClassId = ECN::ECClassId();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void SeekPos::Reset(Class const& queryClass, Property const& queryProp, Utf8CP accessString) const{
    Reset(queryClass);
    m_prop = &queryProp;
    m_accessString.assign(accessString);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool Class::Seek(ECInstanceId rowId, ECN::ECClassId& rowClassId) const {
    if(m_tables.empty()) {
        return false;
    }
    auto it = m_tables.begin();
    if (!(*it)->Seek(rowId, &rowClassId)) {
        return false;
    }
    while(++it != m_tables.end()) {
        if (!(*it)->Seek(rowId)) {
            return false;
        }
    }
    return true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool Property::Seek(ECInstanceId rowId, ECN::ECClassId& rowClassId) const {
    return m_table->Seek(rowId, &rowClassId);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool Table::Seek(ECInstanceId rowId, ECN::ECClassId* classId) const {
    auto& stmt = GetSqliteStmt();
    stmt.Reset();
    stmt.ClearBindings();
    stmt.BindId(1, rowId);
    const auto hasRow =  stmt.Step() == BE_SQLITE_ROW;
    if (hasRow && classId && m_ecClassIdCol >= 0) {
        *classId = stmt.GetValueId<ECN::ECClassId>(m_ecClassIdCol);
    }
    return hasRow;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Table const* Reader::GetOrAddTable(DbTableId tableId) const {
    const auto it = m_queryTableMap.find(tableId);
    if (it != m_queryTableMap.end()) {
        return it->second.get();
    }
    const auto tbl = m_conn.Schemas().Main().GetDbSchema().FindTable(tableId);
    if (tbl == nullptr) {
        return nullptr;
    }
    auto queryTable = Table::Create(m_conn, *tbl);
    if (queryTable == nullptr) {
        return nullptr;
    }
    auto newIt = m_queryTableMap.insert(std::make_pair(tableId, std::move(queryTable)));
    return newIt.first->second.get();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Table const* Reader::GetOrAddTable(DbTable const& tbl) const {
    const auto it = m_queryTableMap.find(tbl.GetId());
    if (it != m_queryTableMap.end()) {
        return it->second.get();
    }
    auto queryTable = Table::Create(m_conn, tbl);
    if (queryTable == nullptr) {
        return nullptr;
    }
    auto newIt = m_queryTableMap.insert(std::make_pair(tbl.GetId(), std::move(queryTable)));
    return newIt.first->second.get();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Class const* Reader::GetOrAddClass(ECN::ECClassCR ecClass) const {
    const auto it = m_queryClassMap.find(ecClass.GetId());
    if (it != m_queryClassMap.end()){
        return it->second.get();
    }
    const auto classMap = m_conn.Schemas().Main().GetClassMap(ecClass);
    if (classMap == nullptr) {
        return nullptr;
    }
    auto queryClass = Class::Create(m_conn, *classMap, [&](DbTable const& tbl) {
        return GetOrAddTable(tbl);
    });
    const auto newIt = m_queryClassMap.insert(std::make_pair(ecClass.GetId(), std::move(queryClass)));
    return newIt.first->second.get();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Class const* Reader::GetOrAddClass(ECN::ECClassId classId) const {
    const auto it = m_queryClassMap.find(classId);
    if (it != m_queryClassMap.end()){
        return it->second.get();
    }
    const auto cl = m_conn.Schemas().GetClass(classId);
    if (cl == nullptr) {
        return nullptr;
    }
    const auto classMap = m_conn.Schemas().Main().GetClassMap(*cl);
    if (classMap == nullptr) {
        return nullptr;
    }
    auto queryClass = Class::Create(m_conn, classId, [&](DbTable const& tbl) {
        return GetOrAddTable(tbl);
    });
    const auto newIt = m_queryClassMap.insert(std::make_pair(classId, std::move(queryClass)));
    return newIt.first->second.get();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<Property::Ptr> Class::Factory::Create(ClassMapCR classMap, std::function<Table const*(DbTable const&)> getTable ) {
    std::vector<Property::Ptr> queryProps;
    for (auto& propertyMap : classMap.GetPropertyMaps()){
        GetTablesPropertyMapVisitor visitor( PropertyMap::Type::All);
        propertyMap->AcceptVisitor(visitor);
        const auto table = (*visitor.GetTables().begin());
        const auto queryTable = getTable(*table);
        if (queryTable == nullptr) {
            return std::vector<Property::Ptr>();
        }
        queryProps.emplace_back(Property::Create(
            *queryTable,
            CreateField(
                queryTable->GetECSqlStmt(),
                *propertyMap,
                *queryTable)
            ));
    }
    return queryProps;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
ECSqlPropertyPath Class::Factory::GetPropertyPath (PropertyMap const& propertyMap) {
    ECSqlPropertyPath propertyPath;
    for(auto& part : propertyMap.GetPath()){
        propertyPath.AddEntry(part->GetProperty());
    }
    return propertyPath;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<ECSqlField> Class::Factory::CreatePrimitiveField(ECSqlSelectPreparedStatement& stmt, PropertyMap const& propertyMap, Table const& tbl) {
    const auto prim = propertyMap.GetProperty().GetAsPrimitiveProperty();
    ECSqlColumnInfo columnInfo(
        ECN::ECTypeDescriptor(prim->GetType()),
        DateTime::Info(),
        nullptr,
        &propertyMap.GetProperty(),
        &propertyMap.GetProperty(),
        propertyMap.IsSystem(),
        false /* = isGenerated */,
        GetPropertyPath(propertyMap),
        ECSqlColumnInfo::RootClass(propertyMap.GetClassMap().GetClass(), "")
    );
    if (prim->GetType() == ECN::PRIMITIVETYPE_Point2d) {
        const auto& pt2dMap = propertyMap.GetAs<Point2dPropertyMap>();
        const auto xCol = tbl.GetColumnIndexOf(pt2dMap.GetX().GetColumn());
        const auto yCol = tbl.GetColumnIndexOf(pt2dMap.GetY().GetColumn());
        return std::make_unique<PointECSqlField>(stmt, columnInfo, xCol, yCol);
    } else if (prim->GetType() == PRIMITIVETYPE_Point3d) {
        const auto& pt3dMap = propertyMap.GetAs<Point3dPropertyMap>();
        const auto xCol = tbl.GetColumnIndexOf(pt3dMap.GetX().GetColumn());
        const auto yCol = tbl.GetColumnIndexOf(pt3dMap.GetY().GetColumn());
        const auto zCol = tbl.GetColumnIndexOf(pt3dMap.GetZ().GetColumn());
        return std::make_unique<PointECSqlField>(stmt, columnInfo, xCol, yCol, zCol);
    } else {
        const auto& primMap = propertyMap.GetAs<SingleColumnDataPropertyMap>();
        const auto nCol = tbl.GetColumnIndexOf(primMap.GetColumn());
        return std::make_unique<PrimitiveECSqlField>(stmt, columnInfo, nCol);
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<ECSqlField>  Class::Factory::CreateSystemField(ECSqlSelectPreparedStatement& stmt, PropertyMap const& propertyMap, Table const& tbl) {
    const auto prim = propertyMap.GetProperty().GetAsPrimitiveProperty();
    ECSqlColumnInfo columnInfo(
        ECN::ECTypeDescriptor(prim->GetType()),
        DateTime::Info(),
        nullptr,
        &propertyMap.GetProperty(),
        &propertyMap.GetProperty(),
        propertyMap.IsSystem(),
        false /* = isGenerated */,
        GetPropertyPath(propertyMap),
        ECSqlColumnInfo:: RootClass(propertyMap.GetClassMap().GetClass(), "")
    );
    const auto& sysMap = propertyMap.GetAs<SystemPropertyMap>();
    const auto dataMap = sysMap.GetDataPropertyMaps().front();
    if (dataMap->GetColumn().IsVirtual()) {
        const auto& ecClass = propertyMap.GetClassMap().GetClass();
        const auto extendedType = ExtendedTypeHelper::GetExtendedType(prim->GetExtendedTypeName());
        if (extendedType == ExtendedTypeHelper::ExtendedType::ClassId) {
            return CreateClassIdField(stmt, propertyMap, ecClass.GetId(), tbl);
        } else if (extendedType == ExtendedTypeHelper::ExtendedType::SourceClassId) {
            const auto constraintClass = ecClass.GetRelationshipClassCP()->GetSource().GetConstraintClasses().front();
            return CreateClassIdField(stmt, propertyMap, constraintClass->GetId(), tbl);
        } else if (extendedType == ExtendedTypeHelper::ExtendedType::SourceClassId) {
            const auto constraintClass = ecClass.GetRelationshipClassCP()->GetTarget().GetConstraintClasses().front();
            return CreateClassIdField(stmt, propertyMap, constraintClass->GetId(), tbl);
        } else {
            BeAssert(false);
        }
    }
    const auto nCol = tbl.GetColumnIndexOf(dataMap->GetColumn());
    return std::make_unique<PrimitiveECSqlField>(stmt, columnInfo, nCol);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<ECSqlField> Class::Factory::CreateStructField(ECSqlSelectPreparedStatement& stmt, PropertyMap const& propertyMap, Table const& tbl) {
    const auto structProp = propertyMap.GetProperty().GetAsStructProperty();
    ECSqlColumnInfo columnInfo(
        ECN::ECTypeDescriptor::CreateStructTypeDescriptor(),
        DateTime::Info(),
        &structProp->GetType(),
        &propertyMap.GetProperty(),
        &propertyMap.GetProperty(),
        propertyMap.IsSystem(),
        false /* = isGenerated */,
        GetPropertyPath(propertyMap),
        ECSqlColumnInfo::RootClass(propertyMap.GetClassMap().GetClass(), "")
    );

    auto newStructField = std::make_unique<StructECSqlField>(stmt, columnInfo);
    auto& structPropertyMap =  propertyMap.GetAs<StructPropertyMap>();
    for(auto& memberMap : structPropertyMap) {
        newStructField->AppendField(CreateField(stmt, *memberMap, tbl));
    }
    return std::move(newStructField);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<ECSqlField>  Class::Factory::CreateClassIdField(ECSqlSelectPreparedStatement& stmt, PropertyMap const& propertyMap, ECN::ECClassId id, Table const& tbl) {
    ECSqlColumnInfo columnInfo(
        ECN::ECTypeDescriptor::CreatePrimitiveTypeDescriptor(PRIMITIVETYPE_Long),
        DateTime::Info(),
        nullptr,
        &propertyMap.GetProperty(),
        &propertyMap.GetProperty(),
        propertyMap.IsSystem(),
        false /* = isGenerated */,
        GetPropertyPath(propertyMap),
        ECSqlColumnInfo::RootClass(propertyMap.GetClassMap().GetClass(), "")
    );

    return std::make_unique<ClassIdECSqlField>(stmt, columnInfo, id);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<ECSqlField>  Class::Factory::CreateNavigationField(ECSqlSelectPreparedStatement& stmt, PropertyMap const& propertyMap, Table const& tbl) {
    const auto prim = propertyMap.GetProperty().GetAsNavigationProperty();
    ECSqlColumnInfo columnInfo(
        ECN::ECTypeDescriptor::CreateNavigationTypeDescriptor(prim->GetType(), prim->IsMultiple()),
        DateTime::Info(),
        nullptr,
        &propertyMap.GetProperty(),
        &propertyMap.GetProperty(),
        propertyMap.IsSystem(),
        false /* = isGenerated */,
        GetPropertyPath(propertyMap),
        ECSqlColumnInfo::RootClass(propertyMap.GetClassMap().GetClass(), "")
    );
    const auto& navMap = propertyMap.GetAs<NavigationPropertyMap>();
    auto idField = CreatePrimitiveField(stmt, navMap.GetIdPropertyMap(), tbl);

    std::unique_ptr<ECSqlField> relClassIdField;
    auto& relClassIdMap = navMap.GetRelECClassIdPropertyMap();
    if (relClassIdMap.GetColumn().IsVirtual()){
        relClassIdField = CreateClassIdField(stmt, relClassIdMap, prim->GetRelationshipClass()->GetId(), tbl);
    } else {
        relClassIdField = CreatePrimitiveField(stmt, navMap.GetRelECClassIdPropertyMap(), tbl);
    }
    auto navField = std::make_unique<NavigationPropertyECSqlField>(stmt, columnInfo);
    navField->SetMembers(std::move(idField), std::move(relClassIdField));
    return std::move(navField);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<ECSqlField>   Class::Factory::CreateArrayField(ECSqlSelectPreparedStatement& stmt, PropertyMap const& propertyMap, Table const& tbl) {
    ECN::ECTypeDescriptor desc;
    const auto& prop = propertyMap.GetProperty();
    if (prop.GetIsStructArray()) {
        desc = ECN::ECTypeDescriptor::CreateStructArrayTypeDescriptor();
    } else {
        auto primType = prop.GetAsPrimitiveArrayProperty()->GetPrimitiveElementType();
        desc = ECN::ECTypeDescriptor::CreatePrimitiveArrayTypeDescriptor(primType);
    }
    ECSqlColumnInfo columnInfo(
        desc,
        DateTime::Info(),
        prop.GetIsStructArray()? &prop.GetAsStructArrayProperty()->GetStructElementType(): nullptr,
        &propertyMap.GetProperty(),
        &propertyMap.GetProperty(),
        propertyMap.IsSystem(),
        false /* = isGenerated */,
        GetPropertyPath(propertyMap),
        ECSqlColumnInfo::RootClass(propertyMap.GetClassMap().GetClass(), "")
    );
    auto& primMap = propertyMap.GetAs<SingleColumnDataPropertyMap>();
    auto nCol =tbl. GetColumnIndexOf(primMap.GetColumn());
    return std::make_unique<ArrayECSqlField>(stmt, columnInfo, nCol);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<ECSqlField>  Class::Factory::CreateField(ECSqlSelectPreparedStatement& stmt, PropertyMap const& propertyMap, Table const& tbl){
    const auto& prop = propertyMap.GetProperty();
    if (propertyMap.IsSystem()) {
        return CreateSystemField(stmt, propertyMap, tbl);
    } else if (prop.GetIsPrimitive() ){
        return CreatePrimitiveField(stmt, propertyMap, tbl);
    } else if (prop.GetIsStruct()) {
        return CreateStructField(stmt, propertyMap, tbl);
    } else if (prop.GetIsNavigation()) {
        return CreateNavigationField(stmt, propertyMap, tbl);
    } else if(prop.GetIsArray()) {
        return CreateArrayField(stmt, propertyMap, tbl);
    }
    return nullptr;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Class::Class(ECN::ECClassId classId, std::vector<Property::Ptr> properties)
    : m_id(classId), m_properties(std::move(properties)) {
        std::set<DbTableId> tableMap;
        for (auto& prop : m_properties) {
            const auto id =prop->GetTable().GetId();
            const auto it = tableMap.find(id);
            if (it != tableMap.end()) {
                continue;
            }
            tableMap.insert(prop->GetTable().GetId());
            m_tables.push_back(&prop->GetTable());
        }
    }

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Class::Ptr Class::Create(ECDbCR conn, ClassMapCR classMap, std::function<Table const*(DbTable const&)> getTable) {
    auto props = Factory::Create(classMap, getTable);
    if (props.empty()) {
        return nullptr;
    }
    return std::make_unique<Class>(classMap.GetClass().GetId(), std::move(props));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Class::Ptr Class::Create(ECDbCR conn, ECN::ECClassId classId, std::function<Table const*(DbTable const&)> getTable) {
    auto classP = conn.Schemas().GetClass(classId);
    if (classP == nullptr ) {
        return nullptr;
    }
    if (!classP->IsEntityClass()  && !classP->IsRelationshipClass()){
        return nullptr;
    }
    auto classMap = conn.Schemas().Main().GetClassMap(*classP);
    if (classMap == nullptr)  {
        return nullptr;
    }
    auto props = Factory::Create(*classMap, getTable);
    if (props.empty()) {
        return nullptr;
    }
    return std::make_unique<Class>(classId, std::move(props));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Property::Ptr Property::Create(Table const& table,  std::unique_ptr<ECSqlField> field) {
    return std::make_unique<Property> (table, std::move(field));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int Table::GetColumnIndexOf(DbColumnId id) const {
    const auto it = m_colIndexMap.find(id);
    return it == m_colIndexMap.end() ? -1 : it->second;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Table::Ptr Table::Create(ECDbCR conn, DbTable const& tbl) {
    NativeSqlBuilder builder;
    Table::Ptr queryTable = std::make_unique<Table>(conn);
    queryTable->m_id = tbl.GetId();

    auto const& columns = tbl.GetColumns();
    int appendCount = 0;
    builder.Append("SELECT ");
    for (auto idx= 0;idx<columns.size(); ++idx) {
        auto& col = columns[idx];
        if (col->IsVirtual()) {
            continue;
        }
        if (appendCount> 0) {
            builder.AppendComma();
        }
        builder.AppendEscaped(col->GetName().c_str());
        queryTable->m_colIndexMap.insert(std::make_pair(col->GetId(), appendCount));
        if (col == &tbl.GetECClassIdColumn()) {
            queryTable->m_ecClassIdCol = appendCount;
        }
        appendCount++;
    }

    if (tbl.GetType() != DbTable::Type::Virtual) {
        builder.Append(" FROM ");
        builder.AppendEscaped(tbl.GetName());
        builder.Append(" WHERE [ROWID]=?");
    }

    const auto rc = queryTable->GetSqliteStmt().Prepare(conn, builder.GetSql().c_str());
    UNUSED_VARIABLE(rc);
    BeAssert(rc == BE_SQLITE_OK);
    return queryTable;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
InstanceReader::~InstanceReader() {
    if (m_pImpl != nullptr) {
        delete m_pImpl;
        m_pImpl = nullptr;
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
InstanceReader::InstanceReader(ECDbCR ecdb): m_pImpl(new Impl(*this, ecdb)) {}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool InstanceReader::Seek(Position const& pos, RowCallback callback ) const {
    return m_pImpl->Seek(pos, callback);
}

//////////////////////////////////////////////////////////////////////////
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void InMemoryPropertyExistMap::Insert(ECN::ECClassId classId, Utf8CP propertyName) {
    auto it = m_propMap.find(propertyName);
    if (it == m_propMap.end()) {
        m_cachedPropNames.push_back(std::make_unique<std::string>(propertyName));
        const auto str = m_cachedPropNames.back()->c_str();
        m_propMap[str].insert(classId.GetValueUnchecked());
    } else {
        m_propMap[propertyName].insert(classId.GetValueUnchecked());
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DbResult InMemoryPropertyExistMap::Build(ECDbCR ecdb, bool onlyTopLevelProperties) {
    m_propMap.clear();
    m_cachedPropNames.clear();
    Statement stmt;
    // when db is created initially it does not have ec_PropertyMap table
    // which cause unecessary logging message.
    if (!ecdb.TableExists("ec_PropertyMap")) {
        return BE_SQLITE_ERROR;
    }

    const auto rc = onlyTopLevelProperties ?
        stmt.Prepare(ecdb,"SELECT pp.AccessString, pm.ClassId FROM ec_PropertyMap pm JOIN ec_PropertyPath pp on pp.Id = pm.PropertyPathId WHERE instr(pp.AccessString,'.')=0"):
        stmt.Prepare(ecdb,"SELECT pp.AccessString, pm.ClassId FROM ec_PropertyMap pm JOIN ec_PropertyPath pp on pp.Id = pm.PropertyPathId") ;
    if (rc != BE_SQLITE_OK) {
        return rc;
    }
    while(stmt.Step() == BE_SQLITE_ROW) {
        Insert(stmt.GetValueId<ECN::ECClassId>(1), stmt.GetValueText(0));
    }
    return rc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool InMemoryPropertyExistMap::Exist(ECN::ECClassId classId, Utf8CP propertyName) const {
    auto it = m_propMap.find(propertyName);
    if (it == m_propMap.end()) {
        return false;
    }
    const auto &classIdSet = it->second;
    return classIdSet.find(classId.GetValueUnchecked()) != classIdSet.end();
}

END_BENTLEY_SQLITE_EC_NAMESPACE
