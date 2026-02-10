/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the repository root for full copyright notice.
*--------------------------------------------------------------------------------------------*/
#include "ECDbPch.h"

USING_NAMESPACE_BENTLEY_EC

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

enum class Stage : uint8_t {
    OLD = 0,
    NEW = 1,
};

struct ChangesetReader {
   public:
    struct Impl;

   private:
    ChangesetReader(ChangesetReader const&) = delete;
    ChangesetReader& operator=(ChangesetReader const&) = delete;
    struct {
        std::unique_ptr<Changes> m_changes;
        std::shared_ptr<ChangeStream> m_changeStream;
        Changes::Change m_change = Changes::Change(nullptr, false);
        Utf8String m_ddl;
        void Reset() {
            m_change = Changes::Change(nullptr, false);
            m_ddl.clear();
        }
    } _cursor;

    auto& Cursor() { return _cursor; }

   public:
    ChangesetReader(ECDb& ecdb) {}
    virtual ~ChangesetReader() {}
    ECSqlStatus Reset();
    int GetColumnCount() const;
    DbResult Step();
    void Finalize();

    IECSqlValue const& GetValue(Stage stage, int columnIndex) const;

    ECSqlColumnInfoCR GetColumnInfo(int columnIndex, Stage stage) const { return GetValue(stage, columnIndex).GetColumnInfo(); }
    bool IsValueNull(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).IsNull(); }
    void const* GetValueBlob(Stage stage, int columnIndex, int* blobSize = nullptr) const { return GetValue(stage, columnIndex).GetBlob(blobSize); }
    bool GetValueBoolean(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).GetBoolean(); }
    DateTime GetValueDateTime(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).GetDateTime(); }
    double GetValueDouble(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).GetDouble(); }
    IGeometryPtr GetValueGeometry(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).GetGeometry(); }
    int GetValueInt(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).GetInt(); }
    ECN::ECEnumeratorCP GetValueEnum(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).GetEnum(); }
    int64_t GetValueInt64(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).GetInt64(); }
    DPoint2d GetValuePoint2d(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).GetPoint2d(); }
    DPoint3d GetValuePoint3d(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).GetPoint3d(); }
    Utf8CP GetValueText(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).GetText(); }
    template <class TBeInt64Id>
    TBeInt64Id GetValueId(Stage stage, int columnIndex) const { return TBeInt64Id(GetValueUInt64(stage, columnIndex)); }
    BeGuid GetValueGuid(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).GetGuid(); }
    template <class TBeInt64Id>
    TBeInt64Id GetValueNavigation(Stage stage, int columnIndex, ECN::ECClassId* relationshipECClassId = nullptr) const { return GetValue(stage, columnIndex).GetNavigation<TBeInt64Id>(relationshipECClassId); }
    uint64_t GetValueUInt64(Stage stage, int columnIndex) const { return GetValue(stage, columnIndex).GetUInt64(); }
};

struct InstanceReader {

    struct Field : public IECSqlValue {
               using Ptr = std::unique_ptr<Field>;
       protected:
        // ChangesetReader &m_changesetReader;
        ECSqlColumnInfo m_columnInfo;
        bool m_requiresOnAfterStep = false;
        bool m_requiresOnAfterReset = false;
        Stage m_stage;

       private:
        ECSqlColumnInfoCR _GetColumnInfo() const override { return m_columnInfo; }
        virtual ECSqlStatus _OnAfterReset() { return ECSqlStatus::Success; }
        virtual ECSqlStatus _OnAfterStep() { return ECSqlStatus::Success; }

       protected:
        Field(ECSqlColumnInfo const& columnInfo, bool needsOnAfterStep, bool needsOnAfterReset)
            : m_columnInfo(columnInfo), m_requiresOnAfterStep(needsOnAfterStep), m_requiresOnAfterReset(needsOnAfterReset) {}
        ChangesetReader& GetChangesetReader() const;
        DbValue GetSqliteValue(int colNum) const;

       public:
        virtual ~Field() {}
        bool RequiresOnAfterStep() const { return m_requiresOnAfterStep; }
        ECSqlStatus OnAfterStep() { return _OnAfterStep(); }
        bool RequiresOnAfterReset() const { return m_requiresOnAfterReset || _GetColumnInfo().IsDynamic(); }
        ECSqlStatus OnAfterReset() { return _OnAfterReset(); }
    };
    struct Factory final {
        private:
            static const ClassMap* GetRootClassMap(DbTable const& tbl, ECDbCR conn) {
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
            static DateTime::Info GetDateTimeInfo(PropertyMap const& propertyMap);
            static ECSqlPropertyPath GetPropertyPath (PropertyMap const&);
            static std::unique_ptr<Field> CreatePrimitiveField(PropertyMap const&, DbTable const&);
            static std::unique_ptr<Field> CreateSystemField(PropertyMap const&, DbTable const&);
            static std::unique_ptr<Field> CreateStructField(PropertyMap const&, DbTable const&);
            static std::unique_ptr<Field> CreateNavigationField(PropertyMap const&, DbTable const&);
            static std::unique_ptr<Field> CreateArrayField(PropertyMap const&, DbTable const&);
            static std::unique_ptr<Field> CreateField(PropertyMap const&, DbTable const&);
            static std::unique_ptr<Field> CreateClassIdField(PropertyMap const&, ECN::ECClassId, DbTable const&);

        public:
            static std::vector<Property::Ptr> Create(ECDbCR conn, DbTable const& tbl) {

            }
            static 
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
        bool _IsNull() const override { return GetSqliteValue(m_columnIndex).IsNull(); }
        void const* _GetBlob(int* blobSize) const override;
        bool _GetBoolean() const override { return GetSqliteValue(m_columnIndex).GetValueInt() != 0; }
        uint64_t _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const override;
        double _GetDateTimeJulianDays(DateTime::Info& metadata) const override;
        double _GetDouble() const override { return GetSqliteValue(m_columnIndex).GetValueDouble(); }
        int _GetInt() const override { return GetSqliteValue(m_columnIndex).GetValueInt(); }
        int64_t _GetInt64() const override { return GetSqliteValue(m_columnIndex).GetValueInt64(); }
        Utf8CP _GetText() const override { return GetSqliteValue(m_columnIndex).GetValueText(); }
        DPoint2d _GetPoint2d() const override;
        DPoint3d _GetPoint3d() const override;
        IGeometryPtr _GetGeometry() const override;
        IECSqlValue const& _GetStructMemberValue(Utf8CP memberName) const override;
        IECSqlValueIterable const& _GetStructIterable() const override;
        int _GetArrayLength() const override;
        IECSqlValueIterable const& _GetArrayIterable() const override;
        void UpdateDateTimeMetaData();

       public:
        PrimitiveField(ECSqlColumnInfo const& columnInfo, int columnIndex)
            : Field(columnInfo, false, false), m_columnIndex(columnIndex) {
            UpdateDateTimeMetaData();
        }
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
        bool IsPoint3d() const { return m_zColumnIndex >= 0; }

       public:
        PointField(ECSqlColumnInfo const& colInfo, int xColumnIndex, int yColumnIndex, int zColumnIndex)
            : Field(colInfo, false, false), m_xColumnIndex(xColumnIndex), m_yColumnIndex(yColumnIndex), m_zColumnIndex(zColumnIndex) {}
        PointField(ECSqlColumnInfo const& colInfo, int xColumnIndex, int yColumnIndex) : PointField(colInfo, xColumnIndex, yColumnIndex, -1) {}
        ~PointField() {}
    };
    struct StructField final : public Field, IECSqlValueIterable {
       private:
        struct IteratorState final : IECSqlValueIterable::IIteratorState {
           private:
            mutable std::map<Utf8CP, std::unique_ptr<Field>, CompareIUtf8Ascii>::const_iterator m_it;
            std::map<Utf8CP, std::unique_ptr<Field>, CompareIUtf8Ascii>::const_iterator m_endIt;
            IteratorState(IteratorState const& rhs) : m_it(rhs.m_it), m_endIt(rhs.m_endIt) {}
            std::unique_ptr<IIteratorState> _Copy() const override { return std::unique_ptr<IIteratorState>(new IteratorState(*this)); }
            void _MoveToNext(bool onInitializingIterator) const override {
                if (!onInitializingIterator)
                    ++m_it;
            }
            bool _IsAtEnd() const override { return m_it == m_endIt; }
            IECSqlValue const& _GetCurrent() const override { return *m_it->second; }

           public:
            explicit IteratorState(std::map<Utf8CP, std::unique_ptr<Field>, CompareIUtf8Ascii> const& memberFields) : IIteratorState(), m_it(memberFields.begin()), m_endIt(memberFields.end()) {}
        };

        std::map<Utf8CP, std::unique_ptr<Field>, CompareIUtf8Ascii> m_structMemberFields;

       private:
        bool _IsNull() const override;
        IECSqlValue const& _GetStructMemberValue(Utf8CP memberName) const override;
        IECSqlValueIterable const& _GetStructIterable() const override { return *this; }
        const_iterator _CreateIterator() const override { return IECSqlValueIterable::const_iterator(std::make_unique<IteratorState>(m_structMemberFields)); }
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
        StructField(ECSqlColumnInfo const& colInfo) : Field(colInfo, false, false) {}
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
                ArrayIteratorState(ArrayIteratorState const& rhs) : m_value(rhs.m_value), m_jsonIterator(rhs.m_jsonIterator), m_jsonIteratorIndex(rhs.m_jsonIteratorIndex) {}
                std::unique_ptr<IIteratorState> _Copy() const override { return std::unique_ptr<IIteratorState>(new ArrayIteratorState(*this)); }
                void _MoveToNext(bool onInitializingIterator) const override;
                bool _IsAtEnd() const override { return GetJson().IsNull() || m_jsonIterator == GetJson().End(); }
                IECSqlValue const& _GetCurrent() const override;
                rapidjson::Value const& GetJson() const { return m_value.m_json; }

               public:
                explicit ArrayIteratorState(JsonECSqlValue const& val) : IIteratorState(), m_value(val) {}
            };

            struct StructIteratorState final : IECSqlValueIterable::IIteratorState {
               private:
                JsonECSqlValue const& m_value;
                mutable ECN::ECPropertyIterable::const_iterator m_memberPropIterator;
                ECN::ECPropertyIterable::const_iterator m_memberPropEndIterator;

               private:
                StructIteratorState(StructIteratorState const& rhs) : m_value(rhs.m_value), m_memberPropIterator(rhs.m_memberPropIterator), m_memberPropEndIterator(rhs.m_memberPropEndIterator) {}
                std::unique_ptr<IIteratorState> _Copy() const override { return std::unique_ptr<IIteratorState>(new StructIteratorState(*this)); }
                void _MoveToNext(bool onInitializingIterator) const override;
                bool _IsAtEnd() const override { return m_memberPropIterator == m_memberPropEndIterator; }
                IECSqlValue const& _GetCurrent() const override;
                rapidjson::Value const& GetJson() const { return m_value.m_json; }

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
            ECSqlColumnInfoCR _GetColumnInfo() const override { return m_columnInfo; }
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
        bool _IsNull() const override { return GetSqliteValue(m_sqliteColumnIndex).IsNull(); }
        void const* _GetBlob(int* blobSize) const override { return GetValue().GetBlob(blobSize); }
        bool _GetBoolean() const override { return GetValue().GetBoolean(); }
        uint64_t _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const override { return GetValue().GetDateTimeJulianDaysMsec(metadata); }
        double _GetDateTimeJulianDays(DateTime::Info& metadata) const override { return GetValue().GetDateTimeJulianDays(metadata); }
        double _GetDouble() const override { return GetValue().GetDouble(); }
        int _GetInt() const override { return GetValue().GetInt(); }
        int64_t _GetInt64() const override { return GetValue().GetInt64(); }
        Utf8CP _GetText() const override { return GetValue().GetText(); }
        DPoint2d _GetPoint2d() const override { return GetValue().GetPoint2d(); }
        DPoint3d _GetPoint3d() const override { return GetValue().GetPoint3d(); }
        IGeometryPtr _GetGeometry() const override { return GetValue().GetGeometry(); }
        IECSqlValue const& _GetStructMemberValue(Utf8CP memberName) const override { return GetValue()[memberName]; }
        IECSqlValueIterable const& _GetStructIterable() const override { return GetValue().GetStructIterable(); }
        int _GetArrayLength() const override { return GetValue().GetArrayLength(); }
        IECSqlValueIterable const& _GetArrayIterable() const override { return GetValue().GetArrayIterable(); }
        ECSqlStatus _OnAfterReset() override;
        ECSqlStatus _OnAfterStep() override;
        void DoReset() const;
        JsonECSqlValue const& GetValue() const {
            BeAssert(m_value != nullptr);
            return *m_value;
        }

       public:
        ArrayField(ECSqlColumnInfo const& colInfo, int sqliteColumnIndex) : Field(colInfo, true, true), m_sqliteColumnIndex(sqliteColumnIndex) {}
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
            IteratorState(IteratorState const& rhs) : m_field(rhs.m_field), m_state(rhs.m_state) {}
            std::unique_ptr<IIteratorState> _Copy() const override { return std::unique_ptr<IIteratorState>(new IteratorState(*this)); }
            void _MoveToNext(bool onInitializingIterator) const override;
            bool _IsAtEnd() const override { return m_state == State::End; }
            IECSqlValue const& _GetCurrent() const override;

           public:
            explicit IteratorState(NavigationField const& field) : m_field(field) {}
        };

        std::unique_ptr<Field> m_idField;
        std::unique_ptr<Field> m_relClassIdField;

       private:
        bool _IsNull() const override {
            BeAssert(m_idField != nullptr);
            return m_idField->IsNull();
        }
        IECSqlValue const& _GetStructMemberValue(Utf8CP memberName) const override;
        IECSqlValueIterable const& _GetStructIterable() const override {
            return *this;
        }
        const_iterator _CreateIterator() const override {
            return const_iterator(std::make_unique<IteratorState>(*this));
        }
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
        NavigationField(ECSqlColumnInfo const& colInfo) : Field(colInfo, false, false) {}
        void SetMembers(std::unique_ptr<Field> idField, std::unique_ptr<Field> relClassIdField);
    };

   private:
    std::map<Utf8String, std::vector<std::unique_ptr<Field>>> m_fieldPerTableMap;
};

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void InstanceReader::PrimitiveField::UpdateDateTimeMetaData() {
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

struct NoopECSqlValue final : public IECSqlValue, IECSqlValueIterable {
   private:
    ECSqlColumnInfo m_dummyColumnInfo;
    static NoopECSqlValue const* s_singleton;
    NoopECSqlValue() : IECSqlValue() {}
    ~NoopECSqlValue() {}
    ECSqlColumnInfoCR _GetColumnInfo() const override { return m_dummyColumnInfo; }
    bool _IsNull() const override { return true; }
    void const* _GetBlob(int* blobSize) const override;
    bool _GetBoolean() const override { return false; }
    uint64_t _GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const override { return INT64_C(0); }
    double _GetDateTimeJulianDays(DateTime::Info& metadata) const override { return 0.0; }
    double _GetDouble() const override { return 0.0; }
    int _GetInt() const override { return 0; }
    int64_t _GetInt64() const override { return INT64_C(0); }
    Utf8CP _GetText() const override { return nullptr; }
    DPoint2d _GetPoint2d() const override { return DPoint2d::From(0.0, 0.0); }
    DPoint3d _GetPoint3d() const override { return DPoint3d::From(0.0, 0.0, 0.0); }
    IGeometryPtr _GetGeometry() const override { return nullptr; }
    IECSqlValue const& _GetStructMemberValue(Utf8CP memberName) const override { return *this; }
    IECSqlValueIterable const& _GetStructIterable() const override { return *this; }
    int _GetArrayLength() const override { return -1; }
    IECSqlValueIterable const& _GetArrayIterable() const override { return *this; }
    const_iterator _CreateIterator() const override { return end(); }

   public:
    static NoopECSqlValue const& GetSingleton();
};
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
// static
NoopECSqlValue const* NoopECSqlValue::s_singleton = nullptr;

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
// static
NoopECSqlValue const& NoopECSqlValue::GetSingleton() {
    if (s_singleton == nullptr)
        s_singleton = new NoopECSqlValue();

    return *s_singleton;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void const* NoopECSqlValue::_GetBlob(int* blobSize) const {
    if (blobSize != nullptr)
        *blobSize = -1;

    return nullptr;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void const* InstanceReader::PrimitiveField::_GetBlob(int* blobSize) const {
    auto val = GetSqliteValue(m_columnIndex);
    if (blobSize != nullptr)
        *blobSize = val.GetValueBytes();

    return val.GetValueBlob();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
uint64_t InstanceReader::PrimitiveField::_GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const {
    const double jd = _GetDateTimeJulianDays(metadata);
    return DateTime::RationalDayToMsec(jd);
}

//**** No-op implementations

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DPoint2d InstanceReader::PrimitiveField::_GetPoint2d() const {
    LOG.error("GetPoint2d cannot be called for columns which are not of the Point2d type.");
    return NoopECSqlValue::GetSingleton().GetPoint2d();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DPoint3d InstanceReader::PrimitiveField::_GetPoint3d() const {
    LOG.error("GetPoint3d cannot be called for columns which are not of the Point3d type.");
    return NoopECSqlValue::GetSingleton().GetPoint3d();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& InstanceReader::PrimitiveField::_GetStructMemberValue(Utf8CP memberName) const {
    LOG.error("GetStructMemberValue cannot be called for primitive columns.");
    return NoopECSqlValue::GetSingleton()[memberName];
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable const& InstanceReader::PrimitiveField::_GetStructIterable() const {
    LOG.error("GetStructIterable cannot be called for primitive columns.");
    return NoopECSqlValue::GetSingleton().GetStructIterable();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int InstanceReader::PrimitiveField::_GetArrayLength() const {
    LOG.error("GetArrayLength cannot be called for primitive columns.");
    return NoopECSqlValue::GetSingleton().GetArrayLength();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable const& InstanceReader::PrimitiveField::_GetArrayIterable() const {
    LOG.error("GetArrayIterable cannot be called for primitive columns.");
    return NoopECSqlValue::GetSingleton().GetArrayIterable();
}
//==================

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
bool InstanceReader::PointField::_IsNull() const {
    const auto xVal = GetSqliteValue(m_xColumnIndex);
    const auto yVal = GetSqliteValue(m_yColumnIndex);
    const auto zVal = IsPoint3d() ? GetSqliteValue(m_zColumnIndex) : DbValue(nullptr);

    const auto coordXValue = xVal.GetValueDouble();
    const auto coordYValue = yVal.GetValueDouble();
    const auto coordZValue = IsPoint3d() ? zVal.GetValueDouble() : 0.0;
    return (xVal.IsNull() || std::isinf(coordXValue) || std::isnan(coordXValue) || yVal.IsNull() || std::isinf(coordYValue) || std::isnan(coordYValue) || (IsPoint3d() && (zVal.IsNull() || std::isinf(coordZValue) || std::isnan(coordZValue))));
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DPoint2d InstanceReader::PointField::_GetPoint2d() const {
    if (IsPoint3d()) {
        LOG.error("GetValuePoint2d cannot be called for Point3d column. Call GetValuePoint3d instead.");
        return NoopECSqlValue::GetSingleton().GetPoint2d();
    }

    return DPoint2d::From(GetSqliteValue(m_xColumnIndex).GetValueDouble(),
                          GetSqliteValue(m_yColumnIndex).GetValueDouble());
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DPoint3d InstanceReader::PointField::_GetPoint3d() const {
    if (!IsPoint3d()) {
        LOG.error("GetValuePoint3d cannot be called for Point2d column. Call GetPoint2d instead.");
        return NoopECSqlValue::GetSingleton().GetPoint3d();
    }

    return DPoint3d::From(GetSqliteValue(m_xColumnIndex).GetValueDouble(),
                          GetSqliteValue(m_yColumnIndex).GetValueDouble(),
                          GetSqliteValue(m_zColumnIndex).GetValueDouble());
}

//****  no-op overrides ***

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void const* InstanceReader::PointField::_GetBlob(int* blobSize) const {
    LOG.error("GetBlob cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetBlob(blobSize);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
bool InstanceReader::PointField::_GetBoolean() const {
    LOG.error("GetBoolean cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetBoolean();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
uint64_t InstanceReader::PointField::_GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const {
    LOG.error("GetDateTime cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetDateTimeJulianDaysMsec(metadata);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
double InstanceReader::PointField::_GetDateTimeJulianDays(DateTime::Info& metadata) const {
    LOG.error("GetDateTime cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetDateTimeJulianDays(metadata);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
double InstanceReader::PointField::_GetDouble() const {
    LOG.error("GetDouble cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetDouble();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
int InstanceReader::PointField::_GetInt() const {
    LOG.error("GetInt cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetInt();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
int64_t InstanceReader::PointField::_GetInt64() const {
    LOG.error("GetInt64 cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetInt64();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8CP InstanceReader::PointField::_GetText() const {
    LOG.error("GetText cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetText();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
IGeometryPtr InstanceReader::PointField::_GetGeometry() const {
    LOG.error("GetGeometry cannot be called for Point2d or Point3d column. Call GetPoint2d / GetPoint3d instead.");
    return NoopECSqlValue::GetSingleton().GetGeometry();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& InstanceReader::PointField::_GetStructMemberValue(Utf8CP memberName) const {
    LOG.error("GetStructMemberValue cannot be called for Point2d or Point3d columns.");
    return NoopECSqlValue::GetSingleton()[memberName];
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable const& InstanceReader::PointField::_GetStructIterable() const {
    LOG.error("GetStructIterable cannot be called for Point2d or Point3d columns.");
    return NoopECSqlValue::GetSingleton().GetStructIterable();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int InstanceReader::PointField::_GetArrayLength() const {
    LOG.error("GetArrayLength cannot be called for Point2d or Point3d columns.");
    return NoopECSqlValue::GetSingleton().GetArrayLength();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable const& InstanceReader::PointField::_GetArrayIterable() const {
    LOG.error("GetArrayIterable cannot be called for Point2d or Point3d columns.");
    return NoopECSqlValue::GetSingleton().GetArrayIterable();
}
/////////////////////////////////

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
bool InstanceReader::StructField::_IsNull() const {
    for (auto const& field : m_structMemberFields) {
        if (!field.second->IsNull())
            return false;
    }

    return true;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlValue const& InstanceReader::StructField::_GetStructMemberValue(Utf8CP memberName) const {
    auto it = m_structMemberFields.find(memberName);
    if (it == m_structMemberFields.end()) {
        LOG.errorv("Struct member '%s' passed to struct IECSqlValue[Utf8CP] does not exist.", memberName);
        return NoopECSqlValue::GetSingleton()[memberName];
    }

    return *it->second;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InstanceReader::StructField::AppendField(std::unique_ptr<Field> field) {
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

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void const* InstanceReader::StructField::_GetBlob(int* blobSize) const {
    LOG.error("GetBlob cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetBlob(blobSize);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
bool InstanceReader::StructField::_GetBoolean() const {
    LOG.error("GetBoolean cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetBoolean();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
uint64_t InstanceReader::StructField::_GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const {
    LOG.error("GetDateTime cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetDateTimeJulianDaysMsec(metadata);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
double InstanceReader::StructField::_GetDateTimeJulianDays(DateTime::Info& metadata) const {
    LOG.error("GetDateTime cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetDateTimeJulianDays(metadata);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
double InstanceReader::StructField::_GetDouble() const {
    LOG.error("GetDouble cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetDouble();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
int InstanceReader::StructField::_GetInt() const {
    LOG.error("GetInt cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetInt();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
int64_t InstanceReader::StructField::_GetInt64() const {
    LOG.error("GetInt64 cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetInt64();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8CP InstanceReader::StructField::_GetText() const {
    LOG.error("GetText cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetText();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
IGeometryPtr InstanceReader::StructField::_GetGeometry() const {
    LOG.error("GetGeometry cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetGeometry();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DPoint2d InstanceReader::StructField::_GetPoint2d() const {
    LOG.error("GetPoint2d cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetPoint2d();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DPoint3d InstanceReader::StructField::_GetPoint3d() const {
    LOG.error("GetPoint3d cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetPoint3d();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int InstanceReader::StructField::_GetArrayLength() const {
    LOG.error("GetArrayLength cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetArrayLength();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable const& InstanceReader::StructField::_GetArrayIterable() const {
    LOG.error("GetArrayIterable cannot be called for a struct column.");
    return NoopECSqlValue::GetSingleton().GetArrayIterable();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus InstanceReader::StructField::_OnAfterStep() {
    for (auto const& memberField : m_structMemberFields) {
        ECSqlStatus stat = memberField.second->OnAfterStep();
        if (!stat.IsSuccess())
            return stat;
    }

    return ECSqlStatus::Success;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus InstanceReader::StructField::_OnAfterReset() {
    for (auto const& memberField : m_structMemberFields) {
        ECSqlStatus stat = memberField.second->OnAfterReset();
        if (!stat.IsSuccess())
            return stat;
    }

    return ECSqlStatus::Success;
}

//===================================
//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
void InstanceReader::NavigationField::SetMembers(std::unique_ptr<Field> idField, std::unique_ptr<Field> relClassIdField) {
    BeAssert(idField != nullptr && !idField->RequiresOnAfterReset() && !idField->RequiresOnAfterStep());
    BeAssert(relClassIdField != nullptr && !relClassIdField->RequiresOnAfterReset() && !relClassIdField->RequiresOnAfterStep());
    m_idField = std::move(idField);
    m_relClassIdField = std::move(relClassIdField);
    BeAssert(m_idField != nullptr && m_relClassIdField != nullptr);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
IECSqlValue const& InstanceReader::NavigationField::_GetStructMemberValue(Utf8CP memberName) const {
    if (BeStringUtilities::StricmpAscii(memberName, "Id") == 0)
        return *m_idField;

    if (BeStringUtilities::StricmpAscii(memberName, "RelECClassId") == 0)
        return *m_relClassIdField;

    LOG.errorv("Member name '%s' passed to navigation property IECSqlValue[Utf8CP] does not exist.", memberName);
    return NoopECSqlValue::GetSingleton()[memberName];
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus InstanceReader::NavigationField::_OnAfterStep() {
    ECSqlStatus stat = m_idField->OnAfterStep();
    if (!stat.IsSuccess())
        return stat;

    return m_relClassIdField->OnAfterStep();
}

//---------------------------------------------------------------------------------------
// @bsimethod
//---------------------------------------------------------------------------------------
ECSqlStatus InstanceReader::NavigationField::_OnAfterReset() {
    ECSqlStatus stat = m_idField->OnAfterReset();
    if (!stat.IsSuccess())
        return stat;

    return m_relClassIdField->OnAfterReset();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
void const* InstanceReader::NavigationField::_GetBlob(int* blobSize) const {
    LOG.error("GetBlob cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetBlob(blobSize);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
bool InstanceReader::NavigationField::_GetBoolean() const {
    LOG.error("GetBoolean cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetBoolean();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
uint64_t InstanceReader::NavigationField::_GetDateTimeJulianDaysMsec(DateTime::Info& metadata) const {
    LOG.error("GetDateTime cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetDateTimeJulianDaysMsec(metadata);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
double InstanceReader::NavigationField::_GetDateTimeJulianDays(DateTime::Info& metadata) const {
    LOG.error("GetDateTime cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetDateTimeJulianDays(metadata);
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
double InstanceReader::NavigationField::_GetDouble() const {
    LOG.error("GetDouble cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetDouble();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
int InstanceReader::NavigationField::_GetInt() const {
    LOG.error("GetInt cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetInt();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
int64_t InstanceReader::NavigationField::_GetInt64() const {
    LOG.error("GetInt64 cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetInt64();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
Utf8CP InstanceReader::NavigationField::_GetText() const {
    LOG.error("GetText cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetText();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+--------
IGeometryPtr InstanceReader::NavigationField::_GetGeometry() const {
    LOG.error("GetGeometry cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetGeometry();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DPoint2d InstanceReader::NavigationField::_GetPoint2d() const {
    LOG.error("GetPoint2d cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetPoint2d();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DPoint3d InstanceReader::NavigationField::_GetPoint3d() const {
    LOG.error("GetPoint3d cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetPoint3d();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
int InstanceReader::NavigationField::_GetArrayLength() const {
    LOG.error("GetArrayLength cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetArrayLength();
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValueIterable const& InstanceReader::NavigationField::_GetArrayIterable() const {
    LOG.error("GetArrayIterable cannot be called for a navigation property column.");
    return NoopECSqlValue::GetSingleton().GetArrayIterable();
}

//***************************** NavigationPropertyECSqlField::IteratorState *****************

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
void InstanceReader::NavigationField::IteratorState::_MoveToNext(bool onInitializingIterator) const {
    uint8_t current = (uint8_t)m_state;
    current++;
    m_state = (State)current;
}

//-----------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
IECSqlValue const& InstanceReader::NavigationField::IteratorState::_GetCurrent() const {
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

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::vector<InstanceReader::Field::Ptr> InstanceReader::Factory::Create(ClassMapCR classMap, std::function<TableView const*(DbTable const&)> getTable ) {
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
ECSqlPropertyPath InstanceReader::Factory::GetPropertyPath (PropertyMap const& propertyMap) {
    ECSqlPropertyPath propertyPath;
    for(auto& part : propertyMap.GetPath()){
        propertyPath.AddEntry(part->GetProperty());
    }
    return propertyPath;
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<InstanceReader::Field> InstanceReader::Factory::CreatePrimitiveField(PropertyMap const& propertyMap, DbTable const& tbl) {
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
std::unique_ptr<InstanceReader::Field>  InstanceReader::Factory::CreateSystemField(PropertyMap const& propertyMap, DbTable const& tbl) {
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
         return std::make_unique<PrimitiveField>(stmt, columnInfo, tbl.GetClassIdCol());
    }
    if (extendedType == ExtendedTypeHelper::ExtendedType::SourceClassId && tbl.GetSourceClassIdCol() >= 0) {
         return std::make_unique<PrimitiveField>(stmt, columnInfo, tbl.GetSourceClassIdCol());
    }
    if (extendedType == ExtendedTypeHelper::ExtendedType::TargetClassId && tbl.GetTargetClassIdCol() >= 0) {
         return std::make_unique<PrimitiveField>(stmt, columnInfo, tbl.GetTargetClassIdCol());
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
std::unique_ptr<InstanceReader::Field> InstanceReader::Factory::CreateStructField(PropertyMap const& propertyMap, DbTable const& tbl) {
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

    auto newStructField = std::make_unique<StructField>(columnInfo);
    auto& structPropertyMap =  propertyMap.GetAs<StructPropertyMap>();
    for(auto& memberMap : structPropertyMap) {
        newStructField->AppendField(CreateField(*memberMap, tbl));
    }
    return std::move(newStructField);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<InstanceReader::Field>  InstanceReader::Factory::CreateClassIdField(PropertyMap const& propertyMap, ECN::ECClassId id, DbTable const& tbl) {
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

    return std::make_unique<ClassIdECSqlField>(columnInfo, id);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<InstanceReader::Field>  InstanceReader::Factory::CreateNavigationField(PropertyMap const& propertyMap, DbTable const& tbl) {
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
    auto idField = CreatePrimitiveField(navMap.GetIdPropertyMap(), tbl);

    std::unique_ptr<Field> relClassIdField;
    auto& relClassIdMap = navMap.GetRelECClassIdPropertyMap();
    if (relClassIdMap.GetColumn().IsVirtual()){
        relClassIdField = CreateClassIdField(relClassIdMap, prim->GetRelationshipClass()->GetId(), tbl);
    } else {
        relClassIdField = CreatePrimitiveField(navMap.GetRelECClassIdPropertyMap(), tbl);
    }
    auto navField = std::make_unique<NavigationField>( columnInfo);
    navField->SetMembers(std::move(idField), std::move(relClassIdField));
    return std::move(navField);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
DateTime::Info InstanceReader::Factory::GetDateTimeInfo(PropertyMap const& propertyMap) {
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
std::unique_ptr<InstanceReader::Field>   InstanceReader::Factory::CreateArrayField(PropertyMap const& propertyMap, DbTable const& tbl) {

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
    auto nCol = tbl.GetColumnIndexOf(primMap.GetColumn());
    return std::make_unique<ArrayField>( columnInfo, nCol);
}

//---------------------------------------------------------------------------------------
// @bsimethod
//+---------------+---------------+---------------+---------------+---------------+------
std::unique_ptr<InstanceReader::Field>  InstanceReader::Factory::CreateField(PropertyMap const& propertyMap, DbTable const& tbl){
    const auto& prop = propertyMap.GetProperty();
    if (propertyMap.IsSystem()) {
        return CreateSystemField(propertyMap, tbl);
    } else if (prop.GetIsPrimitive() ){
        return CreatePrimitiveField(propertyMap, tbl);
    } else if (prop.GetIsStruct()) {
        return CreateStructField(propertyMap, tbl);
    } else if (prop.GetIsNavigation()) {
        return CreateNavigationField(propertyMap, tbl);
    } else if(prop.GetIsArray()) {
        return CreateArrayField(propertyMap, tbl);
    }
    return nullptr;
}

END_BENTLEY_SQLITE_EC_NAMESPACE