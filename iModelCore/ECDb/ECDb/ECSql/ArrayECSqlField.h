/*--------------------------------------------------------------------------------------+
|
|     $Source: ECDb/ECSql/ArrayECSqlField.h $
|
|  $Copyright: (c) 2017 Bentley Systems, Incorporated. All rights reserved. $
|
+--------------------------------------------------------------------------------------*/
#pragma once
//__BENTLEY_INTERNAL_ONLY__
#include "ECSqlField.h"
#include <rapidjson/BeRapidJson.h>
#include <Bentley/ByteStream.h>

BEGIN_BENTLEY_SQLITE_EC_NAMESPACE

//=======================================================================================
//! @bsiclass                                                Krischan.Eberle      02/2016
//+===============+===============+===============+===============+===============+======
struct ArrayECSqlField final : public ECSqlField
    {
public:
    //=======================================================================================
    //! @bsiclass                                                Krischan.Eberle      03/2016
    //+===============+===============+===============+===============+===============+======
    struct JsonECSqlValue final : public IECSqlValue, IECSqlValue::IIterable
        {
        private:
            struct ArrayIteratorState final : IECSqlValue::IIteratorState
                {
            private:
                JsonECSqlValue const& m_value;
                mutable rapidjson::Value::ConstValueIterator m_jsonIterator;
                mutable int m_jsonIteratorIndex = -1;
                mutable std::unique_ptr<JsonECSqlValue> m_currentArrayElement = nullptr;

                ArrayIteratorState(ArrayIteratorState const& rhs) : m_value(rhs.m_value), m_jsonIterator(rhs.m_jsonIterator), m_jsonIteratorIndex(rhs.m_jsonIteratorIndex), m_currentArrayElement(std::move(rhs.m_currentArrayElement)) {}
                
                std::unique_ptr<IIteratorState> _Copy() const override {return std::unique_ptr<IIteratorState>(new ArrayIteratorState(*this)); }
                void _MoveToNext(bool onInitializingIterator) const override;
                bool _IsAtEnd() const override { return m_jsonIterator == GetJson().End(); }
                IECSqlValue const& _GetCurrent() const override { BeAssert(m_currentArrayElement != nullptr); return *m_currentArrayElement; }

                rapidjson::Value const& GetJson() const { return m_value.m_json; }

                public:
                   explicit ArrayIteratorState(JsonECSqlValue const& val) : IIteratorState(), m_value(val) {}

                };

            struct StructIteratorState final : IECSqlValue::IIteratorState
                {
                private:
                    JsonECSqlValue const& m_value;
                    mutable rapidjson::Value::ConstMemberIterator m_jsonIterator;

                    StructIteratorState(StructIteratorState const& rhs) : m_value(rhs.m_value), m_jsonIterator(rhs.m_jsonIterator) {}

                    std::unique_ptr<IIteratorState> _Copy() const override { return std::unique_ptr<IIteratorState>(new StructIteratorState(*this)); }
                    void _MoveToNext(bool onInitializingIterator) const override;
                    bool _IsAtEnd() const override { return m_jsonIterator == GetJson().MemberEnd(); }
                    IECSqlValue const& _GetCurrent() const override;

                    rapidjson::Value const& GetJson() const { return m_value.m_json; }

                public:
                    explicit StructIteratorState(JsonECSqlValue const& val) : IIteratorState(), m_value(val) {}
                };

            static rapidjson::Value const* s_nullJson;

            ECDbCR m_ecdb;
            rapidjson::Value const& m_json;
            ECSqlColumnInfo m_columnInfo;
            //for prims
            mutable ByteStream m_blobCache;
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
            IIterable const& _GetStructIterable() const override;

            int _GetArrayLength() const override;
            IIterable const& _GetArrayIterable() const override;

            const_iterator _CreateIterator() const override;


            IECSqlValue const& CreateStructMemberValue(ECN::ECPropertyCR memberProp, rapidjson::Value const& memberJsonValue) const;

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

    //IECSqlValue
    bool _IsNull() const override { return GetSqliteStatement().IsColumnNull(m_sqliteColumnIndex); }

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
    IIterable const& _GetStructIterable() const override { return GetValue().GetStructIterable(); }

    int _GetArrayLength() const override { return GetValue().GetArrayLength(); }
    IIterable const& _GetArrayIterable() const override { return GetValue().GetArrayIterable(); }

    //ECSqlField
    ECSqlStatus _OnAfterReset() override;
    ECSqlStatus _OnAfterStep() override;

    void DoReset() const;

    JsonECSqlValue const& GetValue() const { BeAssert(m_value != nullptr); return *m_value; }
public:
    ArrayECSqlField(ECSqlStatementBase& stmt, ECSqlColumnInfo const& colInfo, int sqliteColumnIndex) : ECSqlField(stmt, colInfo, true, true), m_sqliteColumnIndex(sqliteColumnIndex) {}
    ~ArrayECSqlField() {}
    };
END_BENTLEY_SQLITE_EC_NAMESPACE

