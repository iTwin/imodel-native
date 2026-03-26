/*---------------------------------------------------------------------------------------------
 * Copyright (c) Bentley Systems, Incorporated. All rights reserved.
 * See LICENSE.md in the repository root for full copyright notice.
 *--------------------------------------------------------------------------------------------*/

#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

using TableView = InstanceReader::Impl::TableView;
using Stage = Changes::Change::Stage;

struct Field : public IECSqlValue {
    using Ptr = std::unique_ptr<Field>;

   protected:
    Changes::Change m_change;
    ECSqlColumnInfo m_columnInfo;
    bool m_requiresOnAfterStep = false;
    bool m_requiresOnAfterReset = false;
    Stage m_stage;

   private:
    ECSqlColumnInfoCR _GetColumnInfo() const override;
    virtual ECSqlStatus _OnAfterReset();
    virtual ECSqlStatus _OnAfterStep();

   protected:
    Field(Changes::Change const& change, Stage const& stage, ECSqlColumnInfo const& columnInfo, bool needsOnAfterStep, bool needsOnAfterReset); 
    DbValue GetSqliteValue(int colNum) const;

   public:
    virtual ~Field() {}
    bool RequiresOnAfterStep() const;
    ECSqlStatus OnAfterStep();
    bool RequiresOnAfterReset() const;
    ECSqlStatus OnAfterReset();
};
struct Factory final {
   private:
    static const ClassMap* GetRootClassMap(DbTable const& tbl, ECDbCR conn);
    static DateTime::Info GetDateTimeInfo(PropertyMap const& propertyMap);
    static ECSqlPropertyPath GetPropertyPath(PropertyMap const&);
    static std::unique_ptr<Field> CreatePrimitiveField(PropertyMap const&, TableView const&, Changes::Change const&, Stage const&);
    static std::unique_ptr<Field> CreateSystemField(PropertyMap const&, TableView const&, Changes::Change const&, Stage const&);
    static std::unique_ptr<Field> CreateStructField(PropertyMap const&, TableView const&, Changes::Change const&, Stage const&);
    static std::unique_ptr<Field> CreateNavigationField(PropertyMap const&, TableView const&, Changes::Change const&, Stage const&);
    static std::unique_ptr<Field> CreateArrayField(PropertyMap const&, TableView const&, Changes::Change const&, Stage const&);
    static std::unique_ptr<Field> CreateField(PropertyMap const&, TableView const&, Changes::Change const&, Stage const&);
    static std::unique_ptr<Field> CreateClassIdField(PropertyMap const&, ECN::ECClassId, TableView const&);

   public:
    static std::vector<Field::Ptr> Create(ECDbCR conn, DbTable const& tbl, Changes::Change const& change, Changes::Change::Stage const& stage);
};
struct TableProperties final {
   private:
    DbTable const& m_table;
    std::vector<Field::Ptr> m_localProps;
    std::vector<Field const*> m_inheritedProps;

   public:
};
struct PrimitiveField final : public Field {
   private:
    int m_columnIndex;
    DateTime::Info m_datetimeMetadata;

   private:
    bool _IsNull() const override;
    void const* _GetBlob(int* blobSize) const override;
    bool _GetBoolean() const override;
    uint64_t _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const override;
    double _GetDateTimeJulianDays(DateTime::Info& metadata) const override;
    double _GetDouble() const override;
    int _GetInt() const override;
    int64_t _GetInt64() const override;
    Utf8CP _GetText() const override;
    DPoint2d _GetPoint2d() const override;
    DPoint3d _GetPoint3d() const override;
    IGeometryPtr _GetGeometry() const override;
    IECSqlValue const& _GetStructMemberValue(Utf8CP memberName) const override;
    IECSqlValueIterable const& _GetStructIterable() const override;
    int _GetArrayLength() const override;
    IECSqlValueIterable const& _GetArrayIterable() const override;
    void UpdateDateTimeMetaData();

   public:
    PrimitiveField(Changes::Change const& change, Stage const& stage,ECSqlColumnInfo const& columnInfo, int columnIndex);
    ~PrimitiveField() {}
};
struct PointField final : public Field {
   private:
    int m_xColumnIndex;
    int m_yColumnIndex;
    int m_zColumnIndex;

   private:
    bool _IsNull() const override;
    void const* _GetBlob(int* blobSize) const override;
    bool _GetBoolean() const override;
    uint64_t _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const override;
    double _GetDateTimeJulianDays(DateTime::Info& metadata) const override;
    double _GetDouble() const override;
    int _GetInt() const override;
    int64_t _GetInt64() const override;
    Utf8CP _GetText() const override;
    DPoint2d _GetPoint2d() const override;
    DPoint3d _GetPoint3d() const override;
    IGeometryPtr _GetGeometry() const override;
    IECSqlValue const& _GetStructMemberValue(Utf8CP memberName) const override;
    IECSqlValueIterable const& _GetStructIterable() const override;
    int _GetArrayLength() const override;
    IECSqlValueIterable const& _GetArrayIterable() const override;
    bool IsPoint3d() const;

   public:
    PointField(Changes::Change const& change, Stage const& stage, ECSqlColumnInfo const& colInfo, int xColumnIndex, int yColumnIndex, int zColumnIndex);
    PointField(Changes::Change const& change, Stage const& stage, ECSqlColumnInfo const& colInfo, int xColumnIndex, int yColumnIndex);
    ~PointField() {}
};
struct StructField final : public Field, IECSqlValueIterable {
   private:
    struct IteratorState final : IECSqlValueIterable::IIteratorState {
       private:
        mutable std::map<Utf8CP, std::unique_ptr<Field>, CompareIUtf8Ascii>::const_iterator m_it;
        std::map<Utf8CP, std::unique_ptr<Field>, CompareIUtf8Ascii>::const_iterator m_endIt;
        IteratorState(IteratorState const& rhs);
        std::unique_ptr<IIteratorState> _Copy() const override;
        void _MoveToNext(bool onInitializingIterator) const override;
        bool _IsAtEnd() const override;
        IECSqlValue const& _GetCurrent() const override;

       public:
        explicit IteratorState(std::map<Utf8CP, std::unique_ptr<Field>, CompareIUtf8Ascii> const& memberFields);
    };

    std::map<Utf8CP, std::unique_ptr<Field>, CompareIUtf8Ascii> m_structMemberFields;

   private:
    bool _IsNull() const override;
    IECSqlValue const& _GetStructMemberValue(Utf8CP memberName) const override;
    IECSqlValueIterable const& _GetStructIterable() const override;
    const_iterator _CreateIterator() const override;
    void const* _GetBlob(int* blobSize) const override;
    bool _GetBoolean() const override;
    uint64_t _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const override;
    double _GetDateTimeJulianDays(DateTime::Info& metadata) const override;
    double _GetDouble() const override;
    int _GetInt() const override;
    int64_t _GetInt64() const override;
    Utf8CP _GetText() const override;
    DPoint2d _GetPoint2d() const override;
    DPoint3d _GetPoint3d() const override;
    IGeometryPtr _GetGeometry() const override;
    int _GetArrayLength() const override;
    IECSqlValueIterable const& _GetArrayIterable() const override;
    ECSqlStatus _OnAfterReset() override;
    ECSqlStatus _OnAfterStep() override;

   public:
    StructField(Changes::Change const& change, Stage const& stage, ECSqlColumnInfo const& colInfo);
    void AppendField(std::unique_ptr<Field> field);
};
struct ArrayField final : public Field {
   public:
    struct JsonECSqlValue final : public IECSqlValue, IECSqlValueIterable {
       private:
        struct ArrayIteratorState final : IECSqlValueIterable::IIteratorState {
           private:
            JsonECSqlValue const& m_value;
            mutable rapidjson::Value::ConstValueIterator m_jsonIterator;
            mutable int m_jsonIteratorIndex = -1;

           private:
            ArrayIteratorState(ArrayIteratorState const& rhs);
            std::unique_ptr<IIteratorState> _Copy() const override;
            void _MoveToNext(bool onInitializingIterator) const override;
            bool _IsAtEnd() const override;
            IECSqlValue const& _GetCurrent() const override;
            rapidjson::Value const& GetJson() const;

           public:
            explicit ArrayIteratorState(JsonECSqlValue const& val);
        };

        struct StructIteratorState final : IECSqlValueIterable::IIteratorState {
           private:
            JsonECSqlValue const& m_value;
            mutable ECN::ECPropertyIterable::const_iterator m_memberPropIterator;
            ECN::ECPropertyIterable::const_iterator m_memberPropEndIterator;

           private:
            StructIteratorState(StructIteratorState const& rhs);
            std::unique_ptr<IIteratorState> _Copy() const override;
            void _MoveToNext(bool onInitializingIterator) const override;
            bool _IsAtEnd() const override;
            IECSqlValue const& _GetCurrent() const override;
            rapidjson::Value const& GetJson() const;

           public:
            StructIteratorState(JsonECSqlValue const& val, ECN::ECPropertyIterableCR structMemberPropertyIterable);
        };

        static rapidjson::Value const* s_nullJson;
        ECDbCR m_ecdb;
        rapidjson::Value const& m_json;
        ECSqlColumnInfo m_columnInfo;
        mutable ByteStream m_blobCache;

       private:
        mutable std::vector<std::unique_ptr<JsonECSqlValue>> m_arrayElementCache;
        mutable std::map<Utf8CP, std::unique_ptr<JsonECSqlValue>, CompareIUtf8Ascii> m_structMemberCache;
        ECSqlColumnInfoCR _GetColumnInfo() const override;
        bool _IsNull() const override;
        void const* _GetBlob(int* blobSize) const override;
        bool _GetBoolean() const override;
        uint64_t _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const override;
        double _GetDateTimeJulianDays(DateTime::Info& metadata) const override;
        double _GetDouble() const override;
        int _GetInt() const override;
        int64_t _GetInt64() const override;
        Utf8CP _GetText() const override;
        DPoint2d _GetPoint2d() const override;
        DPoint3d _GetPoint3d() const override;
        IGeometryPtr _GetGeometry() const override;
        IECSqlValue const& _GetStructMemberValue(Utf8CP memberName) const override;
        IECSqlValueIterable const& _GetStructIterable() const override;
        int _GetArrayLength() const override;
        IECSqlValueIterable const& _GetArrayIterable() const override;
        const_iterator _CreateIterator() const override;
        IECSqlValue const& CreateStructMemberValue(ECN::ECPropertyCR memberProp) const;
        bool CanCallGetFor(ECN::PrimitiveType requestedType) const;
        static Utf8CP GetPrimitiveGetMethodName(ECN::PrimitiveType getMethodType);
        static rapidjson::Value const& GetNullJson();

       public:
        JsonECSqlValue(ECDbCR ecdb, rapidjson::Value const& json, ECSqlColumnInfo const& columnInfo);
        ~JsonECSqlValue() {}
    };

   private:
    int m_sqliteColumnIndex = -1;
    mutable rapidjson::Document m_json = rapidjson::Document(rapidjson::kArrayType);
    mutable std::unique_ptr<JsonECSqlValue> m_value = nullptr;

   private:
    bool _IsNull() const override;
    void const* _GetBlob(int* blobSize) const override;
    bool _GetBoolean() const override;
    uint64_t _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const override;
    double _GetDateTimeJulianDays(DateTime::Info& metadata) const override;
    double _GetDouble() const override;
    int _GetInt() const override;
    int64_t _GetInt64() const override;
    Utf8CP _GetText() const override;
    DPoint2d _GetPoint2d() const override;
    DPoint3d _GetPoint3d() const override;
    IGeometryPtr _GetGeometry() const override;
    IECSqlValue const& _GetStructMemberValue(Utf8CP memberName) const override;
    IECSqlValueIterable const& _GetStructIterable() const override;
    int _GetArrayLength() const override;
    IECSqlValueIterable const& _GetArrayIterable() const override;
    ECSqlStatus _OnAfterReset() override;
    ECSqlStatus _OnAfterStep() override;
    void DoReset() const;
    JsonECSqlValue const& GetValue() const;

   public:
    ArrayField(Changes::Change const& change, Stage const& stage, ECSqlColumnInfo const& colInfo, int sqliteColumnIndex);
    ~ArrayField() {}
};
struct NavigationField final : public Field, IECSqlValueIterable {
   private:
    struct IteratorState final : IECSqlValueIterable::IIteratorState {
       private:
        enum class State : uint8_t {
            New = 0,
            Id = 1,
            RelECClassId = 2,
            End = 3
        };
        NavigationField const& m_field;
        mutable State m_state = State::New;

       private:
        IteratorState(IteratorState const& rhs);
        std::unique_ptr<IIteratorState> _Copy() const override;
        void _MoveToNext(bool onInitializingIterator) const override;
        bool _IsAtEnd() const override;
        IECSqlValue const& _GetCurrent() const override;

       public:
        explicit IteratorState(NavigationField const& field);
    };

    std::unique_ptr<Field> m_idField;
    std::unique_ptr<Field> m_relClassIdField;

   private:
    bool _IsNull() const override;
    IECSqlValue const& _GetStructMemberValue(Utf8CP memberName) const override;
    IECSqlValueIterable const& _GetStructIterable() const override;
    const_iterator _CreateIterator() const override;
    void const* _GetBlob(int* blobSize) const override;
    bool _GetBoolean() const override;
    uint64_t _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const override;
    double _GetDateTimeJulianDays(DateTime::Info& metadata) const override;
    double _GetDouble() const override;
    int _GetInt() const override;
    int64_t _GetInt64() const override;
    Utf8CP _GetText() const override;
    DPoint2d _GetPoint2d() const override;
    DPoint3d _GetPoint3d() const override;
    IGeometryPtr _GetGeometry() const override;
    int _GetArrayLength() const override;
    IECSqlValueIterable const& _GetArrayIterable() const override;
    ECSqlStatus _OnAfterReset() override;
    ECSqlStatus _OnAfterStep() override;

   public:
    NavigationField(Changes::Change const& change, Stage const& stage, ECSqlColumnInfo const& colInfo);
    void SetMembers(std::unique_ptr<Field> idField, std::unique_ptr<Field> relClassIdField);
};
// Replaces a virtual ClassId/SourceClassId/TargetClassId with a constant ClassId field.
struct ClassIdField final : public Field {
   private:
    ECN::ECClassId m_classId;
    mutable Utf8String m_idStr;

   private:
    bool _IsNull() const override { return !m_classId.IsValid(); }
    void const* _GetBlob(int* blobSize) const override { return NoopECSqlValue::GetSingleton().GetBlob(blobSize); }
    bool _GetBoolean() const override { return NoopECSqlValue::GetSingleton().GetBoolean(); }
    uint64_t _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const override { return NoopECSqlValue::GetSingleton().GetDateTimeJulianDaysMsec(metadata); }
    double _GetDateTimeJulianDays(DateTime::Info& metadata) const override { return NoopECSqlValue::GetSingleton().GetDateTimeJulianDays(metadata); }
    double _GetDouble() const override { return NoopECSqlValue::GetSingleton().GetDouble(); }
    int _GetInt() const override { return NoopECSqlValue::GetSingleton().GetInt(); }
    IGeometryPtr _GetGeometry() const override { return NoopECSqlValue::GetSingleton().GetGeometry(); }
    DPoint2d _GetPoint2d() const override { return NoopECSqlValue::GetSingleton().GetPoint2d(); }
    DPoint3d _GetPoint3d() const override { return NoopECSqlValue::GetSingleton().GetPoint3d(); }
    IECSqlValue const& _GetStructMemberValue(Utf8CP structMemberName) const override { return NoopECSqlValue::GetSingleton(); }
    IECSqlValueIterable const& _GetStructIterable() const override { return NoopECSqlValue::GetSingleton().GetStructIterable(); }
    int _GetArrayLength() const override { return NoopECSqlValue::GetSingleton().GetArrayLength(); }
    IECSqlValueIterable const& _GetArrayIterable() const override { return NoopECSqlValue::GetSingleton().GetArrayIterable(); }
    Utf8CP _GetText() const override {
        if (m_idStr.empty())
            m_idStr = m_classId.ToHexStr();
        return m_idStr.c_str();
    }
    int64_t _GetInt64() const override {
        return static_cast<int64_t>(m_classId.GetValueUnchecked());
    }

   public:
    ClassIdField(ECSqlColumnInfo const& columnInfo, ECN::ECClassId classId)
        : Field(Changes::Change(nullptr, false), Stage::New, columnInfo, false, false), m_classId(classId) {}
};

ECSqlColumnInfoCR Field::_GetColumnInfo() const { return m_columnInfo; }
ECSqlStatus Field::_OnAfterReset() { return ECSqlStatus::Success; }
ECSqlStatus Field::_OnAfterStep() { return ECSqlStatus::Success; }
Field::Field(Changes::Change const& change, Stage const& stage, ECSqlColumnInfo const& columnInfo, bool needsOnAfterStep, bool needsOnAfterReset)
    : m_columnInfo(columnInfo), m_requiresOnAfterStep(needsOnAfterStep), m_requiresOnAfterReset(needsOnAfterReset), m_change(change), m_stage(stage) {}
bool Field::RequiresOnAfterStep() const { return m_requiresOnAfterStep; }
ECSqlStatus Field::OnAfterStep() { return _OnAfterStep(); }
bool Field::RequiresOnAfterReset() const { return m_requiresOnAfterReset || _GetColumnInfo().IsDynamic(); }
ECSqlStatus Field::OnAfterReset() { return _OnAfterReset(); }
DbValue Field::GetSqliteValue(int colNum) const {
    BeAssert(colNum >= 0 && colNum < m_change.GetColumnCount() && "Column index is out of bounds. Please double-check the column index provided to the Field constructor.");
    return m_change.GetValue(colNum, m_stage);
}

const ClassMap* Factory::GetRootClassMap(DbTable const& tbl, ECDbCR conn) {
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
}

bool PrimitiveField::_IsNull() const { return GetSqliteValue(m_columnIndex).IsNull(); }
bool PrimitiveField::_GetBoolean() const { return GetSqliteValue(m_columnIndex).GetValueInt() != 0; }
double PrimitiveField::_GetDouble() const { return GetSqliteValue(m_columnIndex).GetValueDouble(); }
int PrimitiveField::_GetInt() const { return GetSqliteValue(m_columnIndex).GetValueInt(); }
int64_t PrimitiveField::_GetInt64() const { return GetSqliteValue(m_columnIndex).GetValueInt64(); }
Utf8CP PrimitiveField::_GetText() const { return GetSqliteValue(m_columnIndex).GetValueText(); }

void PrimitiveField::UpdateDateTimeMetaData() {
    auto& columnInfo = GetColumnInfo();
    if (columnInfo.GetDataType().GetPrimitiveType() == PRIMITIVETYPE_DateTime) {
        ECPropertyCP property = columnInfo.GetProperty();
        BeAssert(property != nullptr && "ColumnInfo::GetProperty can return null. Please double-check");
        if (CoreCustomAttributeHelper::GetDateTimeInfo(m_datetimeMetadata, *property) != ECObjectsStatus::Success) {
            LOG.error("Could not read DateTimeInfo custom attribute from the corresponding ECProperty.");
            BeAssert(false && "Could not read DateTimeInfo custom attribute from the corresponding ECProperty.");
            return;
        }

        if (!m_datetimeMetadata.IsValid())
            m_datetimeMetadata = DateTime::Info::CreateForDateTime(DateTime::Kind::Unspecified);  // default
    }
}

PrimitiveField::PrimitiveField(Changes::Change const& change, Stage const& stage, ECSqlColumnInfo const& columnInfo, int columnIndex)
    : Field(change, stage, columnInfo, false, false), m_columnIndex(columnIndex) {
    UpdateDateTimeMetaData();
}

bool PointField::IsPoint3d() const { return m_zColumnIndex >= 0; }

PointField::PointField(Changes::Change const& change, Stage const& stage, ECSqlColumnInfo const& colInfo, int xColumnIndex, int yColumnIndex, int zColumnIndex)
    : Field(change, stage, colInfo, false, false), m_xColumnIndex(xColumnIndex), m_yColumnIndex(yColumnIndex), m_zColumnIndex(zColumnIndex) {}

PointField::PointField(Changes::Change const& change, Stage const& stage, ECSqlColumnInfo const& colInfo, int xColumnIndex, int yColumnIndex)
    : PointField(change, stage, colInfo, xColumnIndex, yColumnIndex, -1) {}

StructField::IteratorState::IteratorState(IteratorState const& rhs) : m_it(rhs.m_it), m_endIt(rhs.m_endIt) {}

std::unique_ptr<IECSqlValueIterable::IIteratorState> StructField::IteratorState::_Copy() const {
    return std::unique_ptr<IIteratorState>(new IteratorState(*this));
}

void StructField::IteratorState::_MoveToNext(bool onInitializingIterator) const {
    if (!onInitializingIterator)
        ++m_it;
}

bool StructField::IteratorState::_IsAtEnd() const { return m_it == m_endIt; }

IECSqlValue const& StructField::IteratorState::_GetCurrent() const { return *m_it->second; }

StructField::IteratorState::IteratorState(std::map<Utf8CP, std::unique_ptr<Field>, CompareIUtf8Ascii> const& memberFields)
    : IIteratorState(), m_it(memberFields.begin()), m_endIt(memberFields.end()) {}

IECSqlValueIterable const& StructField::_GetStructIterable() const { return *this; }

IECSqlValueIterable::const_iterator StructField::_CreateIterator() const {
    return IECSqlValueIterable::const_iterator(std::make_unique<IteratorState>(m_structMemberFields));
}

StructField::StructField(Changes::Change const& change, Stage const& stage, ECSqlColumnInfo const& colInfo) : Field(change, stage, colInfo, false, false) {}

ArrayField::JsonECSqlValue::ArrayIteratorState::ArrayIteratorState(ArrayIteratorState const& rhs)
    : m_value(rhs.m_value), m_jsonIterator(rhs.m_jsonIterator), m_jsonIteratorIndex(rhs.m_jsonIteratorIndex) {}

std::unique_ptr<IECSqlValueIterable::IIteratorState> ArrayField::JsonECSqlValue::ArrayIteratorState::_Copy() const {
    return std::unique_ptr<IIteratorState>(new ArrayIteratorState(*this));
}

bool ArrayField::JsonECSqlValue::ArrayIteratorState::_IsAtEnd() const {
    return GetJson().IsNull() || m_jsonIterator == GetJson().End();
}

rapidjson::Value const& ArrayField::JsonECSqlValue::ArrayIteratorState::GetJson() const { return m_value.m_json; }

ArrayField::JsonECSqlValue::ArrayIteratorState::ArrayIteratorState(JsonECSqlValue const& val)
    : IIteratorState(), m_value(val) {}

ArrayField::JsonECSqlValue::StructIteratorState::StructIteratorState(StructIteratorState const& rhs)
    : m_value(rhs.m_value), m_memberPropIterator(rhs.m_memberPropIterator), m_memberPropEndIterator(rhs.m_memberPropEndIterator) {}

std::unique_ptr<IECSqlValueIterable::IIteratorState> ArrayField::JsonECSqlValue::StructIteratorState::_Copy() const {
    return std::unique_ptr<IIteratorState>(new StructIteratorState(*this));
}

bool ArrayField::JsonECSqlValue::StructIteratorState::_IsAtEnd() const {
    return m_memberPropIterator == m_memberPropEndIterator;
}

rapidjson::Value const& ArrayField::JsonECSqlValue::StructIteratorState::GetJson() const { return m_value.m_json; }

ECSqlColumnInfoCR ArrayField::JsonECSqlValue::_GetColumnInfo() const { return m_columnInfo; }

bool ArrayField::_IsNull() const { return GetSqliteValue(m_sqliteColumnIndex).IsNull(); }
void const* ArrayField::_GetBlob(int* blobSize) const { return GetValue().GetBlob(blobSize); }
bool ArrayField::_GetBoolean() const { return GetValue().GetBoolean(); }
uint64_t ArrayField::_GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const { return GetValue().GetDateTimeJulianDaysMsec(metadata); }
double ArrayField::_GetDateTimeJulianDays(DateTime::Info& metadata) const { return GetValue().GetDateTimeJulianDays(metadata); }
double ArrayField::_GetDouble() const { return GetValue().GetDouble(); }
int ArrayField::_GetInt() const { return GetValue().GetInt(); }
int64_t ArrayField::_GetInt64() const { return GetValue().GetInt64(); }
Utf8CP ArrayField::_GetText() const { return GetValue().GetText(); }
DPoint2d ArrayField::_GetPoint2d() const { return GetValue().GetPoint2d(); }
DPoint3d ArrayField::_GetPoint3d() const { return GetValue().GetPoint3d(); }
IGeometryPtr ArrayField::_GetGeometry() const { return GetValue().GetGeometry(); }
IECSqlValue const& ArrayField::_GetStructMemberValue(Utf8CP memberName) const { return GetValue()[memberName]; }
IECSqlValueIterable const& ArrayField::_GetStructIterable() const { return GetValue().GetStructIterable(); }
int ArrayField::_GetArrayLength() const { return GetValue().GetArrayLength(); }
IECSqlValueIterable const& ArrayField::_GetArrayIterable() const { return GetValue().GetArrayIterable(); }

ArrayField::JsonECSqlValue const& ArrayField::GetValue() const {
    BeAssert(m_value != nullptr);
    return *m_value;
}

ArrayField::ArrayField(Changes::Change const& change, Stage const& stage, ECSqlColumnInfo const& colInfo, int sqliteColumnIndex)
    : Field(change, stage, colInfo, true, true), m_sqliteColumnIndex(sqliteColumnIndex) {}

NavigationField::IteratorState::IteratorState(IteratorState const& rhs) : m_field(rhs.m_field), m_state(rhs.m_state) {}

std::unique_ptr<IECSqlValueIterable::IIteratorState> NavigationField::IteratorState::_Copy() const {
    return std::unique_ptr<IIteratorState>(new IteratorState(*this));
}

bool NavigationField::IteratorState::_IsAtEnd() const { return m_state == State::End; }

NavigationField::IteratorState::IteratorState(NavigationField const& field) : m_field(field) {}

bool NavigationField::_IsNull() const {
    BeAssert(m_idField != nullptr);
    return m_idField->IsNull();
}

IECSqlValueIterable const& NavigationField::_GetStructIterable() const {
    return *this;
}

IECSqlValueIterable::const_iterator NavigationField::_CreateIterator() const {
    return const_iterator(std::make_unique<IteratorState>(*this));
}

NavigationField::NavigationField(Changes::Change const& change, Stage const& stage, ECSqlColumnInfo const& colInfo) : Field(change, stage, colInfo, false, false) {}

NoopECSqlValue::NoopECSqlValue() : IECSqlValue() {}
NoopECSqlValue::~NoopECSqlValue() {}
ECSqlColumnInfoCR NoopECSqlValue::_GetColumnInfo() const { return m_dummyColumnInfo; }
bool NoopECSqlValue::_IsNull() const { return true; }
bool NoopECSqlValue::_GetBoolean() const { return false; }
uint64_t NoopECSqlValue::_GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const { return INT64_C(0); }
double NoopECSqlValue::_GetDateTimeJulianDays(DateTime::Info& metadata) const { return 0.0; }
double NoopECSqlValue::_GetDouble() const { return 0.0; }
int NoopECSqlValue::_GetInt() const { return 0; }
int64_t NoopECSqlValue::_GetInt64() const { return INT64_C(0); }
Utf8CP NoopECSqlValue::_GetText() const { return nullptr; }
DPoint2d NoopECSqlValue::_GetPoint2d() const { return DPoint2d::From(0.0, 0.0); }
DPoint3d NoopECSqlValue::_GetPoint3d() const { return DPoint3d::From(0.0, 0.0, 0.0); }
IGeometryPtr NoopECSqlValue::_GetGeometry() const { return nullptr; }
IECSqlValue const& NoopECSqlValue::_GetStructMemberValue(Utf8CP memberName) const { return *this; }
IECSqlValueIterable const& NoopECSqlValue::_GetStructIterable() const { return *this; }
int NoopECSqlValue::_GetArrayLength() const { return -1; }
IECSqlValueIterable const& NoopECSqlValue::_GetArrayIterable() const { return *this; }
IECSqlValueIterable::const_iterator NoopECSqlValue::_CreateIterator() const { return end(); }

NoopECSqlValue const* NoopECSqlValue::s_singleton = nullptr;

NoopECSqlValue const& NoopECSqlValue::GetSingleton() {
    if (s_singleton == nullptr)
        s_singleton = new NoopECSqlValue();

    return *s_singleton;
}

void const* NoopECSqlValue::_GetBlob(int* blobSize) const {
    if (blobSize != nullptr)
        *blobSize = -1;

    return nullptr;
}

void const* PrimitiveField::_GetBlob(int* blobSize) const {
    auto val = GetSqliteValue(m_columnIndex);
    if (blobSize != nullptr)
        *blobSize = val.GetValueBytes();

    return val.GetValueBlob();
}

uint64_t PrimitiveField::_GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const {
    const double jd = _GetDateTimeJulianDays(metadata);
    return DateTime::RationalDayToMsec(jd);
}


DPoint2d PrimitiveField::_GetPoint2d() const {
    LOG.error("GetPoint2d cannot be called for columns which are not of the Point2d type.");
    return NoopECSqlValue::GetSingleton().GetPoint2d();
}

DPoint3d PrimitiveField::_GetPoint3d() const {
    LOG.error("GetPoint3d cannot be called for columns which are not of the Point3d type.");
    return NoopECSqlValue::GetSingleton().GetPoint3d();
}

IECSqlValue const& PrimitiveField::_GetStructMemberValue(Utf8CP memberName) const {
    LOG.error("GetStructMemberValue cannot be called for primitive columns.");
    return NoopECSqlValue::GetSingleton()[memberName];
}

IECSqlValueIterable const& PrimitiveField::_GetStructIterable() const {
    LOG.error("GetStructIterable cannot be called for primitive columns.");
    return NoopECSqlValue::GetSingleton().GetStructIterable();
}

int PrimitiveField::_GetArrayLength() const {
    LOG.error("GetArrayLength cannot be called for primitive columns.");
    return NoopECSqlValue::GetSingleton().GetArrayLength();
}

IECSqlValueIterable const& PrimitiveField::_GetArrayIterable() const {
    LOG.error("GetArrayIterable cannot be called for primitive columns.");
    return NoopECSqlValue::GetSingleton().GetArrayIterable();
}

bool PointField::_IsNull() const {
    const auto xVal = GetSqliteValue(m_xColumnIndex);
    const auto yVal = GetSqliteValue(m_yColumnIndex);
    const auto zVal = IsPoint3d() ? GetSqliteValue(m_zColumnIndex) : DbValue(nullptr);

    const auto coordXValue = xVal.GetValueDouble();
    const auto coordYValue = yVal.GetValueDouble();
    const auto coordZValue = IsPoint3d() ? zVal.GetValueDouble() : 0.0;
    return (xVal.IsNull() || std::isinf(coordXValue) || std::isnan(coordXValue) || yVal.IsNull() || std::isinf(coordYValue) || std::isnan(coordYValue) || (IsPoint3d() && (zVal.IsNull() || std::isinf(coordZValue) || std::isnan(coordZValue))));
}

DPoint2d PointField::_GetPoint2d() const {
    if (IsPoint3d()) {
        LOG.error("GetValuePoint2d cannot be called for Point3d column. Call GetValuePoint3d instead.");
        return NoopECSqlValue::GetSingleton().GetPoint2d();
    }

    return DPoint2d::From(GetSqliteValue(m_xColumnIndex).GetValueDouble(),
                          GetSqliteValue(m_yColumnIndex).GetValueDouble());
}

DPoint3d PointField::_GetPoint3d() const {
    if (!IsPoint3d()) {
        LOG.error("GetValuePoint3d cannot be called for Point2d column. Call GetPoint2d instead.");
        return NoopECSqlValue::GetSingleton().GetPoint3d();
    }

    return DPoint3d::From(GetSqliteValue(m_xColumnIndex).GetValueDouble(),
                          GetSqliteValue(m_yColumnIndex).GetValueDouble(),
                          GetSqliteValue(m_zColumnIndex).GetValueDouble());
}


void const* PointField::_GetBlob(int* blobSize) const {
    LOG.error("GetBlob cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetBlob(blobSize);
}

bool PointField::_GetBoolean() const {
    LOG.error("GetBoolean cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetBoolean();
}

uint64_t PointField::_GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const {
    LOG.error("GetDateTime cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetDateTimeJulianDaysMsec(metadata);
}

double PointField::_GetDateTimeJulianDays(DateTime::Info& metadata) const {
    LOG.error("GetDateTime cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetDateTimeJulianDays(metadata);
}

double PointField::_GetDouble() const {
    LOG.error("GetDouble cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetDouble();
}

int PointField::_GetInt() const {
    LOG.error("GetInt cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetInt();
}

int64_t PointField::_GetInt64() const {
    LOG.error("GetInt64 cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetInt64();
}

Utf8CP PointField::_GetText() const {
    LOG.error("GetText cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetText();
}

IGeometryPtr PointField::_GetGeometry() const {
    LOG.error("GetGeometry cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetGeometry();
}

IECSqlValue const& PointField::_GetStructMemberValue(Utf8CP memberName) const {
    LOG.error("GetStructMemberValue cannot be called for Point2d or Point3d columns.");
    return NoopECSqlValue::GetSingleton()[memberName];
}

IECSqlValueIterable const& PointField::_GetStructIterable() const {
    LOG.error("GetStructIterable cannot be called for Point2d or Point3d columns.");
    return NoopECSqlValue::GetSingleton().GetStructIterable();
}

int PointField::_GetArrayLength() const {
    LOG.error("GetArrayLength cannot be called for Point2d or Point3d columns.");
    return NoopECSqlValue::GetSingleton().GetArrayLength();
}

IECSqlValueIterable const& PointField::_GetArrayIterable() const {
    LOG.error("GetArrayIterable cannot be called for Point2d or Point3d columns.");
    return NoopECSqlValue::GetSingleton().GetArrayIterable();
}

bool StructField::_IsNull() const {
    for (auto const& field : m_structMemberFields) {
        if (!field.second->IsNull())
            return false;
    }

    return true;
}

IECSqlValue const& StructField::_GetStructMemberValue(Utf8CP memberName) const {
    auto it = m_structMemberFields.find(memberName);
    if (it == m_structMemberFields.end()) {
        LOG.errorv("Struct member '%s' passed to struct IECSqlValue[Utf8CP] does not exist.", memberName);
        return NoopECSqlValue::GetSingleton()[memberName];
    }

    return *it->second;
}

void StructField::AppendField(std::unique_ptr<Field> field) {
    if (field == nullptr) {
        BeAssert(false);
        return;
    }

    if (field->RequiresOnAfterStep())
        m_requiresOnAfterStep = true;

    if (field->RequiresOnAfterReset())
        m_requiresOnAfterReset = true;

    Utf8CP memberName = field->GetColumnInfo().GetProperty()->GetName().c_str();
    BeAssert(m_structMemberFields.find(memberName) == m_structMemberFields.end());
    m_structMemberFields[memberName] = std::move(field);
}

void const* StructField::_GetBlob(int* blobSize) const {
    LOG.error("GetBlob cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetBlob(blobSize);
}

bool StructField::_GetBoolean() const {
    LOG.error("GetBoolean cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetBoolean();
}

uint64_t StructField::_GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const {
    LOG.error("GetDateTime cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetDateTimeJulianDaysMsec(metadata);
}

double StructField::_GetDateTimeJulianDays(DateTime::Info& metadata) const {
    LOG.error("GetDateTime cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetDateTimeJulianDays(metadata);
}

double StructField::_GetDouble() const {
    LOG.error("GetDouble cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetDouble();
}

int StructField::_GetInt() const {
    LOG.error("GetInt cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetInt();
}

int64_t StructField::_GetInt64() const {
    LOG.error("GetInt64 cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetInt64();
}

Utf8CP StructField::_GetText() const {
    LOG.error("GetText cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetText();
}

IGeometryPtr StructField::_GetGeometry() const {
    LOG.error("GetGeometry cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetGeometry();
}

DPoint2d StructField::_GetPoint2d() const {
    LOG.error("GetPoint2d cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetPoint2d();
}

DPoint3d StructField::_GetPoint3d() const {
    LOG.error("GetPoint3d cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetPoint3d();
}

int StructField::_GetArrayLength() const {
    LOG.error("GetArrayLength cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetArrayLength();
}

IECSqlValueIterable const& StructField::_GetArrayIterable() const {
    LOG.error("GetArrayIterable cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetArrayIterable();
}

ECSqlStatus StructField::_OnAfterStep() {
    for (auto const& memberField : m_structMemberFields) {
        ECSqlStatus stat = memberField.second->OnAfterStep();
        if (!stat.IsSuccess())
            return stat;
    }

    return ECSqlStatus::Success;
}

ECSqlStatus StructField::_OnAfterReset() {
    for (auto const& memberField : m_structMemberFields) {
        ECSqlStatus stat = memberField.second->OnAfterReset();
        if (!stat.IsSuccess())
            return stat;
    }

    return ECSqlStatus::Success;
}

void NavigationField::SetMembers(std::unique_ptr<Field> idField, std::unique_ptr<Field> relClassIdField) {
    BeAssert(idField != nullptr && !idField->RequiresOnAfterReset() && !idField->RequiresOnAfterStep());
    BeAssert(relClassIdField != nullptr && !relClassIdField->RequiresOnAfterReset() && !relClassIdField->RequiresOnAfterStep());
    m_idField = std::move(idField);
    m_relClassIdField = std::move(relClassIdField);
    BeAssert(m_idField != nullptr && m_relClassIdField != nullptr);
}

IECSqlValue const& NavigationField::_GetStructMemberValue(Utf8CP memberName) const {
    if (BeStringUtilities::StricmpAscii(memberName, "Id") == 0)
        return *m_idField;

    if (BeStringUtilities::StricmpAscii(memberName, "RelECClassId") == 0)
        return *m_relClassIdField;

    LOG.errorv("Member name '%s' passed to navigation property IECSqlValue[Utf8CP] does not exist.", memberName);
    return NoopECSqlValue::GetSingleton()[memberName];
}

ECSqlStatus NavigationField::_OnAfterStep() {
    ECSqlStatus stat = m_idField->OnAfterStep();
    if (!stat.IsSuccess())
        return stat;

    return m_relClassIdField->OnAfterStep();
}

ECSqlStatus NavigationField::_OnAfterReset() {
    ECSqlStatus stat = m_idField->OnAfterReset();
    if (!stat.IsSuccess())
        return stat;

    return m_relClassIdField->OnAfterReset();
}

void const* NavigationField::_GetBlob(int* blobSize) const {
    LOG.error("GetBlob cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetBlob(blobSize);
}

bool NavigationField::_GetBoolean() const {
    LOG.error("GetBoolean cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetBoolean();
}

uint64_t NavigationField::_GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const {
    LOG.error("GetDateTime cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetDateTimeJulianDaysMsec(metadata);
}

double NavigationField::_GetDateTimeJulianDays(DateTime::Info& metadata) const {
    LOG.error("GetDateTime cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetDateTimeJulianDays(metadata);
}

double NavigationField::_GetDouble() const {
    LOG.error("GetDouble cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetDouble();
}

int NavigationField::_GetInt() const {
    LOG.error("GetInt cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetInt();
}

int64_t NavigationField::_GetInt64() const {
    LOG.error("GetInt64 cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetInt64();
}

Utf8CP NavigationField::_GetText() const {
    LOG.error("GetText cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetText();
}

IGeometryPtr NavigationField::_GetGeometry() const {
    LOG.error("GetGeometry cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetGeometry();
}

DPoint2d NavigationField::_GetPoint2d() const {
    LOG.error("GetPoint2d cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetPoint2d();
}

DPoint3d NavigationField::_GetPoint3d() const {
    LOG.error("GetPoint3d cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetPoint3d();
}

int NavigationField::_GetArrayLength() const {
    LOG.error("GetArrayLength cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetArrayLength();
}

IECSqlValueIterable const& NavigationField::_GetArrayIterable() const {
    LOG.error("GetArrayIterable cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetArrayIterable();
}


void NavigationField::IteratorState::_MoveToNext(bool onInitializingIterator) const {
    uint8_t current = (uint8_t)m_state;
    current++;
    m_state = (State)current;
}

IECSqlValue const& NavigationField::IteratorState::_GetCurrent() const {
    switch (m_state) {
        case State::Id:
            return *m_field.m_idField;

        case State::RelECClassId:
            return *m_field.m_relClassIdField;

        default:
            BeAssert(false);
            return NoopECSqlValue::GetSingleton();
    }
}

std::vector<Field::Ptr> Factory::Create(ECDbCR conn, DbTable const& tbl, Changes::Change const& change, Stage const& stage) {
    auto classMap = GetRootClassMap(tbl, conn);
    if(classMap == nullptr) {
        return std::vector<Field::Ptr>();
    }
    std::vector<Field::Ptr> queryProps;
    for (auto& propertyMap : classMap->GetPropertyMaps()) {
        GetTablesPropertyMapVisitor visitor(PropertyMap::Type::All);
        propertyMap->AcceptVisitor(visitor);
        DbTable const* table = (*visitor.GetTables().begin());
        if (propertyMap->GetType() == PropertyMap::Type::ConstraintECClassId) {
            if (!propertyMap->IsMappedToClassMapTables()) {
                table = classMap->GetTables().front();
            }
        }
        const auto queryTable = TableView::Create(conn, *table);
        if (queryTable == nullptr) {
            return std::vector<Field::Ptr>();
        }
        queryProps.emplace_back(
            CreateField(*propertyMap, *(queryTable.get()), change, stage));
    }
    return queryProps;
}

ECSqlPropertyPath Factory::GetPropertyPath(PropertyMap const& propertyMap) {
    ECSqlPropertyPath propertyPath;
    for (auto& part : propertyMap.GetPath()) {
        propertyPath.AddEntry(part->GetProperty());
    }
    return propertyPath;
}

std::unique_ptr<Field> Factory::CreatePrimitiveField(PropertyMap const& propertyMap, TableView const& tbl, Changes::Change const& change, Stage const& stage) {
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
        ECSqlColumnInfo::RootClass(propertyMap.GetClassMap().GetClass(), ""));
    if (prim->GetType() == ECN::PRIMITIVETYPE_Point2d) {
        const auto& pt2dMap = propertyMap.GetAs<Point2dPropertyMap>();
        const auto xCol = tbl.GetColumnIndexOf(pt2dMap.GetX().GetColumn());
        const auto yCol = tbl.GetColumnIndexOf(pt2dMap.GetY().GetColumn());
        return std::make_unique<PointField>(change, stage,columnInfo, xCol, yCol);
    } else if (prim->GetType() == PRIMITIVETYPE_Point3d) {
        const auto& pt3dMap = propertyMap.GetAs<Point3dPropertyMap>();
        const auto xCol = tbl.GetColumnIndexOf(pt3dMap.GetX().GetColumn());
        const auto yCol = tbl.GetColumnIndexOf(pt3dMap.GetY().GetColumn());
        const auto zCol = tbl.GetColumnIndexOf(pt3dMap.GetZ().GetColumn());
        return std::make_unique<PointField>(change, stage, columnInfo, xCol, yCol, zCol);
    } else {
        const auto& primMap = propertyMap.GetAs<SingleColumnDataPropertyMap>();
        const auto nCol = tbl.GetColumnIndexOf(primMap.GetColumn());
        return std::make_unique<PrimitiveField>(change, stage, columnInfo, nCol);
    }
}

std::unique_ptr<Field> Factory::CreateSystemField(PropertyMap const& propertyMap, TableView const& tbl, Changes::Change const& change, Stage const& stage) {
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
        ECSqlColumnInfo::RootClass(propertyMap.GetClassMap().GetClass(), ""));

    const auto extendedType = ExtendedTypeHelper::GetExtendedType(prim->GetExtendedTypeName());
    if (extendedType == ExtendedTypeHelper::ExtendedType::ClassId && tbl.GetClassIdCol() >= 0) {
        return std::make_unique<PrimitiveField>(change, stage, columnInfo, tbl.GetClassIdCol());
    }
    if (extendedType == ExtendedTypeHelper::ExtendedType::SourceClassId && tbl.GetSourceClassIdCol() >= 0) {
        return std::make_unique<PrimitiveField>(change, stage, columnInfo, tbl.GetSourceClassIdCol());
    }
    if (extendedType == ExtendedTypeHelper::ExtendedType::TargetClassId && tbl.GetTargetClassIdCol() >= 0) {
        return std::make_unique<PrimitiveField>(change, stage, columnInfo, tbl.GetTargetClassIdCol());
    }

    const auto& sysMap = propertyMap.GetAs<SystemPropertyMap>();
    const auto dataMap = sysMap.GetDataPropertyMaps().front();
    if (dataMap->GetColumn().IsVirtual()) {
        BeAssert(false);
    }
    const auto nCol = tbl.GetColumnIndexOf(dataMap->GetColumn());
    return std::make_unique<PrimitiveField>(change, stage, columnInfo, nCol);
}

std::unique_ptr<Field> Factory::CreateStructField(PropertyMap const& propertyMap, TableView const& tbl, Changes::Change const& change, Stage const& stage) {
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
        ECSqlColumnInfo::RootClass(propertyMap.GetClassMap().GetClass(), ""));

    auto newStructField = std::make_unique<StructField>(change, stage, columnInfo);
    auto& structPropertyMap = propertyMap.GetAs<StructPropertyMap>();
    for (auto& memberMap : structPropertyMap) {
        newStructField->AppendField(CreateField(*memberMap, tbl, change, stage));
    }
    return std::move(newStructField);
}

std::unique_ptr<Field> Factory::CreateClassIdField(PropertyMap const& propertyMap, ECN::ECClassId id, TableView const& tbl) {
    ECSqlColumnInfo columnInfo(
        ECN::ECTypeDescriptor::CreatePrimitiveTypeDescriptor(PRIMITIVETYPE_Long),
        DateTime::Info(),
        nullptr,
        &propertyMap.GetProperty(),
        &propertyMap.GetProperty(),
        propertyMap.IsSystem(),
        false /* = isGenerated */,
        GetPropertyPath(propertyMap),
        ECSqlColumnInfo::RootClass(propertyMap.GetClassMap().GetClass(), ""));

    return std::make_unique<ClassIdField>(columnInfo, id);
}

std::unique_ptr<Field> Factory::CreateNavigationField(PropertyMap const& propertyMap, TableView const& tbl, Changes::Change const& change, Stage const& stage) {
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
        ECSqlColumnInfo::RootClass(propertyMap.GetClassMap().GetClass(), ""));
    const auto& navMap = propertyMap.GetAs<NavigationPropertyMap>();
    auto idField = CreatePrimitiveField(navMap.GetIdPropertyMap(), tbl, change, stage);

    std::unique_ptr<Field> relClassIdField;
    auto& relClassIdMap = navMap.GetRelECClassIdPropertyMap();
    if (relClassIdMap.GetColumn().IsVirtual()) {
        relClassIdField = CreateClassIdField(relClassIdMap, prim->GetRelationshipClass()->GetId(), tbl);
    } else {
        relClassIdField = CreatePrimitiveField(navMap.GetRelECClassIdPropertyMap(), tbl, change, stage);
    }
    auto navField = std::make_unique<NavigationField>(change, stage, columnInfo);
    navField->SetMembers(std::move(idField), std::move(relClassIdField));
    return std::move(navField);
}

DateTime::Info Factory::GetDateTimeInfo(PropertyMap const& propertyMap) {
    DateTime::Info info = DateTime::Info::CreateForDateTime(DateTime::Kind::Unspecified);
    if (propertyMap.GetType() != PropertyMap::Type::PrimitiveArray && propertyMap.GetType() != PropertyMap::Type::Primitive) {
        return info;
    }

    if (auto property = propertyMap.GetProperty().GetAsPrimitiveArrayProperty()) {
        if (property->GetType() == PRIMITIVETYPE_DateTime) {
            if (CoreCustomAttributeHelper::GetDateTimeInfo(info, *property) == ECObjectsStatus::Success) {
                return info;
            }
        }
    }
    if (auto property = propertyMap.GetProperty().GetAsPrimitiveProperty()) {
        if (property->GetType() == PRIMITIVETYPE_DateTime) {
            if (CoreCustomAttributeHelper::GetDateTimeInfo(info, *property) == ECObjectsStatus::Success) {
                return info;
            }
        }
    }

    return info;
}

std::unique_ptr<Field> Factory::CreateArrayField(PropertyMap const& propertyMap, TableView const& tbl, Changes::Change const& change, Stage const& stage) {
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
        prop.GetIsStructArray() ? &prop.GetAsStructArrayProperty()->GetStructElementType() : nullptr,
        &propertyMap.GetProperty(),
        &propertyMap.GetProperty(),
        propertyMap.IsSystem(),
        false /* = isGenerated */,
        GetPropertyPath(propertyMap),
        ECSqlColumnInfo::RootClass(propertyMap.GetClassMap().GetClass(), ""));
    auto& primMap = propertyMap.GetAs<SingleColumnDataPropertyMap>();
    auto nCol = tbl.GetColumnIndexOf(primMap.GetColumn());
    return std::make_unique<ArrayField>(change, stage, columnInfo, nCol);
}

std::unique_ptr<Field> Factory::CreateField(PropertyMap const& propertyMap, TableView const& tbl, Changes::Change const& change, Stage const& stage) {
    const auto& prop = propertyMap.GetProperty();
    if (propertyMap.IsSystem()) {
        return CreateSystemField(propertyMap, tbl, change, stage);
    } else if (prop.GetIsPrimitive()) {
        return CreatePrimitiveField(propertyMap, tbl, change, stage);
    } else if (prop.GetIsStruct()) {
        return CreateStructField(propertyMap, tbl, change, stage);
    } else if (prop.GetIsNavigation()) {
        return CreateNavigationField(propertyMap, tbl, change, stage);
    } else if (prop.GetIsArray()) {
        return CreateArrayField(propertyMap, tbl, change, stage);
    }
    return nullptr;
}

END_BENTLEY_SQLITE_EC_NAMESPACE