/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
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
using TableView=InstanceReader::Impl::TableView;
using PropertyExists=InstanceReader::Impl::PropertyExists;
// ======================================================================================

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void PropertyExists::Clear() const {
    m_propHashTable.clear();
    m_props.clear();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+-----
void PropertyExists::Load() const{
    Clear();
    auto sql = R"x(
        SELECT
            [pm].[ClassId],
            [pp].[AccessString] [prop]
        FROM [main].[ec_PropertyMap] [pm]
            JOIN [main].[ec_ClassMap] [cm] ON [cm].[ClassId] = [pm].[ClassId] AND [cm].[MapStrategy] NOT IN (0, 10, 11)
            JOIN [main].[ec_PropertyPath] [pp] ON [pm].[PropertyPathId] = [pp].[Id]
        UNION
        SELECT
            [pm].[ClassId],
            SUBSTR ([pp].[AccessString], 0, INSTR ([pp].[AccessString], '.')) [prop]
        FROM [main].[ec_PropertyMap] [pm]
            JOIN [main].[ec_ClassMap] [cm] ON [cm].[ClassId] = [pm].[ClassId] AND [cm].[MapStrategy] NOT IN (0, 10, 11)
            JOIN [main].[ec_PropertyPath] [pp] ON [pm].[PropertyPathId] = [pp].[Id] AND INSTR ([pp].[AccessString], '.') != 0
    )x";

    auto stmt = m_conn.GetCachedStatement(sql);
    std::set<Utf8CP, CompareIUtf8Ascii> props;
    std::set<ECN::ECClassId> classIds;
    Entry entry;
    while(stmt->Step() == BE_SQLITE_ROW) {
        entry.m_classId = stmt->GetValueId<ECClassId>(0);
        const auto accessString = stmt->GetValueText(1);
        if (classIds.find(entry.m_classId) == classIds.end()) {
            classIds.insert(entry.m_classId);
            entry.m_accessString = nullptr;
            m_propHashTable.insert(entry);
        }
        auto it = props.find(accessString);
        if (it == props.end()) {
            entry.m_accessString = m_props.emplace_back(std::make_unique<Utf8String>(accessString)).get()->c_str();
            props.insert(entry.m_accessString);
        } else {
            entry.m_accessString = *it;
        }
        m_propHashTable.insert(entry);
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool PropertyExists::Exists(ECN::ECClassId classId, Utf8CP accessString) const {
    if (m_propHashTable.empty()) Load();
    Entry entry;
    entry.m_classId = classId;
    entry.m_accessString = accessString;
    return m_propHashTable.find(entry) != m_propHashTable.end();
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BeJsValue SeekPos::GetJson(JsReadOptions const& param) const {
    if (m_prop == nullptr) {
        return m_rowRender.GetInstanceJsonObject(ECInstanceKey(m_class->GetClassId(), m_rowId),*this, param);
    }
    return m_rowRender.GetPropertyJsonValue(ECInstanceKey(m_class->GetClassId(), m_rowId), m_accessString, m_prop->GetValue(), param);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Property const* Class::FindProperty(Utf8CP propertyName) const {
    auto it = m_propertyMap.find(propertyName);
    if (it == m_propertyMap.end()) return nullptr;
    return it->second;
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
Property::Property(TableView const& table, std::unique_ptr<ECSqlField> field):
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
RowRender::Document& RowRender::ClearAndGetCachedJsonDocument() const {
    m_allocator.Clear();
    m_cachedJsonDoc.RemoveAllMembers();
    m_cachedJsonDoc.SetObject();
    return m_cachedJsonDoc;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void RowRender::Reset() {
    m_allocator.Clear();
    m_cachedJsonDoc.RemoveAllMembers();
    m_cachedJsonDoc.SetObject();
    m_instanceKey = ECInstanceKey();
    m_accessString.clear();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BeJsValue RowRender::GetInstanceJsonObject(ECInstanceKeyCR instanceKey, IECSqlRow const& ecsqlRow, JsReadOptions const& param ) const  {
    if (instanceKey == m_instanceKey && param == m_jsonParam && m_accessString.empty() && !(m_conn.IsDbOpen() && m_conn.IsWriteable())) {
        return BeJsValue(m_cachedJsonDoc);
    }
    auto& rowsDoc = ClearAndGetCachedJsonDocument();
    BeJsValue row(rowsDoc);
    ECSqlRowAdaptor adaptor(m_conn, param);
    adaptor.RenderRowAsObject(row, ecsqlRow);
    m_instanceKey = instanceKey;
    m_jsonParam = param;
    m_accessString.clear();
    return row;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
BeJsValue RowRender::GetPropertyJsonValue(ECInstanceKeyCR instanceKey, Utf8StringCR  accessString, IECSqlValue const& ecsqlValue, JsReadOptions const& param) const  {
    if (instanceKey == m_instanceKey && param == m_jsonParam && m_accessString.Equals(accessString)) {
        return BeJsValue(m_cachedJsonDoc)["$"];
    }
    auto& rowsDoc = ClearAndGetCachedJsonDocument();
    BeJsValue row(rowsDoc);
    auto out = row["$"];
    ECSqlRowAdaptor adaptor(m_conn, param);
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
    m_lastClassResolved = LastClassResolved();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void Reader::InvalidateSeekPos(ECInstanceKey const& key){
    if (!key.IsValid()) {
        m_seekPos.Reset();
    }
    if (m_seekPos.GetRowId() == key.GetInstanceId() && m_seekPos.GetClass() != nullptr && m_seekPos.GetClass()->GetClassId() == key.GetClassId()) {
        m_seekPos.Reset();
    }
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool Reader::Seek(InstanceReader::Position const& pos, InstanceReader::RowCallback callback, InstanceReader::Options const& opt) const {
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
        bool forceSeek = opt.GetForceSeek();
        hasRow = forceSeek ? m_seekPos.Seek(rsPos.GetInstanceId()) : m_seekPos.HasRow();
    } else if (whatChanged == SeekPos::CompareResult::SameSchema) {
        hasRow = m_seekPos.Seek(rsPos.GetInstanceId());
    } else {
        if (!PrepareRowSchema(rsPos.GetClassId(), rsPos.GetAccessString())) {
            return false;
        }
        hasRow = m_seekPos.Seek(rsPos.GetInstanceId());
    }
    if (hasRow) {

        callback(m_seekPos, [&](Utf8CP propName) -> std::optional<PropertyReader> {
            auto prop = m_seekPos.GetClass()->FindProperty(propName);
            if (prop == nullptr) {
                return std::nullopt;
            }
            return PropertyReader(prop->GetValue());
        });
    }
    return hasRow;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool Reader::PrepareRowSchema(ECN::ECClassId classId, Utf8CP accessString) const {
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
    m_rowRender.Reset();
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
    for (auto& prop: m_properties) {
        prop->OnAfterReset();
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
    for (auto& prop: m_properties) {
        prop->OnAfterStep();
    }
    return true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool Property::Seek(ECInstanceId rowId, ECN::ECClassId& rowClassId) const {
    OnAfterReset();
    bool result = m_table->Seek(rowId, &rowClassId);
    OnAfterStep();
    return result;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool TableView::Seek(ECInstanceId rowId, ECN::ECClassId* classId) const {
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
TableView const* Reader::GetOrAddTable(DbTableId tableId) const {
    const auto it = m_queryTableMap.find(tableId);
    if (it != m_queryTableMap.end()) {
        return it->second.get();
    }
    const auto tbl = m_conn.Schemas().Main().GetDbSchema().FindTable(tableId);
    if (tbl == nullptr) {
        return nullptr;
    }
    auto queryTable = TableView::Create(m_conn, *tbl);
    if (queryTable == nullptr) {
        return nullptr;
    }
    auto newIt = m_queryTableMap.insert(std::make_pair(tableId, std::move(queryTable)));
    return newIt.first->second.get();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TableView const* Reader::GetOrAddTable(DbTable const& tbl) const {
    const auto it = m_queryTableMap.find(tbl.GetId());
    if (it != m_queryTableMap.end()) {
        return it->second.get();
    }
    auto queryTable = TableView::Create(m_conn, tbl);
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
std::vector<Property::Ptr> Class::Factory::Create(ClassMapCR classMap, std::function<TableView const*(DbTable const&)> getTable ) {
    std::vector<Property::Ptr> queryProps;
    for (auto& propertyMap : classMap.GetPropertyMaps()){
        GetTablesPropertyMapVisitor visitor(PropertyMap::Type::All);
        propertyMap->AcceptVisitor(visitor);
        DbTable const* table =  (*visitor.GetTables().begin());
        if (propertyMap->GetType() == PropertyMap::Type::ConstraintECClassId) {
            if (!propertyMap->IsMappedToClassMapTables()) {
                table = classMap.GetTables().front();
            }
        }
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
std::unique_ptr<ECSqlField> Class::Factory::CreatePrimitiveField(ECSqlSelectPreparedStatement& stmt, PropertyMap const& propertyMap, TableView const& tbl) {
    const auto prim = propertyMap.GetProperty().GetAsPrimitiveProperty();
    ECSqlColumnInfo columnInfo(
        ECN::ECTypeDescriptor(prim->GetType()),
        GetDateTimeInfo(propertyMap),
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
std::unique_ptr<ECSqlField>  Class::Factory::CreateSystemField(ECSqlSelectPreparedStatement& stmt, PropertyMap const& propertyMap, TableView const& tbl) {
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

    const auto extendedType = ExtendedTypeHelper::GetExtendedType(prim->GetExtendedTypeName());
    if (extendedType == ExtendedTypeHelper::ExtendedType::ClassId && tbl.GetClassIdCol() >= 0) {
         return std::make_unique<PrimitiveECSqlField>(stmt, columnInfo, tbl.GetClassIdCol());
    }
    if (extendedType == ExtendedTypeHelper::ExtendedType::SourceClassId && tbl.GetSourceClassIdCol() >= 0) {
         return std::make_unique<PrimitiveECSqlField>(stmt, columnInfo, tbl.GetSourceClassIdCol());
    }
    if (extendedType == ExtendedTypeHelper::ExtendedType::TargetClassId && tbl.GetTargetClassIdCol() >= 0) {
         return std::make_unique<PrimitiveECSqlField>(stmt, columnInfo, tbl.GetTargetClassIdCol());
    }


     const auto& sysMap = propertyMap.GetAs<SystemPropertyMap>();
     const auto dataMap = sysMap.GetDataPropertyMaps().front();
     if (dataMap->GetColumn().IsVirtual()) {
        //  const auto& ecClass = propertyMap.GetClassMap().GetClass();
        //  if (extendedType == ExtendedTypeHelper::ExtendedType::ClassId) {
        //     return CreateClassIdField(stmt, propertyMap, ecClass.GetId(), tbl);
        // } else if (extendedType == ExtendedTypeHelper::ExtendedType::SourceClassId) {
        //     const auto constraintClass = ecClass.GetRelationshipClassCP()->GetSource().GetConstraintClasses().front();
        //     return CreateClassIdField(stmt, propertyMap, constraintClass->GetId(), tbl);
        // } else if (extendedType == ExtendedTypeHelper::ExtendedType::SourceClassId) {
        //     const auto constraintClass = ecClass.GetRelationshipClassCP()->GetTarget().GetConstraintClasses().front();
        //     return CreateClassIdField(stmt, propertyMap, constraintClass->GetId(), tbl);
        // } else {
            BeAssert(false);
//        }
    }
     const auto nCol = tbl.GetColumnIndexOf(dataMap->GetColumn());
     return std::make_unique<PrimitiveECSqlField>(stmt, columnInfo, nCol);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<ECSqlField> Class::Factory::CreateStructField(ECSqlSelectPreparedStatement& stmt, PropertyMap const& propertyMap, TableView const& tbl) {
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
std::unique_ptr<ECSqlField>  Class::Factory::CreateClassIdField(ECSqlSelectPreparedStatement& stmt, PropertyMap const& propertyMap, ECN::ECClassId id, TableView const& tbl) {
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
std::unique_ptr<ECSqlField>  Class::Factory::CreateNavigationField(ECSqlSelectPreparedStatement& stmt, PropertyMap const& propertyMap, TableView const& tbl) {
    const auto prim = propertyMap.GetProperty().GetAsNavigationProperty();
    ECSqlColumnInfo columnInfo(
        ECN::ECTypeDescriptor::CreateNavigationTypeDescriptor(prim->GetType(), prim->IsMultiple()),
        GetDateTimeInfo(propertyMap),
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
DateTime::Info Class::Factory::GetDateTimeInfo(PropertyMap const& propertyMap) {
    DateTime::Info info = DateTime::Info::CreateForDateTime(DateTime::Kind::Unspecified);
    if (propertyMap.GetType() != PropertyMap::Type::PrimitiveArray && propertyMap.GetType() != PropertyMap::Type::Primitive) {
        return info;
    }

    if (auto property = propertyMap.GetProperty().GetAsPrimitiveArrayProperty()){
        if (property->GetType() == PRIMITIVETYPE_DateTime) {
            if (CoreCustomAttributeHelper::GetDateTimeInfo(info, *property) == ECObjectsStatus::Success) {
                return info;
            }
        }
    }
    if (auto property = propertyMap.GetProperty().GetAsPrimitiveProperty()){
        if (property->GetType() == PRIMITIVETYPE_DateTime) {
            if (CoreCustomAttributeHelper::GetDateTimeInfo(info, *property) == ECObjectsStatus::Success) {
                return info;
            }
        }
    }

    return info;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<ECSqlField>   Class::Factory::CreateArrayField(ECSqlSelectPreparedStatement& stmt, PropertyMap const& propertyMap, TableView const& tbl) {

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
        GetDateTimeInfo(propertyMap),
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
std::unique_ptr<ECSqlField>  Class::Factory::CreateField(ECSqlSelectPreparedStatement& stmt, PropertyMap const& propertyMap, TableView const& tbl){
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
            m_propertyMap.insert(std::make_pair(prop->GetName().c_str(), prop.get()));
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
Class::Ptr Class::Create(ECDbCR conn, ClassMapCR classMap, std::function<TableView const*(DbTable const&)> getTable) {
    auto props = Factory::Create(classMap, getTable);
    if (props.empty()) {
        return nullptr;
    }
    return std::make_unique<Class>(classMap.GetClass().GetId(), std::move(props));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
Class::Ptr Class::Create(ECDbCR conn, ECN::ECClassId classId, std::function<TableView const*(DbTable const&)> getTable) {
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
Property::Ptr Property::Create(TableView const& table,  std::unique_ptr<ECSqlField> field) {
    return std::make_unique<Property> (table, std::move(field));
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int TableView::GetColumnIndexOf(DbColumnId id) const {
    const auto it = m_colIndexMap.find(id);
    return it == m_colIndexMap.end() ? -1 : it->second;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TableView::Ptr TableView::CreateNullTableView(ECDbCR conn, DbTable const& tbl) {
    if (tbl.GetType() != DbTable::Type::Virtual) {
        BeAssert(false && "Expect a virtual table");
        return nullptr;
    }

    auto tableView = std::make_unique<TableView>(conn);
    NativeSqlBuilder builder;
    builder.Append("SELECT ");
    int appendIndex = 0;
    for (auto col : tbl.GetColumns()) {
        if (col != tbl.GetColumns().front()) {
            builder.AppendComma();
        }

        builder.Append("NULL")
            .AppendSpace()
            .AppendEscaped(col->GetName());

        tableView->m_colIndexMap.insert(std::make_pair(col->GetId(), appendIndex));
        appendIndex++;
    }
    builder.AppendSpace().Append("LIMIT 0");

    tableView->m_id = tbl.GetId();
    const auto rc = tableView->GetSqliteStmt().Prepare(conn, builder.GetSql().c_str());
    if (rc != BE_SQLITE_OK) {
         BeAssert(false && "Failed to prepare statement");
        return nullptr;
    }
    return tableView;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TableView::Ptr TableView::CreateTableView(ECDbCR conn, DbTable const& tbl) {
    auto tableView = std::make_unique<TableView>(conn);
    NativeSqlBuilder builder;
    auto const& columns = tbl.GetColumns();
    int appendCount = 0;
    builder.Append("SELECT ");
    for (auto idx= 0; idx < columns.size(); ++idx) {
        auto& col = columns[idx];
        if (col->IsVirtual()) {
            continue;
        }
        if (appendCount > 0) {
            builder.AppendComma();
        }
        builder.AppendEscaped(col->GetName());
        tableView->m_colIndexMap.insert(std::make_pair(col->GetId(), appendCount));
        if (col == &tbl.GetECClassIdColumn()) {
            tableView->m_ecClassIdCol = appendCount;
        }
        appendCount++;
    }

    builder.Append(" FROM ");
    builder.AppendEscaped(tbl.GetName());
    builder.Append(" WHERE [ROWID]=?");

    tableView->m_id = tbl.GetId();
    const auto rc = tableView->GetSqliteStmt().Prepare(conn, builder.GetSql().c_str());
    if (rc != BE_SQLITE_OK) {
         BeAssert(false && "Failed to prepare statement");
        return nullptr;
    }
    return tableView;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TableView::Ptr TableView::CreateLinkTableView(ECDbCR conn, DbTable const& tbl, RelationshipClassLinkTableMap const& rootMap) {
    auto tableView = std::make_unique<TableView>(conn);
    NativeSqlBuilder builder;
    auto const& columns = tbl.GetColumns();
    int appendCount = 0;

    const auto& sourceClassIdSysProp = rootMap.GetSourceECClassIdPropMap()->GetAs<SystemPropertyMap>();
    const auto& targetClassIdSysProp = rootMap.GetTargetECClassIdPropMap()->GetAs<SystemPropertyMap>();

    const auto& sourceIdSysProp = rootMap.GetSourceECInstanceIdPropMap()->GetAs<SystemPropertyMap>().GetDataPropertyMaps().front();
    const auto& targetIdSysProp = rootMap.GetTargetECInstanceIdPropMap()->GetAs<SystemPropertyMap>().GetDataPropertyMaps().front();

    const auto sourceClassIdProp = sourceClassIdSysProp.GetDataPropertyMaps().front();
    const auto targetClassIdProp = targetClassIdSysProp.GetDataPropertyMaps().front();

    builder.Append("SELECT ");
    for (auto idx= 0; idx < columns.size(); ++idx) {
        auto& col = columns[idx];
        if (appendCount > 0) {
            builder.AppendComma();
        }
        if (col->IsVirtual()) {
            if (col == &tbl.GetECClassIdColumn()) {
                builder.Append(rootMap.GetClass().GetId().ToHexStr())
                    .AppendSpace()
                    .AppendEscaped(col->GetName());
            } else if (col == &sourceClassIdProp->GetColumn()) {
                builder.Append(rootMap.GetRelationshipClass().GetSource().GetConstraintClasses().front()->GetId().ToHexStr())
                    .AppendSpace()
                    .AppendEscaped(col->GetName());
            } else if (col == &targetClassIdProp->GetColumn()) {
                builder.Append(rootMap.GetRelationshipClass().GetTarget().GetConstraintClasses().front()->GetId().ToHexStr())
                    .AppendSpace()
                    .AppendEscaped(col->GetName());
            } else {
                continue;
            }
        }

        builder.AppendFullyQualified(tbl.GetName(), col->GetName());
        tableView->m_colIndexMap.insert(std::make_pair(col->GetId(), appendCount));
        if (col == &tbl.GetECClassIdColumn()) {
            tableView->m_ecClassIdCol = appendCount;
        }
        if (col == &tbl.GetECClassIdColumn()) {
            tableView->m_ecSourceClassIdCol = appendCount;
        }
        if (col == &tbl.GetECClassIdColumn()) {
            tableView->m_ecTargetClassIdCol = appendCount;
        }
        ++appendCount;
    }

    NativeSqlBuilder sourceJoinBuilder;
    NativeSqlBuilder targetJoinBuilder;

    --appendCount;
    if (!sourceClassIdSysProp.IsMappedToClassMapTables()) {
        const auto kSourceTableAlias = "SourceClassIdTable";
        builder.AppendComma();
        builder.AppendEscaped(kSourceTableAlias)
            .AppendDot()
            .AppendEscaped(sourceClassIdProp->GetColumn().GetName());
        tableView->m_ecSourceClassIdCol = ++appendCount;

        auto& classIdTable = sourceClassIdProp->GetTable();
        sourceJoinBuilder.AppendFormatted(
            "JOIN [%s] [%s] ON [%s].[%s] = [%s].[%s]",
            classIdTable.GetName().c_str(),
            kSourceTableAlias,
            kSourceTableAlias,
            classIdTable.FindFirst(DbColumn::Kind::ECInstanceId)->GetName().c_str(),
            tbl.GetName().c_str(),
            sourceIdSysProp->GetColumn().GetName().c_str()
        );
    }
    if (!rootMap.GetTargetECClassIdPropMap()->IsMappedToClassMapTables()) {
        const auto kTargetTableAlias = "TargetClassIdTable";
        builder.AppendComma();
        builder.AppendEscaped(kTargetTableAlias)
            .AppendDot()
            .AppendEscaped(targetClassIdProp->GetColumn().GetName());
        tableView->m_ecTargetClassIdCol = ++appendCount;

        auto& classIdTable = targetClassIdProp->GetTable();
        sourceJoinBuilder.AppendFormatted(
            "JOIN [%s] [%s] ON [%s].[%s] = [%s].[%s]",
            classIdTable.GetName().c_str(),
            kTargetTableAlias,
            kTargetTableAlias,
            classIdTable.FindFirst(DbColumn::Kind::ECInstanceId)->GetName().c_str(),
            tbl.GetName().c_str(),
            targetIdSysProp->GetColumn().GetName().c_str()
        );
    }

    builder.Append(" FROM ");
    builder.AppendEscaped(tbl.GetName());

    if (!sourceJoinBuilder.IsEmpty()) {
        builder.Append(sourceJoinBuilder);
    }
    if (!targetJoinBuilder.IsEmpty()) {
        builder.Append(targetJoinBuilder);
    }

    builder.AppendFormatted(" WHERE [%s].[ROWID]=?", tbl.GetName().c_str());
    tableView->m_id = tbl.GetId();
    const auto rc = tableView->GetSqliteStmt().Prepare(conn, builder.GetSql().c_str());
    if (rc != BE_SQLITE_OK) {
         BeAssert(false && "Failed to prepare statement");
        return nullptr;
    }
    return tableView;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TableView::Ptr TableView::CreateEntityTableView(ECDbCR conn, DbTable const& tbl, ClassMapCR rootMap){
    auto tableView = std::make_unique<TableView>(conn);
    NativeSqlBuilder builder;
    auto const& columns = tbl.GetColumns();
    int appendCount = 0;
    builder.Append("SELECT ");
    for (auto idx= 0; idx < columns.size(); ++idx) {
        auto& col = columns[idx];
        if (col == &tbl.GetECClassIdColumn()) {
            if (appendCount > 0) {
                builder.AppendComma();
            }
            if (col->IsVirtual()) {
                builder.Append(rootMap.GetClass().GetId().ToHexStr())
                    .AppendSpace()
                    .AppendEscaped(col->GetName());
            } else {
                builder.AppendEscaped(col->GetName().c_str());
            }
            tableView->m_ecClassIdCol = appendCount;
        } else {
            if (col->IsVirtual()) {
                //! RelECClassId could be virtual as well.
                continue;
            }
            if (appendCount > 0) {
                builder.AppendComma();
            }
            builder.AppendEscaped(col->GetName().c_str());
            tableView->m_colIndexMap.insert(std::make_pair(col->GetId(), appendCount));
        }
        ++appendCount;
    }

    builder.Append(" FROM ");
    builder.AppendEscaped(tbl.GetName());
    builder.Append(" WHERE [ROWID]=?");

    tableView->m_id = tbl.GetId();
    const auto rc = tableView->GetSqliteStmt().Prepare(conn, builder.GetSql().c_str());
    if (rc != BE_SQLITE_OK) {
         BeAssert(false && "Failed to prepare statement");
        return nullptr;
    }
    return tableView;
}
//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
TableView::Ptr TableView::Create(ECDbCR conn, DbTable const& tbl) {
    auto getRootClassMap = [&]() -> ClassMap const* {
        ECClassId rootClassId;
        if (tbl.GetType() == DbTable::Type::Overflow) {
            rootClassId = tbl.GetLinkNode().GetParent()->GetTable().GetExclusiveRootECClassId();
        } else {
            rootClassId = tbl.GetExclusiveRootECClassId();
        }

        const auto rootClass = conn.Schemas().Main().GetClass(rootClassId);
        if (rootClass != nullptr) {
            return conn.Schemas().Main().GetClassMap(*rootClass);
        }
        return nullptr;
    };

    // virtual table
    if (tbl.GetType() == DbTable::Type::Virtual) {
        return CreateNullTableView(conn, tbl);
    }

    const auto rootClassMap = getRootClassMap();
    if(rootClassMap->GetClass().IsMixin() || rootClassMap->GetType() == ClassMap::Type::RelationshipEndTable) {
        //! NOT SUPPORTED
        return nullptr;
    }

    if (rootClassMap == nullptr) {
        return CreateTableView(conn, tbl);
    }

    if (rootClassMap->GetType() == ClassMap::Type::NotMapped) {
        return nullptr;
    }

    if (rootClassMap->GetType() == ClassMap::Type::Class) {
        return CreateEntityTableView(conn, tbl, *rootClassMap);
    }

    if (rootClassMap->GetType() == ClassMap::Type::RelationshipLinkTable) {
        return CreateLinkTableView(conn, tbl, rootClassMap->GetAs<RelationshipClassLinkTableMap>());
    }
    return nullptr;
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
bool InstanceReader::Seek(Position const& pos, RowCallback callback, Options const& opt) const {
    return m_pImpl->Seek(pos, callback, opt);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void InstanceReader::Reset() {
    return m_pImpl->Reset();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void InstanceReader::InvalidateSeekPos(ECInstanceKey const& key){
    return m_pImpl->InvalidateSeekPos(key);
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
